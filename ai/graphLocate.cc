//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/graph.h"

#define  WaitAfterNearbyFound    11
#define  WaitAfterNothingFound   16

// Just want to construct a really good proximity for forcing sorting first below- 
static struct BestProx: public NodeProximity{ BestProx(){makeGood();} } sBestProximity;

//-------------------------------------------------------------------------------------

bool NodeProximity::possible() const
{
   return (mLateral < 4.0);
}

ProximateNode::ProximateNode(const NodeProximity& p, GraphNode* n)
{
   mProximity = p;
   mNode = n;
}

static S32 QSORT_CALLBACK varCompare(const void* a,const void* b)
{
   ProximateNode *   proxA = (ProximateNode*)a;
   ProximateNode *   proxB = (ProximateNode*)b;
   if (proxA->mProximity < proxB->mProximity)
      return -1;
   else if (proxA->mProximity > proxB->mProximity)
      return 1;
   else
      return 0;
}

void ProximateList::sort()
{
   dQsort((void *)address(), size(), sizeof(ProximateNode), varCompare);
}

//-------------------------------------------------------------------------------------

// Ok - "dist" searcher is now a misnomer.  It's really a depth searcher, and it's 
// only used for the locator.  
GraphSearchDist::GraphSearchDist()
{
   mBestMatch = NULL;
   mPoint.set(0,0,0);
   mMaxSearchDist = 100;
   setOnDepth(false);
}

void GraphSearchDist::setDistCap(F32 d)
{
   mMaxSearchDist = d;
}

// For the graph locator, we actually just want to search out to a certain DEPTH, 
// rather than on distance (go a few connections removed).  
F32 GraphSearchDist::getEdgeTime(const GraphEdge* edge)
{
   if (mSearchOnDepth)
      return 1.001f;
   else
      return edge->mDist;
}

// The tracker needs to know travel distance if the location has left it's last node.  
void GraphSearchDist::onQExtraction()
{
   GraphNode * extractNode = extractedNode();
   if (extractNode->indoor()) 
   {
      NodeProximity  metric = extractNode->containment(mPoint);
      
      if (metric.inside()) 
      {
         mBestMatch = extractNode;
         mBestMetric = metric;
         mProximate.clear();
         setEarlyOut(true);
      }
      else 
      {
         // We need to track if we've had any really close matches, and then 
         // disregard the high ones.  Note that the definition of "really close" 
         // must probably account for whether the destination has a bounding box
         // or not.  
         if (metric.possible()) 
            if (metric < mCurThreshold) 
            {
               // Add to the list of possibilities.  
               ProximateNode  candidate(metric, extractNode);
               mProximate.push_back(candidate);
            
               if (metric < mBestMetric) 
               {
                  mBestMetric = metric;
                  mCurThreshold = getMax(F32(metric), 1.0f);
               }
            }
      }
      F32   thusFar = (mSearchOnDepth ? timeSoFar() : distSoFar());
      if (thusFar > mMaxSearchDist)
         setEarlyOut(true);
   }
}

// Find the best indoor node to connect to, return metric for how good.  
// The lower the number the better (using distance from planes).  
NodeProximity GraphSearchDist::findBestMatch(GraphNode * current, const Point3F& loc)
{
   // Set threshold based on how good the initial match is.  
   mBestMatch = NULL;
   mBestMetric.makeBad();
   mProximate.clear();
      
   if (!current)
      current = gNavGraph->closestNode(loc);
      
   if (current) 
   {
      setEarlyOut(false);
      mPoint = loc;
      mCurThreshold = 1e13;
      if (current->indoor())
         setDistCap(3.3);
      else
         setDistCap(2.2);
      setOnDepth(true);
      performSearch(current);
      setOnDepth(false);
   }
      
   return mBestMetric;
}

//-------------------------------------------------------------------------------------

GraphLocate::GraphLocate()
{
   reset();
   mMounted = false;
}

