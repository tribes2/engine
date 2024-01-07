//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GRAPHBSP_H_
#define _GRAPHBSP_H_

template <class T> class AxisAlignedBSP 
{
   public:
      enum {Lower, Upper};
      
      class BSPNode {
        public:
         U16   mAxis;
         U16   mCount;
         union {
            U16   mBranch[2];
            T  *  mLeaf;
         } X;
         F32   mDivide;
         BSPNode()
         {
            mAxis = 0;
            mCount = 0;
            mDivide = 0.0f;
            X.mBranch[Lower] = X.mBranch[Upper] = 0;
            X.mLeaf = NULL;
         }
      };
      
      typedef Vector<BSPNode>  BSPPool;
      
   protected:
      static S32        sSortAxis;
      BSPPool           mTree;
      VectorPtr<T*>     mList;
      
   protected:
      static S32 QSORT_CALLBACK compareAxis(const void* , const void* );
      void  sortOnAxis(VectorPtr<T*>& list, S32 axis);
      void  traverse(S32 ind, const Box3F& box, VectorPtr<T*>* result) const;
      U16   partition(S32 start, S32 N);
      
   public:
      void  makeTree(const VectorPtr<T*>& listIn);
      S32   getIntersecting(VectorPtr<T*>& listOut, Box3F box) const;
      void  clear();
};

//-------------------------------------------------------------------------------------

template <class T> S32 AxisAlignedBSP<T>::sSortAxis;

template <class T> 
S32 QSORT_CALLBACK AxisAlignedBSP<T>::compareAxis(const void* a,const void* b)
{
   const F32 * locA = (*(T**)a)->location();
   const F32 * locB = (*(T**)b)->location();
   F32   A = locA[sSortAxis], B = locB[sSortAxis];
   return (A < B ? -1 : (A > B ? 1 : 0));
}

template <class T> void AxisAlignedBSP<T>::sortOnAxis(VectorPtr<T*>& list, S32 axis)
{
   sSortAxis = axis;
   dQsort((void* )(list.address()), list.size(), sizeof(T* ), compareAxis);
}

//-------------------------------------------------------------------------------------

// Main tree building routine.  Divides up the space at each level until done.  
template <class T> U16 AxisAlignedBSP<T>::partition(S32 start, S32 N)
{
   S32      place = mTree.size();
   BSPNode  assembleNode;
   
   if (N < 2)
   {
      AssertFatal(N == 1, "Bad call to axis-aligned BSP partition");
      assembleNode.mCount = 1;
      assembleNode.X.mLeaf = mList[start];
      mTree.push_back(assembleNode);
   }
   else 
   {
      S32            mid1 = (N >> 1);
      S32            mid0 = mid1 - 1;
      F32            bestSeparation = -1;
      VectorPtr<T*>  divisions[3];
      
      // Try all three divisions.  We take the one whose halves are furthest apart 
      // at the divide.
      for (S32 axis = 0; axis < 3; axis++)
      {
         divisions[axis].setSize(N);
         dMemcpy(&(divisions[axis][0]), &mList[start], N * sizeof(T*));
         sortOnAxis(divisions[axis], axis);
      
         const F32 * endOfFirst = divisions[axis][mid0]->location();
         const F32 * startOf2nd = divisions[axis][mid1]->location();
      
         F32   low = endOfFirst[axis];
         F32   high  = startOf2nd[axis];
         F32   separation = (high - low);
      
         AssertFatal(separation >= 0, "Bad sort in axis-aligned BSP construction");
      
         if (separation > bestSeparation) 
         {
            bestSeparation = separation;
            assembleNode.mCount = N;
            assembleNode.mAxis = axis;
            assembleNode.mDivide = low + (separation * 0.5f);
         }
      }

      // Copy over the sorted elements along the axis we're dividing- 
      dMemcpy(&mList[start], &(divisions[assembleNode.mAxis][0]), N * sizeof(T*));

      // Install the node proper, and call down to further partition. 
      mTree.push_back(assembleNode);
      
      // NOTE!!  This code doesn't work in release build if we haven't pre-reserved the
      // !!!!!!      tree. It looks like a compiler bug to me, not 100% sure though.  
      mTree[place].X.mBranch[Lower] = partition(start, mid1);
      mTree[place].X.mBranch[Upper] = partition(start + mid1, N - mid1);
   }
   
   // Return where we're at in the tree.  
   return place;
}

//-------------------------------------------------------------------------------------
// Make a tree whose leaves are the nodes passed in.

template <class T> void AxisAlignedBSP<T>::makeTree(const VectorPtr<T*>& listIn)
{
   mList = listIn;
   mTree.clear();
   
   AssertFatal(mList.size() < 32000, "AxisAlignedBSP::makeTree() - too many nodes");

   if (mList.size()) {
      mTree.reserve(mList.size() * 3);
      partition(0, mList.size());
      mTree.compact();
   }
   
   mList.clear();
   mList.compact();
}

//-------------------------------------------------------------------------------------

// We want our check to be inclusive of both ends (Box3F method isn't above)
inline bool boxContainsPt(const Box3F& B, const Point3F& P)
{
   if (P.x >= B.min.x && P.x <= B.max.x)
      if (P.y >= B.min.y && P.y <= B.max.y)
         return (P.z >= B.min.z && P.z <= B.max.z);
   return false;
}

//-------------------------------------------------------------------------------------


template <class T> void AxisAlignedBSP<T>::traverse(S32 ind, const Box3F& box, 
                                                      VectorPtr<T*>* result)  const 
{
   const BSPNode & node = mTree[ind];
   
   if (node.mCount == 1) {    // At node- 
      if (boxContainsPt(box, node.X.mLeaf->location()))
         result->push_back(node.X.mLeaf);
   }
   else {
      AssertFatal(node.mCount > 1, "AxisAlignedBSP::traverse()");
      if (node.mDivide > ((const F32*)(box.max))[node.mAxis])
         traverse(node.X.mBranch[Lower], box, result);
      else if (node.mDivide < ((const F32*)(box.min))[node.mAxis])
         traverse(node.X.mBranch[Upper], box, result);
      else {
         Box3F splitUpper(box);
         Box3F splitLower(box);
         ((F32*)(splitUpper.min))[node.mAxis] = node.mDivide;
         ((F32*)(splitLower.max))[node.mAxis] = node.mDivide;
         traverse(node.X.mBranch[Lower], splitLower, result);
         traverse(node.X.mBranch[Upper], splitUpper, result);
      }
   }
}

//-------------------------------------------------------------------------------------
// Push all nodes intersecting the box onto the user-supplied list.  We don't clear
// list because we'll probably have two trees - for small nodes and large nodes.  

template <class T> 
S32 AxisAlignedBSP<T>::getIntersecting(VectorPtr<T*>& result, Box3F box) const
{
   if (mTree.size())
      traverse(0, box, &result);
   return result.size();
}

//-------------------------------------------------------------------------------------

template <class T> void AxisAlignedBSP<T>::clear()
{
   mTree.clear();    mTree.compact();
   mList.clear();    mList.compact();
}


//-------------------------------------------------------------------------------------

#endif
