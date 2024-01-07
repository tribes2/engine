//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/graphGenUtils.h"

//----------------------------------------------------------------
//
// EdgeTable Implementation
//
//----------------------------------------------------------------

EdgeTable::EdgeTable()
{
   hashTable = NULL;
   init();
}

//----------------------------------------------------------------

EdgeTable::~EdgeTable()
{
}

//----------------------------------------------------------------

S32 EdgeTable::hash(DEdge edge)
{
   S32 value = ((edge.start + edge.end) % 1009);// <- prime #
   return value;
}

//----------------------------------------------------------------

void EdgeTable::init()
{
   hashTable = new DEdge *[DefaultTableSize];
   hashTableSize = DefaultTableSize;
   hashEntryCount = 0;
   
   S32 i;
   for(i = 0; i < hashTableSize; i++)
      hashTable[i] = NULL;
}

//----------------------------------------------------------------

void EdgeTable::clear()
{
   DEdge *walk, *temp;
   U32 i;
   for(i = 0; i < hashTableSize; i++)
   {   
   	walk = hashTable[i];
      while(walk)
      {
         temp = walk->next;
         delete walk;
         walk = temp;
   	}
      hashTable[i] = NULL;
   }
   delete [] hashTable;
}

//----------------------------------------------------------------

void EdgeTable::insert(DEdge edge)
{
   DEdge *entry = new DEdge();
   entry->start = edge.start;
   entry->end = edge.end;
   entry->midPoint = edge.midPoint;
   entry->next = NULL;
   entry->right = NULL;
   entry->left = NULL;
   entry->length = edge.length;
   entry->flags = edge.flags;
   // entry->totalBelongsTo = edge.totalBelongsTo;
   
   S32 i;
   // for(i = 0; i < edge.totalBelongsTo; i++)
   //    entry->shared[i] = edge.shared[i];
   
   S32 idx = (hash(*entry) % hashTableSize);
   entry->next = hashTable[idx];
   hashTable[idx] = entry;
   hashEntryCount++;
   if(hashEntryCount > hashTableSize)
   {
      // resize the hash table
      DEdge *head = NULL, *walk, *temp;
   	for(i = 0; i < hashTableSize; i++) 
   	{
   		walk = hashTable[i];
         while(walk)
         {
            temp = walk->next;
            walk->next = head;
            head = walk;
            walk = temp;
         }
   	}
      delete [] hashTable;
      hashTableSize = hashTableSize * 2 + 1;
      hashTable = new DEdge *[hashTableSize];
      
      for(i = 0; i < hashTableSize; i++)
         hashTable[i] = NULL;
      
      while(head)
      {
         temp = head->next;
         idx = (hash(*head) % hashTableSize);
         head->next = hashTable[idx];
         hashTable[idx] = head;
         head = temp;
      }
   }
}

//----------------------------------------------------------------

bool EdgeTable::contains(DEdge edge)
{
   DEdge **walk = &hashTable[hash(edge) % hashTableSize];
   while(*walk)
   {
      if(**walk == edge)
         return true;
      walk = &((*walk)->next);
   }
   return false;
}

//----------------------------------------------------------------

DEdge *EdgeTable::find(DEdge edge)
{
   DEdge **walk = &hashTable[hash(edge) % hashTableSize];
   while(*walk)
   {
      if(**walk == edge)
         return *walk;
      walk = &((*walk)->next);
   }
   return NULL;
}

      
//-------------------------------------------------------------------------------------

DSurf::DSurf()
{
   mEdges = NULL;
   mLongest = NULL;
   mNumEdges = 0;
   mSubSurfCount = 0;
   mDivLevel = 0;
   mMaxRadius = 0.0;
   mMinRadius = 0.0;
   parent = NULL;
   dMemset(mSubSurfs, 0, sizeof(mSubSurfs));
   dMemset(fullWinding, 0, sizeof(fullWinding));
   mNormal.set(0,0,0);
   mCtrIdx = 0;
   volIdx = 0;
   mZone = -1;
}      
      
//-------------------------------------------------------------------------------------

DVolume::DVolume()
{
   mNumPlanes = 0;
   dMemset(mPlanes, 0, sizeof(mPlanes));
   surfIdx = 0;
   ht = 0.0;
   capPt.set(0,0,0);
}

// See if point is inside the planes- 
bool DVolume::checkInside(const Point3F& point)
{
   for (S32 i = 0; i < mNumPlanes; i++)
      if (mPlanes[i].distToPlane(point) >= 0.1)
         return false;
   return true;
}


//-------------------------------------------------------------------------------------
//                                  Interior Detail

InteriorDetail::InteriorDetail()
{ 
   mIndex = -1;
   mNumUnObstructed = 0;
   numPolyLists = 0;
}   

bool InteriorDetail::haveSurfaceNear(const Point3F& pos, F32 rad)
{
   rad *= rad;    // use square of length 
   for (S32 i = 0; i < mSurfaces.size(); i++)
      if ((pos - mSurfaces[i].mCntr).lenSquared() < rad)
         return true;
   return false;
}
