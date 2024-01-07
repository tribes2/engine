//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _CRYPTSHA1_H_
#define _CRYPTSHA1_H_

#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif

class SHA1Context
{
   static const U32 csmHashInitialValues[5];
   static const U32 csmRoundConstants[4];

   U32 mHashVals[5];

   U32  mBytesWritten;
   bool mInitialized;
   bool mHashValid;

   U8  mBuffer[64];
   U32 mBufferLen;

  private:
   void hashBlock(const void*);
   U32  sha1Nonlinear(U32, U32, U32, U32);

  public:
   SHA1Context();
   ~SHA1Context();

   static const U32 csmHashLenBits;
   static const U32 csmHashLenBytes;
   
   void init();
   void hashBytes(const void* input, const U32 inputLen);
   void finalize();

   bool getHash(U8* pHash);
};

#endif  // _H_CRYPTSHA1_
