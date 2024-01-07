//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/graph.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "terrain/terrRender.h"

//-------------------------------------------------------------------------------------

#ifdef DEBUG
#define  DebugCheckHanging    1
#else
#define  DebugCheckHanging    0
#endif

//-------------------------------------------------------------------------------------

static bool cullOneEdgeDup(GraphEdgeList& edges, BitVector& mark)
{
   GraphEdgeList::iterator it, cull = NULL;
   
   for (it = edges.begin(); it != edges.end(); it++)
      if (mark.test(it->mDest)) {
         cull = it;
         break;
      }
      else
         mark.set(it->mDest);
   
   for (it = edges.begin(); it != edges.end(); it++)
      mark.clear(it->mDest);
      
   if (cull) 
      edges.erase_fast(cull);
      
   return (cull != NULL);
}

//-------------------------------------------------------------------------------------
// This is where distances are calculated.  Also used to verify that all edges have a 
// counterpart coming back.  Other final stuff has found its way here as well... 
S32 NavigationGraph::doFinalFixups()
{
   S32               numberHanging = 0, i;
   GraphEdge         edgeBuffOuter[MaxOnDemandEdges];
   GraphEdgeArray    edgeListOuter;

   sTotalEdgeCount = 0;
   
   for (i = 0; i < mNodeList.size(); i++)
   {
      if (GraphNode * node = mNodeList[i])
      {
         edgeListOuter = node->getEdges(edgeBuffOuter);
         sTotalEdgeCount += edgeListOuter.numEdges();
         while (GraphEdge * edgePtrOuter = edgeListOuter++)
         {
            GraphNode * neighbor = mNodeList[edgePtrOuter->mDest];
            AssertFatal(neighbor, "graph has bad neighbor indices");

            // Main edge setup method- 
            mJetManager.initEdge(*edgePtrOuter, node, neighbor);

            #if DebugCheckHanging
            
            if (edgePtrOuter->mDest == i)
               Con::errorf("Node (%d) has edge pointing to self!", i);
            
            // This debugging check only needed when we're modifying generation tools.
            // Otherwise it's slow, and the generation is pretty solid anyway.  
            static GraphEdge sEdgeBuffInner[MaxOnDemandEdges];
            bool  found = false;
            GraphEdgeArray  edgeListInner = neighbor->getEdges(sEdgeBuffInner);
            while (GraphEdge * edgePtrInner = edgeListInner++)
               if (edgePtrInner->mDest == i){
                  found = true;
                  break;
               }
            if (!found)
            {
               numberHanging++;
               Point3F  P = node->location();
               Point3F  N = neighbor->location();
               S32      D = edgePtrOuter->mDest;
               Con::errorf("Hanging edge from %d (%f, %f, %f) to %d (%f, %f, %f)", 
                                              i, P.x, P.y, P.z,  D, N.x, N.y, N.z);
            }
            #endif
         }
      }
   }

   // Keep separate list of the non-transients. 
   mTransientStart = mNodeList.size();
   mNonTransient.reserve(mNodeList.size() + mMaxTransients);
   mNonTransient = mNodeList;
   for (i = 0; i < mMaxTransients; i++ )
      mNodeList.push_back(NULL);
      
   // Trim duplicate edges.  This is rare (interior generator can get a strange
   // boundary with parallel edges), and causes problems.  Not the best fix for 
   // this problem (see graphTransient.cc attempt), but reasonable.  
   BitVector   marker(mNodeList.size());
   S32         numDups = 0;
   marker.clear();
   for (i = 0; i < mNodeList.size(); i++) 
      if (GraphNode * node = mNodeList[i])
         while (cullOneEdgeDup(node->mEdges, marker))
            numDups++;
   if (numDups)
      Con::printf("%d duplicate edges found on nodes.", numDups);

   // Good idea to mark islands AFTER checking for hanging.
   markIslands();
   
   // See navGraph.cc- 
   newIncarnation();
   
   mValidLOSTable = (mLOSTable && mLOSTable->valid(numNodes()));
   
   mValidPathTable = false;      // this has been dropped...  
   
   return numberHanging; 
}

