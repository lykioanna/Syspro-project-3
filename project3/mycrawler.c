#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "ArgumentsCrawler.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "queue.h"


void acquire_handler();
void release_handler();
pthread_mutex_t mtx;
pthread_cond_t cond_handlers_cr;
pthread_cond_t cond_crawler;
int exit_programm = 0;
int queue_busy = 0;
queue* my_queue;

void* Handler(void* ptr){
    node* my_node;
    while (1) {
        acquire_handler();
        my_node = Dequeue(my_queue);
        if(my_node->path!=NULL){
            printf("%ld   Dequeued: %s\n",pthread_self(), my_node->path);
            free(my_node->path);
        }
        free(my_node);
        release_handler();
    }
}

void acquire_crawler(){
  pthread_mutex_lock(&mtx);
  while(queue_busy && exit_programm==0){
    pthread_cond_wait(&cond_crawler, &mtx);
  }
  if(exit_programm == 1){
    pthread_cond_broadcast(&cond_handlers_cr);
    pthread_mutex_unlock(&mtx);
    pthread_exit((void*) 0);
  }
  queue_busy = 1;
  pthread_mutex_unlock(&mtx);
}

void release_crawler(){
  pthread_mutex_lock(&mtx);
  queue_busy = 0;
  pthread_cond_signal(&cond_handlers_cr);
  pthread_mutex_unlock(&mtx);
}

void acquire_handler(){
  pthread_mutex_lock(&mtx);
  while((queue_busy || my_queue->size==0) && exit_programm==0){
    pthread_cond_wait(&cond_handlers_cr, &mtx);
  }
  if(exit_programm == 1){
      printf("%ld thread finishing\n",pthread_self() );
    pthread_cond_broadcast(&cond_handlers_cr);
    // pthread_cond_signal(&cond_server);
    pthread_mutex_unlock(&mtx);
    pthread_exit((void*) 0);
  }
  queue_busy = 1;
  pthread_mutex_unlock(&mtx);
}

void release_handler(){
  pthread_mutex_lock(&mtx);

  queue_busy = 0;

  pthread_cond_signal(&cond_crawler);
  pthread_cond_signal(&cond_handlers_cr);
  pthread_mutex_unlock(&mtx);
}


