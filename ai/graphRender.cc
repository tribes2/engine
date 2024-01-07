//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/graph.h"
#include "dgl/dgl.h"
#include "Core/color.h"

//-------------------------------------------------------------------------------------

static bool sShowFieldEdges = false;
static bool sFilterDownEdges = false;
static S32  sShowFFTeam = 0;
static S32  sEdgesDrawn = 0;

//-------------------------------------------------------------------------------------

struct SortedNode 
{
   GraphNode * node;
   F32         dist;
   SortedNode(GraphNode* n=NULL, F32 d=0.0)   {node = n; dist = d;}
};

class SortedList : public Vector<SortedNode> 
{
      static S32 QSORT_CALLBACK cmpSortedNodes(const void* , const void* );
   public:
      void  init(const GraphNodeList& list, const Point3F& camLoc);
      void  sort();
};

S32 QSORT_CALLBACK SortedList::cmpSortedNodes(const void * a,const void * b)
{
   F32   A = ((SortedNode*)a)->dist;
   F32   B = ((SortedNode*)b)->dist;
   return (A < B ? -1 : (A > B ? 1 : 0));
}

void SortedList::sort()
{
   dQsort((void* )(this->address()), this->size(), sizeof(SortedNode), cmpSortedNodes);
}

// Set up the list with distances (squared (faster)), and sort it.  
void SortedList::init(const GraphNodeList& list, const Point3F& camLoc)
{
   clear();
   for (S32 i = 0; i < list.size(); i++)
   {
      GraphNode   *  node = list[i];
      SortedNode     sortNode(node, (node->location() - camLoc).lenSquared());
      push_back(sortNode);
   }
   sort();
}

//-------------------------------------------------------------------------------------

extern void wireCube(F32 size, Point3F pos);

static void drawNode(const Point3F& pos, const ColorF& color, U32 whichType = 0)
{
   glPointSize(whichType ? 9 : 4);
   glBegin(GL_POINTS);
      if (whichType)
         glColor3f(1, 1, 1);
      else
         glColor3f(color.red, color.green, color.blue);
      glVertex3f(pos.x, pos.y, pos.z);
   glEnd();
}

static void drawNode(const Point3F& pos, U32 whichType = 0)
{
   glPointSize(whichType ? 9 : 4);
   glBegin(GL_POINTS);
      if (whichType)
         glColor3f(1, 1, 1);
      else
         glColor3f(1, 0, 0);
      glVertex3f(pos.x, pos.y, pos.z);
   glEnd();
}

// Draw node size to indicate roam radius amount.  Try square root relationship.  
static void drawOutdoorNode(const Point3F& pos, F32 roamRad, bool highlight)
{
   F32   mapRoamRad = mSqrt(roamRad * 2.0);
   S32   numPoints = mFloor(mapRoamRad) + 1;
   glPointSize(numPoints);
   glBegin(GL_POINTS);
      if (highlight)
         glColor3f(1, 1, 1);
      else
         glColor3f(1, 0, 0);
      glVertex3f(pos.x, pos.y, pos.z);
   glEnd();
}

static void renderEdge(const Point3F& src, const Point3F& dst, 
                  const ColorF* col = 0, bool wide=false, bool flip = false)
{
   const Point3F& from = (flip ? dst : src);
   const Point3F& to = (flip ? src : dst);
   Point3F  extra = (to - from);
   F32      len = extra.len();
   if (len > 0.01)
   {
      extra *= (0.04 / len);
      extra += to;
      glLineWidth(wide ? 3 : 1);
      glBegin(GL_LINES);
         if(col)
            glColor3f(col->red, col->green, col->blue);
         else
            glColor3f(0, 0.3, 1);
         glVertex3f(from.x, from.y, from.z);
         glVertex3f(extra.x, extra.y, extra.z);
      glEnd();
      glLineWidth(1);
      sEdgesDrawn++;
   }
}

//-------------------------------------------------------------------------------------

static void renderSegments(Vector<LineSegment>& segments)
{
   static Point3F    morph(0.0007,0.0037,0.0013);
   static Point3F    a(0,0,0);
   
   for (S32 i = 0; i < segments.size(); i++)
   {
      ColorF    color(mFabs(mCos(a.x)), mFabs(mCos(a.y)), mFabs(mCos(a.z)));
      a += morph;
      if (a.x > M_2PI)     a.x -= M_2PI;
      if (a.y > M_2PI)     a.y -= M_2PI;
      if (a.z > M_2PI)     a.z -= M_2PI;
      
      LineSegment&   seg = segments[i];
      renderEdge(seg.getEnd(0), seg.getEnd(1), &color);
   }
}

static void renderBoxes(Vector<Point3F>& boxLocs)
{
   for (S32 i = 0; i < boxLocs.size(); i++) {
      Point3F  loc = boxLocs[i];
      wireCube(0.22f, loc);
   }
}

void NavigationGraph::pushRenderSeg(const LineSegment& lineSeg)
{
   if (mRenderThese.size() < 800)
      mRenderThese.push_back(lineSeg);
}

