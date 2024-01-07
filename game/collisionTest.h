//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _COLLISIONTEST_H_
#define _COLLISIONTEST_H_

#ifndef _SCENEOBJECT_H_
#include "sim/sceneObject.h"
#endif

#ifndef _POLYTOPE_H_
#include "collision/polytope.h"
#endif
#ifndef _CLIPPEDPOLYLIST_H_
#include "collision/clippedPolyList.h"
#endif
#ifndef _EXTRUDEDPOLYLIST_H_
#include "collision/extrudedPolyList.h"
#endif
#ifndef _DEPTHSORTLIST_H_
#include "collision/depthSortList.h"
#endif
#ifndef _POLYHEDRON_H_
#include "collision/polyhedron.h"
#endif

struct CollisionTest
{
   Box3F boundingBox;
   SphereF boundingSphere;
   static bool renderAlways;

   Point3F testPos;

   // use a slightly different box and sphere for depthSortList
   Box3F mDepthBox;
   SphereF mDepthSphere;
   Point3F mDepthSortExtent;
   
   // Polytopte/BSP test
   static bool testPolytope;
   BSPTree tree;
   Polytope volume;

   // Clipped polylists
   static bool testClippedPolyList;
   ClippedPolyList polyList;
   
   // Depth sort poly lists
   static bool testDepthSortList;
   static bool depthSort;
   static bool depthRender;
   DepthSortList depthSortList;

   // Extruded
   CollisionList collisionList;
   static bool testExtrudedPolyList;
   Polyhedron polyhedron;
   VectorF extrudeVector;
   ExtrudedPolyList extrudedList;

   CollisionTest();
   ~CollisionTest();
   void consoleInit();
   static void callback(SceneObject*, S32 thisPtr);
   void collide(const MatrixF& transform);
   void render();
};


#endif
