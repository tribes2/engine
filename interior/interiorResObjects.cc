//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "interior/interiorResObjects.h"
#include "Core/stream.h"
#include "Math/mathIO.h"

//--------------------------------------------------------------------------
//--------------------------------------
//
bool InteriorResTrigger::read(Stream& stream)
{
   U32 i, size;
   stream.readString(mName);

   // Read the polyhedron
   stream.read(&size);
   mPolyhedron.pointList.setSize(size);
   for (i = 0; i < mPolyhedron.pointList.size(); i++)
      mathRead(stream, &mPolyhedron.pointList[i]);

   stream.read(&size);
   mPolyhedron.planeList.setSize(size);
   for (i = 0; i < mPolyhedron.planeList.size(); i++)
      mathRead(stream, &mPolyhedron.planeList[i]);

   stream.read(&size);
   mPolyhedron.edgeList.setSize(size);
   for (i = 0; i < mPolyhedron.edgeList.size(); i++) {
      Polyhedron::Edge& rEdge = mPolyhedron.edgeList[i];

      stream.read(&rEdge.face[0]);
      stream.read(&rEdge.face[1]);
      stream.read(&rEdge.vertex[0]);
      stream.read(&rEdge.vertex[1]);
   }

   // And the offset
   mathRead(stream, &mOffset);

   return (stream.getStatus() == Stream::Ok);
}

bool InteriorResTrigger::write(Stream& stream) const
{
   U32 i;

   stream.writeString(mName);

   // Write the polyhedron
   stream.write(mPolyhedron.pointList.size());
   for (i = 0; i < mPolyhedron.pointList.size(); i++)
      mathWrite(stream, mPolyhedron.pointList[i]);

   stream.write(mPolyhedron.planeList.size());
   for (i = 0; i < mPolyhedron.planeList.size(); i++)
      mathWrite(stream, mPolyhedron.planeList[i]);

   stream.write(mPolyhedron.edgeList.size());
   for (i = 0; i < mPolyhedron.edgeList.size(); i++) {
      const Polyhedron::Edge& rEdge = mPolyhedron.edgeList[i];

      stream.write(rEdge.face[0]);
      stream.write(rEdge.face[1]);
      stream.write(rEdge.vertex[0]);
      stream.write(rEdge.vertex[1]);
   }

   // And the offset
   mathWrite(stream, mOffset);

   return (stream.getStatus() == Stream::Ok);
}


//--------------------------------------------------------------------------
//--------------------------------------
//
InteriorPath::InteriorPath()
{
   mName[0] = '\0';
   mWayPoints    = NULL;
   mNumWayPoints = 0;
   mLooping      = false;
}

InteriorPath::~InteriorPath()
{
   delete [] mWayPoints;
   mWayPoints = NULL;
   mNumWayPoints = 0;
}

bool InteriorPath::read(Stream& stream)
{
   AssertFatal(mWayPoints == NULL, "Hm, this is probably going to cause problems, reading a path twice...");

   stream.readString(mName);

   stream.read(&mNumWayPoints);
   mWayPoints = new WayPoint[mNumWayPoints];
   for (U32 i = 0; i < mNumWayPoints; i++) {
      mathRead(stream, &mWayPoints[i].pos);
      mathRead(stream, &mWayPoints[i].rot);
      stream.read(&mWayPoints[i].msToNext);
   }
   stream.read(&mTotalMS);
   stream.read(&mLooping);

   return (stream.getStatus() == Stream::Ok);
}

bool InteriorPath::write(Stream& stream) const
{
   stream.writeString(mName);

   stream.write(mNumWayPoints);
   for (U32 i = 0; i < mNumWayPoints; i++) {
      mathWrite(stream, mWayPoints[i].pos);
      mathWrite(stream, mWayPoints[i].rot);
      stream.write(mWayPoints[i].msToNext);
   }
   stream.write(mTotalMS);
   stream.write(mLooping);

   return (stream.getStatus() == Stream::Ok);
}


InteriorPathFollower::InteriorPathFollower()
{
   mName      = "";
   mPathIndex = 0;
   mOffset.set(0, 0, 0);
}

InteriorPathFollower::~InteriorPathFollower()
{

}

bool InteriorPathFollower::read(Stream& stream)
{
   mName = stream.readSTString();
   stream.read(&mInteriorResIndex);
   stream.read(&mPathIndex);
   mathRead(stream, &mOffset);

   U32 numTriggers;
   stream.read(&numTriggers);
   mTriggers.setSize(numTriggers);
   for (U32 i = 0; i < mTriggers.size(); i++)
      mTriggers[i] = stream.readSTString();

   return (stream.getStatus() == Stream::Ok);
}

bool InteriorPathFollower::write(Stream& stream) const
{
   stream.writeString(mName);
   stream.write(mInteriorResIndex);
   stream.write(mPathIndex);
   mathWrite(stream, mOffset);

   stream.write(mTriggers.size());
   for (U32 i = 0; i < mTriggers.size(); i++)
      stream.writeString(mTriggers[i]);

   return (stream.getStatus() == Stream::Ok);
}

AISpecialNode::AISpecialNode()
{
   mName = "";
   mPos.set(0, 0, 0);
}

AISpecialNode::~AISpecialNode()
{
}

bool AISpecialNode::read(Stream& stream)
{
   mName = stream.readSTString();
   mathRead(stream, &mPos);
   
   return (stream.getStatus() == Stream::Ok);
}

bool AISpecialNode::write(Stream& stream) const
{
   stream.writeString(mName);
   mathWrite(stream, mPos);
   
   return (stream.getStatus() == Stream::Ok);
}
