// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "synch.h"

// testnum is set in main.cc
int testnum = 1;
int sum,low,high,item;
int buffer[5];
int rwbuffer[10]={0};
Semaphore* full;
Semaphore* empty;
Lock* mutex;
Monitor_PC* producer_consumer;
Read_Write_Lock* readwritelock;
Lock* rwlock;
Semaphore* barrierSemaphore;
Lock* barrierLock;
Condition* barrierCondition;
int threadCount=0;

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread %d priority: %d name: \"%s\" userId: %d looped %d times\n", 	currentThread->getThreadId(),currentThread->getPriority(),currentThread->getName(),currentThread->getUserId(), num);
      //  currentThread->Yield();
    }
}

void TS()
{
	ListElement *ptr;
	Thread *p;
	printf("***TS***\n");
	for(ptr=threadList->getFirst();ptr->next!=NULL;ptr=ptr->next)
	{
		p=(Thread*)(ptr->item);
		printf("***thread %d name: \"%s\" userId: %d\n",p->getThreadId(),p->getName(),p->getUserId());
	}
	printf("end\n");
}
/*for semaphore
void ProducerThread(int which)
{
   for(int i=0;i<10;i++)
   {
      empty->P();
      mutex->Acquire();
      sum++;
      item=i;
      buffer[high]=item;
      high=(high+1)%5;
      printf("producer %d priority %d,product %d,sum %d\n",currentThread->getThreadId(),currentThread->getPriority(),item,sum);
      mutex->Release();
      full->V();
   }
}
void ConsumerThread(int which)
{
    for(int i=0;i<10;i++)
    {
       full->P();
       mutex->Acquire();
       item=buffer[low];
       buffer[low]=0;
       low=(low+1)%5;
       sum--;
       printf("consumer %d priority %d,product %d,sum %d\n",currentThread->getThreadId(),currentThread->getPriority(),item,sum);
       mutex->Release();
       empty->V();
    }
}
void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");
    full=new Semaphore("full",0);
    empty=new Semaphore("empty",5);
    mutex=new Lock("mutex");
    sum=0;
    low=0;
    high=0;
    Thread* producer=new Thread("producer");
    Thread* consumer=new Thread("consumer");
    producer->setPriority(86);
    consumer->setPriority(177);
    consumer->Fork(ConsumerThread,consumer->getThreadId());
    producer->Fork(ProducerThread,producer->getThreadId());
    SimpleThread(0);
}
*/
/*for conditon 
void ProducerThread(int which)
{
   for(int i=0;i<10;i++)
   {
      producer_consumer->ProduceInsert(mutex,i);
      sum++;
      printf("producer %d priority %d,product %d,sum %d\n",currentThread->getThreadId(),currentThread->getPriority(),i,sum);
   }
}
void ConsumerThread(int which)
{
    for(int i=0;i<10;i++)
    {
       item=producer_consumer->ConsumerRemove(mutex);
       sum--;
       printf("consumer %d priority %d,product %d,sum %d\n",currentThread->getThreadId(),currentThread->getPriority(),item,sum);
    }
}
void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");
    producer_consumer=new Monitor_PC();
    mutex=new Lock("mutex");
    Thread* producer=new Thread("producer");
    Thread* consumer=new Thread("consumer");
    producer->setPriority(86);
    consumer->setPriority(177);
    consumer->Fork(ConsumerThread,consumer->getThreadId());
    producer->Fork(ProducerThread,producer->getThreadId());
    //SimpleThread(0);
}
*/
/*for readwritelock
void ReaderThread(int which){
    readwritelock->AcquireReadLock(rwlock);
    for (int i=0;i<10;i++)
    {
       printf("reader %d read position %d,the number is %d\n",currentThread->getThreadId(),i,rwbuffer[i]);
       currentThread->Yield();
    }
    readwritelock->ReleaseReadLock(rwlock);
}
void WriterThread(int which){
    readwritelock->AcquireWriteLock(rwlock);
    for (int i=0;i<10;i++)
    {
       rwbuffer[i]=i;
       printf("write done!\n");
    }
    readwritelock->ReleaseWriteLock(rwlock);
}
void
ThreadTest1()
{
   DEBUG('t', "Entering ThreadTest1");
   rwlock = new Lock("rwlock");
   readwritelock= new Read_Write_Lock();
   Thread* reader1=new Thread("reader1");
   Thread* reader2=new Thread("reader2");
   Thread* writer=new Thread("writer");
   reader1->setPriority(80);
   reader2->setPriority(85);
   writer->setPriority(80);
   reader1->Fork(ReaderThread,reader1->getThreadId());
   reader2->Fork(ReaderThread,reader2->getThreadId());
   writer->Fork(WriterThread,writer->getThreadId());
}
*/
/***for barrier***/
void StudentThread(int which)
{
   barrierLock->Acquire();
   threadCount++;
   printf("student %d is waiting\n",currentThread->getThreadId());
   if(threadCount=3)
      barrierSemaphore->V();
   barrierCondition->Wait(barrierLock);
   printf("student %d : start class!\n",currentThread->getThreadId());
   barrierLock->Release();
}
void BarrierThread(int which)
{
   barrierLock->Acquire();
   printf("barrier block!\n");
   barrierSemaphore->P();
   printf("barrier: class begins!\n");
   barrierCondition->Broadcast(barrierLock);
   barrierLock->Release();
}
void
ThreadTest1()
{
   DEBUG('t', "Entering ThreadTest1");
   barrierLock=new Lock("barrierLock");
   barrierSemaphore=new Semaphore("barrierSemaphore",0);
   barrierCondition=new Condition("barrierCondition");
   Thread* student1=new Thread("student1");
   Thread* student2=new Thread("student2");
   Thread* student3=new Thread("student3");
   Thread* barrier=new Thread("barrier");
   student1->setPriority(68);
   student2->setPriority(66);
   student3->setPriority(87);
   barrier->setPriority(50);
   student1->Fork(StudentThread,student1->getThreadId());
   student2->Fork(StudentThread,student2->getThreadId());
   student3->Fork(StudentThread,student3->getThreadId());
   barrier->Fork(BarrierThread,barrier->getThreadId());
}
//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------


//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    switch (testnum) {
    case 1:
	ThreadTest1();
	break;
    default:
	printf("No test specified.\n");
	break;
    }
}

