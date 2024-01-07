//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _MQUADPATCH_H_
#define _MQUADPATCH_H_

#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif
#ifndef _MPOINT_H_
#include "Math/mPoint.h"
#endif
#ifndef _MSPLINEPATCH_H_
#include "Math/mSplinePatch.h"
#endif

//------------------------------------------------------------------------------
// Quadratic spline patch
//------------------------------------------------------------------------------
class QuadPatch : public SplinePatch
{
   typedef SplinePatch Parent;

private:
   Point3F a, b, c;

   void calcABC( const Point3F *points );

public:

   QuadPatch();

   virtual void   calc( F32 t, Point3F &result );
   virtual void   calc( Point3F *points, F32 t, Point3F &result );
   virtual void   setControlPoint( Point3F &point, int index );
   virtual void   submitControlPoints( SplCtrlPts &points );
   

};



#endif
