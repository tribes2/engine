//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _INTERIORRESOBJECTS_H_
#define _INTERIORRESOBJECTS_H_

#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif
#ifndef _MPOINT_H_
#include "Math/mPoint.h"
#endif
#ifndef _MBOX_H_
#include "Math/mBox.h"
#endif
#ifndef _MMATRIX_H_
#include "Math/mMatrix.h"
#endif
#ifndef _TVECTOR_H_
#include "Core/tVector.h"
#endif
#ifndef _POLYHEDRON_H_
#include "Collision/polyhedron.h"
#endif

class Stream;

class InteriorResTrigger
{
  public:
   enum Constants {
      MaxNameChars = 255
   };

   char       mName[MaxNameChars+1];

   Point3F    mOffset;
   Polyhedron mPolyhedron;

  public:
   InteriorResTrigger() { }
   
   bool read(Stream& stream);
   bool write(Stream& stream) const;
};

class InteriorPath
{
  public:
   struct WayPoint {
      Point3F  pos;
      QuatF    rot;
      U32      msToNext;
   };

  public:
   char             mName[256];
   WayPoint*        mWayPoints;
   U32              mNumWayPoints;
   U32              mTotalMS;
   bool             mLooping;

  public:
   InteriorPath();
   ~InteriorPath();

   bool read(Stream& stream);
   bool write(Stream& stream) const;
};

class InteriorPathFollower
{
  public:
   StringTableEntry         mName;
   U32                      mInteriorResIndex;
   U32                      mPathIndex;
   Point3F                  mOffset;
   Vector<StringTableEntry> mTriggers;

  public:
   InteriorPathFollower();
   ~InteriorPathFollower();

   bool read(Stream& stream);
   bool write(Stream& stream) const;
};


class AISpecialNode
{
   public:
      enum
      {
         chute = 0,
      };
   
   public:
      StringTableEntry  mName;
      Point3F           mPos;
      //U32               mType;
  
  public:
   AISpecialNode();
   ~AISpecialNode();

   bool read(Stream& stream);
   bool write(Stream& stream) const;
       
};

#endif  // _H_INTERIORRESOBJECTS_
