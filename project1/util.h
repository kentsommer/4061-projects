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

//Defenitions for bool
#define true 1
#define false 0
typedef int bool;

// This stuff is for easy file reading
FILE * file_open(char*);
char * file_getline(char*, FILE*);
int is_file_exist(char *);
int get_file_modification_time(char *);
int compare_modification_time(char *, char *);

//Set up some defines for mallocing things
#define MAX_DEPS 10

typedef struct target Target;

struct target
{
	char* name; //Target name
    char* command; //Target command
    Target** children; //Array of children
    char** dependencies; //Array of dependencies
    int dep_num; //Number of Dependencies
};

typedef struct tree
{
    Target* root;
} Tree;


Tree* initTree(void); //Allocate space for tree
Target* initTarget(void); //Allocate space for target
Target* findTarget(char*, Tree*); //Search tree for target
Target* findTargetRec(char*, Target*); //Recursive helper for search tree
Tree* buildTree(Target**, int); //Build tree using target array
int addtoRoot(Target*, Tree*); //Add target too root of tree
int stripforme(char* dependencies); //Strips leading whitespace
char** getCmdArray(char* ); //Get array of command strings for exec'ing
int getSize(Target**); //Return Size of array
int executeMake(char*, Tree* ,bool); //Execute the tree
int executeMakeRec(Target*, bool); //Recursive helper for execute tree
bool shouldExecute(Target* target);
char** getTreeTargets(Tree*); //Return string array of targets from the tree
void getTreeTargetsRec(Target*, char**, int); //Recursive helper for string array from tree
int setDependencies(Target*, char*); //Set dependencies for a target (relation to tree)
int addConnected(Target** , Tree*); //Add connected targets to tree
int addConnectedRec(Target**, Target*); //Recursive helper for adding connected to tree
void printTargets(Target**, int); //Prints out target array (before tree build)
void removeDependency(char**, int); //Deletes a dependency from the array

#endif