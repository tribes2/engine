//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GRAPHGROUNDVISIT_H_
#define _GRAPHGROUNDVISIT_H_

#ifndef _GRAPHMATH_H_
#include "ai/graphMath.h"
#endif

class GroundPlan;
class GridDetail;

class InspectionVisitor : public GridVisitor 
{
   protected:    
      GroundPlan&             mGPlan;
      Vector<SceneObject*>    mObjects;
      const U32               mMask;
   
   public:
         //==> Might want to pass in mask later- 
      InspectionVisitor(const GridArea &toVisit, GroundPlan &p) :
         mMask(InteriorObjectType|GameBaseObjectType|TurretObjectType),
         GridVisitor(toVisit), mGPlan(p) { }  
      
      bool beforeDivide(const GridArea& R, S32 level);
      bool atLevelZero(const GridArea& R);
};

//--------------------------------------------------------------------------

static void findObjectsCallback(SceneObject *obj, S32 val)
{  
   Vector<SceneObject*> *list = (Vector<SceneObject *> *)val;
   list->push_back(obj);
}

//--------------------------------------------------------------------------

bool InspectionVisitor::beforeDivide(const GridArea& R, S32)
{
   Box3F wBox;
   
   // construct a world box based on these grid squares
   mGPlan.gridToWorld(R.point, wBox.min);
   mGPlan.gridToWorld((R.point+R.extent), wBox.max);
   wBox.min.z = -(wBox.max.z = 5000);

   // Clear the list and fetch objects
   mObjects.clear();
   gServerContainer.findObjects(-1, findObjectsCallback, S32(&mObjects));

   // Scan list- 
   for (Vector<SceneObject*>::iterator i = mObjects.begin(); i != mObjects.end(); i++)
   {
      SceneObject * obj = (* i);
      if (obj->getTypeMask() & mMask)
      {
         const Box3F &  objBox = obj->getWorldBox();
         if (wBox.isOverlapped(objBox))
         {
            if (obj->getName() && dStricmp("SmallRock", obj->getName()) == 0) 
               continue;
               
            return true;
         }
      }
   }

   return false;  
}

//--------------------------------------------------------------------------

bool InspectionVisitor::atLevelZero(const GridArea& R)
{
   AssertFatal(R.extent.x == 1 && R.extent.y == 1, "inspection visitor error!");
   mGPlan.mTotalVisited++;
   S32 index = mGPlan.gridToIndex(R.point);
   GridDetail &detail = mGPlan.mGridDatabase[index];
   mGPlan.inspectSquare(R, detail);
   return true;
}

//--------------------------------------------------------------------------

#endif

