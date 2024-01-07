//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "PlatformWin32/platformWin32.h"
#include "Platform/platformMutex.h"

void * Mutex::createMutex()
{
   CRITICAL_SECTION * mutex = new CRITICAL_SECTION;
   InitializeCriticalSection(mutex);
   return((void*)mutex);
}

void Mutex::destroyMutex(void * mutex)
{
   AssertFatal(mutex, "Mutex::destroyMutex: invalid mutex");
   DeleteCriticalSection((CRITICAL_SECTION*)mutex);
   delete mutex;
}

void Mutex::lockMutex(void * mutex)
{
   AssertFatal(mutex, "Mutex::lockMutex: invalid mutex");
   EnterCriticalSection((CRITICAL_SECTION*)mutex);
}

void Mutex::unlockMutex(void * mutex)
{
   AssertFatal(mutex, "Mutex::unlockMutex: invalid mutex");
   LeaveCriticalSection((CRITICAL_SECTION*)mutex);
}
