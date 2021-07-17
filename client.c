/*
## Group:  
Keshav Beriwal - 2017B4A71301H  
Aman Badjate - 2017B3A70559H  
Prakhar Suryavansh  - 2017B4A71017H  
Garvit Soni - 2017B3A70458H 
Kshitij Verma - 2017B1A71145H 
*/
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <math.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>
#include "RUDP.h"
#define ZERO 0


int readFrame(FILE *fp, struct frame* f, un_int seq){
  int i=0;
  f->frame_header_t.seq = seq;
  f->frame_header_t.type = DATA;
  f->eof_pos = -1;
  /* populate data field */
  while(i<DATA_SIZE)
  {
    char ch = fgetc(fp);
    f->data[i] = ch;
    if(!feof(fp))
    {
      //continue;
    }
    else
    {
      f->eof_pos = i;
      return 1;
    }
    i++;
  }   
  return 0;
}
/* This function will check whether the frame that is to be sent belongs 
within the specified window or not. If yes, the frame is allowed to be sent 
else the client will wait for the acknowledgement before sending more frames.
*/
int seqnum_ok(un_int win_base, un_int win_size, un_int frame_num,
     un_int seq_max){

  // printf("%d\n",win_base);
  // printf("%d\n",win_size);
  // printf("%d\n",frame_num);
  // printf("%d\n",seq_max);
  int checking;
  un_int win_last = (win_size -1 + win_base ) % seq_max;
  if( win_base <= win_last){
    checking = frame_num <= win_last && frame_num >= win_base;    
  }
  else{
    checking = !(frame_num > win_last && frame_num < win_base);    
  }
  if(checking==1)
    return true;
  else
    return false;
}

