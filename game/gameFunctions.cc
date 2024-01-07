//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "sim/sceneObject.h"
#include "core/fileStream.h"

void RegisterGameFunctions();

// query
SimpleQueryList gServerQueryList;
U32 gServerQueryIndex = 0;

//SERVER FUNCTIONS ONLY
static const char* cContainerFindFirst(SimObject *, S32, const char ** argv)
{
   //find out what we're looking for
   U32 typeMask = U32(dAtoi(argv[1]));
   
   //find the center of the container volume
   Point3F origin(0, 0, 0);
   dSscanf(argv[1], "%f %f %f", &origin.x, &origin.y, &origin.z);
   
   //find the box dimensions
   Point3F size(0, 0, 0);
   size.x = mFabs(dAtof(argv[3]));
   size.y = mFabs(dAtof(argv[4]));
   size.z = mFabs(dAtof(argv[5]));
   
   //build the container volume
   Box3F queryBox;
   queryBox.min = origin;
   queryBox.max = origin;
   queryBox.min -= size;
   queryBox.max += size;
   
   //initialize the list, and do the query
   gServerQueryList.mList.clear();
   gServerContainer.findObjects(queryBox, typeMask, SimpleQueryList::insertionCallback, S32(&gServerQueryList));
   
   //return the first element
   gServerQueryIndex = 0;
   char *buff = Con::getReturnBuffer(100);
   if (gServerQueryList.mList.size())
      dSprintf(buff, 100, "%d", gServerQueryList.mList[gServerQueryIndex++]->getId());
   else
      buff[0] = '\0';
      
   return buff;
}

static const char* cContainerFindNext(SimObject *, S32, const char **)
{
   //return the next element
   char *buff = Con::getReturnBuffer(100);
   if (gServerQueryIndex < gServerQueryList.mList.size())
      dSprintf(buff, 100, "%d", gServerQueryList.mList[gServerQueryIndex++]->getId());
   else
      buff[0] = '\0';
      
   return buff;
}



//-----------------------------------------------------------------------------------

void RegisterGameFunctions()
{   
      //NOTE: SERVER FUNCTIONS ONLY
      Con::addCommand("containerFindFirst", cContainerFindFirst, "containerFindFirst(type, point, x, y, z)", 6, 6);
      Con::addCommand("containerFindNext", cContainerFindNext, "containerFindNext()", 1, 1);
}
