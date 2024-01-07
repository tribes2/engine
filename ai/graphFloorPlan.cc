//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/graphFloorPlan.h"
#include "console/consoleTypes.h"
#include "interior/interiorResObjects.h"
#include "game/gameBase.h"
#include "ai/graphData.h"
#include "core/fileStream.h"


//----------------------------------------------------------------
//
// FloorPlan Implementation
//
//----------------------------------------------------------------

FloorPlan *gFloorPlan = NULL;

IMPLEMENT_CONOBJECT(FloorPlan);
static F32 sCollinearDist = 0.01;
static F32 sParallelDot = 0.992;
Point3F FloorPlan::sDebugBreakPoint(-11.31f, -132.0f, 131.4f);
F32  FloorPlan::sFloorThresh = 70;
F32  FloorPlan::sMinDivRadius = 0.8;
F32  FloorPlan::sMaxDivRadius = 20;
bool FloorPlan::sSubDivide = true;
bool FloorPlan::sDrawConnections = true;
bool FloorPlan::sDrawVolumeHts = false;
F32  FloorPlan::sHeightDiff = 10;
F32  FloorPlan::sSpecialMaxRadius = 1;
bool FloorPlan::sUseSpecial = false;
S32  FloorPlan::amountToDivide = 3;
bool FloorPlan::sDisableAsserts = false;
bool FloorPlan::LogMode = false; 

FloorPlan::FloorPlan()
{
   mGraphDetails = NULL;
   currInterior = NULL;
   currInstance = NULL;
   mNumInteriors = 0;
   mExtraInteriors = 0;
   mDataCollected = false;
   newLog = true;
   numZones = 0;
}

//-------------------------------------------------------------------------------------

#define  _FP_DEBUG_  1

#if _FP_DEBUG_
static void debugStopPt() 
{
}
#endif

//----------------------------------------------------------------------------

FloorPlan::~FloorPlan()
{
   if(mGraphDetails)
      delete [] mGraphDetails;
}

//----------------------------------------------------------------------------

bool FloorPlan::onAdd()
{
   if(!Parent::onAdd())
      return false;

   gFloorPlan = this;
   return true;   
}

//----------------------------------------------------------------------------

void FloorPlan::onRemove()
{
   Parent::onRemove();
   gFloorPlan = NULL;
}

//----------------------------------------------------------------------------

static void findObjectsCallback(SceneObject *obj, S32 val)
{  
   Vector<SceneObject*> *list = (Vector<SceneObject *> *)val;
   list->push_back(obj);
}

//----------------------------------------------------------------------------
// Sift though all the objects that are in the Server container.
// Extract the data if its an interior.
//----------------------------------------------------------------------------
void FloorPlan::snoopInteriors()
{
   U32 mask = InteriorObjectType;
   Vector<SceneObject *> objects;
   gServerContainer.findObjects(mask, findObjectsCallback, (S32)&objects);
   
   mNumInteriors = objects.size();
   mGraphDetails = new GraphDetails[mNumInteriors+1024];
   mSlopeThresh = mCos(mDegToRad(sFloorThresh));
   
   for(S32 i = 0; i < mNumInteriors; i++)
   {
      InteriorInstance *intInstant = dynamic_cast<InteriorInstance*>(objects[i]);
      MatrixF const &transform = intInstant->getTransform();
      Point3F const &scale = intInstant->getScale();
      Interior *interior = intInstant->getDetailLevel(0);
      currInterior = interior;
      currInstance = intInstant;
      numZones = interior->mZones.size();
      
#if !DEBUGMODE
      
      InteriorDetail detail;
      
#endif      
      
      detail.mIndex = i;
      
      /*
      char buffer[256];
      dSprintf(buffer, sizeof(buffer), "interiors/%s", intInstant->mInteriorFileName);
      Resource<InteriorResource> interiorRes;
      interiorRes = ResourceManager->load(buffer);
      */
      
      // Special nodes (basically just the chute hints)
      for(S32 g = 0; g < intInstant->mInteriorRes->getNumSpecialNodes(); g++)
      {
         AISpecialNode * node = intInstant->mInteriorRes->getSpecialNode(g);
      
         // Scale and transform to world space
         Point3F  loc = node->mPos;
         loc.x *= scale.x;
         loc.y *= scale.y;
         loc.z *= scale.z;
         transform.mulP(loc);
         mGraphDetails[i].chutes.push_back(loc);
         
         // DSpecNode sNode;
         // sNode.pos = node->mPos;
         // dStricmp(node->mName, sNode.name);         
         // sNode.pos.x *= scale.x;
         // sNode.pos.y *= scale.y;
         // sNode.pos.z *= scale.z;
         // transform.mulP(sNode.pos);
         // detail.mSpecialNodes.push_back(sNode);
      }
      
      detail.mPoints.setSize(interior->mPoints.size());
      for(S32 j = 0; j < detail.mPoints.size(); j++)
      {   
         detail.mPoints[j].e = NULL;
         detail.mPoints[j].x = interior->mPoints[j].point.x;
         detail.mPoints[j].y = interior->mPoints[j].point.y;
         detail.mPoints[j].z = interior->mPoints[j].point.z;
         detail.mPoints[j].x *= scale.x;
         detail.mPoints[j].y *= scale.y;
         detail.mPoints[j].z *= scale.z;
         transform.mulP(detail.mPoints[j]);
         detail.mPoints[j].flags.clear();
      }
      
      detail.mNumUnObstructed = 0;
      
      // extract all plane information
      extractData(interior, &detail, transform);
      
         
#if ITERATEMODE      
      
      for(S32 divideCount = 0; divideCount < amountToDivide; divideCount++)
      {   
         Con::printf("");
         Con::printf("");
         Con::printf("");
         Con::printf("interation: %d", divideCount);
         subdivideSurfaces(&detail);
      }
#else 
     
      // subdivide until we have a workable graph for this interior
      while(subdivideSurfaces(&detail) > 0)
         mDivLevel++;
#endif      
      // sort surface list based on zone
      sortSurfaceList(&detail);
      
      // connect all volumes
      buildConnections(&detail);
      
      // extract the graph
      graphExtraction(&detail, &(mGraphDetails[i]));
      
      buildNodesFromPortals(&detail);
      
      log(avar("Interior %d Volumes: %d", i, detail.mVolumes.size()));

#if !DEBUGMODE
      
      // clean up memory... YAY!!
      detail.mEdgeTable.clear();
      for(S32 x = 0; x < detail.mSurfaces.size(); x++)
         delete [] detail.mSurfaces[x].mEdges;

#endif   
   
   }
   
#if !DEBUGMODE   
   
   // external static shape hook
   buildStaticShapeCenterNodes();
   buildStaticShapeGeometryNodes();

#endif
}

//----------------------------------------------------------------------------

void FloorPlan::log(const char *string)
{
   if(!LogMode)
      return;

   FileStream LogFile;
   
   LogFile.open("GenMetrics.log", FileStream::ReadWrite);

   if(LogFile.getStatus() == Stream::Ok)
   {
      LogFile.setPosition(LogFile.getStreamSize());
      
      LogFile.write(dStrlen(string), string);
      LogFile.write(2, "\r\n");
   }
   
   LogFile.close();
}

//----------------------------------------------------------------------------
// The whole idea here is too extract all data from the passed interior.
// Once all data has been extracted, surfaces that won't be needed
// ex: surfaces that a bot can't traverse. are thrown out.
// What we should end up with are floor surfaces, which will then be 
// flooded with nodes(subdivision) and finally connected together to 
// form the graph for this interior
//----------------------------------------------------------------------------
void FloorPlan::extractData(Interior         *interior, 
                            InteriorDetail   *detail, 
                            const MatrixF    &transform)
{
   detail->mSurfaces.setSize(interior->mSurfaces.size());
   Interior::Surface surf;
   
   // extract out the portals we need from this interior
   extractPortals(interior, detail, transform);

   U32 i;
   for(i = 0; i < detail->mSurfaces.size(); i++)
   {
      // grab a surface....
      surf = interior->mSurfaces[i];
      DSurf *currSurf = &(detail->mSurfaces[i]);
      
      currSurf->mFlags.clear();
      currSurf->mFlipStates.clear();
      
      S32 vertCount = surf.windingCount;
      currSurf->mEdges = new DEdge *[vertCount];
      currSurf->mDivLevel = 0;
      currSurf->parent = NULL;
      currSurf->mSubSurfCount = 0;
      
      // go ahead and extract
      interior->collisionFanFromSurface(surf, currSurf->fullWinding, &(currSurf->mNumEdges)); 
      
      // cull out all unwanted surfaces. Ceilings...ect
      PlaneF plane = interior->getPlane(surf.planeIndex);
      if(Interior::planeIsFlipped(surf.planeIndex))
         plane.neg();

      VectorF normal = plane;
      transform.mulV(normal);
      currSurf->mNormal = normal;
      if(normal.z <= 0)
      {   
         currSurf->mFlags.set(DSurf::isWall);
         currSurf->mFlags.set(DSurf::shouldBeCulled);
      }
      
      // what kind of slope does this surface have?
      if(!currSurf->mFlags.test(DSurf::shouldBeCulled))
      {
         F32 dot = mDot(normal, VectorF(0, 0, 1));
         if(dot < mSlopeThresh)
            currSurf->mFlags.set(DSurf::shouldBeCulled);
      }
      
      // keep a handle to this surface for quick traversable 
      // surface only operations
      if(!currSurf->mFlags.test(DSurf::shouldBeCulled))
         detail->mTraversable.push_back(i);
   }
   
   // build all edges for this interior
   buildEdgeTable(detail);
   buildZoneInfo(detail);
}

//--------------------------------------------------------------------------

void FloorPlan::sortSurfaceList(InteriorDetail *d)
{
   d->sortedSurfList.clear();
   d->sortedSurfList = d->mUnobstructed;
   d->mUnobstructed.clear();
   
   for(S32 zone = -1; zone < numZones; zone++)
   {
      for(S32 surfCount = 0; surfCount < d->sortedSurfList.size(); surfCount++)
      {
         U32 key = d->sortedSurfList[surfCount];
         DSurf &surf = d->mSurfaces[key];
         if(surf.mZone == zone)
            d->mUnobstructed.push_back(key);
      }    
   }
}

