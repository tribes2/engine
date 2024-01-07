//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "Math/mMath.h"

#if !defined(__MWERKS__) && defined(_MSC_VER)
#define asm _asm
#endif

static S32 m_mulDivS32_ASM(S32 a, S32 b, S32 c)
{  // a * b / c
   S32 r;
   asm
   {
      mov   eax, a
      imul  b
      idiv  c      
      mov   r, eax
   }
   return r;
}   


static U32 m_mulDivU32_ASM(S32 a, S32 b, U32 c)
{  // a * b / c
   S32 r;
   asm
   {
      mov   eax, a
      mov   edx, 0
      mul   b
      div   c
      mov   r, eax
   }
   return r;
}   






//------------------------------------------------------------------------------
void mInstallLibrary_ASM()
{
   m_mulDivS32              = m_mulDivS32_ASM;
   m_mulDivU32              = m_mulDivU32_ASM;
}


