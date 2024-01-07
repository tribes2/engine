//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _BUFFERQUEUE_H_
#define _BUFFERQUEUE_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

class BufferQueue
{
   private:

      U8 *mQueue;
      U8 *mHead;
      U8 *mTail;
      U32 mSize;
      bool mIsFull;

      void advanceHead(U32 n);
      void advanceTail(U32 n);

   public:

      BufferQueue();
      ~BufferQueue();

      void setSize(U32 size);
      bool isFull()                 { return mIsFull; }
      bool isEmpty()                { return !mIsFull && mHead == mTail; }
      void clear()                  { mHead = mTail = mQueue; mIsFull = false; }

      U8* getTail()                 { return mIsFull ?  NULL : mTail; }
      U8* getHead()                 { return mHead; }

      // inlined
      U32 getFree();
      U32 getUsed();
      U32 getContiguousFree();
      U32 getContiguousUsed();

      void enqueue(U32 size);
      void enqueue(const U8* data, U32 size);
      U32  dequeue(U32 size);
      U32  dequeue(U8* data, U32 size);
};


//--------------------------------------------------------------------------
// BufferQueue: inlined functions
//--------------------------------------------------------------------------
inline void BufferQueue::advanceHead(U32 n)
{
   mHead += n;                      // advance head
   if (mHead >= (mQueue + mSize))   // wrap around the queue
      mHead -= mSize;
   mIsFull = false;
}

inline void BufferQueue::advanceTail(U32 n)
{
   mTail += n;                      // advance tail
   if (mTail >= (mQueue + mSize))   // wrap around the queue
      mTail -= mSize;

   mIsFull = (mTail == mHead);
}

inline U32 BufferQueue::getFree()
{
   if (mIsFull == false)
      return (mTail < mHead) ? (mHead - mTail) : (mSize - (mTail-mHead)); 
   else
      return 0;
}

inline U32 BufferQueue::getUsed() 
{
   if (mIsFull == false)
      return (mHead <= mTail) ? (mTail - mHead) : (mSize - (mHead-mTail)); 
   else
      return mSize;
}

inline U32 BufferQueue::getContiguousFree() 
{ 
   if (mIsFull == false)
      return (mTail < mHead) ? (mHead - mTail) : (mSize - (mTail-mQueue)); 
   else
      return 0;
}

inline U32 BufferQueue::getContiguousUsed() 
{ 
   if (mIsFull == false)
      return (mHead <= mTail) ? (mTail - mHead) : (mSize - (mHead-mQueue)); 
   else
      return mSize;
}

#endif   // _INC_BUFFERQUEUE
