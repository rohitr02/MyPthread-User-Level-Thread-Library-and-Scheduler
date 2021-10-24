// File:	mypthread.c

// List all group member's name:
// username of iLab:
// iLab Server:

#include "mypthread.h"

// INITAILIZE ALL YOUR VARIABLES HERE
// YOUR CODE HERE

unsigned long int TimeQuantum = 20000;

// tcb* create_tcb(void){
// 	mypthread_t nextID;
// 	tcb* new = malloc(sizeof(tcb));
// 	new->threadID = nextID++;
// 	return new;
// }

tcb* destory_tcb(tcb* TCB){
	free(TCB->threadContext->uc_stack.ss_sp);
	free(TCB);
}


threadQueue* createQueue(void){
	threadQueue* newQueue = malloc(sizeof(threadQueue));
	newQueue->head = NULL;
	// newQueue->size = 0;
	return newQueue;
}

void destroyQueue(threadQueue* q){
	threadNode* prev = NULL;
	threadNode* cur = q->head;

	while(cur != NULL){
		prev = cur;
		cur = cur->next;

		destory_tcb(prev->threadTCB);
		free(prev);
	}
	
	free(q);
}

void removeDestroyStateNodes(threadQueue* q){
	threadNode* prev = q->head;
	threadNode* cur = q->head->next;

	while(cur != NULL){
		
		if(cur->threadTCB->threadStatus == destroy){
			prev->next = cur->next;
			destory_tcb(cur->threadTCB);
			free(cur);
			cur = prev->next;
			continue;
		}
		prev = cur;
		cur = cur->next;
	}
	prev = q->head;
	if(prev->threadTCB->threadStatus == destroy){
		q->head = prev->next;
		destory_tcb(prev->threadTCB);
		free(prev);
	}
}

void enqueue(threadQueue* q, tcb* item, int quantum){
	if (q == NULL || item == NULL) return;
	
	// q->size++;
	
	threadNode* newNode = (threadNode*) malloc(sizeof(threadNode));
	newNode->threadTCB = item;
	newNode->next = NULL;
	newNode->quantum = quantum;

	if(q->head == NULL){
		q->head = newNode;
		return;
	}


	if(q->head->quantum > quantum){
		newNode->next = q->head;
		q->head = newNode;
		return;
	}

	threadNode* prev = q->head;
	threadNode* cur = q->head->next;
	while(cur != NULL && cur->quantum < quantum){
		prev = cur;
		cur = cur->next;
	}
	
	newNode->next = cur;
	prev->next = newNode;
}

tcb* dequeue(threadQueue* q){
	if (q == NULL || q->head == NULL) return NULL;
	
	if(q->head->threadTCB->threadStatus == run){
		threadNode* temp = q->head;
		q->head = q->head->next;
		// q->size -= 1;

		tcb* output = temp->threadTCB;
		free(temp);
		return output;
	}

	threadNode* prev = q->head;
	threadNode* cur = q->head->next;
	while(cur != NULL && cur->threadTCB->threadStatus != run){
		prev = cur;
		cur = cur->next;
	}
	if(cur == NULL) return NULL;

	prev->next = cur->next;
	tcb* output = cur->threadTCB;
	free(cur);
	// q->size -= 1;
	return output;
}

tcb* removeID(threadQueue* q, mypthread_t id){
	if (q == NULL || q->head == NULL) return NULL;

	threadNode* prev = NULL;
	threadNode* cur = q->head;
	tcb* output = NULL;

	while(cur != NULL){
		if(cur->threadTCB->threadID == id){
			if(prev == NULL)
				q->head = cur->next;
			else
				prev->next = cur->next;

			output = cur->threadTCB;
			free(cur);
			return output;
		}
		prev = cur;
		cur = cur->next;
	}
	return output;
}

