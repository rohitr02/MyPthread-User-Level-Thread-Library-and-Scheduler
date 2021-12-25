#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "../mypthread.h"

// /* A scratch program template on which to call and
//  * test mypthread library functions as you implement
//  * them.
//  *
//  * You can modify and use this program as much as possible.
//  * This will not be graded.
//  */

int main(int argc, char **argv){
    return 1;
}


// void enqueue(threadQueue* q, tcb* item, int quantum){
//     if (q == NULL || item == NULL) return;

//     threadNode* newNode = (threadNode*) malloc(sizeof(threadNode));
// 	newNode->block = item;
// 	newNode->next = NULL;
// 	newNode->quantum = quantum;

//     if(q->head == NULL){
// 		q->head = newNode;
// 		return;
// 	}

//     if(q->head->quantum > quantum){
// 		newNode->next = q->head;
// 		q->head = newNode;
// 		return;
// 	}

//     threadNode* prev = q->head;
// 	threadNode* cur = q->head->next;
// 	while(cur != NULL && cur->quantum < quantum){
// 		prev = cur;
// 		cur = cur->next;
// 	}
	
// 	newNode->next = cur;
// 	prev->next = newNode;
// }
// void updateQueueRunnable(threadQueue* q, mypthread_t waiting){
//     threadNode* temp = q->head;
//     while(temp != NULL){
//         if(temp->block->waiting == waiting && temp->block->status == block){
//             temp->block->waiting = -1;
//             temp->block->status = run;
//         }
//         temp = temp->next;
//     }
//     return;
// }
// tcb* dequeue(threadQueue* q){
//     if(q == NULL || q->head == NULL) return NULL;

//     threadNode* head = q->head;
//     if(head->block->status == run){
//         q->head = head->next;
// 		tcb* output = head->block;
// 		free(head);
// 		return output;
// 	}
//     threadNode* prev = head;
//     threadNode* cur = head->next;
//     while(cur != NULL && cur->block->status != run){
//         prev = cur;
//         cur = cur->next;
//     }

//     if(cur == NULL) return NULL;
//     prev->next = cur->next;
//     tcb* block = cur->block;
//     free(cur);
//     return block;
// }
// int checkIfFinished(threadQueue* q, mypthread_t waiting){
//     threadNode* temp = q->head;
//     while(temp != NULL){
//         if(temp->block->tid == waiting && temp->block->status == done)
//             return 1;
//         temp = temp->next;
//     }
//     return 0;
// }
// tcb* getBlock(threadQueue* q, mypthread_t tid){
//     threadNode* temp = q->head;
//     while(temp != NULL){
//         if(temp->block->tid == tid)
//             return temp->block;
//         temp = temp->next;
//     }
//     return NULL;
// }
// void cleanup(threadQueue* q){
// 	if(q == NULL || q->head == NULL) return;
//     threadNode* prev = q->head;
//     threadNode* cur = prev->next;
//     while(cur != NULL){
//         if(cur->block->status == destroy){
//             prev->next = cur->next;
//             free(cur->block->context.uc_stack.ss_sp);
//             free(cur->block);
//             free(cur);
//             cur = prev->next;
//             continue;
//         }
//         prev = cur;
//         cur = cur->next;
//     }
//     prev = q->head;
//     if(prev->block->status == destroy){
//         q->head = prev->next;
//         free(prev->block->context.uc_stack.ss_sp);
//         free(prev->block);
//         free(prev);
//     }
// }

// void printQueue(threadQueue* q){
//     threadNode* temp = q->head;
//     while(temp != NULL){
//         printf("Item ID: %d \t", temp->block->tid);
//         temp = temp->next;
//     }
// 	printf("\nDONE\n");
// }













// // typedef struct PQueue{
// //     struct PQueue* next;
// //     tcb* control;
// //     int quantum;
// // }PQueue;

// // void enqueue(PQueue** queue, tcb* block, int quantum){
// //     if(queue == NULL)
// //         return;
// //     PQueue * temp = malloc(sizeof(PQueue));
// //     temp->next = NULL;
// //     temp->control = block;
// //     temp->quantum = quantum;
    
// //     PQueue* front = *queue;
    
// //     if(front == NULL){
// //         *queue = temp;
// //         return;
// //     }

// //     if(front->quantum > quantum){
// //         temp->next = front;
// //         *queue = temp;
// //         return;
// //     }

// //     PQueue* trail = front;
// //     PQueue* lead = front->next;
// //     while(lead != NULL && lead->quantum < quantum){
// //         trail = lead;
// //         lead = lead->next;
// //     }
// //     temp->next = lead;
// //     trail->next = temp;
// //     *queue = front;
// // }
// // tcb* dequeue(PQueue** queue){
// //     PQueue* front = *queue;
// //     if(front == NULL)
// //         return NULL;
// //     if(front->control->status == run){
// //         *queue = front->next;
// //         tcb* block = front->control;
// //         free(front);
// //         return block;
// //     }
// //     PQueue* trail = front;
// //     PQueue* lead = front->next;
// //     while(lead != NULL && lead->control->status !=run){
// //         trail = lead;
// //         lead = lead->next;
// //     }
// //     if(lead == NULL)
// //         return NULL;
// //     trail->next = lead->next;
// //     tcb* block = lead->control;
// //     free(lead);
// //     return block;
// // }
// // void updateQueueRunnable(PQueue** queue, mypthread_t waiting){
// //     PQueue *temp = *queue;
// //     while(temp != NULL){
// //         if(temp->control->waiting == waiting && temp->control->status == block){
// //             temp->control->waiting = -1;
// //             temp->control->status = run;
// //         }
// //         temp = temp->next;
// //     }
// //     return;
// // }
// // int checkIfFinished(PQueue** queue, mypthread_t waiting){
// //     PQueue* temp = *queue;
// //     while(temp != NULL){
// //         if(temp->control->tid == waiting &&temp->control->status == done)
// //             return 1;
// //         temp = temp->next;
// //     }
// //     return 0;
// // }
// // tcb* getBlock(PQueue** queue, mypthread_t tid){
// //     PQueue* temp = *queue;
// //     while(temp != NULL){
// //         if(temp->control->tid == tid)
// //             return temp->control;
// //         temp = temp->next;

