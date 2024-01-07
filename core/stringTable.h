//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _STRINGTABLE_H_
#define _STRINGTABLE_H_

//Includes
#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif
#ifndef _DATACHUNKER_H_
#include "Core/dataChunker.h"
#endif


//--------------------------------------
class _StringTable
{
private:
   struct Node
   {
      char *val;
      Node *next;
   };

   Node** buckets;
   U32    numBuckets;
   U32    itemCount;
   DataChunker mempool;

  protected:
   static const U32 csm_stInitSize;

   _StringTable();
   ~_StringTable();

  public:
   static void create();
   static void destroy();

   StringTableEntry insert(const char *string, bool caseSens = false);
   StringTableEntry insertn(const char *string, S32 len, bool caseSens = false);
   StringTableEntry lookup(const char *string, bool caseSens = false);
   StringTableEntry lookupn(const char *string, S32 len, bool caseSens = false);
   void             resize(const U32 newSize);

   static U32 hashString(const char* in_pString);
   static U32 hashStringn(const char* in_pString, S32 len);
};


extern _StringTable *StringTable;


#endif //_STRINGTABLE_H_