void NavigationGraph::pushRenderBox(Point3F boxLoc, F32 zadd)
{
   if (mRenderBoxes.size() < 100) {
      boxLoc.z += zadd;
      mRenderBoxes.push_back(boxLoc);
   }
}

//-------------------------------------------------------------------------------------
//       RENDERING

void NavigationGraph::render(Point3F &camPos, bool)
{
   GridArea w = getWorldRect();
   
   if (mNodeList.size() && gotOneWeCanUse())
   {
      U32   filters = Con::getIntVariable("$GraphRenderFilter");
      sFilterDownEdges = (filters & 1);
      
      sShowFFTeam = Con::getIntVariable("$GraphShowFFTeam");
      
      // Signal fields with flashing- 
      sShowFieldEdges = !(Sim::getCurrentTime() & 0x600);
      
      // Render the last path query, avoid transients on ends.  Note we need to switch
      // to something which uses a bit on the edge instead of counter on node.  
      for (S32 k = mTempNodeBuf.size() - 1; k >= 0; k--)
         if (GraphNode * tempNode = lookupNode(mTempNodeBuf[k]))
            if (!tempNode->transient())
               tempNode->setOnPath();
   
      if (haveTerrain() && sDrawOutdoorNodes)
      {
         GridArea c = getGridRectangle(camPos, 13);
         if (w.intersect(c))
         {
            GraphEdge      edgeBuffer[MaxOnDemandEdges]; 
            GraphNodeList  nodesInArea;
            SortedList     sortedList;
         
            S32 numNodes = getNodesInArea(nodesInArea, w);
            sortedList.init(nodesInArea, camPos);
            sEdgesDrawn = 0;
      
            for (S32 i = 0; i < numNodes && sEdgesDrawn < sEdgeRenderMaxOutdoor; i++)
            {
               bool        highlight;
               GraphNode * node = sortedList[i].node;
               if (sShowThreatened < 0)
                  highlight = (node->render0() || node->submerged());
               else
                  highlight = (node->threats() & showWhichThreats());
               Point3F     nodeLoc = node->getRenderPos();
               F32         roamRad = getRoamRadius(nodeLoc);
               drawOutdoorNode(nodeLoc, roamRad, highlight);
               drawNeighbors(node, node->getEdges(edgeBuffer), camPos);
            }
         }
      }
      
      // if (sDrawTransients)
      if (sDrawOutdoorNodes || sDrawIndoorNodes)
         renderTransientNodes();
         
      if (sDrawIndoorNodes)
         renderInteriorNodes(camPos);
      
      if (mRenderThese.size() > 0) {
         renderSegments(mRenderThese);
         if (mRenderThese.size() > 9) {
            for (S32 i = mRenderThese.size() / 8; i >= 0; i--)
               mRenderThese.erase_fast((gRandGen.randI()&0xFFFFFF) % mRenderThese.size());
         }
      }
      
      if (mRenderBoxes.size() > 0) {
         renderBoxes(mRenderBoxes);
         if (mRenderBoxes.size() > 3) {
            for (S32 i = mRenderBoxes.size() / 3; i >= 0; i--)
               mRenderBoxes.erase_fast((gRandGen.randI()&0xFFFFFF) % mRenderBoxes.size());
         }
      }
   }
}

#define  IndoorRenderThresh   49.0
#define  MinIndoorBoxWidth    7.0
#define  MaxIndoorBoxWidth    57.0
#define  MaxIndoorRender      100

void NavigationGraph::renderInteriorNodes(Point3F camPos)
{
   static F32        boxW = 24;
   GraphEdge         edgeBuffer[MaxOnDemandEdges]; 
   ColorF            color(0, 1, 0);
   GraphNodeList     renderList;

   //==>  Would of course be better to clip to box in FRONT of camera...  
   Point3F  boxOff(boxW, boxW, 111);
   Box3F    clipBox(camPos - boxOff, camPos + boxOff);
   mIndoorTree.getIntersecting(renderList, clipBox);
   S32  numNodes = renderList.size();
   
   SortedList     sortedList;
   sortedList.init(renderList, camPos);
   sEdgesDrawn = 0;

   // Resize box based on density.  Note we always want to go tall though.     
   // if (numNodes <= MaxIndoorRender)
   //    boxW = getMin(MaxIndoorBoxWidth, boxW + 0.5);
   // else 
   //    boxW = getMax(MinIndoorBoxWidth, boxW - 0.5);

   for (S32 i = 0; i < numNodes && sEdgesDrawn < sEdgeRenderMaxIndoor; i++) 
   {
      bool           highlight;
      GraphNode *    node = sortedList[i].node;
      Point3F        pos = node->getRenderPos();
      GraphEdgeArray edges = node->getEdges(edgeBuffer);
      if (sShowThreatened < 0)
         highlight = node->render0();
      else
         highlight = (node->threats() & showWhichThreats());
      drawNode(pos, color, highlight);
      drawNeighbors(node, edges, camPos, getMin(boxW + 40.0, 80.0));
   }
}

