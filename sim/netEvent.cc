//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "core/dnet.h"
#include "console/simBase.h"
#include "sim/netConnection.h"
#include "core/bitStream.h"

#define DebugChecksum 0xF00DBAAD

void NetEvent::notifyDelivered(NetConnection *, bool)
{
}

void NetEvent::notifySent(NetConnection *)
{
}

#ifdef DEBUG_NET
const char *NetEvent::getDebugName()
{
   return getClassName();
}
#endif

void NetConnection::eventPacketDropped(PacketNotify *notify)
{
   NetEvent *walk = notify->eventList;
   NetEvent **insertList = &mSendEventQueueHead;
   NetEvent *temp;
   
   while(walk)
   {
      switch(walk->mGuaranteeType)
      {
         case NetEvent::GuaranteedOrdered:
            //Con::printf("EVT  %d: DROP - %d", getId(), walk->mSeqCount);
            while(*insertList && (*insertList)->mSeqCount < walk->mSeqCount)
               insertList = &((*insertList)->mNextEvent);
            
            temp = walk->mNextEvent;
            walk->mNextEvent = *insertList;
            if(!walk->mNextEvent)
               mSendEventQueueTail = walk;
            *insertList = walk;
            insertList = &(walk->mNextEvent);
            walk = temp;
            break;
         case NetEvent::Guaranteed:
            temp = walk->mNextEvent;
            walk->mNextEvent = mUnorderedSendEventQueueHead;
            mUnorderedSendEventQueueHead = walk;
            if(!walk->mNextEvent)
               mUnorderedSendEventQueueTail = walk;
            walk = temp;
            break;
         case NetEvent::Unguaranteed:
            walk->notifyDelivered(this, false);
            temp = walk->mNextEvent;
            delete walk;
            walk = temp;
      }
   }
}

void NetConnection::eventPacketReceived(PacketNotify *notify)
{
   NetEvent *walk = notify->eventList;
   NetEvent **noteList = &mNotifyEventList;

   while(walk)
   {
      NetEvent *next = walk->mNextEvent;
      if(walk->mGuaranteeType != NetEvent::GuaranteedOrdered)
      {
         walk->notifyDelivered(this, true);
         delete walk;
         walk = next;
      }
      else
      {
         while(*noteList && (*noteList)->mSeqCount < walk->mSeqCount)
            noteList = &((*noteList)->mNextEvent);
         
         walk->mNextEvent = *noteList;
         *noteList = walk;
         noteList = &walk->mNextEvent;
         walk = next;
      }
   }
   while(mNotifyEventList && mNotifyEventList->mSeqCount == mLastAckedEventSeq + 1)
   {
      mLastAckedEventSeq++;
      NetEvent *next = mNotifyEventList->mNextEvent;
      //Con::printf("EVT  %d: ACK - %d", getId(), mNotifyEventList->mSeqCount);
      mNotifyEventList->notifyDelivered(this, true);
      delete mNotifyEventList;
      mNotifyEventList = next;
   }
}

void NetConnection::eventWritePacket(BitStream *bstream, PacketNotify *notify)
{
#ifdef DEBUG_NET
   bstream->writeInt(DebugChecksum, 32);
#endif

   NetEvent *packQueueHead = NULL, *packQueueTail = NULL;

   while(mUnorderedSendEventQueueHead)
   {
      if(bstream->isFull())
         break;
      // dequeue the first event
      NetEvent *ev = mUnorderedSendEventQueueHead;
      mUnorderedSendEventQueueHead = ev->mNextEvent;
      U32 start = bstream->getCurPos();
      
      bstream->writeFlag(true);
      S32 classId = ev->getClassId();
      AssertFatal(classId >= NetEventClassFirst && classId <= NetEventClassLast,
         "Out of range event class id... check simBase.h");
      bstream->writeInt(classId - NetEventClassFirst, NetEventClassBitSize);

      ev->pack(this, bstream);
      DEBUG_LOG(("PKLOG %d EVENT %d: %s", getId(), bstream->getCurPos() - start, ev->getDebugName()) );

#ifdef DEBUG_NET
      bstream->writeInt(classId, 10);
      bstream->writeInt(classId ^ DebugChecksum, 32);
#endif
      // add this event onto the packet queue
      ev->mNextEvent = NULL;
      if(!packQueueHead)
         packQueueHead = ev;
      else
         packQueueTail->mNextEvent = ev;
      packQueueTail = ev;
   }
   
   bstream->writeFlag(false);   
   S32 prevSeq = -2;
   
   while(mSendEventQueueHead)
   {
      if(bstream->isFull())
         break;
      
      // if the event window is full, stop processing
      if(mSendEventQueueHead->mSeqCount > mLastAckedEventSeq + 126)
         break;

      // dequeue the first event
      NetEvent *ev = mSendEventQueueHead;
      mSendEventQueueHead = ev->mNextEvent;
      
      //Con::printf("EVT  %d: SEND - %d", getId(), ev->mSeqCount);

      bstream->writeFlag(true);

      ev->mNextEvent = NULL;
      if(!packQueueHead)
         packQueueHead = ev;
      else
         packQueueTail->mNextEvent = ev;
      packQueueTail = ev;
      if(!bstream->writeFlag(ev->mSeqCount == prevSeq + 1))
         bstream->writeInt(ev->mSeqCount, 7);
      prevSeq = ev->mSeqCount;

      U32 start = bstream->getCurPos();
      S32 classId = ev->getClassId();
      AssertFatal(classId >= NetEventClassFirst && classId <= NetEventClassLast,
         "Out of range event class id... check simBase.h");
      bstream->writeInt(classId - NetEventClassFirst, NetEventClassBitSize);

      ev->pack(this, bstream);
      DEBUG_LOG(("PKLOG %d EVENT %d: %s", getId(), bstream->getCurPos() - start, ev->getDebugName()) );
#ifdef DEBUG_NET
      bstream->writeInt(classId, 10);
      bstream->writeInt(classId ^ DebugChecksum, 32);
#endif
   }
   for(NetEvent *ev = packQueueHead; ev; ev = ev->mNextEvent)
      ev->notifySent(this);
      
   notify->eventList = packQueueHead;
   bstream->writeFlag(0);
}

