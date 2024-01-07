//-----------------------------------------------------------------------------
// Torque Game Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _VEHICLEBLOCKER_H_
#define _VEHICLEBLOCKER_H_

#ifndef _SCENEOBJECT_H_
#include "sim/sceneObject.h"
#endif
#ifndef _BOXCONVEX_H_
#include "collision/boxConvex.h"
#endif

//--------------------------------------------------------------------------
class VehicleBlocker : public SceneObject
{
   typedef SceneObject Parent;
   friend class VehicleBlockerConvex;

  protected:
   bool onAdd();
   void onRemove();

   // Collision
   void buildConvex(const Box3F& box, Convex* convex);
  protected:
   Convex* mConvexList;

   Point3F mDimensions;
   
  public:
   VehicleBlocker();
   ~VehicleBlocker();

   DECLARE_CONOBJECT(VehicleBlocker);
   static void initPersistFields();
   static void consoleInit();

   U32  packUpdate(NetConnection *, U32 mask, BitStream *stream);
   void unpackUpdate(NetConnection *, BitStream *stream);
};

#endif // _H_VEHICLEBLOCKER
