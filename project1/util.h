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

// This stuff is for easy file reading
FILE * file_open(char*);
char * file_getline(char*, FILE*);
int is_file_exist(char *);
int get_file_modification_time(char *);
int compare_modification_time(char *, char *);
int makeargv(const char *s, const char *delimiters, char ***argvp);
void freemakeargv(char **argv);

//You will need to fill this struct out to make a graph.
typedef struct target Target;

struct target{
    // name, pid, command, dependencies (array of pointers to other structs), dependencyArray, parent
    char * name;
    int pid;
    char * command; // gcc, etc.
    Target** children; // dependencies as nodes for the tree
    char** dependencies; // dependencies as a 1D array list
    int dep_count;
    Target *parent;

};

void print_target(Target* target);
void setDependencies(Target* targetset, char* dep_names);
Target* initNewTarget();

#endif
