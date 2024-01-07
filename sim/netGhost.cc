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
#include "sim/netObject.h"

#define DebugChecksum 0xF00D

class GhostingMessageEvent : public NetEvent
{
   U32 sequence;
   S32 message;
   U32 ghostCount;
public:
   GhostingMessageEvent(S32 msg=0, U32 seq=0, U32 gc=0)
      { message = msg; sequence = seq; ghostCount = gc;}
   void pack(NetConnection *, BitStream *bstream)
   { 
      bstream->write(sequence);
      bstream->writeInt(message, 3);
      bstream->writeInt(ghostCount, 11);
   }
   void write(NetConnection *, BitStream *bstream)
   {
      bstream->write(sequence);
      bstream->writeInt(message, 3);
      bstream->writeInt(ghostCount, 11);
   }
   void unpack(NetConnection *, BitStream *bstream)
   { 
      bstream->read(&sequence);
      message = bstream->readInt(3);
      ghostCount = bstream->readInt(11);
   }
   void process(NetConnection *ps)
   {
      ps->handleGhostMessage(message, sequence, ghostCount);
   }
   DECLARE_CONOBJECT(GhostingMessageEvent);
};

IMPLEMENT_CO_NETEVENT_V1(GhostingMessageEvent);

class GhostAlwaysObjectEvent : public NetEvent
{
   SimObjectId objectId;
   U32 ghostIndex;
   NetObject *object;
   bool validObject;
public:
   GhostAlwaysObjectEvent(NetObject *obj = NULL, U32 index = 0)
   {
      if(obj)
      {
         objectId = obj->getId();
         ghostIndex = index;
      }
      object = NULL;
   }
   ~GhostAlwaysObjectEvent()
      { delete object; }
   
   void pack(NetConnection *ps, BitStream *bstream)
   {
      bstream->writeInt(ghostIndex, 10);

      NetObject *obj = (NetObject *) Sim::findObject(objectId);
      if(bstream->writeFlag(obj != NULL))
      {
         S32 classId = obj->getClassId();
         AssertFatal(classId >= NetObjectClassFirst && classId <= NetObjectClassLast,
            "Out of range event class id... check simBase.h");
         bstream->writeInt(classId - NetObjectClassFirst, NetObjectClassBitSize);
         obj->packUpdate(ps, 0xFFFFFFFF, bstream);
      }
   }
   void write(NetConnection *ps, BitStream *bstream)
   {
      bstream->writeInt(ghostIndex, 10);
      if(bstream->writeFlag(validObject))
      {
         S32 classId = object->getClassId();
         bstream->writeInt(classId - NetObjectClassFirst, NetObjectClassBitSize);
         object->packUpdate(ps, 0xFFFFFFFF, bstream);
      }
   }
   void unpack(NetConnection *ps, BitStream *bstream)
   {
      ghostIndex = bstream->readInt(10);

      if(bstream->readFlag())
      {
         S32 classId = bstream->readInt(NetObjectClassBitSize) + NetObjectClassFirst;
         object = (NetObject *) ConsoleObject::create(classId);
         if(!object)
         {
            ps->setLastError("Invalid packet.");
            return;
         }
         object->mNetFlags = NetObject::IsGhost;
         object->unpackUpdate(ps, bstream);
         validObject = true;
      }
      else
      {
         object = new NetObject;
         validObject = false;
      }
   }
   void process(NetConnection *ps)
   {
      Con::executef(1, "ghostAlwaysObjectReceived");

      ps->setGhostAlwaysObject(object, ghostIndex);
      object = NULL;
   }
   DECLARE_CONOBJECT(GhostAlwaysObjectEvent);
};

IMPLEMENT_CO_NETEVENT_V1(GhostAlwaysObjectEvent);