//--------------------------------------------------------------------------

void FloorPlan::buildZoneInfo(InteriorDetail *d)
{
   S32 surfCount = d->mTraversable.size();
   DPoint center;
   MatrixF transform = currInstance->mWorldToObj;
   Point3F scale = currInstance->getScale();
   
   for(S32 j = 0; j < surfCount; j++)
   {   
      S32 whichSurface = d->mTraversable[j];
      DSurf &surf = d->mSurfaces[whichSurface];
      createCenterNode(d, &surf, &center);
      center.z += 1;
      transform.mulP(center);
      center.x /= scale.x;
      center.y /= scale.y;
      center.z /= scale.z;
      
      surf.mZone = currInterior->getZoneForPoint(center);
   }
}

//--------------------------------------------------------------------------

static const F32 sPortalThresh = mCos(mDegToRad(20.0f));

void FloorPlan::extractPortals(Interior *i, InteriorDetail *d, const MatrixF &transform)
{
   S32 count = i->mPortals.size();
   for(S32 pIdx = 0; pIdx < count; pIdx++)
   {
      Interior::Portal *sourcePortal = &(i->mPortals[pIdx]);
      U32 index = sourcePortal->planeIndex;
      PlaneF plane = i->getPlane(index);
      if(Interior::planeIsFlipped(index))
         plane.neg();
           
      VectorF normal = plane;
      transform.mulV(normal);
      
      if(mFabs(normal.z) > sPortalThresh)
      {   
         DPortal portal;
         if(buildPortalCenter(d, i, sourcePortal, &portal))
            d->mPortals.push_back(portal);
      }
      else
         continue;
   }
}

//--------------------------------------------------------------------------

bool FloorPlan::buildPortalCenter(InteriorDetail *d, Interior *interior, Interior::Portal *sourcePortal, DPortal *portal)
{
   // extract the trifans and find center
   Point3F center(0, 0, 0);
   //Point3F start(interior->mPoints[interior->mWindings[interior->mWindingIndices[sourcePortal->triFanStart]]].point);
   //transform.mulP(start);
   
   for(U32 j = 0; j < sourcePortal->triFanCount; j++)
   {
      const Interior::TriFan &fan = interior->mWindingIndices[sourcePortal->triFanStart + j];
      U32 numPoints = fan.windingCount;
      
      if(!numPoints)
         continue;
            
      for(U32 k = 0; k < numPoints; k++)
      {
         const Point3F &pt = d->mPoints[interior->mWindings[fan.windingStart + k]];
         center += pt;
      }

      center /= numPoints;
      portal->center = center;
   }
   
   // only use portals that are big enough for players to fit thru
   if(1)//(center - start).len() >= 1.0)
      return true;
   else     
      return false;
}

//--------------------------------------------------------------------------
   
void FloorPlan::buildEdgeTable(InteriorDetail *detail) 
{   
   U32 i, k;
   for(i = 0; i < detail->mSurfaces.size(); i++)
   {   
      DSurf *currSurf = &(detail->mSurfaces[i]);
      
      for(k = 0; k < currSurf->mNumEdges; k++)
      {
         DEdge edge;
         edge.left = NULL;
         edge.right = NULL;
         edge.start = currSurf->fullWinding[k];
         edge.flags.clear();
         // edge.totalBelongsTo = 0;
         if(!currSurf->mFlags.test(DSurf::shouldBeCulled))
            edge.flags.set(DEdge::traversable);
      
         if(k == currSurf->mNumEdges - 1)
            edge.end = currSurf->fullWinding[0];
         else
            edge.end = currSurf->fullWinding[k+1];

         if(!detail->mEdgeTable.contains(edge))
         {   
            edge.length = (detail->mPoints[edge.end] - detail->mPoints[edge.start]).len();
            detail->mEdgeTable.insert(edge);
            currSurf->mEdges[k] = detail->mEdgeTable.find(edge); 
            detail->mPoints[edge.start].e  = currSurf->mEdges[k];        
         }
         else
         {   
            currSurf->mEdges[k] = detail->mEdgeTable.find(edge);   
            if(!detail->mPoints[edge.start].e)
               detail->mPoints[edge.start].e = currSurf->mEdges[k];
         
            if(edge.start != currSurf->mEdges[k]->start)
               currSurf->mFlipStates.set(BIT(k));
         }
      }
      
      // done! compute some more.
      findLongestEdge(currSurf);
      computeSurfaceRadius(detail, currSurf);
      
      if(currSurf->mFlags.test(DSurf::isWall))
         setToWallNodes(detail, *currSurf);
   }
}

//-------------------------------------------------------------------------

// Handle thin nodes differently.  
bool FloorPlan::shouldDivideSurf(DSurf &surf)
{
   if (surf.mMinRadius < 0.4) {
      if (surf.mMinRadius > 0.2)
      {   
         bool canDiv = surf.mMaxRadius >= 2.5;
         if(canDiv)
            return true;
         else
         {
            surf.mFlags.set(DSurf::tooSmall);
            return false;
         }
      }
      else
      {   
         surf.mFlags.set(DSurf::tooSmall);
         return false;
      }   
   }
   else
      return surf.mMaxRadius >= sMinDivRadius;
}

//--------------------------------------------------------------------------

void FloorPlan::setToWallNodes(InteriorDetail *d, DSurf &surf)
{
   U32 i;
   for(i = 0; i < surf.mNumEdges; i++)
   {
      d->mPoints[surf.mEdges[i]->start].flags.set(DPoint::wall);
      d->mPoints[surf.mEdges[i]->end].flags.set(DPoint::wall); 
   }   
}

//--------------------------------------------------------------------------

U32 FloorPlan::subdivideSurfaces(InteriorDetail *d)
{
   U32 totalDivided  = 0;

   // Traversable array grows during below loop - we only want to do the ones that
   // are ready at this iteration of the divide.     
   S32 surfCount = d->mTraversable.size();
   
   for(S32 j = 0; j < surfCount; j++)
   {   
      S32   whichSurface = d->mTraversable[j];

      DSurf &surf = d->mSurfaces[whichSurface];
      
      if(surf.mNumEdges >= 32)
         continue;
      
      if (shouldDivideSurf(surf))
      {
         if(!surf.mFlags.test(DSurf::unObstructed | DSurf::divided))
         {
            // check for obstructions
            DVolume           vol;
            surfCollisionInfo surfColInfo;
            
            if((obstructedSurf(d, surf, &vol, surfColInfo)) || (surf.mMaxRadius > sMaxDivRadius))
            {   
               // something is obstructing this surf, so divided it down
               // so as to isolate the surfaces that are obstructed
               if(!surfColInfo.specialCase)
               {   
                  //Con::printf("subdividing surface %d", whichSurface);
                  subdivideSurface(d, surf, whichSurface);
               }
               else
               {   
                  //Con::printf("split surface %d", whichSurface);
                  //if(whichSurface == 26)
                     //subdivideSurface(d, surf, whichSurface);
                  //else
                     splitSurface(d, surf, whichSurface, surfColInfo);
               }
               totalDivided++;
            }
            /* even though this surface is traversable we need 
               to check if there are some little holes in the ceiling
               that players can jet out of. If there are, just divide more.
            
            else if(offHeightDistribution(d, surf))
            {
               subdivideSurface(d, surf, whichSurface);
               totalDivided++;
            }   
            
            this surface is unobstructed!*/
            else
            {
               surf.mFlags.set(DSurf::unObstructed);
               createCenterNode(d, &surf, &(surf.mCntr));
               vol.capPt.x = surf.mCntr.x;
               vol.capPt.y = surf.mCntr.y;
               vol.capPt.z = (surf.mCntr.z + vol.ht);
               vol.surfIdx = whichSurface;
               surf.volIdx = d->mVolumes.size();
               d->mVolumes.push_back(vol);
               d->mUnobstructed.push_back(whichSurface);
            }
         }
      }
   }
   return totalDivided;
}

//----------------------------------------------------------------------------

void FloorPlan::subdivideSurface(InteriorDetail *d, DSurf &surf, U32 surfIdx)
{
   BitSet32 edgeTracker;
   U32 newSurfCount = 0;
   DEdge *edgePtr;
   
   U32 i;
   for(i = 0; i < surf.mNumEdges; i++)
   {
      edgePtr = surf.mEdges[i];
      if(shouldDivideEdge(surf, edgePtr))
      {   
         edgeTracker.set(BIT(i));
         newSurfCount++;
         if(!edgePtr->flags.test(DEdge::divided))
            subdivideEdge(d, edgePtr, 0.5f);
      }
   }
   
   // we have a special case here
   if(newSurfCount < 2)
   {
      edgeTracker.clear();
      newSurfCount = 0;
      for(i = 0; i < surf.mNumEdges; i++)
      {
         edgePtr = surf.mEdges[i];
         if(!shouldDivideEdge(surf, edgePtr) && edgePtr->length >= 2)
         {
            edgeTracker.set(BIT(i));
            subdivideEdge(d, edgePtr, 0.5f);
            newSurfCount++;
         }   
      }
      if(newSurfCount < 2)
      {   
         surf.mFlags.set(DSurf::divided);
         return;
      }      
   }
   
   // create all the new surfaces
   surf.mSubSurfCount = newSurfCount;
   DEdge **edges = new DEdge *[32];
   
   if(newSurfCount > 2)
   {
      DPoint center;
      createCenterNode(d, &surf, &center);
      center.flags.clear();
      center.flags.set(DPoint::divGenerated | DPoint::center);
      d->mPoints.push_back(center);
      U32 centIdx = (d->mPoints.size() - 1);
      generateNewEdges(d, centIdx, surf, edges, edgeTracker);
      createNewSurfaces(d, surfIdx, edges, edgeTracker);
   }
   else
   {   
      newSurfacesSpecialCase(d, surfIdx, edgeTracker);
   }
   delete [] edges;   
}

