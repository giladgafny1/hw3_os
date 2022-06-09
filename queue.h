//
// Created by student on 6/8/22.
//

#ifndef HW3_OS_MYWORK_QUEUE_H
#define HW3_OS_MYWORK_QUEUE_H

#include <stdio.h>
#include <stdlib.h>

typedef enum queue_result_t{
    QUEUESUCCESS,
    QUEUEEMPTY,
} QueueResult;

typedef struct node_t {
    int data;
    struct node_t* next;
    struct node_t* prev;
} Node;

typedef struct queue_t{
    Node* head;
    Node* tail;
    unsigned int size;
    unsigned int max_size;
} Queue;

Queue* createQueue(int max_size);

void destroyQueue(Queue* queue);

void enqueue(Queue* queue, int data);

int dequeue(Queue* queue);

int queueSize(Queue* queue);



#endif //HW3_OS_MYWORK_QUEUE_H