void NetConnection::ghostPacketDropped(PacketNotify *notify)
{
   GhostRef *packRef = notify->ghostList;
   // loop through all the packRefs in the packet

   while(packRef)
   {
      GhostRef *temp = packRef->nextRef;

      U32 orFlags = 0;
      AssertFatal(packRef->nextUpdateChain == NULL, "Out of order notify!!");

      // clear out the ref for this object, plus or together all
      // flags from updates after this

      GhostRef **walk = &(packRef->ghost->updateChain);
      while(*walk != packRef)
      {
         orFlags |= (*walk)->mask;
         walk = &((*walk)->nextUpdateChain);
      }
      *walk = 0;
      
      // for any flags we haven't updated since this (dropped) packet
      // or them into the mask so they'll get updated soon

      orFlags = packRef->mask & ~orFlags;

      if(orFlags)
      {
         if(!packRef->ghost->updateMask)
         {
            packRef->ghost->updateMask = orFlags;
            ghostPushNonZero(packRef->ghost);
         }
         else
            packRef->ghost->updateMask |= orFlags;
      }
      
      // if this packet was ghosting an object, set it
      // to re ghost at it's earliest convenience

      if(packRef->ghostInfoFlags & GhostInfo::Ghosting)
      {
         packRef->ghost->flags |= GhostInfo::NotYetGhosted;
         packRef->ghost->flags &= ~GhostInfo::Ghosting;
      }
      
      // otherwise, if it was being deleted,
      // set it to re-delete

      else if(packRef->ghostInfoFlags & GhostInfo::KillingGhost)
      {
         packRef->ghost->flags |= GhostInfo::KillGhost;
         packRef->ghost->flags &= ~GhostInfo::KillingGhost;
      }

      delete packRef;
      packRef = temp;
   }
}

void NetConnection::ghostPacketReceived(PacketNotify *notify)
{
   GhostRef *packRef = notify->ghostList;

   // loop through all the notifies in this packet

   while(packRef)
   {
      GhostRef *temp = packRef->nextRef;

      AssertFatal(packRef->nextUpdateChain == NULL, "Out of order notify!!");

      // clear this notify from the end of the object's notify
      // chain

      GhostRef **walk = &(packRef->ghost->updateChain);
      while(*walk != packRef)
         walk = &((*walk)->nextUpdateChain);

      *walk = 0;
      
      // if this object was ghosting , it is now ghosted

      if(packRef->ghostInfoFlags & GhostInfo::Ghosting)
         packRef->ghost->flags &= ~GhostInfo::Ghosting;

      // otherwise, if it was dieing, free the ghost

      else if(packRef->ghostInfoFlags & GhostInfo::KillingGhost)
         freeGhostInfo(packRef->ghost);

      delete packRef;
      packRef = temp;
   }
}

struct UpdateQueueEntry
{
   F32 priority;
   GhostInfo *obj;

   UpdateQueueEntry(F32 in_priority, GhostInfo *in_obj) 
      { priority = in_priority; obj = in_obj; }
};

static S32 QSORT_CALLBACK UQECompare(const void *a,const void *b)
{
   GhostInfo *ga = *((GhostInfo **) a);
   GhostInfo *gb = *((GhostInfo **) b);

   F32 ret = ga->priority - gb->priority;
   return (ret < 0) ? -1 : ((ret > 0) ? 1 : 0);
} 

