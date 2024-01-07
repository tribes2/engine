//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "platformWIN32/platformWin32.h"
#include "console/console.h"
#include "core/stringTable.h"
#include <math.h>

Platform::SystemInfo_struct Platform::SystemInfo;
extern void PlatformBlitInit();

void Processor::init()
{
   // Reference:
   //    www.cyrix.com
   //    www.amd.com
   //    www.intel.com
   //       http://developer.intel.com/design/PentiumII/manuals/24512701.pdf
   Platform::SystemInfo.processor.type = CPU_X86Compatible;
   Platform::SystemInfo.processor.name = StringTable->insert("Unknown x86 Compatible");
   Platform::SystemInfo.processor.mhz  = 0;
   Platform::SystemInfo.processor.properties = CPU_PROP_C;


#ifndef __BORLANDC__
   char     vendor[13] = {0,};
   U32   properties = 0;
   U32   processor  = 0;

   enum
   {
      BIT_FPU     = (1<<0),
      BIT_RDTSC   = (1<<4),  
      BIT_MMX     = (1<<23),  
      BIT_SSE     = (1<<25),  
      BIT_3DNOW   = (1<<31),  
   };

   __asm
   {
      //--------------------------------------
      // is CPUID supported
      push     ebx
      push     edx
      push     ecx
      pushfd      
      pushfd                     // save EFLAGS to stack
      pop      eax               // move EFLAGS into EAX
      mov      ebx, eax
      xor      eax, 0x200000     // flip bit 21
      push     eax
      popfd                      // restore EFLAGS
      pushfd
      pop      eax
      cmp      eax, ebx
      jz       EXIT              // doesn't support CPUID instruction
      
      //--------------------------------------
      // Get Vendor Informaion using CPUID eax==0
      xor      eax, eax
      cpuid                      

      mov      DWORD PTR vendor, ebx
      mov      DWORD PTR vendor+4, edx
      mov      DWORD PTR vendor+8, ecx

      // get Generic Extended CPUID info
      mov      eax, 1
      cpuid                      // eax=1, so cpuid queries feature information
     
      and      eax, 0x0ff0
      mov      processor, eax    // just store the model bits
      mov      properties, edx
      
      // Want to check for 3DNow(tm).  Need to see if extended cpuid functions present.
      mov      eax, 0x80000000
      cpuid
      cmp      eax, 0x80000000
      jbe      MAYBE_3DLATER
      mov      eax, 0x80000001
      cpuid
      and      edx, 0x80000000      // 3DNow if bit 31 set -> put bit in our properties
      or       properties, edx
   MAYBE_3DLATER:
      

   EXIT:
      popfd
      pop      ecx
      pop      edx
      pop      ebx
   }

   Platform::SystemInfo.processor.properties |= (properties & BIT_FPU)   ? CPU_PROP_FPU : 0;
   Platform::SystemInfo.processor.properties |= (properties & BIT_RDTSC) ? CPU_PROP_RDTSC : 0;
   Platform::SystemInfo.processor.properties |= (properties & BIT_MMX)   ? CPU_PROP_MMX : 0;

   //--------------------------------------
   if (dStricmp(vendor, "GenuineIntel") == 0)
   {
      Platform::SystemInfo.processor.properties |= (properties & BIT_SSE) ? CPU_PROP_SSE : 0;
      switch (processor & 0xf00)
      {
         case 0x600:
				switch(processor & 0xf0)
				{
					case 0x10:
						Platform::SystemInfo.processor.type = CPU_Intel_PentiumII;
						Platform::SystemInfo.processor.name = StringTable->insert( "Intel Pentium Pro" );
						break;
					case 0x30:
					case 0x50:
					case 0x60:
						Platform::SystemInfo.processor.type = CPU_Intel_PentiumII;
						Platform::SystemInfo.processor.name = StringTable->insert( "Intel Pentium II" );
						break;
					case 0x70:
					case 0x80:
					case 0xa0:
						Platform::SystemInfo.processor.type = CPU_Intel_PentiumIII;
						Platform::SystemInfo.processor.name = StringTable->insert( "Intel Pentium III" );
						break;
					default:
						Platform::SystemInfo.processor.type = CPU_Intel_PentiumII;
						Platform::SystemInfo.processor.name = StringTable->insert( "Intel (unknown, PPro family)" );
						break;
				}
				break;
         case 0x500:
            Platform::SystemInfo.processor.type = CPU_Intel_Pentium;
            Platform::SystemInfo.processor.name = StringTable->insert("Intel Pentium");
            break;
         default:
            Platform::SystemInfo.processor.type = CPU_Intel_Unknown;
            Platform::SystemInfo.processor.name = StringTable->insert("Intel (unknown)");
            break;
      }
   }
   //--------------------------------------
   else
   if (dStricmp(vendor, "AuthenticAMD") == 0)
   {
      Platform::SystemInfo.processor.properties |= (properties & BIT_3DNOW) ? CPU_PROP_3DNOW : 0;
      switch (processor)
      {
         case 0x700:
         case 0x710:
            Platform::SystemInfo.processor.type = CPU_AMD_K7;
            Platform::SystemInfo.processor.name = StringTable->insert("AMD K7");
            break;
         case 0x660:
         case 0x670:
            Platform::SystemInfo.processor.type = CPU_AMD_K6;
            Platform::SystemInfo.processor.name = StringTable->insert("AMD K6");
            break;
         case 0x680:
            Platform::SystemInfo.processor.type = CPU_AMD_K6_2;
            Platform::SystemInfo.processor.name = StringTable->insert("AMD K6-2");
            break;
         case 0x690:
            Platform::SystemInfo.processor.type = CPU_AMD_K6_3;
            Platform::SystemInfo.processor.name = StringTable->insert("AMD K6-3");
            break;
         case 0x510:
         case 0x520:
         case 0x530:
            Platform::SystemInfo.processor.type = CPU_AMD_K6_3;
            Platform::SystemInfo.processor.name = StringTable->insert("AMD K5");
            break;
         default:
            Platform::SystemInfo.processor.type = CPU_AMD_Unknown;
            Platform::SystemInfo.processor.name = StringTable->insert("AMD (unknown)");
            break;
      }
   }
   //--------------------------------------
   else
   if (dStricmp(vendor, "CyrixInstead") == 0)
   {
      switch (processor)
      {
         case 0x520:
            Platform::SystemInfo.processor.type = CPU_Cyrix_6x86;
            Platform::SystemInfo.processor.name = StringTable->insert("Cyrix 6x86");
            break;
         case 0x440:
            Platform::SystemInfo.processor.type = CPU_Cyrix_MediaGX;
            Platform::SystemInfo.processor.name = StringTable->insert("Cyrix Media GX");
            break;
         case 0x600:
            Platform::SystemInfo.processor.type = CPU_Cyrix_6x86MX;
            Platform::SystemInfo.processor.name = StringTable->insert("Cyrix 6x86mx/MII");
            break;
         case 0x540:
            Platform::SystemInfo.processor.type = CPU_Cyrix_GXm;
            Platform::SystemInfo.processor.name = StringTable->insert("Cyrix GXm");
            break;
         default:
            Platform::SystemInfo.processor.type = CPU_Cyrix_Unknown;
            Platform::SystemInfo.processor.name = StringTable->insert("Cyrix (unknown)");
            break;
      }
   }

   //--------------------------------------
   // if RDTSC support calculate the aproximate Mhz of the CPU
   if (Platform::SystemInfo.processor.properties & CPU_PROP_RDTSC && 
       Platform::SystemInfo.processor.properties & CPU_PROP_FPU)
   {
      const U32 MS_INTERVAL = 750;
      U32 time[2];      
      U32 ticks = 0;      
      __asm
      {
         push  eax
         push  edx
         //db    0fh, 31h
         rdtsc
         mov   DWORD PTR time, eax
         mov   DWORD PTR time+4, edx   
         pop   edx
         pop   eax
      }
      U32 ms = GetTickCount();
      while ( GetTickCount() < ms+MS_INTERVAL )
      { /* empty */ }
      ms = GetTickCount()-ms;
      __asm
      {
         push  eax
         push  edx
         //db    0fh, 31h
         rdtsc
         sub   edx, DWORD PTR time+4
         sbb   eax, DWORD PTR time
         mov   DWORD PTR ticks, eax
         pop   edx
         pop   eax
      }
      U32 mhz = F32(ticks) / F32(ms) / 1000.0f;

      // catch-22 the timing method used above to calc Mhz is generally
      // wrong by a few percent so we want to round to the nearest clock
      // multiple but we also want to be careful to not touch overclocked results

      // measure how close the Raw Mhz number is to the center of each clock bucket
      U32 bucket25 = mhz % 25;
      U32 bucket33 = mhz % 33;
      U32 bucket50 = mhz % 50;

      if (bucket50 < 8 || bucket50 > 42)
         Platform::SystemInfo.processor.mhz = U32((mhz+(50.0f/2.0f))/50.0f) * 50; 
      else if (bucket25 < 5 || bucket25 > 20)
         Platform::SystemInfo.processor.mhz = U32((mhz+(25.0f/2.0f))/25.0f) * 25; 
      else if (bucket33 < 5 || bucket33 > 28)
         Platform::SystemInfo.processor.mhz = U32((mhz+(33.0f/2.0f))/33.0f) * 33; 
      else 
         Platform::SystemInfo.processor.mhz = U32(mhz); 
   }
#endif

   Con::printf("Processor Init:");
   Con::printf("   %s, %d Mhz", Platform::SystemInfo.processor.name, Platform::SystemInfo.processor.mhz);
   if (Platform::SystemInfo.processor.properties & CPU_PROP_FPU)
      Con::printf("   FPU detected");
   if (Platform::SystemInfo.processor.properties & CPU_PROP_MMX)
      Con::printf("   MMX detected");
   if (Platform::SystemInfo.processor.properties & CPU_PROP_3DNOW)
      Con::printf("   3DNow detected");
   if (Platform::SystemInfo.processor.properties & CPU_PROP_SSE)
      Con::printf("   SSE detected");
   Con::printf(" ");

   PlatformBlitInit();
}
