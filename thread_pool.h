//
// Created by student on 6/8/22.
//

#ifndef HW3_OS_MYWORK_THREAD_POOL_H
#define HW3_OS_MYWORK_THREAD_POOL_H
#include "queue.h"
#include <pthread.h>
#include "segel.h"

typedef enum schedalg_t{
    Block,
    Dt,
    Random,
    Dh
} schedAlg;

typedef struct tpool_t
{
    pthread_mutex_t requests_m;
    pthread_mutex_t handled_m;
    pthread_cond_t request_avail;
    pthread_cond_t block_requests;
    //Queue* requests_handled;
    Queue* requests_waiting;
    int max_requests , threads_num , num_handled;
    pthread_t * threads;
    schedAlg schedalg;

}Tpool;

typedef  struct thread_args
{
    Tpool* tpool;
    int thread_index;
}Thread_args;

typedef  struct thread_stats
{
    int thread_count;
    int thread_dynamic;
    int thread_static;
    int thread_id;
}Stats;

Thread_args* CreateThreadArgs(Tpool* tpool, int index);

Stats* CreateThreadStats(int id);


Tpool* CreateTpool(int num_of_threads , int max_requests, char* schedalg);

void DestroyTpool(Tpool* tpool);

//handling requests recieved (master thread)
void ManageRequests(Tpool* tpool, int connfd);

#endif //HW3_OS_MYWORK_THREAD_POOL_H
