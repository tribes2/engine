//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "PlatformWin32/platformWin32.h"
#include "Platform/platformSemaphore.h"

void * Semaphore::createSemaphore(U32 initialCount)
{
   HANDLE * semaphore = new HANDLE;
   *semaphore = CreateSemaphore(0, initialCount, -1, 0);
   return(semaphore);
}

void Semaphore::destroySemaphore(void * semaphore)
{
   AssertFatal(semaphore, "Semaphore::destroySemaphore: invalid semaphore");
   CloseHandle(*(HANDLE*)semaphore);
   delete semaphore;
}

bool Semaphore::acquireSemaphore(void * semaphore, bool block)
{
   AssertFatal(semaphore, "Semaphore::acquireSemaphore: invalid semaphore");
   if(block)
   {
      WaitForSingleObject(*(HANDLE*)semaphore, INFINITE);
      return(true);
   }
   else
   {
      DWORD result = WaitForSingleObject(*(HANDLE*)semaphore, 0);
      return(result == WAIT_OBJECT_0);
   }
}

void Semaphore::releaseSemaphore(void * semaphore)
{
   AssertFatal(semaphore, "Semaphore::releaseSemaphore: invalid semaphore");
   ReleaseSemaphore(*(HANDLE*)semaphore, 1, 0);
}
