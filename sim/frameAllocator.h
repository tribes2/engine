//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _FRAMEALLOCATOR_H_
#define _FRAMEALLOCATOR_H_

#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif

class FrameAllocator
{
   static U8*   smBuffer;
   static U32   smHighWaterMark;
   static U32   smWaterMark;


  public:
   static void init(const U32 frameSize);
   static void destroy();

   static void* alloc(const U32 allocSize);

   static void setWaterMark(const U32);
   static U32  getWaterMark();
   static U32  getHighWaterMark();
};

#endif  // _H_FRAMEALLOCATOR_
