//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _BITMATRIX_H_
#define _BITMATRIX_H_

#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif
#ifndef _BITVECTOR_H_
#include "Core/bitVector.h"
#endif

class BitMatrix
{
   U32 mWidth;
   U32 mHeight;
   U32 mRowByteWidth;

   U8* mBits;
   U32 mSize;

   BitVector mColFlags;
   BitVector mRowFlags;

  public:
   BitMatrix(const U32 width, const U32 height);
   ~BitMatrix();

   void clearAllBits();
   void setAllBits();

   void setBit(const U32 x, const U32 y);
   void clearBit(const U32 x, const U32 y);

   // Queries
   bool isSet(const U32 x, const U32 y) const;

   bool isAnySetCol(const U32 x);
   bool isAnySetRow(const U32 y);
};

inline BitMatrix::BitMatrix(const U32 width, const U32 height)
 : mColFlags(width),
   mRowFlags(height)
{
   AssertFatal(width != 0 && height != 0, "Error, w/h must be non-zero");

   mWidth        = width;
   mHeight       = height;
   mRowByteWidth = (width + 7) >> 3;

   mSize         = mRowByteWidth * mHeight;
   mBits         = new U8[mSize];
}

inline BitMatrix::~BitMatrix()
{
   mWidth        = 0;
   mHeight       = 0;
   mRowByteWidth = 0;
   mSize         = 0;

   delete [] mBits;
   mBits = NULL;
}

inline void BitMatrix::clearAllBits()
{
   AssertFatal(mBits != NULL, "Error, clearing after deletion");

   dMemset(mBits, 0x00, mSize);
   mColFlags.clear();
   mRowFlags.clear();
}

inline void BitMatrix::setAllBits()
{
   AssertFatal(mBits != NULL, "Error, setting after deletion");

   dMemset(mBits, 0xFF, mSize);
   mColFlags.set();
   mRowFlags.set();
}

inline void BitMatrix::setBit(const U32 x, const U32 y)
{
   AssertFatal(x < mWidth && y < mHeight, "Error, out of bounds bit!");

   U8* pRow = &mBits[y * mRowByteWidth];

   U8* pByte = &pRow[x >> 3];
   *pByte   |= 1 << (x & 0x7);

   mColFlags.set(x);
   mRowFlags.set(y);
}

inline void BitMatrix::clearBit(const U32 x, const U32 y)
{
   AssertFatal(x < mWidth && y < mHeight, "Error, out of bounds bit!");

   U8* pRow = &mBits[y * mRowByteWidth];

   U8* pByte = &pRow[x >> 3];
   *pByte   &= ~(1 << (x & 0x7));
}

inline bool BitMatrix::isSet(const U32 x, const U32 y) const
{
   AssertFatal(x < mWidth && y < mHeight, "Error, out of bounds bit!");

   U8* pRow = &mBits[y * mRowByteWidth];

   U8* pByte = &pRow[x >> 3];
   return (*pByte & (1 << (x & 0x7))) != 0;
}

inline bool BitMatrix::isAnySetCol(const U32 x)
{
   AssertFatal(x < mWidth, "Error, out of bounds column!");

   return mColFlags.test(x);
}

inline bool BitMatrix::isAnySetRow(const U32 y)
{
   AssertFatal(y < mHeight, "Error, out of bounds row!");

   return mRowFlags.test(y);
}

#endif  // _H_BITMATRIX_
