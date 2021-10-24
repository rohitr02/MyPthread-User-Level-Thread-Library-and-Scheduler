#include "mypthread.h"

threadNode* ThreadQueue = NULL;
static int threadIDs = 0;
static tcb* runningBlock;


void enqueue(tcb* item){
    threadNode* temp = malloc(sizeof(threadNode));
    temp->next = NULL;
    temp->thread = item;
    temp->thread->elapsedTime = item->elapsedTime;
    
    threadNode* cur = ThreadQueue;
	int elapsedTime = item->elapsedTime;
    
    if(cur == NULL){
    	ThreadQueue = temp;
        return;
    }

    if(cur->thread->elapsedTime > elapsedTime){
        temp->next = cur;
        ThreadQueue = temp;
        return;
    }

    threadNode* prev = cur;
    threadNode* ptr = cur->next;
    while(ptr != NULL && ptr->thread->elapsedTime < elapsedTime){
        prev = ptr;
        ptr = ptr->next;
    }
    temp->next = ptr;
    prev->next = temp;
    ThreadQueue = cur;
}
tcb* dequeue(){
    threadNode* cur = ThreadQueue;
    if(cur == NULL) return NULL;
    if(cur->thread->threadStatus == run){
        ThreadQueue = cur->next;
        tcb* block = cur->thread;
        free(cur);
        return block;
    }
    threadNode* prev = cur;
    threadNode* ptr = cur->next;
    while(ptr != NULL && ptr->thread->threadStatus != run){
        prev = ptr;
        ptr = ptr->next;
    }
    if(ptr == NULL) return NULL;
    prev->next = ptr->next;
    tcb* block = ptr->thread;
    free(ptr);
    return block;
}
void unblockThread(mypthread_t tid){
    threadNode *temp = ThreadQueue;
    while(temp != NULL){
        if(temp->thread->blockingThread == tid && temp->thread->threadStatus == block){
            temp->thread->blockingThread = -1;
            temp->thread->threadStatus = run;
        }
        temp = temp->next;
    }
}
bool isFinished(mypthread_t tid){
    threadNode* temp = ThreadQueue;
    while(temp != NULL){
        if(temp->thread->threadID == tid && temp->thread->threadStatus == done)
            return true;
        temp = temp->next;
    }
    return false;
}
tcb* getTCB(mypthread_t tid){
    threadNode* temp = ThreadQueue;
    while(temp != NULL){
        if(temp->thread->threadID == tid)
            return temp->thread;
        temp = temp->next;
    }
    return NULL;
}
void freeNode(threadNode* t){
    if (t == NULL) return;
    free(t->thread->threadContext.uc_stack.ss_sp);
    free(t->thread);
    free(t);
}
void destroyAll(){
    threadNode* prev = NULL;
    threadNode* cur = ThreadQueue;
    while(cur != NULL){
        if(cur->thread->threadStatus == destroy){
            prev->next = cur->next;
			cur = prev->next;
            freeNode(prev);
            continue;
        }
        prev = cur;
        cur = cur->next;
    }
}
void freeThreadQueue(void){
    while(ThreadQueue != NULL){
        threadNode* temp = ThreadQueue;
        ThreadQueue = ThreadQueue->next;
        freeNode(temp);
    }
    free(runningBlock);
}


tcb* create_tcb(mypthread_t tid, bool createContext){
    tcb* thread = malloc(sizeof(tcb));
    thread->threadID = tid;
    thread->blockingThread = -1;
    thread->threadStatus = run;
    thread->elapsedTime = 0;
    thread->valuePtr = NULL;
    thread->returnVal = NULL;
    if(createContext){
        getcontext(&(thread->threadContext));
        thread->threadContext.uc_link = NULL;
        thread->threadContext.uc_stack.ss_sp = malloc(SIGSTKSZ);
        thread->threadContext.uc_stack.ss_size = SIGSTKSZ;
    }
    return thread;
}

