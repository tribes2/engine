//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GRAPHMATH_H_
#define _GRAPHMATH_H_

#ifndef _TVECTOR_H_
#include "core/tVector.h"
#endif
#ifndef _MSPHERE_H_
#include "math/mSphere.h"
#endif
#ifndef _MATHIO_H_
#include "math/mathIO.h"
#endif

//-------------------------------------------------------------------------------------
// One-line convenience functions:

inline bool validArrayIndex(S32 index, S32 arrayLength)
{
   return U32(index) < U32(arrayLength);     // (Unsigned compare handles < 0 case too)
}

inline F32 triArea(const Point2F& a, const Point2F& b, const Point2F& c)
{
   return mFabs((b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y));
}

inline void triCenter(const Point2F& p1, const Point2F& p2, const Point2F& p3, Point2F &c)
{
   c.x = p1.x + p2.x + p3.x;
   c.y = p1.y + p2.y + p3.y;
}

// prototype for finding the CM(center of mass) of a convex poly
bool polygonCM(S32 n, Point2F *verts, Point2F *CM);

// Find point at intersection of three planes if such exists (returns true if so).  
bool intersectPlanes(const PlaneF& p, const PlaneF& q, const PlaneF& r, Point3F* pOut);

// Lookup into NxN table in which one entry is stored for each pair of unique indices. 
inline S32 triangularTableIndex(S32 i1, S32 i2) {
   if (i1 > i2)
      return (((i1 - 1) * i1) >> 1) + i2; 
   else
      return (((i2 - 1) * i2) >> 1) + i1; 
}

//-------------------------------------------------------------------------------------

// Where vertical line through point hits plane (whose normal.z != 0):
F32 solveForZ(const PlaneF& plane, const Point3F& point);

// Cross product, but with a check for bad input (parallel vectors):
bool findCrossVector(const Point3F &v1, const Point3F &v2, Point3F *result);

//-------------------------------------------------------------------------------------
// Encapsulate some common operations on the grid rectangles 
class GridArea : public RectI
{
  public:
   GridArea() { }
   GridArea(const Point2I& pt,const Point2I& ext) : RectI(pt,ext) {}
   GridArea(S32 L, S32 T, S32 W, S32 H) : RectI(L,T,W,H) {}
   GridArea(const GridArea & g) : RectI(g.point,g.extent) {}

   // grid to index, and vice versa:
   S32      getIndex(Point2I pos) const;
   Point2I  getPos(S32 index) const;
   
   // step through grid in index order (Y outer loop):
   bool     start(Point2I &p) const;
   bool     step(Point2I &p) const;
};

inline S32 GridArea::getIndex(Point2I p) const
{
   p -= point;
   if( validArrayIndex(p.x, extent.x) )
      if( validArrayIndex(p.y, extent.y) )
         return p.y * extent.x + p.x;
   return -1;
}

inline Point2I GridArea::getPos(S32 index) const
{
   Point2I P;
   P.x = point.x + index % extent.x;
   P.y = point.y + index / extent.x;
   return P;
}

//-------------------------------------------------------------------------------------
//          Linear range mapping.  

// Get value that is pct way between first and last.  Note this is probably a better
// way of getting midpoints than (A+B)*.5 due to rounding.  
template <class T> T scaleBetween(T first, T last, F32 pct) 
{
   return first + (pct * (last - first));
}

// Get percent of way of value between min and max, clamping return to range [0,1]
template <class T> F32 getPercentBetween(T value, T min, T max) 
{
   if(mFabs((max - min)) < 0.0001)
      return 0.0f;
   else if(min < max){
      if (value <= min)         return 0.0f;
      else if (value >= max)    return 1.0f;
   }
   else if (min > max){
      if (value >= min)         return 0.0f;
      else if (value <= max)    return 1.0f;
   }
   return F32((value - min) / (max - min));
}

// Map value from domain into the given range, capping on ends- 
template <class T> T mapValueLinear(T value, T dmin, T dmax, T rmin, T rmax) 
{
   return scaleBetween(rmin, rmax, getPercentBetween(value, dmin, dmax));
}

template <class T> T mapValueQuadratic(T value, T dmin, T dmax, T rmin, T rmax) 
{
   F32   pct = getPercentBetween(value, dmin, dmax);
   return scaleBetween(rmin, rmax, pct * pct);
}

template <class T> T mapValueSqrt(T value, T dmin, T dmax, T rmin, T rmax) 
{
   F32   pct = getPercentBetween(value, dmin, dmax);
   return scaleBetween(rmin, rmax, mSqrt(pct));
}

//-------------------------------------------------------------------------------------
// An object to do a quad-tree traversal of a grid region.  

class GridVisitor 
{
   protected:
      bool  mPreCheck;
      bool  mPostCheck;
      
      bool  recurse(GridArea rect, S32 level);
      
   public:
      const GridArea mArea;
      GridVisitor(const GridArea & area);
      
      // virtuals - how you get visited:
      virtual bool beforeDivide(const GridArea& R, S32 level);
      virtual bool atLevelZero(const GridArea& R);
      virtual bool afterDivide(const GridArea& R, S32 level, bool success);
      
      // call the traversal:
      bool traverse();
};

//-------------------------------------------------------------------------------------
// Return if the two points are within the given threshold distance. 

template <class PointType, class DistType>
bool within(PointType p1, const PointType & p2, DistType thresh)
{
   return (p1 -= p2).lenSquared() <= (thresh * thresh);
}

inline bool within_2D(const Point3F & A, const Point3F & B, F32 D)
{
   Point2F A2d( A.x, A.y );
   Point2F B2d( B.x, B.y );
   return within( A2d, B2d, D );
}

