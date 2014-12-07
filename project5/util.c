#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define MAXLOGSIZE 5


static int master_fd = -1;
pthread_mutex_t accept_con_mutex = PTHREAD_MUTEX_INITIALIZER;
int socket_n;

// this function takes a hostname and returns the IP address
int lookup_host (const char *host)
{
    struct addrinfo hints, *res;
    int errcode;
    char addrstr[100];
    void *ptr;

    memset (&hints, 0, sizeof (hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_CANONNAME;

    errcode = getaddrinfo (host, NULL, &hints, &res);
    if (errcode != 0)
    {
        perror ("getaddrinfo");
        return -1;
    }

    printf ("Host: %s\n", host);
    while (res)
    {
        inet_ntop (res->ai_family, res->ai_addr->sa_data, addrstr, 100);

        switch (res->ai_family)
        {
        case AF_INET:
            ptr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;
            break;
        case AF_INET6:
            ptr = &((struct sockaddr_in6 *) res->ai_addr)->sin6_addr;
            break;
        }
        inet_ntop (res->ai_family, ptr, addrstr, 100);
        printf ("IPv%d address: %s (%s)\n", res->ai_family == PF_INET6 ? 6 : 4,
                addrstr, res->ai_canonname);
        res = res->ai_next;
    }

    return 0;
}

int makeargv(const char *s, const char *delimiters, char ***argvp)
{
    int error;
    int i;
    int numtokens;
    const char *snew;
    char *t;

    if ((s == NULL) || (delimiters == NULL) || (argvp == NULL))
    {
        errno = EINVAL;
        return -1;
    }
    *argvp = NULL;
    snew = s + strspn(s, delimiters);
    if ((t = malloc(strlen(snew) + 1)) == NULL)
        return -1;
    strcpy(t, snew);
    numtokens = 0;
    if (strtok(t, delimiters) != NULL)
        for (numtokens = 1; strtok(NULL, delimiters) != NULL; numtokens++) ;

    if ((*argvp = malloc((numtokens + 1) * sizeof(char *))) == NULL)
    {
        error = errno;
        free(t);
        errno = error;
        return -1;
    }

    if (numtokens == 0)
        free(t);
    else
    {
        strcpy(t, snew);
        **argvp = strtok(t, delimiters);
        for (i = 1; i < numtokens; i++)
            *((*argvp) + i) = strtok(NULL, delimiters);
    }
    *((*argvp) + numtokens) = NULL;
    return numtokens;
}

void freemakeargv(char **argv)
{
    if (argv == NULL)
        return;
    if (*argv != NULL)
        free(*argv);
    free(argv);
}

/**********************************************
 * init
   - port is the number of the port you want the server to be
     started on
   - initializes the connection acception/handling system
   - YOU MUST CALL THIS EXACTLY ONCE (not once per thread,
     but exactly one time, in the main thread of your program)
     BEFORE USING ANY OF THE FUNCTIONS BELOW
   - if init encounters any errors, it will call exit().
************************************************/
enum boolean {FALSE, TRUE};
void init(int port)
{
    int error;
    int true = 1;

    if ((socket_n = (socket(PF_INET, SOCK_STREAM, 0))) == -1)
    {
        perror("ERROR: Failed to create socket!");
        exit(-1);
        return;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons((short) port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (setsockopt(socket_n, SOL_SOCKET, SO_REUSEADDR, (char *)&true, sizeof(int)) == -1)
    {
        perror("ERROR: Failed to set socket!");
        return;
    }

    if (bind(socket_n, (struct sockaddr *) &server, sizeof(server)) == -1)
    {
        error = errno;
        while ((close(socket_n) == -1) && (errno == EINTR));
        errno = error;
        perror("ERROR: Failed to bind socket to the port");
        exit(-1);
        return;
    }
    listen(socket_n, MAXLOGSIZE);
}

/**********************************************
 * accept_connection - takes no parameters
   - returns a file descriptor for further request processing.
     DO NOT use the file descriptor on your own -- use
     get_request() instead.
   - if the return value is negative, the thread calling
     accept_connection must exit by calling pthread_exit().
***********************************************/
int accept_connection(void)
{
    printf("Entered 'accept_connection' \n");
    struct sockaddr_in  address;
    int  length;
    int conn = accept(sock, (struct sockaddr *) (&address), &length);

    if (conn == -1)
    {
        perror("ERROR: Failed to accept connection: ");
    }
    return conn;
}

/**********************************************
 * get_request
   - parameters:
      - fd is the file descriptor obtained by accept_connection()
        from where you wish to get a request
      - filename is the location of a character buffer in which
        this function should store the requested filename. (Buffer
        should be of size 1024 bytes.)
   - returns 0 on success, nonzero on failure. You must account
     for failures because some connections might send faulty
     requests. This is a recoverable error - you must not exit
     inside the thread that called get_request. After an error, you
     must NOT use a return_request or return_error function for that
     specific 'connection'.
************************************************/
int get_request(int fd, char *filename)
{
    printf("entered get_request \n");
    size_t num;
    char *buffer = NULL;
    FILE* file; 
    file = fdopen(fd, "r");

    if (file == NULL)
    {
        perror("Failed to open file: ");
        return -1;
    }

    getline(&buffer, &num, file);
    if (fflush(file) != 0)
    {
        perror("Failed file flush: ");
    }

    pthread_mutex_lock(&accept_con_mutex);
    strtok(buffer, " ");
    filename = strcpy(filename, strtok(NULL, " "));
    pthread_mutex_unlock(&accept_con_mutex);

    free(buffer);
    return 0;
}

/**********************************************
 * return_result
   - returns the contents of a file to the requesting client
   - parameters:
      - fd is the file descriptor obtained by accept_connection()
        to where you wish to return the result of a request
      - content_type is a pointer to a string that indicates the
        type of content being returned. possible types include
        "text/html", "text/plain", "image/gif", "image/jpeg" cor-
        responding to .html, .txt, .gif, .jpg files.
      - buf is a pointer to a memory location where the requested
        file has been read into memory (the heap). return_result
        will use this memory location to return the result to the
        user. (remember to use -D_REENTRANT for CFLAGS.) you may
        safely deallocate the memory after the call to
        return_result (if it will not be cached).
      - numbytes is the number of bytes the file takes up in buf
   - returns 0 on success, nonzero on failure.
************************************************/
int return_result(int fd, char *content_type, char *buf, int numbytes)
{

}


/**********************************************
 * return_error
   - returns an error message in response to a bad request
   - parameters:
      - fd is the file descriptor obtained by accept_connection()
        to where you wish to return the error
      - buf is a pointer to the location of the error text
   - returns 0 on success, nonzero on failure.
************************************************/
int return_error(int fd, char *buf)
{
    pthread_mutex_lock(&accept_con_mutex);

    FILE *errorLogFile = fdopen(fd, "a");
    if (errorLogFile == NULL)
    {
        perror("ERROR: Bad file descriptor in return_result: "); // Will eventually have something in return_result with the fd so this check will work
        return -1;
    }

    fprintf(errorLogFile, "HTTP/1.1 404 Not Found\nContent-Type: text/html\nContent-Length: %s\nConnection: Close\n\n%s", "read error" , buf); //sizeof(buffer));
    if (fclose(errorLogFile) != 0)
    {
        perror("ERROR: fclose failed to close result file");
        return -1;
    }

    pthread_mutex_unlock(&accept_con_mutex);
    return 0;
}

