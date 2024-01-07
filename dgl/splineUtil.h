//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _SPLINEUTIL_H_
#define _SPLINEUTIL_H_

#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif
#ifndef _MPOINT_H_
#include "Math/mPoint.h"
#endif
#ifndef _MSPLINEPATCH_H_
#include "Math/mSplinePatch.h"
#endif
#ifndef _COLOR_H_
#include "Core/color.h"
#endif

namespace SplineUtil
{

   struct SplineBeamInfo
   {
      Point3F *      camPos;
      U32            numSegments;
      F32            width;
      SplinePatch *  spline;
      F32            uvOffset;
      F32            numTexRep;
      ColorF         color;
      bool           zeroAlphaStart;  // first part of first segment has 0 alpha value

      SplineBeamInfo()
      {
         dMemset( this, 0, sizeof( SplineBeamInfo ) );
         numTexRep = 1.0;
      }

   };

   //------------------------------------------------------------------------------
   // Draws strip of specified width along spline.  Polys on strip segments are front-facing (billboarded)
   //------------------------------------------------------------------------------
   void drawSplineBeam( const Point3F& camPos, U32 numSegments, F32 width, 
                        SplinePatch &spline, F32 uvOffset = 0.0, F32 numTexRep = 1.0 );

   void drawSplineBeam( SplineBeamInfo &sbi );


}



#endif