//-------------------------------------------------------------------------------------
// Get a list of grid offsets that "spirals" out in order of closeness.

S32 getGridSpiral(Vector<Point2I> & spiral, Vector<F32> & dists, S32 radius);

//-------------------------------------------------------------------------------------
// Given a point, used to find closest distance/point on segment.  

class LineSegment
{
      Point3F  a, b;
      Point3F  soln;
   public:
      LineSegment(const Point3F& _a, const Point3F& _b)     {soln=a =_a;b =_b;}
      LineSegment()                                         {soln=a=b=Point3F(0,0,0);}
   public:
      void     set(const Point3F& _a, const Point3F& _b)     {soln=a =_a;b =_b;}
      F32      distance(const Point3F& p);
      bool     botDistCheck(const Point3F& p, F32 d3, F32 d2);
      Point3F  solution()                                   {return soln;}
      Point3F  getEnd(bool which)                           {return which ? b : a;}
};

//-------------------------------------------------------------------------------------

class LineStepper 
{
      Point3F  A, B;
      Point3F  dir1;       // unit direction vector from A to B.  
      F32      total;      // total dist from A to B.  
      F32      soFar;      // how far we've come.  

      F32      advance;    // solutions to queries are kept until the user 
      Point3F  solution;   //    tells us to "officially" advance.  
      
   public:
      LineStepper(const Point3F & a, const Point3F & b)     { init(a,b);  }
      
      const Point3F & getSolution()                         { return solution;      }
      F32 distSoFar()                                       { return soFar;         }
      F32 totalDist()                                       { return total;         }
      F32 remainingDist()                                   { return total-soFar;   }

      // Methods that actually do stuff:
      void  init(const Point3F & a, const Point3F & b);
      F32   getOutboundIntersection(const SphereF & s);
      const Point3F & advanceToSolution();
};

//-------------------------------------------------------------------------------------
// reverse elements in place

template <class T> Vector<T> & reverseVec( Vector<T> & vector )
{
   for(S32 halfWay = (vector.size() >> 1); halfWay;  /* dec'd in loop */ )
   {
      T  & farOne  = * (vector.end() - halfWay--);
      T  & nearOne = * (vector.begin() + halfWay);
      T  tmp=farOne; farOne=nearOne; nearOne=tmp;     // swap
   }
   return vector;
}

//--------------------------------------------------------------------------
// Read/Write a vector of items, using T.read(stream), T.write(stream). 
// Skip function reads same amount as readVector1(), bypassing data.  

template <class T>   bool readVector1(Stream & stream, Vector<T> & vec)
{
   S32   num, i;
   bool  Ok = stream.read( & num );
   for( i = 0, vec.setSize( num ); i < num && Ok; i++ )
      Ok = vec[i].read( stream );
   return Ok;
}
template <class T>   bool writeVector1(Stream & stream, const Vector<T> & vec)
{
   bool  Ok = stream.write( vec.size() );
   for( U32 i = 0; i < vec.size() && Ok; i++ )
      Ok = vec[i].write( stream );
   return Ok;
}

//-------------------------------------------------------------------------------------
// Read/Write a vector of items, using stream.read(T), stream.write(T).

template <class T>   bool readVector2(Stream & stream, Vector<T> & vec)
{
   S32   num, i;
   bool  Ok = stream.read( & num );
   for( i = 0, vec.setSize( num ); i < num && Ok; i++ )
      Ok = stream.read(&vec[i]);
   return Ok;
}
template <class T>   bool writeVector2(Stream & stream, const Vector<T> & vec)
{
   bool  Ok = stream.write( vec.size() );
   for( S32 i = 0; i < vec.size() && Ok; i++ )
      Ok = stream.write(vec[i]);
   return Ok;
}

//-------------------------------------------------------------------------------------
// Functions to read / write a vector of items that define mathRead() / mathWrite().  

template <class T> bool mathReadVector(Vector<T>& vec, Stream& stream)
{
   S32   num, i;
   bool  Ok = stream.read(&num);
   for (i = 0, vec.setSize(num); i < num && Ok; i++ )
      Ok = mathRead(stream, &vec[i]);
   return Ok;
}

template <class T> bool mathWriteVector(const Vector<T>& vec, Stream& stream)
{
   bool  Ok = stream.write(vec.size());
   for (S32 i = 0; i < vec.size() && Ok; i++)
      Ok = mathWrite(stream, vec[i]);
   return Ok;
}

//-------------------------------------------------------------------------------------
// Perform setSize() and force construction of all the elements.  Done to address some
// difficulty experienced in getting virtual function table pointer constructed.  

template <class T>
void setSizeAndConstruct(Vector<T> & vector, S32 size)
{
   vector.setSize(size);
   T * tList = new T [size];
   S32 memSize = size * sizeof(T);
   dMemcpy(vector.address(), tList, memSize);
   delete [] tList;
}

template <class T>
void destructAndClear(Vector<T> & vector)
{
   for (S32 i = vector.size() - 1; i >= 0; i--)
      vector[i].~T();
   vector.clear();
}

template <class T>
void setSizeAndClear(Vector<T> & vector, S32 size, S32 value=0)
{
   vector.setSize( size );
   dMemset(vector.address(), value, vector.memSize());
}

//--------------------------------------------------------------------------
// Read a vector with construction

template <class T> bool constructVector(Stream & stream, Vector<T> & vec)
{
   S32   num, i;
   bool  Ok = stream.read(&num);
   if(num && Ok) 
   {
      setSizeAndConstruct(vec, num);
      for (i = 0; i < num && Ok; i++)
         Ok = vec[i].read(stream);
   }
   return Ok;
}

#endif
