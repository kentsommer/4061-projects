/* CSci4061 F2014 Assignment 2
* date: 10/27/14
* name: Kent Sommer, Kanad Gupta, Xi Chen
* id: somme282, kgupta, chen2806 */

#include "wrapper.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <gtk/gtk.h>

extern int errno;

#define MAX_TAB 100
#define MAX_OPEN_TAB 99
#define EXIT_STATUS_PIPE_ERROR -1

int run_browser(comm_channel* channels);
comm_channel setup_pipes();
int kill_tab(int fd);
int close_tab(comm_channel* tab, int i);
int close_tabn(comm_channel tab);
pid_t pids[100];


/*
 * Name:		uri_entered_cb
 * Input arguments:'entry'-address bar where the url was entered
 *			 'data'-auxiliary data sent along with the event
 * Output arguments:void
 * Function:	When the user hits the enter after entering the url
 *			in the address bar, 'activate' event is generated
 *			for the Widget Entry, for which 'uri_entered_cb'
 *			callback is called. Controller-tab captures this event
 *			and sends the browsing request to the router(/parent)
 *			process.
 */
void uri_entered_cb(GtkWidget* entry, gpointer data)
{
	if(data == NULL)
	{	
		return;
	}
	browser_window* b_window = (browser_window*)data;
	// Get the URL.
	char* uri = get_entered_uri(entry);
	// Get the tab index where the URL is to be rendered
	int tab_index = query_tab_id_for_request(entry, data);
	if(tab_index < 0 || tab_index >= MAX_TAB)
	{
		perror("error yo, tab is out of range");
        return;
	}
	else
	{
		//Make Request packet to send to router process
		child_req_to_parent req;
		//Fill in req.type
		req.type = NEW_URI_ENTERED;
		//Fill in which tab to render in 
		req.req.uri_req.render_in_tab = tab_index;
		//Fill in the uri 
		strcpy(req.req.uri_req.uri, uri);
		//Send through proper file descriptor 
		if(write(b_window->channel.child_to_parent_fd[1], &req, sizeof(child_req_to_parent)) == -1)
		{
			perror("Failed to write");
		}
	}
}

/*
 * Name:		new_tab_created_cb
 * Input arguments:	'button' - whose click generated this callback
 *			'data' - auxillary data passed along for handling
 *			this event.
 * Output arguments:    void
 * Function:		This is the callback function for the 'create_new_tab'
 *			event which is generated when the user clicks the '+'
 *			button in the controller-tab. The controller-tab
 *			redirects the request to the parent (/router) process
 *			which then creates a new child process for creating
 *			and managing this new tab.
 */ 
void new_tab_created_cb(GtkButton *button, gpointer data)
{
	if(data == NULL)
	{
		return;
	}
	//This channel have pipes to communicate with router. 
	comm_channel channel = ((browser_window*)data)->channel;
 	int tab_index = ((browser_window*)data)->tab_index; // gets updated on a higher level function
	if(tab_index < 0 || tab_index >= MAX_TAB)
	{
		perror("error bish, tab is out of range\n");
                return;
	}
	// Create a new request of type CREATE_TAB
	child_req_to_parent new_req;
	//Populate it with request type CREATE_TAB, and tab index
	new_req.type = CREATE_TAB;
	create_new_tab_req newtab;
	newtab.tab_index = tab_index;
	new_req.req.new_tab_req = newtab;
	// Send through proper file descriptor 
	if (write (channel.child_to_parent_fd[1], &new_req, sizeof(new_req)) == -1)
	{
		perror("Failed to write");
	}
}

/*
 * Name:                run_control
 * Input arguments:     'comm_channel': Includes pipes to communctaion with Router process
 * Output arguments:    void
 * Function:            This function will make a CONTROLLER window and be blocked until the program terminate.
 */
int run_control(comm_channel comm)
{
	browser_window * b_window = NULL;
	//Create controler process
	create_browser(CONTROLLER_TAB, 0, G_CALLBACK(new_tab_created_cb), G_CALLBACK(uri_entered_cb), &b_window, comm);
	//go into infinite loop.
	show_browser();
	return 0;
}

