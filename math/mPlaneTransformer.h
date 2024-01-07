//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _MPLANETRANSFORMER_H_
#define _MPLANETRANSFORMER_H_

#ifndef _MMATRIX_H_
#include "Math/mMatrix.h"
#endif
#ifndef _MPOINT_H_
#include "Math/mPoint.h"
#endif
#ifndef _MPLANE_H_
#include "Math/mPlane.h"
#endif

// =========================================================
class PlaneTransformer
{
   MatrixF mTransform;
   Point3F mScale;
   
   MatrixF mTransposeInverse;

  public:
   void set(const MatrixF& xform, const Point3F& scale);
   void transform(const PlaneF& plane, PlaneF& result);
   void setIdentity();
};

#endif
