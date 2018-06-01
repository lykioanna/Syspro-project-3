#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ArgumentsCrawler.h"

//Arguments' Function Crawler
void read_args(int* host_or_ip, int* port, int* command_port, int* num_of_threads, char** save_dir, char** starting_URL, int argc,char* argv[]){
  int i;
  for(i=0 ; i< argc ; i++){
    if(!strcmp(argv[i],"-d")){
      *save_dir= malloc(sizeof(char)*(strlen(argv[i+1])+1));
      strcpy(*save_dir, argv[i+1]);
      *starting_URL= malloc(sizeof(char)*(strlen(argv[i+2])+1));
      strcpy(*starting_URL, argv[i+2]);
    }else if(!strcmp(argv[i],"-p")){
      *port=atoi(argv[i+1]);
    }else if (!strcmp(argv[i],"-c")){
      *command_port=atoi(argv[i+1]);
    }else if (!strcmp(argv[i],"-t")){
      *num_of_threads=atoi(argv[i+1]);
    }else if (!strcmp(argv[i],"-h")){
      *host_or_ip=atoi(argv[i+1]);
    }
  }
}
