//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _AUDIOTHREAD_H_
#define _AUDIOTHREAD_H_

#ifndef _PLATFORMTHREAD_H_
#include "Platform/platformThread.h"
#endif
#ifndef _PLATFORMSEMAPHORE_H_
#include "Platform/platformSemaphore.h"
#endif
#ifndef _PLATFORMMUTEX_H_
#include "Platform/platformMutex.h"
#endif
#ifndef _AUDIO_H_
#include "audio/audio.h"
#endif

struct AudioResourceEntry
{
   ResourceObject *        mResourceObj;
   AudioBuffer *           mBuffer;
   void *                  mData;
   AUDIOHANDLE             mPlayHandle;
   AudioResourceEntry *    mNext;
   
   AudioResourceEntry()
   {
      mResourceObj   = 0;
      mBuffer        = 0;
      mData          = 0;
      mPlayHandle    = NULL_AUDIOHANDLE;
      mNext          = 0;
   }
};

struct AudioResourceQueue
{
   AudioResourceEntry *    mHead;
   AudioResourceEntry *    mTail;
  
   AudioResourceQueue();
   
   void enqueue(AudioResourceEntry * entry);
   AudioResourceEntry * dequeue();
};

class AudioThread : public Thread
{
   private:

      void *      mStopSemaphore;
      void *      mWakeSemaphore;
      void *      mMutex;

      // accessed in both threads. memory must be managed in main thread
      AudioResourceQueue       mLoadingQueue;
      AudioResourceQueue       mLoadedQueue;
      
   public:

      AudioThread();
      ~AudioThread();

      void wake();
      void stop();

      void lock()    { Mutex::lockMutex(mMutex); }
      void unlock()  { Mutex::unlockMutex(mMutex); }

      void run(S32 arg);

      void setBufferPlayHandle(AudioBuffer * buffer, AUDIOHANDLE handle);
      void loadResource(ResourceObject * resourceObj, AudioBuffer * buffer);
      AudioResourceEntry * getLoadedList();

      // static methods      
      static void create();
      static void destroy();
      static void process();
};

extern AudioThread * gAudioThread;

#endif