//----------------------------------------------------------------------------

void FloorPlan::splitSurface(InteriorDetail *d, DSurf &surf, U32 surfIdx, surfCollisionInfo &info)
{
   BitSet32 edgeTracker;
   surf.mSubSurfCount = 2;
   F32 newFactor = info.factor - (0.1 / surf.mEdges[info.divideEdge1]->length);
   
   edgeTracker.set(BIT(info.divideEdge1));
   edgeTracker.set(BIT(info.divideEdge2));
   
   if(!surf.mFlipStates.test(BIT(info.divideEdge1)))
      subdivideEdge(d, surf.mEdges[info.divideEdge1], newFactor);
   else
      subdivideEdge(d, surf.mEdges[info.divideEdge1], 1 - newFactor);
      
   if(!surf.mFlipStates.test(BIT(info.divideEdge2)))
      subdivideEdge(d, surf.mEdges[info.divideEdge2], 1 - newFactor);
   else
      subdivideEdge(d, surf.mEdges[info.divideEdge2], newFactor);

   newSurfacesSpecialCase(d, surfIdx, edgeTracker);
}

//----------------------------------------------------------------------------

void FloorPlan::newSurfacesSpecialCase(InteriorDetail *d, U32 surfIdx, BitSet32 &edgeTracker)
{
   U32 edgeIdx[2];
   U32 count = 0;
   
   d->mSurfaces[surfIdx].mFlags.set(DSurf::split);
   d->mSurfaces[surfIdx].mFlags.set(DSurf::divided);
   
   U32 i;
   for(i = 0; i < d->mSurfaces[surfIdx].mNumEdges; i++)
      if(edgeTracker.test(BIT(i)))
         edgeIdx[count++] = i;
         
   DEdge *edge1 = d->mSurfaces[surfIdx].mEdges[edgeIdx[0]];
   DEdge *edge2 = d->mSurfaces[surfIdx].mEdges[edgeIdx[1]];
   
   DEdge midEdge;
   midEdge.start = edge1->midPoint;
   midEdge.end = edge2->midPoint;
   midEdge.flags.set(DEdge::divGenerated);
   midEdge.length = (d->mPoints[midEdge.end] - d->mPoints[midEdge.start]).len();
   d->mEdgeTable.insert(midEdge);
   DEdge *edge = d->mEdgeTable.find(midEdge);
   
   // start with a divided edge
   U32 edgeCount = 0; // keeps track of the master surf's edge idx
   while(!edgeTracker.test(BIT(edgeCount)))
      edgeCount++;

   for(i = 0; i < 2; i++)
   {
      DSurf newSurf;
      DSurf *currSurf = &newSurf;
      currSurf->mEdges = new DEdge *[32];
      currSurf->mDivLevel = 0;
      currSurf->mNormal = d->mSurfaces[surfIdx].mNormal;
      currSurf->mFlags.clear();
      currSurf->mFlipStates.clear();
      currSurf->parent = &(d->mSurfaces[surfIdx]);
      currSurf->mSubSurfCount = 0;
      currSurf->mZone = d->mSurfaces[surfIdx].mZone;
      
      U32 edgeIdx = 0;   // keeps track of new surf edge idx's
      
      // add the first edge to the new surf's edge list
      if(!d->mSurfaces[surfIdx].mFlipStates.test(BIT(edgeCount)))
      {   
         currSurf->mEdges[edgeIdx++] = d->mSurfaces[surfIdx].mEdges[edgeCount]->right;
      }
      else
      {   
         currSurf->mEdges[edgeIdx++] = d->mSurfaces[surfIdx].mEdges[edgeCount]->left;
         currSurf->mFlipStates.set(BIT(edgeIdx - 1));
      }
      
      // wrap around
      if(edgeCount == (d->mSurfaces[surfIdx].mNumEdges - 1))
         edgeCount = 0;
      else
         edgeCount++;
      
      // keep adding edges if they are not divided
      while(!edgeTracker.test(BIT(edgeCount)))
      {   
         if(d->mSurfaces[surfIdx].mFlipStates.test(BIT(edgeCount)))
            currSurf->mFlipStates.set(BIT(edgeIdx)); 
         currSurf->mEdges[edgeIdx++] = d->mSurfaces[surfIdx].mEdges[edgeCount];
         
         // wrap around
         if(edgeCount == d->mSurfaces[surfIdx].mNumEdges - 1)
            edgeCount = 0;
         else
            edgeCount++;
      }
      
      // add the last generated edges to complete this surf
      if(!d->mSurfaces[surfIdx].mFlipStates.test(BIT(edgeCount)))
      {   
         currSurf->mEdges[edgeIdx++] = d->mSurfaces[surfIdx].mEdges[edgeCount]->left;
      }
      else
      {   
         currSurf->mEdges[edgeIdx] = d->mSurfaces[surfIdx].mEdges[edgeCount]->right;
         currSurf->mFlipStates.set(BIT(edgeIdx));
         edgeIdx++;
      }
      
      currSurf->mEdges[edgeIdx] = edge;
      edgeIdx++;
       
      if(!i)
         currSurf->mFlipStates.set(BIT(edgeIdx - 1));
               
      // update total edges for this surf
      currSurf->mNumEdges = edgeIdx;
      
      // this surf is created, update other members.
      findLongestEdge(currSurf);
      computeSurfaceRadius(d, currSurf);
      
      d->mSurfaces[surfIdx].mSubSurfs[i] = d->mSurfaces.size();
      d->mSurfaces.push_back(newSurf);
      d->mTraversable.push_back(d->mSurfaces.size() - 1);
   }
}

//----------------------------------------------------------------------------

void FloorPlan::createNewSurfaces(InteriorDetail *d, U32 surfIdx, DEdge **edges, BitSet32 &edgeTracker)
{
   // start with a divided edge
   U32 edgeCount = 0; // keeps track of the outer surf's edge idx
   while(!edgeTracker.test(BIT(edgeCount)))
      edgeCount++;

   U32 i;
   for(i = 0; i < d->mSurfaces[surfIdx].mSubSurfCount; i++)
   {
      DSurf newSurf;
      DSurf *currSurf = &newSurf;
      currSurf->mEdges = new DEdge *[32];
      currSurf->mDivLevel = 0;
      currSurf->mNormal = d->mSurfaces[surfIdx].mNormal;  // all new surfs are co-planar
      currSurf->mFlags.clear();
      currSurf->mFlipStates.clear();
      currSurf->parent = &(d->mSurfaces[surfIdx]);
      currSurf->mSubSurfCount = 0;
      currSurf->mZone = d->mSurfaces[surfIdx].mZone;
      
      U32 edgeIdx = 0;           // keeps track of new surf edge idx's
      U32 startEdge = edgeCount; // keeps track of which edge we started at
      
      // add the first edge to the new surf's edge list
      if(!d->mSurfaces[surfIdx].mFlipStates.test(BIT(edgeCount)))
      {   
         currSurf->mEdges[edgeIdx++] = d->mSurfaces[surfIdx].mEdges[edgeCount]->right;
      }
      else
      {   
         currSurf->mEdges[edgeIdx++] = d->mSurfaces[surfIdx].mEdges[edgeCount]->left;
         currSurf->mFlipStates.set(BIT(edgeIdx - 1));
      }
      
      // wrap around
      if(edgeCount == (d->mSurfaces[surfIdx].mNumEdges - 1))
         edgeCount = 0;
      else
         edgeCount++;
      
      // keep adding edges if they are not divided
      while(!edgeTracker.test(BIT(edgeCount)))
      {   
         if(d->mSurfaces[surfIdx].mFlipStates.test(BIT(edgeCount)))
            currSurf->mFlipStates.set(BIT(edgeIdx)); 
         currSurf->mEdges[edgeIdx++] = d->mSurfaces[surfIdx].mEdges[edgeCount];
         
         // wrap around
         if(edgeCount == d->mSurfaces[surfIdx].mNumEdges - 1)
            edgeCount = 0;
         else
            edgeCount++;
      }
      
      // add the last generated edges to complete this surf
      if(!d->mSurfaces[surfIdx].mFlipStates.test(BIT(edgeCount)))
      {   
         currSurf->mEdges[edgeIdx++] = d->mSurfaces[surfIdx].mEdges[edgeCount]->left;
      }
      else
      {   
         currSurf->mEdges[edgeIdx++] = d->mSurfaces[surfIdx].mEdges[edgeCount]->right;
         currSurf->mFlipStates.set(BIT(edgeIdx - 1));
      }
         
      currSurf->mEdges[edgeIdx++] = edges[edgeCount];
      currSurf->mEdges[edgeIdx++] = edges[startEdge]; // this will be flipped for sure
      currSurf->mFlipStates.set(BIT(edgeIdx - 1));
      currSurf->mNumEdges = edgeIdx;
      
      // this surf is created, update other members.
      findLongestEdge(currSurf);
      computeSurfaceRadius(d, currSurf);
      
      d->mSurfaces[surfIdx].mSubSurfs[i] = d->mSurfaces.size();
      d->mSurfaces.push_back(newSurf);
      d->mTraversable.push_back(d->mSurfaces.size() - 1);
   }
   
   d->mSurfaces[surfIdx].mFlags.set(DSurf::divided);
}

//----------------------------------------------------------------------------

void FloorPlan::findLongestEdge(DSurf *surf)
{
   F32   longest = 0;
   DEdge *longPtr;
   DEdge *edgePtr;
   
   U32 i;
   for(i = 0; i < surf->mNumEdges; i++)
   {
      edgePtr = surf->mEdges[i];
      if(edgePtr->length > longest)
      {   
         longPtr = edgePtr;
         longest = edgePtr->length;
      }
   }
   surf->mLongest = longPtr;
}

//----------------------------------------------------------------------------

