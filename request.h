#ifndef __REQUEST_H__
#include "thread_pool.h"

void requestHandle(int fd , Stats* thread_stats, struct timeval arrival , struct timeval dispatch);

#endif
