//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _MATERIALPROPERTYMAP_H_
#define _MATERIALPROPERTYMAP_H_

#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif
#ifndef _TVECTOR_H_
#include "Core/tVector.h"
#endif
#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif
#ifndef _COLOR_H_
#include "Core/color.h"
#endif

class MaterialPropertyMap : public SimObject
{
   typedef SimObject Parent;

  public:
   enum MaterialType {
      Default
   };

   enum MaterialFlags {
      None = 0 << 0
   };

   struct MapEntry {
      StringTableEntry  name;
      StringTableEntry  detailMapName;
      StringTableEntry  environMapName;

      MaterialType      matType;
      U32               matFlags;

      float             environMapFactor;

      S32               sound;
      ColorF            puffColor[2];
   };

  public:
   MaterialPropertyMap();
   ~MaterialPropertyMap();

   const MapEntry* getMapEntry(StringTableEntry) const;
   const MapEntry* getMapEntryFromIndex(S32 index) const;
   S32 getIndexFromName(StringTableEntry name) const;

   DECLARE_CONOBJECT(MaterialPropertyMap);
   static void consoleInit();

   // Should only be used by console functions
  public:
   bool addMapping(const S32, const char**);

   //-------------------------------------- Internal interface
  private:
   MapEntry* getNCMapEntry(StringTableEntry);
   
   //-------------------------------------- Data
  private:
   Vector<MapEntry>  mMapEntries;
};

#endif  // _H_MATERIALPROPERTYMAPPING_
