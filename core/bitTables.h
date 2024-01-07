//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _BITTABLES_H_
#define _BITTABLES_H_

#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif

class BitTables
{
  private:
   static bool mTablesBuilt;        // For first time build
   static S8   mHighBit[256];       // I.e. crude logarithm
   static S8   mWhichOn[256][8];    // Unroll a bitset collection (note 
   static S8   mNumOn[256];         //    ptr table wastes same amt.)  
      
  public:
   BitTables();
   static const S32   numOn(U8 b)       {  return mNumOn[b];      }
   static const S8 *  whichOn(U8 b)     {  return mWhichOn[b];    }
   static const S32   highBit(U8 b)     {  return mHighBit[b];    }

   static S32 getPower16(U16 x)   { return x<256 ? mHighBit[x] : mHighBit[x>>8]+8; }
   static S32 getPower32(U32 x){
                  if( x < (1<<16) )
                     return( x < (1<<8)  ? mHighBit[x] : mHighBit[x>>8]+8 );
                  else
                     return( x < (1<<24) ? mHighBit[x>>16]+16 : mHighBit[x>>24]+24 );
                 }
};

#endif
