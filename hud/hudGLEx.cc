//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "hud/hudGLEx.h"
#include "math/mBezier2D.h"
#include "math/mPoint.h"
#include "hud/hudGLEx.h"
#include "core/color.h"
#include "math/mPoint.h"
#include "math/mRect.h"
#include "dgl/gFont.h"

/**
 * This function takes two colors and does a gradient line between them
 *
 * @param x1 X coordinate of the first point of the line
 * @param y1 Y coordinate of the first point of the line
 * @param x2 X coordinate of the second point of the line
 * @param y2 Y coordinate of the second point of the line
 * @param color1 Color of the first point of the line
 * @param color2 Color of the second point of the line
 */
void dglDrawLine( S32 x1, S32 y1, S32 x2, S32 y2, const ColorI &color1, const ColorI &color2 ) {
   glEnable( GL_BLEND );
   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
   glDisable( GL_TEXTURE_2D );
   
   glBegin( GL_LINES );
      glColor4ub( color1.red, color1.green, color1.blue, color1.alpha );
      glVertex2f( (F32)x1 + 0.5,  (F32)y1 + 0.5 );
      
      glColor4ub( color2.red, color2.green, color2.blue, color2.alpha );
      glVertex2f( (F32)x2 + 0.5,    (F32)y2 + 0.5 );
   glEnd();
}

/**
 * This function takes two colors and does a gradient line between them
 *
 * @param startPt Start point of the line
 * @param endPt End point of the line
 * @param color1 Color of the start point of the line
 * @param color2 Color of the end point of the line
 */
void dglDrawLine( const Point2I &startPt, const Point2I &endPt, const ColorI &color1, const ColorI &color2 ) {
    dglDrawLine( startPt.x, startPt.y, endPt.x, endPt.y, color1, color2 );
}

/**
 * This function draws a hollow rectangle and 
 * does a gradient blend across it
 *
 * @param upperL Upper left corner point
 * @param lowerR Lower right corner point
 * @param color1 Upper left corner point color
 * @param color2 Lower left corner point color
 * @param color3 Upper right corner point color
 * @param color4 Lower right corner point color
 */
void dglDrawRect( const Point2I &upperL, const Point2I &lowerR, const ColorI &color1, 
                  const ColorI &color2, const ColorI &color3, const ColorI &color4 ) {
   glEnable( GL_BLEND );
   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
   glDisable( GL_TEXTURE_2D );

   glBegin( GL_LINE_LOOP );
      // Upper Left
      glColor4ub( color1.red, color1.green, color1.blue, color1.alpha );
      glVertex2f( (F32)upperL.x + 0.5, (F32)upperL.y + 0.5 );
      // Upper Right
      glColor4ub( color3.red, color3.green, color3.blue, color3.alpha );
      glVertex2f( (F32)lowerR.x + 0.5, (F32)upperL.y + 0.5 );
      // Lower Right
      glColor4ub( color4.red, color4.green, color4.blue, color4.alpha );
      glVertex2f( (F32)lowerR.x + 0.5, (F32)lowerR.y + 0.5 );
      // Lower Left
      glColor4ub( color2.red, color2.green, color2.blue, color2.alpha );
      glVertex2f( (F32)upperL.x + 0.5, (F32)lowerR.y + 0.5 );
   glEnd();
}
                        
/**
 * This function draws a hollow rectangle and 
 * does a gradient blend across it
 * 
 * @param rect Rectangle to draw
 * @param color1 Upper left corner point color
 * @param color2 Lower left corner point color
 * @param color3 Upper right corner point color
 * @param color4 Lower right corner point color
 */
void dglDrawRect( const RectI &rect, const ColorI &color1, const ColorI &color2,
                  const ColorI &color3, const ColorI &color4 ) {
                        
   Point2I lowerR( rect.point.x + rect.extent.x - 1, rect.point.y + rect.extent.y - 1 );
   dglDrawRect( rect.point, lowerR, color1, color2, color3, color4 );                 
}

/**
 * This function draws a filled rectangle and does a gradient
 * blend across it.
 *
 * @param upperL Upper left corner point
 * @param lowerR Lower right corner point
 * @param color1 Upper left corner point color
 * @param color2 Lower left corner point color
 * @param color3 Upper right corner point color
 * @param color4 Lower right corner point color
 */
