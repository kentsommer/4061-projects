/*CSci4061 F2014 Assignment 4
*section: 4
*date: 12/2/14
*names: Kent Sommer, Kanad Gupta, Xi Chen
*id: somme282, kgupta, chen2806
*/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "util.h"
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_THREADS 100
#define MAX_QUEUE_SIZE 100
#define MAX_REQUEST_LENGTH 64

/*SETUP THE LOCKS*/
pthread_mutex_t getRequest = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t accessRequest = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t accessRequest2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t accessOutput = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t waitLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t acceptConnection = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mallocLOCK = PTHREAD_MUTEX_INITIALIZER;

//Structure for Queue.
typedef struct request_queue
{
	int		m_socket;
	char	m_szRequest[MAX_REQUEST_LENGTH];
} request_queue_t;

//Structure for Worker
typedef struct worker
{
	int numworkers;
	char  path[MAX_REQUEST_LENGTH];
} worker_t;

int sizeofqueue = 0;
int maxqueue = 0;
int writen =0;


request_queue_t glblQueue[MAX_QUEUE_SIZE];
FILE *filelog;
sem_t accessible;
sem_t done;

//Lock a given lock
void uLock(pthread_mutex_t *mutex)
{
	if(pthread_mutex_lock(mutex))
	{
		perror("Error locking");
	}
}

//Unlock a given lock
void uUnlock(pthread_mutex_t *mutex)
{
	if(pthread_mutex_unlock(mutex))
	{
		perror("Error unlocking");
	}
}

//Destroy a given lock
void uLockDestroy(pthread_mutex_t *mutex)
{
	if(pthread_mutex_destroy(mutex))
	{
		perror("Error destroying lock");
	}
}

//Check sem post
void uSemPost(sem_t *sem)
{
	if(sem_post(sem))
	{
		perror("Error Semaphore Posting");
	}
}


//Check sem wait
void uSemWait(sem_t *sem)
{
	if(sem_wait(sem))
	{
		perror("Error Semaphore Waiting");
	}
}

//Thread safe implementation for malloc
void * uMalloc(int size)
{
	void *pointer_to_allocated_memory;	
	uLock(&mallocLOCK);	
	pointer_to_allocated_memory=malloc(size);
	if(pointer_to_allocated_memory==NULL)	
	{
	   perror("Failed to Malloc");
	   return NULL;
	}
	uUnlock(&mallocLOCK);
	return pointer_to_allocated_memory;
}

//Pushes onto a given queue
void uPush(request_queue_t * queue, request_queue_t toplace)
{
	if(sizeofqueue == maxqueue)
	{
		//This should never happen 
		printf("Error: max size of queue reached\n");
	}
	sizeofqueue++;

	queue[sizeofqueue-1] = toplace;	
}

//Pops off a given queue
void uPop(request_queue_t * queue)
{
	if(sizeofqueue == 0)
	{
		printf("Error: Nothing in queue\n");
	}
	request_queue_t empty;
	int i =0;
	for(i = 0; i < sizeofqueue; i++)
	{
		if((i+1) == sizeofqueue)
		{
			queue[i] = empty;
		}
		else
		{
			queue[i] = queue[i+1];
		}
	}
	sizeofqueue = sizeofqueue -1;
	queue[i] = empty;
}

void * dispatch(void * arg)
{
	while(1)
	{
		request_queue_t request;
		uLock(&acceptConnection);
		request.m_socket = accept_connection();
		uUnlock(&acceptConnection);

		if(request.m_socket < 0)
		{
			strcpy(request.m_szRequest, "end");
			uLock(&accessRequest);
			uSemWait(&done);//Take a spot or wait until one is available
			uLock(&accessRequest2);//Ensure that memcpy happens in a separate memcpy
			uPush(glblQueue,request);
			uUnlock(&accessRequest2);
			uSemPost(&accessible);
			uUnlock(&accessRequest);
			pthread_exit(NULL);
		}

		//Lock for get_request
		uLock(&getRequest);
		int req = get_request(request.m_socket , request.m_szRequest);
		uUnlock(&getRequest);

		if(req == 0)
		{
			uLock(&accessRequest);//Lock until worker has copied request.
			uSemWait(&done);//Set a spot as taken
			uLock(&accessRequest2);//Lock for memcopy
			uPush(glblQueue,request);
			uUnlock(&accessRequest2);
			uSemPost(&accessible);
			uUnlock(&accessRequest);
		}
		else
		{
			printf("Not a valid request.\n");
		}
	}
	return NULL;
}

