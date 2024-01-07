//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "math/mBezier2D.h"

/**
 * Constructor
 *
 * @param inCtrlPts Array of control points to define the curve
 * @param inPtsSize Size of the array of control points
 * @param numCurvePts Number of points to be calculated for the curve
 */
Bezier2D::Bezier2D( const Point2F *inCtrlPts, const U32 inPtsSize, const U32 numCurvePts ) {
   mControlPoints = new Point2F[inPtsSize];
   mCurvePoints = NULL;
   mLastScaledCurve = NULL;
   mLastScaleValue = 1.0f;
   
   mNumCurvePoints = numCurvePts;
   mNumCtrlPoints = inPtsSize;
   
   if( mNumCurvePoints < MBEZIER2D_MIN_CURVE_POINTS )
      mNumCurvePoints = MBEZIER2D_MIN_CURVE_POINTS;
   else if( mNumCurvePoints > MBEZIER2D_MAX_CURVE_POINTS )
      mNumCurvePoints = MBEZIER2D_MAX_CURVE_POINTS;
   
   for( int i = 0; i < inPtsSize; i++ )
      mControlPoints[i] = inCtrlPts[i];
   
   calcCurve();
}

/**
 * Destructor
 */
Bezier2D::~Bezier2D() {
   delete [] mControlPoints;
   delete [] mCurvePoints;
   
   if( mLastScaledCurve != NULL )
      delete mLastScaledCurve;
}

/**
 * Calculates the curve based on the control points.
 * Uses de Casteljau's algorithm. References:
 *
 *    -http://www.css.tayloru.edu/~btoll/s99/424/res/mtu/Notes/spline/de-casteljau.htm
 *    -http://astronomy.swin.edu.au/pbourke/curves/bezier/
 */
void Bezier2D::calcCurve() {
   Point2F *tempPointArray = new Point2F[mNumCtrlPoints];
   
   F32 precision = ( 1.0f / mNumCurvePoints );
   
   if( mCurvePoints != NULL )
      delete [] mCurvePoints;
      
   mCurvePoints = new Point2F[mNumCurvePoints];
   S32 idx = 0;
   
   for( F32 u = 0.0f; u <= 1.0f; u += precision ) {
      for( S32 i = 0; i < mNumCtrlPoints; i++ )
         tempPointArray[i] = mControlPoints[i]; // Making copies...grrr
                
      for( S32 j = 1; j < mNumCtrlPoints; j++ )
         for( S32 k = 0; k < mNumCtrlPoints - j; k++ ) 
            tempPointArray[k] = ( 1 - u ) * tempPointArray[k] + u * tempPointArray[k + 1];     
                         
      mCurvePoints[idx].set( tempPointArray[0].x, tempPointArray[0].y );
      idx++;
   }

   mRescale = true;
   
   mCurveLength = 0.0f;
   for( int i = 1; i < mNumCurvePoints; i++ ) {
      Point2F &pt1 = mCurvePoints[i - 1];
      Point2F &pt2 = mCurvePoints[i];
      mCurveLength += mSqrt( mPow( pt2.x - pt1.x, 2.0f) + mPow( pt2.y - pt1.y, 2.0f ) );
   }
}

/**
 * Gives the curve a new set of control points and recalculates
 *
 * @param inCtrlPts Array of control points to define the curve
 * @param inPtsSize Size of the array of control points
 */
void Bezier2D::setControlPoints( const Point2F *inCtrlPts, const U32 inPtsSize ) {
   delete [] mControlPoints;
   
   mControlPoints = new Point2F[inPtsSize];
   
   for( int i = 0; i < inPtsSize; i++ )
      mControlPoints[i] = inCtrlPts[i];
   
   mNumCtrlPoints = inPtsSize;
   
   calcCurve();
}

/**
 * Sets a new precision value, and recalculates the curve
 *
 * @param precision How precice the curve caluclations will be. 0.0f < precision < 1.0f
 *                  Smaller value = more precise
 */
void Bezier2D::setNumCalcPoints( const U32 numCurvePts ) {
   mNumCurvePoints = numCurvePts;
  
   calcCurve();
}

/**
 * Returns the number of points this object is calculating 
 * for the Bezier it represents
 *
 * @return Number of curve points calculating
 */
U32 Bezier2D::getNumCurvePoints() const {
   return mNumCurvePoints;
}

/**
 * Returns the number of control points in this Bezier curve
 *
 * @return Number of control points
 */
U32 Bezier2D::getNumCtrlPoints() const {
   return mNumCtrlPoints;
}

/**
 * Returns the control points for this Bezier in an array
 *
 * @return Control points for this Bezier
 */
Point2F *Bezier2D::getControlPoints() const {
   return mControlPoints;
}

/**
 * Returns the calculated curve points with the precision set earlier
 *
 * @return The points needed to render this curve
 */
Point2F *Bezier2D::getCurvePoints() const {
   return mCurvePoints;
}

/**
 * Gets the length of the calculated curve
 *
 * @return Length of the calculated curve
 */
F32 Bezier2D::getCurveLength() const {
   return mCurveLength;
}
   
/**
 * Scales the curve (by scaling the control points and recalculating the curve)
 * Saves the last values so repeated requests for the same scale value don't require recalculation
 *
 * @param scaleValue Value to scale the curve by, scaleValue > 0
 * @return A Bezier2D object with the new curve contained in it
 */
Bezier2D *Bezier2D::getScaledCurve( const F32 scaleValue ) {
   if( !mRescale && mLastScaleValue == scaleValue )
      return mLastScaledCurve;
   
   Point2F *tempPoints = new Point2F[mNumCtrlPoints];
   //F32 areaPolygon = 0.0f;
   Point2F centroid( 0.0f, 0.0f );
   
   for( int i = 0; i < mNumCtrlPoints; i++ ) {
      tempPoints[i] = mControlPoints[i];
      
      Point2F &currPt = mControlPoints[i];
      //Point2F nextPt;
      //if( i + 1 == mNumCtrlPoints )
      //   nextPt = mControlPoints[0];
      //else
      //   nextPt = mControlPoints[i + 1];
      
      //areaPolygon += currPt.x * nextPt.y - nextPt.x * currPt.y;
      
	  centroid.x += currPt.x;
	  centroid.y += currPt.y;
      //centroid.x += ( currPt.x + nextPt.x ) * ( currPt.x * nextPt.y - nextPt.x * currPt.y );
      //centroid.y += ( currPt.y + nextPt.y ) * ( currPt.x * nextPt.y - nextPt.x * currPt.y );
   }
   
   //areaPolygon *= .5f;
   //centroid.x *= ( 1 / 6 * areaPolygon );
   //centroid.y *= ( 1 / 6 * areaPolygon );
   centroid.x /= mNumCtrlPoints;
   centroid.y /= mNumCtrlPoints;
   
   for( int i = 0; i < mNumCtrlPoints; i++ ) {
      tempPoints[i].x -= centroid.x;
      tempPoints[i].y -= centroid.y;
      
      tempPoints[i].x *= scaleValue;
      tempPoints[i].y *= scaleValue;
      
      tempPoints[i].x += centroid.x;
      tempPoints[i].y += centroid.y;
   }
   if( mLastScaledCurve != NULL )
      delete mLastScaledCurve;
   
   mLastScaledCurve = new Bezier2D( tempPoints, mNumCtrlPoints, mNumCurvePoints );
   mRescale = false;

   delete [] tempPoints;  
   
   return mLastScaledCurve;
}