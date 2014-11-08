
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
   Hint: "which" field in the packet will be useful.
 */
static void packet_handler(int sig) 
{
  packet_t pkt;
  void *chunk = mm_get(&mm);
  
  if(msgrcv(msqid, chunk, sizeof(packet_t), QUEUE_MSG_TYPE, 0) == -1)
  {
    perror("Failed to receive packet (msgrcv)");
    exit(EXIT_FAILURE); 
  }

  pkt = ((packet_queue_msg*) chunk)->pkt;
  memmove(chunk, &pkt, sizeof(packet_t));

  pkt_total = pkt.how_many;
  message.num_packets = pkt.how_many;
  message.data[pkt.which] = chunk;
  pkt_cnt++;
}

/*
 * TODO - Create message from packets ... deallocate packets.
 * Return a pointer to the message on success, or NULL
 */
static char *assemble_message() 
{
  char *msg;
  int i;
  int msg_len = message.num_packets * sizeof(data_t);
  /* TODO - Allocate msg and assemble packets into it */
  char *msg_ptr;
  if((msg_ptr = (char *) mm_get(&mm)) == NULL)
  {
    perror("Failed to allocate memory for msg");
    return NULL;
  }

  if(message.num_packets != 0)
  {
    strcpy(msg, ((packet_t*)(message.data[0]))->data);
  }
  else
  {
    return NULL;
  }

  for(i = 0; i < message.num_packets - 1; i++)
  {
    strcat(msg, ((packet_t *)(message.data[i]))->data);
  }
  /* reset these for next message */
  pkt_total = 1;
  pkt_cnt = 0;
  message.num_packets = 0;
  return msg;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: packet_sender <num of messages to receive>\n");
    exit(-1);
  }

  struct sigaction pkt;
  int pid = getpid();
  pid_queue_msg pid_message;
  pid_message.mtype = (long)PID_TYPE;
  pid_message.pid = pid;


  int k = atoi(argv[1]); /* no of messages you will get from the sender */
  int i;
  char *msg;

  /* TODO - init memory manager for NUM_CHUNKS chunks of size CHUNK_SIZE each */
  if(mm_init(&mm, NUM_CHUNKS, CHUNK_SIZE) == -1)
  {
    perror("Error, mm_init failed to allocate memory");
  }
  message.num_packets = 0;

  /* TODO initialize msqid to send pid and receive messages from the message queue. Use the key in packet.h */
  if((msqid = msgget(key, 0666)) == -1)
  {
    perror("Failed on msgget");
  }
  /* TODO send process pid to the sender on the queue */
  if(msgsnd(msqid, &pid_message, sizeof(int), 0) == -1)
  {
    perror("Failed on msgsnd");
  }
  /* TODO set up SIGIO handler to read incoming packets from the queue. Check packet_handler()*/
  pkt.sa_handler = packet_handler;
  if(sigaction(SIGIO, &pkt, NULL) == -1)
  {
    perror("Failed on setting up SIGIO");
  }

  for (i = 1; i <= k; i++) {
    while (pkt_cnt < pkt_total) {
      pause(); /* block until next packet */
    }
  
    msg = assemble_message();
    if (msg == NULL) {
      perror("Failed to assemble message");
    }
    else {
      fprintf(stderr, "GOT IT: message=%s\n", msg);
      free(msg);
    }
  }
  // TODO deallocate memory manager
  mm_release(&mm);
  // TODO remove the queue once done
  msgctl(msqid, IPC_RMID, NULL);
  return EXIT_SUCCESS;
}
