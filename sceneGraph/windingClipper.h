//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _WINDINGCLIPPER_H_
#define _WINDINGCLIPPER_H_

#ifndef _MMATH_H_
#include "Math/mMath.h"
#endif

   void sgUtil_clipToPlane(Point3F* points, U32& rNumPoints, const PlaneF& rPlane);

#endif
