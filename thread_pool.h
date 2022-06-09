//
// Created by student on 6/8/22.
//

#ifndef HW3_OS_MYWORK_THREAD_POOL_H
#define HW3_OS_MYWORK_THREAD_POOL_H
#include "queue.h"
#include <pthread.h>

typedef struct tpool_t
{
    pthread_mutex_t requests_m;
    pthread_cond_t request_avail;
    pthread_cond_t
    Queue aviable_threads;

}Tpool;

Tpool* CreateTpool(int num_of_threads);

void DestroyTpool(Tpool* tpool);



#endif //HW3_OS_MYWORK_THREAD_POOL_H
