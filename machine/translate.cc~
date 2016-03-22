// translate.cc 
//	Routines to translate virtual addresses to physical addresses.
//	Software sets up a table of legal translations.  We look up
//	in the table on every memory reference to find the true physical
//	memory location.
//
// Two types of translation are supported here.
//
//	Linear page table -- the virtual page # is used as an index
//	into the table, to find the physical page #.
//
//	Translation lookaside buffer -- associative lookup in the table
//	to find an entry with the same virtual page #.  If found,
//	this entry is used for the translation.
//	If not, it traps to software with an exception. 
//
//	In practice, the TLB is much smaller than the amount of physical
//	memory (16 entries is common on a machine that has 1000's of
//	pages).  Thus, there must also be a backup translation scheme
//	(such as page tables), but the hardware doesn't need to know
//	anything at all about that.
//
//	Note that the contents of the TLB are specific to an address space.
//	If the address space changes, so does the contents of the TLB!
//
// DO NOT CHANGE -- part of the machine emulation
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "machine.h"
#include "addrspace.h"
#include "system.h"

// Routines for converting Words and Short Words to and from the
// simulated machine's format of little endian.  These end up
// being NOPs when the host machine is also little endian (DEC and Intel).

unsigned int
WordToHost(unsigned int word) {
#ifdef HOST_IS_BIG_ENDIAN
	 register unsigned long result;
	 result = (word >> 24) & 0x000000ff;
	 result |= (word >> 8) & 0x0000ff00;
	 result |= (word << 8) & 0x00ff0000;
	 result |= (word << 24) & 0xff000000;
	 return result;
#else 
	 return word;
#endif /* HOST_IS_BIG_ENDIAN */
}

unsigned short
ShortToHost(unsigned short shortword) {
#ifdef HOST_IS_BIG_ENDIAN
	 register unsigned short result;
	 result = (shortword << 8) & 0xff00;
	 result |= (shortword >> 8) & 0x00ff;
	 return result;
#else 
	 return shortword;
#endif /* HOST_IS_BIG_ENDIAN */
}

unsigned int
WordToMachine(unsigned int word) { return WordToHost(word); }

unsigned short
ShortToMachine(unsigned short shortword) { return ShortToHost(shortword); }


//----------------------------------------------------------------------
// Machine::ReadMem
//      Read "size" (1, 2, or 4) bytes of virtual memory at "addr" into 
//	the location pointed to by "value".
//
//   	Returns FALSE if the translation step from virtual to physical memory
//   	failed.
//
//	"addr" -- the virtual address to read from
//	"size" -- the number of bytes to read (1, 2, or 4)
//	"value" -- the place to write the result
//----------------------------------------------------------------------

bool
Machine::ReadMem(int addr, int size, int *value)
{
    int data;
    ExceptionType exception;
    int physicalAddress;
    
    DEBUG('a', "Reading VA 0x%x, size %d\n", addr, size);
    
    exception = Translate(addr, &physicalAddress, size, FALSE);
    if (exception != NoException) {
	machine->RaiseException(exception, addr);
	return FALSE;
    }
    switch (size) {
      case 1:
	data = machine->mainMemory[physicalAddress];
	*value = data;
	break;
	
      case 2:
	data = *(unsigned short *) &machine->mainMemory[physicalAddress];
	*value = ShortToHost(data);
	break;
	
      case 4:
	data = *(unsigned int *) &machine->mainMemory[physicalAddress];
	*value = WordToHost(data);
	break;

      default: ASSERT(FALSE);
    }
    
    DEBUG('a', "\tvalue read = %8.8x\n", *value);
    return (TRUE);
}

//----------------------------------------------------------------------
// Machine::WriteMem
//      Write "size" (1, 2, or 4) bytes of the contents of "value" into
//	virtual memory at location "addr".
//
//   	Returns FALSE if the translation step from virtual to physical memory
//   	failed.
//
//	"addr" -- the virtual address to write to
//	"size" -- the number of bytes to be written (1, 2, or 4)
//	"value" -- the data to be written
//----------------------------------------------------------------------

bool
Machine::WriteMem(int addr, int size, int value)
{
    ExceptionType exception;
    int physicalAddress;
     
    DEBUG('a', "Writing VA 0x%x, size %d, value 0x%x\n", addr, size, value);

    exception = Translate(addr, &physicalAddress, size, TRUE);
    if (exception != NoException) {
	machine->RaiseException(exception, addr);
	return FALSE;
    }
    switch (size) {
      case 1:
	machine->mainMemory[physicalAddress] = (unsigned char) (value & 0xff);
	break;

      case 2:
	*(unsigned short *) &machine->mainMemory[physicalAddress]
		= ShortToMachine((unsigned short) (value & 0xffff));
	break;
      
      case 4:
	*(unsigned int *) &machine->mainMemory[physicalAddress]
		= WordToMachine((unsigned int) value);
	break;
	
      default: ASSERT(FALSE);
    }
    
    return TRUE;
}