void NetConnection::eventReadPacket(BitStream *bstream)
{
#ifdef DEBUG_NET
   U32 sum = bstream->readInt(32);
   AssertISV(sum == DebugChecksum, "Invalid checksum.");
#endif
   
   S32 prevSeq = -2;
   NetEvent **waitInsert = &mWaitSeqEvents;
   bool unguaranteedPhase = true;
   
   while(true)
   {
      bool bit = bstream->readFlag();
      if(unguaranteedPhase && !bit)
      {
         unguaranteedPhase = false;
         bit = bstream->readFlag();
      }
      if(!unguaranteedPhase && !bit)
         break;
      
      S32 seq = -1;
      
      if(!unguaranteedPhase) // get the sequence
      {
         if(bstream->readFlag())
            seq = (prevSeq + 1) & 0x7f;
         else
            seq = bstream->readInt(7);
         prevSeq = seq;
      }
      S32 classTag = bstream->readInt(NetEventClassBitSize) + NetEventClassFirst;
      NetEvent *evt = (NetEvent *) ConsoleObject::create(classTag);
      if(!evt)
      {
         setLastError("Invalid packet.");
         return;
      }
      AbstractClassRep *rep = evt->getClassRep();
      if((rep->mClassNetClass == NetEventClassClient && !isServerConnection())
         || (rep->mClassNetClass == NetEventClassServer && isServerConnection()) )
      {
         setLastError("Invalid Packet.");
         return;
      }


      evt->mSourceId = getId();
      evt->unpack(this, bstream);
      if(mErrorBuffer[0])
         return;
#ifdef DEBUG_NET
      U32 classId = bstream->readInt(10);
      U32 checksum = bstream->readInt(32);
      AssertISV( (checksum ^ DebugChecksum) == (U32)classTag,
         avar("unpack did not match pack for event of class %s.",
            evt->getClassName()) );
#endif
      if(unguaranteedPhase)
      {
         evt->process(this);
         delete evt;
         if(mErrorBuffer[0])
            return;
         continue;
      }
      seq |= (mNextRecvEventSeq & ~0x7F);
      if(seq < mNextRecvEventSeq)
         seq += 128;
      
      evt->mSeqCount = seq;
      //Con::printf("EVT  %d: RECV - %d", getId(), evt->mSeqCount);
      while(*waitInsert && (*waitInsert)->mSeqCount < seq)
         waitInsert = &((*waitInsert)->mNextEvent);
      
      evt->mNextEvent = *waitInsert;
      *waitInsert = evt;
      waitInsert = &(evt->mNextEvent);
   }
   while(mWaitSeqEvents && mWaitSeqEvents->mSeqCount == mNextRecvEventSeq)
   {
      mNextRecvEventSeq++;
      NetEvent *temp = mWaitSeqEvents;
      mWaitSeqEvents = temp->mNextEvent;
      
      //Con::printf("EVT  %d: PROCESS - %d", getId(), temp->mSeqCount);
      temp->process(this);
      delete temp;
      if(mErrorBuffer[0])
         return;
   }
}

bool NetConnection::postNetEvent(NetEvent *event)
{
   AssertFatal(event->mIsPosted == false, "Event cannot be posted to more than one connection.");
   event->mIsPosted = true;
   
   if(!mSendingEvents)
   {
      delete event;
      return false;
   }
   event->mNextEvent = NULL;
   if(event->mGuaranteeType == NetEvent::GuaranteedOrdered)
   {
      event->mSeqCount = mNextSendEventSeq++;
      if(!mSendEventQueueHead)
         mSendEventQueueHead = event;
      else
         mSendEventQueueTail->mNextEvent = event;
      mSendEventQueueTail = event;
   }
   else
   {
      event->mSeqCount = InvalidSendEventSeq;
      if(!mUnorderedSendEventQueueHead)
         mUnorderedSendEventQueueHead = event;
      else
         mUnorderedSendEventQueueTail->mNextEvent = event;
      mUnorderedSendEventQueueTail = event;
   }
   return true;
}


void NetConnection::eventWriteStartBlock(ResizeBitStream *stream)
{
   stream->write(mNextRecvEventSeq);
   for(NetEvent *walk = mWaitSeqEvents; walk; walk = walk->mNextEvent)
   {
      stream->writeFlag(true);
      S32 classId = walk->getClassId();
      stream->writeInt(classId - NetEventClassFirst, NetEventClassBitSize);
      walk->write(this, stream);
      stream->validate();
   }
   stream->writeFlag(false);
}

void NetConnection::eventReadStartBlock(BitStream *stream)
{
   stream->read(&mNextRecvEventSeq);
   
   NetEvent *lastEvent = NULL;
   while(stream->readFlag())
   {
      S32 classTag = stream->readInt(NetEventClassBitSize) + NetEventClassFirst;
      NetEvent *evt = (NetEvent *) ConsoleObject::create(classTag);
      evt->unpack(this, stream);
      evt->mNextEvent = NULL;
      
      if(!lastEvent)
         mWaitSeqEvents = evt;
      else
         lastEvent->mNextEvent = evt;
      lastEvent = evt;
   }
}
