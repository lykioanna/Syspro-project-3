#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "ArgumentsCrawler.h"
#include "queue.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ctype.h>

void* acquire_handler();
void release_handler();
void acquire_crawler();
void release_crawler();
struct hostent* get_hostent(char*);
pthread_mutex_t mtx;
pthread_cond_t cond_handlers_cr;
pthread_cond_t cond_crawler;
int exit_programm = 0;
int queue_busy = 0;
queue* my_queue;
char *save_dir;
struct stat st = {0};

void* Handler(void* ptr) {
    node* my_node;
    int port;
    char* host_or_ip , *token1, *rest1, *fullpath, *ret2,*path;
    while (1) {
        acquire_handler();
        my_node = Dequeue(my_queue);
        if(my_node!=NULL) {
          if(my_node->path!=NULL)
            printf("%ld   Dequeued: %s\n",pthread_self(), my_node->path);
          else{
            release_handler();
            continue;
          }
        }else{
          release_handler();
          continue;
        }
        release_handler();

        fullpath=malloc(sizeof(char)*(strlen(my_node->path)+1));
        strcpy(fullpath,my_node->path);
        ret2=strstr(fullpath,"//");
        token1=strtok_r(ret2+2*sizeof(char),":",&rest1);
        host_or_ip = malloc(sizeof(char)*strlen(token1)+1);
        strcpy(host_or_ip,token1);
        free(fullpath);

        fullpath=malloc(sizeof(char)*(strlen(my_node->path)+1));
        strcpy(fullpath,my_node->path);
        ret2=strstr(fullpath,":");
        ret2+=2;
        ret2=strstr(ret2,":");
        printf("daad  %s\n", ret2 );
        port = atoi(strtok_r(ret2+1*sizeof(char),"/",&rest1));
        printf("HOSTTTTTT %s %d   %s\n",host_or_ip,port,path);

        token1 = strtok_r(NULL,"",&rest1);
        path = malloc(sizeof(char)*strlen(token1)+2);
        sprintf(path,"/%s",token1);
        free(fullpath);
        free(my_node->path);
        int sizeOfGet = strlen("GET  HTTP/1.1\n\
User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)\n\
Host: www.tutorialspoint.com\n\
Accept-Language: en-us\n\
Accept-Encoding: gzip, deflate\n\
Connection: Keep-Alive\n\n")+strlen(path)+1;
        char *message;
        message= malloc(sizeof(char)*sizeOfGet);
        sprintf(message, "GET %s HTTP/1.1\n\
User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)\n\
Host: www.tutorialspoint.com\n\
Accept-Language: en-us\n\
Accept-Encoding: gzip, deflate\n\
Connection: Keep-Alive\n\n", path);
        printf(" Message Sent: %s\n", message);


        // 1ST SOCKET
        ////////////////////////////////////////////////////////////////////////////////
        struct sockaddr_in address;
        int serving_sock = 0, valread;
        struct sockaddr_in serv_addr;
        char buffer[1025] = {0};
        if ((serving_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("\n Socket creation error \n");
            return NULL;
        }

        memset(&serv_addr, '0', sizeof(serv_addr));

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons( port );

        struct hostent* myhost;
        // Convert IPv4 and IPv6 addresses from text to binary form
        if ( ( (myhost=get_hostent(host_or_ip)) == NULL)) {
            perror("Fail to gethostbyname");
            exit(1);
        }

        memcpy(&serv_addr.sin_addr, myhost->h_addr, myhost->h_length);

        if (connect(serving_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            printf("\nConnection Failed \n");
            return NULL;
        }
        send(serving_sock , message , strlen(message) , 0 );
        printf("message sent\n");

        char* token;
        char* rest;
        char* tmpbuff, *ret, *fmessage;
        fmessage=malloc(sizeof(char)*(1025));
        int filesize=0, response=0;

        valread = read( serving_sock , buffer, 1024); //pairnei tin apantisi
        buffer[valread-1]='\0';
        strcpy(fmessage, buffer);                      //oli i proti apantisi
        tmpbuff = malloc(sizeof(char)*(strlen(buffer)+1));
        strcpy(tmpbuff, buffer);                       //tmpbuff gia na brw tin APANTISI
        token=strtok_r(tmpbuff," ",&rest);
        token=strtok_r(NULL," ",&rest);
        response= atoi(token);                        //arithmos APANTISIS
        printf("Response: %d\n",response);

        ret=strstr(fmessage,"Content-Length:");
        filesize= atoi(ret+15);

        if (response==200) {
            char* subfolder;    //Crawler/site0
            char* sitefile;     //Crawler/site0/page987
            //starting_URL : site0/page0_232
            char *tmp, *ret, *qpath;
            tmp=malloc(sizeof(char)*(strlen(path)+1));
            strcpy(tmp,path);

            sitefile=malloc(sizeof(char)*(strlen(save_dir)+strlen(path)+2));
            strcpy(sitefile,save_dir);
            strcat(sitefile,path);

            token1=strtok_r(tmp,"/",&rest1);
            subfolder=malloc(sizeof(char)*(strlen(save_dir)+strlen(token1)+2));
            strcpy(subfolder,save_dir);
            strcat(subfolder,"/");
            strcat(subfolder,token1);
            printf("SUBFOLDER %s\n",subfolder);


            //anoigei ton ypofakelo
            if (stat(subfolder, &st) == -1) {
                mkdir(subfolder, 0700);
            }
            //anoigei to arxeio
            FILE *fp = fopen(sitefile, "ab+");
            char* link;
            ret=strstr(fmessage,"\n\n");
            fprintf(fp,"%s",ret);
            while(valread>0) {
                valread = read( serving_sock , buffer, 1024);
                if(valread>1) {
                    fprintf(fp, "%s",buffer);
                }
                ret = buffer;
                while ((ret = strstr(ret,"<a href="))!=NULL) {
                    token = strchr(ret,'=');
                    link = strtok_r(token,">",&rest1);
                    link+=7;
                    link=strchr(link,'/');
                    //SOS krataei kai to / stin arxi

                    qpath=malloc(sizeof(char)*(strlen(" http://:")+strlen(host_or_ip)+7+strlen(link))+1);
                    sprintf(qpath,"http://%s:%d%s",host_or_ip,port,link);

                    my_node = (node*) malloc(sizeof (node));
                    my_node->path=malloc(sizeof(char)*strlen(qpath)+1);
                    strcpy(my_node->path,qpath);
                    printf("mypath   %s\n",my_node->path );
                    acquire_crawler();
                    Enqueue(my_queue, my_node);
                    release_crawler();

                    ret += (strlen(link)*sizeof(char));

                    free(qpath);
                }
            }
            free(fmessage);
            free(subfolder);
            free(sitefile);
            free(tmp);
            free(tmpbuff);
        } else if(response==404) {

        } else if(response==403) {

        }
        free(path);
        free(host_or_ip);
    }
}

void acquire_crawler() {
    pthread_mutex_lock(&mtx);
    while(queue_busy && exit_programm==0) {
        pthread_cond_wait(&cond_crawler, &mtx);
    }
    queue_busy = 1;
    pthread_mutex_unlock(&mtx);
}

void release_crawler() {
    pthread_mutex_lock(&mtx);
    queue_busy = 0;
    pthread_cond_signal(&cond_handlers_cr);
    pthread_mutex_unlock(&mtx);
}

void* acquire_handler() {
    pthread_mutex_lock(&mtx);
    while((queue_busy || my_queue->size==0) && exit_programm==0) {
        pthread_cond_wait(&cond_handlers_cr, &mtx);
    }
    if(exit_programm == 1) {
        printf("%ld thread finishing\n",pthread_self() );
        pthread_cond_broadcast(&cond_handlers_cr);
        pthread_mutex_unlock(&mtx);
        pthread_exit((void*) 0);
    }
    queue_busy = 1;
    pthread_mutex_unlock(&mtx);
}

void release_handler() {
    pthread_mutex_lock(&mtx);

    queue_busy = 0;

    pthread_cond_signal(&cond_crawler);
    pthread_cond_signal(&cond_handlers_cr);
    pthread_mutex_unlock(&mtx);
}

struct hostent* get_hostent(char* given_host) {
    int is_name=0;
    int i;
    for (i = 0; i < strlen(given_host); i++) {
        if(given_host[i]=='.' || (given_host[i]<='9' && given_host[i]>='0') ) {
            int do_nothing  ;
        } else {
            is_name=1;
            break;
        }
    }
    if(is_name==1) {
        return gethostbyname(given_host);
    } else {
        struct sockaddr_in ip_a;
        socklen_t len;
        inet_aton(given_host,(struct in_addr*)&ip_a);
        return gethostbyaddr(&ip_a, sizeof(ip_a), AF_INET);
    }
}

int main(int argc, char *argv[]) { ///////////////////////////////////////////////

    int port=0,i=0, command_port=0, num_of_threads=0;
    char *starting_URL;
    char* host_or_ip, *qpath;
    host_or_ip = NULL;
    save_dir=NULL;
    starting_URL=NULL;
    node* my_node;
    read_args(&host_or_ip, &port, &command_port, &num_of_threads, &save_dir, &starting_URL, argc, argv);

    if (argc<11) {
        printf("Less Arguments. Try again\n");
        exit(1);
    } else if ( port==0 || command_port==0 || num_of_threads==0 ) {
        printf("Something's zero. Try again\n");
        exit(1);
    } else if (save_dir==NULL || starting_URL==NULL || host_or_ip==NULL) {
        printf("Something's NULL. Try again\n");
        free(save_dir);
        free(starting_URL);
        exit(1);
    }

    pthread_t handlers[num_of_threads];
    my_queue = ConstructQueue();

    if (pthread_mutex_init(&mtx, NULL) != 0) {
        printf("\n mutex init failed\n");
        return 1;
    }

    pthread_cond_init(&cond_handlers_cr, NULL);
    pthread_cond_init(&cond_crawler, NULL);

    for(i=0; i<num_of_threads; i++) {
        printf("Created handler\n");
        if(pthread_create(&handlers[i], NULL, Handler, NULL)) {
            perror("Fail to create thread");
            exit(1);
        }
    }

    //elegxei an yparxei o fakelos
    if (stat(save_dir, &st) == -1) {
        mkdir(save_dir, 0700);
    }

////////////////////////////////////////////////////////////////////////////////

    qpath=malloc(sizeof(char)*(strlen(" http://:")+strlen(host_or_ip)+7+strlen(starting_URL))+1);
    sprintf(qpath,"http://%s:%d%s",host_or_ip,port,starting_URL);
    my_node = (node*) malloc(sizeof (node));
    my_node->path=malloc(sizeof(char)*strlen(qpath)+1);
    strcpy(my_node->path,qpath);
    acquire_crawler();
    Enqueue(my_queue, my_node);
    release_crawler();

////////////////////////////////////////////////////////////////////////////////

// 2ND SOCKET
    // struct sockaddr_in address2;
    // int command_sock = 0, valread2;
    // struct sockaddr_in serv_addr2;
    // char *hello2 = "Hello from client2";
    // char buffer2[1024] = {0};
    // if ((command_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
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
    // if(inet_pton(AF_INET, "127.0.0.1", &serv_addr2.sin_addr)<=0){
    //     printf("\nInvalid address/ Address not supported \n");
    //     return -1;
    // }
    //
    // if (connect(command_sock, (struct sockaddr *)&serv_addr2, sizeof(serv_addr2)) < 0){
    //     printf("\nConnection Failed \n");
    //     return -1;
    // }
    // send(command_sock , hello2 , strlen(hello2) , 0 );
    // printf("Hello message sent2\n");
    // valread2 = read( command_sock , buffer2, 1024);
    // printf("%s\n",buffer2 );


////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

    sleep(2);
    printf("eksww\n" );
    exit_programm = 1 ;
    pthread_cond_broadcast(&cond_handlers_cr);

    for(i=0; i<num_of_threads; i++) {
        if(pthread_join(handlers[i], NULL)) {
            perror("fail to join thread");
            exit(1);
        }
        printf("Join handler\n");
    }

    if(pthread_mutex_destroy(&mtx)) {
        perror("fail to destroy mtx");
        // exit(1);
    }

    if(pthread_cond_destroy(&cond_handlers_cr)) {
        perror("fail to destroy condition variable");
        exit(1);
    }

    if(pthread_cond_destroy(&cond_crawler)) {
        perror("fail to destroy condition variable");
        exit(1);
    }

    DestructQueue(my_queue);
    free(save_dir);
    free(starting_URL);
    return 0;
}