//-------------------------------------------------------------------------------------
// 
// Assemble the graph from data that has been stored on it, computed, etc.  
// 

bool NavigationGraph::makeGraph(bool needEdgePool)
{
   mPushedBridges = 0;
   
   makeRunTimeNodes(needEdgePool);     // (graphOutdoors.cc)
   Con::printf("MakeGraph: %d indoor, %d outdoor", mNumIndoor, mNumOutdoor);

   mIndoorPtrs.clear();    // convenience list
   mIndoorTree.clear();    // BSP for it
   
   if (mNumIndoor) 
   {
      mIndoorPtrs.reserve(mNumIndoor);
      for (S32 i = 0; i < mNumIndoor; i++)
         mIndoorPtrs.push_back(mNodeList[i + mNumOutdoor]);
         
      // Set up tree for quickly finding indoor nodes- 
      mIndoorTree.makeTree(mIndoorPtrs);
   }

   return true;
}

//-------------------------------------------------------------------------------------
// For now this just makes the roaming distance table.  
//    We compute these data separately since they take a while.  

S32 NavigationGraph::makeTables()
{
   // Get a table of ordered offsets.  
   mTerrainInfo.computeRoamRadii();
   return 0;
}

//-------------------------------------------------------------------------------------
// Initialize the terrain info data from the ground plan.  

bool NavigationGraph::setGround(GroundPlan * gp)
{
   mTerrainInfo.haveGraph = true;
   if (gp->setTerrainGraphInfo(&mTerrainInfo)) 
   {
      mTerrainInfo.doFinalDataSetup();
      mTerrainInfo.consolidateData(mConjoin);
   }
   else
   {
      mTerrainInfo.haveGraph = false;
   }
   return mTerrainInfo.haveGraph;
}

//-------------------------------------------------------------------------------------

#define     LOOK_AT_SPECIFIC_NODE   0

#if LOOK_AT_SPECIFIC_NODE
Point3F  gSpecificNodeLoc(-149, -131.828, 163);
F32      gSpecificNodeRad = 9.0;
S32 findSpecificNode(const NodeInfoList& nodes)
{
   S32   found = -1;
   for (S32 i = 0; i < nodes.size(); i++)
   {
      Point3F  point = nodes[i].pos;
      if ((point - gSpecificNodeLoc).len() < gSpecificNodeRad)
         Con::printf("Node %d is where it's at", found = i);
   }
   return found;
}
#endif

//-------------------------------------------------------------------------------------
// Get / Set interior node data.  This doesn't affect the graph (building 
// the graph from indoor and outdoor databases is a step that must be 
// called explicitly).  

S32 NavigationGraph::getNodesAndEdges(EdgeInfoList& edges, NodeInfoList& nodes)
{
   edges = mEdgeInfoList;
   nodes = mNodeInfoList;
   return nodes.size();
}

// Set the system up with the given list of edges and nodes.  Resets / hooks up 
// nodes indoors.  
S32 NavigationGraph::setNodesAndEdges(const EdgeInfoList& edges, const NodeInfoList& nodes)
{
   mEdgeInfoList = edges;
   mNodeInfoList = nodes;
   return nodes.size();
}

//-------------------------------------------------------------------------------------

S32 NavigationGraph::setEdgesAndNodes(const EdgeInfoList& edges, const NodeInfoList& nodes)
{
   return setAlgorithmic(edges, nodes);
}

//-------------------------------------------------------------------------------------

// Interior node generator calls this to add data to graph, plus these lists
// then become hand-editable.  We always clean out existing algorithmic first.  
S32 NavigationGraph::setAlgorithmic(const EdgeInfoList& edges, const NodeInfoList& nodes)
{
   #if LOOK_AT_SPECIFIC_NODE
      findSpecificNode(nodes);
   #endif
   clearAlgorithmic();
   // Add the edges with remapped dest index, and marked as algorithmic.  
   S32   indexAdd = mNodeInfoList.size();
   for (EdgeInfoList::const_iterator e = edges.begin(); e != edges.end(); e++) {
      GraphEdgeInfo  edge = * e;
      edge.setAlgorithmic();
      edge.to[0].dest += indexAdd;
      edge.to[1].dest += indexAdd;
      mEdgeInfoList.push_back(edge);
   }
   for (NodeInfoList::const_iterator n = nodes.begin(); n != nodes.end(); n++) {
      IndoorNodeInfo  node = * n;
      node.setAlgorithmic();
      mNodeInfoList.push_back(node);
   }
   return 0;
}

