//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "dgl/dgl.h"
#include "Core/dataChunker.h"
#include "Collision/collision.h"
#include "sceneGraph/sceneGraph.h"
#include "Sim/sceneObject.h"
#include "terrain/terrData.h"
#include "Collision/convex.h"

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

static DataChunker sChunker;

CollisionStateList CollisionStateList::sFreeList;
CollisionWorkingList CollisionWorkingList::sFreeList;
F32 sqrDistanceEdges(const Point3F& start0,
                     const Point3F& end0,
                     const Point3F& start1,
                     const Point3F& end1,
                     Point3F*       is,
                     Point3F*       it);


//----------------------------------------------------------------------------
// Feature Collision
//----------------------------------------------------------------------------

bool ConvexFeature::collide(ConvexFeature& cf,CollisionList* cList, F32 tol)
{
   // Our vertices vs. CF faces
   const Point3F* vert = mVertexList.begin();
   const Point3F* vend = mVertexList.end();
   while (vert != vend)
   {
      cf.testVertex(*vert,cList,false, tol);
      vert++;
   }

   // CF vertices vs. our faces
   vert = cf.mVertexList.begin();
   vend = cf.mVertexList.end();
   while (vert != vend)
   {
      U32 storeCount = cList->count;
      testVertex(*vert,cList,true, tol);

      // Fix up last reference.  material and object are copied from this rather
      //  than the object we're colliding against.
      if (storeCount != cList->count)
      {
         cList->collision[cList->count - 1].material = cf.material;
         cList->collision[cList->count - 1].object   = cf.object;
      }
      vert++;
   }

   // Edge vs. Edge
   const Edge* edge = mEdgeList.begin();
   const Edge* eend = mEdgeList.end();
   while (edge != eend)
   {
      cf.testEdge(mVertexList[edge->vertex[0]],
                  mVertexList[edge->vertex[1]],cList, tol);
      edge++;
   }

   return true;
}

inline bool isInside(const Point3F& p, const Point3F& a, const Point3F& b, const VectorF& n)
{
   VectorF v;
   mCross(n,b - a,&v);
   F32 result = mDot(v,p - a);
   return result <= 0.0f;
}

void ConvexFeature::testVertex(const Point3F& v,CollisionList* cList,bool flip, F32 tol)
{
   // Test vertex against all faces
   const Face* face = mFaceList.begin();
   const Face* end  = mFaceList.end();
   for (; face != end; face++) {
      if (cList->count >= CollisionList::MaxCollisions)
         return;

      const Point3F& p0 = mVertexList[face->vertex[0]];
      const Point3F& p1 = mVertexList[face->vertex[1]];
      const Point3F& p2 = mVertexList[face->vertex[2]];
      F32 distance = mFabs(mDot(face->normal,v - p0));

      if (distance > tol)
         continue;

      if (isInside(v,p0,p1,face->normal) &&
          isInside(v,p1,p2,face->normal) &&
          isInside(v,p2,p0,face->normal)) {
         // Add collision to this face
         Collision& info = cList->collision[cList->count++];
         AssertFatal(cList->count <= CollisionList::MaxCollisions, "Error, exceeded max collisions!");
         info.point = v;
         info.normal = face->normal;
         if (flip)
            info.normal.neg();
         info.material = material;
         info.object = object;
         info.distance = distance;
      }
   }
}

void ConvexFeature::testEdge(const Point3F& s1, const Point3F& e1, CollisionList* cList, F32 tol)
{

   F32 tolSquared = tol*tol;

   // Test edges against edges
   VectorF v1 = e1 - s1;
   const Edge* edge = mEdgeList.begin();
   const Edge* end  = mEdgeList.end();
   for (; edge != end; edge++) {
      if (cList->count >= CollisionList::MaxCollisions)
         return;

      const Point3F& s2 = mVertexList[edge->vertex[0]];
      const Point3F& e2 = mVertexList[edge->vertex[1]];
      VectorF n,v2 = e2 - s2;

      // Normal to both lines
      mCross(v1,v2,&n);
      if (n.lenSquared() < 1e-9) {
         continue;
      }

      Point3F is;
      Point3F it;
      F32 distance = sqrDistanceEdges(s1, e1,
                                      s2, e2,
                                      &is, &it);
      
      if (distance > tolSquared)
         continue;
      distance = mSqrt(distance);

      // Return a collision
      Collision& info = cList->collision[cList->count++];
      AssertFatal(cList->count <= CollisionList::MaxCollisions, "Error, exceeded max collisions!");
      info.point    = is;
      info.normal   = is - it;
      if (info.normal.isZero())
         info.normal.set(0, 0, 1);
      else
         info.normal.normalize();
      info.distance = distance;
      info.material = material;
      info.object   = object;
   }
}


