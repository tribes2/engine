//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _TSSHAPECONSTRUCT_H_
#define _TSSHAPECONSTRUCT_H_

#ifndef _TSSHAPE_H_
#include "ts/tsShape.h"
#endif
#ifndef _CONSOLEOBJECT_H_
#include "console/consoleObject.h"
#endif
#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif

class TSShapeConstructor : public SimDataBlock
{
   typedef SimDataBlock Parent;

   enum {
      NumSequenceBits = 7,
      MaxSequences = (1 << NumSequenceBits) - 1
   };

   StringTableEntry mShape;
   StringTableEntry mSequence[MaxSequences];
   
   Resource<TSShape> hShape;

public:
   
   TSShapeConstructor();
   ~TSShapeConstructor();
   bool onAdd();
   void packData(BitStream* stream);
   void unpackData(BitStream* stream);
      
   DECLARE_CONOBJECT(TSShapeConstructor);
   static void consoleInit();
   static void initPersistFields();

   static const char * csmShapeDirectory;
};

#endif
