/********************
 * util.h
 *
 * You may put your utility function definitions here
 * also your structs, if you create any
 *********************/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

// the following ifdef/def pair prevents us from having problems if 
// we've included util.h in multiple places... it's a handy trick
#ifndef _UTIL_H_
#define _UTIL_H_

//You can change any value in this file as you want. 
#define INELIGIBLE 0
#define READY 1
#define RUNNING 2
#define FINISHED 3

#define MAX_LENGTH 1024
#define MAX_DEPENDENCIES 10
#define MAX_TARGETS 10
#define MAX_PARENTS 10
#define MAX_CHILDREN 10

 //Use booleans
typedef enum { false, true } bool;

// This stuff is for easy file reading
FILE * file_open(char*);
char * file_getline(char*, FILE*);
int is_file_exist(char *);
bool isTarget(char * lpszLine);
int get_file_modification_time(char *);
int compare_modification_time(char *, char *);
int makeargv(const char *s, const char *delimiters, char ***argvp);
void freemakeargv(char **argv);

//You will need to fill this struct out to make a graph.
typedef struct target{
	int status; //Status (running, waiting, ready etc)
	int linenum; //Line number of target (can be pulled from nLine to be regrabbed)
	char * children[MAX_CHILDREN]; //Children line numbers (max of 10) (dependencies)
	char * parents[MAX_PARENTS]; //Parents line numbers (max of 10)
	pid_t pid; 
	int numparent; //Number of parent targets
	int numchild; //Number of child targets
	bool hasDeps; //
}target_t;

#endif
