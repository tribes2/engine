//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include <math.h>

#include "Platform/platform.h"
#include "platformLinux/platformLinux.h"
#include "console/console.h"
#include "Core/stringTable.h"

extern "C" cpu_init_ASM( char* vendor, unsigned int* processor, unsigned int* properties );

Platform::SystemInfo_struct Platform::SystemInfo;

void Processor::init( void )
{
	Platform::SystemInfo.processor.type = CPU_X86Compatible;
	Platform::SystemInfo.processor.name = StringTable->insert( "Unknown x86 Compatible" );
	Platform::SystemInfo.processor.mhz = 0;
	Platform::SystemInfo.processor.properties = CPU_PROP_C;

	char vendor[13];
	U32 properties = 0;
	U32 processor = 0;

	memset( vendor, 0, 13 );

	enum {
		BIT_FPU = 1,
		BIT_RDTSC = 1 << 4,
		BIT_MMX = 1 << 23,
		BIT_SSE = 1 << 25,
		BIT_3DNOW = 1 << 31
	};

	// when this code was previously inline, gcc would
	// mysteriously optimize away the stores to properties, etc.
	cpu_init_ASM( vendor, &processor, &properties );

	Platform::SystemInfo.processor.properties |= ( properties & BIT_FPU ) ? CPU_PROP_FPU : 0;
	Platform::SystemInfo.processor.properties |= ( properties & BIT_RDTSC ) ? CPU_PROP_RDTSC : 0;
	Platform::SystemInfo.processor.properties |= ( properties & BIT_MMX ) ? CPU_PROP_MMX : 0;

	if( dStricmp( vendor, "GenuineIntel" ) == 0 ) {

		Platform::SystemInfo.processor.properties |= ( properties & BIT_SSE ) ? CPU_PROP_SSE : 0;

		switch( processor & 0xf00 ) {
		case 0x600:

			switch( processor & 0xf0 ) {
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
			Platform::SystemInfo.processor.name = StringTable->insert( "Intel Pentium" );
			break;
		default:
			Platform::SystemInfo.processor.type = CPU_Intel_Unknown;
			Platform::SystemInfo.processor.name = StringTable->insert( "Intel (unknown)" );
			break;
		}

	} else if( dStricmp( vendor, "AuthenticAMD" ) == 0 ) {
		Platform::SystemInfo.processor.properties |= ( properties & BIT_3DNOW ) ? CPU_PROP_3DNOW : 0;

		switch( processor ) {
		case 0x700:
		case 0x710:
			Platform::SystemInfo.processor.type = CPU_AMD_K7;
			Platform::SystemInfo.processor.name = StringTable->insert( "AMD K7" );
			break;
		case 0x660:
		case 0x670:
			Platform::SystemInfo.processor.type = CPU_AMD_K6;
			Platform::SystemInfo.processor.name = StringTable->insert( "AMD K6" );
			break;
		case 0x680:
			Platform::SystemInfo.processor.type = CPU_AMD_K6_2;
			Platform::SystemInfo.processor.name = StringTable->insert( "AMD K6-2" );
			break;
		case 0x690:
			Platform::SystemInfo.processor.type = CPU_AMD_K6_3;
			Platform::SystemInfo.processor.name = StringTable->insert( "AMD K6-3" );
			break;
		case 0x510:
		case 0x520:
		case 0x530:
			Platform::SystemInfo.processor.type = CPU_AMD_K6_3;
			Platform::SystemInfo.processor.name = StringTable->insert( "AMD K5" );
			break;
		default:
			Platform::SystemInfo.processor.type = CPU_AMD_Unknown;
			Platform::SystemInfo.processor.name = StringTable->insert( "AMD (unknown)" );
			break;
		}

	} else if( dStricmp( vendor, "CyrixInstead" ) == 0 ) {

		switch( processor ) {
		case 0x520:
			Platform::SystemInfo.processor.type = CPU_Cyrix_6x86;
			Platform::SystemInfo.processor.name = StringTable->insert( "Cyrix 6x86" );
			break;
		case 0x440:
			Platform::SystemInfo.processor.type = CPU_Cyrix_MediaGX;
			Platform::SystemInfo.processor.name = StringTable->insert( "Cyrix Media GX" );
			break;
		case 0x600:
			Platform::SystemInfo.processor.type = CPU_Cyrix_6x86MX;
			Platform::SystemInfo.processor.name = StringTable->insert( "Cyrix 6x86mx/MII" );
			break;
		case 0x540:
			Platform::SystemInfo.processor.type = CPU_Cyrix_GXm;
			Platform::SystemInfo.processor.name = StringTable->insert( "Cyrix GXm" );
			break;
		default:
			Platform::SystemInfo.processor.type = CPU_Cyrix_Unknown;
			Platform::SystemInfo.processor.name = StringTable->insert( "Cyrix (unknown)" );
			break;
		}

	}

	if( Platform::SystemInfo.processor.properties & CPU_PROP_RDTSC &&
	    Platform::SystemInfo.processor.properties & CPU_PROP_FPU ) {
		const U32 MS_INTERVAL = 750;
		U32 timing[2];
		U32 ticks = 0;

		__asm__ __volatile__( "pushl %%eax                    \n\t"
				      "pushl %%edx                    \n\t"
				      "rdtsc                          \n\t"
				      "movl %%eax, (%0)               \n\t"
				      "movl %%edx, 4(%0)              \n\t"
				      "popl %%edx                     \n\t"
				      "popl %%eax                     \n\t"
				      : /* no outputs */
				      : "r" (timing)
				      : "eax", "edx", "cc", "memory" );

		U32 ms = Platform::getRealMilliseconds( );

		while( Platform::getRealMilliseconds ( ) < ms + MS_INTERVAL ) {
			// empty
		}

		ms = Platform::getRealMilliseconds( ) - ms;

		__asm__ __volatile__( "pushl %%eax                    \n\t"
				      "pushl %%edx                    \n\t"
				      "rdtsc                          \n\t"
				      "subl 4(%1), %%edx              \n\t"
				      "sbbl (%1), %%eax               \n\t"
				      "movl %%eax, %0                 \n\t"
				      "popl %%edx                     \n\t"
				      "popl %%eax                     \n\t"
				      : "=g" (ticks)
				      : "r" (timing)
				      : "eax", "edx", "cc", "memory" );

		U32 mhz = static_cast<F32>( ticks ) / static_cast<F32>( ms ) / 1000.0f;

		U32 bucket25 = mhz % 25;
		U32 bucket33 = mhz % 33;
		U32 bucket50 = mhz % 50;

		if( bucket50 < 8 || bucket50 > 42 ) {
			Platform::SystemInfo.processor.mhz = static_cast<U32>( ( mhz + ( 50.0f / 2.0f ) ) / 50.0f ) * 50; 
		} else if( bucket25 < 5 || bucket25 > 20 ) {
			Platform::SystemInfo.processor.mhz = static_cast<U32>( ( mhz + ( 25.0f / 2.0f ) ) / 25.0f ) * 25; 
		} else if( bucket33 < 5 || bucket33 > 28 ) {
			Platform::SystemInfo.processor.mhz = static_cast<U32>( ( mhz + ( 33.0f / 2.0f ) ) / 33.0f ) * 33; 
		} else {
			Platform::SystemInfo.processor.mhz = static_cast<U32>( mhz );
		}

	}

	Con::printf( "Processor Init:" );
	Con::printf( "    %s, %d Mhz",
		     Platform::SystemInfo.processor.name,
		     Platform::SystemInfo.processor.mhz );

	if( Platform::SystemInfo.processor.properties & CPU_PROP_FPU ) {
		Con::printf( "    FPU detected" );
	}

	if( Platform::SystemInfo.processor.properties & CPU_PROP_MMX ) {
		Con::printf( "    MMX detected" );
	}

	if( Platform::SystemInfo.processor.properties & CPU_PROP_3DNOW ) {
		Con::printf( "    3DNow detected" );
	}

	if( Platform::SystemInfo.processor.properties & CPU_PROP_SSE ) {
		Con::printf( "    SSE detected" );
	}

	PlatformBlitInit( );
}