TranslationEntry* 
Machine::FindInPageTable(int virtAddr)
{
    unsigned int vpn, offset;
    TranslationEntry *entry;
    vpn = (unsigned) virtAddr / PageSize;
    offset = (unsigned) virtAddr % PageSize;
    if (vpn >= pageTableSize) {
	    DEBUG('a', "virtual page # %d too large for page table size %d!\n", 
			virtAddr, pageTableSize);
	    printf("AddressErrorException\n");
	} else if (!pageTable[vpn].valid) {
	    DEBUG('a', "virtual page # %d too large for page table size %d!\n", 
			virtAddr, pageTableSize);
	    printf("PageFaultException\n");
	}
    entry = &pageTable[vpn];
    return entry;
}

int 
Machine::FIFOFindPos()
{
   int min=0;
   for(int i=0;i<TLBSize;i++){
      if(tlb[i].CreatTime<tlb[min].CreatTime)
         min=i;
   }
   return min;
}

void 
Machine::FIFOUpdateTLB(TranslationEntry* entry)
{
   int i;
   for(i=0;i<TLBSize;i++){
      if(!tlb[i].valid)
        break;
   }
   if(i==TLBSize)
      i=FIFOFindPos();
   tlb[i].virtualPage=entry->virtualPage;
   tlb[i].physicalPage=entry->physicalPage;
   tlb[i].readOnly=entry->readOnly;
   tlb[i].use=entry->use;
   tlb[i].dirty=entry->dirty;
   tlb[i].CreatTime=stats->totalTicks;
   tlb[i].valid=true;
}
int 
Machine::LRUFindPos()
{
   int min=0;
   for(int i=0;i<TLBSize;i++){
      if(tlb[i].LastUseTime<tlb[min].LastUseTime)
         min=i;
   }
   return min;
}
void 
Machine::LRUUpdateTLB(TranslationEntry* entry)
{
   int i;
   for(i=0;i<TLBSize;i++){
      if(!tlb[i].valid)
        break;
   }
   if(i==TLBSize)
      i=LRUFindPos();
   tlb[i].virtualPage=entry->virtualPage;
   tlb[i].physicalPage=entry->physicalPage;
   tlb[i].readOnly=entry->readOnly;
   tlb[i].use=entry->use;
   tlb[i].dirty=entry->dirty;
   tlb[i].LastUseTime=stats->totalTicks;
   tlb[i].valid=true;
}
/*
bool 
Machine::FindInBuffer(int vp){
   for(int i=0;i<BufferNum;i++)
   {
      if(myDataBuffer[i].valid&&muDataBuffer[i].temPage.virtualPage==vp)
      {
         pageTable[vp] = myDataBuffer.temPage;
         return true;
      }
   }
   return false;
}

void
Machine::replace(){
  int minuse=99999999;
  int min;
  for(int i=0;i<pageTableSize;i++)
  {
    if(pageTable[i].valid&&pageTable[i].useNum<minuse){
       min = i;
       minuse = pageTable[i].useNum;
    }
  }
  int pos;
  for(int i=0;i<BufferNum;i++)
  {
     if(!myDataBuffer[i].valid)
     {
       pos=i;
       break;
     }
  }
  pageTable[min].valid = false;
  myDataBuffer[pos].temPage = pageTable[min];
  myDataBuffer[pos].threadNum = currentThread->getThreadId();
  myDataBuffer[pos].valid = true;
  bcopy(&(machine->mainMemory[pageTable[min].physicalPage*pageSize]),myDataBuffer[pos].content,Pagesize);
  memoryManagement->Clear(pageTable[min].physicalPage);
  for(int i=0;i<TLBSize;i++)
  {
    if(tlb[i].virtualPage==pageTable[min].virtualPage)
      tlb[i].valid = false;
   }
}

void
Machine::SwapPage(int addr){
   int vpn = (unsigned) addr / PageSize;
   int offset = (unsigned) addr % PageSize; 
   if(FindInBuffer(vpn)) return;
   printf("return from buffer\n");
   if(addr>=noffH->code.virtualAddr && addr<=noffH->code.virtualAddr)
   {
      pageTable[vpn].virtualPage = vpn;
      pageTable[vpn].physicalPage = memoryManagement->Find();
      printf("physicalPage = %d\n",pageTable[vpn].physicalPage);
      while(pageTable[vpn].physicalPage == -1)
      {
         replace();
         pageTable[vpn].physicalPage = memoryManagement->Find();
      }
      pageTable[vpn].valid = true;
      pageTable[vpn].use = false;
      pageTable[vpn].dirty = false;
      pageTable[vpn].readOnly = false;
      int innerAddr = addr - noffH->code.virtualAddr;
      int innerPage = divRoundDown(innerAddr,PageSize);
      int readInnerAddr = pageTable[vpn].physicalPage*PageSize;
      int realAddr = innerPage*PageSize+noffH->code.inFileAddr;
      execute->ReadAt(&(machine->mainMemory[readInnerAddr]),PageSize,realAddr);
      int innerAddr = addr - noffH->code.virtualAddr;
      int innerPage = divRoundDown(innerAddr,PageSize);
      int readInnerAddr = pageTable[vpn].physicalPage*PageSize;
      int realAddr = innerPage*PageSize+noffH->code.inFileAddr;
      execute->ReadAt(&(machine->mainMemory[readInnerAddr]),PageSize,realAddr);
}

TranslationEntry* 
Machine::FindInPageTable1(int virtAddr){
    unsigned int vpn, offset;
    vpn = (unsigned) virtAddr / PageSize;
    offset = (unsigned) virtAddr % PageSize;
    if(!pageTable[vpn].valid){
       printf("Page Fault!\n");
       SwapPage(virtAddr);
    }
    //pageTable[vpn].useNum++;
    ASSERT(vpn<pageTableSize);
    return (&pageTable[vpn]);
}
*/
//----------------------------------------------------------------------
// Machine::Translate
// 	Translate a virtual address into a physical address, using 
//	either a page table or a TLB.  Check for alignment and all sorts 
//	of other errors, and if everything is ok, set the use/dirty bits in 
//	the translation table entry, and store the translated physical 
//	address in "physAddr".  If there was an error, returns the type
//	of the exception.
//
//	"virtAddr" -- the virtual address to translate
//	"physAddr" -- the place to store the physical address
//	"size" -- the amount of memory being read or written
// 	"writing" -- if TRUE, check the "read-only" bit in the TLB
//----------------------------------------------------------------------

