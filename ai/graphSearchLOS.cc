//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/graph.h"

//-------------------------------------------------------------------------------------
//                                  Searcher

// This "visitor" callback does much of the work for three different types of LOS-based
// queries:  hiding (two types), acquiring LOS, and a "choke point" finder.  
void GraphSearchLOS::onQExtraction()
{
   GraphNode   * extNode = extractedNode();
   mExtractLoc = extNode->location();
   mExtractInd = extNode->getIndex();
   setOnRelax(false);
   
   if (mLOSTable && !extNode->transient()) {
      if (mSeekingLOS) {
         if (!mNeedInside || (mLOSLoc - mExtractLoc).lenSquared() < mFarDist) {
            // Check for LOS if we're looking for it.  Note table doesn't store LOS
            // with same entry.  We save the best entry relative to the search
            // sphere if the user wants that.      
            if (mExtractInd != mLOSKey) {
               if (mLOSTable->muzzleLOS(mLOSKey, mExtractInd)) {
                  if (mNearSphere) {
                     // If we're in band outside of this sphere, take it, else
                     // keep looking but record best one found.  
                     if (mOuterSphere.isContained(mExtractLoc)) {
                        setEarlyOut(true);
                        setTarget(mExtractInd);
                     }
                     else {
                        F32 dSqrd = (mExtractLoc - mNearSphere->center).lenSquared();
                        if (dSqrd < mBestDistSqrd) {
                           setTarget(mExtractInd);
                           mBestDistSqrd = dSqrd;
                        }
                     }
                  }
                  else {
                     // No sphere, so we take first LOS point found
                     setEarlyOut(true);
                     setTarget(mExtractInd);
                  }
               }
            }
         }
      }
      else { // LOS Avoid (find hidden / choke point query)- 
         if (mExtractInd != mLOSKey) {
            if (mAvoidingLOS) {
               if (mLOSTable->hidden(mLOSKey, mExtractInd)) {
                  F32  distHidden = getSearchRef(mExtractInd)->mUser.f;
                  if (mDistanceBased) {
                     if (distHidden > mMinHidden) {
                        setEarlyOut(true);
                        setTarget(mExtractInd);
                     }
                     else {
                        // For hidden nodes that aren't the final solution, propogate
                        // distance - which happens in the relaxation virtual below...
                        setOnRelax(true);
                        if (distHidden > mBestHidden) {
                           mBestHidden = distHidden;
                           setTarget(mExtractInd);
                        }
                     }
                  }
                  else {
                     // Hide based on slope.  Look for better than supplied minimum
                     // but track the best we've found.  Note that small 2d lengths 
                     // can happen indoors- we ignore as being indeterminate.  
                     Point3F  V(mExtractLoc.x-mLOSLoc.x, mExtractLoc.y-mLOSLoc.y, 0);
                     F32   L = V.len();
                     F32   dot = (L > 0.001 ? mDot(extNode->getNormal(), V /= L) : -1);
                     
                     #ifdef _GRAPH_DEBUG_ 
                        Point3F  nodeNormal = extNode->getNormal();
                        nodeNormal *= 10;
                        LineSegment lineSeg(mExtractLoc, mExtractLoc + nodeNormal);
                        mDebugSegs.push_back(lineSeg);
                     #endif
                        
                     if (dot > mMinSlopeDot) {
                        setEarlyOut(true);
                        setTarget(mExtractInd);
                     }
                     else if (dot > mBestSlopeDot) {
                        mBestSlopeDot = dot;
                        setTarget(mExtractInd);
                     }
                  }
               }
            }
            else {// Choke point query
               if (distSoFar() < mMaxChokeDist) {
                  // Choke point query. We're interested in first time obstructed nodes
                  // which have lots of other obstructed nodes that descend from them.
                  // When we extract a descendant, we see about a new best distance.  
                  if (mLOSTable->hidden(mLOSKey, mExtractInd)) {
                     setOnRelax(true);
                     if (mHead.mPrev >= 0) {
                        mChokePoints.push_back(mExtractInd);
                     }
                     else {
                        mExtractInd = ~mHead.mPrev;   // Ones compliment has its uses!
                        SearchRef   * ancestor = getSearchRef(mExtractInd);
                        if (mHead.mUser.f > ancestor->mUser.f)
                           ancestor->mUser.f = mHead.mUser.f;
                     }
                  }
               }
               else {
                  setEarlyOut(true);
               }
            }
         }
      }//(LOS Avoid Queries)
   }
}