//----------------------------------------------------------------------------
// Collision State management
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

CollisionStateList::CollisionStateList()
{
   mPrev = mNext = this;
   mState = NULL;
}

void CollisionStateList::linkAfter(CollisionStateList* ptr)
{
   mPrev = ptr;
   mNext = ptr->mNext;
   ptr->mNext = this;
   mNext->mPrev = this;
}

void CollisionStateList::unlink()
{
   mPrev->mNext = mNext;
   mNext->mPrev = mPrev;
   mPrev = mNext = this;
}

CollisionStateList* CollisionStateList::alloc()
{
   if (!sFreeList.isEmpty()) {
      CollisionStateList* nxt = sFreeList.mNext;
      nxt->unlink();
      nxt->mState = NULL;
      return nxt;
   }
   return constructInPlace((CollisionStateList*)sChunker.alloc(sizeof(CollisionStateList)));
}

void CollisionStateList::free()
{
   unlink();
   linkAfter(&sFreeList);
}


//----------------------------------------------------------------------------

CollisionWorkingList::CollisionWorkingList()
{
   wLink.mPrev = wLink.mNext = this;
   rLink.mPrev = rLink.mNext = this;
}

void CollisionWorkingList::wLinkAfter(CollisionWorkingList* ptr)
{
   wLink.mPrev = ptr;
   wLink.mNext = ptr->wLink.mNext;
   ptr->wLink.mNext = this;
   wLink.mNext->wLink.mPrev = this;
}

void CollisionWorkingList::rLinkAfter(CollisionWorkingList* ptr)
{
   rLink.mPrev = ptr;
   rLink.mNext = ptr->rLink.mNext;
   ptr->rLink.mNext = this;
   rLink.mNext->rLink.mPrev = this;
}

void CollisionWorkingList::unlink()
{
   wLink.mPrev->wLink.mNext = wLink.mNext;
   wLink.mNext->wLink.mPrev = wLink.mPrev;
   wLink.mPrev = wLink.mNext = this;

   rLink.mPrev->rLink.mNext = rLink.mNext;
   rLink.mNext->rLink.mPrev = rLink.mPrev;
   rLink.mPrev = rLink.mNext = this;
}

CollisionWorkingList* CollisionWorkingList::alloc()
{
   if (sFreeList.wLink.mNext != &sFreeList) {
      CollisionWorkingList* nxt = sFreeList.wLink.mNext;
      nxt->unlink();
      return nxt;
   }
   return constructInPlace((CollisionWorkingList*)sChunker.alloc(sizeof(CollisionWorkingList)));
}

void CollisionWorkingList::free()
{
   unlink();
   wLinkAfter(&sFreeList);
}


//----------------------------------------------------------------------------
// Convex Base Class
//----------------------------------------------------------------------------

U32 Convex::sTag = -1;

//----------------------------------------------------------------------------

Convex::Convex()
{
   mNext = mPrev = this;
   mTag = 0;
}

Convex::~Convex()
{
   // Unlink from Convex Database
   unlink();

   // Delete collision states
   while (mList.mNext != &mList)
      delete mList.mNext->mState;

   // Free up working list
   while (mWorking.wLink.mNext != &mWorking)
      mWorking.wLink.mNext->free();

   // Free up references
   while (mReference.rLink.mNext != &mReference)
      mReference.rLink.mNext->free();
}


//----------------------------------------------------------------------------

void Convex::collectGarbage()
{
   // Delete unreferenced Convex Objects
   for (Convex* itr = mNext; itr != this; itr = itr->mNext) {
      if (itr->mReference.rLink.mNext == &itr->mReference) {
         Convex* ptr = itr;
         itr = itr->mPrev;
         delete ptr;
      }
   }
}

void Convex::nukeList()
{
   // Delete all Convex Objects
   for (Convex* itr = mNext; itr != this; itr = itr->mNext) {
      Convex* ptr = itr;
      itr = itr->mPrev;
      delete ptr;
   }
}




