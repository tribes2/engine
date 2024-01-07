//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _PLANEEXTRACTOR_H_
#define _PLANEEXTRACTOR_H_

#ifndef _MMATH_H_
#include "Math/mMath.h"
#endif
#ifndef _TVECTOR_H_
#include "Core/tVector.h"
#endif
#ifndef _ABSTRACTPOLYLIST_H_
#include "Collision/abstractPolyList.h"
#endif


//----------------------------------------------------------------------------

class PlaneExtractorPolyList: public AbstractPolyList
{
   void memcpy(U32* d, U32* s,U32 size);

public:
   // Internal data
   typedef Vector<Point3F> VertexList;
   VertexList mVertexList;

   Vector<PlaneF> mPolyPlaneList;
   
   // Set by caller
   Vector<PlaneF>* mPlaneList;

   //
   PlaneExtractorPolyList();
   ~PlaneExtractorPolyList();
   void clear();
   void render();

   // Virtual methods
   bool isEmpty() const;
   U32  addPoint(const Point3F& p);
   U32  addPlane(const PlaneF& plane);
   void begin(U32 material,U32 surfaceKey);
   void plane(U32 v1,U32 v2,U32 v3);
   void plane(const PlaneF& p);
   void plane(const U32 index);
   void vertex(U32 vi);
   void end();

  protected:
   const PlaneF& getIndexedPlane(const U32 index);
};


#endif
