#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "Arguments.h"
#include "myhttpd.h"
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <string.h>
#include <time.h>


void acquire_handler();
void release_handler();
pthread_mutex_t mtx;
pthread_cond_t cond_handlers;
pthread_cond_t cond_server;
pool_t* pool;
int pool_busy = 0;
int exit_programm = 0;

void* Handler(void* ptr){
  acquire_handler();
  //printf("%s\n",pool->requests[pool->start] );
  release_handler();
  return NULL;
}

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

void acquire_server(){
  pthread_mutex_lock(&mtx);
  while((pool_busy || pool->count == POOL_SIZE) && exit_programm==0){
    pthread_cond_wait(&cond_server, &mtx);
  }
  if(exit_programm == 1){
    pthread_cond_signal(&cond_handlers);
    pthread_mutex_unlock(&mtx);
    pthread_exit((void*) 0);
  }
  pool_busy = 1;
  pthread_mutex_unlock(&mtx);
}

void release_server(){
  pthread_mutex_lock(&mtx);
  pool-> end =( pool->end + 1) % POOL_SIZE ;
  pool->count++;
  pool_busy = 0;
  pthread_cond_signal(&cond_handlers);
  pthread_mutex_unlock(&mtx);
}

void acquire_handler(){
  pthread_mutex_lock(&mtx);
  while((pool_busy || pool->count == 0) && exit_programm==0){
    pthread_cond_wait(&cond_handlers, &mtx);
  }
  if(exit_programm == 1){
    printf("eksw\n" );
    pthread_cond_signal(&cond_handlers);
    pthread_cond_signal(&cond_server);
    pthread_mutex_unlock(&mtx);
    pthread_exit((void*) 0);
  }
  pool_busy = 1;
  pthread_mutex_unlock(&mtx);
}

void release_handler(){
  pthread_mutex_lock(&mtx);
  free(pool->requests[pool->start]);
  pool->start = ( pool ->start + 1) % POOL_SIZE ;
  pool->count--;
  pool_busy = 0;
  pthread_cond_signal(&cond_handlers);
  pthread_cond_signal(&cond_server);
  pthread_mutex_unlock(&mtx);
}

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]){
  int serving_port=0, command_port=0, num_of_threads=0, i, sock;
  char *root_dir;

  if( argc != 9){
    printf("Wrong Number of args. Try again\n");
    exit(1);
  }

  root_dir = NULL;

  read_args(&serving_port, &command_port, &num_of_threads, &root_dir, argc, argv);

  if (argc<8){
    printf("Less Arguments. Try again\n");
    exit(1);
  }else if (serving_port==0 || command_port==0 || num_of_threads==0 ){
    printf("Something's zero. Try again\n");
    exit(1);
  }else if (root_dir==NULL){
    printf("Something's NULL. Try again\n");
    free(root_dir);
    exit(1);
  }

////1ST SOCKET
// Creating socket file descriptor /////////////////////////////////////////////
  int server_fd, new_socket, valread;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);
  char buffer[1024] = {0};
  char *hello = "Hello from server";
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
      perror("socket failed");
      exit(EXIT_FAILURE);
  }

  // Forcefully attaching socket to the port 8080
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                &opt, sizeof(opt))){
      perror("setsockopt");
      exit(EXIT_FAILURE);
  }
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons( serving_port );

  // Forcefully attaching socket to the port 8080
  if (bind(server_fd, (struct sockaddr *)&address,sizeof(address))<0){
      perror("bind failed");
      exit(EXIT_FAILURE);
  }
  if (listen(server_fd, 3) < 0){
      perror("listen");
      exit(EXIT_FAILURE);
  }
////////////////////////////////////////////////////////////////////////////////