void NetConnection::ghostWritePacket(BitStream *bstream, PacketNotify *notify)
{
#ifdef    DEBUG_NET
   bstream->writeInt(DebugChecksum, 32);
#endif

   notify->ghostList = NULL;
   
   if(!mGhostFrom)
      return;
   
   if(!bstream->writeFlag(mGhosting))
      return;
      
   // fill a packet (or two) with ghosting data

   // first step is to check all our polled ghosts:

   // 1. Scope query - find if any new objects have come into
   //    scope and if any have gone out.
   // 2. call scoped objects' priority functions if the flag set is nonzero
   //    A removed ghost is assumed to have a high priority
   // 3. call updates based on sorted priority until the packet is
   //    full.  set flags to zero for all updated objects

   CameraScopeQuery camInfo;
   
   camInfo.camera = NULL;
   camInfo.pos.set(0,0,0);
   camInfo.orientation.set(0,1,0);
   camInfo.visibleDistance = 1;
   camInfo.fov = (F32)(3.1415f / 4.0f);
   camInfo.sinFov = 0.7071f;
   camInfo.cosFov = 0.7071f;

   GhostInfo *walk;

   // only need to worry about the ghosts that have update masks set...
   S32 maxIndex = 0;
   S32 i;
   for(i = 0; i < mGhostZeroUpdateIndex; i++)
   {
      // increment the updateSkip for everyone... it's all good
      walk = mGhostArray[i];
      walk->updateSkipCount++;
      if(!(walk->flags & (GhostInfo::ScopeAlways | GhostInfo::ScopeLocalAlways)))
         walk->flags &= ~GhostInfo::InScope;
   }

   if(mScopeObject)
      mScopeObject->onCameraScopeQuery(this, &camInfo);

   for(i = mGhostZeroUpdateIndex - 1; i >= 0; i--)
   {
      if(!(mGhostArray[i]->flags & GhostInfo::InScope))
         detachObject(mGhostArray[i]);
   }

   for(i = mGhostZeroUpdateIndex - 1; i >= 0; i--)
   {
      walk = mGhostArray[i];
      if(walk->index > maxIndex)
         maxIndex = walk->index;

      // clear out any kill objects that haven't been ghosted yet
      if((walk->flags & GhostInfo::KillGhost) && (walk->flags & GhostInfo::NotYetGhosted))
      {
         freeGhostInfo(walk);
         continue;
      }
      // don't do any ghost processing on objects that are being killed
      // or in the process of ghosting
      else if(!(walk->flags & (GhostInfo::KillingGhost | GhostInfo::Ghosting)))
      {
         if(walk->flags & GhostInfo::KillGhost)
            walk->priority = 10000;
         else
            walk->priority = walk->obj->getUpdatePriority(&camInfo, walk->updateMask, walk->updateSkipCount);
      }
      else
         walk->priority = 0;
   }
   GhostRef *updateList = NULL;
   dQsort(mGhostArray, mGhostZeroUpdateIndex, sizeof(GhostInfo *), UQECompare);
   // reset the array indices...
   for(i = mGhostZeroUpdateIndex - 1; i >= 0; i--)
      mGhostArray[i]->arrayIndex = i;

   S32 sendSize = 1;
   while(maxIndex >>= 1)
      sendSize++;

   if(sendSize < 3)
      sendSize = 3;

   bstream->writeInt(sendSize - 3, 3); // 0-7 3 bit number

   U32 count = 0;
   // 
   for(i = mGhostZeroUpdateIndex - 1; i >= 0 && !bstream->isFull(); i--)
   {
      GhostInfo *walk = mGhostArray[i];
		if(walk->flags & (GhostInfo::KillingGhost | GhostInfo::Ghosting))
		   continue;
		   
      //S32 startPos = bstream->getCurPos();
      bstream->writeFlag(true);

      bstream->writeInt(walk->index, sendSize);
      U32 updateMask = walk->updateMask;
      
      GhostRef *upd = new GhostRef;

      upd->nextRef = updateList;
      updateList = upd;
      upd->nextUpdateChain = walk->updateChain;
      walk->updateChain = upd;

      upd->ghost = walk;
      upd->ghostInfoFlags = 0;

      if(walk->flags & GhostInfo::KillGhost)
      {
         walk->flags &= ~GhostInfo::KillGhost;
         walk->flags |= GhostInfo::KillingGhost;
         walk->updateMask = 0;
         upd->mask = updateMask;
         ghostPushToZero(walk);
         upd->ghostInfoFlags = GhostInfo::KillingGhost;
         bstream->writeFlag(true); // killing ghost
      }
      else 
      {
         bstream->writeFlag(false);
         U32 startPos = bstream->getCurPos();
         if(walk->flags & GhostInfo::NotYetGhosted)
         {
            S32 classId = walk->obj->getClassId();

            AssertFatal(classId >= NetObjectClassFirst && classId <= NetObjectClassLast, "Bad ghost tag.");

            walk->flags &= ~GhostInfo::NotYetGhosted;
            walk->flags |= GhostInfo::Ghosting;
            upd->ghostInfoFlags = GhostInfo::Ghosting;
            bstream->writeInt(classId - NetObjectClassFirst, NetObjectClassBitSize);
         }
#ifdef DEBUG_NET
         bstream->writeInt(walk->obj->getClassId(), 10);
         bstream->writeInt(walk->obj->getClassId() ^ DebugChecksum, 16);
#endif
         // update the object
         U32 retMask = walk->obj->packUpdate(this, updateMask, bstream);
         DEBUG_LOG(("PKLOG %d GHOST %d: %s", getId(), bstream->getCurPos() - 16 - startPos, walk->obj->getClassName()));

         AssertFatal((retMask & (~updateMask)) == 0, "Cannot set new bits in packUpdate return");

         walk->updateMask = retMask;
         if(!retMask)
            ghostPushToZero(walk);

         upd->mask = updateMask & ~retMask;

         //PacketStream::getStats()->addBits(PacketStats::Send, bstream->getCurPos() - startPos, walk->obj->getPersistTag());
#ifdef DEBUG_NET
         bstream->writeInt(walk->index ^ DebugChecksum, 16);
#endif
      }
      walk->updateSkipCount = 0;
      count++;
   }
   //Con::printf("Ghosts updated: %d (%d remain)", count, mGhostZeroUpdateIndex);
   // no more objects...
   bstream->writeFlag(false);
   notify->ghostList = updateList;
}