void FloorPlan::subdivideEdge(InteriorDetail *d, DEdge *edge, F32 factor)
{
   DPoint &p1 = d->mPoints[edge->start];
   DPoint &p2 = d->mPoints[edge->end];

   // subdivide the edge
   DPoint p3;
   
   p3.x = p1.x + ((p2.x - p1.x) * factor);
   p3.y = p1.y + ((p2.y - p1.y) * factor);
   p3.z = p1.z + ((p2.z - p1.z) * factor);
   
   p3.flags.set(DPoint::divGenerated);
   p3.e = edge;
   
   // if both are wall nodes, so is this one
   if(p1.flags.test(DPoint::wall) && p2.flags.test(DPoint::wall))
      p3.flags.set(DPoint::wall);

   d->mPoints.push_back(p3);
   U32 idx = (d->mPoints.size() - 1);
   edge->midPoint = idx;
   DEdge e1, e2;
   e1.flags.clear();
   e2.flags.clear();
   e1.flags.set(DEdge::divGenerated | DEdge::traversable);
   e2.flags.set(DEdge::divGenerated | DEdge::traversable);

   // construct the new edges and add them to the edgeTable
   e1.start = edge->start;
   e1.end = idx;
   e2.start = idx;
   e2.end = edge->end;
    
   e1.right = e1.left = e1.next = NULL;
   
   F32 length = edge->length - (edge->length * factor); 
   e2.length = length;
   e2.right = e2.left = e2.next = NULL;
   e1.length = (edge->length * factor);
   d->mEdgeTable.insert(e1);
   d->mEdgeTable.insert(e2);
   edge->right = d->mEdgeTable.find(e2);
   edge->left = d->mEdgeTable.find(e1);
   edge->flags.set(DEdge::divided);
}

//----------------------------------------------------------------------------

// only call this after subdividing all edges of this surface and after
// creating its center node
void FloorPlan::generateNewEdges(InteriorDetail *d, U32 cenIdx, DSurf &surf, DEdge **edges, BitSet32 &edgeTracker)
{
   DEdge *edgePtr;
   U32 count = 0;
   
   U32 i;
   for(i = 0; i < surf.mNumEdges; i++)
   {
      edgePtr = surf.mEdges[i];
      if(edgeTracker.test(BIT(i)))
      {
         DEdge edge;
         edge.start = edgePtr->midPoint;
         edge.end = cenIdx;
         edge.length = (d->mPoints[edge.end] - d->mPoints[edge.start]).len();
         edge.flags.clear();
         edge.flags.set(DEdge::divGenerated | DEdge::traversable);
         edge.right = edge.left = edge.next = NULL;
         d->mEdgeTable.insert(edge);
         edges[i] = d->mEdgeTable.find(edge);
      }
      else
         edges[i] = NULL;
   }
}

//----------------------------------------------------------------------------
// use the center of mass of a poly as the center of this surf.
void FloorPlan::createCenterNode(InteriorDetail *d, DSurf *surf, DPoint *CM)
{
   Point2F* points = new Point2F[surf->mNumEdges];
   F32      heightSum = 0;
   
   for(S32 i = 0; i < surf->mNumEdges; i++)
   {
      S32   ind = (surf->mFlipStates.test(BIT(i)) ? surf->mEdges[i]->end : surf->mEdges[i]->start);
      points[i].x = d->mPoints[ind].x;
      points[i].y = d->mPoints[ind].y;
      heightSum +=  d->mPoints[ind].z;
   }
   
   Point2F c;
   bool ok = polygonCM(surf->mNumEdges, points, &c);
   
   CM->x = c.x;
   CM->y = c.y;
   CM->z = heightSum/surf->mNumEdges;
   CM->e = surf->mEdges[0];
   
   // trying to track a bug here.
   bool nodeOk = validatePoint(*CM);
   
   if(!nodeOk)
   {   
      CM = NULL;
      return;
   }
   
   delete [] points;
}

//----------------------------------------------------------------------------

void FloorPlan::graphExtraction(InteriorDetail *d, GraphDetails *g)
{
   EdgeInfoList      edges;
   NodeInfoList      nodes;
   EdgeInfoList      segs;
   GraphVolumeList   volumes;
   U32               indexCount = 0;
   BitVector         tracker;
   U32               *reMap;
   GraphEdgeInfo     *edgePtr;
   IndoorNodeInfo    *nodePtr;
   GraphVolInfo      *volPtr;
   U32               planeCount = 0;
   
   // Take each edge from each traversable surface and first remap
   // its verts into the new vertex array, and then finely add the 
   // edge to the edge list. I can't possibly see how a stray node 
   // that isn't part of any edge could be passed to the navGraph.
   U32 j, k;
   reMap = new U32[d->mConPoints.size()];
   tracker.setSize(d->mConPoints.size());
   tracker.clear();
   
   for(j = 0; j < d->mConnections.size(); j++)
   {
      DConnection &con = d->mConnections[j];
      U32 p1Idx = con.start;
      U32 p2Idx = con.end;
         
      // get each node index of this edge. remap it then add it
      // to the point list
      if(!tracker.test(p1Idx))
      {
         tracker.set(p1Idx);
         reMap[p1Idx] = indexCount;
         nodes.increment();
         nodePtr = &(nodes[indexCount++]);
         nodePtr->pos = Point3F(d->mConPoints[p1Idx].x, d->mConPoints[p1Idx].y, d->mConPoints[p1Idx].z);
         nodePtr->flags.clear();
         
         // add volume info
         DVolume *vol = &(d->mVolumes[d->mSurfaces[d->mConPoints[p1Idx].surfIdx].volIdx]);
         volumes.increment();
         volPtr = &(volumes[volumes.size() - 1]); 
         volPtr->mPlaneCount = vol->mNumPlanes;
         volPtr->mPlaneIndex = planeCount;
         for(k = 0; k < vol->mNumPlanes; k++)
         {   
            volumes.mPlanes.increment();
            volumes.mPlanes[planeCount++] = vol->mPlanes[k];   
         }
      }
      if(!tracker.test(p2Idx))
      {
         tracker.set(p2Idx);
         reMap[p2Idx] = indexCount;
         nodes.increment();
         nodePtr = &(nodes[indexCount++]);
         nodePtr->pos = Point3F(d->mConPoints[p2Idx].x, d->mConPoints[p2Idx].y, d->mConPoints[p2Idx].z);
         nodePtr->flags.clear();
         
         // add volume info
         DVolume *vol = &(d->mVolumes[d->mSurfaces[d->mConPoints[p2Idx].surfIdx].volIdx]);
         volumes.increment();
         volPtr = &(volumes[volumes.size() - 1]); 
         volPtr->mPlaneCount = vol->mNumPlanes;
         volPtr->mPlaneIndex = planeCount;
         for(k = 0; k < vol->mNumPlanes; k++)
         {   
            volumes.mPlanes.increment();
            volumes.mPlanes[planeCount++] = vol->mPlanes[k];   
         }
      }
      
      // now pack the edge into the edge list
      edges.increment();
      edgePtr = &(edges[edges.size() - 1]);
      edgePtr->to[0].dest = reMap[con.start];
      edgePtr->to[1].dest = reMap[con.end];
      
      // now pack the interface data
      GraphEdgeInfo  edgeSeg;
      edgeSeg.segPoints[0] = con.segNodes[0];
      edgeSeg.segPoints[1] = con.segNodes[1];
      edgeSeg.to[0].dest = reMap[con.start];
      edgeSeg.to[1].dest = reMap[con.end];
      segs.push_back(edgeSeg);
   }
   
   // find all islands and add them to the node list
   U32 idx, pIdx;
   for(idx = 0; idx < tracker.getSize(); idx++)
   {
      if (!tracker.test(idx))
      {
         tracker.set(idx);
         reMap[idx] = indexCount;
         nodes.increment();
         nodePtr = &(nodes[indexCount++]);
         bool invNode = d->mConPoints[idx].flags.test(DPoint::inventory);
         nodePtr->pos = Point3F(d->mConPoints[idx].x, d->mConPoints[idx].y, d->mConPoints[idx].z);
         nodePtr->flags.clear();
         
         if(invNode)
            nodePtr->setInventory();
         
         // volume info
         if(!invNode)
         {   
            DVolume &vol = d->mVolumes[d->mSurfaces[d->mConPoints[idx].surfIdx].volIdx];
            volumes.increment();
            volPtr = &(volumes[volumes.size() - 1]); 
            volPtr->mPlaneCount = vol.mNumPlanes;
            volPtr->mPlaneIndex = planeCount;
            for(pIdx = 0; pIdx < vol.mNumPlanes; pIdx++)
            {   
               volumes.mPlanes.increment();
               volumes.mPlanes[planeCount++] = vol.mPlanes[pIdx];   
            }
         }
         else
         {   
            DVolume vol;
            createInventoryVol(d->mConPoints[idx], vol);
            volumes.increment();
            volPtr = &(volumes[volumes.size() - 1]); 
            volPtr->mPlaneCount = vol.mNumPlanes;
            volPtr->mPlaneIndex = planeCount;
            for(pIdx = 0; pIdx < vol.mNumPlanes; pIdx++)
            {   
               volumes.mPlanes.increment();
               volumes.mPlanes[planeCount++] = vol.mPlanes[pIdx];   
            }
         }      
      }        
   }
   
   delete [] reMap; 
   g->volumes = volumes;
   g->edges = edges;
   g->nodes = nodes;
   g->segs = segs;
}

//----------------------------------------------------------------------------

