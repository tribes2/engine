//-----------------------------------------------------------------------------
// Torque Game Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _TSORTEDSCENEOBJECTLIST_H_
#define _TSORTEDSCENEOBJECTLIST_H_

#ifndef _TVECTOR_H_
#include "core/tVector.h"
#endif

#ifndef _SCENEOBJECT_H_
#include "sim/sceneObject.h"
#endif

class SortedSceneObjectList {

   private:

      class SceneObjectNode {
         public:
            SceneObjectNode *next;

            SceneObject *object;
            Point3F tag;

            SceneObjectNode( SceneObject *obj, Point3F &tg, SceneObjectNode *nxt ) {
               object = obj;
               next = nxt;
               tag = tg;
            }
      };

      SceneObjectNode *mHead;

   public: 
   
      SortedSceneObjectList();
      ~SortedSceneObjectList();

      void addObject( SceneObject *toAdd, Point3F &tag );
      void removeObject( const SceneObject *toRemove );
      bool contains( const SceneObject *test ) const;

      Point3F getTag( const SceneObject *test ) const;

      Vector<SceneObject *> toVector() const;
};

#endif