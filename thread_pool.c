//
// Created by student on 6/9/22.
//

#include "thread_pool.h"
#include "request.h"
#include "math.h"
static void *tpool_worker(void* arg);


Tpool* CreateTpool(int num_of_threads, int max_requests, char* schedalg)
{
    Tpool* tpool =(Tpool*)malloc(sizeof(Tpool));
    if (tpool==NULL)
        return NULL;
    //tpool->requests_handled = createQueue(num_of_threads);
    tpool->requests_waiting = createQueue(max_requests);
    tpool->max_requests = max_requests;
    tpool->threads_num = num_of_threads;
    tpool->num_handled = 0;
    if(strcmp(schedalg, "block") == 0)
        tpool->schedalg=Block;
    else if (strcmp(schedalg, "dt") == 0)
        tpool->schedalg=Dt;
    else if (strcmp(schedalg, "random") == 0)
        tpool->schedalg = Random;
    else if (strcmp(schedalg, "dh") == 0)
        tpool->schedalg = Dh;
    //add test to see if schedalg is legeal?
    pthread_mutex_init(&(tpool->requests_m), NULL);
    pthread_mutex_init(&(tpool->handled_m), NULL);
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
        pthread_mutex_unlock(&tpool->requests_m);

        pthread_mutex_lock(&tpool->handled_m);
        tpool->num_handled++;
        //enqueue(tpool->requests_handled,request_fd ,current_time );
        pthread_mutex_unlock(&tpool->handled_m);


        timersub(& current_time,&arrival_time, &dispatch_time);

        requestHandle(request_fd , thread_stats , arrival_time, dispatch_time);
        Close(request_fd);

        pthread_mutex_lock(&tpool->handled_m);
        //dequeue_data(tpool->requests_handled ,request_fd ) ;// move out the data
        tpool->num_handled--;
        pthread_cond_signal(&tpool->block_requests);
        pthread_mutex_unlock(&tpool->handled_m);
    }
}
//returns 1 if can continue 0 if not
static int HandleOverload(Tpool* tpool, int connfd, schedAlg sched)
{
    if(sched == Block)
    {
        pthread_mutex_lock(&tpool->requests_m);
        while(queueSize(tpool->requests_waiting) + tpool->num_handled == tpool->max_requests)
            pthread_cond_wait(&tpool->block_requests, &tpool->requests_m);
        pthread_mutex_unlock(&tpool->requests_m);
        return 1;
    }
    else if (sched == Dt)
    {
        Close(connfd);
        return 0;
    }
    else if (sched == Random)
    {
        pthread_mutex_lock(&tpool->requests_m);
        int requests_waiting = queueSize(tpool->requests_waiting);
        int num_to_remove = ceil((double)(requests_waiting)*0.3);
        if (requests_waiting == 0)
        {
            pthread_mutex_unlock(&tpool->requests_m);
            Close(connfd);
            return 0;
        }
        pthread_mutex_unlock(&tpool->requests_m);
        int rand_idx = -1;
        int fd_to_close = -1;
        for (int i=0;i<num_to_remove;i++)
        {
            rand_idx = rand() % (requests_waiting -i);
            //if -1 then remove failed, so no more requests to remove
            pthread_mutex_lock(&tpool->requests_m);
            fd_to_close = dequeue_index(tpool->requests_waiting, rand_idx);
            pthread_mutex_unlock(&tpool->requests_m);
            if (fd_to_close==-1)
                break;
            Close(fd_to_close);
        }

        return 1;
    }
    else if (sched == Dh)
    {
        pthread_mutex_lock(&tpool->requests_m);
        if (queueSize(tpool->requests_waiting) == 0)
        {
            pthread_mutex_unlock(&tpool->requests_m);
            Close(connfd);
            return 0;
        }
        pthread_mutex_unlock(&tpool->requests_m);
        Close(dequeue(tpool->requests_waiting));
        return 1;
    }
    return 0;
}
void ManageRequests(Tpool* tpool, int connfd , struct timeval current_time)
{
    schedAlg sched = tpool->schedalg;
    pthread_mutex_lock(&tpool->requests_m);
    int requests_waiting = queueSize(tpool->requests_waiting);
    int requests_handled = tpool->num_handled;
    pthread_mutex_unlock(&tpool->requests_m);
    if(requests_waiting + requests_handled == tpool->max_requests)
    {
        if(HandleOverload(tpool, connfd, sched)==0)
            return;
    }


    pthread_mutex_lock(&tpool->requests_m);
    enqueue(tpool->requests_waiting, connfd , current_time);
    pthread_cond_signal(&tpool->request_avail);
    pthread_mutex_unlock(&tpool->requests_m);
}