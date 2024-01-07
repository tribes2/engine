//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "Core/idGenerator.h"

void IdGenerator::reclaim()
{
   // attempt to keep the pool vector as small as possible by reclaiming 
   // pool entries back into the nextIdBlock variable

   while (!mPool.empty() && (mPool.last() == (mNextId-1)) )
   {
      mNextId--;
      mPool.pop_back();
   }   
}   

