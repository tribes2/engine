//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "crypt/cryptSHA1.h"


namespace {

inline U32 rollLeft(U32 val, U32 rol)
{
   return (val << rol) | (val >> (32 - rol));
}

} // namespace {}


//--------------------------------------------------------------------------
//-------------------------------------- SHA1Context
//
const U32 SHA1Context::csmHashInitialValues[5] = { 0x67452301,
                                                   0xEFCDAB89,
                                                   0x98BADCFE,
                                                   0x10325476,
                                                   0xC3D2E1F0 };

const U32 SHA1Context::csmRoundConstants[4] = { 0x5A827999,
                                                0x6ED9EBA1,
                                                0x8F1BBCDC,
                                                0xCA62C1D6 };
const U32 SHA1Context::csmHashLenBits  = 160;
const U32 SHA1Context::csmHashLenBytes = 20;

SHA1Context::SHA1Context()
{
   U32  mBytesWritten = 0;
   bool mInitialized  = false;
   bool mHashValid    = false;

   U32 mBufferLen     = 0;
}

SHA1Context::~SHA1Context()
{
   U32  mBytesWritten = 0;
   bool mInitialized  = false;
   bool mHashValid    = false;

   U32 mBufferLen     = 0;
}

void SHA1Context::init()
{
   for (U32 i = 0; i < 5; i++)
      mHashVals[i] = csmHashInitialValues[i];

   mBufferLen    = 0;
   mBytesWritten = 0;
   mInitialized  = true;
   mHashValid    = false;
}

void SHA1Context::hashBytes(const void* input, const U32 inputLen)
{
   AssertFatal(input != NULL, "Error, invalid input pointer");
   AssertFatal(mInitialized == true, "Error, SHA1Context not initialized.  Must init() before writing bytes");
   AssertFatal(mHashValid == false, "Error, SHA1Context already finalized.  Must reinit() before writing more bytes");

   if (inputLen == 0)
      return;

   const U8* pByteInput = reinterpret_cast<const U8*>(input);
   U32 currInputPos = 0;

   if (mBufferLen != 0) {
      // Copy out enough bytes to finish off the buffer, if enough exist
      while (mBufferLen < 64 && currInputPos < inputLen) {
         mBuffer[mBufferLen++] = pByteInput[currInputPos++];
         mBytesWritten++;
      }

      if (mBufferLen == 64) {
         hashBlock(mBuffer);
         mBufferLen = 0;
      }
   }
   AssertFatal(mBufferLen == 0 || currInputPos == inputLen, "Hm, something goofed");

   if (currInputPos < inputLen) {
      // More bytes to hash...
      U32 bytesLeft = inputLen - currInputPos;

      while (bytesLeft >= 64) {
         hashBlock(&pByteInput[currInputPos]);
         currInputPos  += 64;
         bytesLeft     -= 64;
         mBytesWritten += 64;
      }

      if (bytesLeft != 0) {
         dMemcpy(mBuffer, &pByteInput[currInputPos], bytesLeft);
         mBytesWritten += bytesLeft;
         mBufferLen     = bytesLeft;
      }
   }
}

void SHA1Context::finalize()
{
   // We assume that bytes written is less than 256 megs.  Seems safe for now.
   AssertFatal(mInitialized == true, "Error, hash context not initialized.");
   AssertFatal(mBytesWritten < (1 << 27), "Error, too many bytes written to SHAContext");
   AssertFatal(mBufferLen < 64, "Error, unflushed buffer, or invalid bufferlen");

   if (mBufferLen < (64 - 8 - 1)) {
      // We have enough room in this buffer for the padding structure...
      //
      mBuffer[mBufferLen++] = 0x80;
      for (U32 i = mBufferLen; i < (64 - 8); i++)
         mBuffer[mBufferLen++] = 0x0;
   } else {
      // We have to create a new block to finalize this one...

      mBuffer[mBufferLen++] = 0x80;
      for (U32 i = mBufferLen; i < 64; i++)
         mBuffer[i] = 0x00;

      hashBlock(mBuffer);
      mBufferLen = 0;

      for (U32 i = 0; i < (64 - 8); i++)
         mBuffer[mBufferLen++] = 0x00;
   }
   AssertFatal(mBufferLen == (64 - 8), "Error in one of the above loops");

   U32* pLenVals = reinterpret_cast<U32*>(&mBuffer[(64 - 8)]);
   pLenVals[0] = 0;
   pLenVals[1] = convertHostToBEndian(mBytesWritten * 8);

   hashBlock(mBuffer);
   mBufferLen    = 0;
   mHashValid    = true;
   mBytesWritten = 0;
}

bool SHA1Context::getHash(U8* pHash)
{
   if (mHashValid == true) {
      U32* pWordHash = reinterpret_cast<U32*>(pHash);

      pWordHash[0] = convertHostToBEndian(mHashVals[0]);
      pWordHash[1] = convertHostToBEndian(mHashVals[1]);
      pWordHash[2] = convertHostToBEndian(mHashVals[2]);
      pWordHash[3] = convertHostToBEndian(mHashVals[3]);
      pWordHash[4] = convertHostToBEndian(mHashVals[4]);

      return true;
   }

   return false;
}


//--------------------------------------------------------------------------
//-------------------------------------- Note that this isn't the most optimal
//                                        implementation, but it's easy, and since
//                                        we're not overly concerned with speed
//                                        (right now), we're fine
//
inline U32 SHA1Context::sha1Nonlinear(U32 x, U32 y, U32 z, U32 t)
{
   AssertFatal(t < 80, "Error, invalid round");

   if (t <= 19) {
      return ((x & y) | ((~x) & z)) + csmRoundConstants[0];
   } else if (t <= 39) {
      return (x ^ y ^ z) + csmRoundConstants[1];
   } else if (t <= 59) {
      return ((x & y) | (x & z) | (y & z)) + csmRoundConstants[2];
   } else {
      return (x ^ y ^ z) + csmRoundConstants[3];
   }
}

void SHA1Context::hashBlock(const void* pInput)
{
   const U32* pWordInput = reinterpret_cast<const U32*>(pInput);

   // First expand the message to 80 words
   //
   U32 expandedBlock[80];
   for (U32 i = 0; i < 16; i++)
      expandedBlock[i] = convertBEndianToHost(pWordInput[i]);
   for (U32 i = 16; i < 80; i++)
      expandedBlock[i] = rollLeft(expandedBlock[i - 3]  ^ expandedBlock[i - 8] ^
                                  expandedBlock[i - 14] ^ expandedBlock[i - 16], 1);

   register U32 a = mHashVals[0];
   register U32 b = mHashVals[1];
   register U32 c = mHashVals[2];
   register U32 d = mHashVals[3];
   register U32 e = mHashVals[4];
   U32 tmp;   

   for (U32 i = 0; i < 80; i++) {
      // In this implementation, the round constants are in the nonlinear function
      //
      tmp = rollLeft(a, 5)            +
            sha1Nonlinear(b, c, d, i) +
            e                         +
            expandedBlock[i];

      e = d;
      d = c;
      c = rollLeft(b, 30);
      b = a;
      a = tmp;
   }

   mHashVals[0] += a;
   mHashVals[1] += b;
   mHashVals[2] += c;
   mHashVals[3] += d;
   mHashVals[4] += e;
}

