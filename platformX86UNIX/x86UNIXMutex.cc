//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "platformX86UNIX/platformX86UNIX.h"
#include "platform/platformMutex.h"
#include <pthread.h>

void * Mutex::createMutex()
{
   pthread_mutex_t *mutex;

   mutex = (pthread_mutex_t *) dRealMalloc(sizeof(pthread_mutex_t));
   pthread_mutex_init(mutex, NULL);

   return((void*)mutex);
}

void Mutex::destroyMutex(void * mutex)
{
   AssertFatal(mutex, "Mutex::destroyMutex: invalid mutex");
   pthread_mutex_destroy((pthread_mutex_t *)mutex);
   delete mutex;
}

void Mutex::lockMutex(void * mutex)
{
   AssertFatal(mutex, "Mutex::lockMutex: invalid mutex");
   pthread_mutex_lock((pthread_mutex_t *)mutex);
}

void Mutex::unlockMutex(void * mutex)
{
   AssertFatal(mutex, "Mutex::unlockMutex: invalid mutex");
   pthread_mutex_unlock((pthread_mutex_t *)mutex);
}
