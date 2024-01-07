//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "PlatformMacCarb/platformMacCarb.h"
#include "console/console.h"
#include "Core/stringTable.h"
#include <math.h>

#include <Gestalt.h>
// possibly useful gestalts:
//gestaltCarbonVersion

//    gestaltNativeCPUtype        = FOUR_CHAR_CODE('cput'),       /* Native CPU type                          */
//    gestaltNativeCPUfamily      = FOUR_CHAR_CODE('cpuf'),       /* Native CPU family                      */
//    gestaltCPU601               = 0x0101,                       /* IBM 601                               */
//    gestaltCPU603               = 0x0103,
//    gestaltCPU604               = 0x0104,
//    gestaltCPU603e              = 0x0106,
//    gestaltCPU603ev             = 0x0107,
//    gestaltCPU750               = 0x0108,                       /* Also 740 - "G3" */
//    gestaltCPU604e              = 0x0109,
//    gestaltCPU604ev             = 0x010A                        /* Mach 5, 250Mhz and up */

//    gestaltDisplayMgrVers       = FOUR_CHAR_CODE('dplv')        /* Display Manager version */
//    gestaltDisplayMgrAttr       = FOUR_CHAR_CODE('dply'),       /* Display Manager attributes */

//    gestaltLogicalRAMSize       = FOUR_CHAR_CODE('lram')        /* logical ram size -- can be lower than physram... */
//    gestaltPhysicalRAMSize      = FOUR_CHAR_CODE('ram ')        /* physical RAM size */

//    gestaltOSAttr               = FOUR_CHAR_CODE('os  '),       /* o/s attributes */
//    gestaltSysZoneGrowable      = 0,                            /* system heap is growable */
//    gestaltLaunchCanReturn      = 1,                            /* can return from launch */
//    gestaltLaunchFullFileSpec   = 2,                            /* can launch from full file spec */
//    gestaltLaunchControl        = 3,                            /* launch control support available */
//    gestaltTempMemSupport       = 4,                            /* temp memory support */
//    gestaltRealTempMemory       = 5,                            /* temp memory handles are real */
//    gestaltTempMemTracked       = 6,                            /* temporary memory handles are tracked */
//    gestaltIPCSupport           = 7,                            /* IPC support is present */
//    gestaltSysDebuggerSupport   = 8                             /* system debugger support is present */

//    gestaltPowerMgrAttr         = FOUR_CHAR_CODE('powr'),       /* power manager attributes */
//    gestaltPMgrExists           = 0,
//    gestaltPMgrCPUIdle          = 1,
//    gestaltPMgrSCC              = 2,
//    gestaltPMgrSound            = 3,
//    gestaltPMgrDispatchExists   = 4,
//    gestaltPMgrSupportsAVPowerStateAtSleepWake = 5

//    gestaltPowerPCProcessorFeatures = FOUR_CHAR_CODE('ppcf'),   /* Optional PowerPC processor features */
//    gestaltPowerPCHasGraphicsInstructions = 0,                  /* has fres, frsqrte, and fsel instructions */
//    gestaltPowerPCHasSTFIWXInstruction = 1,                     /* has stfiwx instruction */
//    gestaltPowerPCHasSquareRootInstructions = 2,                /* has fsqrt and fsqrts instructions */
//    gestaltPowerPCHasDCBAInstruction = 3,                       /* has dcba instruction */
//    gestaltPowerPCHasVectorInstructions = 4,                    /* has vector instructions */
//    gestaltPowerPCHasDataStreams = 5                            /* has dst, dstt, dstst, dss, and dssall instructions */

// should add multiprocessing check
// should add threadmgr check -- and we'll need optional code to do something diff.

Platform::SystemInfo_struct Platform::SystemInfo;

void Processor::init()
{
#pragma message("we need code to determine processor speed and specific processor class")
   Platform::SystemInfo.processor.type = CPU_PowerPC_G3;
   Platform::SystemInfo.processor.name = StringTable->insert("Unknown PowerPC");
   Platform::SystemInfo.processor.mhz  = 200; //!!!!!!!TBD - safe min value.
   Platform::SystemInfo.processor.properties = CPU_PROP_PPCMIN;

   Con::printf("Processor Init:");
   Con::printf("   %s, %d Mhz", Platform::SystemInfo.processor.name, Platform::SystemInfo.processor.mhz);
   if (Platform::SystemInfo.processor.properties & CPU_PROP_PPCMIN)
      Con::printf("   FPU detected");
   Con::printf(" ");
}
