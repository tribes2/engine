//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

/******************************************************************************
 *  FILENAME:       D:\Tribes\darkstar\terrain\bvQuadTree.h
 *
 *  DESCRIPTION:    
 *
 *  CREATED:        1/17/00  4:02:53 PM
 *
 *  BY:             PeteW
 ******************************************************************************/

#ifndef _BVQUADTREE_H_
#define _BVQUADTREE_H_

//#define BV_QUADTREE_DEBUG

#ifndef _TVECTOR_H_
#include "Core/tVector.h"
#endif
#ifndef _BITVECTOR_H_
#include "Core/bitVector.h"
#endif
#ifndef _MPOINT_H_
#include "Math/mPoint.h"
#endif

class BVQuadTree
{
protected:
   VectorPtr<BitVector*> mQTHierarchy;
   U32 mResolution;
public:
   BVQuadTree(BitVector *bv = NULL);
   ~BVQuadTree();

   bool isSet(const Point2F &pos, S32 level) const;
   bool isClear(const Point2F &pos, S32 level) const;

   void init(const BitVector &bv);
#ifdef BV_QUADTREE_DEBUG
   void dump() const;
#endif
   U32 countLevels() const                               { return(mQTHierarchy.size()); }
protected:
   void buildHierarchy(U32 level);
private:
   BVQuadTree(const BVQuadTree &);
   BVQuadTree& operator=(const BVQuadTree &);
};

#endif
