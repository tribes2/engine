//-----------------------------------------------------------------------------
// Torque Game Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "game/aiCore.h"

/**
 * Merge lists
 */
void AICore::processNewObjectList( Vector<SceneObject *> &newList, Vector<SceneObject *> &ignore ) {

   for( Vector<SceneObject *>::iterator i = newList.begin(); i != newList.end(); i++ ) {
      bool reallyAdd = true;

      for( Vector<SceneObject *>::iterator j = ignore.begin(); j != ignore.end(); j++ ) {
         if( (*j)->getId() == (*i)->getId() ) {
            reallyAdd = false;
            break;
         }
      }
      
      if( reallyAdd )
         mTrackedObjects.addObject( (*i), (*i)->getPosition() );
   }
}

/**
 * Test to see if the list contains this object
 */
bool AICore::contains( const SceneObject *testObj ) const {
   return mTrackedObjects.contains( testObj );
}

/**
 * Get the list of objects that are tracked that collide with the line
 */
Vector<AITrackedObject> AICore::testLine( const Point3F &a, const Point3F &b ) const {

   Vector<AITrackedObject> retVect;
   Vector<SceneObject *> temp = mTrackedObjects.toVector();

   for( Vector<SceneObject *>::iterator i = temp.begin(); i != temp.end(); i++ ) {
      
      if( (*i)->getWorldBox().collideLine( a, b ) ) {
         // Add to collision list
         retVect.push_back( AITrackedObject( (*i), mTrackedObjects.getTag( (*i) ) ) );
      }
   }

   return retVect;
}

/**
 * Get all the objects tracked
 */
Vector<AITrackedObject> AICore::getObjectList() const {

   Vector<AITrackedObject> retVect;
   Vector<SceneObject *> temp( mTrackedObjects.toVector() );

   for( Vector<SceneObject *>::iterator i = temp.begin(); i != temp.end(); i++ )
         retVect.push_back( AITrackedObject( (*i), mTrackedObjects.getTag( (*i) ) ) );
   
   return retVect;
}

/**
 * Gets the tag of the object
 */
Point3F AICore::getTag( const SceneObject *test ) const {
   return mTrackedObjects.getTag( test );
}
