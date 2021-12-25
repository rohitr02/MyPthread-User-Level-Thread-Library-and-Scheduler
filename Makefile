CC = gcc
CFLAGS = -g -c
AR = ar -rc
RANLIB = ranlib

SCHED = PSJF

all: mypthread.a

mypthread.a: mypthread.o
	$(AR) libmypthread.a mypthread.o
	$(RANLIB) libmypthread.a

mypthread.o: mypthread.h

ifeq ($(SCHED), PSJF)
	$(CC) -pthread $(CFLAGS) mypthread.c
else ifeq ($(SCHED), MLFQ)
	$(CC) -pthread $(CFLAGS) -DMLFQ mypthread.c
else
	echo "no such scheduling algorithm"
endif

clean:
	rm -rf testfile *.o *.a
