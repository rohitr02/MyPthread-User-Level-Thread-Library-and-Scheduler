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
void enqueue(tcb* item);
tcb* dequeue();
void unblockThread(mypthread_t tid);
bool isFinished(mypthread_t tid);
tcb* getTCB(mypthread_t tid);
void freeNode(threadNode* t);
void destroyAll();
void freeThreads(void);
static void sched_stcf();
tcb* create_tcb(mypthread_t tid, bool createContext);
void setupAction();
void setupTimer();
void createMainThread();
void freeThreads(void);

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