void FloorPlan::upload2NavGraph(NavigationGraph *navGraph)
{
   AssertFatal(navGraph, "Upload2NavGraph(): Bad pointer to navGraph passed.");
   
   EdgeInfoList      edges;
   NodeInfoList      nodes;
   EdgeInfoList      segs;
   GraphVolumeList   volumes;
   Vector<PlaneF>    planes;
   Vector<Point3F>   chutes;
   
   for(S32 i = 0; i < (mNumInteriors + mExtraInteriors); i++)
   {
      // The graph data for this interior
      GraphDetails &g = mGraphDetails[i];
      
      // Offset the node indices on all the edges- 
      for (S32 D = 0; D < 2; D++) {
         for(S32 e = 0; e < g.edges.size(); e++)   g.edges[e].to[D].dest += nodes.size();
         for(S32 s = 0; s < g.segs.size(); s++)    g.segs[s].to[D].dest  += nodes.size();
      }
         
      // Offset plane indices on the volumes- 
      for(S32 p = 0; p < g.volumes.size(); p++)
         g.volumes[p].mPlaneIndex += planes.size();
         
      // Accumulate the master lists of edges, nodes, volumes, etc. 
      edges.merge(g.edges);
      segs.merge(g.segs);
      nodes.merge(g.nodes); 
      volumes.merge(g.volumes);
      planes.merge(g.volumes.mPlanes);
      chutes.merge(g.chutes);
   }
   
   // set the plane list to the volume plane list
   volumes.mPlanes = planes;
   
   // pass all this data back to navGraph for build
   navGraph->setAlgorithmic(edges, nodes);
   navGraph->setEdgesAndNodes(segs, nodes);
   navGraph->setNodeVolumes(volumes);
   navGraph->setChuteHints(chutes);
}

//----------------------------------------------------------------------------

#define defaultVolHt 3

void FloorPlan::createInventoryVol(DPoint p, DVolume &vol)
{
   vol.mNumPlanes = 6;
   vol.ht = defaultVolHt;
   vol.capPt.set(p.x, p.y, (p.z + defaultVolHt));
   
   Point3F origPt(p.x, p.y, p.z);
   Point3F pt;
   F32 interfaceWidth = 0.8f;
   
   // define the volume's planes
   pt.set(origPt.x - interfaceWidth, origPt.y, origPt.z);
   vol.mPlanes[0].set(pt, VectorF(-1, 0, 0));
   pt.set(origPt.x, origPt.y + interfaceWidth, origPt.z);
   vol.mPlanes[1].set(pt, VectorF(0, 1, 0));
   pt.set(origPt.x + interfaceWidth, origPt.y, origPt.z);
   vol.mPlanes[2].set(pt, VectorF(1, 0, 0));
   pt.set(origPt.x, origPt.y - interfaceWidth, origPt.z);
   vol.mPlanes[3].set(pt, VectorF(0, -1, 0));
   pt.set(origPt.x, origPt.y, origPt.z - 1);
   vol.mPlanes[4].set(pt, VectorF(0, 0, -1));
   pt.set(origPt.x, origPt.y, origPt.z + 2.0);
   vol.mPlanes[5].set(pt, VectorF(0, 0, 1));
}

//----------------------------------------------------------------------------

void FloorPlan::buildNodesFromPortals(InteriorDetail *d)
{
   for(S32 portalIdx = 0; portalIdx < d->mPortals.size(); portalIdx++)
   {
      bool usePnt = true;
      
      // get the point to create a node from
      Point3F pos;
      Point3F start = d->mPortals[portalIdx].center;
      Point3F end = start;
      end.z -= 1000;
      
      RayInfo info;
      if(gServerContainer.castRay(start, end, (InteriorObjectType | TerrainObjectType), &info))
      {   
         pos = info.point;
         pos.z += 0.1;
      }
      else
         continue;
         
      if (d->haveSurfaceNear(pos, 0.15))
         continue;
      
      for(S32 ptsCount = 0; ptsCount < portalPoints.size(); ptsCount++)
         if(pointsAreEqual(pos, portalPoints[ptsCount])) {
            usePnt = false;
            break;
         }
      
      if(usePnt)
      {
         portalPoints.push_back(pos);
         
         // create the node
         IndoorNodeInfo node;
         node.pos.set(pos.x, pos.y, pos.z);
         node.setBelowPortal();
   
         // create the volume
         DVolume invVol;
         GraphVolInfo vol;
         DPoint p;
         p.set(pos.x, pos.y, pos.z);
         createInventoryVol(p, invVol);
         vol.mPlaneIndex = 0;
         vol.mPlaneCount = invVol.mNumPlanes;
   
         // now make the simple graph object & add all the data   
         GraphDetails d;
         d.nodes.push_back(node);
         d.volumes.push_back(vol);
   
         // add the planes that will define this node
         for(U32 k = 0; k < 6; k++)
            d.volumes.mPlanes.push_back(invVol.mPlanes[k]);
   
         // no segs or edges needed, woohoo!
   
         // now push this onto the graphDetails list
         mGraphDetails[mNumInteriors + mExtraInteriors++] = d;
      }
   }
}

//----------------------------------------------------------------------------

bool FloorPlan::pointsAreEqual(Point3F &a, Point3F &b)
{
   if((b - a).len() > 2)
      return false;
   else
      return true;
}

//----------------------------------------------------------------------------

void FloorPlan::buildStaticShapeCenterNodes()
{
   Con::executef(this, 1, "staticShapeListConstruct");
   
   Vector<ShapeBase *>::iterator i;
   for(i = mStaticShapeCenterList.begin(); i != mStaticShapeCenterList.end(); i++)
      processShape(i, true);   
}

//----------------------------------------------------------------------------

void FloorPlan::buildStaticShapeGeometryNodes()
{
   Con::executef(this, 1, "staticShapeListConstruct");
   
   Vector<ShapeBase *>::iterator i;
   for(i = mStaticShapeGeometryList.begin(); i != mStaticShapeGeometryList.end(); i++)
      processShape(i, false);   
}

//----------------------------------------------------------------------------

void FloorPlan::processShape(ShapeBase **s, bool center)
{
   // for now this only includes inventory shapes
   if(center)
   {
      // get the point to create a node from
      Point3F pos;
      (*s)->getTransform().getColumn(3, &pos);
      pos.z++; // raise it up a bit
      
      // create the node
      IndoorNodeInfo node;
      node.pos.set(pos.x, pos.y, pos.z);
      node.flags.clear();
      node.setInventory();      
      
      // create the volume
      DVolume invVol;
      GraphVolInfo vol;
      DPoint p;
      p.set(pos.x, pos.y, pos.z);
      createInventoryVol(p, invVol);
      vol.mPlaneIndex = 0;
      vol.mPlaneCount = invVol.mNumPlanes;
      
      // now make the simple graph object & add all the data   
      GraphDetails d;
      d.nodes.push_back(node);
      d.volumes.push_back(vol);
      
      // add the planes that will define this node
      U32 k;
      for(k = 0; k < 6; k++)
         d.volumes.mPlanes.push_back(invVol.mPlanes[k]);
      
      // no segs or edges needed, woohoo!
      
      // now push this onto the graphDetails list
      mGraphDetails[mNumInteriors + mExtraInteriors++] = d;
   }
   
   // this graph will be based off of this shape's geometry 
   else
   {
      U32 i, j, k;
      Box3F box = (*s)->getWorldBox();
      
      // simple way to get our collision surfs
      ClippedPolyList polyList;
      polyList.mPlaneList.clear();
      polyList.mNormal.set(0, 0, 0);
      
      // this is all we will need from the polylist
      Vector<ClippedPolyList::Poly>   polys;
      Vector<ClippedPolyList::Vertex> verts;
      
      // these will be needed for reMap of verts
      U32 reMapSize = 0;
      U32 vCount = 0;
      U32 *reMap = NULL;
      
      // get our collision surfs from the polylist polys
      U32 mask = StaticShapeObjectType;
      if(gServerContainer.buildPolyList(box, mask, &polyList))
      {
         for(i = 0; i < polyList.mPolyList.size(); i++)  
         {
            ClippedPolyList::Poly *poly = &(polyList.mPolyList[i]);
            if(poly->object->getId() == (*s)->getId()) // make sure its the same obj
            {   
               VectorF normal;
               normal.set(poly->plane.x, poly->plane.y, poly->plane.z);
               mSlopeThresh = mCos(mDegToRad(45.0f));
               
               // what kind of slope does this surface have?
               F32 dot = mDot(normal, VectorF(0, 0, 1));
               if(dot >= mSlopeThresh)
                  polys.push_back(polyList.mPolyList[i]);
            }   
         }
      }
      else
         return; // didn't get any polys back from the polylist
      
      InteriorDetail d;
      
      // copy over the point list
      for(i = 0; i < polyList.mVertexList.size(); i++)
      {   
         Point3F pt = polyList.mVertexList[i].point;
         DPoint dpt;
         dpt.set(pt.x, pt.y, pt.z);
         dpt.flags.clear();
         dpt.e = NULL;
         d.mPoints.push_back(dpt);
      }
      
      // now create the surfs
      for(i = 0; i < polys.size(); i++)
      {
         DSurf currSurf;

         currSurf.mFlags.clear();
         currSurf.mFlipStates.clear();

         S32 vertCount = polys[i].vertexCount;
         currSurf.mEdges = new DEdge *[vertCount];
         currSurf.mDivLevel = 0;
         currSurf.parent = NULL;
         currSurf.mSubSurfCount = 0;
         currSurf.mNormal = polys[i].plane;
         
         for(j = 0, k = polys[i].vertexStart; k < (polys[i].vertexStart + polys[i].vertexCount); j++, k++)
            currSurf.fullWinding[j] = polyList.mIndexList[k]; 
         
         currSurf.mNumEdges = polys[i].vertexCount;
         
         d.mSurfaces.push_back(currSurf);
         d.mTraversable.push_back(i);
      }
      
      // build the edge table for this shape
      buildEdgeTable(&d);
      
      // subdivide until we have a workable graph for this shape
      U32 dummy = 0;
      while(subdivideSurfaces(&d) > 0)
         dummy++;
         
      // connect all volumes
      buildConnections(&d);
      
      // extract the graph
      graphExtraction(&d, &(mGraphDetails[mNumInteriors + mExtraInteriors++]));
      
      // clean up memory... YAY!!
      d.mEdgeTable.clear();
      U32 x;
      for(x = 0; x < d.mSurfaces.size(); x++)
         delete [] d.mSurfaces[x].mEdges;
   }
}

