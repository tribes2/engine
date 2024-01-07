//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

/*
   
   BinHeap.h
   jan 17, 2000

*/
#ifndef BINHEAP_H
#define BINHEAP_H

#ifndef _TVECTOR_H_
#include "Core/tVector.h"
#endif

#define  BinHeapInline  inline

#define  BinHeapParent(child)          (((child) - 1) >> 1)
#define  BinHeapRight(parent)          (((parent) + 1) << 1)
#define  BinHeapLeft(parent)           (((parent) << 1) + 1)

// Much perfomed operation for shifting in the heap
#define  BinHeapMove(src, dst)         (mBack[ mHeap[dst] = mHeap[src] ] = dst)


//
// BinaryHeap Class
//
template <class T>
class BinHeap
{
   protected:
      T     *  mPool;
      S16   *  mHeap;
      S16   *  mBack;
   
      Vector<T>         mPoolVec;
      Vector<S16>       mHeapVec;
      Vector<S16>       mBackVec;
      bool              mIsHeapified;
      S32               mHeapCount;
   
   protected:
      void     setPointers();
      void     keyChange(S32 heapIndex);
      void     keyImprove(S32 heapIndex);
      void     shiftDown(S32 parent, S32 child);
      void     shiftUp(S32 parent, S32 child);
      
   public:
      BinHeap();
     ~BinHeap();
        
      void     changeKey(S32 indexInArray);
      void     improveKey(S32 indexInArray);
      void     clear();
      void     removeHead();
      void     insert(const T &elem);
      T        *head();
      S32      headIndex();
      S32      count();
      S32      size();
      void     buildHeap();
      void     heapify(S32 heapIndex);
      T        &operator[](U32 index);
      void     reserve(S32 amount);
      bool     validateBack();
      bool     validateHeap();
};


// inlines
//--------------------------------------------------------------------------------

template <class T>
BinHeapInline BinHeap<T>::BinHeap()
{
   mHeapCount = 0;
   mIsHeapified = false;
   setPointers();
}

//--------------------------------------------------------------------------------

template <class T>
BinHeapInline S32 BinHeap<T>::count()
{
   return mHeapCount;
}

//--------------------------------------------------------------------------------

template <class T>
BinHeapInline S32 BinHeap<T>::size()
{
   return mPoolVec.size();
}

//--------------------------------------------------------------------------------

template <class T>
BinHeapInline S32 BinHeap<T>::headIndex()
{
   return (mHeapCount > 0 ? mHeap[0] : -1);
}

//--------------------------------------------------------------------------------

template <class T>
BinHeapInline T * BinHeap<T>::head()
{
   if(mHeapCount > 0) 
      return & mPool[mHeap[0]] ;
   else 
      return NULL;
}

//--------------------------------------------------------------------------------

template <class T>
BinHeapInline void BinHeap<T>::setPointers()
{
   mPool = mPoolVec.address();
   mHeap = mHeapVec.address();
   mBack = mBackVec.address();
}

//--------------------------------------------------------------------------------

template <class T>
BinHeapInline void BinHeap<T>::shiftDown(S32 parent, S32 child)
{
    mHeap[child] = mHeap[parent];
    mBack[mHeap[child]] = child;
}

//--------------------------------------------------------------------------------

template <class T>
BinHeapInline void BinHeap<T>::shiftUp(S32 parent, S32 child)
{
    mHeap[parent] = mHeap[child];
    mBack[mHeap[parent]] = parent;
}

// implementation
//--------------------------------------------------------------------------------

template <class T>
BinHeap<T>::~BinHeap()
{
}

//--------------------------------------------------------------------------------

template <class T>
BinHeapInline void BinHeap<T>::changeKey(S32 indexInArray)
{
   S32 indexInHeap = mBack[indexInArray];
   keyChange(indexInHeap);
}

//--------------------------------------------------------------------------------

template <class T>
BinHeapInline void BinHeap<T>::improveKey(S32 index)
{
   keyImprove(mBack[index]);
}

//--------------------------------------------------------------------------------

template <class T>
void BinHeap<T>::keyChange(S32 heapIndex)
{
   S32 i = heapIndex;
   S32 tempHeap2Vec = mHeap[heapIndex];
   mIsHeapified = false;
    
   while(i > 0)
   {   
      if(mPool[mHeap[BinHeapParent(i)]] < mPool[tempHeap2Vec])
      {   
         mIsHeapified = true;
         shiftDown(BinHeapParent(i), i);
         i = BinHeapParent(i);
      }
      else
         break;
   }
   mHeap[i] = tempHeap2Vec;
   mBack[mHeap[i]] = i;

   if(!mIsHeapified)
      heapify(heapIndex); 
}

//--------------------------------------------------------------------------------

