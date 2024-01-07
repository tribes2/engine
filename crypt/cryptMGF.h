//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _CRYPTMGF_H_
#define _CRYPTMGF_H_

#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif

bool MGF1(const U8* seed, const U32 seedLen,
          U8*       mask, const U32 maskLen);

#endif  // _H_CRYPTMGF_
