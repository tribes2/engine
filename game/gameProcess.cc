//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "core/dnet.h"
#include "game/gameConnection.h"
#include "game/gameBase.h"
#include "game/shapeBase.h"
#include "game/targetManager.h"
#include "platform/profiler.h"

//----------------------------------------------------------------------------

ProcessList gClientProcessList;
ProcessList gServerProcessList;

ProcessList::ProcessList()
{
   mDirty = false;
   mCurrentTag = 0;
   mLastTick = 0;
   mLastTime = 0;
   mLastDelta = 0;
}


//----------------------------------------------------------------------------

void ProcessList::orderList()
{
   // GameBase tags are intialized to 0, so current tag
   // should never be 0.
   if (!++mCurrentTag)
      mCurrentTag++;

   // Install a temporary head node
   GameBase list;
   list.plLinkBefore(head.mProcessLink.next);
   head.plUnlink();

   // Reverse topological sort into the orignal head node
   while (list.mProcessLink.next != &list) {
      GameBase* ptr = list.mProcessLink.next;
      ptr->mProcessTag = mCurrentTag;
      ptr->plUnlink();
      if (ptr->mAfterObject) {
         // Build chain "stack" of dependant objects and patch
         // it to the end of the current list.
         while (bool(ptr->mAfterObject) &&
               ptr->mAfterObject->mProcessTag != mCurrentTag) {
            ptr->mAfterObject->mProcessTag = mCurrentTag;
            ptr->mAfterObject->plUnlink();
            ptr->mAfterObject->plLinkBefore(ptr);
            ptr = ptr->mAfterObject;
         }
         ptr->plJoin(&head);
      }
      else
         ptr->plLinkBefore(&head);
   }
   mDirty = false;

   /*
   GameBase *walk = head.mProcessLink.next;
   Con::printf("Sorted process list.");
   while(walk != &head)
   {
      if (walk->mTypeMask & ShapeBaseObjectType)
      {
         ShapeBase *sb = (ShapeBase *) walk;
         if(sb->isMounted())
            Con::printf(" %s %d: mounted to %d", sb->getClassName(), sb->getId(), sb->getObjectMount()->getId());
         if(sb->getMountedObjectCount())
         {
            Con::printf(" %s %d: has %d mounted objects", sb->getClassName(), sb->getId(), sb->getMountedObjectCount());
            for(U32 i = 0; i < sb->getMountedObjectCount(); i++)
               Con::printf("    %s %d", sb->getMountedObject(i)->getClassName(),sb->getMountedObject(i)->getId());
         }
         if(sb->getControllingObject() != NULL)
            Con::printf(" %s %d: controlled by %d", sb->getClassName(), sb->getId(), sb->getControllingObject()->getId());
         if(sb->getProcessAfter())
            Con::printf(" %s %d: processes after %d", sb->getClassName(), sb->getId(), sb->getProcessAfter()->getId());
      }
      walk = walk->mProcessLink.next;
   }*/
}


//----------------------------------------------------------------------------

void ProcessList::advanceServerTime(SimTime timeDelta)
{
   PROFILE_START(AdvanceServerTime);

   if (mDirty) orderList();

   SimTime targetTime = mLastTime + timeDelta;
   SimTime targetTick = targetTime & ~TickMask;
   SimTime tickCount = (targetTick - mLastTick) >> TickShift;

   // Advance all the objects
   for (; mLastTick != targetTick; mLastTick += TickMs)
   {
      gTargetManager->tickSensorState();
      advanceObjects();
   }

   // Credit all the connections with the elapsed ticks.
   SimGroup *g = Sim::getClientGroup();
   for (SimGroup::iterator i = g->begin(); i != g->end(); i++)
      if (GameConnection *t = dynamic_cast<GameConnection *>(*i))
         t->incMoveCredit(tickCount);

   mLastTime = targetTime;
   PROFILE_END();
}


//----------------------------------------------------------------------------

void ProcessList::advanceClientTime(SimTime timeDelta)
{
   PROFILE_START(AdvanceClientTime);

   if (mDirty) orderList();

   SimTime targetTime = mLastTime + timeDelta;
   SimTime targetTick = (targetTime + TickMask) & ~TickMask;
   SimTime tickCount = (targetTick - mLastTick) >> TickShift;

   // See if the control object has pending moves.
   GameBase* control = 0;
   GameConnection* connection = GameConnection::getServerConnection();
   if (connection) {
      // If the connection to the server is backlogged
      // the simulation is frozen.
      if (connection->isBacklogged())
      {
         mLastTime = targetTime;
         mLastTick = targetTick;
         PROFILE_END();
         return;
      }
      if (connection->areMovesPending())
         control = connection->getControlObject();
   }

   // If we are going to tick, or have moves pending for the 
   // control object, we need to reset everyone back to their
   // last full tick pos.
   if (mLastDelta && (tickCount || control))
   {
      for (GameBase* obj = head.mProcessLink.next; obj != &head; 
            obj = obj->mProcessLink.next)
      {
         if (obj->mProcessTick)
            obj->interpolateTick(0);
      }
   }

   // Produce new moves and advance all the objects
   if (tickCount) {
      for (; mLastTick != targetTick; mLastTick += TickMs) {
         if(connection)
            connection->collectMove(mLastTick);
         advanceObjects();
      }
   }
   else
   {
      if (control) {
         // Sync up the control object with the latest client moves.
         Move* movePtr;
         U32 m = 0, numMoves;
         connection->getMoveList(&movePtr, &numMoves);
         while (m < numMoves)
            control->processTick(&movePtr[m++]);
         connection->clearMoves(m);
      }
   }

   mLastDelta = (TickMs - (targetTime & TickMask)) & TickMask;
   F32 dt = mLastDelta / F32(TickMs);
   for (GameBase* obj = head.mProcessLink.next; obj != &head;
         obj = obj->mProcessLink.next)
   {
      if (obj->mProcessTick)
         obj->interpolateTick(dt);
   }

   // Inform objects of total elapsed delta so they can advance
   // client side animations.
   dt = F32(timeDelta) / 1000;
   for (GameBase* obj = head.mProcessLink.next; obj != &head;
         obj = obj->mProcessLink.next)
   {
      obj->advanceTime(dt);
   }

   // update the hud targets
   gTargetList->update(targetTime);

   mLastTime = targetTime;

   PROFILE_END();
}


//----------------------------------------------------------------------------

void ProcessList::advanceObjects()
{
   PROFILE_START(AdvanceObjects);

   // A little link list shuffling is done here to avoid problems
   // with objects being deleted from within the process method.
   GameBase list;
   GameBase* obj;
   list.plLinkBefore(head.mProcessLink.next);
   head.plUnlink();
   while ((obj = list.mProcessLink.next) != &list)
   {
      obj->plUnlink();
      obj->plLinkBefore(&head);

      // Each object is either advanced a single tick, or if it's
      // being controlled by a client, ticked once for each pending move.
      if (obj->mTypeMask & ShapeBaseObjectType) {
         ShapeBase* pSB = static_cast<ShapeBase*>(obj);
         GameConnection* con = pSB->getControllingClient();
         if (con && con->getControlObject() == pSB) {
            Move* movePtr;
            U32 m,numMoves;
            con->getMoveList(&movePtr, &numMoves);
            for (m = 0; m < numMoves && pSB->getControllingClient() == con; )
               obj->processTick(&movePtr[m++]);
            con->clearMoves(m);
            continue;
         }
      }
      if (obj->mProcessTick)
         obj->processTick(0);
   }
   PROFILE_END();
}
