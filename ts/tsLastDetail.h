//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _TSLASTDETAIL_H_
#define _TSLASTDETAIL_H_

#ifndef _MATHTYPES_H_
#include "Math/mathTypes.h"
#endif
#ifndef _MPOINT_H_
#include "Math/mPoint.h"
#endif
#ifndef _MMATRIX_H_
#include "Math/mMatrix.h"
#endif
#ifndef _TVECTOR_H_
#include "Core/tVector.h"
#endif

class TSShape;
class GBitmap;
class TextureHandle;

class TSLastDetail
{
   U32 mNumEquatorSteps; // number steps around the equator of the globe
   U32 mNumPolarSteps;   // number of steps to go from equator to each polar region (0 means equator only)
   F32 mPolarAngle;      // angle in radians of sub-polar regions
   bool mIncludePoles;   // include poles in snapshot?
   
   Point3F mCenter;
   
   // remember these values so we can save some work next time we render...
   U32 mBitmapIndex;
   F32 mRotY;

   Vector<GBitmap*> mBitmaps;
   Vector<TextureHandle*> mTextures;

   Point3F mPoints[4];   // always draw poly defined by these points...
   static Point3F smNorms[4];
   static Point2F smTVerts[4];

   public:

   TSLastDetail(TSShape * shape, U32 numEquatorSteps, U32 numPolarSteps, F32 polarAngle, bool includePoles, S32 dl, S32 dim);
   ~TSLastDetail();

   void render(F32 alpha, bool drawFog);
   void renderNoFog(F32 alpha);
   void renderFog_Combine(F32 alpha);
   void renderFog_MultiCombine(F32 alpha);
   void chooseView(const MatrixF &, const Point3F & scale);
};


#endif // _TS_LAST_DETAIL


