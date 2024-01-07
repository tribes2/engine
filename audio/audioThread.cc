//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "audio/audioThread.h"

// AudioResourceQueue: -----------------------------------------------------
AudioResourceQueue::AudioResourceQueue()
{
   mHead = mTail = 0;
}

void AudioResourceQueue::enqueue(AudioResourceEntry * entry)
{
   AssertFatal(entry, "AudioResourceQueue::enqueue: invalid entry");
   entry->mNext = 0;

   if(!mHead)
      mHead = mTail = entry;
   else
   {
      mTail->mNext = entry;
      mTail = entry;
   }
}

AudioResourceEntry * AudioResourceQueue::dequeue()
{
   AudioResourceEntry * entry = mHead;
   
   if(mHead == mTail)
      mHead = mTail = 0;
   else
      mHead = mHead->mNext;

   return(entry);
}

// AudioThread: ------------------------------------------------------------
AudioThread * gAudioThread = 0;

AudioThread::AudioThread() : Thread(0, 0, false)
{
   mStopSemaphore = Semaphore::createSemaphore(0);
   mWakeSemaphore = Semaphore::createSemaphore();
   mMutex = Mutex::createMutex();

   // Now that the semaphores are created, start up the thread
   start();
}

AudioThread::~AudioThread()
{
   Semaphore::destroySemaphore(mStopSemaphore);
   Semaphore::destroySemaphore(mWakeSemaphore);
   Mutex::destroyMutex(mMutex);
}

void AudioThread::wake()
{
   if(!isAlive())
      return;

   Semaphore::releaseSemaphore(mWakeSemaphore);
}

void AudioThread::stop()
{
   if(!isAlive())
      return;

   Semaphore::releaseSemaphore(mStopSemaphore);
   wake();
   join();
}

void AudioThread::run(S32)
{
   while(1)
   {
      Semaphore::acquireSemaphore(mWakeSemaphore);

      if(Semaphore::acquireSemaphore(mStopSemaphore, false))
         return;

      lock();
      AudioResourceEntry * entry = mLoadingQueue.mHead;
      
      if(!entry)
      {
         unlock();
         continue;
      }
      
      // load it in   
      Stream * stream = ResourceManager->openStream(entry->mResourceObj);
      stream->read(entry->mResourceObj->fileSize, entry->mData);
      ResourceManager->closeStream(stream);

      mLoadedQueue.enqueue(mLoadingQueue.dequeue());
      unlock();      
   }
}

//--------------------------------------------------------------------------
void AudioThread::loadResource(ResourceObject * obj, AudioBuffer * buffer)
{
   AssertFatal(!buffer->mLoading, "AudioThread::loadResource: buffer already loading");
   AudioResourceEntry * entry = new AudioResourceEntry();

   entry->mResourceObj = obj;
   entry->mBuffer = buffer;
   entry->mData = dMalloc(obj->fileSize);
   dMemset(entry->mData, 0, obj->fileSize);

   buffer->mLoading = true;
   
   lock();
   mLoadingQueue.enqueue(entry);   
   unlock();

   Semaphore::acquireSemaphore(mWakeSemaphore, false);
   Semaphore::releaseSemaphore(mWakeSemaphore);
}

void AudioThread::setBufferPlayHandle(AudioBuffer * buffer, AUDIOHANDLE handle)
{
   lock();
   
   // search the loading list
   AudioResourceEntry * entry = mLoadingQueue.mHead;
   bool found = false;

   while(entry && !found)
   {
      if(entry->mBuffer == buffer)
      {
         entry->mPlayHandle = handle;
         found = true;
      }
      entry = entry->mNext;
   }
   
   // search the loaded list
   if(!found)
   {
      entry = mLoadedQueue.mHead;

      while(entry && !found)
      {
         if(entry->mBuffer == buffer)
         {
            entry->mPlayHandle = handle;
            found = true;
         }
         entry = entry->mNext;
      }
   }
   
   unlock();
}

AudioResourceEntry * AudioThread::getLoadedList()
{
   lock();
   AudioResourceEntry * list = mLoadedQueue.mHead;
   mLoadedQueue.mHead = mLoadedQueue.mTail = 0;
   unlock();

   return(list);
}

// static methods: --------------------------------------------------------
void AudioThread::create()
{
   if(gAudioThread)
      return;

   gAudioThread = new AudioThread();
}

void AudioThread::destroy()
{
   if(!gAudioThread)
      return;
   
   gAudioThread->stop();
   delete gAudioThread;
   gAudioThread = 0;
}

void AudioThread::process()
{
   if(!gAudioThread)
      return;
   
   AudioResourceEntry * entry = gAudioThread->getLoadedList();

   // sync all the loaded buffers and play those marked   
   while(entry)
   {
      entry->mBuffer->mLoading = false;
      
      if(alBufferSyncData_EXT(entry->mBuffer->malBuffer, AL_FORMAT_WAVE_EXT, entry->mData,
         entry->mResourceObj->fileSize, 0))
      {
         if(entry->mPlayHandle != NULL_AUDIOHANDLE)     
            alxPlay(entry->mPlayHandle);
      }

      AudioResourceEntry * next = entry->mNext;
      delete entry;
      entry = next;
   }
}
