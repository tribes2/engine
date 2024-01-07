//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "Sim/pathManager.h"
#include "Sim/netConnection.h"
#include "Core/bitStream.h"
#include "interior/interiorInstance.h"
#include "Math/mathIO.h"

namespace {

U32 countNumBits(U32 n)
{
   U32 count = 0;
   while (n != 0) {
      n >>= 1;
      count++;
   }

   return count ? count : 1;
}

} // namespace {}



//--------------------------------------------------------------------------
//-------------------------------------- PathManagerEvent
//
class PathManagerEvent : public NetEvent
{
  public:
   enum MessageType {
      NewPaths,
      ModifyPath
   };

   MessageType message;
   U32         modifiedPath;

   Vector<PathManager::PathEntry*> mPaths;

  public:
   PathManagerEvent() {
      VECTOR_SET_ASSOCIATION(mPaths);
   }

   void pack(NetConnection*, BitStream*);
   void write(NetConnection*, BitStream*);
   void unpack(NetConnection*, BitStream*);
   void process(NetConnection*);

   DECLARE_CONOBJECT(PathManagerEvent);
};

void PathManagerEvent::pack(NetConnection*, BitStream* stream)
{ 
   AssertFatal(mPaths.size() == 0, "Hm, bad");

   if (stream->writeFlag(message == NewPaths) == true) {
      // Write out all the new paths...
      stream->write(gServerPathManager->mPaths.size());
      for (U32 i = 0; i < gServerPathManager->mPaths.size(); i++) {
         stream->write(gServerPathManager->mPaths[i]->totalTime);

         stream->write(gServerPathManager->mPaths[i]->positions.size());
         for (U32 j = 0; j < gServerPathManager->mPaths[i]->positions.size(); j++) {
            mathWrite(*stream, gServerPathManager->mPaths[i]->positions[j]);
            stream->write(gServerPathManager->mPaths[i]->msToNext[j]);
         }
      }
   } else {
      // Write out the modified path...
      stream->write(modifiedPath);
      stream->write(gServerPathManager->mPaths[modifiedPath]->totalTime);
      stream->write(gServerPathManager->mPaths[modifiedPath]->positions.size());
      for (U32 j = 0; j < gServerPathManager->mPaths[modifiedPath]->positions.size(); j++) {
         mathWrite(*stream, gServerPathManager->mPaths[modifiedPath]->positions[j]);
         stream->write(gServerPathManager->mPaths[modifiedPath]->msToNext[j]);
      }
   }
}

void PathManagerEvent::write(NetConnection*, BitStream *stream)
{
   if (stream->writeFlag(message == NewPaths) == true) {
      // Write out all the new paths...
      stream->write(mPaths.size());
      for (U32 i = 0; i < mPaths.size(); i++) {
         stream->write(mPaths[i]->totalTime);

         stream->write(mPaths[i]->positions.size());
         for (U32 j = 0; j < mPaths[i]->positions.size(); j++) {
            mathWrite(*stream, mPaths[i]->positions[j]);
            stream->write(mPaths[i]->msToNext[j]);
         }
      }
   } else {
      // Write out the modified path...
      stream->write(modifiedPath);
      stream->write(mPaths[0]->totalTime);
      stream->write(mPaths[0]->positions.size());
      for (U32 j = 0; j < mPaths[0]->positions.size(); j++) {
         mathWrite(*stream, mPaths[0]->positions[j]);
         stream->write(mPaths[0]->msToNext[j]);
      }
   }
}