ExceptionType
Machine::Translate(int virtAddr, int* physAddr, int size, bool writing)
{
    int i;
    unsigned int vpn, offset;
    TranslationEntry *entry;
    unsigned int pageFrame;

    DEBUG('a', "\tTranslate 0x%x, %s: ", virtAddr, writing ? "write" : "read");

// check for alignment errors
    if (((size == 4) && (virtAddr & 0x3)) || ((size == 2) && (virtAddr & 0x1))){
	DEBUG('a', "alignment problem at %d, size %d!\n", virtAddr, size);
	return AddressErrorException;
    }
    
    // we must have either a TLB or a page table, but not both!
    //ASSERT(tlb == NULL || pageTable == NULL);	
    //ASSERT(tlb != NULL || pageTable != NULL);	

// calculate the virtual page number, and offset within the page,
// from the virtual address
    vpn = (unsigned) virtAddr / PageSize;
    offset = (unsigned) virtAddr % PageSize;
    
    if (tlb == NULL) {		// => page table => vpn is index into table
	if (vpn >= pageTableSize) {
	    DEBUG('a', "virtual page # %d too large for page table size %d!\n", 
			virtAddr, pageTableSize);
	    return AddressErrorException;
	} else if (!pageTable[vpn].valid) {
	    DEBUG('a', "virtual page # %d too large for page table size %d!\n", 
			virtAddr, pageTableSize);
	    return PageFaultException;
	}
	entry = &pageTable[vpn];
    } else {
        for (entry = NULL, i = 0; i < TLBSize; i++)
    	    if (tlb[i].valid && (tlb[i].virtualPage == vpn)) {
		entry = &tlb[i];			// FOUND!
                tlb[i].LastUseTime = stats->totalTicks;
		break;
	    }
	if (entry == NULL) {				// not found
    	    DEBUG('a', "*** no valid TLB entry found for this virtual page!\n");
            machine->missnum++;
            //printf("missnum: %d\n",machine->missnum);
            machine->RaiseException(TLBPageFaultException,virtAddr);
            for(entry = NULL,i=0;i<TLBSize;i++)
               if(tlb[i].valid&&(tlb[i].virtualPage==vpn)){
               //   printf("***find virtual page %d in TLB [%d]\n",vpn,i);
                  tlb[i].LastUseTime = stats->totalTicks;
                  entry = &tlb[i];
                  break;
               }
	}
    }

    if (entry->readOnly && writing) {	// trying to write to a read-only page
	DEBUG('a', "%d mapped read-only at %d in TLB!\n", virtAddr, i);
	return ReadOnlyException;
    }
    pageFrame = entry->physicalPage;

    // if the pageFrame is too big, there is something really wrong! 
    // An invalid translation was loaded into the page table or TLB. 
    if (pageFrame >= NumPhysPages) { 
	DEBUG('a', "*** frame %d > %d!\n", pageFrame, NumPhysPages);
	return BusErrorException;
    }
    entry->use = TRUE;		// set the use, dirty bits
    if (writing)
	entry->dirty = TRUE;
    *physAddr = pageFrame * PageSize + offset;
    ASSERT((*physAddr >= 0) && ((*physAddr + size) <= MemorySize));
    DEBUG('a', "phys addr = 0x%x\n", *physAddr);
    return NoException;
}
