//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/graph.h"

//-------------------------------------------------------------------------------------
//                                  GraphEdge

GraphEdge::GraphEdge()
{
   mDest = -1;       // Destination node index
   mBorder = -1;     // Border segment on interiors
   mDist = 1.0;      // 3D dist, though jet edges are adjusted a little so ratings work
   mJet = 0;         // Jetting connection
   mHopOver = 0;     // For jetting, amount to hop up and over
   
   // Factors that influence edge scaling- 
   //
   mTeam = 0;        // An edge that only this team can pass - i.e. force fields.  
   mJump = 0;        // On jetting edges - is it flat enough to jump?  
   mLateral = 0;     // Crude lateral distance - needed to know if we can jet down.  
   mDown = 0;        //    Down side of a jet?  
   mSteep = 0;       // Can't walk up
   setInverse(1.0);
}

const char * GraphEdge::problems() const 
{
   if (mDist < 0)
      return "Dijkstra() forbids edge with negative distance";
   if (getInverse() < 0)
      return "Edge with negative scale";
   return NULL;
}

// Given the jet rating, can this connection be traversed?  If down, must satisfy 
// lateral only.  Used by searcher, and by partition builder.  In the latter case it 
// must (on 1st of its 2 passes) treat down same as up to find stranded partitions.
bool GraphEdge::canJet(const F32* ratings, bool both) const
{
   if (!mDown || both) 
   {
      F32   rating = ratings[mJump];
      return (rating >= mDist);
   }
   else 
   {
      F32   rating = ratings[mJump];
      return rating > F32(mLateral);
   }
}

// This table gives our inverse speed values, so we can keep it in a few bits.  Should
// still give the desired behavior, though a little coarsely at times.  Perfect paths
// in terms of travel times is not something that people seem to percieve that much...
const F32 GraphEdge::csInvSpdTab[InvTabSz + 1] = 
{
   0.0,     0.1,     0.2,     0.3,     0.4, 
   0.6,     0.8,     1.0,     1.2,     1.4, 
   
   1.7,     2.0,     2.6,     3.5,     4.6, 
   5.7,     6.8,     8.0,     9.2,     10.4, 
   
   12.0,    14.1,    17.0,    20.0,    24.0, 
   30.0,    37.0,    45.0,    54.0,    64.0, 
   
   80.0,    100.0,   125.0,   156.0,   200.0, 
   400.0,   800.0,   1600.0,  3200.0,  6400.0, 
   
   2.56e4,  1.02e5,  4.1e5,   1.6e6,   6.5e6, 
   2.6e7,   1.05e8,  4.19e8,  1.68e9,  9.9e10, 
   
   1e11,    1e12,    1e13,    1e14,    1e15,  
   1e16,    1e17,    1e18,    1e19,    1e20,
   1e21,    1e22,    1e23,    1e24, 

   // Sentinal value -  
   1e40, 
};

// Find the 6-bit number using a binary search...  
void GraphEdge::setInverse(F32 is)
{
   S32   lo = 0;
   S32   hi = InvTabSz;
   
   AssertFatal(is < csInvSpdTab[hi] && is > csInvSpdTab[lo], "Bad edge inverse speed");

   // Find the largest value we're greater than in the table- 
   for (S32 i = InvSpdBits; i > 0; i--)
   {
      S32   mid = (hi + lo >> 1);
      if (is >= csInvSpdTab[mid])
         lo = mid;
      else
         hi = mid;
   }
   
   mInverse = lo;
}

//-------------------------------------------------------------------------------------
//                         GraphNode Methods, Default Virtuals

GraphNode::GraphNode()
{
   mIsland = -1;
   mOnPath = 0;
   mAvoidUntil = 0;
   mLoc.set(-1,-1,-1);
   // mOwnedPtr = NULL;
   // mOwnedCnt = 0;
   mThreats = 0;
}

// Derived classes must supply fetcher which takes a buffer to build in. This method
// patches to that, and should serve fine if pointers to these aren't kept around.  
const Point3F& GraphNode::location() const
{
   static Point3F buff[8];
   static U8      ind = 0;
   return fetchLoc(buff[ind++ & 7]);
}

// Outdoor nodes use level (and they start at -1), others don't.  Though this is also
// used by path randomization to determine if it's reasonable to avoid the node.  
S32 GraphNode::getLevel() const
{
   return -2;
}

Point3F GraphNode::getRenderPos() const
{
   Point3F  buff, adjusted;
   adjusted = fetchLoc(buff);
   adjusted.z += 0.3;
   return adjusted;
}

// Set if not already set (shouldn't be set already, caller asserts)
void GraphNode::setIsland(S32 num)
{
   AssertFatal(!mFlags.test(GotIsland), "GraphNode::setIsland()");
   mFlags.set(GotIsland);
   mIsland = num;
}

// Ideally this ratio would accurately reflect travel speeds along the slopes.  
// Here we use 
//       xy / total           If we're going down.  Hence XY is net path distance.
//       total / xy           If we're going up.  
static F32 getBaseEdgeScale(const Point3F& src, Point3F dst)
{
   F32   totalDist = (dst -= src).len();
   bool  goingUp = (dst.z > 0);
         dst.z = 0;
   F32   xyDist = dst.len();
   F32   ratio = 1.0;

   if (xyDist > 0.01 && totalDist > 0.01)
   {
      if (goingUp)
      {
         ratio = (totalDist / xyDist);
         if (ratio < 1.2)
            ratio = 1.0;
      }
      else
         ratio = (xyDist / totalDist);

      // Want to do something between square root and straight value, so we'll 
      // raise it to the 3/4 power here, which will bring it to 1 from either side.
      ratio = mSqrt(mSqrt(ratio) * ratio);
   }
   
   return ratio;
}