/*
* Name:                 run_url_browser
* Input arguments:      'nTabIndex': URL-RENDERING tab index
                        'comm_channel': Includes pipes to communctaion with Router process
* Output arguments:     void
* Function:             This function will make a URL-RENDRERING tab Note.
*                       You need to use below functions to handle tab event. 
*                       1. process_all_gtk_events();
*                       2. process_single_gtk_event();
*                       3. render_web_page_in_tab(uri, b_window);
*                       For more details please Appendix B.
*/
int run_url_browser(int nTabIndex, comm_channel comm)
{
	browser_window * b_window = NULL;
	
	//Create controler window
	create_browser(URL_RENDERING_TAB, nTabIndex, G_CALLBACK(new_tab_created_cb), G_CALLBACK(uri_entered_cb), &b_window, comm);
	while (1) //Loop until we hit a return (tab killed)
	{
		usleep(1000);

		child_req_to_parent new_req;

		if(read(comm.parent_to_child_fd[0], &new_req, sizeof(new_req)) < 0) // No data
		{
			if(errno != EAGAIN)
			{
				perror("Error processing fd");
				exit(-1);
			}
		}
		else
		{
			if(new_req.type == NEW_URI_ENTERED)
			{
				render_web_page_in_tab(new_req.req.uri_req.uri, b_window);
			}

			if(new_req.type == TAB_KILLED)
			{
				close_tabn(comm);
				return 0;
			}
		}
		process_single_gtk_event();
	}
	return 0;
}

int kill_tab(int fd)
{
	int c;
	c =close(fd);
	if(c < 0) 
	{
		perror("close error");
		printf("on fd %d\n", fd);
	}
	return c;
}

int close_tab(comm_channel* tab, int i)
{
	kill_tab(tab[i].parent_to_child_fd[0]);
	kill_tab(tab[i].parent_to_child_fd[1]);
	kill_tab(tab[i].child_to_parent_fd[0]);
	kill_tab(tab[i].child_to_parent_fd[1]);
	return 0;
}

int close_tabn(comm_channel tab)
{
	kill_tab(tab.parent_to_child_fd[0]);
	kill_tab(tab.parent_to_child_fd[1]);
	kill_tab(tab.child_to_parent_fd[0]);
	kill_tab(tab.child_to_parent_fd[1]);
	return 0;
}

comm_channel setup_pipes()
{
	comm_channel channel;
	int i;
	int fileControl;
	//Open pipe
	if(pipe(channel.parent_to_child_fd) < 0)
	{
		perror("Failed to create parent to child pipe");
		exit(-1);
	}
	//Open pipe
	if(pipe(channel.child_to_parent_fd) < 0)
	{
		perror("Failed to create child to parent pipe");
		exit(-1);
	}	

	// Set flags
	for(i = 0; i < 1; i++)
	{
		fileControl = fcntl(channel.parent_to_child_fd[i], F_GETFL, 0);

		if(fileControl == -1)
		{
			perror("Failed to get file flags");
			exit(-1);
		}

		fileControl |= O_NONBLOCK;
		if(fcntl(channel.parent_to_child_fd[i], F_SETFL, fileControl) == -1)
		{
			perror("Failed to set file flags");
			exit(-1);
		}
	}
	// Set flags
	for(i = 0; i < 1; i++)
	{
		fileControl = fcntl(channel.child_to_parent_fd[i], F_GETFL, 0);

		if(fileControl == -1)
		{
			perror("Failed to get file flags");
			exit(-1);
		}

		fileControl |= O_NONBLOCK;
		if(fcntl(channel.child_to_parent_fd[i], F_SETFL, fileControl) == -1)
		{
			perror("Failed to set file flags");
			exit(-1);
		}
	}
	return channel;
}

