//
// Created by student on 6/9/22.
//

#include "thread_pool.h"
#include "request.h"
static void *tpool_worker(void* arg);


Tpool* CreateTpool(int num_of_threads, int max_requests, char* schedalg)
{
    Tpool* tpool =(Tpool*)malloc(sizeof(Tpool));
    if (tpool==NULL)
        return NULL;
    tpool->requests_handled = createQueue(num_of_threads);
    tpool->requests_waiting = createQueue(max_requests);
    tpool->max_requests = max_requests;
    tpool->threads_num = num_of_threads;
    tpool->schedalg = schedalg;
    //add test to see if schedalg is legeal?
    pthread_mutex_init(&(tpool->requests_m), NULL);
    pthread_cond_init(&(tpool->block_requests), NULL);
    pthread_cond_init(&(tpool->request_avail), NULL);
    tpool->threads = (pthread_t*)malloc(sizeof (pthread_t) * num_of_threads);
    Thread_args* thread_args;
    for(int i =0 ; i<num_of_threads ; i++)
    {
        thread_args = CreateThreadArgs(tpool ,i );
        pthread_create(&tpool->threads[i], NULL ,tpool_worker , thread_args);
    }
    return tpool;
}

Thread_args* CreateThreadArgs(Tpool* tpool, int index){
    Thread_args * thread_args = (Thread_args*)malloc(sizeof(Thread_args));
    if( thread_args == NULL)
        return NULL;
    thread_args->tpool = tpool;
    thread_args->thread_index = index;
    return thread_args;
}

Stats* CreateThreadStats(int id)
{
    Stats* thread_stats = (Stats*)malloc(sizeof(Stats));
    if( thread_stats == NULL)
        return NULL;
    thread_stats->thread_count = 0;
    thread_stats->thread_dynamic = 0;
    thread_stats->thread_static = 0;
    thread_stats->thread_id = id;
    return thread_stats;
}

static void *tpool_worker(void* arg)
{
    Thread_args* thread_args = arg;
    Tpool* tpool = thread_args->tpool;
    Stats * thread_stats = CreateThreadStats(thread_args->thread_index);

    while(1)
    {
        pthread_mutex_lock(&tpool->requests_m);
        while (tpool->requests_waiting->size == 0)
        {
            pthread_cond_wait(&(tpool->request_avail) , &(tpool->requests_m));
        }
        struct timeval current_time , arrival_time , dispatch_time;
        arrival_time = getTimeDequeue(tpool->requests_waiting);
        gettimeofday(&current_time, NULL);

        int request_fd = dequeue(tpool->requests_waiting);
        enqueue(tpool->requests_handled,request_fd ,current_time );
        pthread_mutex_unlock(&tpool->requests_m);


        timersub(&arrival_time, & current_time, &dispatch_time);

        requestHandle(request_fd , thread_stats , arrival_time, dispatch_time);
        close(request_fd);

        pthread_mutex_lock(&tpool->requests_m);
        dequeue_data(tpool->requests_handled ,request_fd ) ;// move out the data

        pthread_cond_signal(&tpool->block_requests);
        pthread_mutex_unlock(&tpool->requests_m);
    }
}
//returns 1 if can continue 0 if not
static int HandleOverload(Tpool* tpool, int connfd, int requests_waiting, int requests_handled, char* sched)
{
    if(strcmp(sched, "block") == 0)
    {
        while(requests_waiting + requests_handled == tpool->max_requests)
            pthread_cond_wait(&tpool->block_requests, &tpool->requests_m);
        return 1;
    }
    else if (strcmp(sched, "dt") == 0)
    {
        pthread_mutex_unlock(&tpool->requests_m);
        close(connfd);
        return 0;
    }
    else if (strcmp(sched, "random")==0)
    {
        int num_to_remove = (requests_waiting + requests_handled)*0.3;
        if (requests_waiting == 0)
        {
            pthread_mutex_unlock(&tpool->requests_m);
            close(connfd);
            return 0;
        }
        int rand_idx = -1;
        for (int i=0;i<num_to_remove;i++)
        {
            rand_idx = rand() % requests_waiting;
            //if -1 then remove failed, so no more requests to remove
            if (dequeue_index(tpool->requests_waiting, rand_idx)==-1)
                break;
        }
        return 1;
    }
    else if (strcmp(sched, "dh")==0)
    {
        if (requests_waiting == 0)
        {
            pthread_mutex_unlock(&tpool->requests_m);
            close(connfd);
            return 0;
        }
        close(dequeue(tpool->requests_waiting));
    }
    return 0;
}
void ManageRequests(Tpool* tpool, int connfd)
{
    char* sched = tpool->schedalg;
    pthread_mutex_lock(&tpool->requests_m);
    int requests_waiting = queueSize(tpool->requests_waiting);
    int requests_handled = queueSize(tpool->requests_handled);
    if(requests_waiting + requests_handled == tpool->max_requests)
    {
        if(HandleOverload(tpool, connfd, requests_waiting, requests_handled, sched)==0)
            return;
    }
    struct timeval current_time;
    gettimeofday(&current_time, NULL);

    enqueue(tpool->requests_waiting, connfd , current_time);
    pthread_cond_signal(&tpool->request_avail);
    pthread_mutex_unlock(&tpool->requests_m);
}