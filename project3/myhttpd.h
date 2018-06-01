#define POOL_SIZE 5

typedef struct pool_t{
  char** requests;
  int start ;
  int end ;
  int count ;
}pool_t;
