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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <unistd.h>
#include "RUDP.h"
#include <sys/time.h>
#define MAX_LINE 256
#define MAX_CLIENTS 16
#define ZERO 0

int write_frame(FILE *fp, struct frame *f){

  int num_bytes;
  int retval;  
  if(f->eof_pos < 0){
    num_bytes = DATA_SIZE;
    retval = 0;
  }
  else{
    num_bytes = f->eof_pos;
    retval = 1;
  }
  int c;
  for(int i = 0; i <= num_bytes-1; i++){
    c = f->data[i];    
    fputc(c, fp);
  }
  return retval; 
}

int circ_offset(int first, int last, int buf_size){
  int answer;
  if( first < last || first == last )
  {
    answer = -1 * (first - last);    
  }
  else
  {
    answer = last - first + buf_size;    
  }
  return answer;  
}

int seqnum_ok(un_int win_base, un_int win_size, un_int frame_num,
 un_int seq_max){

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

int main(int argc, char* argv[]){
  struct timeval timer;
  long long int t1, t2;
  FILE *fp;
  struct hostent *host;
  struct sockaddr_in client_sin;
  un_int lfr =last_frame_received;
  un_int lfa =last_frame_acked;
  un_int rws = window_size;
  un_int seq_max = max_sequence;
  socklen_t len;
  int s;
  int new_s;
  int first_time = 1;  
  struct ack ack_frame;
  int i;
  srand(123);

  struct frame frame_buf[BUF_SIZE];

  for(i = 0; i <= BUF_SIZE-1; i++){
    frame_buf[i].frame_header_t.type = EMPTY;
  }  

  unsigned int port = atoi(argv[1]);

  // Setting the connection at server side.
  struct sockaddr_in server_sin;
  socklen_t addr_len = sizeof(server_sin);
  memset((char *)&server_sin, '\0', sizeof(server_sin));


  server_sin.sin_family = AF_INET;
  server_sin.sin_port = htons(port);         
  server_sin.sin_addr.s_addr = INADDR_ANY;
  

  if ((s = socket(PF_INET, SOCK_DGRAM, ZERO)) < 0) {
    printf("Failed to create a socket! Exiting.");
    exit(1);
  }
  
  if ((bind(s, (struct sockaddr *) &server_sin, sizeof(server_sin))) < ZERO) {
    printf("Binding failed! Exiting.");
    exit(1);
  }
  struct frame frame;
  int flag = 1;
  
  // Receive file from client.
  for(; ;){

    // while we don't receive reply of length>0 from client, continue looping.
    while(true)
    {
      int recvlen = recvfrom(s, &frame, sizeof(struct frame), ZERO,(struct sockaddr *) &client_sin, &addr_len);
      if(recvlen>0)
        break;
      else
        continue;
    }
    if(flag==1)
    {    
      gettimeofday(&timer, NULL);
      t1 = ((long long int) timer.tv_sec * 1000000ll) + (long long int) timer.tv_usec;
      printf("t1 = %lld\n",t1 );
      flag = 0;
    }
    

    /* Compose and send an ack */    
    ack_frame.type = ACK;
    ack_frame.seq = frame.frame_header_t.seq;
    printf("\nFrame No: %d received.",frame.frame_header_t.seq);

    // Sending acknowledgement to the client for the frame received.
    if(sendto(s, (char*) &ack_frame, sizeof(struct ack), ZERO,(struct sockaddr *) &client_sin, sizeof(client_sin) ) < 0){
      printf("Failed to send! Exiting.");
      exit(1);
    }

    // If the frame is outside the window, ignore it.
    if(seqnum_ok((1 + lfa) % max_sequence, window_size, frame.frame_header_t.seq, max_sequence) == false) {     
      continue;
    }
    // Writing the frame to the buffer.
  un_int seqnum_recv = frame.frame_header_t.seq;
  frame_buf[seqnum_recv % BUF_SIZE] = frame;
  int got_eof=0;

  // If we receive the expected frame, update the LFR (Last Frame Received).
  if(frame.frame_header_t.seq == (1 + lfa) % max_sequence)
  {
    int first=1;
    frametype* type = &(frame_buf[ (1 + lfa) % BUF_SIZE].frame_header_t.type);
    while(first==1 || *type == DATA)
    {
      first = 0;
      lfa = (1 + lfa) % max_sequence;
      type = &(frame_buf[lfa % BUF_SIZE].frame_header_t.type);
      if(*type == FT_REQ)
      {
        *type = EMPTY;
        int inx = lfa%BUF_SIZE;
        char mode[2] = "wb";
        if((fp = fopen((char*) &frame_buf[inx].data, mode)) == NULL){
         printf("Failed to open file");
         exit(1);
       }
      }
      else if(*type == DATA)
      {
        *type = EMPTY;
        got_eof = write_frame(fp, &(frame_buf[lfa%BUF_SIZE]));
      }
      type = &(frame_buf[((lfa+1)%seq_max)%BUF_SIZE].frame_header_t.type);
    }
 }
 if(got_eof == 1)
 {
  if(fp != NULL)
  {
   fclose(fp);
   fp = NULL;
  }
  
  printf("File Transfer Complete.. Please press ctrl z to exit.\n"); 
  
  //sleep(15000);
  for(i = 0; i <= BUF_SIZE-1; i++){
    frame_buf[i].frame_header_t.type = EMPTY;
  }
  lfr =last_frame_received;
  lfa =last_frame_acked;
  rws = window_size;
  seq_max = max_sequence;
  flag=1;
  gettimeofday(&timer, NULL);
  t2 = ((long long int) timer.tv_sec) * 1000000ll + (long long int) timer.tv_usec;
  printf("Time: %lf seconds\n", (t2 - t1) / 1000000.0);
  //FILE *fpp;
  //fpp =fopen("intermediate.txt","a");
  //fprintf(fpp,"%lf\n",(t2-t1)/1000000.0);
  //fclose(fpp);
}
}
return 0;
}