//----------------------------------------------------------------------------

Point3F FloorPlan::getMinBoxPoint(InteriorDetail *d, U8 n_edges, DEdge **edges)
{
   Point3F  minPt(1e9, 1e9, 1e9);
   for(S32 i = 0; i < n_edges; i++)
      minPt.setMin(d->mPoints[edges[i]->start]);
   return minPt;
}

//----------------------------------------------------------------------------

Point3F FloorPlan::getMaxBoxPoint(InteriorDetail *d, U8 n_edges, DEdge **edges)
{
   Point3F  maxPt(-1e9, -1e9, -1e9);
   for(S32 i = 0; i < n_edges; i++)
      maxPt.setMax(d->mPoints[edges[i]->start]);
   return maxPt;
}

//----------------------------------------------------------------------------

void FloorPlan::buildEdgeNormals(InteriorDetail *d, DSurf &surf, VectorF *normalList)
{
   Point3F start, end;
   F32 temp;
   
   U32 i;
   for(i = 0; i < surf.mNumEdges; i++)
   {
      start = d->mPoints[surf.mEdges[i]->start];
      end = d->mPoints[surf.mEdges[i]->end];
      normalList[i] = end - start;
      temp = normalList[i].x;
      
      if(!surf.mFlipStates.test(BIT(i)))
      {   
         normalList[i].x = (normalList[i].y * -1);
         normalList[i].y = temp;
      }
      else
      {   
         normalList[i].x = normalList[i].y;
         normalList[i].y = (temp * -1);
      }
      normalList[i].z = 0;
      normalList[i].normalize();
   }
}

//----------------------------------------------------------------------------

bool FloorPlan::obstructedSurf(InteriorDetail *d, DSurf &surf, DVolume *vol, surfCollisionInfo &info)
{
   F32 ht;
   bool obstruction = setupPolyList(d, surf, &ht, vol, info);
   
   if(obstruction)
      if(info.specialCase)
         return true;
      else
         return (ht < 2.4);
   else
      return false;
}

//-------------------------------------------------------------------------------------

// We lower the top when collisions are found so that later extrusion processing
// doesn't get a collision.                                                  
#define  LowerTheTop    0.05f

bool FloorPlan::setupPolyList(InteriorDetail *d, DSurf &surf, F32 *height, DVolume *vol, surfCollisionInfo &info)      
{   
   info;
   Box3F bBox;
   bBox.min = getMinBoxPoint(d, surf.mNumEdges, surf.mEdges);
   bBox.max = getMaxBoxPoint(d, surf.mNumEdges, surf.mEdges);
   bBox.max.z += (UncappedNodeVolumeHt + 10);
   F32 len;
   
   createCenterNode(d, &surf, &(surf.mCntr));
   
   if(surf.mLongest->length == surf.mEdges[0]->length)
      len = surf.mEdges[1]->length;
   else
      len = surf.mEdges[0]->length;   
   
   F32 threshold = 0.3 * len;
   bool useSpecial = false;   

   struct collisionInfo {   
      enum CollisionType {
        normal = 0,
        ground = 1,
      };
      
      CollisionType  type;
      F32            ht;
      
      collisionInfo()   {ht = 10000000; type = normal;}
   };
   
   ClippedPolyList polyList;
   ClippedPolyList testList;
   
   // build a volume for this surface
   vol->mNumPlanes = surf.mNumEdges+2;
   
   polyList.mPlaneList.clear();
   polyList.mPlaneList.setSize(surf.mNumEdges+1);
   polyList.mNormal.set(0, 0, 0);
   
   testList.mPlaneList.clear();
   testList.mPlaneList.setSize(surf.mNumEdges+1);
   testList.mNormal.set(0, 0, 0);
   
   VectorF *normals;
   normals = new VectorF[surf.mNumEdges];
   buildEdgeNormals(d, surf, normals);
   
   #if _FP_DEBUG_
   VectorF  saveNormals[40];
   Point3F  savePoints[40];
   S32      saveIndex = 0;
   #endif
   
   for(S32 i = 0; i < surf.mNumEdges; i++)
   {   
      // check flipstate for edge
      S32      ind = (surf.mFlipStates.test(BIT(i)) ? surf.mEdges[i]->end : surf.mEdges[i]->start);
      Point3F  pnt = d->mPoints[ind];
      Point3F  tPnt = d->mPoints[ind];

      // Note that we output a shrunk volume.  It is adjusted back out later after
      // some further processing is done on volumes.  
      pnt -= (normals[i] * NodeVolumeShrink);
      tPnt -= (normals[i] * threshold);
      
      polyList.mPlaneList[i].set(pnt, normals[i]);
      testList.mPlaneList[i].set(tPnt, normals[i]);
      vol->mPlanes[i].set(pnt, normals[i]);
      
      #if _FP_DEBUG_
      saveNormals[saveIndex] = normals[i];
      savePoints[saveIndex++] = pnt;
      #endif
      
   }
   delete [] normals;
   
   
   // cap the bottom for surfaces that have some slope
   Point3F p = d->mPoints[surf.mEdges[0]->start];
   
   VectorF vec(surf.mNormal);
   p += (vec * NodeVolumeShrink);
   vec.neg();
   
   polyList.mPlaneList[surf.mNumEdges].set(p, vec);
   testList.mPlaneList[surf.mNumEdges].set(p, vec);
   
   // cap the top of the test volume
   Point3F aP = p;
   aP.z += 2.5;
   VectorF  pU(0,0,1);
   PlaneF   tP(aP, pU);
   testList.mPlaneList.push_back(tP);
   
   
   vol->mPlanes[surf.mNumEdges].set(p, vec);
   *height = 0.0f;
   
   #if _FP_DEBUG_
   saveNormals[saveIndex] = vec;
   savePoints[saveIndex++] = p;
   // Debug check to stop on a surface containing sDebugBreakPoint
   if (mFabs(vol->mPlanes[surf.mNumEdges].distToPlane(sDebugBreakPoint)) < 0.6) {
      // volume doesn't have top plane right now, hence dec & inc:
      vol->mNumPlanes--;
      if (vol->checkInside(sDebugBreakPoint))
         debugStopPt();
      vol->mNumPlanes++;
   }
   #endif
   
#if TESTSPECIAL  
   
   // determine if we can split this surface in a way that will better our floorPlan
   useSpecial = isQuadSurface(d, surf) && specSurfTest(surf, testList, bBox);

#endif   
   
   // build the poly list
   U32 mask = InteriorObjectType | StaticShapeObjectType | TerrainObjectType | StaticObjectType; 
   if(gServerContainer.buildPolyList(bBox, mask, &polyList))
   {
      if (polyList.mPolyList.size()) 
      {
         F32                    maxHt = maxZforSurf(d, surf);
         Point3F                lowPt;
         F32                    ht;
         collisionInfo          lowCollision;
         DSide                  sides[4];
      
         for(S32 j = 0; j < polyList.mPolyList.size(); j++)  
         {
            ClippedPolyList::Poly *p = &(polyList.mPolyList[j]);
            for(S32 k = p->vertexStart; k < (p->vertexStart + p->vertexCount); k++)
            {   
               d->polyTestPoints.push_back(polyList.mVertexList[polyList.mIndexList[k]].point);
               ht = polyList.mVertexList[polyList.mIndexList[k]].point.z - maxHt;
               if(ht < lowCollision.ht)
               {   
                  lowPt = polyList.mVertexList[polyList.mIndexList[k]].point;
                  lowPt.z -= LowerTheTop;
                  lowCollision.ht = (ht - LowerTheTop);
                  
                  // determine collision type
                  if(p->object->getTypeMask() & TerrainObjectType)
                     lowCollision.type = collisionInfo::ground;
                  else 
                     lowCollision.type = collisionInfo::normal;
               }
               
#if TESTSPECIAL
               
               if(useSpecial && ht < 2.4) // ************optimize the division ****************
               {
                  Point3F  colPt = polyList.mVertexList[polyList.mIndexList[k]].point;
                  F32      distToPlane;
                  bool     found = false;
                  
                  for(U32 side = 0; side < 4; side++)
                  {
                     distToPlane = mFabs(polyList.mPlaneList[side].distToPlane(colPt));
                     if(distToPlane <= threshold)
                     {
                        if(distToPlane >= 0.05)
                        {    
                           sides[side].closePts++;
                           if(distToPlane > sides[side].bestDist)
                              sides[side].bestDist = distToPlane;
                           
                        }
                        found = true;   
                     }
                  }
                  
                  if(!found)
                     useSpecial = false;
               }
#endif         
            }
         }
         
#if TESTSPECIAL          
         
         if(useSpecial && haveGoodData(sides))
         {
            setCollisionInfoData(info, sides, surf);
            return true;  
         }
#endif         
         
         // if the lowest height was terrain, this space must be obstructed
         if(lowCollision.type == collisionInfo::ground)
            *height = 0;
         else 
         {
            *height = lowCollision.ht;
            vol->ht = *height;
            vol->mPlanes[surf.mNumEdges+1].set(lowPt, VectorF(0, 0, 1));
         }   
         return true;
      }
   }
   
   //if(!inSolid(surf.mCntr))
   //{
      *height = UncappedNodeVolumeHt;
      Point3F pt = d->mPoints[surf.mEdges[0]->start];
      pt.z += UncappedNodeVolumeHt;
      vol->mPlanes[surf.mNumEdges+1].set(pt, VectorF(0, 0, 1));
      vol->ht = UncappedNodeVolumeHt;
      return false;
   //}
   //else
   //{
      //*height = 0;
      //return true;
   //}   
}

//-------------------------------------------------------------------------------------

bool FloorPlan::inSolid(DPoint &center)
{
   Point3F start, end;
   start = center;
   start.z += 0.1;
   end = start;
   end.z += 1;
   
   RayInfo info;
   if(gServerContainer.castRay(start, end, InteriorObjectType, &info))
      return true;
   else
      return false;      
}

//-------------------------------------------------------------------------------------