void GraphLocate::reset()
{
   mLocation.set(1e13,1e13,1e13);
   mLoc2D = mLocation;
   mTraveled = 1e13;
   mClosest = NULL;
   mCounter = 0;
   mTerrain = false;
   mUpInAir = false;
}

void GraphLocate::forceCheck()
{
   mCounter = 0;
   mTraveled = 1e13;
}

//-------------------------------------------------------------------------------------

static const U32 sMask = InteriorObjectType;

static bool haveLOS(Point3F src, Point3F dst, bool indoor)
{
   static RayInfo coll;
   src.z += 0.13;
   dst.z += 0.13;
   
   // Probably don't usually need collide with terrain at all, but there are some cases
   // on proximity check to indoor nodes.  
   U32   mask = indoor ? (sMask|TerrainObjectType) : sMask;
   
   if (!gServerContainer.castRay(src, dst, mask, &coll))
      return true;
   else {
      // check vehicle collision- ? 
      return false;
   }
}

// We allow a slight increase on the lower height, up to step height (see Generators
// down in Masada bunker - top of steps, fail LOS).  
static bool checkLOS(const Point3F& src, const Point3F& dst, bool indoor)
{
   if (!haveLOS(src, dst, indoor)) 
   {
      // Do more liberal check, raising up the lower point by up to 0.7 (~step height).
      Point3F  high, low;
      if (src.z < dst.z)
         low = src, high = dst;
      else
         low = dst, high = src;
      low.z += getMin(0.7f, high.z - low.z);
      return haveLOS(low, high, indoor);
   }
   return true;
}

//-------------------------------------------------------------------------------------


bool GraphLocate::canHookTo(const GraphLocate& other, bool& mustJet) const
{
   bool  canHook = false;
   
   if (mClosest && other.mClosest) 
   {
      if (onTerrain() && other.onTerrain()) 
      {
         canHook = (mClosest == other.mClosest || mClosest->neighbors(other.mClosest));
      }
      else 
      {
         // might want more checking here (which is why the clause is separate...)
         canHook = (mClosest == other.mClosest);
      }
   }
   
   if (canHook)
      mustJet = (mUpInAir || other.mUpInAir);
   
   return canHook;
}

void GraphLocate::setMounted(bool b)
{
   if (mMounted != b) 
   {
      mMounted = b;
      mClosest = NULL;
      forceCheck();
   }
}

// I think we can get away with doing the 2D check here since it's established that
// we're Ok in 3D and we're not looking at jet connections.  (The reason it's needed 
// is because guys (especially heavies) are actually far off the edge in Z on slopes!).
bool GraphLocate::checkRegularEdge(Point3F fromLoc, GraphEdge* edge)
{
   GraphNode   *  destNode = gNavGraph->lookupNode(edge->mDest);
   Point3F        destLoc = destNode->location();
   destLoc.z = fromLoc.z = 0.0f;
   LineSegment edgeSeg(fromLoc, destLoc);
   
   //==> If we can preprocess a walking width onto these edges, that could be good.  
   //==>    For now the value is hard coded.  
   F32   dist = edgeSeg.distance(mLoc2D);
   return (dist <= 0.8);
}

ProximateNode * GraphLocate::examineCandidates(ProximateList& proximate)
{
   ProximateNode   * bestIndoor = NULL;
   ProximateNode   * bestOutdoor = NULL;
   
   mConnections.clear();
   if (proximate.size()) 
   {
      proximate.sort();
      for (ProximateList::iterator i = proximate.begin(); i != proximate.end(); i++) 
      {
         bool  isIndoor = i->mNode->indoor();
         if (checkLOS(i->mNode->location(), mLocation, isIndoor)) 
         {
            pushEdge(i->mNode);
            if (bestIndoor == NULL)                // check for return value
            {
               if (isIndoor)
                  bestIndoor = i;
               else
                  bestOutdoor = i;
            }
         }
         if (mConnections.size() > 3)
            break;
      }
   }
   else 
   {
      #if _GRAPH_WARNINGS_
         NavigationGraph::warning("No proximate nodes found in graph locator");
      #endif
   }
   
   // See if we want final jetting connection, and set that only for the best match.  
   if (mMounted && mConnections.size()) 
   {
      mConnections[0].mBorder = -1;
      mConnections[0].setJetting();
   }
   
   if (bestIndoor)
      return bestIndoor;
   else
      return bestOutdoor;
}