void NetConnection::ghostReadPacket(BitStream *bstream)
{
#ifdef    DEBUG_NET
   U32 sum = bstream->readInt(32);
   AssertISV(sum == DebugChecksum, "Invalid checksum.");
#endif

   if(!mGhostTo)
      return;
   if(!bstream->readFlag())
      return;

   S32 idSize;
   idSize = bstream->readInt( 3);
   idSize += 3;

   // while there's an object waiting...
   NetConnection *remoteConnection = NULL;
   if(mConnectionObjectId)
      remoteConnection = (NetConnection *) Sim::findObject(mConnectionObjectId);

   while(bstream->readFlag())
   {
      U32 index;
      //S32 startPos = bstream->getCurPos();
      index = (U32) bstream->readInt(idSize);
      if(bstream->readFlag()) // is this ghost being deleted?
      {
         AssertFatal(mLocalGhosts[index] != NULL, "Error, NULL ghost encountered.");
         mLocalGhosts[index]->deleteObject();
         mLocalGhosts[index] = NULL;
      }
      else
      {
         if(!mLocalGhosts[index]) // it's a new ghost... cool
         {
            U32 tag;

            tag = (U32) bstream->readInt(NetObjectClassBitSize) + NetObjectClassFirst;

            NetObject *obj = (NetObject *) ConsoleObject::create(tag);
            if(!obj)
            {
               setLastError("Invalid packet.");
               return;
            }
            obj->mNetFlags = NetObject::IsGhost;

            // object gets initial update before adding to the manager

            obj->mNetIndex = index;
            mLocalGhosts[index] = obj;
#ifdef DEBUG_NET
            U32 classId = bstream->readInt(10);
            U32 checksum = bstream->readInt(16);
            AssertISV(mLocalGhosts[index] != NULL, "Invalid dest ghost.");
            AssertISV( (checksum ^ DebugChecksum) == mLocalGhosts[index]->getClassId(),
               avar("class id mismatch for dest class %s.",
                  mLocalGhosts[index]->getClassName()) );          
#endif
            mLocalGhosts[index]->unpackUpdate(this, bstream);
            
            if(!obj->registerObject())
            {
               if(!mErrorBuffer[0])
                  setLastError("Invalid packet.");
               return;
            }
            if(remoteConnection)
               obj->mServerObject = remoteConnection->resolveGhostParent(index);

            addObject(obj);
         }
         else
         {
#ifdef DEBUG_NET
            U32 classId = bstream->readInt(10);
            U32 checksum = bstream->readInt(16);
            AssertISV(mLocalGhosts[index] != NULL, "Invalid dest ghost.");
            AssertISV( (checksum ^ DebugChecksum) == mLocalGhosts[index]->getClassId(),
               avar("class id mismatch for dest class %s.",
                  mLocalGhosts[index]->getClassName()) );          
#endif
            mLocalGhosts[index]->unpackUpdate(this, bstream);
         }
         //PacketStream::getStats()->addBits(PacketStats::Receive, bstream->getCurPos() - startPos, ghostRefs[index].localGhost->getPersistTag());
#ifdef DEBUG_NET
         U32 checksum = bstream->readInt(16);
         AssertISV( (checksum ^ DebugChecksum) == index,
            avar("unpackUpdate did not match packUpdate for object of class %s.",
               mLocalGhosts[index]->getClassName()) );          
#endif
         if(mErrorBuffer[0])
            return;
      }
   }
}

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------

void NetConnection::setScopeObject(NetObject *obj)
{
   if(((NetObject *) mScopeObject) == obj)
      return;
   mScopeObject = obj;
}

