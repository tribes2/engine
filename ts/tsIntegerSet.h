//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _TSINTEGERSET_H_
#define _TSINTEGERSET_H_

#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif
#ifndef _TVECTOR_H_
#include "Core/tVector.h"
#endif


#ifdef MAX_UTIL
#define MAX_TS_SET_DWORDS 32
#else
#define MAX_TS_SET_DWORDS 6
#endif

#define MAX_TS_SET_SIZE   (32*MAX_TS_SET_DWORDS)

class Stream;

class TSIntegerSet
{
   U32 bits[MAX_TS_SET_DWORDS];

public:

   void clear(S32 index);
   void set(S32 index);
   bool test(S32 index) const;
   
   void clearAll(S32 upto = MAX_TS_SET_SIZE);
   void setAll(S32 upto = MAX_TS_SET_SIZE);
   bool testAll(S32 upto = MAX_TS_SET_SIZE) const;

   void intersect(const TSIntegerSet&);
   void overlap(const TSIntegerSet&);
   void difference(const TSIntegerSet&);
   void takeAway(const TSIntegerSet&);

   void copy(const TSIntegerSet&);

   void operator=(const TSIntegerSet& otherSet) { copy(otherSet); }

   S32 start() const;
   S32 end() const;
   void next(S32 & i) const;
   
   void read(Stream *);
   void write(Stream *);
   
   TSIntegerSet();
   TSIntegerSet(const TSIntegerSet&);
};

inline void TSIntegerSet::clear(S32 index)
{
   AssertFatal(index>=0 && index<MAX_TS_SET_SIZE,"TS::IntegerSet::clear");
   
   bits[index>>5] &= ~(1 << (index & 31));
}

inline void TSIntegerSet::set(S32 index)
{
   AssertFatal(index>=0 && index<MAX_TS_SET_SIZE,"TS::IntegerSet::set");
   
   bits[index>>5] |= 1 << (index & 31);
}

inline bool TSIntegerSet::test(S32 index) const
{
   AssertFatal(index>=0 && index<MAX_TS_SET_SIZE,"TS::IntegerSet::test");
   
   return bits[index>>5] & (1 << (index & 31));
}

#endif
