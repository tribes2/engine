//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _MSPLINEPATCH_H_
#define _MSPLINEPATCH_H_

#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif
#ifndef _MPOINT_H_
#include "Math/mPoint.h"
#endif
#ifndef _TVECTOR_H_
#include "Core/tVector.h"
#endif

//------------------------------------------------------------------------------
// Spline control points
//------------------------------------------------------------------------------
class SplCtrlPts
{
private:
   Vector <Point3F > mPoints;

public:
   
   SplCtrlPts();
   virtual ~SplCtrlPts();
   
   U32               getNumPoints(){ return mPoints.size(); }
   const Point3F *   getPoint( U32 pointNum );
   void              setPoint( Point3F &point, U32 pointNum );
   void              addPoint( Point3F &point );
   void              submitPoints( Point3F *pts, U32 num );
};

//------------------------------------------------------------------------------
// Base class for spline patches
//------------------------------------------------------------------------------
class SplinePatch
{
private:
   U32         mNumReqControlPoints;
   SplCtrlPts  mControlPoints;

protected:
   void     setNumReqControlPoints( U32 numPts ){ mNumReqControlPoints = numPts; }

public:

   SplinePatch();

   U32                  getNumReqControlPoints(){ return mNumReqControlPoints; }
   const SplCtrlPts *   getControlPoints(){ return &mControlPoints; }
   const Point3F *      getControlPoint( U32 index ){ return mControlPoints.getPoint( index ); }

   // virtuals
   virtual void         setControlPoint( Point3F &point, int index );
   virtual void         submitControlPoints( SplCtrlPts &points ){ mControlPoints = points; }
   virtual void         calc( F32 t, Point3F &result) = 0;
   virtual void         calc( Point3F *points, F32 t, Point3F &result ) = 0;

};


#endif
