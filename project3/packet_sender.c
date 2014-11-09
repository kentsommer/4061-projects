
#include <time.h>
#include "packet.h"

static int pkt_cnt = 0;     /* how many packets have been sent for current message */
static int pkt_total = 1;   /* how many packets to send send for the message */
static int msqid = -1; /* id of the message queue */
static int receiver_pid; /* pid of the receiver */

/*
   Returns the packet for the current message. The packet is selected randomly.
   The number of packets for the current message are decided randomly.
   Each packet has a how_many field which denotes the number of packets in the current message.
   Each packet is string of 3 characters. All 3 characters for given packet are the same.
   For example, the message with 3 packets will be aaabbbccc. But these packets will be sent out order.
   So, message aaabbbccc can be sent as bbb -> aaa -> ccc
   */
static packet_t get_packet() {
  static int which = -1;
  static int how_many;
  static int num_of_packets_sent = 0;
  static int is_packet_sent[MAX_PACKETS];
  int i;

  packet_t pkt;

  if (num_of_packets_sent == 0) {
    how_many = rand() % MAX_PACKETS;
    if (how_many == 0) {
      how_many = 1;
    }
    pkt_total = how_many;
    printf("Number of packets in current message: %d\n", how_many);
    which = -1;
    for (i = 0; i < MAX_PACKETS; ++i) {
      is_packet_sent[i] = 0;
    }
  }
  which = rand() % how_many;
  if (is_packet_sent[which] == 1) {
    i = (which + 1) % how_many;
    while (i != which) {
      if (is_packet_sent[i] == 0) {
        which = i;
        break;
      }
      i = (i + 1) % how_many;
    } 

  }
  pkt.how_many = how_many;
  pkt.which = which;

  memset(pkt.data, 'a' + which, sizeof(data_t));

  is_packet_sent[which] = 1;
  num_of_packets_sent++;
  if (num_of_packets_sent == how_many) {
    num_of_packets_sent = 0;
  }

  return pkt;
}

static void packet_sender(int sig) {
	printf("In packet sender, sig is: %d\n", sig);
  packet_t pkt;

  pkt = get_packet();
  // temp is just used for temporarily printing the packet.
  char temp[PACKET_SIZE + 2];
  strcpy(temp, pkt.data);
  temp[3] = '\0';
  printf ("Sending packet: %s\n", temp);
  pkt_cnt++;

  // TODO Create a packet_queue_msg for the current packet.
  packet_queue_msg message = {QUEUE_MSG_TYPE, pkt};
  // TODO send this packet_queue_msg to the receiver. Handle any error appropriately.
  if(msgsnd(msqid, &message, sizeof(packet_queue_msg), 0) == -1)
  {
    perror("Failed while doing msgsnd");
  }
  //printf("Receiver pid is: %d\n", receiver_pid);
  // TODO send SIGIO to the receiver if message sending was successful.
  kill(receiver_pid, SIGIO);
}

int main(int argc, char **argv) {

  pid_queue_msg queuemsg;
  if (argc != 2) {
    printf("Usage: packet_sender <num of messages to send>\n");
    exit(-1);
  }

  int k = atoi(argv[1]); /* number of messages  to send */
  srand(time(NULL)); /* seed for random number generator */

  int i;

  sigset_t set;
  struct itimerval interval;
  struct sigaction act;           

  /* TODO Create a message queue */ 
  if((msqid = msgget(key, IPC_CREAT | 0666)) == -1)
  {
    perror("Failed while doing msgget");
  }
  else
  {
  	printf("msgget succeeded, msqid is: %d\n", msqid);
  }
  /*  TODO read the receiver pid from the queue and store it for future use*/
  if(msgrcv(msqid, &queuemsg, sizeof(int), PID_TYPE, 0) == -1)
  {
    perror("Failed while going msgrcv");
  }
  receiver_pid = queuemsg.pid;


  printf("Got pid : %d\n", receiver_pid);
 

  act.sa_handler = packet_sender;
  act.sa_flags = 0;
  sigemptyset(&act.sa_mask);

  if(sigaction(SIGALRM, &act, NULL) == -1)
  {
      fprintf(stderr, "Error: sigaction error.\nProgram will now exit.\n");
      return EXIT_FAILURE;
  }
  /* DONE!!
   *TODO - set up alarm handler -- mask all signals within it
   * The alarm handler will get the packet and send the packet to the receiver. Check packet_sender();
   * Don't care about the old mask, and SIGALRM will be blocked for us anyway,
   * but we want to make sure act is properly initialized.
  */
  // struct sigaction alarm;
  // alarm.sa_handler = packet_sender;
  // alarm.sa_flags = 0;

  // if(sigaction(SIGALRM, &alarm, NULL) == -1)
  // {
  //   perror("Error, failed to set alarm to catch sig");
  // }

  //   DONE!!!
  //  * TODO - turn on alarm timer ...
  //  * use  INTERVAL and INTERVAL_USEC for sec and usec values
  
  struct itimerval timer;
  timer.it_value.tv_sec = INTERVAL;
  timer.it_value.tv_usec = INTERVAL_USEC;
  timer.it_interval.tv_sec = INTERVAL;
  timer.it_interval.tv_usec = INTERVAL_USEC;
  if(setitimer(ITIMER_REAL, &timer, NULL) == -1)
  {
    perror("Failed doing setitimer");
  }
  /* And the timer */ //IS there anything else needed?

  /* NOTE: the below code wont run now as you have not set the SIGALARM handler. Hence, 
     set up the SIGALARM handler and the timer first. */
  for (i = 1; i <= k; i++) {
    printf("==========================\n", i);
    printf("Sending Message: %d\n", i);
    while (pkt_cnt < pkt_total) {
    	//printf("Packet count is: %d\n", pkt_cnt);
    	//printf("Packet count is: %d\n", pkt_total);
      pause(); /* block until next packet is sent. SIGALARM will unblock and call the handler.*/
    }
    //printf("Reset packet total\n");
    pkt_total = 1;
    pkt_cnt = 0;
  }

  return EXIT_SUCCESS;
}
