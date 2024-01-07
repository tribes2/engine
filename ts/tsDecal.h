//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _TSDECAL_H_
#define _TSDECAL_H_

#ifndef _TSMESH_H_
#include "ts/tsMesh.h"
#endif

class TSDecalMesh
{
public:

   // The mesh that we decal...
   TSMesh * targetMesh;

   // topology...
   ToolVector<TSDrawPrimitive> primitives;
   ToolVector<U16> indices;

   // indexed by decal frame...
   ToolVector<S32> startPrimitive;
   ToolVector<Point4F> texgenS;
   ToolVector<Point4F> texgenT;

   // We only allow 1 material per decal...
   S32 materialIndex;

   // override render function
   void render(S32 frame, S32 decalFrame, TSMaterialList *);

   void disassemble();
   void assemble(bool skip);

   static void initDecalMaterials();
   static void resetDecalMaterials();
};


#endif