int main(int argc, char *argv[]){

  int host_or_ip=0, port=0, command_port=0, num_of_threads=0,i=0;
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

  pthread_t handlers[num_of_threads];
  my_queue = ConstructQueue();

  for(i=0;i<num_of_threads;i++){
      printf("Created handler\n");
    if(pthread_create(&handlers[i], NULL, Handler, NULL)){
          perror("Fail to create thread");
          exit(1);
      }
  }
  if (pthread_mutex_init(&mtx, NULL) != 0){
        printf("\n mutex init failed\n");
        return 1;
    }

  pthread_cond_init(&cond_handlers_cr, NULL);
  pthread_cond_init(&cond_crawler, NULL);


//   struct stat st = {0};
//
//   if (stat(save_dir, &st) == -1) {
//       mkdir(save_dir, 0700);
//   }
//
//   int sizeOfGet = strlen("GET  HTTP/1.1\n\
// User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)\n\
// Host: www.tutorialspoint.com\n\
// Accept-Language: en-us\n\
// Accept-Encoding: gzip, deflate\n\
// Connection: Keep-Alive\n\n")+strlen(starting_URL)+1;
//   char *message;
//   message= malloc(sizeof(char)*sizeOfGet);
//   sprintf(message, "GET %s HTTP/1.1\n\
// User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)\n\
// Host: www.tutorialspoint.com\n\
// Accept-Language: en-us\n\
// Accept-Encoding: gzip, deflate\n\
// Connection: Keep-Alive\n\n", starting_URL);
//   printf(" TO MINImA %s\n", message);


// 1ST SOCKET
////////////////////////////////////////////////////////////////////////////////
  // struct sockaddr_in address;
  // int serving_sock = 0, valread;
  // struct sockaddr_in serv_addr;
  // char buffer[1024] = {0};
  // if ((serving_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
  //     printf("\n Socket creation error \n");
  //     return -1;
  // }
  //
  // memset(&serv_addr, '0', sizeof(serv_addr));
  //
  // serv_addr.sin_family = AF_INET;
  // serv_addr.sin_port = htons( port );
  //
  // // Convert IPv4 and IPv6 addresses from text to binary form
  // int a= inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
  // if(a<=0){
  //     printf("\nInvalid address/ Address not supported \n");
  //     return -1;
  // }
  //
  // if (connect(serving_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
  //     printf("\nConnection Failed \n");
  //     return -1;
  // }
  // send(serving_sock , message , strlen(message) , 0 );
  // printf("message sent\n");
  //
  // char* token;
  // char* rest;
  // char* tmpbuff;
  //
  // valread = read( serving_sock , buffer, 1024);
  // buffer[valread-1]='\0';
  // tmpbuff = malloc(sizeof(char)*strlen(buffer)+1);
  // strcpy(tmpbuff, buffer);
  // token=strtok_r(tmpbuff," ",&rest);
  // token=strtok_r(NULL," ",&rest);
  // printf("TOKEN %s\n",token);
  //
  // if (!strcmp(token,"200")){
  //
  // }else if(!strcmp(token,"404")){
  //
  // }else if(!strcmp(token,"403")){
  //
  // }
  //
  // while (valread>0){
  //   //printf("%s\n",buffer);
  //   valread = read( serving_sock , buffer, 1024);
  //   buffer[valread-1]='\0';
  // }
  //
  //


////////////////////////////////////////////////////////////////////////////////

// 2ND SOCKET
  // struct sockaddr_in address2;
  // int command_sock = 0, valread2;
  // struct sockaddr_in serv_addr2;
  // char *hello2 = "Hello from client2";
  // char buffer2[1024] = {0};
  // if ((command_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  // {
  //     printf("\n Socket creation error \n");
  //     return -1;
  // }
  //
  // memset(&serv_addr2, '0', sizeof(serv_addr2));
  //
  // serv_addr2.sin_family = AF_INET;
  // serv_addr2.sin_port = htons( command_port );
  //
  // // Convert IPv4 and IPv6 addresses from text to binary form
  // if(inet_pton(AF_INET, "127.0.0.1", &serv_addr2.sin_addr)<=0)
  // {
  //     printf("\nInvalid address/ Address not supported \n");
  //     return -1;
  // }
  //
  // if (connect(command_sock, (struct sockaddr *)&serv_addr2, sizeof(serv_addr2)) < 0)
  // {
  //     printf("\nConnection Failed \n");
  //     return -1;
  // }
  // send(command_sock , hello2 , strlen(hello2) , 0 );
  // printf("Hello message sent2\n");
  // valread2 = read( command_sock , buffer2, 1024);
  // printf("%s\n",buffer2 );
////////////////////////////////////////////////////////////////////////////////
    node* my_node;
    for (i = 0; i < 9; i++) {
        my_node = (node*) malloc(sizeof (node));
        my_node->path=malloc(sizeof(char)*10);
        strcpy(my_node->path,"geia");
        char temp[10];
        sprintf(temp, "%d", i);
        strcat(my_node->path,temp);
        acquire_crawler();
        Enqueue(my_queue, my_node);
        release_crawler();
    }
    sleep(2);
    printf("eksww\n" );
    exit_programm = 1 ;
    pthread_cond_broadcast(&cond_handlers_cr);

    for(i=0;i<num_of_threads;i++){
        if(pthread_join(handlers[i], NULL)){
            perror("fail to join thread");
            exit(1);
        }
        printf("Join handler\n");
    }

    if(pthread_mutex_destroy(&mtx)){
        perror("fail to destroy mtx");
        // exit(1);
    }

    if(pthread_cond_destroy(&cond_handlers_cr)){
        perror("fail to destroy condition variable");
        exit(1);
    }

    if(pthread_cond_destroy(&cond_crawler)){
        perror("fail to destroy condition variable");
        exit(1);
    }
// DestructQueue(my_queue);
  free(save_dir);
  free(starting_URL);
  return 0;
}
