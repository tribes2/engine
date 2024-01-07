//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "ai/graph.h"
#include "game/missionMarker.h"

//-------------------------------------------------------------------------------------

// Utility routine for below-  Get nodes in sphere, separated into indoor/outdoor
void NavigationGraph::getInSphere(const SphereF& S, GraphNodeList& I, GraphNodeList& O)
{
   // Find all the candidate nodes.  
   for (S32 i = 0; i < numNodes(); i++)
      if (GraphNode * node = lookupNode(i))
         if (S.isContained(node->location()))
            (node->outdoor() ? O : I).push_back(node);
}

static void findSpawnSpheres(Vector<SpawnSphere* > & list)
{  
   for (SimSetIterator itr(Sim::getRootGroup()); *itr; ++itr)
      if (SpawnSphere * ss = dynamic_cast<SpawnSphere *>(*itr))
         if (!ss->isClientObject())
            for (S32 i = list.size() - 1; i > -2; i--)   // Check if it's already in our
               if (i < 0)                                // list since the spawn spheres
                  list.push_back(ss);                    // are kept in a couple of sets
               else if (ss == list[i]) 
                  break;
}

// Build the spawn data out from the node list.  We're basically making a pre-
// randomized list per spawn sphere so the spawn lookups happen fast, plus
// we control the amount of data we're using for this system.  
void NavigationGraph::makeSpawnList()
{
   mSpawnList.reset();
   Vector<SpawnSphere* > spawnSpheres;
   findSpawnSpheres(spawnSpheres);
   if (spawnSpheres.size() == 0)
   {
      Con::printf("Warning!  No spawn sphere markers found in mission file!");
      return;
   }

   // For holding lists for each sphere-    
   GraphNodeList  indoor;
   GraphNodeList  outdoor;
   indoor.reserve(mNumIndoor);
   outdoor.reserve(mNumOutdoor);

   // List to build-    
   SpawnLocations    spawnList;
   
   // Build our list of spheres for indoor / outdoor.  
   for (S32 s = 0; s < spawnSpheres.size(); s++)
   {
      outdoor.clear();
      indoor.clear();

      // Get the lists-       
      SpawnSphere * ss = spawnSpheres[s];
      SpawnLocations::Sphere sphereList(ss->getPosition(), ss->mRadius);
      getInSphere(sphereList.mSphere, indoor, outdoor);

      // Add spheres for outdoor and indoor if wanted and available.        
      if (ss->mIndoorWeight) {
         if (indoor.size()) {
            sphereList.mInside = true;
            spawnList.mSpheres.push_back(sphereList);
         }
         else
            Con::printf("Warning- Spawn sphere with no INDOOR nodes found!");
      }
      if (ss->mOutdoorWeight) {
         if (outdoor.size()) {
            sphereList.mInside = false;
            spawnList.mSpheres.push_back(sphereList);
         }
         else
            Con::printf("Warning- Spawn sphere with no OUTDOOR nodes found!");
      }
   }
   
   // Figure out max budget per spawn sphere.  Try to keep it to 300 total, but each one
   // should have at least 20.  
   S32   maxPer = 50;
   if (spawnList.mSpheres.size() > 1) 
      maxPer = getMin(maxPer, 300 / spawnList.mSpheres.size());
   maxPer = getMax(20, maxPer);
   
   // Create pre-randomized location list for each sphere.  
   for (S32 i = 0; i < spawnList.mSpheres.size(); i++)
   {
      SpawnLocations::Sphere & sphereList = spawnList.mSpheres[i];
      outdoor.clear();
      indoor.clear();
      getInSphere(sphereList.mSphere, indoor, outdoor);
      
      S32   maxPointsHere = maxPer;
      
      // Get the list of points.  If it's indoor and the # of nodes is smaller
      // than the budget, then just scan all of them.  Otherwise ... randomize.
      sphereList.mOffset = spawnList.size();
      if (sphereList.mInside && indoor.size() < maxPointsHere)
      {
         for (S32 j = 0; j < indoor.size(); j++)
         {
            GraphNode   *  node = indoor[j];
            Point3F        point = node->location();
            if (sanctionSpawnLoc(node, point))
               spawnList.push_back(point);
         }
      }
      else
      {
         GraphNodeList &   nodeList = (sphereList.mInside ? indoor : outdoor);
         U32               nodeListSize = nodeList.size();
         S32               numPushed = 0;

         // Go for a while longer in case a lot are filtered- 
         for (S32 j = 0; j < (maxPointsHere * 10) && numPushed < maxPointsHere; j++)
         {
            U32            randIndex = (gRandGen.randI() % nodeListSize);
            GraphNode   *  node = nodeList[randIndex];
            Point3F        point = node->randomLoc();
            
            if (sanctionSpawnLoc(node, point))
            {
               spawnList.push_back(point);
               numPushed++;
            }
         }
      }

      // Set how many actually got added- 
      sphereList.mCount = (spawnList.size() - sphereList.mOffset);

      // Hopefully some got added-
      if (sphereList.mCount == 0)
         Con::printf("Warning:  Spawn sphere didn't have any valid nodes within it.");
   }
   
   // List is made, copy it over- 
   mSpawnList = spawnList;
}