int run_browser(comm_channel* channels)
{
	int i;
	int opentabs = 0;
	comm_channel pipes = setup_pipes();

	for(i=0; i <100; i++)
	{
		pids[i] = 0;
	}

	int childprocesses= 0;
	childprocesses = childprocesses + 1;
	pid_t controlPid = fork();

	if(controlPid < 0)
	{
		perror("Failed to fork from controller tab");
		return 1;
	}

	/*Run the CONTROLLER*/
	if(controlPid == 0)
	{
		run_control(pipes);
		exit(1);
	}

	else
	{
		while(childprocesses > 0)
		{
			usleep(1000);
			child_req_to_parent new_req;

			/*Read from controler*/
			if(read(pipes.child_to_parent_fd[0], &new_req, sizeof(new_req)) < 0)
			{
				if(errno != EAGAIN)
				{
					perror("Error reading from pipe");
					exit(-1);
				}
			}
			else 
			{
				if(new_req.type == CREATE_TAB)
				{
					if (opentabs < MAX_OPEN_TAB) //1 controller and 99 tabs 
					{
						childprocesses = childprocesses +1;
						opentabs = opentabs + 1;
						/*Set up pipes for communication and polling*/
						comm_channel urlPipes = setup_pipes();
						int tabtoopen =-1;
						for(i = 1; i <= opentabs; i++) // 
						{
							if(pids[i] == 0)
								{
									tabtoopen = i;
									break;
								}
						}
						if(tabtoopen == -1)
						{
							perror("Problem creating tab");
							exit(-1);
						}
						channels[tabtoopen] = urlPipes;
						/*Fork a process for the new tab*/
						pids[tabtoopen] = fork();
						if(pids[tabtoopen] < 0)
						{
							perror("Failure to fork for CREATE_TAB");
							exit(-1);
						}
						/*Tab process*/
						if(pids[tabtoopen] == 0)
						{	
							run_url_browser(tabtoopen,channels[tabtoopen]);
							exit(0);
						}
					}
					else
					{
						printf("Too many tabs open please close one\n");
					}
				}
				else if(new_req.type == NEW_URI_ENTERED)
				{
					int w;
					//Verify tab is not invalid
					if(new_req.req.uri_req.render_in_tab <= 0 || new_req.req.uri_req.render_in_tab > MAX_OPEN_TAB || pids[new_req.req.uri_req.render_in_tab] == 0)
					{
						printf("Invalid tab\n");
					}
					else
					{
						w = write(channels[new_req.req.uri_req.render_in_tab].parent_to_child_fd[1],&new_req, sizeof(new_req)) ;
						if(w < 0)
						{
							perror("problem writing uri to child");
						}
					}
				}

				else if(new_req.type == TAB_KILLED)
				{
					childprocesses = childprocesses -1; //One less child, killed one
					close_tabn(pipes);
					for(i = 1; i <= MAX_OPEN_TAB ; i++)
					{
						child_req_to_parent new_req;
						new_req.type = TAB_KILLED;
						new_req.req.killed_req.tab_index = i;
						if(pids[i] != 0) //if it hasn't been closed yet 
						{
							opentabs = opentabs -1;
							int w;
							w =write(channels[i].parent_to_child_fd[1], &new_req, sizeof(new_req));
							if (w < 0)
						    {
									perror("error writting tab kill");
						    }
						    //Close file descriptors and pipes 
							close_tab(channels, i);
							childprocesses = childprocesses -1;
							int status;
							pid_t change = waitpid(pids[i], &status, 0);
							if(change != pids[i])
							{
								perror("error, wait:");
							}
							pids[i] = 0;
						}
					}
				}
			}


			//read from tabs 
			for(i=1 ; i <= opentabs; i++)
			{
				if(pids[i] != 0)
				{
					if(read(channels[i].child_to_parent_fd[0], &new_req, sizeof(new_req)) < 0)
					{
						if(errno != EAGAIN)
						{
							perror("error");
							exit(-1);
						}
					}
					else
					{
						if(new_req.type == TAB_KILLED)
						{
							//Set req type, index, close tab 
							child_req_to_parent new_req;
							new_req.type = TAB_KILLED;
							tab_killed_req killReq;
							killReq.tab_index = i;
							new_req.req.killed_req = killReq;
							opentabs = opentabs -1;
							if (write(channels[i].parent_to_child_fd[1], &new_req, sizeof(new_req)) < 0)
							{
								perror("failed writing tab killed");
							}
							//Close out tab
							close_tab(channels, i);
							childprocesses = childprocesses -1;
							int status;
							pid_t change = waitpid(pids[i], &status, 0);
							if(change != pids[i])
							{
								perror("error, wait:");
							}
							pids[i] = 0; //Reset the pid
						}
					}
				}
			}
		}	
	}
	return 0;
}

int main()
{
	//Allocate memory
	comm_channel* channels = (comm_channel*)calloc(MAX_TAB, sizeof(comm_channel));
	// starts router and controller processes. Polls, and returns once all windows including controller are closed
	run_browser(channels);
	//Free the allocated memory
	free(channels);
	return 0;
}