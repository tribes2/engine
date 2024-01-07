//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _CONVEX_H_
#define _CONVEX_H_

#ifndef _MMATH_H_
#include "Math/mMath.h"
#endif
#ifndef _TVECTOR_H_
#include "Core/tVector.h"
#endif
#ifndef _GJK_H_
#include "Collision/gjk.h"
#endif

struct Collision;
struct CollisionList;
class AbstractPolyList;
class SceneObject;
class Convex;


//----------------------------------------------------------------------------

class ConvexFeature
{
public:
   struct Edge {
      S32 vertex[2];
   };
   struct Face {
      VectorF normal;
      S32 vertex[3];
   };

   Vector<Point3F> mVertexList;
   Vector<Edge> mEdgeList;
   Vector<Face> mFaceList;
   S32 material;
   SceneObject* object;

   ConvexFeature() : mVertexList(64), mEdgeList(128), mFaceList(64) {
      VECTOR_SET_ASSOCIATION(mVertexList);
      VECTOR_SET_ASSOCIATION(mEdgeList);
      VECTOR_SET_ASSOCIATION(mFaceList);
   }

   bool collide(ConvexFeature& cf,CollisionList* cList, F32 tol = 0.1);
   void testVertex(const Point3F& v,CollisionList* cList,bool,F32 tol);
   void testEdge(const Point3F& s1, const Point3F& e1, CollisionList* cList, F32 tol);
};


//----------------------------------------------------------------------------

enum ConvexType {
   TSConvexType,
   BoxConvexType,
   TerrainConvexType,
   InteriorConvexType,
   ShapeBaseConvexType,
   TSStaticConvexType
};


//----------------------------------------------------------------------------

struct CollisionStateList
{
   static CollisionStateList sFreeList;
   CollisionStateList* mNext;
   CollisionStateList* mPrev;
   CollisionState* mState;

   CollisionStateList();
   void linkAfter(CollisionStateList* next);
   void unlink();
   bool isEmpty() { return mNext == this; }

   static CollisionStateList* alloc();
   void free();
};


//----------------------------------------------------------------------------

struct CollisionWorkingList
{
   static CollisionWorkingList sFreeList;
   struct WLink {
      CollisionWorkingList* mNext;
      CollisionWorkingList* mPrev;
   } wLink;
   struct RLink {
      CollisionWorkingList* mNext;
      CollisionWorkingList* mPrev;
   } rLink;
   Convex* mConvex;

   void wLinkAfter(CollisionWorkingList* next);
   void rLinkAfter(CollisionWorkingList* next);
   void unlink();
   CollisionWorkingList();

   static CollisionWorkingList* alloc();
   void free();
};


//----------------------------------------------------------------------------

class Convex {
   Convex* mNext;
   Convex* mPrev;

   void linkAfter(Convex* next);
   void unlink();

   U32 mTag;
   static U32 sTag;

protected:
   CollisionStateList   mList;
   CollisionWorkingList mWorking;
   CollisionWorkingList mReference;
   SceneObject* mObject;
   ConvexType mType;

   //
public:
   Convex();
   virtual ~Convex();

   // 
   void registerObject(Convex *convex);
   void collectGarbage();
   void nukeList();

   //
   ConvexType getType()      { return mType;   }
   SceneObject* getObject()  { return mObject; }

   void render();

   void                  addToWorkingList(Convex* ptr);
   CollisionWorkingList& getWorkingList() { return mWorking; }

   CollisionState* findClosestState(const MatrixF& mat, const Point3F& scale);
   CollisionState* findClosestStateBounded(const MatrixF& mat, const Point3F& scale, const F32 dontCareDist);

   CollisionStateList*   getStateList()   { return mList.mNext; }

   void updateStateList(const MatrixF& mat, const Point3F& scale, const Point3F* displacement = NULL);
   void updateWorkingList(const Box3F& box, const U32 colMask);

   virtual const MatrixF& getTransform() const;
   virtual const Point3F& getScale() const;
   virtual Box3F getBoundingBox() const;
   virtual Box3F getBoundingBox(const MatrixF& mat, const Point3F& scale) const;
   virtual Point3F support(const VectorF& v) const;
   virtual void getFeatures(const MatrixF& mat,const VectorF& n, ConvexFeature* cf);

   virtual void getPolyList(AbstractPolyList* list);

   bool getCollisionInfo(const MatrixF& mat, const Point3F& scale, CollisionList* cList,F32 tol);
};

#endif
