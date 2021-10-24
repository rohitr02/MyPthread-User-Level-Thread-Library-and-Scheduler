#include "mypthread.h"

// METHODS FOR THE QUEUE DATA STRUCTURE

threadNode* ThreadQueue = NULL;
static mypthread_t currentThread;
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
int isFinished(mypthread_t tid){
    threadNode* temp = ThreadQueue;
    while(temp != NULL){
        if(temp->thread->threadID == tid &&temp->thread->threadStatus == done)
            return 1;
        temp = temp->next;
    }
    return 0;
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
void destroyAll(){
    threadNode* prev = NULL;
    threadNode* cur = ThreadQueue;
    while(cur != NULL){
        if(cur->thread->threadStatus == destroy){
            prev->next = cur->next;
			cur = prev->next;
            free(prev->thread->threadContext.uc_stack.ss_sp);
            free(prev->thread);
            free(prev);
            continue;
        }
        prev = cur;
        cur = cur->next;
    }
}


// INITAILIZE ALL YOUR VARIABLES HERE
void freeThreads(void);
static void sched_stcf();


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

void resetTimer(){
    struct sigaction action;
    struct itimerval timer;

    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = &sched_stcf;
    sigaction(SIGPROF, &action, NULL);

    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = QUANTUM;

    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = QUANTUM;
    setitimer(ITIMER_PROF, &timer, NULL);
}

void createMainThread(){
    atexit(freeThreads);
    currentThread = threadIDs++;
    runningBlock = create_tcb(currentThread, false);
    
    resetTimer();
    getcontext(&(runningBlock->threadContext));
}

/* create a new thread */
int mypthread_create(mypthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
    if(!threadIDs) createMainThread();

    currentThread = threadIDs;
    tcb* threadBlock = create_tcb(currentThread, true);
    *thread = threadIDs++;
	threadBlock->elapsedTime = 0;
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
    if(!threadIDs) createMainThread();
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
        temp->thread->elapsedTime = 0;
        mutex->waitList = temp;
        sched_stcf();
    }
    return 0;
};

/* release the mutex lock */
int mypthread_mutex_unlock(mypthread_mutex_t *mutex) {
    threadNode* temp = mutex->waitList;
    while(temp != NULL){
        temp->thread->threadStatus = run;
        threadNode* k = temp;
        temp = temp->next;
        free(k);
    }
    mutex->waitList = NULL;
    mutex->lock = 0;
	return 0;
};


/* destroy the mutex */
int mypthread_mutex_destroy(mypthread_mutex_t *mutex) {
	return 0;
};

/* Preemptive SJF (STCF) scheduling algorithm */
static void sched_stcf() {
    destroyAll();
    signal(SIGPROF, SIG_IGN);
    
    tcb* prevThread = runningBlock;
    runningBlock = dequeue();
    if(runningBlock == NULL){
        runningBlock = prevThread;
        return;
    }
    prevThread->elapsedTime++;
    enqueue(prevThread);
    
    resetTimer();
    swapcontext(&(prevThread->threadContext), &(runningBlock->threadContext));
}

void freeThreads(void){
    while(ThreadQueue != NULL){
        threadNode* temp = ThreadQueue;
        ThreadQueue = ThreadQueue->next;
        free(temp->thread->threadContext.uc_stack.ss_sp);
        free(temp->thread);
        free(temp);
    }
    free(runningBlock);
}














































// // File:	mypthread.c

// // List all group member's name:
// // username of iLab:
// // iLab Server:

// #include "mypthread.h"

// // INITAILIZE ALL YOUR VARIABLES HERE
// // YOUR CODE HERE
// int threadID = 0;
// Node* currentlyRunningThread;
// Node* readyQueue = NULL;
// Node* sleepingQueue = NULL;
// Node* finishedQueue = NULL;
// Node* freeThreads = NULL;

// void destroyAll();
// static void sched_stcf();
// static void schedule();

// void resetTimer() {
// 	struct sigaction action;
//     struct itimerval timer;

//     memset(&action, 0, sizeof(struct sigaction));
//     action.sa_handler = &schedule;
//     sigaction(SIGPROF, &action, NULL);

//     timer.it_value.tv_sec = 0;
//     timer.it_value.tv_usec = QUANTUM;

//     timer.it_interval.tv_sec = 0;
//     timer.it_interval.tv_usec = QUANTUM;
//     setitimer(ITIMER_PROF, &timer, NULL);
// }

// void runMainThread() {
// 	atexit(destroyAll);
// 	threadControlBlock* mainThread = malloc(sizeof(threadControlBlock));
// 	mainThread->threadID = threadID++;
// 	mainThread->threadStatus = READY;
// 	mainThread->sleptByThreadID = -1;
// 	mainThread->quantumsElapsed = 0;
// 	mainThread->valuePtr = NULL;
// 	mainThread->returnValue = NULL;

