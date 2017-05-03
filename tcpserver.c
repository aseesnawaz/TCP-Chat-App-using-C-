#include "stdio.h"
#include "stdlib.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "string.h"
#include "netinet/in.h"
#include "pthread.h"


#define PORT 4444 //Globally Accessible Macros
#define BUF_SIZE 2000
#define CLADDR_LEN 100

void * receiveMessage(void * socket){
  int sockfd, ret;
  char buffer[BUF_SIZE];
  sockfd = (int) socket;
  memset(buffer, 0, BUF_SIZE);
  for(;;){
    ret = recvfrom(sockfd, buffer, BUF_SIZE, 0, NULL, NULL);
    if(ret < 0){
      printf("Error receiving data!\n");
    }
    else{
      printf("Client: ");
      fputs(buffer, stdout);
    }
  }
}

void main(){
  struct sockaddr_in addr, cl_addr;
  // int sockfd, len, ret, newsockfd; Described below
  char buffer[BUF_SIZE];
  pid_t childpid;
  char clientAddr[CLADDR_LEN];
  pthread_t rThread;

  //Create Socket and print status
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
  printf("Error creating socket!\n");
  exit(1);
  }
  printf("Socket created...\n");

  //set memory
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = PORT;

  //Bind the above create socket to IP, PORT, and Family
  int ret = bind(sockfd, (struct sockaddr *) &addr, sizeof(addr));
  if (ret < 0) {
  printf("Error binding!\n");
  exit(1);
  }
  printf("Binding done...\n");
  printf("Waiting for a connection...\n");

  //Listen for connections with maximum backlog of 5 connections (waiting while a connection is beging handled)
  listen(sockfd, 5);

  //When conencted, accespt the connection
  int len = sizeof(cl_addr);
  int newsockfd = accept(sockfd, (struct sockaddr_in *) &cl_addr, &len);

  if (newsockfd < 0) {
    printf("Error accepting connection!\n");
    exit(1);
   }

  inet_ntop(AF_INET, &(cl_addr.sin_addr), clientAddr, CLADDR_LEN);
  printf("Connection accepted from %s...\n", clientAddr);

  memset(buffer, 0, BUF_SIZE);
  printf("Enter your messages one by one and press return key!\n");

  //Create a new thread or this connection and receive message
  ret = pthread_create(&rThread, NULL, receiveMessage, (void *) newsockfd);
  if (ret) {
    printf("ERROR: Return Code from pthread_create() is %d\n", ret);
    exit(1);
  }

  while(fgets(buffer, BUF_SIZE, stdin) != NULL){
    ret = sendto(newsockfd, buffer, BUF_SIZE,0, (struct sockaddr *) &cl_addr, len);
    if (ret < 0) {
      printf("Error sending data!\n");
      exit(1);
    }
  }

  close(newsockfd);
  close(sockfd);

  pthread_exit(NULL);
  return;
}
