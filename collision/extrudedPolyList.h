//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _EXTRUDEDPOLYLIST_H_
#define _EXTRUDEDPOLYLIST_H_

#ifndef _MMATH_H_
#include "Math/mMath.h"
#endif
#ifndef _TVECTOR_H_
#include "Core/tVector.h"
#endif
#ifndef _ABSTRACTPOLYLIST_H_
#include "Collision/abstractPolyList.h"
#endif

struct Polyhedron;
struct CollisionList;

//----------------------------------------------------------------------------

class ExtrudedPolyList: public AbstractPolyList
{
public:
   struct Vertex {
      Point3F point;
      U32 mask;
   };

   struct Poly {
      PlaneF plane;
      SceneObject* object;
      U32 material;
   };

   struct ExtrudedFace {
      bool active;
      PlaneF plane;
      F32 maxDistance;
      U32 planeMask;
      F32 faceDot;
      F32 faceShift;
      F32 time;
      Point3F point;
      F32 height;
   };

   typedef Vector<ExtrudedFace> ExtrudedList;
   typedef Vector<PlaneF> PlaneList;
   typedef Vector<Vertex> VertexList;
   typedef Vector<U32> IndexList;

   static F32 EqualEpsilon;
   static F32 FaceEpsilon;

   // Internal data
   VertexList   mVertexList;
   IndexList    mIndexList;
   ExtrudedList mExtrudedList;
   PlaneList    mPlaneList;
   VectorF      mVelocity;
   VectorF      mNormalVelocity;
   F32          mFaceShift;
   Poly         mPoly;

   // Returned info
   CollisionList* mCollisionList;

   PlaneList mPolyPlaneList;
   
   //
private:
   bool testPoly(ExtrudedFace&);

public:
   ExtrudedPolyList();
   ~ExtrudedPolyList();
   void extrude(const Polyhedron&, const VectorF& vec);
   void setVelocity(const VectorF& velocity);
   void setCollisionList(CollisionList*);
   void adjustCollisionTime();
   void render();

   // Virtual methods
   bool isEmpty() const;
   U32  addPoint(const Point3F& p);
   U32  addPlane(const PlaneF& plane);
   void begin(U32 material, U32 /*surfaceKey*/);
   void plane(U32 v1,U32 v2,U32 v3);
   void plane(const PlaneF& p);
   void plane(const U32 index);
   void vertex(U32 vi);
   void end();

  protected:
   const PlaneF& getIndexedPlane(const U32 index);
};

#endif