// Clear algorithmic nodes out of main list, plus edges that connect to any.  
S32 NavigationGraph::clearAlgorithmic()
{
   Vector<S32>    mapIndices;
   NodeInfoList   keepNodes;
   EdgeInfoList   keepEdges;
   
   setSizeAndClear(mapIndices, mNodeInfoList.size(), 0xff);

   // get which nodes we're keeping and set up remap
   for (S32 i = 0; i < mNodeInfoList.size(); i++)
      if (mNodeInfoList[i].isAlgorithmic() == false) {
         mapIndices[i] = keepNodes.size();
         keepNodes.push_back(mNodeInfoList[i]);
      }
   
   // get the edges we're keeping and remap their destination indices
   EdgeInfoList::iterator edge;
   for (edge = mEdgeInfoList.begin(); edge != mEdgeInfoList.end(); edge++)
      if (edge->isAlgorithmic() == false)
         if (mNodeInfoList[edge->to[0].dest].isAlgorithmic() == false)
            if (mNodeInfoList[edge->to[1].dest].isAlgorithmic() == false) {
               for (S32 j = 0; j < 2; j++)
                  edge->to[j].dest = mapIndices[edge->to[j].dest];
               keepEdges.push_back(*edge);
            }
   
   // copy back the culled lists
   mNodeInfoList = keepNodes;
   mEdgeInfoList = keepEdges;
   
   return mNodeInfoList.size();
}

//-------------------------------------------------------------------------------------

S32 NavigationGraph::setNodeVolumes(const GraphVolumeList& volumes)
{
   mNodeVolumes = volumes;
   mNodeVolumes.nudgeVolumesOut();
   return mNodeVolumes.size();
}

//-------------------------------------------------------------------------------------

S32 NavigationGraph::setChuteHints(const Vector<Point3F>& hints)
{
   S32   numHints = hints.size();
   mChutes.init(hints);
   return numHints;
}

//-------------------------------------------------------------------------------------

static const U32 sMask = InteriorObjectType|StaticShapeObjectType|TerrainObjectType;

// The path logic as it pertains to jetting connections (often) needs to know for sure
// that the player can jump (makes a big difference in achievable height).  This method
// must figure this out.  It's Ok if we miss a few where it's possible to jump, but we 
// don't want to err the other way (SOMETIMES can't jump, but think we can).  
S32 NavigationGraph::findJumpableNodes()
{
   S32   total = 0;

   if (!mIsSpawnGraph) 
   {
      F32         check45 = mSqrt(0.5);
      RayInfo     coll;
      S32         i, j;
      Point3F     down(0, 0, -6);
   
      // ==> As an optimization, we should only do this with nodes that you jet out of.
      
      // Um, because of where this is called, we can't use numNodes()- 
      S32   numNodes = (mNumIndoor + mNumOutdoor);
      
      for (i = 0; i < numNodes; i++) {
         if (GraphNode * node = lookupNode(i)) {
            if (node->getNormal().z > check45) {
               // Cast several rays down and check their normals.  
               Point3F  loc = node->location();
               loc += Point3F(-0.8, -0.9, 2.0);
               for (j = 0; j < 4; j++) {
                  Point3F  corner((j & 1 != 0) * 1.6, (j & 2 != 0) * 1.7, 0.0);
                  corner += loc;
                  if (gServerContainer.castRay(corner, corner + down, sMask, &coll))
                     if (coll.normal.z < check45)
                        break;
               }
               if (j == 4) {
                  node->set(GraphNode::Flat);
                  total++;
               }
            }
         }
      }
   }
   
   return total;
}

