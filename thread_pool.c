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

static void *tpool_worker(void* arg)
{
    Thread_args* thread_args = arg;
    Tpool* tpool = thread_args->tpool;
    int index_p = thread_args->thread_index;

    while(1)
    {
        pthread_mutex_lock(&tpool->requests_m);
        while (tpool->requests_waiting->size == 0)
        {
            pthread_cond_wait(&(tpool->request_avail) , &(tpool->requests_m));
        }
        int request_fd = dequeue(tpool->requests_waiting);
        enqueue(tpool->requests_handled,request_fd);
        pthread_mutex_unlock(&tpool->requests_m);

        requestHandle(request_fd);


        pthread_cond_signal(&tpool->request_avail);
        pthread_mutex_unlock(&tpool->requests_m);
    }
}

void ManageRequests(Tpool* tpool, int connfd)
{
    char* sched = tpool->schedalg;
    pthread_mutex_lock(&tpool->requests_m);
    int requests_waiting = queueSize(tpool->requests_waiting);
    int requests_handled = queueSize(tpool->requests_handled);
    if(requests_waiting + requests_handled == tpool->max_requests)
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
        else if (strcmp(sched, "random")==0)
        {
            int num_to_remove = (requests_waiting + requests_handled)*0.3;
            if (requests_waiting == 0)
            {
                pthread_mutex_unlock(&tpool->requests_m);
                close(connfd);
                return;
            }
            int rand_idx = -1;
            int remove_result = 0;
            for (int i=0;i<num_to_remove;i++)
            {
                rand_idx = rand() % requests_waiting;
                //if -1 then remove failed
                if (dequeue_index(tpool->requests_waiting, rand_idx)==-1)
                    break;
            }
        }
        else if (strcmp(sched, "dh")==0)
        {
            if (requests_waiting == 0)
            {
                pthread_mutex_unlock(&tpool->requests_m);
                close(connfd);
                return;
            }
            dequeue(tpool->requests_waiting);
        }
    }
    enqueue(tpool->requests_waiting, connfd);
    pthread_cond_signal(&tpool->request_avail);
    pthread_mutex_unlock(&tpool->requests_m);
}