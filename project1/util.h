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
#define SKIP 4

#define MAX_LENGTH 1024
#define MAX_DEPENDENCIES 10
#define MAX_TARGETS 10
#define MAX_COMMANDS 10

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
int areDependenciesCompiled(char * children[]);

//You will need to fill this struct out to make a graph.
typedef struct target{
	char * name;
	int status; //Status (uncompiled 0, compiled 1 etc)
	int linenum; //Line number of target (can be pulled from nLine to be regrabbed)
	char * deps[MAX_DEPENDENCIES]; //Children line numbers (max of 10) (dependencies)
	pid_t pid; 
	int numparent; //Number of parent targets
	int numchild; //Number of child targets
	int numcmd;
	bool hasDeps; //
	char * commands[MAX_COMMANDS];
}target_t;

bool isReady(struct target targetsArray[], int size);
bool isIndependent(struct target target, struct target targetcheck);
void print_target(struct target * target);

#endif