// 	Node* newBlock = malloc(sizeof(Node));
// 	newBlock->next = NULL;
// 	newBlock->threadInfo = mainThread;

// 	currentlyRunningThread = newBlock;
// 	// currentlyRunningThread->next = NULL;
// 	// currentlyRunningThread->threadInfo = mainThread;
// 	resetTimer();
// 	getcontext(&(currentlyRunningThread->threadInfo->threadContext));
// }


// // adds blocks to the head of the reayQueue
// // when we search for the next job we'll traverse the ready queue and select accordingly, so insertion is not too important
// void addToReadyQueue(Node* thread) {
// 	if(thread->threadInfo->threadID == 0)
// 		return;
// 	if(readyQueue == NULL) readyQueue = thread;
// 	else {

// 		Node* currentThread = readyQueue;
// 		Node* prevThread = NULL;

// 		while(currentThread != NULL) {
// 			int currentThreadQuantum = currentThread->threadInfo->quantumsElapsed;
// 			int incomingThreadQuantum = thread->threadInfo->quantumsElapsed;

// 			if(incomingThreadQuantum < currentThreadQuantum) {
// 				if(prevThread == NULL) { // the first quantum is greater than the incoming thread's, make head new thread
// 					thread->next = readyQueue;
// 					readyQueue = thread;
// 				} else {
// 					prevThread->next = thread;
// 					thread->next = currentThread;
// 				}
// 				return;
// 			}

// 			prevThread = currentThread;
// 			currentThread = currentThread->next;
// 		}

// 		// incoming thread has the greatest quantum of all elements in queue
// 		prevThread->next = thread;
// 	}

// 	return;
// }


// /* create a new thread */
// int mypthread_create(mypthread_t * thread, pthread_attr_t * attr,
//                       void *(*function)(void*), void * arg) {
//        // create Thread Control Block
//        // create and initialize the context of this thread
//        // allocate space of stack for this thread to run
//        // after everything is all set, push this thread int
//        // YOUR CODE HERE

// 	  /*
// 	  each thread is a represented by a thread control block. In other words,
// 	  we're not thinking about individuals threads, but individuals control blocks

// 	  each control block, which in this case is synonymous with a thread, has the following 
// 	  attributes:

// 	  1) threadId
// 	  2) threadStatus
// 	  3) context
// 	  4)
// 	  // add to readyQueue
// 	  */

// 	 // TODO: most likely have to account for main thread somehow
// 	if(threadID == 0) runMainThread();
// 	threadControlBlock* newThread = malloc(sizeof(threadControlBlock));
// 	newThread->threadID = threadID;
// 	newThread->threadStatus = READY;
// 	newThread->sleptByThreadID = -1;
// 	newThread->quantumsElapsed = 0;
// 	newThread->valuePtr = NULL;
// 	newThread->returnValue = NULL;
// 	*thread = threadID++;
// 	Node* newBlock = malloc(sizeof(Node));
// 	newBlock->next = NULL;
// 	newBlock->threadInfo = newThread;
// 	addToReadyQueue(newBlock); // doesnt work yet, function doesnt work yet

// 	getcontext(&(newThread->threadContext));
// 	newThread->threadContext.uc_link = NULL;
// 	newThread->threadContext.uc_stack.ss_sp = malloc(SIGSTKSZ);
// 	newThread->threadContext.uc_stack.ss_size = SIGSTKSZ;
// 	makecontext(&(newThread->threadContext), (void*) function, 1, arg);
//     return 0;
// }




// // gets job with lowest QUANTUMelapsed, lowest elapsedQuantum indicates the thread that has ran the least
// Node* getNextReadyThread() {
// 	if(readyQueue == NULL) return NULL;
// 	else {
// 		// return the current head, lowest quantum
// 		Node* lowestQuantumThread = readyQueue;
// 		readyQueue = readyQueue->next;
// 		return lowestQuantumThread;
// 	}
// }


// /* give CPU possession to other user-level threads voluntarily: Done */
// int mypthread_yield() {

// 	// change thread state from Running to Ready
// 	// save context of this thread to its thread control block
// 	// wwitch from thread context to scheduler context
// 	schedule();
//     return 0;
// };

// void addToFinishedQueue(Node* currentThread) {
// 	if(finishedQueue == NULL) finishedQueue = currentThread;
// 	else {
// 		currentThread->next = finishedQueue;
// 		finishedQueue = currentThread;
// 	}
// }

// void removeFromFinishedQueue(mypthread_t threadID) {
// 	if(finishedQueue == NULL) return;

// 	Node* prevThread = NULL;
// 	Node* currentThread = finishedQueue;

// 	while(currentThread != NULL) {
// 		if(currentThread->threadInfo->threadID == threadID) {
// 			if(prevThread == NULL) finishedQueue = finishedQueue->next;
// 			else {
// 				prevThread->next = currentThread->next;
// 			}
// 			return;
// 		}

