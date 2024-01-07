//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/graphMath.h"
#include "Core/realComp.h"
#include "ai/tBinHeap.h"

//-------------------------------------------------------------------------------------

F32 solveForZ(const PlaneF& plane, const Point3F& point)
{
   AssertFatal(mFabs(plane.z) > 0.0001, "solveForZ() expects non-vertical plane");
   return (-plane.d - plane.x * point.x - plane.y * point.y) / plane.z;
}

//-------------------------------------------------------------------------------------

// Simple "iterator", used as in- 
//   for (mArea.start(step); mArea.pointInRect(step); mArea.step(step))
//       ... 
bool GridArea::start(Point2I& steppingPoint) const
{
   if (isValidRect())
   {
      steppingPoint = point;
      return true;
   }
   return false;
}
bool GridArea::step(Point2I& steppingPoint) const
{
   if (++steppingPoint.x >= (point.x + extent.x)) 
   {
      steppingPoint.x = point.x;
      if (++steppingPoint.y >= (point.y + extent.y))
         return false;
   }
   return true;
}

//-------------------------------------------------------------------------------------

GridVisitor::GridVisitor(const GridArea & area)
   :  mArea(area.point, area.extent)
{
   mPostCheck = mPreCheck = true;
}

// Default versions of virtuals. The Before and After callbacks just remove themselves:
bool GridVisitor::beforeDivide(const GridArea&,S32)      {return !(mPreCheck = false);}
bool GridVisitor::atLevelZero(const GridArea&)           {return true; }
bool GridVisitor::afterDivide(const GridArea&,S32,bool)  {return !(mPostCheck= false);}

// Recursive region traversal.  
//    R is a box of width 2^L, and aligned on like boundary (L low bits all zero)
bool GridVisitor::recurse(GridArea R, S32 L)
{
   bool  success = true;
   
   if (! R.overlaps(mArea)) {
      success = false;
   }
   else if (L == 0) {
      success = atLevelZero(R);
   }
   else if (mPreCheck && mArea.contains(R) && !beforeDivide(R, L)) {  // early out?
      success = false;
   }
   else {   
            // Made it to sub-division! 
      S32   half = 1 << (L - 1);
      for (S32 y = half; y >= 0; y -= half) 
        for (S32 x = half; x >= 0; x -= half) 
          if (!recurse(GridArea(R.point.x + x, R.point.y + y, half, half), L - 1))
            success = false;

            // Post order stuff- 
      if (mPostCheck && mArea.contains(R) && !afterDivide(R, L, success))
         success = false;
   }
   return success;
}

bool GridVisitor::traverse()
{
   S32         level = 1;
   GridArea    powerTwoRect (-1, -1, 2, 2);
   
   // Find the power-of-two-sized rect that encloses user-supplied grid.  
   while (!powerTwoRect.contains(mArea)) 
      if (++level < 31) {
         powerTwoRect.point *= 2;
         powerTwoRect.extent *= 2;
      }
      else 
         return false;

   // We have our rect and starting level, perform the recursive traversal:
   return recurse(powerTwoRect, level);
}


//-------------------------------------------------------------------------------------
// Find distance of point from the segment.  If the point's projection onto
// line isn't within the segment, then take distance from the endpoints.  
//
// This also leaves the closest point set in soln.  
// 
F32 LineSegment::distance(const Point3F & p)
{
   VectorF  vec1 = b - a, vec2 = p - a;
   F32      len = vec1.len(), dist = vec2.len();
   
   soln = a;      // good starting default.  

   // First check for case where A and B are basically same (avoid overflow) - 
   //    can return distance from either point.  
   if( len > 0.001 )
   {
      // normalize vector from A to B and get projection of AP along it.  
      F32   dot = mDot( vec1 *= (1/len), vec2 );
      if(dot >= 0)
      {
         if(dot <= len)
         {
            F32   sideSquared = (dist * dist) - (dot * dot);
            if (sideSquared <= 0)   // slight imprecision led to NaN
               dist = sideSquared = 0;
            else
               dist = mSqrt(sideSquared); 
            soln += (vec1 * dot);
         }
         else
         {
            soln = b;
            dist = (b - p).len(); 
         }
      }
   }

   return dist;
}

// Special check which does different math in 3D and 2D.  If the test is passed
// in 3d, then do a 2d check.  
bool LineSegment::botDistCheck(const Point3F& p, F32 dist3, F32 dist2)
{
   F32   d3 = distance(p);
   if (d3 < dist3)
   {
      Point2F  proj2d(soln.x - p.x, soln.y - p.y);
      F32   d2 = proj2d.lenSquared();
      dist2 *= dist2;
      return (d2 < dist2);
   }
   return false;
}


//-------------------------------------------------------------------------------------
// Get a list of grid offsets that "spirals" out in order of closeness. 
//    This is used by the navigation (preprocess) code to find nearest 
//       obstructed grid location, and it uses this for its search order.