bool FloorPlan::specSurfTest(DSurf &surf, ClippedPolyList &list, Box3F &b)
{
   surf;
   U32 mask = InteriorObjectType | StaticShapeObjectType | TerrainObjectType | StaticObjectType; 
   if(gServerContainer.buildPolyList(b, mask, &list))// || inSolid(surf.mCntr))
      return false;
   else
      return true;
}

//-------------------------------------------------------------------------------------

bool FloorPlan::haveGoodData(DSide *sides)
{
   S32 sideToUse = -1;
   S32 highSideCount = 0;
   
   for(S32 side = 0; side < 4; side++)
   {
      if(sides[side].closePts > highSideCount)
      {   
         sideToUse = side;
         highSideCount = sides[side].closePts;
      }        
   }
   
   return sideToUse != -1;
}

//-------------------------------------------------------------------------------------

bool FloorPlan::isQuadSurface(InteriorDetail *d, DSurf &surf)
{
   if(surf.mNumEdges == 4)
   {
      DEdge *edge0 = surf.mEdges[0];
      DEdge *edge1 = surf.mEdges[1];
      DEdge *edge2 = surf.mEdges[2];
      DEdge *edge3 = surf.mEdges[3];
      
      VectorF e0, e1, e2, e3;
      
      if(!surf.mFlipStates.test(BIT(0)))
         e0 = d->mPoints[edge0->end] - d->mPoints[edge0->start];
      else
         e0 = d->mPoints[edge0->start] - d->mPoints[edge0->end];
      
      if(!surf.mFlipStates.test(BIT(1)))
         e1 = d->mPoints[edge1->end] - d->mPoints[edge1->start];
      else
         e1 = d->mPoints[edge1->start] - d->mPoints[edge1->end];
      
      if(!surf.mFlipStates.test(BIT(2)))
         e2 = d->mPoints[edge2->end] - d->mPoints[edge2->start];
      else
         e2 = d->mPoints[edge2->start] - d->mPoints[edge2->end];
      
      if(!surf.mFlipStates.test(BIT(3)))
         e3 = d->mPoints[edge3->end] - d->mPoints[edge3->start];
      else
         e3 = d->mPoints[edge3->start] - d->mPoints[edge3->end];
         
      e0.normalize();
      e1.normalize();
      e2.normalize();
      e3.normalize();
      
      bool E0parE2 = (mFabs(mDot(e0, e2)) > 0.992);
      bool E1parE3 = (mFabs(mDot(e1, e3)) > 0.992);
      
      bool  useSpecial = E0parE2 && E1parE3;
      return useSpecial;
   }
   else
      return false;
}

//-------------------------------------------------------------------------------------

void FloorPlan::setCollisionInfoData(surfCollisionInfo &info, DSide *sides, DSurf &surf)
{
   info.specialCase = true;
   S32 sideToUse = -1;
   S32 highSideCount = 0;
   
   for(S32 side = 0; side < 4; side++)
   {
      if(sides[side].closePts > highSideCount)
      {   
         sideToUse = side;
         highSideCount = sides[side].closePts;
      }        
   }
   
   AssertFatal(sideToUse != -1, "no side chosen for collision info data");
   
   switch(sideToUse)
   {
      case 0:
         info.divideEdge1 = 3;
         info.divideEdge2 = 1;
         break;
      case 1:
         info.divideEdge1 = 0;
         info.divideEdge2 = 2;
         break;
      case 2:
         info.divideEdge1 = 1;
         info.divideEdge2 = 3;
         break;
      case 3:
         info.divideEdge1 = 2;
         info.divideEdge2 = 0;
         break;
   }
   
   S32 sideA = info.divideEdge1;
   S32 sideB = info.divideEdge2;
   
   F32 factorA = 1 - (sides[sideToUse].bestDist / surf.mEdges[sideA]->length);
   info.factor = factorA;   
}

//-------------------------------------------------------------------------------------

void FloorPlan::computeSurfaceRadius(InteriorDetail *d, DSurf *surf)
{
   if (surf->mFlags.test(DSurf::shouldBeCulled))
      return;     // else we get NaN values below (center of vertical surfaces)
   
   DPoint center;
   createCenterNode(d, surf, &center);
   
   F32   longest = -1e9f;
   F32   shortest = 1e9f;
   DEdge * * edges = surf->mEdges;
   for(S32 i = 0; i < surf->mNumEdges; i++)
   {
      LineSegment seg(d->mPoints[edges[i]->start], d->mPoints[edges[i]->end]);
      F32   D = seg.distance(center);
      longest = getMax(longest, D);
      shortest = getMin(shortest, D);
   }
   
   // longest from center to a vert = radius
   surf->mMaxRadius = longest;
   surf->mMinRadius = shortest;
}

//----------------------------------------------------------------------------

F32 FloorPlan::maxZforSurf(InteriorDetail *d, DSurf &surf)
{
   F32 maxZ = -1e12;
   for(S32 i = 0; i < surf.mNumEdges; i++)
      if(d->mPoints[surf.mEdges[i]->start].z > maxZ)
         maxZ = d->mPoints[surf.mEdges[i]->start].z;
   
   return maxZ;
}

//----------------------------------------------------------------------------

void FloorPlan::newConnection(InteriorDetail *d, U32 start, U32 end, DConnection &con)
{
   DConnection connect;
   connect.start = start;
   connect.end = end;
   connect.segNodes[0] = con.segNodes[0];
   connect.segNodes[1] = con.segNodes[1];
   
   d->mConnections.push_back(connect);
}

//----------------------------------------------------------------------------

void FloorPlan::buildConnections(InteriorDetail *d)
{
   U32 j, k, c, g;
   d->mConPoints.setSize(d->mUnobstructed.size());
   
   for(c = 0; c < d->mUnobstructed.size(); c++)
   {   
      DSurf &s = d->mSurfaces[d->mUnobstructed[c]];
      d->mConPoints[c] = s.mCntr;
      d->mConPoints[c].surfIdx = d->mUnobstructed[c];
      d->mConPoints[c].flags.clear();
      s.mCtrIdx = c;
      log(avar("surface %d zone: %d", c, s.mZone));
   } 
   
   d->mConnections.clear();
   
   for(j = 0; j < d->mUnobstructed.size(); j++)
   {   
      DSurf &surf_1 = d->mSurfaces[d->mUnobstructed[j]];
      for(k = (j+1); k < d->mUnobstructed.size(); k++)
      {
         DSurf &surf_2 = d->mSurfaces[d->mUnobstructed[k]];
         
         // early out based on distance
         if(!passDistanceCheck(surf_1, surf_2))
            continue;
         
         for(c = 0; c < surf_1.mNumEdges; c++)
         {
            DEdge *edge_1 = surf_1.mEdges[c];
            for(g = 0; g < surf_2.mNumEdges; g++)
            {
               DEdge *edge_2 = surf_2.mEdges[g];
               DConnection con;
               if(isSharedEdge(d, edge_1, edge_2, con))
               {   
                  newConnection(d, surf_1.mCtrIdx, surf_2.mCtrIdx, con);
                  break;
               }   
            }
         }
      }
   }
   //findDuplicateCons(d);
}

//----------------------------------------------------------------------------

bool FloorPlan::offHeightDistribution(InteriorDetail *d, DSurf &surf)
{
   // only perform this on flat surfaces
   if(surf.mNormal.z != 1)
      return false;
   
   // grab all the data
   Vector<DPoint> points;
   points.setSize(surf.mNumEdges+1);
   createCenterNode(d, &surf, &points[0]);
   points[0].z += 0.1;
   
   U32 i;
   for(i = 1; i <= surf.mNumEdges; i++)
   {
      if(!surf.mFlipStates.test(BIT(i-1)))
         points[i] = d->mPoints[surf.mEdges[i-1]->start];
      else
         points[i] = d->mPoints[surf.mEdges[i-1]->end];
      points[i].z += 0.1;     
   }
   
   // adjust our points
   VectorF *vecs = new VectorF[surf.mNumEdges];
   for(i = 1; i <= surf.mNumEdges; i++)
   {
      vecs[i-1] = points[0] - points[i];
      vecs[i-1].normalize();
      points[i] += (1.75 * vecs[i-1]);   
   }
   
   // now do the line of sights
   bool contact = false;
   F32 *hts = new F32[surf.mNumEdges+1];
   for(i = 0; i <= surf.mNumEdges; i++)
   {
      RayInfo info;
      Point3F end(points[i]);
      end.z += 100;
      if(gServerContainer.castRay(points[i], end, InteriorObjectType, &info))
      {   
         contact = true;
         hts[i] = info.point.z - points[i].z;     
      }
      else
         hts[i] = 100;
   }
   
   // don't bother if we have a completly unobstructed volume
   bool differs = false;
   if(contact)
      if(surf.mMaxRadius > sSpecialMaxRadius)
         differs = htsDifferAlot(hts, surf.mNumEdges+1);
   
   // clean up
   delete [] vecs;
   delete [] hts;
   return differs;
}

//----------------------------------------------------------------------------

bool FloorPlan::htsDifferAlot(F32 *hts, U8 n)
{
   // sort these first
   U32 i, j;
   for(i = 0; i < n; i++)
   {
      for(j = i; j < n; j++)
      {
         if(hts[i] > hts[j])
         {
            F32 temp = hts[i];
            hts[i] = hts[j];
            hts[j] = temp;
         }
      }
   }
   return (hts[n-1] - hts[0] > sHeightDiff);
}

//----------------------------------------------------------------------------

bool FloorPlan::passDistanceCheck(DSurf &surf_1, DSurf &surf_2)
{
   bool distance = (surf_2.mCntr - surf_1.mCntr).len() < (sMaxDivRadius*3);
  
//    VectorF floor(0, 0, 1);
//    if((surf_1.mNormal == floor) && (surf_2.mNormal == floor))
//    {   
//       bool sameZ = mFabs(surf_2.mCntr.z) == mFabs(surf_1.mCntr.z);
//       return distance && sameZ;
//    }
//    else
      return distance;
}

