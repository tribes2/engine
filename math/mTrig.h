//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _MTRIG_H_
#define _MTRIG_H_

#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif
#ifndef _MMATHFN_H_
#include "Math/mMathFn.h"
#endif

//-------------------------------------- External assembly helpers...
//
//#ifdef WIN32
//
//extern "C" {
//   F32 m_reduceAngle_asm(const F32*);
//}
//
//inline F32
//m_reduceAngle(const F32 in_angle)
//{
//   F32 initial = m_reduceAngle_asm(&in_angle);
//   if (initial < 0.0)
//      initial += Float_2Pi;
//   return initial;
//}
//
//#else
//
//F32 m_reduceAngle(const F32 in_angle);
//
//#endif



#endif //_TRIG_H_