void * worker(void * arg)
{
	worker_t * workerStruct;
	workerStruct =  (worker_t*) arg;
	int numworkers = workerStruct->numworkers;
	if(workerStruct == NULL)
	{
		perror("Failed: worker is NULL");
		return NULL;
	}

	int numberReqDone=0;
	while(1)
	{
		request_queue_t current;
		uLock(&waitLock);
		uSemWait(&accessible);
		uLock(&accessRequest2);
		current = glblQueue[0];

		if(current.m_szRequest == "end")
		{
			printf("ending early\n");
			uSemPost(&accessible);
			uSemPost(&done);
			uUnlock(&accessRequest2);
			uUnlock(&waitLock);
			pthread_exit(NULL);
		}
		else
		{
			uPop(glblQueue);
		}
		uUnlock(&accessRequest2);

		//Set spot on queue as empty
		uSemPost(&done);	
		uUnlock(&waitLock);

		//Get request filetype
		char type[12];
		if(strstr(current.m_szRequest , ".html") || strstr(current.m_szRequest , "htm"))
		{
			strcpy(type, "text/html");
		}
		else if(strstr(current.m_szRequest , ".jpg"))
		{
			strcpy(type, "image/jpeg");
		}
		else if(strstr(current.m_szRequest, ".gif"))
		{
			strcpy(type, "image/gif");
		}
		else
		{
			strcpy(type, "text/plain");
		}


		char  pathcopy[MAX_REQUEST_LENGTH];
		strcpy(pathcopy, workerStruct->path);
		strcpy(current.m_szRequest, strcat(pathcopy, current.m_szRequest));
		FILE * data = fopen(current.m_szRequest , "rb");
		int size;

		if(data == NULL)
		{
			perror("Failed to open file");
		}
		else
		{
			int fd = fileno(data);
			struct stat statStruct;
			if (fstat(fd, &statStruct) >= 0)
			{
				size = (int) statStruct.st_size;// originally st+size is of type off_t not sure if int
			}
			else
			{
				//fstat failed
				perror("Failure to retrieve size from fstat");
			}
		}
		char error[25];
		int errnum = 0;
		char *mData = uMalloc(size+1);
		int numread;
		numberReqDone++;

		//Read file
		if(data) 
		{
			if((numread = fread(mData, 1, size, data)) >= 0)
			{	
				fclose(data);
				return_result(current.m_socket, type,  mData, numread);
			}				
			else
			{
				strcpy(error, "failed to read file");
				errnum = 1;
				return_error(current.m_socket, error);
			}
		}
		else
		{
			strcpy(error, "failed to open file");
			errnum = 1;
			return_error(current.m_socket, error);
		}
		free(mData);


		//Setup and output to log
		uLock(&accessOutput);
		if(errnum==1)
		{
			if (fprintf(filelog, "[%d][%d][%d][%s][%s]\n", numworkers, numberReqDone, current.m_socket, current.m_szRequest, error) <= 0)
			{
				printf("ERROR: THERE IS A PROBLEM WITH THE LOGFILE");
			}
		}
		else
		{
			if (fprintf(filelog, "[%d][%d][%d][%s][%d]\n", numworkers, numberReqDone, current.m_socket, current.m_szRequest, numread) <= 0) 
			{
				printf("ERROR: THERE IS A PROBLEM WITH THE LOGFILE");	
			}		
		}
		if(fflush(filelog))
		{
			perror("ERROR: LOGFILE CLOSED UNEXPECTEDLY");
		}
		uUnlock(&accessOutput);
	}
	return NULL;
}