// Fetch neighbors onto list.  Be careful with all the different edge types (jet, 
// border edges, walkable straight connections).  Routine is only called when we're
// inside the node, so we can connect to all except certain straight line edges.  
void GraphLocate::getNeighbors(GraphNode* ofNode, bool outdoor, bool makeJet)
{
   static GraphEdge  edgeBuffer[MaxOnDemandEdges];
   GraphEdgeArray    edgeList = ofNode->getEdges(edgeBuffer);
   S32               outdoorNumber = gNavGraph->numOutdoor();
   Point3F           fromLoc = ofNode->location();
   
   if (outdoor) {
      while(GraphEdge * edge = edgeList++) {
         if(edge->mDest < outdoorNumber) {
            GraphEdge   connection = * edge;
            if (!connection.isSteep())
            {
               if (makeJet || connection.isJetting())
                  connection.setJetting();
               mConnections.push_back(connection);
            }
         }
         else {
            // Check if we're near the edge.  Note we know this isn't a 'border' edge 
            // because terrains nodes don't have border edges going out.  
            if (!edge->isJetting() && checkRegularEdge(fromLoc, edge))
               mConnections.push_back(* edge);
         }
      }
   }
   else {
      // From indoor node we're inside.
      while (GraphEdge * edge = edgeList++) {
         if (edge->mDest >= outdoorNumber) {  // (indoor dest)
            if (makeJet) {
               // If we're inside the neighbor's top and bottom planes, we can make
               //    a jet connection. 
               if (gNavGraph->verticallyInside(edge->mDest, mLocation)) {
                  GraphNode * destNode = gNavGraph->lookupNode(edge->mDest);
                  if (gNavGraph->possibleToJet(mLocation, destNode->location())) {
                     GraphEdge   jetEdge;
                     jetEdge.mDest = edge->mDest;
                     jetEdge.setJetting();
                     jetEdge.mDist = edge->mDist;
                     jetEdge.copyInverse(edge);
                     mConnections.push_back(jetEdge);
                  }
               }
            }
            else {
               if (!edge->isJetting())
                  if (edge->isBorder() || checkRegularEdge(fromLoc, edge))
                     mConnections.push_back(* edge);
            }
         }
         else {  // Neighbor is outdoor- 
            if (makeJet) {
               // Since this indoor node has a terrain neighbor, we know the location
               // is above the terain.  However the loc could be under top of node
               // in the case of some shaded nodes.  
               GraphNode * destNode = gNavGraph->lookupNode(edge->mDest);
               Point3F     destLoc = destNode->location();
               F32  topZ = (destLoc.z + destNode->terrHeight());
               if (mLocation.z < topZ) {
                  if (gNavGraph->possibleToJet(mLocation, destLoc)) {
                     mConnections.push_back(* edge);
                     mConnections.last().setJetting();
                  }
               }
            }
            else {
               // Can check near edge here on other types of connections.
               if (!edge->isJetting() && checkRegularEdge(fromLoc, edge))
                  mConnections.push_back(* edge);
            }
         }
      }
   }
}

void GraphLocate::pushEdge(GraphNode * to, F32 * getCosine)
{
   GraphEdge   edge;
   edge.mDest = to->getIndex();
   Point3F  toLoc = to->location();
   edge.mDist = (toLoc -= mLocation).len();

   // Added for steepness detection (LH 11/12/00).  
   if (getCosine)
   {
      if (edge.mDist < 0.01)
         * getCosine = 1.0;
      else 
      {  
         toLoc.z = 0;
         * getCosine = (toLoc.len() / edge.mDist);
      }
   }
   
   mConnections.push_back(edge);
}

