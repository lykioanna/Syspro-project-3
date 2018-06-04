typedef struct node {
    char* path;
    struct node *prev;
} node;

/* the HEAD of the Queue, hold the amount of node's that are in the queue*/
typedef struct queue {
    node *head;
    node *tail;
    int size;
} queue;



queue *ConstructQueue();
void DestructQueue(queue *my_queue);
int Enqueue(queue *my_queue, node *item);
node *Dequeue(queue *my_queue);
int isEmpty(queue* my_queue);