//----------------------------------------------------------------------------

bool FloorPlan::isSharedEdge(InteriorDetail *d, DEdge *edge_1, DEdge *edge_2, DConnection &con)
{
   DEdge *small;
   DEdge *big;
   DEdge *sourceEdge = edge_1;
   
   if(edge_1->length > edge_2->length)
   {   
      big = edge_1;
      small = edge_2;
   }
   else
   {   
      big = edge_2;
      small = edge_1;
   }
   
   // are any points equal?
   bool  ss_bs = small->start == big->start;
   bool  ss_be = small->start == big->end;
   bool  se_bs = small->end == big->start;
   bool  se_be = small->end == big->end;
   
   // easy case, hook up and leave
   if(((ss_bs) || (ss_be)) && ((se_bs) || (se_be))) 
   {   
      con.segNodes[0] = d->mPoints[small->start];
      con.segNodes[1] = d->mPoints[small->end];
      return isUsableInterface(con);
   }
   
   VectorF sourceVec = (d->mPoints[sourceEdge->end] - d->mPoints[sourceEdge->start]);
   VectorF vec_1 = (d->mPoints[big->end] - d->mPoints[big->start]);
   VectorF vec_2 = (d->mPoints[small->end] - d->mPoints[small->start]);
   vec_1.normalize();
   vec_2.normalize();
   sourceVec.normalize();
   
   Point3F p1 = d->mPoints[small->start];
   Point3F p2 = d->mPoints[small->end];
   Point3F p3 = d->mPoints[big->start];
   Point3F p4 = d->mPoints[big->end];
   
   // check if parallel
   bool parallel = (mFabs(mDot(vec_1, vec_2)) > sParallelDot);
   
   if(parallel)
   {
      LineSegment line(d->mPoints[big->start], d->mPoints[big->end]);
      
      F32 dist_1 = line.distance(d->mPoints[small->start]);
      F32 dist_2 = line.distance(d->mPoints[small->end]);
      
      // check if collinear
      if((dist_1 < sCollinearDist) || (dist_2 < sCollinearDist) || isSteppable(d, big, small))
      {
         // project Points of small line onto larger line
         VectorF vec_3 = (d->mPoints[small->start] - d->mPoints[big->start]); 
         VectorF vec_4 = (d->mPoints[small->end] - d->mPoints[big->start]);
         
         F32 proj_1 = mDot(vec_1, vec_3);
         F32 proj_2 = mDot(vec_1, vec_4);
         
         // get our t values based on the projection.
         F32 t1 = proj_1/big->length;
         F32 t2 = proj_2/big->length;
         
         // sorting makes things alittle easier
         bool  flipped = false;
         if(t1 > t2)
         {
            F32 temp = t1;  t1 = t2; t2 = temp;
            flipped = true;
         }
         
         // check if lines are overlapping
         if(t2 <= 0 || t1 >= 1)
            return false;
         
         // we know the small line is overlapping the bigger line
         // we just need to determine which case of overlap
         
         // case 1
         // containment or equal
         if((t1 >= 0) && (t2 <= 1))
         {
            con.segNodes[0] = d->mPoints[small->start];
            con.segNodes[1] = d->mPoints[small->end];
         }
         else 
         {
            // case 2
            // the edges overlap on the start end
            if(t1 < 0)
            {
               if (!flipped)
               {
                  con.segNodes[0] = d->mPoints[big->start];
                  con.segNodes[1] = d->mPoints[small->end];
               }
               else
               {
                  con.segNodes[0] = d->mPoints[big->start];
                  con.segNodes[1] = d->mPoints[small->start];
               }
            }
            // case 3
            // the edges overlap with t1 >= 0 and t2 extending past big->end
            else 
            {
               if (!flipped)
               {
                  con.segNodes[0] = d->mPoints[small->start];
                  con.segNodes[1] = d->mPoints[big->end];
               }
               else
               {
                  con.segNodes[0] = d->mPoints[small->end];
                  con.segNodes[1] = d->mPoints[big->end];
               }
            }
         }
         
         // Project both nodes onto the proper segment
         VectorF conVec1 = (con.segNodes[0] - d->mPoints[sourceEdge->start]);
         VectorF conVec2 = (con.segNodes[1] - d->mPoints[sourceEdge->start]);
         F32 conPro1 = mDot(sourceVec, conVec1);
         F32 conPro2 = mDot(sourceVec, conVec2);
         F32 tval1 = conPro1 / sourceEdge->length;
         F32 tval2 = conPro2 / sourceEdge->length;
         
         con.segNodes[0] = d->mPoints[sourceEdge->start] + ((d->mPoints[sourceEdge->end] - d->mPoints[sourceEdge->start]) * tval1);
         con.segNodes[1] = d->mPoints[sourceEdge->start] + ((d->mPoints[sourceEdge->end] - d->mPoints[sourceEdge->start]) * tval2);
         
         return isUsableInterface(con);
      }
   }      
   return false;   
}

//----------------------------------------------------------------------------

bool FloorPlan::isSteppable(InteriorDetail *d, DEdge *big, DEdge *small)
{
   F32 smallZ = d->mPoints[small->start].z;
   F32 bigZ   = d->mPoints[big->start].z; 
   
   if(mFabs(smallZ - bigZ) > 0.75)
      return false;
   
   Point3F ss = d->mPoints[small->start];
   Point3F se = d->mPoints[small->end];
   Point3F bs = d->mPoints[big->start];
   Point3F be = d->mPoints[big->end];
   se.z = ss.z = bs.z = be.z = 0.0f;
   
   LineSegment line(bs, be);
   
   return (line.distance(ss) < sCollinearDist) || (line.distance(se) < sCollinearDist);
}

//----------------------------------------------------------------------------

void FloorPlan::initPersistFields()
{
   Parent::initPersistFields();
}

//----------------------------------------------------------------------------

void FloorPlan::generate()
{
   mDivLevel = 0;
   snoopInteriors();
   
   Con::printf("Total portal nodes created: %d\n", portalPoints.size());   
}

//----------------------------------------------------------------------------

void FloorPlan::findDuplicateCons(InteriorDetail *d)
{
   U32 theSame = 0;
   
   U32 i, j;
   for(i = 0; i < d->mConnections.size(); i++)
   {
      DConnection &con_1 = d->mConnections[i];
      for(j = (i+1); j < d->mConnections.size(); j++)
      {
         DConnection &con_2 = d->mConnections[j];
         if(con_1 == con_2)
         {
            theSame++;
            d->mConnections.erase(j--);
         }
      }
   } 
   Con::printf("duplicate connections removed from %d: %d\n", d->mIndex, theSame); 
}


// console functions
//----------------------------------------------------------------------------

static void cGenerate(SimObject *ptr, S32, const char **)
{
   FloorPlan *obj = static_cast<FloorPlan *>(ptr);
   
   if(obj)
      obj->generate();
}

//----------------------------------------------------------------------------

static void cUpload(SimObject *ptr, S32, const char **)
{
   FloorPlan *obj = static_cast<FloorPlan *>(ptr);
   NavigationGraph *ng = dynamic_cast<NavigationGraph*>(Sim::findObject("NavGraph"));
   
   if(!ng)
      Con::printf("no navigationGraph exists!");
   
   if(obj)
      obj->upload2NavGraph(ng);
}

//----------------------------------------------------------------------------

static void cAddStaticCenter(SimObject *ptr, S32, const char **argv)
{
   FloorPlan *fp = static_cast<FloorPlan *>(ptr);
   
   S32 id = dAtoi(argv[2]);
   ShapeBase *shape = NULL;
   Sim::findObject(id, shape);
   
   fp->mStaticShapeCenterList.push_back(shape);
}

//----------------------------------------------------------------------------

static void cAddStaticGeom(SimObject *ptr, S32, const char **argv)
{
   FloorPlan *fp = static_cast<FloorPlan *>(ptr);
   
   S32 id = dAtoi(argv[2]);
   ShapeBase *shape = NULL;
   Sim::findObject(id, shape);
   
   fp->mStaticShapeGeometryList.push_back(shape);
}

//----------------------------------------------------------------------------

void FloorPlan::consoleInit()
{
   Parent::consoleInit();
   
   Con::addVariable("FloorPlan::CollinearDist", TypeF32, &sCollinearDist);
   Con::addVariable("FloorPlan::ParallelDot", TypeF32, &sParallelDot);
   Con::addVariable("FloorPlan::FloorAngleThresh", TypeS32, &sFloorThresh);
   Con::addVariable("FloorPlan::HeightDiff", TypeF32, &sHeightDiff);
   Con::addVariable("FloorPlan::SpecialMaxRadius", TypeF32, &sSpecialMaxRadius);
   Con::addVariable("FloorPlan::MaxRadius", TypeF32, &sMaxDivRadius);
   Con::addVariable("FloorPlan::MinRadius", TypeF32, &sMinDivRadius);
   Con::addVariable("FloorPlan::subdivide", TypeBool, &sSubDivide);
   Con::addVariable("FloorPlan::drawConnections", TypeBool, &sDrawConnections);
   Con::addVariable("FloorPlan::drawVols", TypeBool, &sDrawVolumeHts);
   Con::addVariable("FP::special", TypeBool, &sUseSpecial);
   Con::addVariable("FP::DisableAsserts", TypeBool, &sDisableAsserts);
   Con::addVariable("FP::divCount", TypeS32, &amountToDivide);
   Con::addCommand("FloorPlan", "generate", cGenerate, "obj.generate()", 2, 2);
   Con::addCommand("FloorPlan", "upload", cUpload, "obj.upload()", 2, 2);
   Con::addCommand("FloorPlan", "addStaticCenter", cAddStaticCenter, "obj.addStaticCenter( shape )", 3, 3);
   Con::addCommand("FloorPlan", "addStaticGeom", cAddStaticGeom, "obj.addStaticGeom( shape )", 3, 3);
   Con::linkNamespaces("SimObject", "FloorPlan");
}