void PathManagerEvent::unpack(NetConnection*, BitStream* stream)
{ 
   AssertFatal(mPaths.size() == 0, "Hm, bad");

   message = stream->readFlag() ? NewPaths : ModifyPath;

   if (message == NewPaths) {
      // Read in all the paths
      U32 numPaths, i;
      stream->read(&numPaths);
      mPaths.setSize(numPaths);
      for (i = 0; i < mPaths.size(); i++)
         mPaths[i] = new PathManager::PathEntry;
      
      for (i = 0; i < mPaths.size(); i++) {
         PathManager::PathEntry& rEntry = *(mPaths[i]);

         stream->read(&rEntry.totalTime);

         U32 numPoints;
         stream->read(&numPoints);
         rEntry.positions.setSize(numPoints);
         rEntry.msToNext.setSize(numPoints);
         for (U32 j = 0; j < rEntry.positions.size(); j++) {
            mathRead(*stream, &rEntry.positions[j]);
            stream->read(&rEntry.msToNext[j]);
         }
      }
   } else {
      // Read in the modified path...
      stream->read(&modifiedPath);
      AssertFatal(modifiedPath <= gClientPathManager->mPaths.size(), "Error out of bounds path!");

      mPaths.push_back(new PathManager::PathEntry);
      PathManager::PathEntry& rEntry = *(mPaths[0]);

      stream->read(&rEntry.totalTime);
      
      U32 numPoints;
      stream->read(&numPoints);
      rEntry.positions.setSize(numPoints);
      rEntry.msToNext.setSize(numPoints);
      for (U32 j = 0; j < rEntry.positions.size(); j++) {
         mathRead(*stream, &rEntry.positions[j]);
         stream->read(&rEntry.msToNext[j]);
      }
   }
}

void PathManagerEvent::process(NetConnection*)
{
   if (message == NewPaths) {
      // Destroy all previous paths...
      U32 i;
      for (i = 0; i < gClientPathManager->mPaths.size(); i++)
         delete gClientPathManager->mPaths[i];

      gClientPathManager->mPaths.setSize(mPaths.size());
      for (i = 0; i < mPaths.size(); i++) {
         gClientPathManager->mPaths[i] = mPaths[i];
         mPaths[i] = NULL;
      }
   } else {
      if (modifiedPath == gClientPathManager->mPaths.size()) {
         gClientPathManager->mPaths.push_back(mPaths[0]);
         mPaths[0] = NULL;
      } else {
         delete gClientPathManager->mPaths[modifiedPath];
         gClientPathManager->mPaths[modifiedPath] = mPaths[0];
         mPaths[0] = NULL;
      }
   }
}

IMPLEMENT_CO_NETEVENT_V1(PathManagerEvent);


//--------------------------------------------------------------------------
//-------------------------------------- PathManager Implementation
//
PathManager* gClientPathManager = NULL;
PathManager* gServerPathManager = NULL;

//--------------------------------------------------------------------------
PathManager::PathManager(const bool isServer)
{
   VECTOR_SET_ASSOCIATION(mPaths);

   mIsServer  = isServer;
}

PathManager::~PathManager()
{
   for (U32 i = 0; i < mPaths.size(); i++)
      delete mPaths[i];
}

void PathManager::init()
{
   AssertFatal(gClientPathManager == NULL && gServerPathManager == NULL, "Error, already initialized the path manager!");

   gClientPathManager = new PathManager(false);
   gServerPathManager = new PathManager(true);
}

void PathManager::destroy()
{
   AssertFatal(gClientPathManager != NULL && gServerPathManager != NULL, "Error, path manager not initialized!");

   delete gClientPathManager;
   gClientPathManager = NULL;
   delete gServerPathManager;
   gServerPathManager = NULL;
}


//--------------------------------------------------------------------------
U32 PathManager::allocatePathId()
{
   mPaths.increment();
   mPaths.last() = new PathEntry;

   return (mPaths.size() - 1);
}


void PathManager::updatePath(const U32              id,
                             const Vector<Point3F>& positions,
                             const Vector<U32>&     times)
{
   AssertFatal(mIsServer == true, "PathManager::updatePath: Error, must be called on the server side");
   AssertFatal(id < mPaths.size(), "PathManager::updatePath: error, id out of range");
   AssertFatal(positions.size() == times.size(), "Error, times and positions must match!");

   PathEntry& rEntry = *mPaths[id];

   rEntry.positions = positions;
   rEntry.msToNext  = times;

   rEntry.totalTime = 0;
   for (S32 i = 0; i < S32(rEntry.msToNext.size()) - 1; i++)
      rEntry.totalTime += rEntry.msToNext[i];

   transmitPath(id);
}


//--------------------------------------------------------------------------
void PathManager::transmitPaths(NetConnection* nc)
{
   AssertFatal(mIsServer, "Error, cannot call transmitPaths on client path manager!");

   // Send over paths
   PathManagerEvent* event = new PathManagerEvent;
   event->message          = PathManagerEvent::NewPaths;

   nc->postNetEvent(event);
}

