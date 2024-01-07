//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "core/dnet.h"
#include "console/simBase.h"
#include "sim/netConnection.h"
#include "sim/netStringTable.h"
#include "core/stringTable.h"

NetStringTable *gNetStringTable = NULL;

NetStringTable::NetStringTable()
{
   firstFree = 1;
   firstValid = 1;
   for(U32 i = 0; i < MaxStrings; i++)
   {
      table[i].next = i + 1;
      table[i].refCount = 0;
   }
   for(U32 j = 0; j < HashTableSize; j++)
      hashTable[j] = 0;
   allocator = new DataChunker(DataChunkerSize);
}

NetStringTable::~NetStringTable()
{
   delete allocator;
}

void NetStringTable::clearSentBit(U32 id)
{
   // a string has been added - loop through the connections
   // and mark it as not yet sent.
   for(NetConnection *conn = NetConnection::getConnectionList(); conn; conn = conn->getNext())
      conn->clearString(id);
}

void NetStringTable::incStringRef(U32 id)
{
   AssertFatal(table[id].refCount != 0, "Cannot inc ref count from zero.");
   table[id].refCount++;
}

U32 NetStringTable::addString(const char *string)
{
   U32 hash = _StringTable::hashString(string);
   U32 bucket = hash % HashTableSize;
   for(U16 walk = hashTable[bucket];walk; walk = table[walk].next)
   {
      if(!dStrcmp(table[walk].string, string))
      {
         table[walk].refCount++;
         return walk;
      }
   }
   U16 e = firstFree;
   firstFree = table[e].next;
   table[e].refCount++;
   table[e].string = (char *) allocator->alloc(dStrlen(string) + 1);
   dStrcpy(table[e].string, string);
   table[e].next = hashTable[bucket];
   hashTable[bucket] = e;
   table[e].link = firstValid;
   table[firstValid].prevLink = e;
   firstValid = e;
   table[e].prevLink = 0;
   clearSentBit(e);
   return e;
}

U32 GameAddTaggedString(const char *string)
{
   return gNetStringTable->addString(string);
}

const char *NetStringTable::lookupString(U32 id)
{
   if(table[id].refCount == 0)
      return NULL;
   return table[id].string;
}

void NetStringTable::removeString(U32 id)
{
   if(--table[id].refCount)
      return;
   // unlink first:
   U16 prev = table[id].prevLink;
   U16 next = table[id].link;
   if(next)
      table[next].prevLink = prev;
   if(prev)
      table[prev].link = next;
   else
      firstValid = next;
   // remove it from the hash table
   U32 hash = _StringTable::hashString(table[id].string);
   U32 bucket = hash % HashTableSize;
   for(U16 *walk = &hashTable[bucket];*walk; walk = &table[*walk].next)
   {
      if(*walk == id)
      {
         *walk = table[id].next;
         break;
      }
   }
   table[id].next = firstFree;
   firstFree = id;
}                       

void NetStringTable::repack()
{
   DataChunker *newAllocator = new DataChunker(DataChunkerSize);
   for(U16 walk = firstValid; walk; walk = table[walk].link)
   {
      const char *prevStr = table[walk].string;


      table[walk].string = (char *) newAllocator->alloc(dStrlen(prevStr) + 1);
      dStrcpy(table[walk].string, prevStr);
   }
   delete allocator;
   allocator = newAllocator;
}

void NetStringTable::create()
{
   AssertFatal(gNetStringTable == NULL, "Error, calling NetStringTable::create twice.");   
   gNetStringTable = new NetStringTable();
}

void NetStringTable::destroy()
{
   AssertFatal(gNetStringTable != NULL, "Error, not calling NetStringTable::create.");   
   delete gNetStringTable;
   gNetStringTable = NULL;
}

void NetStringTable::expandString(U32 id, char *buf, U32 bufSize, U32 argc, const char **argv)
{
   buf[0] = StringTagPrefixByte;
   dSprintf(buf + 1, bufSize - 1, "%d ", id);

   const char *string = gNetStringTable->lookupString(id);
   if (string != NULL) {
      U32 index = dStrlen(buf);
      while(index < bufSize)
      {
         char c = *string++;
         if(c == '%')
         {
            c = *string++;
            if(c >= '1' && c <= '9')
            {
               U32 strIndex = c - '1';
               if(strIndex >= argc)
                  continue;
               // start copying out of arg index
               const char *copy = argv[strIndex];
               // skip past any tags:
               if(*copy == StringTagPrefixByte)
               {
                  while(*copy && *copy != ' ')
                     copy++;
                  if(*copy)
                     copy++;
               }
            
               while(*copy && index < bufSize)
                  buf[index++] = *copy++;
               continue;
            }
         }
         buf[index++] = c;
         if(!c)
            break;
      }
      buf[bufSize - 1] = 0;
   } else {
      dStrcat(buf, "<NULL>");
   }
}

#ifdef DEBUG
void NetStringTable::dumpToConsole()
{
   U32 count = 0;
   S32 maxIndex = -1;
   for ( U32 i = 0; i < MaxStrings; i++ )
   {
      if ( table[i].refCount > 0 )
      {
         Con::printf( "%d: \"%c%s%c\" REF: %d", i, 0x10, table[i].string, 0x11, table[i].refCount );
         if ( maxIndex == -1 || table[i].refCount > table[maxIndex].refCount )
            maxIndex = i;
         count++;
      }
   }
   Con::printf( ">> STRINGS: %d MAX REF COUNT: %d \"%c%s%c\" <<", 
         count, 
         ( maxIndex == -1 ) ? 0 : table[maxIndex].refCount, 
         0x10, 
         ( maxIndex == -1 ) ? "" : table[maxIndex].string, 
         0x11 );
}

ConsoleFunction( dumpNetStringTable, void, 1, 1, "dumpNetStringTable()" )
{
   argc; argv;
   gNetStringTable->dumpToConsole();
}
#endif // DEBUG
