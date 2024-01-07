//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _PLATFORMSEMAPHORE_H_
#define _PLATFORMSEMAPHORE_H_

#ifndef _TYPES_H_
#include "Platform/types.h"
#endif

struct Semaphore
{
   static void * createSemaphore(U32 initialCount = 1);
   static void destroySemaphore(void * semaphore);
   static bool acquireSemaphore(void * semaphore, bool block = true);
   static void releaseSemaphore(void * semaphore);

   inline static bool P(void * semaphore, bool block = true) {return(acquireSemaphore(semaphore, block));}
   inline static void V(void * semaphore) {releaseSemaphore(semaphore);}
};

#endif
