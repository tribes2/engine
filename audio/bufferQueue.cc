//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "audio/bufferQueue.h"

//--------------------------------------------------------------------------
// Class BufferQueue:
//--------------------------------------------------------------------------
BufferQueue::BufferQueue()
{
   mQueue = NULL;
   mSize  = 0;
   mHead  = NULL;
   mTail  = NULL;
   mIsFull= false;
}

BufferQueue::~BufferQueue()
{
   setSize(0);
}

//--------------------------------------------------------------------------
void BufferQueue::setSize(U32 size)
{
   mIsFull = false;
   if (size == mSize)
   {
      clear();
      return;
   }
   mSize = size;

   if (mQueue)
   {
      delete [] mQueue;
      mQueue = NULL;
   }

   if (mSize)
      mQueue = new U8[mSize];

   mTail = mQueue;
   mHead = mQueue;
}

//--------------------------------------------------------------------------
void BufferQueue::enqueue(U32 size)
{
   AssertFatal(size <= getContiguousFree(), "BufferQueue: enqueue overflow.");

   if (size == 0)
      return;

   advanceTail(size);
}

void BufferQueue::enqueue(const U8* data, U32 size)
{
   AssertFatal(size <= getFree(), "BufferQueue: enqueue overflow.");
   if (size == 0)
      return;

   U32 con= getContiguousFree();
   if (size <= con)
      dMemcpy(mTail, data, size);
   else
   {
      dMemcpy(mTail, data, con);
      dMemcpy(mQueue, &data[con], size-con);
   }

   advanceTail(size);
}   


//--------------------------------------------------------------------------
U32 BufferQueue::dequeue(U32 request)
{
   if (request == 0)
      return 0;

   request = getMin(request, getContiguousUsed());

   advanceHead(request);
   return request;
}

U32 BufferQueue::dequeue(U8* data, U32 request)
{
   if (request == 0)
      return 0;

   request = getMin(request, getUsed());
   U32 con=getContiguousUsed();

   if (con >= request)
      dMemcpy(data, mHead, request);
   else
   {
      dMemcpy(data, mHead, con);
      dMemcpy(data+con, mQueue, request-con);
   }

   advanceHead(request);
   return request;
}
