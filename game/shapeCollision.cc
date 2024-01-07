//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "game/shapeBase.h"
#include "game/item.h"
#include "game/trigger.h"

//----------------------------------------------------------------------------

void collisionFilter(SceneObject* object,S32 key)
{
   Container::CallbackInfo* info = reinterpret_cast<Container::CallbackInfo*>(key);
   ShapeBase* ptr = reinterpret_cast<ShapeBase*>(info->key);

   if (object->getTypeMask() & ItemObjectType) {
      // We've hit it's bounding box, that's close enough for items.
      Item* item = static_cast<Item*>(object);
      if (ptr != item->getCollisionObject())
         ptr->queueCollision(item);
   }
   else
      if (object->getTypeMask() & TriggerObjectType) {
         // We've hit it's bounding box, that's close enough for triggers
         Trigger* pTrigger = static_cast<Trigger*>(object);
         pTrigger->potentialEnterObject(ptr);
      }
      else
         if (object->getTypeMask() & CorpseObjectType)
            // Ok, guess it's close enough for corpses too...
            ptr->queueCollision(static_cast<ShapeBase*>(object));
         else
            object->buildPolyList(info->polyList,info->boundingBox,info->boundingSphere);
}

void defaultFilter(SceneObject* object,S32 key)
{
   Container::CallbackInfo* info = reinterpret_cast<Container::CallbackInfo*>(key);
   object->buildPolyList(info->polyList,info->boundingBox,info->boundingSphere);
}

