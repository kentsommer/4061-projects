#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <unistd.h>

#include "util.h"


//FLAGS 
int printcmds = 0; //This is global flag for print commands only (if -n)

//Array holder
struct target targets[MAX_TARGETS];
int targetnum=0;
int cmd_index=0;

//This is a test comment
//This function will parse makefile input from user or default makeFile. 
int parse(char * lpszFileName)
{
	int nLine=0;
	char szLine[1024];
	char * lpszLine;
	char * lpszLinec; 
	char * cmdLine;
	char * tofree;
	char * token;
	char * dependencies;
	int chopnum = 0;
	FILE * fp = file_open(lpszFileName);
	FILE * deps = fopen("deps", "a"); 
	FILE * targs = fopen("targs", "a");
	
	//String tokin (tokin get it??)
	char * fstarget;

	if(fp == NULL)
	{
		return -1;
	}

	if(printcmds == 1)
	{
		fprintf(stderr, "We only want to print commands, not run \n");
	}

	while(file_getline(szLine, fp) != NULL) 
	{
		nLine++;

		struct target *current = malloc(sizeof(struct target) * 1024);
		 //This will be used to fill current
				// Target information and save to some list/array
        
        if (strcmp(szLine, "\n") == 0) 
        {
            //printf("empty line\n");
            continue;
        }
		//Remove newline character at end if there is one
		lpszLine = strtok(szLine, "\n");

		//check if it is a command line and add to the proper target
		char key[] = {'\t'};
        if (strpbrk (lpszLine, key) != NULL)
        {
        	cmdLine = (char *) malloc(1024);
        	strcpy(cmdLine, lpszLine);
        	cmdLine = cmdLine + 1; //Remove tab char
        	targets[targetnum-1].commands[cmd_index] = cmdLine;
        	cmd_index++;
        	targets[targetnum-1].numcmd = cmd_index;
        }
        
		lpszLinec = (char *) malloc(1024);
		//Make a copy of the string and remove anything before token ":"
		strcpy(lpszLinec, lpszLine);
		fstarget = strtok(lpszLinec, ":");
		
		//Compare original to target, if equal, line is not a target line. 
		if (strlen(lpszLine) != strlen(fstarget)) 
		{
			cmd_index = 0;
			current->name = fstarget;
			current->linenum = nLine;
			dependencies = (char *) malloc(1024); //REMOVE THIS SHIT LATER!!!!!!! :(
			strcpy(dependencies, lpszLine);
			chopnum = strlen(fstarget) + 2; 
			dependencies += chopnum; //remove target name (point addition because baller)
			//fprintf(stderr, "Dependencies are: \"%s\"\n", dependencies);
			
			tofree = (char *) malloc(1024); //REMOVE ME WHYYYYYY
			strcpy(tofree, dependencies);
			int dep_index = 0; 
			while((token = strsep(&dependencies, " ")) != NULL)
			{
				if(strcmp(token, "") != 0)
				{
					current->deps[dep_index] = token;
					dep_index++;
				}
			}
			current->numchild = dep_index;
			targets[targetnum] = *current;
			targetnum++;
		}

		//You need to check below for parsing. 
		//Skip if blank or comment.
		//Remove leading whitespace.
		//Skip if whitespace-only.
		//Only single command is allowed.
		//If you found any syntax error, stop parsing. 
		//If lpszLine starts with '\t' it will be command else it will be target.
		//It is possbile that target may not have a command as you can see from the example on project write-up. (target:all)
		//You can use any data structure (array, linked list ...) as you want to build a graph
	}

	//Close the makefile and deps and targs file
	fclose(fp);
	fclose(deps);
	fclose(targs);

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
	//parse(szMakefile); 

	while((ch = getopt(argc, argv, format)) != -1) 
	{
		switch(ch) 
		{
			case 'f':
				strcpy(szMakefile, strdup(optarg));
				break;
			case 'n':
				printcmds = 1;
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

	fprintf(stderr, "File is called: %s \n", szMakefile);

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


	/* Parse graph file or die */
	if((parse(szMakefile)) == -1) 
	{
		return EXIT_FAILURE;
	}
	//printf("Number of targets %d\n", targetnum);

	int i = 0;
	while(i < targetnum)
	{
		print_target(&targets[i]);
		 i++;
	}

	printf("Is ready: %d\n", isReady(targets, targetnum));

	i = 0;
	while(i < targetnum)
	{
		print_target(&targets[i]);
		 i++;
	}
	//after parsing the file, you'll want to check all dependencies (whether they are available targets or files)
	//then execute all of the targets that were specified on the command line, along with their dependencies, etc.
	return EXIT_SUCCESS;
}
