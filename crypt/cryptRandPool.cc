//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "crypt/cryptRandPool.h"
#include "Core/bitStream.h"
#include "crypt/cryptSHA1.h"

namespace {

CryptRandomPool* sgRandPool = NULL;

} // namespace {}


void CryptRandomPool::init()
{
   AssertFatal(sgRandPool == NULL, "RandPool already initialized");

   sgRandPool = new CryptRandomPool;
}

void CryptRandomPool::destroy()
{
   AssertFatal(sgRandPool != NULL, "RandPool not initialized");
   delete sgRandPool;
   sgRandPool = NULL;
}


CryptRandomPool::CryptRandomPool()
{
   mPoolPos        = 0;
   mAvailableBytes = 0;
}

CryptRandomPool::~CryptRandomPool()
{

}

void CryptRandomPool::submitEntropy(U32 value, U32 entropicBits)
{
   AssertFatal(sgRandPool != NULL, "No Random pool!");

   for (U32 i = 0; i < entropicBits; i++) {
      U8 bit = value & 0x1;
      value >>= 1;

      // Write this bit to the current pool pos
      U32 byte = sgRandPool->mPoolPos / 8;
      U32 mask = sgRandPool->mPoolPos % 8;
      sgRandPool->mPool[byte] ^= bit << mask;
      sgRandPool->mPoolPos = (sgRandPool->mPoolPos + 1) % (55 * 8);
   }
}

void CryptRandomPool::extractRandomBytes(U8* pOutput, U32 bytesNeeded)
{
   AssertFatal(sgRandPool != NULL, "No Random pool!");

   U32 currOutput = 0;

   while (sgRandPool->mAvailableBytes != 0 && bytesNeeded != 0) {
      sgRandPool->mAvailableBytes--;
      bytesNeeded--;
      pOutput[currOutput++] = sgRandPool->mRandomBytes[sgRandPool->mAvailableBytes];
   }

   while (bytesNeeded != 0) {
      sgRandPool->churnPool();
      
      while (sgRandPool->mAvailableBytes != 0 && bytesNeeded != 0) {
         sgRandPool->mAvailableBytes--;
         bytesNeeded--;
         pOutput[currOutput++] = sgRandPool->mRandomBytes[sgRandPool->mAvailableBytes];
      }
   }
}


void CryptRandomPool::churnPool()
{
   SHA1Context hashCTX;
   hashCTX.init();
   hashCTX.hashBytes(mPool, 55);
   hashCTX.finalize();

   hashCTX.getHash(mRandomBytes);
   mAvailableBytes = 20;

   const U32* pChurn = (const U32*)mRandomBytes;
   submitEntropy(pChurn[0], 32);
   submitEntropy(pChurn[1], 32);
   submitEntropy(pChurn[2], 32);
   submitEntropy(pChurn[3], 32);
   submitEntropy(pChurn[4], 32);
   
}
