//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "platformX86UNIX/platformX86UNIX.h"
#include "platform/platformSemaphore.h"
#include <fcntl.h>
#include <semaphore.h>

void * Semaphore::createSemaphore(U32 initialCount)
{
   sem_t *semaphore;
   /* hell, I want an elite semaphore, OK?! - rjp */
   semaphore = sem_open("/tmp/eliteQueue.31337", O_CREAT, 0664, initialCount);
   return(semaphore);
}

void Semaphore::destroySemaphore(void * semaphore)
{
   AssertFatal(semaphore, "Semaphore::destroySemaphore: invalid semaphore");
   sem_close((sem_t *)semaphore);
   delete semaphore;
}

bool Semaphore::acquireSemaphore(void * semaphore, bool block)
{
   AssertFatal(semaphore, "Semaphore::acquireSemaphore: invalid semaphore");
   if(block)
   {
      sem_wait((sem_t *)semaphore);
      return(true);
   }
   else
   {
      U32 result = sem_trywait((sem_t *)semaphore);
      return(result == 0);
   }
}

void Semaphore::releaseSemaphore(void * semaphore)
{
   AssertFatal(semaphore, "Semaphore::releaseSemaphore: invalid semaphore");
   sem_unlink("/tmp/eliteQueue.31337");
}
