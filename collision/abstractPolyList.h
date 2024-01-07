//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _ABSTRACTPOLYLIST_H_
#define _ABSTRACTPOLYLIST_H_

#ifndef _MMATH_H_
#include "Math/mMath.h"
#endif
#ifndef _MPLANETRANSFORMER_H_
#include "Math/mPlaneTransformer.h"
#endif
#ifndef _TVECTOR_H_
#include "Core/tVector.h"
#endif

class SceneObject;

//----------------------------------------------------------------------------

class AbstractPolyList
{
protected:
   // User set state
   SceneObject* mCurrObject;

   MatrixF  mBaseMatrix;
   MatrixF  mMatrix;
   Point3F  mScale;

   PlaneTransformer mPlaneTransformer;
   
   bool     mInterestNormalRegistered;
   Point3F  mInterestNormal;

public:
   AbstractPolyList();
   virtual ~AbstractPolyList();

   // Common functionality
   void setBaseTransform(const MatrixF& mat);

   void setTransform(const MatrixF* mat, const Point3F& scale);
   void getTransform(MatrixF* mat, Point3F * scale);
   void setObject(SceneObject*);
   void addBox(const Box3F &box);
   void doConstruct();
   
   // Interface functions
   virtual bool isEmpty() const = 0;
   virtual U32  addPoint(const Point3F& p) = 0;
   virtual U32  addPlane(const PlaneF& plane) = 0;
   virtual void begin(U32 material,U32 surfaceKey) = 0;
   virtual void plane(U32 v1,U32 v2,U32 v3) = 0;
   virtual void plane(const PlaneF& p) = 0;
   virtual void plane(const U32 index) = 0;
   virtual void vertex(U32 vi) = 0;
   virtual void end() = 0;
   virtual bool getMapping(MatrixF *, Box3F *);

   // Interest functionality
   void setInterestNormal(const Point3F& /*normal*/);
   void clearInterestNormal()                        { mInterestNormalRegistered = false; }
   virtual bool isInterestedInPlane(const PlaneF& /*plane*/);
   virtual bool isInterestedInPlane(const U32 index);

  protected:
   virtual const PlaneF& getIndexedPlane(const U32 index) = 0;
};

inline AbstractPolyList::AbstractPolyList()
{
   doConstruct();
}

inline void AbstractPolyList::doConstruct()
{
   mCurrObject = NULL;
   mBaseMatrix.identity();
   mMatrix.identity();
   mScale.set(1, 1, 1);

   mPlaneTransformer.setIdentity();

   mInterestNormalRegistered = false;
}

inline void AbstractPolyList::setBaseTransform(const MatrixF& mat)
{
   mBaseMatrix = mat;
}

inline void AbstractPolyList::setTransform(const MatrixF* mat, const Point3F& scale)
{
   mMatrix = mBaseMatrix;
   mMatrix.mul(*mat);
   mScale = scale;

   mPlaneTransformer.set(mMatrix, mScale);
}

inline void AbstractPolyList::getTransform(MatrixF* mat, Point3F * scale)
{
   *mat   = mMatrix;
   *scale = mScale;
}

inline void AbstractPolyList::setObject(SceneObject* obj)
{
   mCurrObject = obj;
}


#endif