void dglDrawRectFill( const Point2I &upperL, const Point2I &lowerR, const ColorI &color1, 
                  const ColorI &color2, const ColorI &color3, const ColorI &color4 ) {
   glEnable( GL_BLEND );
   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
   glDisable( GL_TEXTURE_2D );

   glBegin( GL_QUADS );
      // Upper Left
      glColor4ub( color1.red, color1.green, color1.blue, color1.alpha );
      glVertex2f( (F32)upperL.x + 0.5, (F32)upperL.y + 0.5 );
      // Upper Right
      glColor4ub( color3.red, color3.green, color3.blue, color3.alpha );
      glVertex2f( (F32)lowerR.x + 0.5, (F32)upperL.y + 0.5 );
      // Lower Right
      glColor4ub( color4.red, color4.green, color4.blue, color4.alpha );
      glVertex2f( (F32)lowerR.x + 0.5, (F32)lowerR.y + 0.5 );
      // Lower Left
      glColor4ub( color2.red, color2.green, color2.blue, color2.alpha );
      glVertex2f( (F32)upperL.x + 0.5, (F32)lowerR.y + 0.5 );
   glEnd();
}
                        
/**
 * This function draws a hollow rectangle and 
 * does a gradient blend across it
 * 
 * @param rect Rectangle to draw
 * @param color1 Upper left corner point color
 * @param color2 Lower left corner point color
 * @param color3 Upper right corner point color
 * @param color4 Lower right corner point color
 */
void dglDrawRectFill( const RectI &rect, const ColorI &color1, 
                  const ColorI &color2, const ColorI &color3, const ColorI &color4 ) {

   Point2I lowerR( rect.point.x + rect.extent.x - 1, rect.point.y + rect.extent.y - 1 );
   dglDrawRectFill( rect.point, lowerR, color1, color2, color3, color4 );                 
}

/**
 * This draws a Beizer curve using a line strip
 * 
 * @param curve          Bezier curve object to draw
 * @param color          Color to draw the curve
 */
void dglDrawBezier( Bezier2D *curve, const ColorI &color, const Point2F &offset ) {
   
   glEnable( GL_BLEND );
   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
   glDisable( GL_TEXTURE_2D );
   
   glColor4ub( color.red, color.green, color.blue, color.alpha );

   glBegin( GL_LINE_STRIP );
      for( int i = 0; i < curve->getNumCurvePoints(); i++ )
         glVertex2f( curve->getCurvePoints()[i].x - offset.x, curve->getCurvePoints()[i].y - offset.y );
   glEnd();
}

/**
 * This draws a Beizer curve using a line strip with a gradient
 * 
 * @param curve          Bezier curve object to draw
 * @param color1         First color for the curve gradient
 * @param color2         Second color for the curve gradient
 */
void dglDrawBezier( Bezier2D *curve, const ColorI &color1, const ColorI &color2, const Point2F &offset ) {
   ColorF colorStep;
   ColorF currColor( color1.red, color1.green, color1.blue, color1.alpha );
      
   // Calculate the color steps
   F32 stepDelta = curve->getNumCurvePoints() / 100.0f;

   colorStep.red = ( ( color2.red - color1.red ) / 85.3 ) * stepDelta;
   colorStep.green = ( ( color2.green - color1.green ) / 85.3 ) * stepDelta;
   colorStep.blue = ( ( color2.blue - color1.blue ) / 85.3 ) * stepDelta;
   
   glEnable( GL_BLEND );
   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
   glDisable( GL_TEXTURE_2D );

   glBegin( GL_LINE_STRIP );
      for( int i = 0; i < curve->getNumCurvePoints(); i++ ) {
         currColor.red += colorStep.red * i;
         currColor.green += colorStep.green * i;
         currColor.blue += colorStep.blue * i;
         
         glColor4ub( currColor.red, currColor.green, currColor.blue, currColor.alpha );
		 glVertex2f( curve->getCurvePoints()[i].x - offset.x, curve->getCurvePoints()[i].y - offset.y );
      }
   glEnd();
}

