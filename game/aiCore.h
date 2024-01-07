//-----------------------------------------------------------------------------
// Torque Game Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _AICORE_H_
#define _AICORE_H_

#ifndef _TSORTEDSCENEOBJECTLIST_H_
#include "core/tSortedSceneObjectList.h"
#endif

#ifndef _TVECTOR_H_
#include "core/tVector.h"
#endif

#ifndef _SCENEOBJECT_H_
#include "sim/sceneObject.h"
#endif

#include "dgl/dgl.h"

class AITrackedObject {

   private:
      Point3F mLoc;
      SceneObject *mObj;

   public:
      AITrackedObject( SceneObject *obj, Point3F &loc ) :
         mObj( obj ), mLoc( loc ) { };

      Point3F getLoc() const { return mLoc; };
      SceneObject *getObject() { return mObj; };
};

class AICore {

   private:
      SortedSceneObjectList mTrackedObjects;

   public:

      void processNewObjectList( Vector<SceneObject *> &newList, Vector<SceneObject *> &ignore );
      bool contains( const SceneObject *testObj ) const;
      Vector<AITrackedObject> testLine( const Point3F &a, const Point3F &b ) const;
      Vector<AITrackedObject> getObjectList() const;
      Point3F getTag( const SceneObject *test ) const;
};

#endif