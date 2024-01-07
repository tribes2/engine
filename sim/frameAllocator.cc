//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "Sim/frameAllocator.h"
#include "console/console.h"

U8*   FrameAllocator::smBuffer = NULL;
U32   FrameAllocator::smWaterMark = 0;
U32   FrameAllocator::smHighWaterMark = 0;

#ifdef DEBUG
S32 sgMaxFrameAllocation = 0;

ConsoleFunction(getMaxFrameAllocation, S32, 1,1, "getMaxFrameAllocation();")
{
   argc, argv;
   return sgMaxFrameAllocation;
}

#endif

void FrameAllocator::init(const U32 frameSize)
{
   AssertFatal(smBuffer == NULL, "Error, already initialized");
   smBuffer = new U8[frameSize];
   smWaterMark = 0;
   smHighWaterMark = frameSize;
}

void FrameAllocator::destroy()
{
   AssertFatal(smBuffer != NULL, "Error, not initialized");
   
   delete [] smBuffer;
   smBuffer = NULL;
   smWaterMark = 0;
   smHighWaterMark = 0;
}


void* FrameAllocator::alloc(const U32 allocSize)
{
   AssertFatal(smBuffer != NULL, "Error, no buffer!");
   AssertFatal(smWaterMark + allocSize <= smHighWaterMark, "Error alloc too large, increase frame size!");

   U8* p = &smBuffer[smWaterMark];
   smWaterMark += allocSize;

#ifdef DEBUG
   if (smWaterMark > sgMaxFrameAllocation)
      sgMaxFrameAllocation = smWaterMark;
#endif
   
   return p;
}


void FrameAllocator::setWaterMark(const U32 waterMark)
{
   AssertFatal(waterMark < smHighWaterMark, "Error, invalid waterMark");

   smWaterMark = waterMark;
}

U32 FrameAllocator::getWaterMark()
{
   return smWaterMark;
}

U32 FrameAllocator::getHighWaterMark()
{
   return smHighWaterMark;
}



