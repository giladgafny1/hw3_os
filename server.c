#include "segel.h"
#include "request.h"
#include <pthread.h>
#include "queue.h"
#include "thread_pool.h"


// 
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// HW3: Parse the new arguments too
void getargs(int *port, int  *thread_num, int* max_requests, char* schedalg , int argc, char *argv[])
{
    if (argc < 5) {
	fprintf(stderr, "Usage: %s <port>\n", argv[0]);
	exit(1);
    }
    *port = atoi(argv[1]);
    *thread_num = atoi(argv[2]);
    *max_requests = atoi(argv[3]);
    strcpy(schedalg, argv[4]);
}
//./server port thread queuesize schedlg


int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen, thread_num , max_requests;
    char schedalg[7];
    struct sockaddr_in clientaddr;

    getargs(&port,&thread_num, &max_requests , schedalg, argc, argv);

    Tpool* tpool = CreateTpool(thread_num, max_requests, schedalg);
    // 
    // HW3: Create some threads...
    //

    listenfd = Open_listenfd(port);
    while (1) {
	clientlen = sizeof(clientaddr);
	connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

	// 
	// HW3: In general, don't handle the request in the main thread.
	// Save the relevant info in a buffer and have one of the worker threads 
	// do the work. 
	// 
	ManageRequests(tpool, connfd);
    }

}


    


 
