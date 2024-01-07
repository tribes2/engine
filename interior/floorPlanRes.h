//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _FLOORPLANRES_H_
#define _FLOORPLANRES_H_

#ifndef _RESMANAGER_H_
#include "Core/resManager.h"
#endif
#ifndef _MPOINT_H_
#include "Math/mPoint.h"
#endif
#ifndef _MPLANE_H_
#include "Math/mPlane.h"
#endif
class Stream;

class FloorPlanResource : public ResourceInstance
{
   typedef ResourceInstance Parent;
   static const U32 smFileVersion;
   
  public:
   struct Area    // basically a Winding, the info we need from it
   {
      S16      pointCount;
      S32      pointStart;
      S32      plane;
      Area(S16 C, S32 S, S32 P)  { pointCount=C; pointStart=S; plane=P;  }
   };

  protected:
   Vector<PlaneF>       mPlaneTable;
   Vector<Point3F>      mPointTable;
   Vector<S32>          mPointLists;
   Vector<Area>         mAreas;

  public:
   FloorPlanResource();
   ~FloorPlanResource();

   bool     read(Stream& stream);
   bool     write(Stream& stream) const;
};

extern ResourceInstance * constructFloorPlanFLR(Stream& stream);

#endif  // _H_FLOORPLANRES_
