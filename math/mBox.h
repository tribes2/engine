//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _MBOX_H_
#define _MBOX_H_

#ifndef _MPOINT_H_
#include "Math/mPoint.h"
#endif

//------------------------------------------------------------------------------
class Box3F
{
  public:
   Point3F min;
   Point3F max;

  public:
   Box3F() { }
   Box3F(const Point3F& in_rMin, const Point3F& in_rMax, const bool in_overrideCheck = false);
   Box3F(F32 xmin, F32 ymin, F32 zmin, F32 max, F32 ymax, F32 zmax);

   bool isContained(const Point3F& in_rContained) const;
   bool isOverlapped(const Box3F&  in_rOverlap) const;
   bool isContained(const Box3F& in_rContained) const;

   F32 len_x() const;
   F32 len_y() const;
   F32 len_z() const;

   void intersect(const Box3F& in_rIntersect);
   void getCenter(Point3F* center) const;

   bool collideLine(const Point3F& start, const Point3F& end, F32*, Point3F*) const;
   bool collideLine(const Point3F& start, const Point3F& end) const;
   bool collideOrientedBox(const Point3F & radii, const MatrixF & toUs) const;

   bool isValidBox() const { return (min.x <= max.x) &&
                                    (min.y <= max.y) &&
                                    (min.z <= max.z); }
   Point3F getClosestPoint(const Point3F& refPt) const;
};

inline Box3F::Box3F(const Point3F& in_rMin, const Point3F& in_rMax, const bool in_overrideCheck)
 : min(in_rMin),
   max(in_rMax)
{
   if (in_overrideCheck == false) {
      min.setMin(in_rMax);
      max.setMax(in_rMin);
   }
}

inline Box3F::Box3F(F32 xMin, F32 yMin, F32 zMin, F32 xMax, F32 yMax, F32 zMax)
   : min(xMin,yMin,zMin),
     max(xMax,yMax,zMax)
{
}

inline bool Box3F::isContained(const Point3F& in_rContained) const
{
   return (in_rContained.x >= min.x && in_rContained.x < max.x) &&
          (in_rContained.y >= min.y && in_rContained.y < max.y) &&
          (in_rContained.z >= min.z && in_rContained.z < max.z);
}

inline bool Box3F::isOverlapped(const Box3F& in_rOverlap) const
{
   if (in_rOverlap.min.x > max.x ||
       in_rOverlap.min.y > max.y ||
       in_rOverlap.min.z > max.z)
      return false;
   if (in_rOverlap.max.x < min.x ||
       in_rOverlap.max.y < min.y ||
       in_rOverlap.max.z < min.z)
      return false;
   return true;
}

inline bool Box3F::isContained(const Box3F& in_rContained) const
{
   return (min.x <= in_rContained.min.x) &&
          (min.y <= in_rContained.min.y) &&
          (min.z <= in_rContained.min.z) &&
          (max.x >= in_rContained.max.x) &&
          (max.y >= in_rContained.max.y) &&
          (max.z >= in_rContained.max.z);
}

inline F32 Box3F::len_x() const
{
   return max.x - min.x;
}

inline F32 Box3F::len_y() const
{
   return max.y - min.y;
}

inline F32 Box3F::len_z() const
{
   return max.z - min.z;
}

inline void Box3F::intersect(const Box3F& in_rIntersect)
{
   min.setMin(in_rIntersect.min);
   max.setMax(in_rIntersect.max);
}

inline void Box3F::getCenter(Point3F* center) const
{
   center->x = F32((min.x + max.x) * 0.5);
   center->y = F32((min.y + max.y) * 0.5);
   center->z = F32((min.z + max.z) * 0.5);
}   

inline Point3F Box3F::getClosestPoint(const Point3F& refPt) const
{
   Point3F closest;
   if      (refPt.x <= min.x) closest.x = min.x;
   else if (refPt.x >  max.x) closest.x = max.x;
   else                       closest.x = refPt.x;

   if      (refPt.y <= min.y) closest.y = min.y;
   else if (refPt.y >  max.y) closest.y = max.y;
   else                       closest.y = refPt.y;

   if      (refPt.z <= min.z) closest.z = min.z;
   else if (refPt.z >  max.z) closest.z = max.z;
   else                       closest.z = refPt.z;

   return closest;
}


//------------------------------------------------------------------------------
class Box3D
{
  public:
   Point3D min;
   Point3D max;

  public:
   Box3D() { }
   Box3D(const Point3D& in_rMin, const Point3D& in_rMax, const bool in_overrideCheck = false);

   bool isContained(const Point3D& in_rContained) const;
   bool isOverlapped(const Box3D&  in_rOverlap) const;

   F64 len_x() const;
   F64 len_y() const;
   F64 len_z() const;

   void intersect(const Box3D& in_rIntersect);
   void getCenter(Point3D* center) const;
};

inline Box3D::Box3D(const Point3D& in_rMin, const Point3D& in_rMax, const bool in_overrideCheck)
 : min(in_rMin),
   max(in_rMax)
{
   if (in_overrideCheck == false) {
      min.setMin(in_rMax);
      max.setMax(in_rMin);
   }
}

inline bool Box3D::isContained(const Point3D& in_rContained) const
{
   return (in_rContained.x >= min.x && in_rContained.x < max.x) &&
          (in_rContained.y >= min.y && in_rContained.y < max.y) &&
          (in_rContained.z >= min.z && in_rContained.z < max.z);
}

inline bool Box3D::isOverlapped(const Box3D& in_rOverlap) const
{
   if (in_rOverlap.min.x > max.x ||
       in_rOverlap.min.y > max.y ||
       in_rOverlap.min.z > max.z)
      return false;
   if (in_rOverlap.max.x < min.x ||
       in_rOverlap.max.y < min.y ||
       in_rOverlap.max.z < min.z)
      return false;
   return true;
}

inline F64 Box3D::len_x() const
{
   return max.x - min.x;
}

inline F64 Box3D::len_y() const
{
   return max.y - min.y;
}

inline F64 Box3D::len_z() const
{
   return max.z - min.z;
}

inline void Box3D::intersect(const Box3D& in_rIntersect)
{
   min.setMin(in_rIntersect.min);
   max.setMax(in_rIntersect.max);
}

inline void Box3D::getCenter(Point3D* center) const
{
   center->x = (min.x + max.x) * 0.5;
   center->y = (min.y + max.y) * 0.5;
   center->z = (min.z + max.z) * 0.5;
}   


#endif // _DBOX_H_