void Convex::registerObject(Convex *convex)
{
   convex->linkAfter(this);
}


//----------------------------------------------------------------------------

void Convex::linkAfter(Convex* ptr)
{
   mPrev = ptr;
   mNext = ptr->mNext;
   ptr->mNext = this;
   mNext->mPrev = this;
}

void Convex::unlink()
{
   mPrev->mNext = mNext;
   mNext->mPrev = mPrev;
   mPrev = mNext = this;
}


//----------------------------------------------------------------------------

Point3F Convex::support(const VectorF&) const
{
   return Point3F(0,0,0);
}

void Convex::getFeatures(const MatrixF&,const VectorF&,ConvexFeature* f)
{
   f->object = NULL;
} 

const MatrixF& Convex::getTransform() const
{
   return mObject->getTransform();
}

const Point3F& Convex::getScale() const
{
   return mObject->getScale();
}

Box3F Convex::getBoundingBox() const
{
   return mObject->getWorldBox();
}

Box3F Convex::getBoundingBox(const MatrixF& mat, const Point3F& scale) const
{
   Box3F wBox = mObject->getObjBox();
   wBox.min.convolve(scale);
   wBox.max.convolve(scale);
   mat.mul(wBox);
   return wBox;
}


//----------------------------------------------------------------------------

void Convex::addToWorkingList(Convex* ptr)
{
   CollisionWorkingList* cl = CollisionWorkingList::alloc();
   cl->wLinkAfter(&mWorking);
   cl->rLinkAfter(&ptr->mReference);
   cl->mConvex = ptr;
};


//----------------------------------------------------------------------------

void Convex::updateWorkingList(const Box3F& box, const U32 colMask)
{
   sTag++;

   // Clear objects off the working list that are no longer intersecting
   for (CollisionWorkingList* itr = mWorking.wLink.mNext; itr != &mWorking; itr = itr->wLink.mNext) {
      itr->mConvex->mTag = sTag;
      if (!box.isOverlapped(itr->mConvex->getBoundingBox())) {
         CollisionWorkingList* cl = itr;
         itr = itr->wLink.mPrev;
         cl->free();
      }
   }

   // Special processing for the terrain and interiors...
   AssertFatal(mObject->getContainer(), "Must be in a container!");

   SimpleQueryList sql;
   mObject->getContainer()->findObjects(box, colMask,
                                        SimpleQueryList::insertionCallback, U32(&sql));
   for (U32 i = 0; i < sql.mList.size(); i++)
      sql.mList[i]->buildConvex(box, this);
}

// ---------------------------------------------------------------------------

void Convex::updateStateList(const MatrixF& mat, const Point3F& scale, const Point3F* displacement)
{
   Box3F box1 = getBoundingBox(mat, scale);
   box1.min -= Point3F(1, 1, 1);
   box1.max += Point3F(1, 1, 1);
   if (displacement) {
      Point3F oldMin = box1.min;
      Point3F oldMax = box1.max;

      box1.min.setMin(oldMin + *displacement);
      box1.min.setMin(oldMax + *displacement);
      box1.max.setMax(oldMin + *displacement);
      box1.max.setMax(oldMax + *displacement);
   }
   sTag++;

   // Destroy states which are no longer intersecting
   for (CollisionStateList* itr = mList.mNext; itr != &mList; itr = itr->mNext) {
      Convex* cv = (itr->mState->a == this)? itr->mState->b: itr->mState->a;
      cv->mTag = sTag;
      if (!box1.isOverlapped(cv->getBoundingBox())) {
         CollisionState* cs = itr->mState;
         itr = itr->mPrev;
         delete cs;
      }
   }

   // Add collision states for new overlapping objects
   for (CollisionWorkingList* itr0 = mWorking.wLink.mNext; itr0 != &mWorking; itr0 = itr0->wLink.mNext) {
      register Convex* cv = itr0->mConvex;
      if (cv->mTag != sTag && box1.isOverlapped(cv->getBoundingBox())) {
         CollisionState* state = new CollisionState;
         state->set(this,cv,mat,cv->getTransform());
         state->mLista->linkAfter(&mList);
         state->mListb->linkAfter(&cv->mList);
      }
   }
}