void NavigationGraph::renderTransientNodes()
{
   GraphEdge   edgeBuffer[MaxOnDemandEdges];
   
   for (S32 i = 0; i < mMaxTransients; i++)
      if (GraphNode * node = mNodeList[mTransientStart + i]) {
         Point3F pos = node->getRenderPos();
         drawNode(pos);
         drawNeighbors(node, node->getEdges(edgeBuffer), pos);
      }
}

// Find the two intermediate points to connect to between node volumes.  We are given 
// that the edge has a border definition.  
void NavigationGraph::getCrossingPoints(S32 from, Point3F* twoPoints, GraphEdge* edge)
{
   GraphBoundary &   B = mBoundaries[edge->mBorder];
   twoPoints[0] = B.seekPt;
   
   if (GraphEdge * edgeBack = lookupNode(edge->mDest)->getEdgeTo(from))
      twoPoints[1] = mBoundaries[edgeBack->mBorder].seekPt;
   else
      twoPoints[1] = B.seekPt + (B.normal * (B.distIn * 2));
   
   twoPoints[0].z += 0.22;
   twoPoints[1].z += 0.22;
}

static const ColorF  borderColor(0.0, 0.9, 0.11);        // green
static const ColorF  steepColor(0.5, 0.5, 0.5);          // grey 
static const ColorF  jettingColor(0.9, 0.9, 0.0);        // yellow
static const ColorF  pathColor(1.0, 1.0, 1.0);           // white

// Note that index is only used for comparison to eliminate drawing both ways.  
// The transient draw routine above forces the draw by passing zero.  The others
// use their proper index.  
void NavigationGraph::drawNeighbors(GraphNode* fromNode, GraphEdgeArray edges, 
            const Point3F& camPos, F32 within)
{
   Point3F  fromLoc = fromNode->getRenderPos();
   S32      fromIndex = fromNode->getIndex();
   bool     isTransient = fromNode->transient();
   
   within *= within;

   while (GraphEdge * edge = edges++) {
      // if (edge->mDest > fromIndex || isTransient) {
      if (1) {         // Think we need to render both ways...  
         // Flash field edges- 
         if (edge->getTeam() && !sShowFieldEdges)
         {
            // Console variable $graphShowFFTeam will make it only flash those edges
            // owned by the specified team- 
            if (!sShowFFTeam || edge->getTeam() == sShowFFTeam)
               continue;
         }
         GraphNode * toNode = mNodeList[edge->mDest];
         Point3F     toLoc = toNode->getRenderPos(), mid, low, high;
         ColorF      color(0.07, 0.07, 0.98);
         F32         shadePct = F32(toNode->onPath()) * (1.0 / F32(GraphMaxOnPath));
         bool        whitenPath = (shadePct > 0.0 && fromNode->onPath());
         bool        isJetting = edge->isJetting();
         bool        isSteep = edge->isSteep();
         bool        isBorder = edge->isBorder();
         
         if (sFilterDownEdges && (toLoc.z < fromLoc.z))
            continue;

         if       (isSteep)      color = steepColor;
         else if  (isJetting)    color = jettingColor;
         else if  (isBorder)     color = borderColor;
         
         if (whitenPath) {
            color.red = scaleBetween(color.red, 1.0f, shadePct);
            color.green = scaleBetween(color.green, 1.0f, shadePct);
            color.blue = scaleBetween(color.blue, 1.0f, shadePct);
            toNode->decOnPath();
            fromNode->decOnPath();
         }

         if (isJetting) {
            if (sDrawJetConnections && (toLoc-camPos).lenSquared() < within) {
               bool  flip;
               if(fromLoc.z > toLoc.z) {
                  flip = true;
                  (mid = low = toLoc).z = (high = fromLoc).z;
               }
               else {
                  flip = false;
                  (mid = low = fromLoc).z = (high = toLoc).z;
               }
                  
               if (edge->hasHop()) {
                  Point3F  aboveH(high.x, high.y, high.z + edge->getHop());
                  mid.z += edge->getHop();
                  renderEdge(low, mid, &color, whitenPath, flip);
                  renderEdge(mid, aboveH, &color, whitenPath, flip);
                  renderEdge(aboveH, high, &color, whitenPath, flip);
               }
               else {
                  renderEdge(fromLoc, mid, &color, whitenPath);
                  renderEdge(mid, toLoc, &color, whitenPath);
               }
            }
         }
         else if (isBorder && mHaveVolumes) {
            Point3F  connectPts[3+1];
            connectPts[0] = fromLoc;
            connectPts[3] = toLoc;
            getCrossingPoints(fromIndex, &connectPts[1], edge);
            for (S32 c = 0; c < 3;) {
               Point3F  end = connectPts[c++];
               Point3F  start = connectPts[c];
               if (isTransient && !whitenPath) {
                  end.z += 0.28, start.z += 0.28;
                  ColorF   invertColor(1-color.red, 1-color.green, 1-color.blue);
                  renderEdge(start, end, &invertColor, whitenPath);
               }
               else {
                  renderEdge(start, end, &color, whitenPath);
               }
            }
         }
         else {
            renderEdge(fromLoc, toLoc, &color, whitenPath);
         }
      }
   }//edge loop
}                                