tcb* getTCB(threadQueue* q, mypthread_t id){
	if (q == NULL || q->head == NULL) return NULL;

	threadNode* cur = q->head;
	while(cur != NULL){
		if(cur->threadTCB->threadID == id)
			return cur->threadTCB;
		cur = cur->next;
	}
	return NULL;
}

bool isDone(threadQueue* q, mypthread_t id){
	if (q == NULL || q->head == NULL) return NULL;

	threadNode* cur = q->head;
	while(cur != NULL){
		if(cur->threadTCB->threadID == id && cur->threadTCB->threadStatus == done)
			return true;
	}
	return false;
}

void updateBlocked(threadQueue* q, mypthread_t blocked){
	if (q == NULL || q->head == NULL) return ;

	threadNode* cur = q->head;
	while(cur != NULL){
		if(cur->threadTCB->blocked == blocked && cur->threadTCB->threadStatus == block){
			cur->threadTCB->blocked = -1;
			cur->threadTCB->threadStatus = run;
		}
		cur = cur->next;
	}
}

static void sched_stcf();
static void schedule();



// unsigned int Stack_Size = 1048576; // One megabyte

unsigned int threadIDs = 0;
threadQueue* threadQ = NULL;
tcb* runningThread = NULL;
// ucontext_t mainContext;

void destroyEverything(){
	destroyQueue(threadQ);
	// free(runningThread);
}


/* create a new thread */
int mypthread_create(mypthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
	if(!threadIDs){
		atexit(destroyEverything);
		tcb* mainThread = malloc(sizeof(tcb));
		mainThread->threadID = threadIDs++;
		mainThread->threadStatus = run;
		mainThread->threadPriority = 0;
		mainThread->blocked = -1;
		mainThread->quantum = 0;
		mainThread->returnValue = NULL;
		mainThread->valuePtr = NULL;


		//NEED TO FIGURE OUT WHAT THIS PART DOES AND REWRITE IT//
		struct sigaction action;
		struct itimerval timer;

		memset(&action, 0, sizeof(struct sigaction));
		action.sa_handler = &schedule;
		sigaction(SIGPROF, &action, NULL);

		timer.it_value.tv_sec = 0;
		timer.it_value.tv_usec = TimeQuantum;

		timer.it_interval.tv_sec = 0;
		timer.it_interval.tv_usec = TimeQuantum;

		setitimer(ITIMER_PROF, &timer, NULL);
		getcontext(runningThread->threadContext);
	}

	tcb* newThread = malloc(sizeof(tcb));
	newThread->threadID = threadIDs++;
	newThread->threadStatus = run;
	newThread->threadPriority = 0;
	newThread->blocked = -1;
	newThread->quantum = 0;
	newThread->valuePtr = NULL;
	newThread->returnValue = NULL;
	enqueue(threadQ, newThread, 0);
	
	getcontext(newThread->threadContext);
	newThread->threadContext->uc_link = NULL;
	newThread->threadContext->uc_stack.ss_sp = malloc(SIGSTKSZ);
	newThread->threadContext->uc_stack.ss_size = SIGSTKSZ;
	makecontext(newThread->threadContext, (void*) function, 1, arg);
};

/* give CPU possession to other user-level threads voluntarily */
int mypthread_yield() {

	// change thread state from Running to Ready
	// save context of this thread to its thread control block
	// wwitch from thread context to scheduler context

	// YOUR CODE HERE
	schedule();
};

/* terminate a thread */
void mypthread_exit(void *value_ptr) {
	// Deallocated any dynamic memory created when starting this thread
	// YOUR CODE HERE
	runningThread->threadStatus = done;
	if(runningThread->valuePtr == NULL){
		*runningThread->valuePtr = value_ptr;
		runningThread->threadStatus = destroy;
	}
	else runningThread->returnValue = value_ptr;
	updateBlocked(threadQ, runningThread->threadID);
	schedule();
};


