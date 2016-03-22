// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!
/***Lock***/
Lock::Lock(char* debugName) {
    mutex = new Semaphore("lock",1);
    name = debugName;
    threadHoldLock=NULL;
}
Lock::~Lock() {
    delete mutex;
}
void Lock::Acquire() {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    mutex->P();
    threadHoldLock = currentThread;
    (void) interrupt->SetLevel(oldLevel);
}
void Lock::Release() {
   IntStatus oldLevel = interrupt->SetLevel(IntOff);
 //  ASSERT(isHeldByCurrentThread());
   mutex->V();
   threadHoldLock = NULL;
   (void) interrupt->SetLevel(oldLevel);
}
bool Lock::isHeldByCurrentThread(){
    return threadHoldLock == currentThread;
}
/***Lock***/

/***Condition***/
Condition::Condition(char* debugName) {
    name = debugName;
    queue = new List();
}
Condition::~Condition() {
    delete queue;
}
void Condition::Wait(Lock* conditionLock) { 
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    ASSERT(conditionLock->isHeldByCurrentThread());
    conditionLock->Release();
    queue->Append((void*)currentThread);
    currentThread->Sleep();
    conditionLock->Acquire();
    (void) interrupt->SetLevel(oldLevel);
}
void Condition::Signal(Lock* conditionLock) {
    Thread* tmp;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    ASSERT(conditionLock->isHeldByCurrentThread());
    if(!queue->IsEmpty())
    {
       tmp = (Thread*)queue->Remove();
       scheduler->ReadyToRun((Thread*)tmp);
    }
    (void) interrupt->SetLevel(oldLevel);
}
void Condition::Broadcast(Lock* conditionLock) {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    ASSERT(conditionLock->isHeldByCurrentThread());
    while(!queue->IsEmpty())
    {
       Signal(conditionLock);
    }
    (void) interrupt->SetLevel(oldLevel);
}
/***Condition***/

/***Monitor_PC***/
void Monitor_PC::ProduceInsert(Lock* conditionLock,int item){
    conditionLock->Acquire();
    if(count==5)
    {
      full->Wait(conditionLock);
    }
    buffer[high]=item;
    high=(high+1)%5;
    count++;
    if(count==1)
    {
      empty->Signal(conditionLock);
    }
    conditionLock->Release();
}
int Monitor_PC::ConsumerRemove(Lock* conditionLock){
    conditionLock->Acquire();
    if(count==0)
    {
       empty->Wait(conditionLock);
    }
    tem=buffer[low];
    buffer[low]=0;
    low=(low+1)%5;
    count--;
    if(count==4)
    {
       full->Signal(conditionLock);
    }
    conditionLock->Release();
    return tem;
}
Monitor_PC::Monitor_PC(){
    low=0;
    high=0;
    count=0;
    full=new Condition("full");
    empty=new Condition("empty");
}
Monitor_PC::~Monitor_PC(){
    delete full;
    delete empty;
}
/***Monitor_PC***/

/***read write lock***/
void Read_Write_Lock::AcquireReadLock(Lock* rwLock){
    mutex->Acquire();
    rc++;
    if(rc==1)
       rwLock->Acquire();
    mutex->Release();
}
void Read_Write_Lock::AcquireWriteLock(Lock* rwLock){
    rwLock->Acquire();
}
void Read_Write_Lock::ReleaseReadLock(Lock* rwLock){
    mutex->Acquire();
    rc--;
    if(rc==0)
      rwLock->Release();
    mutex->Release();
}
void Read_Write_Lock::ReleaseWriteLock(Lock* rwLock){
    rwLock->Release();
}
Read_Write_Lock::Read_Write_Lock(){
    mutex=new Lock("mutex");
    rc=0;
}
Read_Write_Lock::~Read_Write_Lock(){
    delete mutex;
}
/***read write lock***/
