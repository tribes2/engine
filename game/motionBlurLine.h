//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _H_MOTIONBLURLINE
#define _H_MOTIONBLURLINE

#ifndef _COLOR_H_
#include "Core/color.h"
#endif
#ifndef _TVECTOR_H_
#include "Core/tVector.h"
#endif

class Point3F;

//**************************************************************************
// Motion Blur line segment
//**************************************************************************
struct MotionBlurLineSeg
{
   Point3F  start;
   Point3F  end;
   ColorF   color;
   F32      startElapsedTime;
   F32      endElapsedTime;
   F32      startAlpha;
   F32      endAlpha;

   MotionBlurLineSeg()
   {
      start.set( 0.0, 0.0, 0.0 );
      end.set( 0.0, 0.0, 0.0 );
      color.set( 0.0, 0.0, 0.0 );
      startAlpha = 1.0;
      endAlpha = 1.0;
      startElapsedTime = 0.0;
      endElapsedTime = 0.0;
   }
};


//**************************************************************************
// Motion Blur line - maintains and renders a set of line segments which
// represent a "motion blur" from a fast moving object
//**************************************************************************
class MotionBlurLine
{
private:
   Vector < MotionBlurLineSeg > segments;

   F32      mLifetime;
   F32      mWidth;
   ColorF   mColor;

public:
   MotionBlurLine();

   void  addSegment( const Point3F &start, const Point3F &end );
   void  calcLineAlpha();
   Box3F getBox( const Point3F &relPoint );
   U32   numSegments(){ return segments.size(); }
   void  render( const Point3F &camPos );
   void  renderSegment( MotionBlurLineSeg &segment, const Point3F &camPos );
   void  setColor( ColorF &color ){ mColor = color; }
   void  setLifetime( F32 lifetime ){ mLifetime = lifetime; }
   void  setWidth( F32 w ){ mWidth = w; }
   void  update( F32 dt );
   

};





#endif
