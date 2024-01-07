//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _REALCOMP_H_
#define _REALCOMP_H_

//Includes
#ifndef _MMATHFN_H_
#include "Math/mMathFn.h"
#endif
#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif

inline bool isEqual(F32 a, F32 b)
{
   return mFabs(a - b) < __EQUAL_CONST_F;
}

inline bool isZero(F32 a)
{
   return mFabs(a) < __EQUAL_CONST_F;
}

#endif //_REALCOMP_H_