/**
 * This draws a Beizer curve using a quad strip
 * 
 * @param curve          Bezier curve object to draw
 * @param offset         Delta to offset the second curve by
 * @param scale          How big the second curve is in relation to the first
 * @param color          Color to draw the curve
 * @param per            Optional parameter, allows you to specify a percent of the graph to render
 * @param rightToLeft    Optional parameter, draw the graph from right to left instead of left to right (Default)
 */
void dglDrawBezierBand( Bezier2D *curve, const Point2F &offset, const F32 scale, const ColorI &color, const F32 per, bool rightToLeft ) {
   Bezier2D *innerCurve = curve->getScaledCurve( scale );
   F32 percent = per;
   
   if( percent > 1.0f )
      percent = 1.0f;
   else if( percent < 0.0f )
      percent = 0.0f;
      
   U32 i = 0;
   U32 max = curve->getNumCurvePoints();
   
   if( rightToLeft )
      i = (U32)( max * ( 1.0f - percent ) );
   else if( percent != 1.0f )
      max = (U32)( max * percent );
      
   glEnable( GL_BLEND );
   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
   glDisable( GL_TEXTURE_2D );
   
   glColor4ub( color.red, color.green, color.blue, color.alpha );

   glBegin( GL_QUAD_STRIP );
      for( i; i < max;  i++ ) {
         glVertex2f( curve->getCurvePoints()[i].x, curve->getCurvePoints()[i].y );
         glVertex2f( innerCurve->getCurvePoints()[i].x - offset.x, 
                     innerCurve->getCurvePoints()[i].y - offset.y );
      }
   glEnd();
}

/**
 * This draws a Beizer curve using a quad strip with a gradient
 * 
 * @param curve          Bezier curve object to draw
 * @param offset         Delta to offset the second curve by
 * @param scale          How big the second curve is in relation to the first
 * @param color1         First color for the curve gradient
 * @param color2         Second color for the curve gradient
 * @param per            Optional parameter, allows you to specify a percent of the graph to render
 * @param rightToLeft    Optional parameter, draw the graph from right to left instead of left to right (Default)
 */
void dglDrawBezierBand( Bezier2D *curve, const Point2F &offset, const F32 scale, const ColorI &color1, const ColorI &color2, const F32 per, bool rightToLeft ) {
   Bezier2D *innerCurve = curve->getScaledCurve( scale );
   ColorF colorStep( 0, 0, 0 );
   ColorF currColor( color1.red, color1.green, color1.blue, color1.alpha );
   F32 percent = per;
   U32 i = 0;
   U32 max = curve->getNumCurvePoints();
   
   if( percent > 1.0f )
      percent = 1.0f;
   else if( percent < 0.0f )
      percent = 0.0f;
      

   // Calculate the color steps
   colorStep.red = ( (F32)color2.red - (F32)color1.red ) / (F32)curve->getNumCurvePoints();
   colorStep.green = ( (F32)color2.green - (F32)color1.green ) / (F32)curve->getNumCurvePoints();
   colorStep.blue = ( (F32)color2.blue - (F32)color1.blue ) / (F32)curve->getNumCurvePoints();
   
   if( rightToLeft ) {
      i = (U32)( max * ( 1.0f - percent ) );
	  currColor.set( color2.red, color2.green, color2.blue, color2.alpha );

	  colorStep.red *= -1.0f;
      colorStep.green *= -1.0f;
	  colorStep.blue *= -1.0f;

      currColor.red += i * colorStep.red;
      currColor.green += i * colorStep.green;
      currColor.blue += i * colorStep.blue;
   }
   else if( percent != 1.0f )
      max = (U32)( max * percent );
   
   glEnable( GL_BLEND );
   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
   glDisable( GL_TEXTURE_2D );

   glBegin( GL_QUAD_STRIP );
      for( i; i < max;  i++ ) {    
         glColor4ub( currColor.red, currColor.green, currColor.blue, currColor.alpha );
         
         glVertex2f( curve->getCurvePoints()[i].x, curve->getCurvePoints()[i].y );
         glVertex2f( innerCurve->getCurvePoints()[i].x - offset.x, 
                     innerCurve->getCurvePoints()[i].y - offset.y );

		 currColor.red += colorStep.red;
         currColor.green += colorStep.green;
         currColor.blue += colorStep.blue;
      }
   glEnd();
}