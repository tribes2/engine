//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _POLYLIST_H_
#define _POLYLIST_H_

//Includes
#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif
#ifndef _TVECTOR_H_
#include "Core/tVector.h"
#endif
#ifndef _MPOINT_H_
#include "Math/mPoint.h"
#endif
#ifndef _MPLANE_H_
#include "Math/mPlane.h"
#endif

class PolyList
{
  public:
   struct Poly {
     public:
      PlaneF plane;
      U32    material;

      U16   vStart;
      U16   vCount;

      // NOTE: USE THESE FUNCTIONS!  The above exposed implementation is likely
      //  to change soon.
     public:
      U16 getVIndex(U16 n) const { return U16(vStart + n); }
      U16 getVCount()      const { return vCount;     }

      const PlaneF& getPlane()    const { return plane;    }
      U32           getMaterial() const { return material; }
   };

  public:
   PolyList() {
      VECTOR_SET_ASSOCIATION(mPolys);
      VECTOR_SET_ASSOCIATION(mVertices);
   }

   Vector<Poly>    mPolys;
   Vector<Point3F> mVertices;
};

#endif //_POLYLIST_H_
