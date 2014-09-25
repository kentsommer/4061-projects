/************************
 * util.c
 *
 * utility functions
 *
 ************************/

#include "util.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//Vars
char * lpszLinec; 
char * fstarget;

/***************
 * These functions are just some handy file functions.
 * We have not yet covered opening and reading from files in C,
 * so we're saving you the pain of dealing with it, for now.
 *******/
FILE * file_open(char* filename) 
{
	FILE* fp = fopen(filename, "r");
	if(fp == NULL) 
	{
		fprintf(stderr, "make4061: %s: No such file or directory.\n", filename);
		exit(1);
	}

	return fp;
}

//This function will return the line.
char* file_getline(char* buffer, FILE* fp) 
{
	buffer = fgets(buffer, 1024, fp);
	return buffer;
}

//Return -1 if file does not exist
int is_file_exist(char * lpszFileName)
{
	return access(lpszFileName, F_OK); 
}

//return -1 if file does not exist. 
//return last modified time of file
//bigger number means that it is newer (more recently modified). 
int get_file_modification_time(char * lpszFileName)
{
	if(is_file_exist(lpszFileName) != -1)
	{
		struct stat buf;
		int nStat = stat(lpszFileName, &buf);
		return buf.st_mtime;
	}
	
	return -1;
}

//Prints the target information including runstatus and dependencies
void print_target(target_t * target)
{
   int p, c;
   printf("ID is: %d\n", target->linenum);
   printf("Status is: %d\n", target->status);
   printf("PID is: %d\n", target->pid);
   printf("Children are: \n");
   for(c = 0; c < target->numchild; c++)
   {
      printf("%s\n", target->deps[c]);
   }
}

bool isTarget(char * lpszLine)
{
   lpszLinec = (char *) malloc(1024);
   //Make a copy of the string and remove anything before token ":"
   strcpy(lpszLinec, lpszLine);
   fstarget = strtok(lpszLinec, ":");
   if (strlen(lpszLine) != strlen(fstarget))
   {
      free(lpszLinec);
      return true; 
   }
   free(lpszLinec);
   return false;
}
//return 0, uncompilable
//return 1, compiable 
bool isCompilable(struct target * targetToCheck)
{
   if(targetToCheck->status==1)
   {
    return false;
   }
   if(targetToCheck->status==0)
   {
        if (areDependenciesCompiled(targetToCheck->deps)==1)
        {
            return true;
        }
   }

   return false;
 }

//return 0 if not all dependencies are  all compiled or lost file
//return 1 if all dependencies are compiled and file exist
 int areDependenciesCompiled(char * children[])
 {
      for (int i = 0; i < sizeof(children)/sizeof(char); ++i)
      {
       }

 }


 bool hasDeps(struct target targetsArray[], int size)
 {
   int i =0;
   while(i < size)
   {
      struct target current = targetsArray[i];
      int y = 0;
      while(y < targetsArray[i].numchild)
      {
         int z = 0;
         while(z < size)
         {
            if(strcmp(targetsArray[z].name, targetsArray[i].deps[y]) == 0)
            {
               return true;
            }
            z++;
         }
         y++;
      }
      i++;
   }
   return false;
 }


//Compare the last modified time between two files.
//return -1, if any one of file does not exist. 
//return 0, if both modified time is the same.
//return 1, if first parameter is bigger (more recent)
//return 2, if second parameter is bigger (more recent)
int compare_modification_time(char * lpsz1, char * lpsz2)
{	
	int nTime1 = get_file_modification_time(lpsz1);
	int nTime2 = get_file_modification_time(lpsz2);

	if(nTime1 == -1 || nTime2 == -1)
	{
		return -1;
	}

	if(nTime1 == nTime2)
	{
		return 0;
	}
	else if(nTime1 > nTime2)
	{
		return 1;
	}
	else
	{
		return 2;
	}
}

// makeargv
/* Taken from Unix Systems Programming, Robbins & Robbins, p37 */
int makeargv(const char *s, const char *delimiters, char ***argvp) {
   int error;
   int i;
   int numtokens;
   const char *snew;
   char *t;

   if ((s == NULL) || (delimiters == NULL) || (argvp == NULL)) {
      errno = EINVAL;
      return -1;
   }
   *argvp = NULL;
   snew = s + strspn(s, delimiters);
   if ((t = malloc(strlen(snew) + 1)) == NULL)
      return -1;
   strcpy(t,snew);
   numtokens = 0;
   if (strtok(t, delimiters) != NULL)
      for (numtokens = 1; strtok(NULL, delimiters) != NULL; numtokens++) ;

   if ((*argvp = malloc((numtokens + 1)*sizeof(char *))) == NULL) {
      error = errno;
      free(t);
      errno = error;
      return -1;
   }

   if (numtokens == 0)
      free(t);
   else {
      strcpy(t,snew);
      **argvp = strtok(t,delimiters);
      for (i=1; i<numtokens; i++)
         *((*argvp) +i) = strtok(NULL,delimiters);
   }

   *((*argvp) + numtokens) = NULL;
   return numtokens;
}

//You should call this function when you done with makeargv()
void freemakeargv(char **argv) {
   if (argv == NULL)
      return;
   if (*argv != NULL)
      free(*argv);
   free(argv);
}