//-------------------------------------------------------------------------------------
// Spawn node functions.  Old system operates if there's a NAV, else the new one 
// if it's just a spawn graph.  

const Point3F * NavigationGraph::getSpawnLoc(S32 nodeIndex)
{
   if (mSpawnList.empty())
   {
      if (validArrayIndex(nodeIndex, numNodes()))
         if (GraphNode * node = lookupNode(nodeIndex))
            return & node->location();
   }
   else
   {
      if (validArrayIndex(nodeIndex, mSpawnList.size()))
         return & mSpawnList[nodeIndex];
   }
   return NULL;
}

const Point3F * NavigationGraph::getRandSpawnLoc(S32 nodeIndex)
{
   if (mSpawnList.empty())
   {
      if (validArrayIndex(nodeIndex, numNodes()))
         if (GraphNode * node = lookupNode(nodeIndex))
         {
            static Point3F sPoint;
            sPoint = node->randomLoc();
            return & sPoint;
         }
   }
   else
   {
      if (validArrayIndex(nodeIndex, mSpawnList.size()))
         return & mSpawnList[nodeIndex];
   }
   return NULL;
}

//-------------------------------------------------------------------------------------

#define  CastShift         6
#define  NumCasts          (1 << CastShift)
#define  MaskOff           (NumCasts - 1)
#define  Periphery         (NumCasts / 4 + 1)
#define  AngleInc          ((M_PI * 2.0) / F32(NumCasts))
#define  CapDistance       60.0

// Do NumCasts LOS casts and save the distances (squared) and angles.  Return minimum
// of the dists array.  Used by the spawn function to find where to look, and by the 
// code that pre-generates the random spawn locations. 
static F32 lookAround(Point3F P, F32 dists[NumCasts], F32 qAngles[NumCasts])
{
   F32         minDist = (CapDistance * CapDistance);
   RayInfo     coll;

   // Maybe get eye-height here, though midsection level would work better for 
   // guaranteeing a nicer view, in most cases. 
   P.z += 1.0;
   
   // Gather distances.  
   for (S32 i = 0; i < NumCasts; i++) 
   {
      // Get quat angle, and do 90 degree conversion
      F32   ang = F32(i) * AngleInc;
      qAngles[i] = ang;
      ang += (M_PI / 2.0);
      
      // Get Direction vector- convert to Dest vector.  
      VectorF  D(mCos(ang), mSin(ang), 0);
      D *= CapDistance;
      D += P;

      // Find distances squared, and the min- 
      // const U32 mask = (InteriorObjectType|StaticShapeObjectType|TerrainObjectType);
      if (gServerContainer.castRay(P, D, U32(-1), &coll)) 
         minDist = getMin( dists[i] = (coll.point -= P).lenSquared(), minDist );
      else
         dists[i] = (CapDistance * CapDistance);
   }
   
   return minDist;
}

// This really isn't a graph function per se, but since this is affiliated with the 
// spawn graph stuff, might as well go in the graph code.  
F32 NavigationGraph::whereToLook(Point3F P)
{
   F32   dists[NumCasts];
   F32   qAngles[NumCasts];

   // Do circle of LOS casts-    
   lookAround(P, dists, qAngles);

   // Now find out what looks the best.  Maximize difference of squares form forward
   // to side.  Note that side angle is a little more than 90 deg so that we angle 
   // away from wall a little bit.  
   S32         bestIndex = 0;
   F32         bestMetric = -1e13, sideDist, metric;
   const U32   wrap = (NumCasts - 1);
   for (U32 j = 0; j < NumCasts; j++) 
   {
      // Take smaller off two off to the side distances- 
      sideDist = getMin(dists[(j - Periphery) & wrap], dists[(j + Periphery) & wrap]);
     
      // Metric is (forward dist squared) - ((smaller) side distance squared)
      if ((metric = (dists[j] - sideDist)) > bestMetric)
         bestMetric = metric, bestIndex = j;
   }
   
   // Return the angle need for get/set transform.  
   return qAngles[bestIndex];
}

