//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _TYPES_H_
#define _TYPES_H_



#ifdef __POWERPC__
#ifndef _TYPESPPC_H_
#include "Platform/typesPPC.h"
#endif
#endif

#if defined(_WIN32) || defined(WIN32)
#ifndef _TYPESWIN32_H_
#include "Platform/typesWin32.h"
#endif
#endif

#ifdef __linux
#ifndef _TYPESLINUX_H_
#include "Platform/typesLinux.h"
#endif
#endif




//-------------------------------------- A couple of all-around useful inlines and
//                                        globals
//
U32 getNextPow2(U32 io_num);
U32 getBinLog2(U32 io_num);

inline bool isPow2(const U32 in_num)
{
   return (in_num == getNextPow2(in_num));
}

inline U16 endianSwap(const U16 in_swap)
{
   return U16(((in_swap >> 8) & 0x00ff) |
              ((in_swap << 8) & 0xff00));
}

inline U32 endianSwap(const U32 in_swap)
{
   return U32(((in_swap >> 24) & 0x000000ff) |
              ((in_swap >>  8) & 0x0000ff00) |
              ((in_swap <<  8) & 0x00ff0000) |
              ((in_swap << 24) & 0xff000000));
}

//----------------Many versions of min and max-------------
//---not using template functions because MS VC++ chokes---

inline U32 getMin(U32 a, U32 b)
{
   return a>b ? b : a;
}

inline U16 getMin(U16 a, U16 b)
{
   return a>b ? b : a;
}

inline U8 getMin(U8 a, U8 b)
{
   return a>b ? b : a;
}

inline S32 getMin(S32 a, S32 b)
{
   return a>b ? b : a;
}

inline S16 getMin(S16 a, S16 b)
{
   return a>b ? b : a;
}

inline S8 getMin(S8 a, S8 b)
{
   return a>b ? b : a;
}

inline float getMin(float a, float b)
{
   return a>b ? b : a;
}

inline double getMin(double a, double b)
{
   return a>b ? b : a;
}

inline U32 getMax(U32 a, U32 b)
{
   return a>b ? a : b;
}

inline U16 getMax(U16 a, U16 b)
{
   return a>b ? a : b;
}

inline U8 getMax(U8 a, U8 b)
{
   return a>b ? a : b;
}

inline S32 getMax(S32 a, S32 b)
{
   return a>b ? a : b;
}

inline S16 getMax(S16 a, S16 b)
{
   return a>b ? a : b;
}

inline S8 getMax(S8 a, S8 b)
{
   return a>b ? a : b;
}

inline float getMax(float a, float b)
{
   return a>b ? a : b;
}

inline double getMax(double a, double b)
{
   return a>b ? a : b;
}

//-------------------------------------- Use this instead of Win32 FOURCC()
//                                        macro...
//
#define makeFourCCTag(ch0, ch1, ch2, ch3)    \
   ((U32(U8(ch0)) << 0)  |             \
    (U32(U8(ch1)) << 8)  |             \
    (U32(U8(ch2)) << 16) |             \
    (U32(U8(ch3)) << 24))

#define BIT(x) (1 << (x))

#endif //_NTYPES_H_
