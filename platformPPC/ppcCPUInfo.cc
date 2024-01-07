//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "PlatformPPC/platformPPC.h"
#include "console/console.h"
#include "Core/stringTable.h"
#include <math.h>

Platform::SystemInfo_struct Platform::SystemInfo;


void Processor::init()
{
   // Reference:
   //    www.cyrix.com
   //    www.amd.com
   //    www.intel.com
   //       http://developer.intel.com/design/PentiumII/manuals/24512701.pdf
   Platform::SystemInfo.processor.type = CPU_PowerPC_G3;
   Platform::SystemInfo.processor.name = StringTable->insert("Unknown PPC Compatible");
   Platform::SystemInfo.processor.mhz  = 0;
   Platform::SystemInfo.processor.properties = CPU_PROP_C;


   Con::printf("Processor Init:");
   Con::printf("   %s, %d Mhz", Platform::SystemInfo.processor.name, Platform::SystemInfo.processor.mhz);
   if (Platform::SystemInfo.processor.properties & CPU_PROP_FPU)
      Con::printf("   FPU detected");
   Con::printf(" ");
}
