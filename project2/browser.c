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

#define ERR_PRFX "ERROR | PROC %6d, LINE %4d"
#define ERR_SUFX "\t%s\n"
#define MSG_PRFX "messg | PROC %6d, LINE %4d"
#define WRN_PRFX "warng | PROC %6d, LINE %4d"

#define EXIT_STATUS_PIPE_ERROR -1

int setup_process(comm_channel* channels, int tab_index);
int poll_for_children(comm_channel* channels, int total_tabs, int max_tab_count);
int kill_tab(comm_channel* channels, int tab_index);

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
		printf("error yo, tab is out of range\n");
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
			fprintf(stderr, ERR_PRFX " --Failed to write to controller channel.child_to_parent_fd[1]==%d\n" ERR_SUFX, getpid(), __LINE__, b_window->channel.child_to_parent_fd[1], strerror(errno));
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
	// KANAD WORK ON THIS HAHA!!
	if(data == NULL)
	{
		return;
	}
	
	//This channel have pipes to communicate with router. 
	comm_channel channel = ((browser_window*)data)->channel;

 	int tab_index = ((browser_window*)data)->tab_index; // gets updated on a higher level function

	if(tab_index < 0 || tab_index >= MAX_TAB)
	{
		printf("error bish, tab is out of range\n");
                return;
	}

	// Create a new request of type CREATE_TAB
	child_req_to_parent new_req;
	
	//Populate it with request type, CREATE_TAB, and tab index
	new_req.type = CREATE_TAB;
	new_req.req.new_tab_req.tab_index = tab_index;

	// Send through proper file descriptor 
	if (write (channel.child_to_parent_fd[1], &new_req, sizeof(child_req_to_parent)) == -1)
	{
		fprintf(stderr, ERR_PRFX "  -- Failure to write to controller's channel.child_to_parent_fd[1]==%d: \n" ERR_SUFX, getpid(), __LINE__, channel.child_to_parent_fd[1], strerror(errno));
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

	child_req_to_parent msg;
	size_t read_return;
	
	while (1) 
	{
		usleep(1000);

		read_return = read(comm.parent_to_child_fd[0], &msg, sizeof(child_req_to_parent));

		if(read_return == -1 && errno == EAGAIN) // No data
		{
			process_single_gtk_event();
		}
		else
		{
			switch(msg.type)
			{
				case NEW_URI_ENTERED:
					render_web_page_in_tab(msg.req.uri_req.uri, b_window);
					break;

				case TAB_KILLED:
					fprintf(stderr, MSG_PRFX " -- Tab %d is closed\n", getpid(), __LINE__, msg.req.killed_req.tab_index);
					process_all_gtk_events();
				case CREATE_TAB:
					//Don't do anything, tabs don't handle this
				default:
					break;
			}
		}
	}

	return 0;
}

int kill_tab(comm_channel* channels, int tab_index)
{
	return 0; //LOL THIS DOESNT WORK;
}

int poll_for_children(comm_channel* channels, int total_tabs, int max_tab_count)
{
	//This needs to do stuff.
	bool controller_open = true;
	while(controller_open)
	{
		usleep(1000);

		int i;
		int closing_tab_itr;
		size_t read_return;

		child_req_to_parent msg;

		for (i = 0; i < total_tabs; i++)
		{
			if(channels[i].open)
			{
				read_return = read(channels[i].child_to_parent_fd[0], &msg, sizeof(child_req_to_parent));
				if(read_return == -1)
				{
					if(errno != EAGAIN)
					{
						fprintf(stderr, ERR_PRFX "  -- Failure to read tab %d's child_to_parent_fd[0]==%d\n" ERR_SUFX, getpid(), __LINE__, i, channels[i].child_to_parent_fd[0], strerror(errno));
						exit(EXIT_STATUS_PIPE_ERROR);
					}
				}
			}
			else
			{
				switch(msg.type)
				{
					case CREATE_TAB:
						if(total_tabs < MAX_TAB)
						{
							setup_process(channels, total_tabs);
							total_tabs++;
						}
						else
						{
							fprintf(stderr, WRN_PRFX "  -- Could not open tab %d - tab is out of range.\n", getpid(), __LINE__, max_tab_count);
						}
						break;

					case NEW_URI_ENTERED:
						if(channels[msg.req.uri_req.render_in_tab].open)
						{
							write(channels[msg.req.uri_req.render_in_tab].parent_to_child_fd[1], &msg, sizeof(child_req_to_parent));
						}
						else
						{
							fprintf(stderr, WRN_PRFX "  -- Could not open uri %s in tab %d - tab is not open.\n", getpid(), __LINE__, msg.req.uri_req.uri, msg.req.uri_req.render_in_tab);						
						}
						break;

					case TAB_KILLED:
						if(msg.req.killed_req.tab_index > CONTROLLER_TAB)
						{
							kill_tab(channels, msg.req.killed_req.tab_index);
						}
						else //Controller tab
						{
							//Close all remaining open tabs
							for(closing_tab_itr = 1; closing_tab_itr < total_tabs; closing_tab_itr++)
							{
								if(channels[closing_tab_itr].open)
								{
									kill_tab(channels, closing_tab_itr);
								}
							}
							controller_open = false;
							kill_tab(channels, CONTROLLER_TAB);
						}
						break;

					default:
						fprintf(stderr, WRN_PRFX "  -- Um, what is this type yo, you broke it.\n", getpid(), __LINE__);
				}
			}
		}
	}
	return 0;
}

