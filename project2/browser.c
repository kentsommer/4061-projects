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
	comm_channel channel = b_window->channel;

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
			fprintf(stderr, ERR_PRFX " --Failed to write to controller channel.child_to_parent_fd[1]==%d\n" ERR_SUFX,
				getpid(), __LINE__, b_window->channel.child_to_parent_fd[1], stderror(errno));
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

	child_req_to_parent req;
	
	while (1) 
	{
		process_single_gtk_event();
		usleep(1000);

		//Need to communicate with Router process here.
		//Insert code here!!
	        //Handle each type of message (few mentioned below)
		//  NEW_URI_ENTERED: render_web_page_in_tab(uri, b_window);
		//  TAB_KILLED: process all gtk events();
	}

	return 0;
}

int main()
{

	comm_channel comm[MAX_TAB];
	comm_channel controller_router_channel;
	//This is Router process
	//Make a controller and URL-RENDERING tab when user request it. 
	int pid;
	//Fork controller
	pid=fork();
	if(pid==0)
	{
		//this is CONTROLLER
		if(pipe(int controller_router_channel.parent_to_child_fd)==-1)
		{
				perror (“pipe error”);
				exit (1);
		}
		if(pipe(int controller_router_channel.child_to_parent_fd)==-1)
		{
				perror (“pipe error”);
				exit (1);
		}
		run_control(controller_router_channel);    //fork controller and use controller_router_channel to communicate
		
	}
	if(pid>0)
	{
		//this is ROUTER







		//With pipes, this process should communicate with controller and tabs.
		
		//poll for requests from child on one to many pipes
		//Use non-blocking read call to read data, identify the type of message and act accordingly
		//  CREATE_TAB:
		//	Create two pipes for bi-directional communication
		//	Fork URL_RENDERING process
		//  NEW_URI_ENTERED:
		//	Write this message on the pipe connecting to ROUTER and URL_RENDERING process.
		//  TAB_KILLED:
		//	Close file descriptors of corresponding tab's pipes.
		//When all child processes exit including controller, exit a success!
		//For more accurate details see section 4.1 in writeup.

		return 0;
	}
}
