#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "../mypthread.h"

/* A scratch program template on which to call and
 * test mypthread library functions as you implement
 * them.
 *
 * You can modify and use this program as much as possible.
 * This will not be graded.
 */

void *func(void *arg)
{
	printf("thread 0 - arg = %ld\n", (long)arg);
	return (void *)0;
}

int main(int argc, char **argv) {

	/* Implement HERE */
	printf("Test0\n");
	mypthread_t id;
	mypthread_create(&id, NULL, &func, NULL);

	printf("Test1\n");
	mypthread_t id2;
	mypthread_create(&id2, NULL, &func, NULL);

	printf("Test2\n");
	mypthread_t id3;
	mypthread_create(&id3, NULL, &func, NULL);

	mypthread_exit(id);
	mypthread_join(id2, NULL);
	mypthread_join(id3, NULL);


	return 0;
}
