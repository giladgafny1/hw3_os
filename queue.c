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

static void removeNode(Queue* queue, Node* node)
{
    if(node->prev != NULL)
    {
        node->prev->next = node->next;
    }
    if(node->next!=NULL)
    {
        node->next->prev = node->prev;
    }
    if(node==queue->head)
    {
        queue->head = node->next;
    }
    if(node==queue->tail)
    {
        queue->tail = node->prev;
    }
    free(node);
    queue->size--;
}
int dequeue_index(Queue* queue, int index)
{
    if (queue==NULL)
        return -1;
    if (index >= queue->size)
        return -1;
    int data_to_return=-1;
    int curr_idx = 0;
    Node* itr = queue->head;
    while(itr!=NULL)
    {
        if(curr_idx == index)
        {
            data_to_return = itr->data;
            removeNode(queue, itr);
        }
        itr++;
        curr_idx++;
    }
    return data_to_return;
}

int dequeue_data(Queue* queue, int data)
{
    if(queue==NULL)
        return -1;
    int data_to_return=-1;
    Node* itr = queue->head;
    while(itr!=NULL)
    {
        if(itr->data == data)
        {
            data_to_return = itr->data;
            removeNode(queue, itr);
        }
        itr=itr->next;
    }
    return data_to_return;
}

int queueSize(Queue* queue)
{
    return queue->size;
}