// This function is called by an independent thread to receive acknowledgement from the server for packages it has received.
void* ack_recv(void *args){
  printf("In ACK");  
  struct send_file_args *state = (struct send_file_args*) args;
  struct ack ack_frame;
  un_int ack_seq;
  socklen_t addr_len = sizeof(state->local_sin);
  int recvlen;
  int got_eof = 0;

  // Waiting for acknowledgement for packages sent.
  for(; ; ){

    recvlen = recvfrom(state->s, &ack_frame, sizeof(struct ack), ZERO, ZERO, &addr_len);
    if(recvlen < 0 || recvlen==0){      
      continue;      
    }

    // The acknowledge sequence number.
    ack_seq = ack_frame.seq;    
    // Continue if acknowledgement is already received.
    ack_seq = ack_seq % BUF_SIZE;
    if(state->frame_buf[ack_seq].has_ack == true){ 
  
      continue;
    }
    else
    {
      printf("\nAcknowledgement of frame no:%d received.",ack_frame.seq);
    }
    

    // To indicate that acknowledgement was received and hence, cancel the timeout.
    state->frame_buf[ack_seq].has_ack = true;

    un_int conf_sec = state->frame_buf[ack_seq % BUF_SIZE].send_frame.frame_header_t.seq;
    un_int nfe = ((state->lar) + 1) % max_sequence; // nfe: next frame expected
    un_int nfe_i = nfe % BUF_SIZE; // index of nfe in buffer

    // Cancel the timeout.
    ack_seq = ack_seq % BUF_SIZE;
    pthread_cancel(state->frame_buf[ack_seq].timeout);
    
    // Updating the LAR to actual last acknowledged frame number.
    while(state->frame_buf[nfe % BUF_SIZE ].has_ack==true)
    {     
      // Resetting the acknowledgement field. 
      nfe = nfe % BUF_SIZE;
      state->frame_buf[nfe].has_ack = false;
      if(state->frame_buf[nfe].send_frame.eof_pos > 0 || state->frame_buf[nfe].send_frame.eof_pos==0)
      {
        got_eof = 1;
      }
      state->lar = ((1 + state->lar) % max_sequence);
      nfe = ((1 + state->lar) % max_sequence);      
    }
    if(got_eof == 1)
    {
      printf("Transfer Complete\n");
      return NULL;
    }
  }  
}
/*
This function will add header bits like SYN, ACK, EAK, RST and TCS to the header frame.
It also adds the type of frame and sequence number of the frame to the header.
*/
void* add_header(struct frame *send_frame, bool SYN, bool ACK, bool EAK, bool RST, bool TCS, un_int send_seqnum, frametype type)
{
  send_frame->frame_header_t.SYN=SYN;
  send_frame->frame_header_t.ACK=ACK;
  send_frame->frame_header_t.EAK=EAK;
  send_frame->frame_header_t.RST=RST;
  send_frame->frame_header_t.TCS=TCS;
  send_frame->frame_header_t.seq=send_seqnum;
  send_frame->frame_header_t.type=FT_REQ;
}
/*
This function is independently called by a thread and it adds timeout to the frame. 
The value of timeout is decided by both client and server. When the timeout occurs,
it instructs the client to resent the package to the server.
*/
void* add_timout(struct timeout_args* timeout_state, int duration, struct frame* f, int s )
{
  timeout_state->duration = duration;
  timeout_state->s = s;
  timeout_state->frame = f;
}
/* The timeout Function.
*/
void* timeout_func(void *args){

  struct timeout_args *params = (struct timeout_args*) args;
  time_t resend_time;
  int socketdesc = params->s;
  int frame_size = sizeof(struct frame); 
  for(; ;){
    usleep(params->duration);
    if(sendto(socketdesc, (char*) params->frame, frame_size, ZERO, (struct sockaddr *) &(params->remote_sin), sizeof(params->remote_sin) ) < ZERO){
      printf("Failed to send! Exiting.");
      exit(1);
    }    
  }
  return NULL;
}
/*
This is the main send_file function that sends the packsges to the server.
*/
void* send_file(void *args){

  un_int i;
  int got_eof;     
  struct send_file_args *state = (struct send_file_args*) args;

  // Opening the file
  FILE *fp;
  char *file_name = state->file_name;
  if((fp = fopen(file_name, "rb")) == NULL){
    printf("Failed to open file! Exiting");
    exit(1);
  }

  // Sending the first frame.
  un_int send_seqnum;
  send_seqnum = (1 + state->lfs ) % state->seq_max;
  i = send_seqnum % BUF_SIZE;

  // Load the header of first frame. The arguments are in sequence: frame, SYN, ACK, EAK, RST, TCS, sequence number of frame, frametype.
  add_header(&(state->frame_buf[i].send_frame),1,0,0,0,0,send_seqnum,FT_REQ);
  
  state->frame_buf[i].send_frame.eof_pos = -1;
  strcpy(state->frame_buf[i].send_frame.data, state->new_filename);

  // Add timeout to first frame.

  add_timout(&(state->frame_buf[i].timeout_state), timeoutVal, &(state->frame_buf[i].send_frame), state->s ); 
  struct sockaddr_in r_sin;
  struct send_file_args sfa;
  
  r_sin = state->remote_sin;
  state->frame_buf[i].timeout_state.remote_sin = r_sin;  
 
  struct send_slot *time;
  time = &(state->frame_buf[i]);
  pthread_create(&(time->timeout), NULL, timeout_func, &(time->timeout_state));
  state->lfs = send_seqnum;

   // Set has ack.
  state->frame_buf[i].has_ack = false;

  //Actually send the frame.
  int socketdesc = state->s;
  char *send_buffer = (char*) &(state->frame_buf[i].send_frame);
  int frame_size = sizeof(struct frame);
  printf("\nSending frame No: %d.",state->frame_buf[i].send_frame.frame_header_t.seq);
  if(sendto(socketdesc, send_buffer, frame_size, ZERO, (struct sockaddr *) &(state->remote_sin), sizeof(state->remote_sin) ) < 0){
    printf("Failed to send package! Exiting.");
    exit(1);
  }
  else
  {
    //printf("\nFrame No: %d sent.",state->frame_buf[i].send_frame.frame_header_t.seq);
  } 
  int flag = 1;
  for(; ;){

    // getting the next seqnnum.
    send_seqnum = (1 + state->lfs) % max_sequence;   
    if(seqnum_ok( (1+ state->lar) % max_sequence, state->sws, send_seqnum, max_sequence) ==false){    
      sched_yield();
      continue;
    }

    // Add this frame to the queue.
    i = send_seqnum % BUF_SIZE;    

    // Adding timeout to the frame
    r_sin = state->remote_sin;
    state->frame_buf[i].timeout_state.remote_sin = r_sin; 
    add_timout(&(state->frame_buf[i].timeout_state), timeoutVal, &(state->frame_buf[i].send_frame), state->s); 
    //state->frame_buf[i].timeout_state.remote_sin = state->remote_sin;
    time = &(state->frame_buf[i]);
    pthread_create(&(time->timeout), NULL, timeout_func, &(time->timeout_state));
    //pthread_create(&(state->frame_buf[i].timeout), NULL, timeout_func, &(state->frame_buf[i].timeout_state));

    // Update the last frame sent (lfs). 
    state->lfs = send_seqnum;

    // Read the frame to check if it's the last frame to send.
    got_eof = readFrame(fp, &(state->frame_buf[i].send_frame), send_seqnum);
    
    // Set the acknowledgement as 0.
    state->frame_buf[i].has_ack = false;
    
    // Actually send the frame
    int frame_size = sizeof(struct frame);
    int remoteSize = sizeof(state->remote_sin);
    int socketdesc = state->s;
    printf("\nSending frame No: %d.",state->frame_buf[i].send_frame.frame_header_t.seq);
    if(sendto(state->s, (char*) &(state->frame_buf[i].send_frame), frame_size, ZERO, (struct sockaddr *) &(state->remote_sin), remoteSize ) < 0){
      printf("Failed to send! Exixting");
      exit(1);
    }
    else
    {
      //printf("\nFrame No: %d sent.",state->frame_buf[i].send_frame.frame_header_t.seq);
    }
    
    // Terminate the thread if eof was reached.
    if(got_eof ==1)
      break;
  }
  fclose(fp);
  return NULL;
}