// Here we propogate hidden distance to neighbors that are also hidden.  
void GraphSearchLOS::onRelaxEdge(const GraphEdge* edge)
{
   if (edge->mDest != mLOSKey) 
   {
      if (mLOSTable->hidden(mLOSKey, edge->mDest)) 
      {
         SearchRef   * sref = getSearchRef(edge->mDest);
         F32   hiddenDistSoFar = mHead.mUser.f;
         sref->mUser.f = (hiddenDistSoFar + edge->mDist);
         
         // For choke point query, use mPrev field for ancestor propogation- 
         if (mChokePtQuery)
            sref->mPrev = ~mExtractInd;
      }
   }
}

U32   gGraphSearchLOSCalls = 0;

// The choke point query requires that we filter out edges going down.  We only 
// do this on those down edges which have LOS to the key point since we want the 
// ones that don't have LOS.
F32 GraphSearchLOS::getEdgeTime(const GraphEdge* edge)
{
   if (mChokePtQuery && edge->isJetting() && edge->isDown())
      if (edge->mDest != mLOSKey)
         if (!mLOSTable->hidden(mLOSKey, edge->mDest))
         {
            // Probably won't happen often, but there could be a hop down that doesn't
            // wind up as a choke point (maybe partial LOS), which will result in some
            // point further along the path (that goes through this down hop) being 
            // a choke point, and the hop down won't get filtered.  
            return SearchFailureAssure;
         }
         else
         {
            // We want short hops from edges to take priority over longer lateral
            // hops downward.  So we scale up any lateral over a few.  
            gGraphSearchLOSCalls++;
            F32   lateral = JetManager::invertLateralMod(F32(edge->getLateral()));
            F32   truncLat = getMax(lateral - 4.0f, 0.0f);
            return (truncLat * truncLat);
         }
         
   return Parent::getEdgeTime(edge);
}

//-------------------------------------------------------------------------------------

void GraphSearchLOS::setNullDefaults()
{
   setEarlyOut(false);
   mLOSTable = NULL;
   mLOSLoc.set(-1,-1,-1);
   mAvoidingLOS = mSeekingLOS = mNeedInside = mChokePtQuery = false;
   mDistanceBased = true;
   mMinSlopeDot = -1.0;
   mBestSlopeDot = -1.0;
   mFarDist = 1e22;
   mThreatCount = 0;
   mMaxChokeDist = 500;
   mBestHidden = 0.0f;
   mMinHidden = 22.0f;
   mNearSphere = NULL;
   mDebugSegs.clear();
   setOnRelax(false);
}

//-------------------------------------------------------------------------------------

Point3F GraphSearchLOS::findLOSLoc(const Point3F& from, const Point3F& wantToSee, 
   F32 minDist, const SphereF& getCloseTo, F32 capDist)
{
   setNullDefaults();
   
   if (GraphNode * losNode = gNavGraph->closestNode(wantToSee)) {
      if (GraphNode * sourceNode = gNavGraph->closestNode(from)) {

         // Configure the search for LOS seeking      
         mLOSKey = losNode->getIndex();
         mLOSLoc = losNode->location();
         mLOSTable = gNavGraph->getLOSXref();
         mNeedInside = (mFarDist = capDist) < 1e6;
         mFarDist *= mFarDist;
         (mOuterSphere = * (mNearSphere = & getCloseTo)).radius += (mNearWidth = 10);
         mBestDistSqrd = 1e13;
         mSeekingLOS = true;
         
         gNavGraph->threats()->pushTempThreat(losNode, wantToSee, minDist);
         S32   iters = GraphSearch::performSearch(sourceNode, NULL);
         gNavGraph->threats()->popTempThreat();

         if (getTarget() >= 0) {
            getPathIndices(gNavGraph->tempNodeBuff());
            return gNavGraph->lookupNode(getTarget())->location();
         }
      }
   }
   return from;
}

