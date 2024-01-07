//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------
#ifndef _BEZIER2D_H_
#define _BEZIER2D_H_

// For some reason, the engine will bomb out if you try to make a curve with 50+ calculated points
#define MBEZIER2D_MIN_CURVE_POINTS 2
#define MBEZIER2D_MAX_CURVE_POINTS 49

#include "math/mPoint.h"

class Bezier2D {
   
   private:
      Point2F *mControlPoints;
      Point2F *mCurvePoints;
      
      U32      mNumCtrlPoints;
      U32      mNumCurvePoints;
      F32      mCurveLength;
      
      bool     mRescale;
      Bezier2D *mLastScaledCurve;
      F32      mLastScaleValue;
      
      void calcCurve();
   
   public:
      Bezier2D( const Point2F *inCtrlPts, const U32 inPtsSize, const U32 numCurvePts );
      ~Bezier2D();
      
      void setControlPoints( const Point2F *inCtrlPts, const U32 inPtsSize );
      void setNumCalcPoints( const U32 numCurvePts );
      
      U32 getNumCurvePoints() const;
      U32 getNumCtrlPoints() const;
      Point2F *getControlPoints() const;
      Point2F *getCurvePoints() const;
      F32 getCurveLength() const;
      
      Bezier2D *getScaledCurve( const F32 scaleValue );
};

#endif

// mBezier2D.h