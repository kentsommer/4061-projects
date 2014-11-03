
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
  void *chunk;
  pkt = get_packet();
  // TODO get the "packet_queue_msg" from the queue.
  if(pkt.which == 0)
  {
    pkt_total = pkt.how_many;
  }
  // TODO extract the packet from "packet_queue_msg" and store it in the memory from memory manager
  if((message.data[pkt.which] = (packet_t *) mm_get(&mm)) == NULL)
  {
    perror("Error, Failed to allocate memory");
  }
  memcpy(message.data[pkt.which], &pkt, sizeof(packet_t));
  pkt_cnt++; 
  message.num_packets = pkt_cnt;
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
  if((msg_ptr = (char *) malloc(sizeof(char) * msg_len + 1)) == NULL)
  {
    perror("ASSEMBLE_MSG: Failed to allocate memory to msg\n");
  }
  msg_ptr = msg;
  //copy packet data to message
  for(i = 0; i < pkt_total; i++)
  {
    memcpy(msg_ptr, ((packet_t *) message.data[i])->data, sizeof(data_t));
    msg_ptr += sizeof(data_t);
    mm_put(&mm, message.data[i]);
  }
  //Set terminal char 
  *msg_ptr = '\0'; 
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
  
  /* TODO send process pid to the sender on the queue */
  
  /* TODO set up SIGIO handler to read incoming packets from the queue. Check packet_handler()*/

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
  return EXIT_SUCCESS;
}
