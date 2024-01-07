//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _TSSORTEDMESH_H_
#define _TSSORTEDMESH_H_

#ifndef _TSMESH_H_
#include "ts/tsMesh.h"
#endif

class TSSortedMesh : public TSMesh
{
public:
   typedef TSMesh Parent;
   
   struct Cluster
   {
      S32 startPrimitive;
      S32 endPrimitive;
      Point3F normal;
      F32 k;
      S32 frontCluster; // go to this cluster if in front of plane, if frontCluster<0, no cluster
      S32 backCluster;  // go to this cluster if in back of plane, if backCluster<0, no cluster
                        // if frontCluster==backCluster, no plane to test against...
   };

   ToolVector<Cluster> clusters;
   ToolVector<S32> startCluster; // indexed by frame number
   ToolVector<S32> firstVerts;   // indexed by frame number
   ToolVector<S32> numVerts;     // indexed by frame number
   ToolVector<S32> firstTVerts;  // indexed by frame number or matFrame number, depending on which one animates (never both)

   // sometimes, we want to write the depth value to the frame buffer even when object is translucent
   bool alwaysWriteDepth;
   
   // render methods..
   void render(S32 frame, S32 matFrame, TSMaterialList *);
   void renderFog(S32 frame);

   // collision methods...
   bool buildPolyList(S32 frame, AbstractPolyList * polyList, U32 & surfaceKey);
   bool castRay(S32 frame, const Point3F & start, const Point3F & end, RayInfo * rayInfo);
   bool buildConvexHull(); // does nothing, skins don't use this
   S32 getNumPolys();

   void assemble(bool skip);
   void disassemble();

   TSSortedMesh() {
      meshType = SortedMeshType;
   }
};

#endif // _TS_SORTED_MESH


