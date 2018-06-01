#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "ArgumentsCrawler.h"
#include "myhttpd.h"



pool_t* pool;
int pool_busy = 0;

pool_t* initPool(){
  pool_t* pool;
  pool = malloc(sizeof(pool_t));
  pool->start = pool->count = 0;
  pool->end = -1;
  pool->requests = malloc(POOL_SIZE*sizeof(char*));
  int i ;
  for(i = 0 ; i < POOL_SIZE ; i++){
    pool->requests[i] = NULL;
  }
  return pool;
}

void freePool(pool_t** pool){
  int i;
  for(i = 0 ; i < POOL_SIZE ; i++){
    if((*pool)->requests[i]!=NULL)free((*pool)->requests[i]);
  }
  free((*pool)->requests);
  free(*pool);
  *pool = NULL;
}

int main(int argc, char *argv[]){

  int host_or_ip=0, port=0, command_port=0, num_of_threads=0;
  char *save_dir, *starting_URL;
  save_dir=NULL;
  starting_URL=NULL;
  read_args(&host_or_ip, &port, &command_port, &num_of_threads, &save_dir, &starting_URL, argc, argv);

  if (argc<11){
    printf("Less Arguments. Try again\n");
    exit(1);
  }else if (host_or_ip==0 || port==0 || command_port==0 || num_of_threads==0 ){
    printf("Something's zero. Try again\n");
    exit(1);
  }else if (save_dir==NULL || starting_URL==NULL){
    printf("Something's NULL. Try again\n");
    free(save_dir);
    free(starting_URL);
    exit(1);
  }

  pool = initPool();
  int sizeOfGet = strlen("GET  HTTP/1.1\n\
  User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)\n\
  Host: www.tutorialspoint.com\n\
  Accept-Language: en-us\n\
  Accept-Encoding: gzip, deflate\n\
  Connection: Keep-Alive\n\
                      \n")+strlen(starting_URL)+1;
  char *message;
  message= malloc(sizeof(char)*sizeOfGet);
  sprintf(message, "GET %s HTTP/1.1\n\
User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)\n\
Host: www.tutorialspoint.com\n\
Accept-Language: en-us\n\
Accept-Encoding: gzip, deflate\n\
Connection: Keep-Alive\n\
                      \n", starting_URL);
  printf(" TO MINImA %s\n", message);


// 1ST SOCKET
////////////////////////////////////////////////////////////////////////////////
  struct sockaddr_in address;
  int serving_sock = 0, valread;
  struct sockaddr_in serv_addr;
  char *hello = "Hello from client";
  char buffer[1024] = {0};
  if ((serving_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
      printf("\n Socket creation error \n");
      return -1;
  }

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons( port );

  // Convert IPv4 and IPv6 addresses from text to binary form
  int a= inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
  if(a<=0){
      printf("\nInvalid address/ Address not supported \n");
      return -1;
  }

  if (connect(serving_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
      printf("\nConnection Failed \n");
      return -1;
  }
  send(serving_sock , message , strlen(message) , 0 );
  printf("Hello message sent\n");
  while(1){
    valread = read( serving_sock , buffer, 1024);
    printf("%s\n",buffer );
  }

////////////////////////////////////////////////////////////////////////////////

// 2ND SOCKET
  struct sockaddr_in address2;
  int command_sock = 0, valread2;
  struct sockaddr_in serv_addr2;
  char *hello2 = "Hello from client2";
  char buffer2[1024] = {0};
  if ((command_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
      printf("\n Socket creation error \n");
      return -1;
  }

  memset(&serv_addr2, '0', sizeof(serv_addr2));

  serv_addr2.sin_family = AF_INET;
  serv_addr2.sin_port = htons( command_port );

  // Convert IPv4 and IPv6 addresses from text to binary form
  if(inet_pton(AF_INET, "127.0.0.1", &serv_addr2.sin_addr)<=0)
  {
      printf("\nInvalid address/ Address not supported \n");
      return -1;
  }

  if (connect(command_sock, (struct sockaddr *)&serv_addr2, sizeof(serv_addr2)) < 0)
  {
      printf("\nConnection Failed \n");
      return -1;
  }
  send(command_sock , hello2 , strlen(hello2) , 0 );
  printf("Hello message sent2\n");
  valread2 = read( command_sock , buffer2, 1024);
  printf("%s\n",buffer2 );
////////////////////////////////////////////////////////////////////////////////

  freePool(&pool);
  free(save_dir);
  free(starting_URL);
  return 0;
}
