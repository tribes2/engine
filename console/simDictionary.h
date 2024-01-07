//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _SIMDICTIONARY_H_
#define _SIMDICTIONARY_H_
#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif
#ifndef _STRINGTABLE_H_
#include "Core/stringTable.h"
#endif

class SimObject;

//----------------------------------------------------------------------------
// Map of Names to simObjects
// Provides fast lookup for name->object and
// for fast removal of an object given object*
//
class SimNameDictionary
{
   enum
   {
      DefaultTableSize = 29
   };

   SimObject **hashTable;  // hash the pointers of the names...
   S32 hashTableSize;
   S32 hashEntryCount;
public:
	void insert(SimObject* obj);
	void remove(SimObject* obj);
	SimObject* find(StringTableEntry name);

	SimNameDictionary();
	~SimNameDictionary();
};

class SimManagerNameDictionary
{
   enum
   {
      DefaultTableSize = 29
   };

   SimObject **hashTable;  // hash the pointers of the names...
   S32 hashTableSize;
   S32 hashEntryCount;
public:
	void insert(SimObject* obj);
	void remove(SimObject* obj);
	SimObject* find(StringTableEntry name);

	SimManagerNameDictionary();
	~SimManagerNameDictionary();
};

//----------------------------------------------------------------------------
// Map of ID's to simObjects
// Provides fast lookup for ID->object and
// for fast removal of an object given object*
//
class SimIdDictionary
{
   enum
   {
      DefaultTableSize = 4096,
      TableBitMask = 4095
   };
   SimObject *table[DefaultTableSize];
public:
	void insert(SimObject* obj);
	void remove(SimObject* obj);
	SimObject* find(S32 id);

	SimIdDictionary();
	~SimIdDictionary();
};

#endif //_SIMDICTIONARY_H_
