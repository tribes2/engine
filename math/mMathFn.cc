//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "Math/mMathFn.h"
#include "Math/mPlane.h"
#include "Math/mMatrix.h"


void mTransformPlane(const MatrixF& mat,
                     const Point3F& scale,
                     const PlaneF&  plane,
                     PlaneF*        result)
{
   m_matF_x_scale_x_planeF(mat, &scale.x, &plane.x, &result->x);
}


