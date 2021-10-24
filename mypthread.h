#ifndef MYTHREAD_T_H
#define MYTHREAD_T_H

#define _GNU_SOURCE

/* To use Linux pthread Library in Benchmark, you have to comment the USE_MYTHREAD macro */
#define USE_MYTHREAD 1

/* include lib header files that you need here: */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/time.h>
#include <stdatomic.h>
#include <string.h>


#define QUANTUM 15000

typedef unsigned int mypthread_t;

typedef enum {
    run, block, done, destroy
} mypthread_status;

typedef struct {
    mypthread_t threadID;
    ucontext_t threadContext;
    mypthread_status threadStatus;
    int elapsedTime;
    mypthread_t blockingThread;
    void** valuePtr;
    void* returnVal;
} tcb;


typedef struct threadNode{
	tcb* thread;
	struct threadNode* next;
}threadNode;


typedef struct mypthread_mutex_t {
	int lock;
    threadNode* waitList;
} mypthread_mutex_t;


/* Function Declarations: */
/* create a new thread */
int mypthread_create(mypthread_t * thread, pthread_attr_t * attr, void
    *(*function)(void*), void * arg);

/* give CPU pocession to other user level threads voluntarily */
int mypthread_yield();

/* terminate a thread */
void mypthread_exit(void *value_ptr);

/* wait for thread termination */
int mypthread_join(mypthread_t thread, void **value_ptr);

/* initial the mutex lock */
int mypthread_mutex_init(mypthread_mutex_t *mutex, const pthread_mutexattr_t
    *mutexattr);

/* aquire the mutex lock */
int mypthread_mutex_lock(mypthread_mutex_t *mutex);

/* release the mutex lock */
int mypthread_mutex_unlock(mypthread_mutex_t *mutex);

/* destroy the mutex */
int mypthread_mutex_destroy(mypthread_mutex_t *mutex);

void mainThreadAdd();

#ifdef USE_MYTHREAD
#define pthread_t mypthread_t
#define pthread_mutex_t mypthread_mutex_t
#define pthread_create mypthread_create
#define pthread_exit mypthread_exit
#define pthread_join mypthread_join
#define pthread_mutex_init mypthread_mutex_init
#define pthread_mutex_lock mypthread_mutex_lock
#define pthread_mutex_unlock mypthread_mutex_unlock
#define pthread_mutex_destroy mypthread_mutex_destroy
#endif

#endif






































// // File:    mypthread_t.h

// // List all group member's name:
// // username of iLab:
// // iLab Server:

// #ifndef MYTHREAD_T_H
// #define MYTHREAD_T_H

// #define _GNU_SOURCE

// /* To use Linux pthread Library in Benchmark, you have to comment the USE_MYTHREAD macro */
// #define USE_MYTHREAD 1

// #define QUANTUM 15000;

// /* include lib header files that you need here: */
// #include <unistd.h>
// #include <sys/syscall.h>
// #include <sys/types.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <sys/time.h>
// #include <signal.h>
// #include <string.h>
// #include <ucontext.h>
// #include <stdatomic.h>
// #include <ucontext.h>
// #include <signal.h>
// #include <stdbool.h>
// #include <sys/time.h>
// #include <stdatomic.h>
// #include <string.h>

// typedef unsigned int mypthread_t;

// typedef enum {
//     run, block, done, destroy
// }mypthread_status;
// typedef enum {
//     READY,
//     RUNNING,
//     FINISHED,
//     SLEEP,
//     FREE,
//     WAITING
// } threadStatus;

// typedef struct threadControlBlock {
//     /* add important states in a thread control block */
//     // thread Id
//     // thread status
//     // thread context
//     // thread stack
//     // thread priority
//     // And more ...
//     mypthread_t threadID;
//     threadStatus threadStatus;
//     ucontext_t threadContext;
//     int quantumsElapsed;
//     mypthread_t sleptByThreadID;
//     void* returnValue;
//     void** valuePtr;
// }threadControlBlock;

// /* define your data structures here: */
// // Feel free to add your own auxiliary data structures (linked list or queue etc...)

// typedef struct Node {
//     threadControlBlock* threadInfo;
//     struct Node* next;
// } Node;


// /* mutex struct definition */
// typedef struct mypthread_mutex_t {
//     /* add something here */
//     Node* waitList;
//     int lock;
// } mypthread_mutex_t;

// // typedef struct mutexNode {
// //       mypthread_mutex_t* mutex;
// //       struct mutexNode* next;
// // } mutexNode;

// // YOUR CODE HERE


// /* Function Declarations: */



// /* create a new thread */
// int mypthread_create(mypthread_t * thread, pthread_attr_t * attr, void
//     *(*function)(void*), void * arg);

// /* give CPU pocession to other user level threads voluntarily */
// int mypthread_yield();

// /* terminate a thread */
// void mypthread_exit(void *value_ptr);

// /* wait for thread termination */
// int mypthread_join(mypthread_t thread, void **value_ptr);

// /* initial the mutex lock */
// int mypthread_mutex_init(mypthread_mutex_t *mutex, const pthread_mutexattr_t
//     *mutexattr);

// /* aquire the mutex lock */
// int mypthread_mutex_lock(mypthread_mutex_t *mutex);

// /* release the mutex lock */
// int mypthread_mutex_unlock(mypthread_mutex_t *mutex);

// /* destroy the mutex */
// int mypthread_mutex_destroy(mypthread_mutex_t *mutex);

// #ifdef USE_MYTHREAD
// #define pthread_t mypthread_t
// #define pthread_mutex_t mypthread_mutex_t
// #define pthread_create mypthread_create
// #define pthread_exit mypthread_exit
// #define pthread_join mypthread_join
// #define pthread_mutex_init mypthread_mutex_init
// #define pthread_mutex_lock mypthread_mutex_lock
// #define pthread_mutex_unlock mypthread_mutex_unlock
// #define pthread_mutex_destroy mypthread_mutex_destroy
// #endif

// #endif