// //     }
// //     return NULL;
// // }

// // void cleanup(PQueue** queue){
// //     PQueue* trail = *queue;
// //     PQueue* lead = trail->next;
// //     while(lead != NULL){
// //         if(lead->control->status == destroy){
// //             trail->next = lead->next;
// //             free(lead->control->context.uc_stack.ss_sp);
// //             free(lead->control);
// //             free(lead);
// //             lead = trail->next;
// //             continue;
// //         }
// //         trail = lead;
// //         lead = lead->next;
// //     }
// //     trail = *queue;
// //     if(trail->control->status == destroy){
// //         *queue = trail->next;
// //         free(trail->control->context.uc_stack.ss_sp);
// //         free(trail->control);
// //         free(trail);
// //     }
// // }

// // void printQueue(PQueue** q){
// //     PQueue* temp = *q;
// //     while(temp != NULL){
// //         printf("Item ID: %d \t", temp->control->tid);
// //         temp = temp->next;
// //     }
// // 	printf("\nDONE\n");
// // }




















// threadQueue* test;
// // PQueue* testP;

// // void *func(void *arg)
// // {
// // 	printf("thread 0 - arg = %ld\n", (long)arg);
// // 	return (void *)0;
// // }

// int main(int argc, char **argv) {

// 	// /* Implement HERE */
// 	// printf("Test0\n");
// 	// mypthread_t id;
// 	// mypthread_create(&id, NULL, &func, NULL);

// 	// printf("Test1\n");
// 	// mypthread_t id2;
// 	// mypthread_create(&id2, NULL, &func, NULL);

// 	// printf("Test2\n");
// 	// mypthread_t id3;
// 	// mypthread_create(&id3, NULL, &func, NULL);

// 	// mypthread_exit(id);
// 	// mypthread_join(id2, NULL);
// 	// mypthread_join(id3, NULL);

// 	test = malloc(sizeof(threadQueue));
// 	tcb* item = malloc(sizeof(tcb));
// 	item->tid = 22;
// 	tcb* item2 = malloc(sizeof(tcb));
// 	item2->tid = 52;
// 	tcb* item3 = malloc(sizeof(tcb));
// 	item3->tid = 62;
// 	tcb* item4 = malloc(sizeof(tcb));
// 	item4->tid = 212;
// 	tcb* item5 = malloc(sizeof(tcb));
// 	item5->tid = 226;
// 	// item->status = run;
// 	// item2->status = run;
// 	// item3->status = run;
// 	// item4->status = run;
// 	// item5->status = run;
// 	item->status = destroy;
// 	item2->status = destroy;
// 	item3->status = destroy;
// 	item4->status = destroy;
// 	item5->status = destroy;


// 	// printQueue(test);
// 	// enqueue(test, item, 0);
// 	// printQueue(test);
// 	// enqueue(test, item2, 0);
// 	// printQueue(test);
// 	// enqueue(test, item3, 0);
// 	// printQueue(test);
// 	// // free(dequeue(test));
// 	// // printQueue(test);
// 	// enqueue(test, item4, 0);
// 	// printQueue(test);
// 	// enqueue(test, item5, 0);
// 	// printQueue(test);
// 	// free(dequeue(test));
// 	// printQueue(test);
// 	cleanup(test);
// 	printQueue(test);
// // free(dequeue(test));
// // 	printQueue(test);
// // free(dequeue(test));
// // 	printQueue(test);
// // free(dequeue(test));
// // 	printQueue(test);
// // 	free(dequeue(test));
// // 	printQueue(test);
// // 	free(test);




// // printQueue(&testP);
// // 	enqueue(&testP, item, 0);
// // 	printQueue(&testP);
// // 	enqueue(&testP, item2, 0);
// // 	printQueue(&testP);
// // 	enqueue(&testP, item3, 0);
// // 	printQueue(&testP);
// // 	// free(dequeue(&testP));
// // 	// printQueue(&testP);
// // 	enqueue(&testP, item4, 0);
// // 	printQueue(&testP);
// // 	enqueue(&testP, item5, 0);
// // 	printQueue(&testP);
// // 	free(dequeue(&testP));
// // 	printQueue(&testP);
// // free(dequeue(&testP));
// // 	printQueue(&testP);
// // free(dequeue(&testP));
// // 	printQueue(&testP);
// // free(dequeue(&testP));
// // 	printQueue(&testP);
// // 	free(dequeue(&testP));
// // 	printQueue(&testP);
// // 	free(testP);
// 	return 0;
// }