//----------------------------------------------------------------------------

CollisionState* Convex::findClosestState(const MatrixF& mat, const Point3F& scale)
{
   updateStateList(mat, scale);
   F32 dist = +1E30;
   CollisionState *st = 0;

   MatrixF axform = mat;
   axform.scale(scale);
   
   for (CollisionStateList* itr = mList.mNext; itr != &mList; itr = itr->mNext) {
      CollisionState* state = itr->mState;
      if (state->mLista != itr)
         state->swap();

      MatrixF bxform = state->b->getTransform();
      bxform.scale(state->b->getScale());
      F32 dd = state->distance(axform, bxform);
      if (dd < dist) {
         dist = dd;
         st = state;
      }
   }
   return st;
}


//----------------------------------------------------------------------------

CollisionState* Convex::findClosestStateBounded(const MatrixF& mat, const Point3F& scale, const F32 dontCareDist)
{
   updateStateList(mat, scale);
   F32 dist = +1E30;
   CollisionState *st = 0;

   MatrixF axform = mat;
   axform.scale(scale);
   MatrixF axforminv(true);
   MatrixF temp(mat);
   axforminv.scale(Point3F(1.0f/scale.x,
                           1.0f/scale.y,
                           1.0f/scale.z));
   temp.affineInverse();
   axforminv.mul(temp);
   
   for (CollisionStateList* itr = mList.mNext; itr != &mList; itr = itr->mNext) {
      CollisionState* state = itr->mState;
      if (state->mLista != itr)
         state->swap();

      MatrixF bxform = state->b->getTransform();
      temp = bxform;
      Point3F bscale = state->b->getScale();
      bxform.scale(bscale);
      MatrixF bxforminv(true);
      bxforminv.scale(Point3F(1.0f/bscale.x,
                              1.0f/bscale.y,
                              1.0f/bscale.z));
      temp.affineInverse();
      bxforminv.mul(temp);
      F32 dd = state->distanceBounded(axform, bxform, dontCareDist, &axforminv, &bxforminv);
      if (dd < dist) {
         dist = dd;
         st = state;
      }
   }
   if (dist < dontCareDist)
      return st;
   else
      return NULL;
}


//----------------------------------------------------------------------------

bool Convex::getCollisionInfo(const MatrixF& mat, const Point3F& scale, CollisionList* cList,F32 tol)
{
   for (CollisionStateList* itr = mList.mNext; itr != &mList; itr = itr->mNext) {
      CollisionState* state = itr->mState;
      if (state->mLista != itr)
         state->swap();
      if (state->dist < tol) {
         ConvexFeature fa,fb;
         VectorF v;

         MatrixF imat = mat;
         imat.scale(scale);
         imat.inverse();
         imat.mulV(-state->v,&v);
         getFeatures(mat,v,&fa);

         imat = state->b->getTransform();
         imat.scale(state->b->getScale());
         MatrixF bxform = imat;
         imat.inverse();
         imat.mulV(state->v,&v);
         state->b->getFeatures(bxform,v,&fb);

         fa.collide(fb,cList,tol);
      }
   }
   return (cList->count != 0);
}

void Convex::getPolyList(AbstractPolyList*)
{
   
}



