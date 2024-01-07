//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _HUDGLEX_H_
#define _HUDGLEX_H_

#include "math/mPoint.h"
#include "dgl/dgl.h"

class Bezier2D;

void dglDrawBezier( Bezier2D *curve, const ColorI &color, const Point2F &offset = Point2F( 0.0f, 0.0f ) );
void dglDrawBezier( Bezier2D *curve, const ColorI &color1, const ColorI &color2, const Point2F &offset = Point2F( 0.0f, 0.0f ) );

void dglDrawBezierBand( Bezier2D *curve, const Point2F &offset, const F32 scale, const ColorI &color, const F32 per = 1.0f, bool rightToLeft = false );
void dglDrawBezierBand( Bezier2D *curve, const Point2F &offset, const F32 scale, const ColorI &color1, const ColorI &color2, const F32 per = 1.0f, bool rightToLeft = false );

void dglDrawRectFill( const Point2I &upperL, const Point2I &lowerR, const ColorI &color1, 
                  const ColorI &color2, const ColorI &color3, const ColorI &color4 );
void dglDrawRectFill( const RectI &rect, const ColorI &color1, 
                  const ColorI &color2, const ColorI &color3, const ColorI &color4 );
                  
void dglDrawRect( const Point2I &upperL, const Point2I &lowerR, const ColorI &color1, 
                  const ColorI &color2, const ColorI &color3, const ColorI &color4 );
void dglDrawRect( const RectI &rect, const ColorI &color1, const ColorI &color2,
                  const ColorI &color3, const ColorI &color4 );
                  
void dglDrawLine( S32 x1, S32 y1, S32 x2, S32 y2, const ColorI &color1, const ColorI &color2 );
void dglDrawLine( const Point2I &startPt, const Point2I &endPt, const ColorI &color1, const ColorI &color2 );                                    

#endif