void setupAction(){
    struct sigaction action;
    action.sa_handler = &sched_stcf;
    sigaction(SIGPROF, &action, NULL);
}

void setupTimer(){
    struct itimerval timer;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = QUANTUM;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = QUANTUM;
    setitimer(ITIMER_PROF, &timer, NULL);
}

void createMainThread(){
    atexit(freeThreadQueue);
    runningBlock = create_tcb(threadIDs++, false);
    setupAction();
    setupTimer();
    getcontext(&(runningBlock->threadContext));
}

/* create a new thread */
int mypthread_create(mypthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
    if(!threadIDs) createMainThread();
    tcb* threadBlock = create_tcb(threadIDs++, true);
    *thread = threadBlock->threadID;
    enqueue(threadBlock);
    makecontext(&(threadBlock->threadContext), (void*) function, 1, arg); 
    return 0;
};  

/* give CPU possession to other user-level threads voluntarily */
int mypthread_yield() {
    sched_stcf();
    return 0;
};

/* terminate a thread */
void mypthread_exit(void *value_ptr) {
    runningBlock->threadStatus = done;
    if(runningBlock->valuePtr != NULL){
        *runningBlock->valuePtr = value_ptr;
        runningBlock->threadStatus = destroy;
    }
    else runningBlock->returnVal = value_ptr;
    unblockThread(runningBlock->threadID);
    sched_stcf();
};


/* Wait for thread termination */
int mypthread_join(mypthread_t thread, void **value_ptr) {
    if(isFinished(thread)){
        tcb* block = getTCB(thread);
        if(value_ptr != NULL){
            block->threadStatus = destroy;
            *value_ptr = block->returnVal;
        }
        return 0;
    }
    runningBlock->threadStatus = block;
    runningBlock->blockingThread = thread;
    tcb* block = getTCB(thread);
    if(block != NULL)
        block->valuePtr = value_ptr;
    sched_stcf();
    return 0;
};

/* initialize the mutex lock */
int mypthread_mutex_init(mypthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) {
	//initialize data structures for this mutex
    if(threadIDs==0) createMainThread();
    mutex->waitList = NULL;
    mutex->lock = 0;
    return 0;
};

/* aquire the mutex lock */
int mypthread_mutex_lock(mypthread_mutex_t *mutex) {
    while(atomic_flag_test_and_set(&(mutex->lock))){
        runningBlock->threadStatus = block;
        threadNode* temp = malloc(sizeof(threadNode));
        temp->next = mutex->waitList;
        temp->thread = runningBlock;
        mutex->waitList = temp;
        sched_stcf();
    }
    return 0;
};

/* release the mutex lock */
int mypthread_mutex_unlock(mypthread_mutex_t *mutex) {
    threadNode* cur = mutex->waitList;
    while(cur != NULL){
        cur->thread->threadStatus = run;
        threadNode* temp = cur;
        cur = cur->next;
        free(temp);
    }
    mutex->waitList = NULL;
    mutex->lock = 0;
	return 0;
};


/* destroy the mutex */
int mypthread_mutex_destroy(mypthread_mutex_t *mutex) {
    threadNode* cur = mutex->waitList;
    while(cur != NULL){
        cur->thread->threadStatus = run;
        threadNode* temp = cur;
        cur = cur->next;
        free(temp);
    }
	return 0;
};

/* Preemptive SJF (STCF) scheduling algorithm */
static void sched_stcf() {
    destroyAll();
    signal(SIGPROF, SIG_IGN);
    
    tcb* prevThread = runningBlock;
    runningBlock = dequeue();
    if(runningBlock != NULL){
        prevThread->elapsedTime++;
        enqueue(prevThread);
        setupAction();
        setupTimer();
        swapcontext(&(prevThread->threadContext), &(runningBlock->threadContext));
    }
    else runningBlock = prevThread;
}