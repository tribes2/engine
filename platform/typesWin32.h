//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _TYPESWIN32_H_
#define _TYPESWIN32_H_

// We have to check this.  Since every file will eventually wind up including
//  this header, but not every header includes a windows or system header...
//
#ifndef NULL
#define NULL 0
#endif

// Let's just have this in a nice central location.  Again, since every file
//  will wind up including this file, we can affect compilation most effectively
//  from this location.
//
#define PLATFORM_LITTLE_ENDIAN


#define FN_CDECL __cdecl

//------------------------------------------------------------------------------
//-------------------------------------- Basic Types...

typedef signed char     	S8;
typedef unsigned char   	U8;

typedef signed short    	S16;
typedef unsigned short  	U16;

typedef signed int      	S32;
typedef unsigned int    	U32;

#ifdef __BORLANDC__
typedef signed __int64     S64;
typedef unsigned __int64   U64;
#elif defined(__MWERKS__) // This has to go b4 MSC_VER since CodeWarrior defines MSC_VER too
typedef signed long long   S64;
typedef unsigned long long U64;
#elif defined(_MSC_VER)
typedef signed _int64      S64;
typedef unsigned _int64    U64;
#pragma warning(disable: 4291) // disable warning caused by memory layer...
#else
typedef signed long long   S64;
typedef unsigned long long U64;
#endif

typedef float           	F32;
typedef double          	F64;

// size_t is needed to overload new
// size_t tends to be OS and compiler specific and may need to 
// be if/def'ed in the future
typedef unsigned int  dsize_t;

typedef const char* StringTableEntry;

struct FileTime
{
   U32 v1;
   U32 v2;
};

//------------------------------------------------------------------------------
//-------------------------------------- Type constants...
#define __EQUAL_CONST_F F32(0.000001)

static const F32 Float_One  = F32(1.0);
static const F32 Float_Half = F32(0.5);
static const F32 Float_Zero = F32(0.0);
static const F32 Float_Pi   = F32(3.14159265358979323846);
static const F32 Float_2Pi  = F32(2.0 * 3.14159265358979323846);

static const S8  S8_MIN  = S8(-128);
static const S8  S8_MAX  = S8(127);
static const U8  U8_MAX  = U8(255);

static const S16 S16_MIN = S16(-32768);
static const S16 S16_MAX = S16(32767);
static const U16 U16_MAX = U16(65535);

static const S32 S32_MIN = S32(-2147483647 - 1);
static const S32 S32_MAX = S32(2147483647);
static const U32 U32_MAX = U32(0xffffffff);


#ifdef _MSC_VER
#define for if(true) for
#endif


#endif //_NTYPES_H_
