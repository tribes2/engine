//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "dgl/splineUtil.h"

#include "PlatformWin32/platformGL.h"

namespace SplineUtil{


//------------------------------------------------------------------------------
// Draws strip of specified width along spline.  Polys on strip segments are 
//    front-facing (billboarded)
//------------------------------------------------------------------------------
void drawSplineBeam( const Point3F& camPos, U32 numSegments, 
                     F32 width, SplinePatch &spline, F32 uvOffset, F32 numTexRep )
{

   Point3F beginPoint, endPoint;
   spline.calc( 0.0, beginPoint );
   spline.calc( 1.0, endPoint );

   F32 approxBeamLength = (beginPoint - endPoint).len();
   F32 texRepFactor = approxBeamLength * numTexRep;


   glBegin(GL_TRIANGLE_STRIP);


   for( int i=0; i<numSegments; i++ )
   {
      F32 t = ((F32)i) / ((F32)(numSegments - 1));

      Point3F curPoint;
      spline.calc( t, curPoint );

      Point3F segmentDir;

      // handle last segment case
      Point3F nextPoint;
      if( i == (numSegments - 1) )
      {
         F32 modT = ((F32)(numSegments - 1)) / ((F32)numSegments);
         spline.calc( modT, nextPoint );
         segmentDir = curPoint - nextPoint;
      }
      else
      {
         F32 modT = t + (1.0 / numSegments);
         spline.calc( modT, nextPoint );
         segmentDir = nextPoint - curPoint;
      }

      if( segmentDir.isZero() ) continue;
      segmentDir.normalize();
         

      Point3F dirFromCam = curPoint - camPos;
      Point3F crossVec;
      mCross(dirFromCam, segmentDir, &crossVec);
      crossVec.normalize();

      crossVec *= width * 0.5;

      F32 u = uvOffset + texRepFactor * t;

      glTexCoord2f( u, 0.0 );
      glVertex3fv( curPoint + crossVec );

      glTexCoord2f( u, 1.0 );
      glVertex3fv( curPoint - crossVec );

   }

   glEnd();


}

//------------------------------------------------------------------------------
// Draws strip of specified width along spline.  Polys on strip segments are 
//    front-facing (billboarded)
//------------------------------------------------------------------------------
void drawSplineBeam( SplineBeamInfo &sbi )
{
   if( !sbi.camPos || !sbi.spline ) return;

   Point3F beginPoint, endPoint;
   sbi.spline->calc( 0.0, beginPoint );
   sbi.spline->calc( 1.0, endPoint );

   F32 approxBeamLength = (beginPoint - endPoint).len();
   F32 texRepFactor = approxBeamLength * sbi.numTexRep;


   glBegin(GL_TRIANGLE_STRIP);


   for( int i=0; i<sbi.numSegments; i++ )
   {
      F32 t = ((F32)i) / ((F32)(sbi.numSegments - 1));

      Point3F curPoint;
      sbi.spline->calc( t, curPoint );

      Point3F segmentDir;

      // handle last segment case
      Point3F nextPoint;
      if( i == (sbi.numSegments - 1) )
      {
         F32 modT = ((F32)(sbi.numSegments - 1)) / ((F32)sbi.numSegments);
         sbi.spline->calc( modT, nextPoint );
         segmentDir = curPoint - nextPoint;
      }
      else
      {
         F32 modT = t + (1.0 / sbi.numSegments);
         sbi.spline->calc( modT, nextPoint );
         segmentDir = nextPoint - curPoint;
      }

      if( segmentDir.isZero() ) continue;
      segmentDir.normalize();
         

      Point3F dirFromCam = curPoint - *sbi.camPos;
      Point3F crossVec;
      mCross(dirFromCam, segmentDir, &crossVec);
      crossVec.normalize();

      crossVec *= sbi.width * 0.5;

      F32 u = sbi.uvOffset + texRepFactor * t;

      if( i== 0 && sbi.zeroAlphaStart )
      {
         glColor4f( sbi.color.red, sbi.color.green, sbi.color.blue, 0.0 );
      }
      else
      {
         glColor4fv( sbi.color );
      }

      glTexCoord2f( u, 0.0 );
      glVertex3fv( curPoint + crossVec );

      glTexCoord2f( u, 1.0 );
      glVertex3fv( curPoint - crossVec );

   }

   glEnd();

}





} // end SplineUtil namespace
