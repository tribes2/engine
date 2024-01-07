//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _CRYPTRANDPOOL_H_
#define _CRYPTRANDPOOL_H_

#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif

class BitStream;

class CryptRandomPool
{
   U8         mPool[55];
   U32        mPoolPos;

   U8         mRandomBytes[20];
   U32        mAvailableBytes;

   void churnPool();

   CryptRandomPool();
   ~CryptRandomPool();
  public:

   static void init();
   static void destroy();

   static void submitEntropy(U32 value, U32 entropicBits);
   static void extractRandomBytes(U8* pOutput,
                                  U32 bytesNeeded);
};

#endif  // _H_CRYPTRANDPOOL_
