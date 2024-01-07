//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _NETSTRINGTABLE_H_
#define _NETSTRINGTABLE_H_

#ifndef _DATACHUNKER_H_
#include "engine/core/dataChunker.h"
#endif

class NetConnection;

class NetStringTable
{
public:
   enum {
      MaxStrings = 4096,
      HashTableSize = 1287,
      StringIdBitSize = 12,
      DataChunkerSize = 65536
   };
private:
   struct Entry
   {
      char *string;
      U16 refCount;
      U16 next;
      U16 link;
      U16 prevLink;
      U32 seq;
   };
   U16 firstFree;
   U16 firstValid;
   U32 sequenceCount;
   
   Entry table[MaxStrings];
   U16 hashTable[HashTableSize];
   DataChunker *allocator;

public:
    NetStringTable();
   ~NetStringTable();

   void clearSentBit(U32 id);
public:
   U32 addString(const char *string);
   const char *lookupString(U32 id);
   void incStringRef(U32 id);
   void removeString(U32 id);
   void sendString(U32 id);
   void repack();
   static void create();
   static void destroy();
   
   static void expandString(U32 id, char *buf, U32 bufSize, U32 argc, const char **argv);

#ifdef DEBUG
   void dumpToConsole();
#endif // DEBUG
};

extern NetStringTable *gNetStringTable;
#endif
