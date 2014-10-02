#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

//RESULT FOR SEARCH
Target* result = NULL;

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
//return true if empty dependencies
//return false if it has dependencies
bool stripforme(char* dependencies)
{
  char* dep_copy; 
  printf("Stuff\n");
  strcpy(dep_copy, dependencies);
  printf("Finished copy\n");
	// while(isspace(*dep_copy))
	// {
 //    printf("Dep is now: %s\n", dep_copy);
	// 	dep_copy++;
	// }
  if(dep_copy != NULL)
  {
    if(isspace(dep_copy[0]))
    {
      printf("%s\n", dep_copy[0]);
      dep_copy = dep_copy + 1;
    }
    printf("1");
    if(dep_copy == NULL)
    {
      return true;
    }
  }
  else return false;
}

int addDependency(Target* target, Target** list)
{
  int i=0,j=0;
  int childSize;
  int y = target->dep_num;
  bool isAdd = false;
  bool isFound;
  while(target->dependencies[i] != NULL)
  {
    j = 0;
    isFound = false;
    while(list[j] != NULL && list[j]->name != NULL && target != NULL && target->dependencies[i] != NULL)
    {
      if(strcmp(list[j]->name, target->dependencies[i]) == 0)
      {
        childSize = getSize(target->children);
        target->children[childSize] = list[j];
        removeDependency(target->dependencies,i);
        isAdd = true;
        isFound = true;
      }
      j++;
    }

    if(!isFound)
    {
      removeDependency(target->dependencies,i);
    }
    
  }
  return isAdd;
}

int addtoRoot(Target* target,Tree* tree)
{
  int i = 0;
  while(tree->root->children[i] != NULL && strcmp(tree->root->children[i]->name, "") != 0)
  {
    i++;
  }
  tree->root->children[i] = target;
  return 1;
}

char** getCmdArray(char* str)
{
  int i = 0;
  char** array = (char**)malloc(30 * sizeof(char*));
  char* element = strtok(str," ");
  while(element != NULL)
  {
    array[i++] = element;
    element = strtok(NULL," ");
  } 
  return array;
}

int getSize(Target** array)
{
  int size = 0;
  while(array[size] != NULL && strcmp(array[size]->name, " ") != 0)
  {
    size++;
  }
  return size;
}

int executeMake(char* rootName, Tree* tree, bool execute)
{
  Target* subroot;
  if(strcmp(rootName, "root") == 0)
  {
    executeMakeRec(tree->root->children[0],execute);
  }
  else
  {
    subroot = findTarget(rootName,tree);
    if(subroot == NULL)
    {
      printf("Yo... That target doesn't exist. Stopping\n");
      exit(1);
    }
    executeMakeRec(subroot,execute);
  }
  return 1;
}

int executeMakeRec(Target* target, bool execute)
{
  int size = getSize(target->children);;
  int i;
  pid_t childpid;
  if(target->children != NULL)
  {
    for(i=0;i<size;i++)
    {
      executeMakeRec(target->children[i],execute);
    }
  }

  if(target->execute == false)
  {
    return 1;
  }

  if(strcmp(target->command, "echo") == 0)
  {
    return 1;
  }

  if(execute && target->command != NULL)
  {
    printf("%s\n",target->command);
    char** str = getCmdArray(target->command);
    childpid = fork();
    if (childpid == -1) 
    {
      perror("Failed to fork");
      exit(1);
    }
    if(childpid == 0)
    {
      execvp(target->command, str);
    }
    if (childpid != wait(NULL)) 
    {     
      perror("Warning, parent did not wait because");
      exit(1);
    }
    i++;
  }
  else
  {
    printf("%s\n",target->command);
  }
  return 1;  
}


int updateCheck(Target** targetArray, int targetCount)
{
  int i = 0;
  while(i < targetCount)
  {
    shouldExecute(targetArray[i]);
    i++;
  }
  return 0;
}

int shouldExecute(Target* target)
{
  int size = target->dep_num;
  int i = 0;
  if(is_file_exist(target->name) == -1)
  {
    //printf("Returning true (no file) for %s\n", target->name);
    target->execute = true;
    return 0;
  }
  while(i < size)
  {
    if(compare_modification_time(target->name, target->dependencies[i]) == 2)
    {
      //printf("Returning true (updated deps) for %s\n", target->name);
      target->execute = true;
      return 0;
    }
    i++;
  }
  //printf("Returning false for %s\n", target->name);
  target->execute = false;
  return 0;
}

char** getTreeTargets(Tree* tree)
{
  int count = 0;
  char** Namelist = (char**)malloc(10 * sizeof(char*));
  getTreeTargetsRec(tree->root, Namelist, count);
  return Namelist;
}

