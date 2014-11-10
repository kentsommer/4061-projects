/*CSci4061 F2014 Assignment 3
*section: 4
*date: 11/10/14
*names: Kent Sommer, Kanad Gupta, Xi Chen
*id: somme282, kgupta, chen2806
*/

#include "packet.h"

int msqid = -1;

static message_t message;   /* current message structure */
static mm_t mm;             /* memory manager will allocate memory for packets */
static int pkt_cnt = 0;     /* how many packets have arrived for current message */
static int pkt_total = 1;   /* how many packets to be received for the message */

/*
   Handles the incoming packet. 
   Store the packet in a chunk from memory manager.
   The packets for given message will come out of order. 
   Hence you need to take care to store and assemble it correctly.
   Example, message "aaabbb" can come as bbb -> aaa, hence, you need to assemble it
   as aaabbb.
 */
static void packet_handler(int sig) 
{
  packet_t pkt;
  void *chunk = mm_get(&mm);
  
  if(msgrcv(msqid, chunk, sizeof(packet_t) * MAX_PACKETS, QUEUE_MSG_TYPE, 0) == -1)
  {
    perror("Failed to receive packet (msgrcv)");
    exit(-1);
  }

  pkt = ((packet_queue_msg*) chunk)->pkt;
  memmove(chunk, &pkt, sizeof(packet_t));

  pkt_total = pkt.how_many;
  message.num_packets = pkt.how_many;
  message.data[pkt.which] = chunk;
  pkt_cnt++;
}

/*
 * Assemble Message takes in a set of packets and returns
 * a pointer to the message on success, or NULL
 */
static char *assemble_message() 
{
  char *msg;
  int i;
  int msg_len = message.num_packets * sizeof(data_t);
  /* Allocates the msg and assemble packets into it */
  char *msg_ptr; // Used to set end of message as '\0'
  if((msg = (char *) malloc(sizeof(char) * msg_len + 1)) == NULL)
  {
    perror("Failed to allocate memory for msg");
    return NULL;
  }
  msg_ptr = msg;

  for(i = 0; i < pkt_total; i++)
  {
	memcpy(msg_ptr, ((packet_t *) message.data[i])->data, sizeof(data_t));
	msg_ptr += sizeof(data_t);
	mm_put(&mm, message.data[i]);
  }
  *msg_ptr = '\0'; // Sets end so that when printed you don't have extra k's on the end

  pkt_total = 1;
  pkt_cnt = 0;
  message.num_packets = 0;
  return msg;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: packet_receiver <num of messages to receive>\n");
    exit(-1);
  }

  //Setup sigaction
  struct sigaction pkt;
  int pid = getpid();
  pid_queue_msg pid_message;
  pid_message.mtype = (long)PID_TYPE;
  pid_message.pid = pid;

  int k = atoi(argv[1]); /* number of messages you will get from the sender */
  int i;
  char *msg;

  /* Initializes the memory manager for NUM_CHUNKS chunks of size CHUNK_SIZE */
  if(mm_init(&mm, NUM_CHUNKS, CHUNK_SIZE) == -1)
  {
    perror("Error, mm_init failed to allocate memory");
    exit(-1);
  }
  message.num_packets = 0;
  /* Initializes msqid to send pid and receive messages from the message queue. Uses the key in packet.h */
  if((msqid = msgget(key, 0666)) == -1)
  {
    perror("Failed on msgget");
    exit(-1);
  }

  /* Sends the process pid to the sender on the queue */
  if(msgsnd(msqid, &pid_message, sizeof(int), 0) == -1)
  {
    perror("Failed on msgsnd");
    exit(-1);
  }
  /* Sets up SIGIO handler to read incoming packets from the queue. Check packet_handler()*/
  pkt.sa_handler = packet_handler;
  if(sigaction(SIGIO, &pkt, NULL) == -1)
  {
    perror("Failed on setting up SIGIO");
    exit(-1);
  }

  for (i = 1; i <= k; i++) 
  {
    while (pkt_cnt < pkt_total) 
    {
      pause(); /* block until next packet */
    }
    msg = assemble_message();
    if (msg == NULL) 
    {
      perror("Failed to assemble message");
      exit(-1);
    }
    else 
    {
      fprintf(stderr, "GOT IT: message=%s\n", msg);
      free(msg);
    }
  }
  // deallocate memory manager
  mm_release(&mm);
  // remove the queue once finished
  msgctl(msqid, IPC_RMID, NULL);
  return EXIT_SUCCESS;
}