int setup_process(comm_channel* channels, int tab_index)
{
	// Create bi-directional pipes (hence 2 pipes) for 
	// communication between the parent and child process.
	// Remember to error check.
	int flags;

	if (pipe(channels[tab_index].parent_to_child_fd) == -1) 
	{
		perror("Could not open parent_to_child pipe:");
		fprintf(stderr, ERR_PRFX "  -- Could not open parent_to_child pipe:\n" ERR_SUFX, getpid(), __LINE__, strerror(errno));
		return(EXIT_STATUS_PIPE_ERROR);
	}
	
	if (pipe(channels[tab_index].child_to_parent_fd) == -1) 
	{
		perror("Could not open child_to_parent pipe:");
		return(EXIT_STATUS_PIPE_ERROR);
	}

	flags = fcntl (channels[tab_index].child_to_parent_fd[0], F_GETFL, 0);
	if (flags == -1)
	{
		fprintf(stderr, ERR_PRFX "  -- Failure to get tab %d's child_to_parent_fd[0]==%d flags:\n" ERR_SUFX, getpid(), __LINE__, tab_index, channels[tab_index].child_to_parent_fd[0], strerror(errno));
		return(EXIT_STATUS_PIPE_ERROR);
	}
	else
	{
		if (fcntl (channels[tab_index].child_to_parent_fd[0], F_SETFL, flags | O_NONBLOCK) == -1)
		{
			fprintf(stderr, ERR_PRFX "  -- Failure to set tab %d's child_to_parent_fd[0]==%d flag to NONBLOCK:\n" ERR_SUFX, getpid(), __LINE__, tab_index, channels[tab_index].child_to_parent_fd[0], strerror(errno));
			return(-1);
		}
	}
  
	flags = fcntl(channels[tab_index].parent_to_child_fd[0], F_GETFL, 0);
	if (flags == -1)
	{
		fprintf(stderr, ERR_PRFX "  -- Failure to get tab %d's parent_to_child_fd[0]==%d flags:\n" ERR_SUFX, getpid(), __LINE__, tab_index, channels[tab_index].child_to_parent_fd[0], strerror(errno));
		return(EXIT_STATUS_PIPE_ERROR);
	}
	else
	{
		if (fcntl(channels[tab_index].parent_to_child_fd[0], F_SETFL, flags | O_NONBLOCK) == -1)
		{
			fprintf(stderr, ERR_PRFX "  -- Failure to set tab %d's parent_to_child_fd[0]==%d flag to NONBLOCK:\n" ERR_SUFX, getpid(), __LINE__, tab_index, channels[tab_index].child_to_parent_fd[0], strerror(errno));
			return(EXIT_STATUS_PIPE_ERROR);
		}
	}

	/////////////////////////////////
	//      CHILD STUFF (Fork)     //
	/////////////////////////////////
	channels[tab_index].open = true;
	pid_t childpid = fork();
	if (childpid == 0) //IS CHILD PROCESS
	{
		//Child process should close unused pipes and launch
		// a window for either CONTROLLER or URL-RENDERING tabs.
		if (close(channels[tab_index].child_to_parent_fd[0]) == -1)
		{
			fprintf(stderr, ERR_PRFX "  -- Failure to close tab %d's child_to_parent_fd[0]==%d\n" ERR_SUFX, getpid(), __LINE__, tab_index, channels[tab_index].child_to_parent_fd[0], strerror(errno));
			exit(EXIT_STATUS_PIPE_ERROR);
		}
		if (close(channels[tab_index].parent_to_child_fd[1]) == -1)
		{
			fprintf(stderr, ERR_PRFX "  -- Failure to close tab %d's parent_to_child_fd[1]==%d\n" ERR_SUFX, getpid(), __LINE__, tab_index, channels[tab_index].parent_to_child_fd[1], strerror(errno));
			exit(EXIT_STATUS_PIPE_ERROR);
		}


		//Check to see if controller. If it is set up controller else setup normal new tab
		if((tab_index) == CONTROLLER_TAB)
		{
			if(run_control(channels[tab_index]) == -1)
			{
				printf("Failed to create control\n");
			}
		}
		else
		{
			if(run_url_browser(tab_index, channels[tab_index]) == -1)
			{
				printf("Failed to create new tab\n");
			}
		}
		exit(0);
	}
	/////////////////////////////////
	//     Parent STUFF (Fork)     //
	/////////////////////////////////
	else //This is a parent
	{
		//Close out the correct FDs and poll for requests from children
		close(channels[tab_index].child_to_parent_fd[1]);
		close(channels[tab_index].parent_to_child_fd[0]);

		if((tab_index) == CONTROLLER_TAB)
		{
			poll_for_children(channels, 1, MAX_TAB);
		}
	}
	return 0;
}

int main()
{
	comm_channel * channels = (comm_channel*)calloc(MAX_TAB, sizeof(comm_channel));
	setup_process(channels, 0);
	free(channels);
	return 0;
}