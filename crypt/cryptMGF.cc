//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "crypt/cryptMGF.h"
#include "crypt/cryptSHA1.h"

bool MGF1(const U8* seed, const U32 seedLen,
          U8*       mask, const U32 maskLen)
{
   AssertFatal(seed != NULL, "Error, no seed!");
   AssertFatal(mask != NULL && maskLen != 0, "No mask pointer or maskLen == 0");

   U32 upper = (maskLen + SHA1Context::csmHashLenBytes - 1) / SHA1Context::csmHashLenBytes;
   U8* maskBuffer = new U8[upper * SHA1Context::csmHashLenBytes];

   U8* catBuffer  = new U8[seedLen + 4];
   dMemcpy(catBuffer, seed, seedLen);

   SHA1Context hashContext;
   for (U32 i = 0; i < upper; i++) {
      catBuffer[seedLen + 0] = (i >> 24) & 0xFF;
      catBuffer[seedLen + 1] = (i >> 16) & 0xFF;
      catBuffer[seedLen + 2] = (i >>  8) & 0xFF;
      catBuffer[seedLen + 3] = (i >>  0) & 0xFF;

      hashContext.init();
      hashContext.hashBytes(catBuffer, seedLen + 4);
      hashContext.finalize();

      hashContext.getHash(maskBuffer + (SHA1Context::csmHashLenBytes * i));
   }

   dMemcpy(mask, maskBuffer, maskLen);

   delete [] catBuffer;
   delete [] maskBuffer;

   return true;
}