// 		prevThread = currentThread;
// 		currentThread = currentThread->next;
// 	}
// }

// void addToFree(Node* currentThread) {
// 	if(freeThreads == NULL) finishedQueue = currentThread;
// 	else {
// 		currentThread->next = freeThreads;
// 		freeThreads = currentThread;
// 	}
// }

// void addToSleepingQueue(Node* currentThread) {
// 	if(sleepingQueue == NULL) sleepingQueue = currentThread;
// 	else {
// 		currentThread->next = sleepingQueue;
// 		sleepingQueue = currentThread;
// 	}
// }

// Node* getFromFinishedQueue(mypthread_t threadID) {
// 	if(finishedQueue == NULL) return NULL;

// 	Node* currentThread = finishedQueue;
// 	while(currentThread != NULL) {
// 		if(currentThread->threadInfo->threadID == threadID) return currentThread;

// 		currentThread = currentThread->next;
// 	}

// 	return NULL;
// }

// Node* getFromSleepingQueue(mypthread_t threadID) {
// 	if(sleepingQueue == NULL) return NULL;

// 	Node* currentThread = sleepingQueue;
// 	while(currentThread != NULL) {
// 		if(currentThread->threadInfo->threadID == threadID) return currentThread;

// 		currentThread = currentThread->next;
// 	}

// 	return NULL;
// }

// Node* getFromReadyQueue(mypthread_t threadID) {
// 	if(readyQueue == NULL) return NULL;

// 	Node* currentThread = readyQueue;
// 	while(currentThread != NULL) {
// 		if(currentThread->threadInfo->threadID == threadID) return currentThread;

// 		currentThread = currentThread->next;
// 	}

// 	return NULL;
// }

// void wakeUpThread(mypthread_t threadID) {
// 	if(sleepingQueue == NULL) return;

// 	Node* currentThread = sleepingQueue;
// 	Node* prevThread = NULL;
// 	while(currentThread != NULL) {
// 		if(currentThread->threadInfo->sleptByThreadID == threadID) {
// 			if(prevThread == NULL) {
// 				sleepingQueue = sleepingQueue->next;
// 			} else {
// 				prevThread->next = currentThread->next;
// 			}
// 			currentThread->threadInfo->sleptByThreadID = -1;
// 			addToReadyQueue(currentThread);
// 			return;
// 		}

// 		prevThread = currentThread;
// 		currentThread = currentThread->next;
// 	}

// 	return;
// }


// /* terminate a thread */
// void mypthread_exit(void *value_ptr) {
// 	// Deallocated any dynamic memory created when starting this thread
// 	// YOUR CODE HERE
// 	if(currentlyRunningThread->threadInfo->valuePtr != NULL) {
// 		*currentlyRunningThread->threadInfo->valuePtr = value_ptr;
// 		currentlyRunningThread->threadInfo->threadStatus = FREE;
// 		addToFree(currentlyRunningThread);
// 	} else {
// 		currentlyRunningThread->threadInfo->returnValue = value_ptr;
// 		currentlyRunningThread->threadInfo->threadStatus = FINISHED;
// 		addToFinishedQueue(currentlyRunningThread);
// 	}

// 	wakeUpThread(currentlyRunningThread->threadInfo->threadID);
// 	schedule();
// };


// /* Wait for thread termination */
// int mypthread_join(mypthread_t thread, void **value_ptr) {

// 	// wait for a specific thread to terminate
// 	// de-allocate any dynamic memory created by the joining thread

// 	// YOUR CODE HERE
// 	Node* threadToJoin = getFromFinishedQueue(thread);
// 	if(threadToJoin != NULL) {
// 		if(value_ptr != NULL) {
// 			removeFromFinishedQueue(thread);
// 			threadToJoin->threadInfo->threadStatus = FREE;
// 			addToFree(threadToJoin);
// 			*value_ptr = threadToJoin->threadInfo->returnValue;
// 		}

// 		return EXIT_SUCCESS;
// 	}

// 	currentlyRunningThread->threadInfo->threadStatus = SLEEP;
// 	currentlyRunningThread->threadInfo->sleptByThreadID = thread;

// 	Node* newSleepingThread = malloc(sizeof(Node));
// 	newSleepingThread->next = NULL;
// 	newSleepingThread->threadInfo = currentlyRunningThread->threadInfo;
// 	addToSleepingQueue(newSleepingThread);

// 	threadToJoin = getFromReadyQueue(thread);
// 	if(threadToJoin != NULL) {
// 		threadToJoin->threadInfo->valuePtr = value_ptr;
// 	}
// 	threadToJoin = getFromSleepingQueue(thread);
// 	if(threadToJoin != NULL) {
// 		threadToJoin->threadInfo->valuePtr = value_ptr;
// 	}