int create_thread(pthread_t *thread, int i, struct send_file_args *state)
{

  if(i==1)
  {
    pthread_create(thread, NULL, send_file, (void*) state); 
  }
  else
  {
    pthread_create(thread, NULL, ack_recv, (void*) state); 
  }
  return 1;  
}
int main(int argc, char *argv[]){  
  // struct timeval timer;
  // long long int t1, t2;

  struct sockaddr_in remote_sin, local_sin;
  socklen_t addr_len;
  unsigned int my_port, remote_port; // port number to communicate with server
  char* hostIP;
  int s;
  struct frame send_frame;
  pthread_t send_thread;
  pthread_t ack_thread;
  int i,j=0;

  // Initializing the values of send_file_args with values given as arguments by user
  struct send_file_args *state;
  state = (struct send_file_args*) malloc(sizeof(struct send_file_args));

  // The name of the file to send to the server.
  state->file_name = (char*) malloc((strlen(argv[1]) + 1)); 
  strcpy(state->file_name,argv[1]);

  // The new name of the file by which the file sent by the client will be saved.
  state->new_filename=(char*) malloc(strlen(argv[2])+1);
  strcpy(state->new_filename,argv[2]);

  // Storing Server IP : HostIP
  hostIP = argv[3];

  // Client Port
  my_port = atoi(argv[4]);

  // Server Port
  remote_port = atoi(argv[5]);

  // Set the parameters as agreed upon by both the peers.
  state->lar = last_ack_recv;
  state->lfs = last_frame_sent;
  state->sws = window_size;
  state->seq_max = max_sequence;  

  
  // Client Address
  int size = sizeof(state->local_sin); 
  bzero((char*) &(state->local_sin), size);
  state->local_sin.sin_family = AF_INET;
  state->local_sin.sin_addr.s_addr = INADDR_ANY;
  state->local_sin.sin_port = htons(my_port); 

  // Server Address
  bzero((char*) &(state->remote_sin), size);
  state->remote_sin.sin_family = AF_INET;
  state->remote_sin.sin_addr.s_addr=inet_addr(hostIP);
  state->remote_sin.sin_port = htons(remote_port);

  // Initially make all frames in the buffer as unacknowledged.
  while(j<=BUF_SIZE-1)
  {
    state->frame_buf[j++].has_ack = false;
  }

  // Allocate the socket Id for the client to send over. */
  int socketdesc;
  if ((socketdesc = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
    printf( "Failed to create a socket");
    exit(1);
  }
  state->s = socketdesc;

  // Bind to client socket to receive messages */
  if ((bind(socketdesc, (struct sockaddr *) &(state->local_sin), sizeof(state->local_sin))) < 0) {
      printf("Binding Failed! Exiting.");
      exit(1);
  }


  // Sending the file to the server.
  printf("Your Transfer will begin shortly.\n");
  printf("Sending files.\n");
  struct timeval timer;
  long long int t1, t2;
  gettimeofday(&timer, NULL);
  t1 = ((long long int) timer.tv_sec * 1000000ll) + (long long int) timer.tv_usec;
  if(create_thread(&send_thread, 1,  (void*) state)==0) // is 3rd argument is 1, send_file function is called by the newly created thread. Else, Acknowledgement received function is called.
  {
    printf("Falied to create thread! Exiting.");
  }
  if(create_thread(&ack_thread, 0,  (void*) state)==0)
  {
    printf("Falied to create thread! Exiting.");
  }
  pthread_join(send_thread, NULL); 
  pthread_join(ack_thread, NULL); 
  gettimeofday(&timer, NULL);
  t2 = ((long long int) timer.tv_sec) * 1000000ll + (long long int) timer.tv_usec;
  printf("Time: %lf seconds\n", (t2 - t1) / 1000000.0);
  FILE *fpp;
  fpp =fopen("intermediate.txt","a");
  fprintf(fpp,"%lf\n",(t2-t1)/1000000.0);
  fclose(fpp);
  close(state->s); 		   
  return 0;
}
