//-----------------------------------------------------------------------------
// Torque Game Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "core/tSortedSceneObjectList.h"

/**
 * Constructor, initalize pointers to null, counter to 0
 */
SortedSceneObjectList::SortedSceneObjectList() {
   mHead = NULL;
}

SortedSceneObjectList::~SortedSceneObjectList() {
   
   while( mHead != NULL ) {
      SceneObjectNode *temp = mHead;
      mHead = mHead->next;
      
      delete temp;
   }
}

/**
 * Adds an object, no duplicates, adds it in order
 */
void SortedSceneObjectList::addObject( SceneObject *toAdd, Point3F &tag ) {

   if( contains( toAdd ) ) {
      // Just update the tag
      SceneObjectNode *i = mHead;

      while( i != NULL ) {
         if( i->object->getId() == toAdd->getId() ) {
            i->tag = tag;
            return;
         }

         i = i->next;
      }
   }

   SceneObjectNode *previous = mHead;
   SceneObjectNode *current = mHead;

   while( current != NULL && toAdd->getId() > current->object->getId() ) {
      previous = current;
      current = current -> next;
   }

   if( previous == current ) 
      mHead = new SceneObjectNode( toAdd, tag, mHead );
   else      
      previous->next = new SceneObjectNode( toAdd, tag, current );
}

/**
 * Test to see if this list contains the test object
 */
bool SortedSceneObjectList::contains( const SceneObject *test ) const {
   
   SceneObjectNode *i = mHead;

   while( i != NULL ) {
      if( i->object->getId() == test->getId() )
         return true;

      i = i->next;
   }

   return false;
}
   
/**
 * Remove an object from the list
 */
void SortedSceneObjectList::removeObject( const SceneObject *toRemove ) {

   SceneObjectNode *previous = mHead;
   SceneObjectNode *current = mHead;

   while( current != NULL && toRemove->getId() > current->object->getId() ) {
      previous = current;
      current = current->next;
   }

   if( current == NULL )
      return;

   if( previous == current) 
      mHead = mHead->next;
   else      
      previous->next = current->next;

   delete current;
}

/**
 * Gets the tag on an object
 */
Point3F SortedSceneObjectList::getTag( const SceneObject *test ) const {

   SceneObjectNode *current = mHead;
   SceneObjectNode *previous = mHead;

   while( test != NULL && current != NULL && test->getId() > current->object->getId() ) {
      previous = current;
      current = current->next;
   }

   if( current == NULL )
      return Point3F( 42.42f, 42.42f, 42.42f ); // Eh, it's easy to test for
   else
      return current->tag;
}

/**
 * Returns this as a vector
 */
Vector<SceneObject *> SortedSceneObjectList::toVector() const {

   Vector<SceneObject *> retVector;

   SceneObjectNode *i = mHead;

   while( i != NULL ) {
      retVector.push_back( i->object );

      i = i->next;
   }

   return retVector;
}