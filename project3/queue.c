#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include <string.h>

queue *ConstructQueue() {
    queue *my_queue = (queue*) malloc(sizeof (queue));
    if (my_queue == NULL) {
        return NULL;
    }

    my_queue->size = 0;
    my_queue->head = NULL;
    my_queue->tail = NULL;

    return my_queue;
}

void DestructQueue(queue *my_queue) {
    node *my_node;
    while (!isEmpty(my_queue)) {
        my_node = Dequeue(my_queue);
        if(my_node->path!=NULL)free(my_node->path);
        free(my_node);
    }
    free(my_queue);
}


int Enqueue(queue *my_queue, node *item) {
    /* Bad parameter */
    if ((my_queue == NULL) || (item == NULL)) {
        return -1;
    }

    /*the queue is empty*/
    item->prev = NULL;
    if (my_queue->size == 0) {
        my_queue->head = item;
        my_queue->tail = item;

    } else {
        /*adding item to the end of the queue*/
        my_queue->tail->prev = item;
        my_queue->tail = item;
    }
    my_queue->size++;
    return 1;
}

node * Dequeue(queue *my_queue) {
    /*the queue is empty or bad param*/
    node *item;
    if (isEmpty(my_queue)==1)
        return NULL;
    item = my_queue->head;
    my_queue->head = (my_queue->head)->prev;
    my_queue->size--;
    return item;
}

int isEmpty(queue* my_queue) {
    if (my_queue == NULL) {
        return -1;
    }
    if (my_queue->size == 0) {
        return 1;
    } else {
        return -1;
    }
}