//2ND SOCKET////////////////////////////////////////////////////////////////////
    int command_fd, new_socket2, valread2;
    struct sockaddr_in address2;
    int opt2 = 1;
    int addrlen2 = sizeof(address2);
    char buffer2[1024] = {0};
    char *hello2 = "Hello from server2";

    // Creating socket file descriptor
    if ((command_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(command_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt2, sizeof(opt2)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address2.sin_family = AF_INET;
    address2.sin_addr.s_addr = INADDR_ANY;
    address2.sin_port = htons( command_port );

    // Forcefully attaching socket to the port 8080
    if (bind(command_fd, (struct sockaddr *)&address2,
                                 sizeof(address2))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(command_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
////////////////////////////////////////////////////////////////////////////////

//gia na katalabainei ti akouei
  fd_set read_fds;
  while(1){
    FD_ZERO(&read_fds);
    FD_SET(server_fd, &read_fds);
    FD_SET(command_fd, &read_fds);
    int ret= select(command_fd+1,&read_fds,NULL, NULL, NULL);
    if (ret==-1){
      printf("failed\n");
    }else if(ret==0){
      printf("timeout\n");
    }else {
      if (FD_ISSET(server_fd, &read_fds)){
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,(socklen_t*)&addrlen))<0){
            perror("accept");
            exit(EXIT_FAILURE);
        }
        valread = read( new_socket , buffer, 1024);
        char *token;
        char *rest;

        //printf("%s\n",buffer );
        token = strtok_r(buffer, " ", &rest);
        token = strtok_r(NULL," ", &rest);

        printf("TO TOKEN: %s \n", token);

        ///////////////////////////////

        if(access(token, F_OK | R_OK) != -1){
          FILE *fp;
          fp=fopen(token, "r");
          if (fp == NULL){
              perror("FP OPEN");
              exit(-1);
          }
          fseek(fp, 0, SEEK_END);
          int size=0;
          size = ftell(fp);
          fseek(fp, 0, SEEK_SET);
          char* text=NULL;
          text= malloc(size);
          if (text){
            fread(text,1,size,fp);
          }
          fclose(fp);
          int sizeOfMessage= strlen("HTTP/1.1 200 OK \n\
Date: Mon, 27 May 2018 12:28:53 GMT\n\
Server: myhttpd/1.0.0 (Ubuntu64)\n\
Content-Length: 8873\n\
Content-Type: text/html\n\
Connection: Closed\n\
                     \n")+ strlen(text);
          char* message;
          message= malloc(sizeof(char)*sizeOfMessage);
          time_t t;
          time(&t);
          strcpy(message, "HTTP/1.1 200 OK \nDate: ");
          time_t curtime;
    			struct tm *info;
    			char buffertime[80];
    			time( &curtime );
    			info = localtime( &curtime );
    			strftime(buffertime,80,"%a, %d %b %Y %X GMT \n", info);
          strcat(message, buffertime);
          strcat(message, "Server: myhttpd/1.0.0 (Ubuntu64)\nContent-Length:");
          char txtsize[20];
          sprintf(txtsize,"%d", size);
          strcat(message, txtsize);
          strcat(message, "\nContent-Type: text/html\nConnection: Closed \n\
               \n");
          strcat(message, text);
          send(new_socket , message , strlen(message)+500 , 0 );
          FD_CLR(server_fd,&read_fds);
          close(new_socket);
          free(message);

        }else if( access(token, F_OK )== -1){
          int sizeOfMessage= strlen("HTTP/1.1 404 Not Found\n\
Date: Mon, 27 May 2018 12:28:53 GMT\n\
Server: myhttpd/1.0.0 (Ubuntu64)\n\
Content-Length: 124\n\
Content-Type: text/html\n\
Connection: Closed\n\
                     \n");
          char* message;
          message=malloc(sizeof(char)*sizeOfMessage);
          strcpy(message, "HTTP/1.1 404 Not Found\nDate: ");
          time_t curtime;
    			struct tm *info;
    			char buffertime[80];
    			time( &curtime );
    			info = localtime( &curtime );
    			strftime(buffertime,80,"%a, %d %b %Y %X GMT \n", info);
          strcat(message, buffertime);
          sizeOfMessage= strlen(">Sorry dude, couldn’t find this file.<");
          strcat(message, "Server: myhttpd/1.0.0 (Ubuntu64)\nContent-Length: ");
          char txtsize[20];
          sprintf(txtsize,"%d", sizeOfMessage);
          strcat(message, txtsize);
          strcat(message, "\n Content-Type: text/html\nConnection: Closed\n\
          \n\
<html>Sorry dude, couldn’t find this file.</html>");

          free(message);

        }

      }
      if (FD_ISSET(command_fd, &read_fds)){
        if ((new_socket2 = accept(command_fd, (struct sockaddr *)&address2,
                           (socklen_t*)&addrlen2))<0){
            perror("accept");
            exit(EXIT_FAILURE);
        }
        valread2 = read( new_socket2 , buffer2, 1024);
        printf("%s\n",buffer2 );
        send(new_socket2 , hello2 , strlen(hello2) , 0 );
        printf("Hello message sent2\n");
        FD_CLR(command_fd,&read_fds);
        close(new_socket2);

      }
    }
  }
////////////////////////////////////////////////////////////////////////////////

  pthread_t handlers[num_of_threads];
  pool = initPool();

  for(i=0;i<num_of_threads;i++){
      printf("Created handler\n");
    if(pthread_create(&handlers[i], NULL, Handler, NULL)){
          perror("Fail to create thread");
          exit(1);
      }
  }
  pthread_mutex_init(&mtx, NULL);
  pthread_cond_init(&cond_handlers, NULL);
  pthread_cond_init(&cond_server, NULL);

  acquire_server();
  pool->requests[pool->end+1] = malloc(sizeof(char)*strlen(buffer));
  strcpy(pool->requests[pool->end+1],buffer);
  release_server();

  sleep(2);
  printf("telos\n" );
  exit_programm = 1 ;
  pthread_cond_signal(&cond_handlers);

  for(i=0;i<num_of_threads;i++){
      if(pthread_join(handlers[i], NULL)){
          perror("fail to join thread");
          exit(1);
      }
      printf("Join handler\n");
  }

  if(pthread_mutex_destroy(&mtx)){
      perror("fail to destroy mtx");
      exit(1);
  }

  if(pthread_cond_destroy(&cond_handlers)){
      perror("fail to destroy condition variable");
      exit(1);
  }

  if(pthread_cond_destroy(&cond_server)){
      perror("fail to destroy condition variable");
      exit(1);
  }

  //freePool(&pool);
  free(root_dir);
  return 0;
}
