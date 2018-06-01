#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//Arguments' Function
void read_args(int* serving_port, int* command_port, int* num_of_threads, char** root_dir,int argc,char* argv[]){
  int i;
  for(i=0 ; i< argc ; i++){
    if(!strcmp(argv[i],"-d")){
      *root_dir= malloc(sizeof(char)*(strlen(argv[i+1])+1));
      strcpy(*root_dir, argv[i+1]);
    }else if(!strcmp(argv[i],"-p")){
      *serving_port=atoi(argv[i+1]);
    }else if (!strcmp(argv[i],"-c")){
      *command_port=atoi(argv[i+1]);
    }else if (!strcmp(argv[i],"-t")){
      *num_of_threads=atoi(argv[i+1]);
    }
  }
}
