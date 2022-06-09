//
// Created by student on 6/9/22.
//

#include "thread_pool.h"

static void *tpool_worker(void* arg);


Tpool* CreateTpool(int num_of_threads, int max_requests, char* schedalg)
{
    Tpool* tpool =(Tpool*)malloc(sizeof(Tpool));
    if (tpool==NULL)
        return NULL;
    pthread_t  thread;
    tpool->working_threads = createQueue(num_of_threads);
    tpool->requests = createQueue(max_requests);
    tpool->max_requests = max_requests;
    tpool->threads_num = num_of_threads;
    tpool->schedalg = schedalg;
    //add test to see if schedalg is legeal?
    pthread_mutex_init(&(tpool->requests_m), NULL);
    pthread_cond_init(&(tpool->block_requests), NULL);
    pthread_cond_init(&(tpool->request_avail), NULL);

    for(int i =0 ; i<num_of_threads ; i++)
    {
        pthread_create(&thread, NULL ,tpool_worker , tpool);
    }
    return tpool;
}

static void *tpool_worker(void* arg)
{
    Tpool* tpool = arg;

    while(1)
    {
        pthread_mutex_lock(&tpool->requests_m);


    }
}

void HandleRequests(Tpool* tpool, int connfd)
{
    char* sched = tpool->schedalg;
    pthread_mutex_lock(&tpool->requests_m);
    if(queueSize(tpool->requests) + queueSize(tpool->working_threads) == tpool->max_requests)
    {
        if(strcmp(sched, "block") == 0)
        {
            pthread_cond_wait(&tpool->block_requests, &tpool->requests_m);
        }
        else if (strcmp(sched, "dt") == 0)
        {
            pthread_mutex_unlock(&tpool->requests_m);
            close(connfd);
            return;
        }
        else if (strcmp(sched, "random"))
        {

        }
    }
}