void NetConnection::detachObject(GhostInfo *info)
{
   // mark it for ghost killin'
   info->flags |= GhostInfo::KillGhost;

   // if the mask is in the zero range, we've got to move it up...
   if(!info->updateMask)
   {
      info->updateMask = 0xFFFFFFFF;
      ghostPushNonZero(info);
   }
   if(info->obj)
   {
      if(info->prevObjectRef)
         info->prevObjectRef->nextObjectRef = info->nextObjectRef;
      else
         info->obj->mFirstObjectRef = info->nextObjectRef;
      if(info->nextObjectRef)
         info->nextObjectRef->prevObjectRef = info->prevObjectRef;
      // remove it from the lookup table
      
      U32 id = info->obj->getId();
      for(GhostInfo **walk = &mGhostLookupTable[id & (GhostLookupTableSize - 1)]; *walk; walk = &((*walk)->nextLookupInfo))
      {
         GhostInfo *temp = *walk;
         if(temp == info)
         {
            *walk = temp->nextLookupInfo;
            break;
         }
      }
      info->prevObjectRef = info->nextObjectRef = NULL;
      info->obj = NULL;
   }
}

void NetConnection::freeGhostInfo(GhostInfo *ghost)
{
   AssertFatal(ghost->arrayIndex < mGhostFreeIndex, "Ghost already freed.");
   if(ghost->arrayIndex < mGhostZeroUpdateIndex)
   {
      AssertFatal(ghost->updateMask != 0, "Invalid ghost mask.");
      ghost->updateMask = 0;
      ghostPushToZero(ghost);
   }
   ghostPushZeroToFree(ghost);
   AssertFatal(ghost->updateChain == NULL, "Ack!");
}

//-----------------------------------------------------------------------------

void NetConnection::objectLocalScopeAlways(NetObject *obj)
{
   objectInScope(obj);
   for(GhostInfo *walk = mGhostLookupTable[obj->getId() & (GhostLookupTableSize - 1)]; walk; walk = walk->nextLookupInfo)
   {
      if(walk->obj != obj)
         continue;
      walk->flags |= GhostInfo::ScopeLocalAlways;
      return;
   }
}

void NetConnection::objectLocalClearAlways(NetObject *obj)
{
   for(GhostInfo *walk = mGhostLookupTable[obj->getId() & (GhostLookupTableSize - 1)]; walk; walk = walk->nextLookupInfo)
   {
      if(walk->obj != obj)
         continue;
      walk->flags &= ~GhostInfo::ScopeLocalAlways;
      return;
   }
}

bool NetConnection::validateGhostArray()
{
   AssertFatal(mGhostZeroUpdateIndex >= 0 && mGhostZeroUpdateIndex <= mGhostFreeIndex, "Invalid update index range.");
   AssertFatal(mGhostFreeIndex <= MaxGhostCount, "Invalid free index range.");
   U32 i;
   for(i = 0; i < mGhostZeroUpdateIndex; i ++)
   {
      AssertFatal(mGhostArray[i]->arrayIndex == i, "Invalid array index.");
      AssertFatal(mGhostArray[i]->updateMask != 0, "Invalid ghost mask.");
   }
   for(; i < mGhostFreeIndex; i ++)
   {
      AssertFatal(mGhostArray[i]->arrayIndex == i, "Invalid array index.");
      AssertFatal(mGhostArray[i]->updateMask == 0, "Invalid ghost mask.");
   }
   for(; i < MaxGhostCount; i++)
   {
      AssertFatal(mGhostArray[i]->arrayIndex == i, "Invalid array index.");
   }
   return true;
}

void NetConnection::objectInScope(NetObject *obj)
{
   if(!mScoping)
      return;
	if (obj->isScopeLocal() && !mConnectionObjectId)
		return;
   S32 index = obj->getId() & (GhostLookupTableSize - 1);
   
   // check if it's already in scope
   // the object may have been cleared out without the lookupTable being cleared
   // so validate that the object pointers are the same.

   for(GhostInfo *walk = mGhostLookupTable[index ]; walk; walk = walk->nextLookupInfo)
   {
      if(walk->obj != obj)
         continue;
      walk->flags |= GhostInfo::InScope;
      return;
   }

   if (mGhostFreeIndex == MaxGhostCount)
      return;

   GhostInfo *giptr = mGhostArray[mGhostFreeIndex];
   ghostPushFreeToZero(giptr);
   giptr->updateMask = 0xFFFFFFFF;
   ghostPushNonZero(giptr);

   giptr->flags = GhostInfo::NotYetGhosted | GhostInfo::InScope;
   
   if(obj->mNetFlags.test(NetObject::ScopeAlways))
      giptr->flags |= GhostInfo::ScopeAlways;

   giptr->obj = obj;
   giptr->updateChain = NULL;
   giptr->updateSkipCount = 0;

   giptr->connection = this;

   giptr->nextObjectRef = obj->mFirstObjectRef;
   if(obj->mFirstObjectRef)
      obj->mFirstObjectRef->prevObjectRef = giptr;
   giptr->prevObjectRef = NULL;
   obj->mFirstObjectRef = giptr;
   
   giptr->nextLookupInfo = mGhostLookupTable[index];
   mGhostLookupTable[index] = giptr;
   //AssertFatal(validateGhostArray(), "Invalid ghost array!");
}

