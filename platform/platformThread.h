//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _PLATFORMTHREAD_H_
#define _PLATFORMTHREAD_H_

#ifndef _TYPES_H_
#include "Platform/types.h"
#endif

typedef void (*ThreadRunFunction)(S32);

class Thread
{
   protected:
      void *      mData;
      void start();

   public:
      Thread(ThreadRunFunction func = 0, S32 arg = 0, bool start_thread = true);
      virtual ~Thread();

      bool join();

      virtual void run(S32 arg = 0);

      bool isAlive();
};

#endif
