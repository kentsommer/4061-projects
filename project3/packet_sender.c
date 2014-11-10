/*CSci4061 F2014 Assignment 3
*section: 4
*date: 11/10/14
*names: Kent Sommer, Kanad Gupta, Xi Chen
*id: somme282, kgupta, chen2806
*/

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
    ////////////////////////////////////////////////////////////////////////////////////////
    // This line was added to facilitate proper tracking of the number of packets to send //
    pkt_total = how_many;                                                                 //
    ////////////////////////////////////////////////////////////////////////////////////////
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
  packet_t pkt;

  pkt = get_packet();
  // temp is just used for temporarily printing the packet.
  char temp[PACKET_SIZE + 2];
  strcpy(temp, pkt.data);
  temp[3] = '\0';
  printf ("Sending packet: %s\n", temp);
  pkt_cnt++;

  // Creates a packet_queue_msg for the current packet
  packet_queue_msg message = {QUEUE_MSG_TYPE, pkt};
  // Sends packet_queue_msg to the receiver
  if(msgsnd(msqid, &message, sizeof(packet_queue_msg), 0) == -1)
  {
    perror("Failed while doing msgsnd");
    exit(-1);
  }
  // Sends SIGIO to the receiver if message sending was successful
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

  //Sigaction and interval setup
  sigset_t set;
  struct itimerval interval;
  struct sigaction saction;           

  /* Creates a message queue */ 
  if((msqid = msgget(key, IPC_CREAT | 0666)) == -1)
  {
    perror("Failed while doing msgget");
    exit(-1);
  }
  /*  Reads the receiver pid from the queue and stores it for future use*/
  if(msgrcv(msqid, &queuemsg, sizeof(int), PID_TYPE, 0) == -1)
  {
    perror("Failed while going msgrcv");
    exit(-1);
  }
  receiver_pid = queuemsg.pid;

  //Setup sigaction handler, flags, and mask
  saction.sa_handler = packet_sender;
  saction.sa_flags = 0;
  sigemptyset(&saction.sa_mask);

  if(sigaction(SIGALRM, &saction, NULL) == -1)
  {
      perror("Failed: sigaction error.");
      exit(-1);
  }
  
  //Setup itimer
  struct itimerval timer;
  timer.it_value.tv_sec = INTERVAL;
  timer.it_value.tv_usec = INTERVAL_USEC;
  timer.it_interval.tv_sec = INTERVAL;
  timer.it_interval.tv_usec = INTERVAL_USEC;
  if(setitimer(ITIMER_REAL, &timer, NULL) == -1)
  {
    perror("Failed doing setitimer");
    exit(-1);
  }

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