//-------------------------------------------------------------------------------------

#define  SpawnSlopeLimit      0.866                // cos 30

static bool checkSlopeBelow(const Point3F& point) 
{
   RayInfo     coll;
   Point3F     below(point.x, point.y, point.z - 10.0);
   
   if (gServerContainer.castRay(point, below, U32(-1), &coll)) 
      if (coll.normal.z > SpawnSlopeLimit)
         return true;
         
   return false;
}

// A spawn location is only generated if, looking from waist out in a circle, no 
// collisions found within these thresholds.  
#define  IndoorSpawnExtraDist    1.8
#define  OutdoorSpawnExtraDist   7.2

bool NavigationGraph::sanctionSpawnLoc(GraphNode * node, const Point3F& point) const
{
   if (!node->liquidZone() && !node->inventory() && checkSlopeBelow(point))
   {
      F32   dists[NumCasts];
      F32   angles[NumCasts];
      F32   thresh = (node->indoor() ? IndoorSpawnExtraDist : OutdoorSpawnExtraDist);
      F32   minDistSqrd = lookAround(point, dists, angles);
      return minDistSqrd > (thresh * thresh);
   }
   return false;
}

//-------------------------------------------------------------------------------------

// The parameters are from randI(), make a 64 bit return value in range [0,1]
static F64 getRandom64(U64 r1, U64 r2)
{
   r1 <<= 31;
   r1 |= r2;                                     
   F64   frand = F64(r1);       
   F64   fnorm = frand * (1.0 / 4611686018427387904.0);
   return fnorm;
}

S32 NavigationGraph::randNode(const Point3F& P, F32 R, bool indoor, bool outdoor)
{
   SphereF  sphere(P, R);

   // If there's a valid spawn list, that's what we use.  This will occcur when 
   // it's a SPN file, or a NAV file from which only the spawn data was loaded. 
   if (!mSpawnList.empty())
   {
      return mSpawnList.getRandom(sphere, indoor, gRandGen.randI());
   }

   // Use graph utility lists to save on realloc calls.  
   mUtilityNodeList1.clear();
   mUtilityNodeList2.clear();
   
   // Note terrain should be done first since getNodesInArea() clears list...
   if (outdoor && haveTerrain())
   {
      S32         gridRad = S32(R / gNavGlobs.mSquareWidth) + 1;
      GridArea    gridArea = getGridRectangle(P, gridRad);
      getNodesInArea(mUtilityNodeList1, gridArea);
   }

   // ... and indoor second since getIntersecting() appends to list.
   if (indoor)
   {   
      Box3F    wBox(P, P, true);
      Point3F  ext(R, R, R);
      wBox.min -= ext;
      wBox.max += ext;
      mIndoorTree.getIntersecting(mUtilityNodeList1, wBox);
   }
      
   S32   count = mUtilityNodeList2.searchSphere(sphere, mUtilityNodeList1);
   
   #if 1
      if (count > 0)
         return mUtilityNodeList2[gRandGen.randI() % count]->getIndex();
   #else
      // Stuff to do weighted average- 
      // Get total area, along with a cutout point that should give us a 
      // weighted random of the nodes based on their estimated area- 
      F64   cutoffArea = 0.0f;
      for (S32 pass = 1; pass <= 2; pass++) 
      {
         F64   totalArea = 0.0f;
         for (S32 i = 0; i < count; i++) 
         {
            F64   R = insideSphere[i]->radius();
            F64   area = (R * R);
            totalArea += area;
            if (pass == 2 && totalArea >= cutoffArea)
               return insideSphere[i]->getIndex();
         }
         if (pass == 1)
            cutoffArea = totalArea * getRandom64(gRandGen.randI(), gRandGen.randI());
      }
   #endif
   
   return -1;
}