void PathManager::transmitPath(const U32 id)
{
   AssertFatal(mIsServer, "Error, cannot call transmitNewPath on client path manager!");

   // Post to all active clients that have already received their paths...
   //
   SimGroup* pClientGroup = Sim::getClientGroup();
   for (SimGroup::iterator itr = pClientGroup->begin(); itr != pClientGroup->end(); itr++) {
      NetConnection* nc = dynamic_cast<NetConnection*>(*itr);
      if (nc && nc->missionPathsSent()) {
         // Transmit the updated path...
         PathManagerEvent* event = new PathManagerEvent;
         event->message          = PathManagerEvent::ModifyPath;
         event->modifiedPath     = id;
         nc->postNetEvent(event);
      }
   }
}

void PathManager::getPathPosition(const U32 id,
                                  const U32 msPosition,
                                  Point3F&  rPosition)
{
   AssertFatal(isValidPath(id), "Error, this is not a valid path!");

   // Ok, query holds our path information...
   U32 ms = msPosition;
   if (ms > mPaths[id]->totalTime)
      ms = mPaths[id]->totalTime;

   U32 startNode = 0;
   while (ms > mPaths[id]->msToNext[startNode]) {
      ms -= mPaths[id]->msToNext[startNode];
      startNode++;
   }
   U32 endNode = (startNode + 1) % mPaths[id]->positions.size();

   Point3F& rStart = mPaths[id]->positions[startNode];
   Point3F& rEnd   = mPaths[id]->positions[endNode];

   F32 interp = F32(ms) / F32(mPaths[id]->msToNext[startNode]);

   rPosition = (rStart * (1.0 - interp)) + (rEnd * interp);
}


U32 PathManager::getPathTotalTime(const U32 id) const
{
   AssertFatal(isValidPath(id), "Error, this is not a valid path!");

   return mPaths[id]->totalTime;
}

U32 PathManager::getPathNumWaypoints(const U32 id) const
{
   AssertFatal(isValidPath(id), "Error, this is not a valid path!");

   return mPaths[id]->positions.size();
}

U32 PathManager::getWaypointTime(const U32 id, const U32 wayPoint) const
{
   AssertFatal(isValidPath(id), "Error, this is not a valid path!");
   AssertFatal(wayPoint < getPathNumWaypoints(id), "Invalid waypoint!");
   
   U32 time = 0;
   for (U32 i = 0; i < wayPoint; i++)
      time += mPaths[id]->msToNext[i];

   return time;
}

U32 PathManager::getPathTimeBits(const U32 id)
{
   AssertFatal(isValidPath(id), "Error, this is not a valid path!");

   return countNumBits(mPaths[id]->totalTime);
}

U32 PathManager::getPathWaypointBits(const U32 id)
{
   AssertFatal(isValidPath(id), "Error, this is not a valid path!");

   return countNumBits(mPaths[id]->positions.size());
}


bool PathManager::dumpState(BitStream* stream) const
{
   stream->write(mPaths.size());

   for (U32 i = 0; i < mPaths.size(); i++) {
      const PathEntry& rEntry = *mPaths[i];
      stream->write(rEntry.totalTime);

      stream->write(rEntry.positions.size());
      for (U32 j = 0; j < rEntry.positions.size(); j++) {
         mathWrite(*stream, rEntry.positions[j]);
         stream->write(rEntry.msToNext[j]);
      }
   }

   return stream->getStatus() == Stream::Ok;
}

bool PathManager::readState(BitStream* stream)
{
   U32 i;
   for (i = 0; i < mPaths.size(); i++)
      delete mPaths[i];

   U32 numPaths;
   stream->read(&numPaths);
   mPaths.setSize(numPaths);

   for (i = 0; i < mPaths.size(); i++) {
      mPaths[i] = new PathEntry;
      PathEntry& rEntry = *mPaths[i];

      stream->read(&rEntry.totalTime);

      U32 numPositions;
      stream->read(&numPositions);
      rEntry.positions.setSize(numPositions);
      rEntry.msToNext.setSize(numPositions);
      for (U32 j = 0; j < rEntry.positions.size(); j++) {
         mathRead(*stream, &rEntry.positions[j]);
         stream->read(&rEntry.msToNext[j]);
      }
   }

   return stream->getStatus() == Stream::Ok;
}