//-----------------------------------------------------------------------------

void NetConnection::handleGhostMessage(S32 message, U32 sequence, U32 ghostCount)
{
   if((message == GhostAlwaysStarting || message == GhostAlwaysDone || message == EndGhosting) && !mGhostTo)
   {
      setLastError("Invalid packet.");
      return;
   }
   
   S32 i;
   switch(message)
   {
      case GhostAlwaysDone:
         mGhostingSequence = sequence;
         postNetEvent(new GhostingMessageEvent(ReadyForNormalGhosts, sequence));
         break;
      case ReadyForNormalGhosts:
         if(sequence != mGhostingSequence)
            return;
         mGhosting = true;
         for(i = 0; i < mGhostFreeIndex; i++)
         {
            if(mGhostArray[i]->flags & GhostInfo::ScopedEvent)
               mGhostArray[i]->flags &= ~(GhostInfo::Ghosting | GhostInfo::ScopedEvent);
         }
         break;
      case EndGhosting:
         // just delete all the local ghosts
         for(i = 0; i < MaxGhostCount; i++)
         {
            if(mLocalGhosts[i])
            {
               mLocalGhosts[i]->deleteObject();
               mLocalGhosts[i] = NULL;
            }
         }
         break;
      case GhostAlwaysStarting:
         Con::executef(2, "ghostAlwaysStarted", Con::getIntArg(ghostCount));
         break;
   }
}

void NetConnection::activateGhosting()
{
   if(!mGhostFrom)
      return;

   mGhostingSequence++;
   
   // iterate through the ghost always objects and InScope them...
   // also post em all to the other side.
   
   SimSet* ghostAlwaysSet = Sim::getGhostAlwaysSet();

   SimSet::iterator i;

   AssertFatal((mGhostFreeIndex == 0) && (mGhostZeroUpdateIndex == 0), "Error: ghosts in the ghost list before activate.");

   U32 sz = ghostAlwaysSet->size();
   S32 j;

   for(j = 0; j < sz; j++)
   {
      U32 idx = MaxGhostCount - sz + j;
      mGhostArray[j] = mGhostRefs + idx;
      mGhostArray[j]->arrayIndex = j;
   }
   for(j = sz; j < MaxGhostCount; j++)
   {
      U32 idx = j - sz;
      mGhostArray[j] = mGhostRefs + idx;
      mGhostArray[j]->arrayIndex = j;
   }
   mScoping = true; // so that objectInScope will work
   for(i = ghostAlwaysSet->begin(); i != ghostAlwaysSet->end(); i++)
   {
      AssertFatal(dynamic_cast<NetObject *>(*i) != NULL, avar("Non NetObject in GhostAlwaysSet: %s", (*i)->getClassName()));
      NetObject *obj = (NetObject *)(*i);
      if(obj->mNetFlags.test(NetObject::Ghostable))
         objectInScope(obj);
   }
   postNetEvent(new GhostingMessageEvent(GhostAlwaysStarting, mGhostingSequence, ghostAlwaysSet->size()));
   for(j = mGhostZeroUpdateIndex - 1; j >= 0; j--)
   {
      AssertFatal((mGhostArray[j]->flags & GhostInfo::ScopeAlways) != 0, "Non-scope always in the scope always list.")

      // we may end up resending state here, but at least initial state
      // will not be resent.
      mGhostArray[j]->updateMask = 0;
      ghostPushToZero(mGhostArray[j]);
      mGhostArray[j]->flags &= ~GhostInfo::NotYetGhosted;
      mGhostArray[j]->flags |= GhostInfo::ScopedEvent;

      postNetEvent(new GhostAlwaysObjectEvent(mGhostArray[j]->obj, mGhostArray[j]->index));
   }
   postNetEvent(new GhostingMessageEvent(GhostAlwaysDone, mGhostingSequence));
   //AssertFatal(validateGhostArray(), "Invalid ghost array!");
}