//-------------------------------------------------------------------------------------

Point3F GraphSearchLOS::hidingPlace(const Point3F& from, const Point3F& avoid, 
               F32 avoidRad, F32 basisForAvoidance, bool avoidOnSlope)
{
   if (GraphNode * losNode = gNavGraph->closestNode(avoid)) {
      if (GraphNode * sourceNode = gNavGraph->closestNode(from)) {
      
         // Clear out search machinery and set up what we want to do
         setNullDefaults();
      
         mLOSKey = losNode->getIndex();
         mLOSLoc = losNode->location();
         mLOSTable = gNavGraph->getLOSXref();
         mAvoidingLOS = true;
   
         // Can avoid on distance (beyond first hidden point), or on slope.  
         if (avoidOnSlope) {
            mDistanceBased = false;
            mMinSlopeDot = mCos(basisForAvoidance);
         }
         else {
            mDistanceBased = true;
            mMinHidden = basisForAvoidance;
         }
      
         // Run the search with avoid center temporarily affected.  
         gNavGraph->threats()->pushTempThreat(losNode, avoid, avoidRad);
         S32   iters = GraphSearch::performSearch(sourceNode, NULL);
         gNavGraph->threats()->popTempThreat();

         // Return if something found-       
         if (getTarget() >= 0) {
            getPathIndices(gNavGraph->tempNodeBuff());      // (debug rendering)
            return gNavGraph->lookupNode(getTarget())->location();
         }
      }
   }
   else {
      #if _GRAPH_WARNINGS_
      NavigationGraph::warning("hidingPlace() didn't find what it wanted");
      #endif
   }
   return from;
}

//-------------------------------------------------------------------------------------

// Special purpose search which makes use of mPrev field in SearchRef to 
// accomplish what it wants- relies on that field not being needed because we're
// not finding a path.  A choke point is a point just barely out of LOS from a given
// source, but beyond which there exists a long unbroken hidden string of nodes.  This
// condition keeps the bots from finding choke points around poles or into alcoves.
S32 GraphSearchLOS::findChokePoints(GraphNode* S, Vector<Point3F>& points, 
                F32 minHideDist, F32 maxSearchDist)
{
   setNullDefaults();
   points.clear();
   if ((mLOSTable = gNavGraph->getLOSXref()) != NULL) 
   {
      // Configure the machinery- 
      mLOSKey = S->getIndex();
      mLOSLoc = S->location();
      mMaxChokeDist = (maxSearchDist + minHideDist);
      mMinHidden = minHideDist;
      mChokePoints.clear();
      mChokePtQuery = true;
      
      // Do the work- 
      GraphSearch::performSearch(S, NULL);
      
      GraphNodeList  downList;
      for (S32 i = 0; i < mChokePoints.size(); i++) 
      {
         S32         nodeInd = mChokePoints[i];
         SearchRef * searchRef = getSearchRef(nodeInd);
         if (searchRef->mUser.f > minHideDist) 
         {
            // We filter out those nodes which are down a jet connection EXCEPT that if
            // very few choke points have been found, then we may use some of the nodes 
            // that are the top of such jet connections, so these are saved in downList
            S32         beforeIndex = lookupSearchRef(searchRef->mPrev)->mIndex;
            GraphNode * beforeNode = gNavGraph->lookupNode(beforeIndex);
            GraphEdge * edgeTo = beforeNode->getEdgeTo(nodeInd);
            
            if (edgeTo->isJetting() && edgeTo->isDown())
               downList.addUnique(beforeNode);
            else
               points.push_back(gNavGraph->lookupNode(nodeInd)->location());
         }
      }
      
      // If not very many points were found, then use some of the downList of locations
      // that are just prior to a downward jet connection (to a hidden choke point).  
      // ===> Might want to select a random element of downList per iteration here... 
      while (downList.size() && points.size() < 4)
      {
         points.push_back(downList.last()->location());
         downList.pop_back();
      }
   }
   return points.size();
}
