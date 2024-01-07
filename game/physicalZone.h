//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _H_PHYSICALZONE
#define _H_PHYSICALZONE

#ifndef _SCENEOBJECT_H_
#include "Sim/sceneObject.h"
#endif
#ifndef _EARLYOUTPOLYLIST_H_
#include "Collision/earlyOutPolyList.h"
#endif


class Convex;

// -------------------------------------------------------------------------
class PhysicalZone : public SceneObject
{
   typedef SceneObject Parent;

  protected:
   bool onAdd();
   void onRemove();

   enum UpdateMasks {
      InitialUpdateMask = 1 << 0,
      ActiveMask        = 1 << 1
   };

  public:
   void setTransform(const MatrixF&);

  protected:
   F32        mVelocityMod;
   F32        mGravityMod;
   Point3F    mAppliedForce;

   // Basically ripped from trigger
   Polyhedron           mPolyhedron;
   EarlyOutPolyList     mClippedList;

   bool mActive;

   Convex* mConvexList;
   void buildConvex(const Box3F& box, Convex* convex);

  public:
   PhysicalZone();
   ~PhysicalZone();

   F32 getVelocityMod() const      { return mVelocityMod; }
   F32 getGravityMod()  const      { return mGravityMod;  }
   const Point3F& getForce() const { return mAppliedForce; }

   void setPolyhedron(const Polyhedron&);
   bool testObject(SceneObject*);

   void activate();
   void deactivate();
   bool isActive() const { return mActive; }

   DECLARE_CONOBJECT(PhysicalZone);
   static void initPersistFields();
   static void consoleInit();

   U32  packUpdate(NetConnection*, U32 mask, BitStream* stream);
   void unpackUpdate(NetConnection*, BitStream* stream);
};

#endif // _H_PHYSICALZONE

