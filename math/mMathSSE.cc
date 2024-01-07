//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "math/mMathFn.h"
#include "math/mPlane.h"
#include "math/mMatrix.h"


U32 gSSE_MatXMat_Calls = 0;
void SSE_MatrixF_x_MatrixF(const F32 *matA, const F32 *matB, F32 *result)
{
   #ifdef INTERNAL_RELEASE
   gSSE_MatXMat_Calls++;
   #endif
   
   __asm {
      mov         edx, matA 
      mov         eax, result
      mov         ecx, matB

      movss       xmm0, dword ptr [edx]
      movups      xmm1, xmmword ptr [ecx]
      shufps      xmm0, xmm0, 0
      movss       xmm2, dword ptr [edx+4]
      mulps       xmm0, xmm1
      shufps      xmm2, xmm2, 0
      movups      xmm3, xmmword ptr [ecx+10h]
      movss       xmm7, dword ptr [edx+8]
      mulps       xmm2, xmm3
      shufps      xmm7, xmm7, 0
      addps       xmm0, xmm2
      movups      xmm4, xmmword ptr [ecx+20h]
      movss       xmm2, dword ptr [edx+0Ch]
      mulps       xmm7, xmm4
      shufps      xmm2, xmm2, 0
      addps       xmm0, xmm7
      movups      xmm5, xmmword ptr [ecx+30h]
      movss       xmm6, dword ptr [edx+10h]
      mulps       xmm2, xmm5
      movss       xmm7, dword ptr [edx+14h]
      shufps      xmm6, xmm6, 0
      addps       xmm0, xmm2
      shufps      xmm7, xmm7, 0
      movlps      qword ptr [eax], xmm0
      movhps      qword ptr [eax+8], xmm0
      mulps       xmm7, xmm3
      movss       xmm0, dword ptr [edx+18h]
      mulps       xmm6, xmm1
      shufps      xmm0, xmm0, 0
      addps       xmm6, xmm7
      mulps       xmm0, xmm4
      movss       xmm2, dword ptr [edx+24h]
      addps       xmm6, xmm0
      movss       xmm0, dword ptr [edx+1Ch]
      movss       xmm7, dword ptr [edx+20h]
      shufps      xmm0, xmm0, 0
      shufps      xmm7, xmm7, 0
      mulps       xmm0, xmm5
      mulps       xmm7, xmm1
      addps       xmm6, xmm0
      shufps      xmm2, xmm2, 0
      movlps      qword ptr [eax+10h], xmm6
      movhps      qword ptr [eax+18h], xmm6
      mulps       xmm2, xmm3
      movss       xmm6, dword ptr [edx+28h]
      addps       xmm7, xmm2
      shufps      xmm6, xmm6, 0
      movss       xmm2, dword ptr [edx+2Ch]
      mulps       xmm6, xmm4
      shufps      xmm2, xmm2, 0
      addps       xmm7, xmm6
      mulps       xmm2, xmm5
      movss       xmm0, dword ptr [edx+34h]
      addps       xmm7, xmm2
      shufps      xmm0, xmm0, 0
      movlps      qword ptr [eax+20h], xmm7
      movss       xmm2, dword ptr [edx+30h]
      movhps      qword ptr [eax+28h], xmm7
      mulps       xmm0, xmm3
      shufps      xmm2, xmm2, 0
      movss       xmm6, dword ptr [edx+38h]
      mulps       xmm2, xmm1
      shufps      xmm6, xmm6, 0
      addps       xmm2, xmm0
      mulps       xmm6, xmm4
      movss       xmm7, dword ptr [edx+3Ch]
      shufps      xmm7, xmm7, 0
      addps       xmm2, xmm6
      mulps       xmm7, xmm5
      addps       xmm2, xmm7
      movups      xmmword ptr [eax+30h], xmm2
   }
}


#if 0
void SSE_MatrixF_x_VectorF(const F32 *matrix, const F32 *vector, F32 *result)
{
}
#endif


void mInstall_Library_SSE()
{
   m_matF_x_matF           = SSE_MatrixF_x_MatrixF;
   // m_matF_x_point3F = Athlon_MatrixF_x_Point3F;
   // m_matF_x_vectorF = Athlon_MatrixF_x_VectorF;
}