/* Wait for thread termination */
int mypthread_join(mypthread_t thread, void **value_ptr) {

	// wait for a specific thread to terminate
	// de-allocate any dynamic memory created by the joining thread

	// YOUR CODE HERE
	if(isDone(threadQ, thread)){
		tcb* finTCB = getTCB(threadQ, thread);
		if(value_ptr != NULL){
			finTCB->threadStatus = destroy;
			*value_ptr = finTCB->returnValue;
		}
		return 0;
	}

	runningThread->threadStatus = block;
	runningThread->blocked = thread;
	tcb* finTCB = getTCB(threadQ, thread);
	finTCB->valuePtr = value_ptr;
	schedule();
};

/* initialize the mutex lock */
int mypthread_mutex_init(mypthread_mutex_t *mutex,
                          const pthread_mutexattr_t *mutexattr) {
	//initialize data structures for this mutex

	// YOUR CODE HERE
	if(!threadIDs) {
		atexit(destroyEverything);
		tcb* mainThread = malloc(sizeof(tcb));
		mainThread->threadID = threadIDs++;
		mainThread->threadStatus = run;
		mainThread->threadPriority = 0;
		mainThread->blocked = -1;
		mainThread->quantum = 0;
		mainThread->returnValue = NULL;
		mainThread->valuePtr = NULL;


		//NEED TO FIGURE OUT WHAT THIS PART DOES AND REWRITE IT//
		struct sigaction action;
		struct itimerval timer;

		memset(&action, 0, sizeof(struct sigaction));
		action.sa_handler = &schedule;
		sigaction(SIGPROF, &action, NULL);

		timer.it_value.tv_sec = 0;
		timer.it_value.tv_usec = TimeQuantum;

		timer.it_interval.tv_sec = 0;
		timer.it_interval.tv_usec = TimeQuantum;

		setitimer(ITIMER_PROF, &timer, NULL);
		getcontext(runningThread->threadContext);
	}
	mutex->waitList = NULL;
	mutex->lock = 0;
	return 0;
};

/* aquire the mutex lock */
int mypthread_mutex_lock(mypthread_mutex_t *mutex) {
        // use the built-in test-and-set atomic function to test the mutex
        // if the mutex is acquired successfully, enter the critical section
        // if acquiring mutex fails, push current thread into block list and //
        // context switch to the scheduler thread

        // YOUR CODE HERE

		while(atomic_flag_test_and_set(&(mutex->lock))){
			runningThread->threadStatus = block;
			threadNode* t = malloc(sizeof(threadNode));
			t->next = mutex->waitList;
			t->threadTCB = runningThread;
			mutex->waitList = t;
			schedule();
		}
        return 0;
};

/* release the mutex lock */
int mypthread_mutex_unlock(mypthread_mutex_t *mutex) {
	// Release mutex and make it available again.
	// Put threads in block list to run queue
	// so that they could compete for mutex later.

	// YOUR CODE HERE
	threadNode* cur = mutex->waitList;
	while(cur != NULL){
		cur->threadTCB->threadStatus = run;
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
	// Deallocate dynamic memory created in mypthread_mutex_init
	return 0;
};

/* scheduler */
static void schedule() {
	sched_stcf();
}

/* Preemptive SJF (STCF) scheduling algorithm */
static void sched_stcf() {
	// Your own implementation of STCF
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
	removeDestroyStateNodes(threadQ);
	signal(SIGPROF, SIG_IGN);

	tcb* prev = runningThread;
	runningThread = dequeue(threadQ);
	if(runningThread == NULL){
		runningThread = prev;
		return;
	}

	prev->quantum++;
	enqueue(threadQ, prev, prev->quantum);


	struct sigaction action;
    struct itimerval timer;

    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = &schedule;
    sigaction(SIGPROF, &action, NULL);

    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = TimeQuantum;

    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = TimeQuantum;
    setitimer(ITIMER_PROF, &timer, NULL);

	swapcontext(prev->threadContext, runningThread->threadContext);
}