S32 getGridSpiral(Vector<Point2I> & spiral, Vector<F32> & dists, S32 radius)
{
   S32               x, y, i;
   Vector<Point2I>   pool;
   BinHeap<F32>      heap;

   // Build all the points that we'll want to consider.  We're going to 
   //    leave off those points that are outside of circle.  
   for( y = -radius; y <= radius; y++ )
      for( x = -radius; x <= radius; x++ )
      {
         F32  dist = Point2F( F32(x), F32(y) ).len();
         if( dist <= F32(radius) + 0.00001 )
         {
            pool.push_back( Point2I(x,y) );
            heap.insert( -dist );               // negate for sort order
         }
      }

   // Get the elements out in order.  
   heap.buildHeap();
   while( (i = heap.headIndex()) >= 0 )
   {
      spiral.push_back( pool[i] );
      dists.push_back( -heap[i] );              // get the (sign-restored) distance
      heap.removeHead();
   }

   return  spiral.size();
}


//-----------------------------------------------------------------
// Line Stepper class.  

void LineStepper::init(const Point3F & a, const Point3F & b)
{
   solution = A = a, B = b;
   total = (dir1 = B - A).len();
   soFar = advance = 0.0f;
   
   if( total > 0.00001 )
   {
      dir1 *= (1 / total);
   }
   else
   {
      total = 0.0f;
      dir1.set(0,0,0);
   }
}

// 
// Finds the intersection of the line with the sphere on the way out.  If there
// are two intersections, and we're starting outside, this should find the
// second.  
// 
F32 LineStepper::getOutboundIntersection(const SphereF & S)
{
   // get projection of sphere center along our line
   F32      project = mDot( S.center - A, dir1 );
   
   // find point nearest to center of sphere
   Point3F  nearPoint = A + (dir1 * project);
   
   // if this isn't in the sphere, then there's no intersection.  negative return flags this.  
   if( ! S.isContained(nearPoint) )
      return -1.0f;
      
   // there is an intersection, figure how far from nearest point it is
   F32  length = mSqrt( S.radius*S.radius - (nearPoint-S.center).lenSquared() );

   // find the solution point and advance amount (which is return value)
   solution = nearPoint + dir1 * length;
   return( advance = length + project );
}

const Point3F& LineStepper::advanceToSolution()
{
   if( advance != 0.0f )
   {
      soFar += advance;
      A = solution;
      advance = 0.0f;
   }
   return solution;
}

//---------------------------------------------------------------
// Finds the center of Mass of a CONVEX polygon.
// Verts should contain an array of Point2F's, in order,  
// and of size n, that will represent the polygon. 
//---------------------------------------------------------------
bool polygonCM(S32 n, Point2F *verts, Point2F *CM)
{
   if(!verts || n < 3)
      return false;
      
   F32 area, area2 = 0.0;
   Point2F cent;
   CM->x = 0;
   CM->y = 0;
   
   U32 i;
   for(i = 1; i < (n-1); i++) 
   {
      triCenter(verts[0], verts[i], verts[i+1], cent);
      area = triArea(verts[0], verts[i], verts[i+1]);
      CM->x += area * cent.x;
      CM->y += area * cent.y;
      area2 += area;
   }
   
   CM->x /= 3 * area2;
   CM->y /= 3 * area2;
   return true;
}

//-------------------------------------------------------------------------------------

// Lifted from tools/morian/CSGBrush.cc. This converts to doubles for insurance, which
// shouldn't pose a speed problem since this is mainly used in graph preprocess.  
bool intersectPlanes(const PlaneF& p, const PlaneF& q, const PlaneF& r, Point3F* pOut)
{
   Point3D  p1(p.x, p.y, p.z);
   Point3D  p2(q.x, q.y, q.z);
   Point3D  p3(r.x, r.y, r.z);
   F64      d1 = p.d, d2 = q.d, d3 = r.d;
   
   F64 bc  = (p2.y * p3.z) - (p3.y * p2.z);
   F64 ac  = (p2.x * p3.z) - (p3.x * p2.z);
   F64 ab  = (p2.x * p3.y) - (p3.x * p2.y);
   F64 det = (p1.x * bc) - (p1.y * ac) + (p1.z * ab);

   // Parallel planes 
   if (mFabsD(det) < 1e-5) 
      return false;

   F64 dc     = (d2 * p3.z) - (d3 * p2.z);
   F64 db     = (d2 * p3.y) - (d3 * p2.y);
   F64 ad     = (d3 * p2.x) - (d2 * p3.x);
   F64 detInv = 1.0 / det;

   pOut->x = ((p1.y * dc) - (d1     * bc) - (p1.z * db)) * detInv;
   pOut->y = ((d1     * ac) - (p1.x * dc) - (p1.z * ad)) * detInv;
   pOut->z = ((p1.y * ad) + (p1.x * db) - (d1     * ab)) * detInv;

   return true;
}

bool findCrossVector(const Point3F &v1, const Point3F &v2, Point3F *result)
{
   if (v1.isZero() || v2.isZero() || (result == NULL))
      return false;
      
   Point3F v1Norm = v1, v2Norm = v2;
   v1Norm.normalize();
   v2Norm.normalize();
   
   if ((! isZero(1.0f - v1Norm.len())) || (! isZero(1.0f - v2Norm.len())))
      return false;
      
   //make sure the vectors are non-colinear
   F32 dot = mDot(v1Norm, v2Norm);
   if (dot > 0.999f || dot < -0.999f)
      return false;
      
   //find the cross product, and normalize the result
   mCross(v1Norm, v2Norm, result);
   result->normalize();
   
   return true;
}