int main(int argc, char **argv)
{

	///////////////////////////////////////////////////
	//////            ERROR CHECKING             //////
	///////////////////////////////////////////////////

	if(argc != 6 && argc != 7)
	{
		printf("usage: %s port path num_dispatcher num_workers queue_length [cache_size]\n", argv[0]);
		return -1;
	}

	if(atoi(argv[1])<1025 || atoi(argv[1])>65535)
	{
		printf("ERROR: PORT NUMBER %s OUT OF USABLE PORT RANGE, PICK A PORT 1025-65535\n", argv[1]);
		return -1;
	}
	if(argv[2]==NULL)
	{
		printf("ERROR: PATH IS INVALID");
		return -1;
	}
	if(atoi(argv[3])>MAX_THREADS)
	{
		printf("ERROR: MAXIMUM NUMBER OF DISPATCHER THREADS EXCEDED, %s IS INVALID\n", argv[3]);
		return -1;
	}
	if(atoi(argv[4])>MAX_THREADS)
	{
		printf("ERROR: MAXIMUM NUMBER OF WORKER THREADS EXCEDED, %s IS INVALID\n", argv[4]);
		return -1;
	}

	if(atoi(argv[5])>MAX_QUEUE_SIZE)
	{
		printf("ERROR: MAXIMUM QUEUE SIZE EXCEEDED, %s IS INVALID\n", argv[5]);
		return -1;
	}

	///////////////////////////////////////////////////
	//////             INITIALIZATION            //////
	///////////////////////////////////////////////////

	init(atoi(argv[1]));
	maxqueue = atoi(argv[5]);
	//Setup the semaphores 
	sem_init(&accessible,0,0);
	sem_init(&done,0,maxqueue);

	filelog = fopen("web_server_log", "workerStruct");
	if(filelog==NULL)
	{
		printf("ERROR: COULD NOT OPEN LOG FILE\n");
	}

	//Setup the threads
	pthread_t dispatch_thread[MAX_THREADS], worker_thread[MAX_THREADS];

	int i;
	for(i = 0; i < atoi(argv[3]); i++)
	{
		int err = pthread_create(&(dispatch_thread[i]), NULL, dispatch, NULL);
		if(err != 0)
		{
			//failed pthread_create 
			while (err == EAGAIN)
			{
				err = pthread_create(&(dispatch_thread[i]), NULL, dispatch, NULL);
			}
		}
	}

	for(i = 0; i < atoi(argv[4]); i++)
	{
		worker_t workerStruct;
		workerStruct.numworkers = i;
		strcpy(workerStruct.path, argv[2]);
		int err = pthread_create(&(worker_thread[i]), NULL, worker, &workerStruct);
		if(err != 0)
		{
			//failed to pthread_create 
			while (err == EAGAIN)
			{
				err = pthread_create(&(worker_thread[i]), NULL, worker, &workerStruct);
			}
		}
	}

	//Join up the threads 
	for(i = 0; i < atoi(argv[4]); i++)
	{
		if(pthread_join(worker_thread[i], NULL))
		{
			perror("ERROR: COULD NOT JOIN THREAD \"worker_thread\"");
		}
	}
	for(i = 0; i < atoi(argv[3]); i++)
	{
		if(pthread_join(dispatch_thread[i], NULL))
		{
			perror("ERROR: COULD NOT JOIN THREAD \"dispatch_thread\"");
		}
	}

	///////////////////////////////////////////////////
	////// FINAL CLEANUP OF LOCKS AND SEMAPHORES //////
	///////////////////////////////////////////////////

	//Destroy locks
	uLockDestroy(&accessRequest);
	uLockDestroy(&accessRequest2);
	uLockDestroy(&accessOutput);
	uLockDestroy(&waitLock);
	uLockDestroy(&mallocLOCK);
	uLockDestroy(&getRequest);
	//Destroy semaphores
	if(sem_destroy(&done))
	{
		perror("ERROR: COULD NOT DESTROY SEMAPHORE \"DONE\"");
	}
	if(sem_destroy(&accessible))
	{
		perror("ERROR: COULD NOT DESTROY SEMAPHORE \"ACCCESSIBLE\"");
	}
	if(fclose(filelog))
	{
		perror("ERROR: COULD NOT CLOSE LOGFILE");
	}

	///////////////////////////////////////////////////
	//////             ALL DONE, EXIT!           //////
	///////////////////////////////////////////////////
	return 0;
}
