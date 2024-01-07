//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _DGL_H_
#define _DGL_H_

#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif
#ifndef _PLATFORMGL_H_
#include "PlatformWin32/platformGL.h"
#endif

class TextureObject;
class GFont;
class MatrixF;
class RectI;
class ColorI;
class ColorF;
class Point2I;
class Point2F;
class Point3F;

//------------------------------------------------------------------------------
//-------------------------------------- Bitmap Drawing
//
static const U32 GFlip_None = 0;
static const U32 GFlip_X    = 1 << 0;
static const U32 GFlip_Y    = 1 << 1;
static const U32 GFlip_XY   = GFlip_X | GFlip_Y;

void dglSetBitmapModulation(const ColorF& in_rColor);
void dglGetBitmapModulation(ColorF* color);
void dglGetBitmapModulation(ColorI* color);
void dglClearBitmapModulation();

// Note that you must call this _after_ SetBitmapModulation if the two are different
//  SetBMod sets the text anchor to the modulation color
void dglSetTextAnchorColor(const ColorF&);

void dglDrawBitmap(TextureObject* texObject,
                   const Point2I& in_rAt,
                   const U32      in_flip = GFlip_None);
void dglDrawBitmapStretch(TextureObject* texObject,
                          const RectI&   in_rStretch,
                          const U32      in_flip = GFlip_None);
void dglDrawBitmapSR(TextureObject* texObject,
                     const Point2I& in_rAt,
                     const RectI&   in_rSubRegion,
                     const U32      in_flip = GFlip_None);
void dglDrawBitmapStretchSR(TextureObject* texObject,
                            const RectI&   in_rStretch,
                            const RectI&   in_rSubRegion,
                            const U32      in_flip = GFlip_None);

// Returns the number of x pixels traversed
U32 dglDrawText(GFont *font, const Point2I &ptDraw, const void *in_string, const ColorI *colorTable = NULL, const U32 maxColorIndex = 9);
U32 dglDrawTextN(GFont *font, const Point2I &ptDraw, const void *in_string, U32 n, const ColorI *colorTable = NULL, const U32 maxColorIndex = 9);

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //
// Drawing primitives

void dglDrawLine(S32 x1, S32 y1, S32 x2, S32 y2, const ColorI &color);
void dglDrawLine(const Point2I &startPt, const Point2I &endPt, const ColorI &color);
void dglDrawRect(const Point2I &upperL, const Point2I &lowerR, const ColorI &color);
void dglDrawRect(const RectI &rect, const ColorI &color);
void dglDrawRectFill(const Point2I &upperL, const Point2I &lowerR, const ColorI &color);
void dglDrawRectFill(const RectI &rect, const ColorI &color);
void dglDraw2DSquare( const Point2F &screenPoint, F32 width, F32 spinAngle );
void dglDrawBillboard( const Point3F &position, F32 width, F32 spinAngle );

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //
// Matrix functions

void dglLoadMatrix(const MatrixF *m);
void dglMultMatrix(const MatrixF *m);
void dglGetModelview(MatrixF *m);
void dglGetProjection(MatrixF *m);

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //
// Camera functions

F32 dglGetPixelScale();
F32 dglGetWorldToScreenScale();
F32 dglProjectRadius(F32 dist, F32 radius);

void dglSetViewport(const RectI &aViewPort);
void dglGetViewport(RectI* outViewport);
void dglSetFrustum(F64 left, F64 right, F64 bottom, F64 top, F64 nearDist, F64 farDist, bool ortho = false);
void dglGetFrustum(F64 *left, F64 *right, F64 *bottom, F64 *top, F64 *nearDist, F64 *farDist);
bool dglIsOrtho();
void dglSetClipRect(const RectI &clipRect);
const RectI& dglGetClipRect();

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //
// Misc
bool  dglPointToScreen( Point3F &point3D, Point3F &screenPoint );

//--------------------------------------------------------------------------
// Debug stuff
bool dglIsInCanonicalState();
void dglSetCanonicalState();

void dglGetTransformState(S32* mvDepth,
                          S32* pDepth,
                          S32* t0Depth,
                          F32* t0Matrix,
                          S32* t1Depth,
                          F32* t1Matrix,
                          S32* vp);
bool dglCheckState(const S32 mvDepth, const S32 pDepth,
                   const S32 t0Depth, const F32* t0Matrix,
                   const S32 t1Depth, const F32* t1Matrix,
                   const S32* vp);

#endif // _H_DGL