// This function taken from:
//  3D Game Engine Design, David H. Eberly
// Ref page 43.  Code from included cd.
//
F32 sqrDistanceEdges(const Point3F& start0,
                     const Point3F& end0,
                     const Point3F& start1,
                     const Point3F& end1,
                     Point3F*       is,
                     Point3F*       it)
{
   Point3F direction0 = end0 - start0;
   Point3F direction1 = end1 - start1;

   Point3F kDiff = start0 - start1;
   F32 fA00 = direction0.lenSquared();
   F32 fA01 = -mDot(direction0, direction1);
   F32 fA11 = direction1.lenSquared();
   F32 fB0  = mDot(kDiff, direction0);
   F32 fC   = kDiff.lenSquared();
   F32 fDet = mAbs(fA00*fA11 - fA01*fA01);
   F32 fB1, fS, fT, fSqrDist, fTmp;
   
   if ( fDet >= 0.00001 )
   {
      // line segments are not parallel
      fB1 = -mDot(kDiff, direction1);
      fS  = fA01*fB1-fA11*fB0;
      fT  = fA01*fB0-fA00*fB1;
        
      if ( fS >= 0.0 )
      {
         if ( fS <= fDet )
         {
            if ( fT >= 0.0 )
            {
               if ( fT <= fDet )  // region 0 (interior)
               {
                  // minimum at two interior points of 3D lines
                  F32 fInvDet = 1.0/fDet;
                  fS         *= fInvDet;
                  fT         *= fInvDet;
                  fSqrDist    = (fS*(fA00*fS + fA01*fT + 2.0*fB0) +
                                 fT*(fA01*fS + fA11*fT + 2.0*fB1) + fC);
               }
               else  // region 3 (side)
               {
                  fT   = 1.0;
                  fTmp = fA01 + fB0;
                  if ( fTmp >= 0.0 )
                  {
                     fS = 0.0;
                     fSqrDist = fA11 + 2.0*fB1 + fC;
                  }
                  else if ( -fTmp >= fA00 )
                  {
                     fS = 1.0;
                     fSqrDist = fA00 + fA11 + fC + 2.0*(fB1 + fTmp);
                  }
                  else
                  {
                     fS = -fTmp/fA00;
                     fSqrDist = fTmp*fS+fA11+2.0*fB1+fC;
                  }
               }
            }
            else  // region 7 (side)
            {
               fT = 0.0;
               if ( fB0 >= 0.0 )
               {
                  fS = 0.0;
                  fSqrDist = fC;
               }
               else if ( -fB0 >= fA00 )
               {
                  fS = 1.0;
                  fSqrDist = fA00+2.0*fB0+fC;
               }
               else
               {
                  fS = -fB0/fA00;
                  fSqrDist = fB0*fS+fC;
               }
            }
         }
         else
         {
            if ( fT >= 0.0 )
            {
               if ( fT <= fDet )  // region 1 (side)
               {
                  fS = 1.0;
                  fTmp = fA01+fB1;
                  if ( fTmp >= 0.0 )
                  {
                     fT = 0.0;
                     fSqrDist = fA00+2.0*fB0+fC;
                  }
                  else if ( -fTmp >= fA11 )
                  {
                     fT = 1.0;
                     fSqrDist = fA00+fA11+fC+2.0*(fB0+fTmp);
                  }
                  else
                  {
                     fT = -fTmp/fA11;
                     fSqrDist = fTmp*fT+fA00+2.0*fB0+fC;
                  }
               }
               else  // region 2 (corner)
               {
                  fTmp = fA01+fB0;
                  if ( -fTmp <= fA00 )
                  {
                     fT = 1.0;
                     if ( fTmp >= 0.0 )
                     {
                        fS = 0.0;
                        fSqrDist = fA11+2.0*fB1+fC;
                     }
                     else
                     {
                        fS = -fTmp/fA00;
                        fSqrDist = fTmp*fS+fA11+2.0*fB1+fC;
                     }
                  }
                  else
                  {
                     fS = 1.0;
                     fTmp = fA01+fB1;
                     if ( fTmp >= 0.0 )
                     {
                        fT = 0.0;
                        fSqrDist = fA00+2.0*fB0+fC;
                     }
                     else if ( -fTmp >= fA11 )
                     {
                        fT = 1.0;
                        fSqrDist = fA00+fA11+fC+2.0*(fB0+fTmp);
                     }
                     else
                     {
                        fT = -fTmp/fA11;
                        fSqrDist = fTmp*fT+fA00+2.0*fB0+fC;
                     }
                  }
               }
            }
            else  // region 8 (corner)
            {
               if ( -fB0 < fA00 )
               {
                  fT = 0.0;
                  if ( fB0 >= 0.0 )
                  {
                     fS = 0.0;
                     fSqrDist = fC;
                  }
                  else
                  {
                     fS = -fB0/fA00;
                     fSqrDist = fB0*fS+fC;
                  }
               }
               else
               {
                  fS = 1.0;
                  fTmp = fA01+fB1;
                  if ( fTmp >= 0.0 )
                  {
                     fT = 0.0;
                     fSqrDist = fA00+2.0*fB0+fC;
                  }
                  else if ( -fTmp >= fA11 )
                  {
                     fT = 1.0;
                     fSqrDist = fA00+fA11+fC+2.0*(fB0+fTmp);
                  }
                  else
                  {
                     fT = -fTmp/fA11;
                     fSqrDist = fTmp*fT+fA00+2.0*fB0+fC;
                  }
               }
            }
         }
      }
      else 
      {
         if ( fT >= 0.0 )
         {
            if ( fT <= fDet )  // region 5 (side)
            {
               fS = 0.0;
               if ( fB1 >= 0.0 )
               {
                  fT = 0.0;
                  fSqrDist = fC;
               }
               else if ( -fB1 >= fA11 )
               {
                  fT = 1.0;
                  fSqrDist = fA11+2.0*fB1+fC;
               }
               else
               {
                  fT = -fB1/fA11;
                  fSqrDist = fB1*fT+fC;
               }
            }
            else  // region 4 (corner)
            {
               fTmp = fA01+fB0;
               if ( fTmp < 0.0 )
               {
                  fT = 1.0;
                  if ( -fTmp >= fA00 )
                  {
                     fS = 1.0;
                     fSqrDist = fA00+fA11+fC+2.0*(fB1+fTmp);
                  }
                  else
                  {
                     fS = -fTmp/fA00;
                     fSqrDist = fTmp*fS+fA11+2.0*fB1+fC;
                  }
               }
               else
               {
                  fS = 0.0;
                  if ( fB1 >= 0.0 )
                  {
                     fT = 0.0;
                     fSqrDist = fC;
                  }
                  else if ( -fB1 >= fA11 )
                  {
                     fT = 1.0;
                     fSqrDist = fA11+2.0*fB1+fC;
                  }
                  else
                  {
                     fT = -fB1/fA11;
                     fSqrDist = fB1*fT+fC;
                  }
               }
            }
         }
         else   // region 6 (corner)
         {
            if ( fB0 < 0.0 )
            {
               fT = 0.0;
               if ( -fB0 >= fA00 )
               {
                  fS = 1.0;
                  fSqrDist = fA00+2.0*fB0+fC;
               }
               else
               {
                  fS = -fB0/fA00;
                  fSqrDist = fB0*fS+fC;
               }
            }
            else
            {
               fS = 0.0;
               if ( fB1 >= 0.0 )
               {
                  fT = 0.0;
                  fSqrDist = fC;
               }
               else if ( -fB1 >= fA11 )
               {
                  fT = 1.0;
                  fSqrDist = fA11+2.0*fB1+fC;
               }
               else
               {
                  fT = -fB1/fA11;
                  fSqrDist = fB1*fT+fC;
               }
            }
         }
      }
   }
   else
   {
      // line segments are parallel
      if ( fA01 > 0.0 )
      {
         // direction vectors form an obtuse angle
         if ( fB0 >= 0.0 )
         {
            fS = 0.0;
            fT = 0.0;
            fSqrDist = fC;
         }
         else if ( -fB0 <= fA00 )
         {
            fS = -fB0/fA00;
            fT = 0.0;
            fSqrDist = fB0*fS+fC;
         }
         else
         {
            fB1 = -mDot(kDiff, direction1);
            fS = 1.0;
            fTmp = fA00+fB0;
            if ( -fTmp >= fA01 )
            {
               fT = 1.0;
               fSqrDist = fA00+fA11+fC+2.0*(fA01+fB0+fB1);
            }
            else
            {
               fT = -fTmp/fA01;
               fSqrDist = fA00+2.0*fB0+fC+fT*(fA11*fT+2.0*(fA01+fB1));
            }
         }
      }
      else
      {
         // direction vectors form an acute angle
         if ( -fB0 >= fA00 )
         {
            fS = 1.0;
            fT = 0.0;
            fSqrDist = fA00+2.0*fB0+fC;
         }
         else if ( fB0 <= 0.0 )
         {
            fS = -fB0/fA00;
            fT = 0.0;
            fSqrDist = fB0*fS+fC;
         }
         else
         {
            fB1 = -mDot(kDiff, direction1);
            fS = 0.0;
            if ( fB0 >= -fA01 )
            {
               fT = 1.0;
               fSqrDist = fA11+2.0*fB1+fC;
            }
            else
            {
               fT = -fB0/fA01;
               fSqrDist = fC+fT*(2.0*fB1+fA11*fT);
            }
         }
      }
   }

   *is = start0 + direction0 * fS;
   *it = start1 + direction1 * fT;

   return mFabs(fSqrDist);
}
