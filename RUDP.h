/*
## Group:  
Keshav Beriwal - 2017B4A71301H  
Aman Badjate - 2017B3A70559H  
Prakhar Suryavansh  - 2017B4A71017H  
Garvit Soni - 2017B3A70458H 
Kshitij Verma - 2017B1A71145H 
*/
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <sys/time.h>
#include<stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>

#define DATA_SIZE 300 // in bytes.
#define MAX_PENDING 256
#define BUF_SIZE 64 // Maximum no of frames in buffer.
#define un_int uint32_t // Unsigned integer to handle sequence number.
#define last_ack_recv 0 // Initially set last acknowledgement received as 0
#define last_frame_acked 0
#define SocketType int
#define last_frame_sent 0 // Initially set last frame sent as 0.
#define last_frame_received 0 // Initially set last frame received as 0.
#define window_size 4 // Set the window size as agreed by both peers.
#define max_sequence 8 // set maximum possible sequence number as agreed by bith peers.
# define timeoutVal 100000


typedef enum{EMPTY, ACK, DATA, FT_REQ} frametype; // Frame Types 


/* Format for frame sent over the network */

struct frame_header{
  bool SYN,ACK,EAK,RST,TCS;
  un_int seq;
  frametype type;
};

struct frame{  
  unsigned char data[DATA_SIZE];
  struct frame_header frame_header_t;
  int eof_pos; // position of eof if there is one. else negative
  
};

/* Arguments for timeout thread */
struct timeout_args{
  unsigned int duration;
    SocketType s;     
    struct sockaddr_in remote_sin;
    struct frame *frame; // pointer to frame to resend
};

/* A slot in the send buffer: frame + metadata */
struct send_slot{
  struct timeout_args timeout_state;  
  bool has_ack; // boolean: has the frame in this slot been acked?
  bool check;
  struct frame send_frame;
  pthread_t timeout;
};

/* The state of the client, for use by recv_ack and send threads */
struct send_file_args{

  struct sockaddr_in remote_sin, local_sin;
  int s; // socket id
  char *file_name;
  int eof_frame_sent;
  char *new_filename;
  un_int lar; // last ack received
  un_int lfs; // [seq_num of] last frame received
  bool check;
  un_int sws; // send window size
  un_int seq_max; // highest possible sequence number  
  struct send_slot frame_buf[BUF_SIZE]; // frame buffer
};

/* Format for non-data frame */
struct ack{  
  un_int seq;
  frametype type;
};