// This version of keyChange() is for values known only to improve - and thus move
// towards head of queue.  Dijkstra() knows this, so we remove many wasted calls
// to heapify() for case where the key didn't percolate up.  
template <class T> 
void BinHeap<T>::keyImprove(S32 heapIndex)
{
   S32 i = heapIndex;
   S32 tempHeap2Vec = mHeap[heapIndex];
    
   while(i > 0)
   {   
      S32 parent = BinHeapParent(i);
      if(mPool[mHeap[parent]] < mPool[tempHeap2Vec])
      {   
         // shiftDown(parent, i);
         BinHeapMove(parent, i);
         i = parent;
      }
      else
      {
         // BinHeapMove(tempHeap2Vec, i);
         mHeap[i] = tempHeap2Vec;
         mBack[mHeap[i]] = i;
         return;
      }
   }
}

//--------------------------------------------------------------------------------

template <class T>
void BinHeap<T>::clear()
{
   mPoolVec.clear();
   mHeapVec.clear();
   mBackVec.clear();
   setPointers();
   mIsHeapified = false;
   mHeapCount = 0;
}

//--------------------------------------------------------------------------------

template <class T>
void BinHeap<T>::insert(const T &elem)
{
   S32 indexInArray = mPoolVec.size();
   mPoolVec.push_back(elem);
   mHeapVec.increment(1);
   mBackVec.push_back(mHeapCount);
   setPointers();
   mHeap[mHeapCount++] = indexInArray;

   if(mIsHeapified)
   {
      register S32   i = mHeapCount - 1;
      register S32   tempHeap2Vec = mHeap[i];
      
      while(i > 0)
      {   
         if(mPool[mHeap[BinHeapParent(i)]] < elem)
         {
            shiftDown(BinHeapParent(i), i);
            i = BinHeapParent(i); 
         }
         else  
             break;  
      }
      mHeap[i] = tempHeap2Vec;
      mBack[tempHeap2Vec] = i;
   }
}

//---------------------------------------------------------------------------------

template <class T>
void BinHeap<T>::heapify(S32 parent)
{
   S32   l, r;
   S32   largest = parent;
   S32   tempHeap2Vec = mHeap[parent];

   while(1)
   {
      if( (l = BinHeapLeft(parent)) < mHeapCount)   // only carry further if left exists.  
      { 
         if(mPool[tempHeap2Vec] < mPool[mHeap[l]])
            largest = l;

         if( (r = BinHeapRight(parent)) < mHeapCount )    // don't do below work if no right
         {
            if(largest == parent && mHeap[parent] != tempHeap2Vec)
            {
               if( mPool[tempHeap2Vec] < mPool[mHeap[r]] )
                  largest = r;
            }
            else
            {
               if( mPool[mHeap[largest]] < mPool[mHeap[r]] )
                  largest = r;
            }
         }
      }
       
      if(largest != parent)
      {    
         shiftUp(parent, largest);
         parent = largest;
      }
      else 
      {    
         mHeap[parent] = tempHeap2Vec;
         mBack[tempHeap2Vec] = parent;
         break;
      }
   }
   mIsHeapified = true;
}

//--------------------------------------------------------------------------------

template <class T>
bool BinHeap<T>::validateBack()
{
   bool valid = true;
   for(S32 i = 0; i < mHeapCount; i++)
   {
      if(mBack[i] == -1)
         continue;
      if(mHeap[mBack[i]] != i || mBack[mHeap[i]] != i)
         valid = false;
   }
   return valid;
}

//--------------------------------------------------------------------------------

template <class T> void BinHeap<T>::reserve(S32 amount)
{
   mPoolVec.reserve(amount);
   mHeapVec.reserve(amount);
   mBackVec.reserve(amount);
}
      
//--------------------------------------------------------------------------------

template <class T>
bool BinHeap<T>::validateHeap()
{
   if(!mIsHeapified)
      buildHeap();

   bool valid = true;
   S32 parents = (mHeapCount-1) >> 1;

   for(S32 i = parents; i >= 0; i--) 
   {
      S32 l = BinHeapLeft(i);
      S32 r = BinHeapRight(i);

      if(l < mHeapCount && mPool[mHeap[i]] < mPool[mHeap[l]])
      {   
         printf("error: (%d < l)parent with lower key than child!\n", i);
         valid = false;
      }
      if(r < mHeapCount && mPool[mHeap[i]] < mPool[mHeap[r]])
      {   
         printf("Error: (%d < r)parent with lower key than child!\n", i);
         valid = false;
      }   
   }
   return valid;
}

//---------------------------------------------------------------------------------

template <class T>
void BinHeap<T>::buildHeap()
{
   mIsHeapified = true;
   for(S32 j = (mHeapCount >> 1) - 1; j >= 0; j--)
      heapify(j);
}

//---------------------------------------------------------------------------------

template <class T>
void BinHeap<T>::removeHead()
{
   if(mHeapCount < 1)
      return;
   if(!mIsHeapified)
      buildHeap();
   
   mBack[mHeap[0]] = -1;
   mBack[mHeap[0]=mHeap[--mHeapCount]] = 0;
   if(mHeapCount)
      heapify(0);
}

//-----------------------------------------------------------------------------------

template <class T>
BinHeapInline T & BinHeap<T>::operator[](U32 index)
{
   return mPool[index];
}

//-----------------------------------------------------------------------------------

#endif
