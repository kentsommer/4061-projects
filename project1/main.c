#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <unistd.h>

#include "util.h"

//Array holder for targets
Target** targetArray;
int targetCount = 0;

//This function will parse makefile input from user or default makeFile. 
int parse(char * lpszFileName)
{
	int nLine=0;
	char szLine[1024];
	char* lpszLine;
	char* linecopy;
	FILE * fp = file_open(lpszFileName);

	//Added struct vars and ints
	char* dep_names;
	Target* current = NULL;

	if(fp == NULL)
	{
		return -1;
	}

	while(file_getline(szLine, fp) != NULL) 
	{
		linecopy = (char*)malloc(1024);
		nLine++;

		lpszLine = strtok(szLine, "\n"); //Remove newline character at end if there is one
		//Line is target line
		char colon[] = {':'};
		if(strpbrk (lpszLine, colon) != NULL)
		{
			printf("Setting a target\n");
			current = initNewTarget(); //Malloc the struct
			strcpy(linecopy, lpszLine); //Make line copy
			printf("Finished string copy\n");
			current->name = strtok(linecopy, ":"); //Set targetname
			printf("Name was set to: %s\n", current->name);
			dep_names = strtok(NULL, ":"); //Get string of dependencies
			setDependencies(current, dep_names); //Set current's dependencies
			targetCount++;
			continue;
		}
		//Line is command line
		char tab[] = {'\t'};
        if (strpbrk (lpszLine, tab) != NULL)
        {
        	printf("Setting a command\n";)
        	lpszLine = lpszLine + 1; //Remove tab char
        	current->command = lpszLine;
        	targetArray[targetCount] = (Target *)malloc(sizeof(Target));
      		targetArray[targetCount++] = current;
      		current = NULL;
  			continue;
        }
	}

	//Close the makefile. 
	fclose(fp);

	return 0;
}

void show_error_message(char * lpszFileName)
{
	fprintf(stderr, "Usage: %s [options] [target] : only single target is allowed.\n", lpszFileName);
	fprintf(stderr, "-f FILE\t\tRead FILE as a maumfile.\n");
	fprintf(stderr, "-h\t\tPrint this message and exit.\n");
	fprintf(stderr, "-n\t\tDon't actually execute commands, just print them.\n");
	fprintf(stderr, "-B\t\tDon't check files timestamps.\n");
	fprintf(stderr, "-m FILE\t\tRedirect the output to the file specified .\n");
	exit(0);
}

int main(int argc, char **argv) 
{
	// Declarations for getopt
	extern int optind;
	extern char * optarg;
	int ch;
	char * format = "f:hnBm:";
	
	// Default makefile name will be Makefile
	char szMakefile[64] = "Makefile";
	char szTarget[64];
	char szLog[64];

	while((ch = getopt(argc, argv, format)) != -1) 
	{
		switch(ch) 
		{
			case 'f':
				strcpy(szMakefile, strdup(optarg));
				break;
			case 'n':
				break;
			case 'B':
				break;
			case 'm':
				strcpy(szLog, strdup(optarg));
				break;
			case 'h':
			default:
				show_error_message(argv[0]);
				exit(1);
		}
	}

	argc -= optind;
	argv += optind;

	// at this point, what is left in argv is the targets that were 
	// specified on the command line. argc has the number of them.
	// If getopt is still really confusing,
	// try printing out what's in argv right here, then just running 
	// with various command-line arguments.

	if(argc > 1)
	{
		show_error_message(argv[0]);
		return EXIT_FAILURE;
	}

	//You may start your program by setting the target that make4061 should build.
	//if target is not set, set it to default (first target from makefile)
	if(argc == 1)
	{
	}
	else
	{
	}

	printf("Starting parse\n");
	/* Parse graph file or die */
	if((parse(szMakefile)) == -1) 
	{
		return EXIT_FAILURE;
	}
	printf("Finished parse\n"); 

	printf("Starting Print\n")
	//Test print to see what we have;
	int i = 0;
	while(i < targetCount)
	{
		print_target(targetArray[i]);
		i++;
	}

	//after parsing the file, you'll want to check all dependencies (whether they are available targets or files)
	//then execute all of the targets that were specified on the command line, along with their dependencies, etc.
	return EXIT_SUCCESS;
}