// We're inside this one, so we can hook to it and all neighbors.  
void GraphLocate::beLikeNode(GraphNode* ourNode)
{
   mConnections.clear();
   bool  tryJet = false;      
   bool  outdoor = !ourNode->indoor();
   if (mMounted) {
      if (outdoor) {
         F32   height;
         if (gNavGraph->terrainHeight(mLocation, &height))
            tryJet = (mLocation.z - height) > 2.0;
      }
      else {
         // Handle indoor with volume checks.  Since we know we're inside, just
         // check the height above the floor.  
         if (gNavGraph->heightAboveFloor(ourNode->getIndex(), mLocation) > 2.0)
            tryJet = true;
      }
   }
   
   // Get all neighbors of the node we're inside of
   getNeighbors(mClosest = ourNode, outdoor, tryJet);
   
   F32   cosAngle = 1.0;
   pushEdge(mClosest, outdoor ? &cosAngle : NULL);
   if (tryJet || cosAngle < gNavGlobs.mPrettySteepDot)
      mConnections.last().setJetting();
   mTraveled = 0.0f;
   mCounter = 0;
   mUpInAir = tryJet;
}

void GraphLocate::update(const Point3F& newLoc)
{
   bool     searchIsReady = ((--mCounter < 0) && (mTraveled > 0));
   F32      moveDist = (mLocation - newLoc).len();
   
   // Update the traveled total
   mTraveled += moveDist;

   // Check for big jumps - redo search.     
   // if (moveDist > 30.0)
   if (moveDist > 5.0)
      mClosest =  NULL;
   
   if ((moveDist > 0.1) || searchIsReady)
   {
      mLocation = newLoc;
      mLoc2D.set(mLocation.x, mLocation.y, 0.0);
      mUpInAir = false;
      
      if (GraphNode * onTerr = gNavGraph->findTerrainNode(mLocation)) 
      {
         mTerrain = true;
         mMetric.makeBad();
         beLikeNode(onTerr);
      }
      else 
      {
      
         mTerrain = false;
         NodeProximity  newMetric;

         // If the node match is good, it means we can afford to search as soon as
         // the location leaves that node.  Note we can move inside a node that
         // we were just near to before, so the connections should be remade.  
         if (mClosest && (newMetric = mClosest->containment(mLocation)).inside()) 
         {
            if (!mMetric.inside())
               beLikeNode(mClosest);
            else if (mCounter < 0) 
            {
               // Want to re-evaluate 'straight-line' edge connections every so often. 
               if (mCounter < -4) 
                  beLikeNode(mClosest);
            }
            else
               mCounter = 0;
               
            mMetric = newMetric;
         }
         else if (searchIsReady) 
         {
            GraphSearchDist * searcher = gNavGraph->getDistSearcher();
            mMetric = searcher->findBestMatch(mClosest, mLocation);
            if (searcher->bestMatch()) 
            {
               beLikeNode(searcher->bestMatch());
            }
            else 
            {
               // Check proximate list.  
               ProximateList&  nearby = searcher->getProximate();
               
               if (GraphNode * terr = gNavGraph->nearbyTerrainNode(mLocation)) 
               {
                  ProximateNode  willSortFirst(sBestProximity, terr);
                  nearby.push_back(willSortFirst);
               }

               if (ProximateNode * bestBet = examineCandidates(nearby)) 
               {
                  mClosest = bestBet->mNode;
                  mCounter = WaitAfterNearbyFound;
                  mTraveled = getMax( static_cast<F32>( bestBet->mProximity ), 0.0f );
               }
               else 
               {
                  // When nothing found, we don't reset the travel distance - this
                  // will increase the search distance gradually in the case where 
                  // we somehow miss a match, get out of system.  Uh, should work..
                  mClosest = NULL;
                  mCounter = WaitAfterNothingFound;
               }
               
               mCounter += (gRandGen.randI() & 3);     // Little bit of stagger
            }
         }
      }
   }
}