void NetConnection::clearGhostInfo()
{
   // gotta clear out the ghosts...
   for(PacketNotify *walk = mNotifyQueueHead; walk; walk = walk->nextPacket)
   {
      ghostPacketReceived(walk);
      walk->ghostList = NULL;
   }
   for(S32 i = 0; i < MaxGhostCount; i++)
   {
      if(mGhostRefs[i].arrayIndex < mGhostFreeIndex)
      {
         detachObject(&mGhostRefs[i]);
         freeGhostInfo(&mGhostRefs[i]);
      }
   }
   AssertFatal((mGhostFreeIndex == 0) && (mGhostZeroUpdateIndex == 0), "Invalid indices.");
}

void NetConnection::resetGhosting()
{
   if(!mGhostFrom)
      return;
   // stop all ghosting activity
   // send a message to the other side notifying of this
   
   mGhosting = false;
   mScoping = false;
   postNetEvent(new GhostingMessageEvent(EndGhosting, mGhostingSequence));
   mGhostingSequence++;
   clearGhostInfo();
   //AssertFatal(validateGhostArray(), "Invalid ghost array!");
}

void NetConnection::setGhostAlwaysObject(NetObject *object, U32 index)
{
   if(!mGhostTo)
   {
      object->deleteObject();
      setLastError("Invalid packet.");
      return;
   }

   AssertFatal(mLocalGhosts[index] == NULL, "Ghost already in table!");
   object->mNetFlags = NetObject::IsGhost;
   if(!object->registerObject())
   {
      if(!mErrorBuffer[0])
         setLastError("Invalid packet.");
      return;
   }
   addObject(object);
   
   mLocalGhosts[index] = object;
}

//-----------------------------------------------------------------------------

NetObject *NetConnection::resolveGhost(S32 id)
{
   return mLocalGhosts[id];
}

NetObject *NetConnection::resolveGhostParent(S32 id)
{
   return mGhostRefs[id].obj;
}

S32 NetConnection::getGhostIndex(NetObject *obj)
{
   if(!mGhostFrom)
      return obj->mNetIndex;
   S32 index = obj->getId() & (GhostLookupTableSize - 1);

   for(GhostInfo *gptr = mGhostLookupTable[index]; gptr; gptr = gptr->nextLookupInfo)
   {
      if(gptr->obj == obj && (gptr->flags & (GhostInfo::KillingGhost | GhostInfo::Ghosting | GhostInfo::NotYetGhosted | GhostInfo::KillGhost)) == 0)
         return gptr->index;
   }
   return -1;
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

void NetConnection::ghostWriteStartBlock(ResizeBitStream *stream)
{
   stream->write(mGhostingSequence);
   for(U32 i = 0; i < MaxGhostCount; i++)
   {
      if(mLocalGhosts[i])
      {
         stream->writeFlag(true);
         stream->writeInt(i, 10);
         stream->writeInt(mLocalGhosts[i]->getClassId() - NetObjectClassFirst, NetObjectClassBitSize);
         mLocalGhosts[i]->packUpdate(this, 0xFFFFFFFF, stream);
         stream->validate();
      }
   }
   stream->writeFlag(false);
}

void NetConnection::ghostReadStartBlock(BitStream *stream)
{
   stream->read(&mGhostingSequence);
   while(stream->readFlag())
   {
      U32 index = stream->readInt(10);
      U32 tag = (U32) stream->readInt(NetObjectClassBitSize) + NetObjectClassFirst;
      NetObject *obj = (NetObject *) ConsoleObject::create(tag);
      if(!obj)
      {
         setLastError("Invalid packet.");
         return;
      }
      obj->mNetFlags = NetObject::IsGhost;
      obj->mNetIndex = index;
      mLocalGhosts[index] = obj;
      mLocalGhosts[index]->unpackUpdate(this, stream);
      if(!obj->registerObject())
      {
         if(mErrorBuffer[0])
            setLastError("Invalid packet.");
         return;
      }
      addObject(obj);
   }
}
