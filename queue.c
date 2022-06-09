//
// Created by student on 6/8/22.
//

#include "queue.h"

Node* CreateNode(int data){
    Node* temp = (Node*)(malloc(sizeof(Node)));
    if (temp==NULL)
        return NULL;
    temp->data = data;
    temp->next = NULL;
    temp->prev = NULL;
    return temp;
}

Queue* createQueue(int max_size)
{
    Queue* new_queue = (Queue*)malloc(sizeof(Queue));
    if (new_queue==NULL)
        return NULL;
    new_queue->head = NULL;
    new_queue->tail = NULL;
    new_queue->size = 0;
    new_queue->max_size = max_size;
    return new_queue;
}


void destroyQueue(Queue* queue)
{
    if (queue==NULL)
        return;
    Node* iterator = queue->head;
    Node* prev;
    while(iterator!=NULL)
    {
        prev = iterator;
        iterator=iterator->next;
        free(prev);
        //queue->size--; might be useful for debugging
    }
    free(queue);
}
void enqueue(Queue* queue, int data)
{
    if (queue==NULL)
        return;
    if(queue->head==NULL) {
        queue->head = CreateNode(data);
        queue->tail = queue->head;
        queue->size++;
        return;
    }
    queue->head->prev = CreateNode(data);
    queue->head->prev->next = queue->head;
    queue->head = queue->head->prev;
    queue->size++;
}

int dequeue(Queue* queue)
{
    if (queue==NULL)
        return -1;
    if (queue->tail == NULL)
    {
        return -1;
    }
    int ret_value = queue->tail->data;
    queue->tail = queue->tail->prev;
    if(queue->tail!=NULL)
        free(queue->tail->next);
    else
        queue->head = NULL;
    queue->size--;
    return ret_value;
}

int queueSize(Queue* queue)
{
    return queue->size;
}