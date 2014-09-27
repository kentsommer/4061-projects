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

Target* initNewTarget()
{
   Target* result = (Target *) malloc(sizeof(Target) * 1024);
   result->name = (char *) malloc(9 * sizeof(char*));
   result->command = (char *) malloc(1024);
   result->children =(Target **) malloc(9 * sizeof(Target *));
   result->dependencies = (char **) malloc(9 * sizeof(char *));
   return result;
}

Tree* initTree()
{
   Tree* tree = (Tree *) malloc(sizeof(Tree));
   Target* root = initNewTarget();
   strcpy(root->name, "root");
   return tree;
}

void setDependencies(Target* targetset, char* dep_names)
{
   int dep_count = 0;
   char* current = (char*)malloc(9 * sizeof(char));
   if(dep_names == NULL)
   {
      targetset->dependencies = NULL;
      return;
   }

   current = strtok(dep_names, " ");
   while(current != NULL)
   {
      targetset->dependencies[dep_count] = (char *)malloc(9 * sizeof(char) + 1);
      targetset->dependencies[dep_count++] = current;
      current = strtok(NULL, " ");
   }
   targetset->dep_count = dep_count;
}

//Prints the target information including runstatus and dependencies
void print_target(Target* target)
{
   printf("\n");
   int d = 0;
   printf("Testies");
   printf("Target is %s\n", target->name);
   printf("PID is: %d\n", target->pid);
   printf("Dependencies are: \n");
   for(d = 0; d < target->dep_count; d++)
   {
      printf("\t%s\n", target->dependencies[d]);
   }
   printf("Command is: %s\n", target->command);
   printf("\n");
}

Target* buildTree(Target** targetArray, int targetCount, char * mainTarget)
{
   bool addToRoot = true;
   int i = 0;
   int y = 0;
   int c = 0;
   Tree* tree = initTree();

   // if(!holdsMainTarget(targetArray, mainTarget))
   // {
   //    fprintf(stderr, "ERROR: Target specified does not exist.\n");
   //    exit(1);
   // }

   //addMainToRoot(targetArray, tree, mainTarget);

   while(addToTree(targetArray, tree) != 0)
   {
      c++;

      if(c > targetCount);
      {
         fprintf(stderr, "YO you have a cycle in the dependencies I'm not dealing with this shit. \n");
         exit(1);
      }
   }
   return tree;
}

int addToTree(Target** targetArray, Target* target)
{
   int i = 0;
   int result = 0;
   if(target->children[0]!= NULL)
   {
      while(target->children[i] != NULL)
      {
         result = result || addToTree(targetArray, target->children[i]);
         i++;
      }
   }
   else
   {
      result = addDependencies(target, targetArray);
   }
   return result;
}

int addDependencies(Target* target, Target** targetArray)
{
   int i = 0;
   int y = 0;
   int numDeps;
   bool added = false;
   bool found;
   while(target->dependencies[i] != NULL)
   {
      y = 0;
      found = false;
      while(targetArray[y] != NULL)
      {
         if(strcmp(targetArray[y]->name, target->dependencies[i]) == 0)
         {
            numDeps = sizeOfArray(target->children);
            target->children[numDeps] = targetArray[y];
            removeDependency(target->dependencies, i);
            added = true;
            found = true;
            break;
         }
         y++;
      }
      if(!found)
      {
         printf("Well isn't that shitty, looks like a dependency doesn't exist\n");
         removeDependency(target->dependencies, i);
      }
   }
   return added;
}

int sizeOfArray(Target** targetArray)
{
   int result = 0;
   while(targetArray[result] != NULL)
   {
      result++;
   }
   return result;
}

void removeDependency(char** dependencies, int index)
{
   while(dependencies[index] != NULL)
   {
      dependencies[index] = dependencies[index+1];
      index++;
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