//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _BITSET_H_
#define _BITSET_H_

//Includes
#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif

class BitSet32 
{
private:
   U32 mbits;

public:
   BitSet32()                         { mbits = 0; }
   BitSet32(const BitSet32& in_rCopy) { mbits = in_rCopy.mbits; }
   BitSet32(const U32 in_mask)        { mbits = in_mask; }

   operator U32() const               { return mbits; }
   U32 getMask() const                { return mbits; }
   
   void set()                         { mbits  = 0xFFFFFFFFUL; }
   void set(const U32 m)              { mbits |= m; }
   void set(BitSet32 s, bool b)       { mbits = (mbits&~(s.mbits))|(b?s.mbits:0); }
   
   void clear()                       { mbits  = 0; }
   void clear(const U32 m)            { mbits &= ~m; }

   void toggle(const U32 m)           { mbits ^= m; }

   bool test(const U32 m) const       { return (mbits & m) != 0; }
   bool testStrict(const U32 m) const { return (mbits & m) == m; }

   BitSet32& operator =(const U32 m)  { mbits  = m;  return *this; }
   BitSet32& operator|=(const U32 m)  { mbits |= m; return *this; }
   BitSet32& operator&=(const U32 m)  { mbits &= m; return *this; }
   BitSet32& operator^=(const U32 m)  { mbits ^= m; return *this; }

   BitSet32 operator|(const U32 m) const { return BitSet32(mbits | m); }
   BitSet32 operator&(const U32 m) const { return BitSet32(mbits & m); }
   BitSet32 operator^(const U32 m) const { return BitSet32(mbits ^ m); }
};


#endif //_NBITSET_H_