// 	schedule();
// 	return EXIT_SUCCESS;
// }

// /* initialize the mutex lock */
// int mypthread_mutex_init(mypthread_mutex_t *mutex,
//                           const pthread_mutexattr_t *mutexattr) {
// 	//initialize data structures for this mutex
// 	if(threadID == 0) runMainThread();
// 	mutex->waitList = NULL;
// 	mutex->lock = 0;
//     return EXIT_SUCCESS;
// };

// /* aquire the mutex lock */
// int mypthread_mutex_lock(mypthread_mutex_t *mutex) {
//         // use the built-in test-and-set atomic function to test the mutex
//         // if the mutex is acquired successfully, enter the critical section
//         // if acquiring mutex fails, push current thread into block list and //
//         // context switch to the scheduler thread
// 		while(atomic_flag_test_and_set(&(mutex->lock))){
// 			currentlyRunningThread->threadInfo->threadStatus = WAITING;

// 			Node* newThreadWaiting = malloc(sizeof(Node));
// 			newThreadWaiting->next = mutex->waitList;
// 			newThreadWaiting->threadInfo = currentlyRunningThread->threadInfo;
// 			mutex->waitList = newThreadWaiting;
// 			schedule();
// 		}
//         return 0;
// };

// /* release the mutex lock */
// int mypthread_mutex_unlock(mypthread_mutex_t *mutex) {
// 	// Release mutex and make it available again.
// 	// Put threads in block list to run queue
// 	// so that they could compete for mutex later.

// 	// YOUR CODE HERE
// 	while(mutex->waitList != NULL) {
// 		addToReadyQueue(mutex->waitList);
// 		mutex->waitList = mutex->waitList->next;
// 	}
// 	mutex->waitList = NULL;
// 	mutex->lock = 0;
// 	return 0;
// };


// /* destroy the mutex */
// int mypthread_mutex_destroy(mypthread_mutex_t *mutex) {
// 	// Deallocate dynamic memory created in mypthread_mutex_init
// 	return 0;
// };

// /* scheduler: Done */
// static void schedule() {
// 	sched_stcf();
// }

// void cleanFree() {
// 	if(freeThreads == NULL) return;

// 	Node* prevThread = NULL;
// 	Node* currentThread = freeThreads;

// 	while(currentThread != NULL) {
// 		prevThread = currentThread;
// 		currentThread = currentThread->next;

// 		//free prev node
// 		free(prevThread->threadInfo->threadContext.uc_stack.ss_sp);
// 		free(prevThread->threadInfo);
// 		free(prevThread);
// 	}

// 	if(prevThread != NULL) {
// 		free(prevThread->threadInfo->threadContext.uc_stack.ss_sp);
// 		free(prevThread->threadInfo);
// 		free(prevThread);
// 	}

// 	freeThreads = NULL;
// }

// /* Preemptive SJF (STCF) scheduling algorithm: Done */
// static void sched_stcf() {
// 	// Your own implementation of STCF
// 	// (feel free to modify arguments and return types)
// 	cleanFree();
// 	signal(SIGPROF, SIG_IGN);
// 	Node* nextThreadToRun = getNextReadyThread();

// 	// there isn't another thread ready to run, so keep running the same one
// 	if(nextThreadToRun == NULL) {
// 		return ;
// 	}

// 	// at this point there IS another thread to yield to, so we increment the current thread's quantum (times it ran)
// 	currentlyRunningThread->threadInfo->quantumsElapsed++;

// 	// change the state of the thread that was currently running to ready and add it to the ready queue
// 	currentlyRunningThread->threadInfo->threadStatus = READY;
// 	addToReadyQueue(currentlyRunningThread);

// 	// change the new thread's state to running;
// 	nextThreadToRun->threadInfo->threadStatus = RUNNING;
// 	currentlyRunningThread = nextThreadToRun;
// 	resetTimer();
// 	swapcontext(&(currentlyRunningThread->threadInfo->threadContext), &(nextThreadToRun->threadInfo->threadContext));
// }

// void destroyAll() {
// 	if(readyQueue == NULL) return;

// 	Node* currentNode = readyQueue;
// 	while(currentNode != NULL) {
// 		Node* prevThread = currentNode;
// 		currentNode = currentNode->next;

// 		fprintf(stdout, "%d", prevThread->threadInfo->threadID);
// 		fflush(stdout);
// 		free(prevThread->threadInfo->threadContext.uc_stack.ss_sp);
// 		free(prevThread->threadInfo);
// 		free(prevThread);
// 	}
// 	if(currentlyRunningThread != NULL){
// 		free(currentlyRunningThread->threadInfo->threadContext.uc_stack.ss_sp);
// 		free(currentlyRunningThread->threadInfo);
// 		free(currentlyRunningThread);
// 	}
// }