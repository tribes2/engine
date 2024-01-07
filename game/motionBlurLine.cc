//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "Math/mMath.h"

#include "game/motionBlurLine.h"
#include "PlatformWin32/platformGL.h"
#include "dgl/dgl.h"

#define MAX_SEG_LENGTH 10.0

//**************************************************************************
// Motion Blur line
//**************************************************************************
MotionBlurLine::MotionBlurLine()
{
   mLifetime = 1.0;
   mWidth = 0.5;
   mColor.set( 1.0, 1.0, 1.0, 1.0 );

   segments.reserve( 100 );
}

//--------------------------------------------------------------------------
// Add segment
//--------------------------------------------------------------------------
void MotionBlurLine::addSegment( const Point3F &start, const Point3F &end )
{
   MotionBlurLineSeg segment;
   segment.start = start;
   segment.end = end;
   segment.startElapsedTime = 0.0;
   segment.endElapsedTime = 0.0;
   segment.color = mColor;

   if( segments.size() != 0 )
   {
      MotionBlurLineSeg *lastSeg = &segments[ segments.size() - 1 ];
      Point3F dir1 = lastSeg->end - lastSeg->start;
      F32 dir1len = dir1.len();
      dir1.normalize();
      Point3F dir2 = end - start;
      if( dir2.isZero() ) return;
      F32 dir2len = dir2.len();
      dir2.normalize();

      // check if colinear
      if( dir1.equal( dir2 ) )
      {
         if( dir1len + dir2len < MAX_SEG_LENGTH )
         {
            lastSeg->end = end;
            lastSeg->endElapsedTime = 0.0;
            return;
         }
      }

      lastSeg->endElapsedTime = 0.0; // synch segments
   }

   segments.push_back( segment );
}

//--------------------------------------------------------------------------
// Render
//--------------------------------------------------------------------------
void MotionBlurLine::render( const Point3F &camPos )
{
   U32 i;
   for( i=0; i<segments.size(); i++ )
   {
      if( segments[i].endElapsedTime < mLifetime )
      {
         renderSegment( segments[i], camPos );
      }
   }
}

//--------------------------------------------------------------------------
// Render Segment
//--------------------------------------------------------------------------
void MotionBlurLine::renderSegment( MotionBlurLineSeg &segment, const Point3F &camPos )
{

   Point3F pos = segment.start;
   Point3F dirFromCam = pos - camPos;
   Point3F dir = segment.end - segment.start;
   Point3F crossDir;
   mCross( dirFromCam, dir, &crossDir );
   crossDir.normalize();
   dir.normalize();

   F32 width = mWidth * 0.5;
   dir *= width;
   crossDir *= width;
   Point3F start = segment.start;
   Point3F end = segment.end;


   glDisable( GL_TEXTURE_2D );
   glDisable(GL_CULL_FACE);
   glDepthMask(GL_FALSE);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE);

   glBegin(GL_QUADS);

      segment.color.alpha = segment.startAlpha;
      glColor4fv( segment.color );

      glTexCoord2f(0, 0);
      glVertex3fv( start + crossDir );

      glTexCoord2f(0, 1);
      glVertex3fv( start - crossDir );


      segment.color.alpha = segment.endAlpha;
      glColor4fv( segment.color );

      glTexCoord2f(1, 1);
      glVertex3fv( end - crossDir );

      glTexCoord2f(1, 0);
      glVertex3fv( end + crossDir );

   glEnd();

   glDepthMask(GL_TRUE);
   glDisable(GL_BLEND);

}

//--------------------------------------------------------------------------
// Update
//--------------------------------------------------------------------------
void MotionBlurLine::update( F32 dt )
{

   U32 i;
   for( i=0; i<segments.size(); i++ )
   {
      segments[i].startElapsedTime += dt;
      segments[i].endElapsedTime += dt;
   }

   calcLineAlpha();
}

//--------------------------------------------------------------------------
// Get visibility box
//--------------------------------------------------------------------------
Box3F MotionBlurLine::getBox( const Point3F &relPoint )
{
   Point3F min;
   Point3F max;

   min.set( 0.0, 0.0, 0.0 );
   max.set( 0.0, 0.0, 0.0 );


   U32 i;
   for( i=0; i<segments.size(); i++ )
   {

      if( segments[i].endElapsedTime < mLifetime && 
          segments[i].startElapsedTime < mLifetime ) 
      {
         min.setMin( segments[i].start - relPoint );
         min.setMin( segments[i].end - relPoint );
         max.setMax( segments[i].start - relPoint );
         max.setMax( segments[i].end - relPoint );
      }
   }


   min += Point3F( -2.0, -2.0, -2.0 );
   max += Point3F(  2.0,  2.0,  2.0 );

   return Box3F( min, max );
}

//--------------------------------------------------------------------------
// Calc alpha at each vertex
//--------------------------------------------------------------------------
void MotionBlurLine::calcLineAlpha()
{

   U32 i;
   for( i=0; i<segments.size(); i++ )
   {
      F32 percentDone;

      // make the very end clear so you can't see the poly edge
      if( i == 0 )
      {
         segments[i].startAlpha = 0.0;
      }
      else
      {
         percentDone = segments[i].startElapsedTime / mLifetime;
         segments[i].startAlpha = 1.0 - percentDone;
      }

      percentDone = segments[i].endElapsedTime / mLifetime;
      segments[i].endAlpha = 1.0 - percentDone;
   }

}