void getTreeTargetsRec(Target* target, char** Namelist, int count)
{
  int size = getSize(target->children);
  int i;

  Namelist[count] = (char*) malloc(10 * sizeof(char));
  strcpy(Namelist[count],target->name);

  for(i=0;i<size;i++)
  {
    if(target->children[i] != NULL && strcmp(target->children[i]->name, "") != 0)
    {
      getTreeTargetsRec(target->children[i], Namelist, count++);
    }
  }
}

int setDependencies(Target* newtarget, char* dependencies)
{
  int targetCount = 0;
  char* element = (char*) malloc(10 * sizeof(char));
  if(dependencies == NULL)
  {
    newtarget->dependencies = NULL;
    return;
  }

  element = strtok(dependencies," ");
  //printf("Current element is: \"%s\"\n", element);
  while(element != NULL)
  {
    newtarget->dependencies[targetCount] = (char *)malloc(MAX_DEPS*sizeof(char) + 1);
    newtarget->dependencies[targetCount++] = element;
    element = strtok(NULL," ");
  }
  newtarget->dep_num = targetCount;
  return 0;
}

Target* initTarget()
{
  Target* target = (Target *)malloc(sizeof(Target));
  target->name = (char *)malloc(10 * sizeof(char));
  target->command = (char *)malloc(10 * sizeof(char*));
  target->children = (Target **)malloc(MAX_DEPS * sizeof(Target *));
  target->dependencies = (char **)malloc(MAX_DEPS * sizeof(char *));
  return target;
}


Tree* initTree(void)
{
  Tree* tree = (Tree *)malloc(sizeof(Tree));
  Target* root = initTarget();
  strcpy(root->name, "root");
  tree->root = root;
  return tree;
}

int addConnected(Target** list, Tree* tree)
{
  return addConnectedRec(list,tree->root);
}

int addConnectedRec(Target** list, Target* target)
{
  if(target != NULL && target->name != NULL)
  {
  	//printf("Current addConected is: %s\n", target->name);
  }
  int i = 0,result = 0;
  if(target->children[0] != NULL && strcmp(target->children[i]->name, "") != 0)
  {
    while(target->children[i] != NULL)
    {
      result = result || addConnectedRec(list, target->children[i]);
      i++;
    }
  }
  else
  {
    if(target->dep_num != 0 && strcmp(target->name, " ") != 0)
    {
      result = addDependency(target,list);
    }
  }
  return result;

}

void printTargets(Target** nodelist, int targetCount)
{
  int i = 0;
  int j = 0;
  int k = 0;
  while(i < targetCount)
  {
    k = 0;
    j = 0;
    printf("\n");
    printf("nodeName is: %s\n",nodelist[i]->name);
    printf("Command is: %s\n",nodelist[i]->command);
    while(k < nodelist[i]->dep_num)
    {
      printf("dependencies is: %s\n",nodelist[i]->dependencies[k]);
      k++;
    }
    i++;
    printf("\n");
  }
}

void removeDependency(char** list, int start)
{
  while(list[start] != NULL)
  {
    list[start] = list[start + 1];
    start++;
  }
}

Target* findTarget(char* name, Tree* tree)
{
      return findTargetRec(name, tree->root);
}

Target* findTargetRec(char* name, Target* target)
{
  int i;
  int size = getSize(target->children);
  // printf("Current is: \"%s\"\n", target->name);
  // printf("Current has %d deps\n", size);
  // printf("Name    is: \"%s\"\n", name);
  for(i = 0; i< size; i++)
  {
    result = findTargetRec(name, target->children[i]);
  }

  if(strcmp(name, target->name) == 0)
  {
    result = target;
  }

  if(result != NULL)
  {
    return result;
  }
  else
  {
    return NULL;
  }

}


Tree* buildTree(Target** list,int nodeSum)
{
  bool isAddToRoot = true;
  int i,j,roundCount = 0;
  Tree* tree = initTree();

  addtoRoot(list[0],tree);

  while(addConnected(list, tree) != 0)
  {
    roundCount++;

    if(roundCount > nodeSum)
    {
      fprintf(stderr, "There is cycle in dependencies, what are you trying to pull mister.\n");
      exit(1);
    }
  }

  char** allNodeName = getTreeTargets(tree);
  int x = 0;
  while(allNodeName[x] != NULL)
  {
  	x++;
  }

  for(i = 0; i< nodeSum; i++)
  {
   j = 0;
    while(allNodeName[j] != NULL)
    {
      if(strcmp(allNodeName[j], list[i]->name) == 0)
      {
        isAddToRoot = false;
        break;
      }
      j++;
    }
    if(isAddToRoot)
    {
      addtoRoot(list[i],tree);
    }
    isAddToRoot = true;
    j = 0;
  }
  return tree;
}