F32 GraphNode::edgeScale(const GraphNode * dest) const
{
   // F32   scale = 1.0;
   F32   scale = getBaseEdgeScale(location(), dest->location());
   
   if (submerged())
      scale *= gNavGraph->submergedScale();
   else if (shoreline())
      scale *= gNavGraph->shoreLineScale();
   
   if (dest->submerged())
      scale *= gNavGraph->submergedScale();
   else if (dest->shoreline())
      scale *= gNavGraph->shoreLineScale();

   // Searcher reserves REALLY large numbers (10^20th about) to signal search 
   // failure, so make sure we don't inadvertently interfere with any of that.
   scale = getMin(scale, 1000000000.0f);
      
   return scale;
}

static GraphEdge sEdgeBuff[MaxOnDemandEdges];

bool GraphNode::neighbors(const GraphNode* other) const
{
   GraphEdgeArray edgeList = other->getEdges(sEdgeBuff);
   S32            indexOfThis = getIndex();
   
   while (GraphEdge * edge = edgeList++)
      if (edge->mDest == indexOfThis)
         return true;
         
   return false;
}
   
GraphEdge * GraphNode::getEdgeTo(S32 to) const
{
   GraphEdgeArray edgeList = getEdges(sEdgeBuff);
   
   while (GraphEdge * edge = edgeList++)
      if (edge->mDest == to)
         return edge;
         
   return NULL;
}

// Virtual that is only used on non-grid nodes
const GraphEdge * GraphNode::getEdgePtr() const
{
   return 0;
}

// This is only called on nodes known to be terrain.  
F32 GraphNode::terrHeight() const
{
   return 1e17;
}

// Initial use of this is for weighted random selection of nodes for spawn points.
F32 GraphNode::radius() const
{
   return 2.0;
}

// Not in yet, but we'll use this instead of radius for weighted random spawn points
F32 GraphNode::area() const
{
   return 0.01;
}

// Used to know thinness of node.  Spawn system avoids, as do smoothing edges.  
F32 GraphNode::minDim() const
{
   return 0.01;
}

const Point3F& GraphNode::getNormal() const
{
   static const Point3F sStraightUp(0,0,1);
   return sStraightUp;
}

// Used for finding how much a node contains a point.  More negative => point is 
// more inside.  
NodeProximity GraphNode::containment(const Point3F& point) const
{
   NodeProximity  nodeProx;
   nodeProx.mLateral = (point - location()).len() - radius();
   return nodeProx;
}

Point3F GraphNode::randomLoc() const 
{
   return location();
}

// Nodes which have volumes associated.  Added to allow the transient nodes to 
// respond with the proper volume that they may be associated with.  
S32 GraphNode::volumeIndex() const 
{
   return getIndex();
}

// Set the avoidance if it isn't already.  The inmportant flag will override though.
void GraphNode::setAvoid(U32 duration, bool isImportant)
{
   U32   curTime = Sim::getCurrentTime();
   
   if (curTime > mAvoidUntil || isImportant) 
   {
      mAvoidUntil = curTime + duration;
      mFlags.set(StuckAvoid, isImportant);

      // On avoidance, also mark neighbors-    
      if (isImportant) 
      {   
         GraphEdgeArray edgeList = getEdges(sEdgeBuff);
         while (GraphEdge * edge = edgeList++) 
         {
            GraphNode * node = gNavGraph->lookupNode(edge->mDest);
            node->mAvoidUntil = mAvoidUntil;
            node->mFlags.clear(StuckAvoid);     // slight avoid here. 
         }
      }
   }
}

//-------------------------------------------------------------------------------------
//             Default RegularNode Virtuals

RegularNode::RegularNode()
{
   mIndex = -1;
   mNormal.set(0,0,1);
}

GraphEdgeArray RegularNode::getEdges(GraphEdge*) const
{
   return GraphEdgeArray(mEdges.size(), mEdges.address());
}

GraphEdge & RegularNode::pushEdge(GraphNode * node)
{
   GraphEdge   edge;
   edge.mDest = node->getIndex();
   mEdges.push_back(edge);
   return mEdges.last();
}

const Point3F& RegularNode::getNormal() const
{
   return mNormal;
}

//-------------------------------------------------------------------------------------

void GraphNodeList::setFlags(U32 bits)
{
   for(const_iterator it = begin(); it != end(); it++)
      if(*it)
         (*it)->mFlags.set(bits);
}

void GraphNodeList::clearFlags(U32 bits)
{
   for(const_iterator it = begin(); it != end(); it++)
      if(*it)
         (*it)->mFlags.clear(bits);
}

S32 GraphNodeList::searchSphere(const SphereF& sphere, const GraphNodeList& listIn)
{
   clear();
   F32 radiusSquared = (sphere.radius * sphere.radius);
   for(const_iterator it = listIn.begin(); it != listIn.end(); it++)
      if(*it)
         if((sphere.center - (*it)->location()).lenSquared() < radiusSquared)
            push_back(*it);
   return size();
}

GraphNode * GraphNodeList::closest(const Point3F& loc, bool)
{
   GraphNode * bestNode = NULL;
   F32         bestRadSquared = 1e12, dSqrd;
   for (const_iterator it = begin(); it != end(); it++)
      if (*it)
         if ((dSqrd = (loc - (*it)->location()).lenSquared()) < bestRadSquared)
         {
            bestRadSquared = dSqrd;
            bestNode = *it;
         }
   return bestNode;
}

// Add the node pointer if it's not already in the list. 
bool GraphNodeList::addUnique(GraphNode * node)
{
   for (iterator n = begin(); n != end(); n++)
      if (* n == node)
         return false;
   push_back(node);
   return true;
}

