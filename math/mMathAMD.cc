//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "math/mMathFn.h"
#include "math/mPlane.h"
#include "math/mMatrix.h"


// extern void (*m_matF_x_point3F)(const F32 *m, const F32 *p, F32 *presult);
// extern void (*m_matF_x_vectorF)(const F32 *m, const F32 *v, F32 *vresult);


void Athlon_MatrixF_x_Point3F(const F32 *m, const F32 *p, F32 *presult)
{
   m;
   p; 
   presult;
}

//  
//  Here's the C code - note that the code below does it in a different order
//
// r[0] = a[0]*b[0] + a[1]*b[4] + a[2]*b[8]  + a[3]*b[12];
// r[1] = a[0]*b[1] + a[1]*b[5] + a[2]*b[9]  + a[3]*b[13];
// r[2] = a[0]*b[2] + a[1]*b[6] + a[2]*b[10] + a[3]*b[14];
// r[3] = a[0]*b[3] + a[1]*b[7] + a[2]*b[11] + a[3]*b[15];

// r[4] = a[4]*b[0] + a[5]*b[4] + a[6]*b[8]  + a[7]*b[12];
// r[5] = a[4]*b[1] + a[5]*b[5] + a[6]*b[9]  + a[7]*b[13];
// r[6] = a[4]*b[2] + a[5]*b[6] + a[6]*b[10] + a[7]*b[14];
// r[7] = a[4]*b[3] + a[5]*b[7] + a[6]*b[11] + a[7]*b[15];

// r[8] = a[8]*b[0] + a[9]*b[4] + a[10]*b[8] + a[11]*b[12];
// r[9] = a[8]*b[1] + a[9]*b[5] + a[10]*b[9] + a[11]*b[13];
// r[10]= a[8]*b[2] + a[9]*b[6] + a[10]*b[10]+ a[11]*b[14];
// r[11]= a[8]*b[3] + a[9]*b[7] + a[10]*b[11]+ a[11]*b[15];

// r[12]= a[12]*b[0]+ a[13]*b[4]+ a[14]*b[8] + a[15]*b[12];
// r[13]= a[12]*b[1]+ a[13]*b[5]+ a[14]*b[9] + a[15]*b[13];
// r[14]= a[12]*b[2]+ a[13]*b[6]+ a[14]*b[10]+ a[15]*b[14];
// r[15]= a[12]*b[3]+ a[13]*b[7]+ a[14]*b[11]+ a[15]*b[15];

void Athlon_MatrixF_x_MatrixF(const F32 *matA, const F32 *matB, F32 *result)
{
   __asm {

      femms
      mov         ecx,matA
      mov         eax,result
      mov         edx,matB
      
      prefetch    [ecx+32]       // These may help -
      prefetch    [edx+32]       //    and probably don't hurt
      
      movq        mm0,[ecx]		// a21	| a11
      movq        mm1,[ecx+8]		// a41	| a31
      movq        mm4,[edx]		// b21	| b11
      punpckhdq   mm2,mm0			// a21	| 
      movq        mm5,[edx+16]	// b22	| b12
      punpckhdq   mm3,mm1			// a41	| 
      movq        mm6,[edx+32]	// b23	| b13
      punpckldq   mm0,mm0			// a11	| a11
      punpckldq   mm1,mm1			// a31	| a31
      pfmul       mm4,mm0			// a11*b21 | a11*b11
      punpckhdq   mm2,mm2			// a21	| a21
      pfmul       mm0,[edx+8]		// a11*b41 | a11*b31
      movq        mm7,[edx+48]	// b24	| b14
      pfmul       mm5,mm2			// a21*b22 | a21*b12
      punpckhdq   mm3,mm3			// a41	| a41
      pfmul       mm2,[edx+24]	// a21*b42 | a21*b32
      pfmul       mm6,mm1			// a31*b23 | a31*b13 
      pfadd       mm5,mm4			// a21*b22 + a11*b21 | a21*b12 + a11*b11
      pfmul       mm1,[edx+40]	// a31*b43 | a31*b33
      pfadd       mm2,mm0			// a21*b42 + a11*b41 | a21*b32 + a11*b31
      pfmul       mm7,mm3			// a41*b24 | a41*b14 
      pfadd       mm6,mm5			// a21*b22 + a11*b21 + a31*b23 | a21*b12 + a11*b11 + a31*b13
      pfmul       mm3,[edx+56]	// a41*b44 | a41*b34
      pfadd       mm2,mm1			// a21*b42 + a11*b41 + a31*b43 | a21*b32 + a11*b31 + a31*b33 
      pfadd       mm7,mm6			// a41*b24 + a21*b22 + a11*b21 + a31*b23 |  a41*b14 + a21*b12 + a11*b11 + a31*b13
      movq        mm0,[ecx+16]	// a22	| a12
      pfadd       mm3,mm2			// a41*b44 + a21*b42 + a11*b41 + a31*b43 | a41*b34 + a21*b32 + a11*b31 + a31*b33 
      movq        mm1,[ecx+24]	// a42	| a32
      movq        [eax],mm7		// r21	| r11 
      movq        mm4,[edx]		// b21	| b11
      movq        [eax+8],mm3		// r41	| r31

      punpckhdq   mm2,mm0			// a22	| XXX
      movq        mm5,[edx+16]	// b22	| b12
      punpckhdq   mm3,mm1			// a42	| XXX
      movq        mm6,[edx+32]	// b23	| b13
      punpckldq   mm0,mm0			// a12	| a12
      punpckldq   mm1,mm1			// a32	| a32
      pfmul       mm4,mm0			// a12*b21 | a12*b11
      punpckhdq   mm2,mm2			// a22	| a22
      pfmul       mm0,[edx+8]		// a12*b41 | a12*b31
      movq        mm7,[edx+48]	// b24	| b14
      pfmul       mm5,mm2			// a22*b22 | a22*b12
      punpckhdq   mm3,mm3			// a42	| a42
      pfmul       mm2,[edx+24]	// a22*b42 | a22*b32
      pfmul       mm6,mm1			// a32*b23 | a32*b13
      pfadd       mm5,mm4			// a12*b21 + a22*b22 | a12*b11 + a22*b12
      pfmul       mm1,[edx+40]	// a32*b43 | a32*b33
      pfadd       mm2,mm0			// a12*b41 + a22*b42 | a12*b11 + a22*b32
      pfmul       mm7,mm3			// a42*b24 | a42*b14
      pfadd       mm6,mm5			// a32*b23 + a12*b21 + a22*b22 | a32*b13 + a12*b11 + a22*b12
      pfmul       mm3,[edx+56]	// a42*b44 | a42*b34
      pfadd       mm2,mm1			// a32*b43 + a12*b41 + a22*b42 | a32*b33 + a12*b11 + a22*b32
      pfadd       mm7,mm6			// a42*b24 + a32*b23 + a12*b21 + a22*b22 | a42*b14 + a32*b13 + a12*b11 + a22*b12
      movq        mm0,[ecx+32]	// a23 | a13
      pfadd       mm3,mm2			// a42*b44 + a32*b43 + a12*b41 + a22*b42 | a42*b34 + a32*b33 + a12*b11 + a22*b32
      movq        mm1,[ecx+40]	// a43 | a33
      movq        [eax+16],mm7	// r22 | r12
      movq        mm4,[edx]		// b21	| b11
      movq        [eax+24],mm3	// r42 | r32

      punpckhdq   mm2,mm0			// a23 | XXX
      movq        mm5,[edx+16]	// b22 | b12
      punpckhdq   mm3,mm1			// a43 | XXX
      movq        mm6,[edx+32]	// b23 | b13
      punpckldq   mm0,mm0			// a13 | a13
      punpckldq   mm1,mm1			// a33 | a33
      pfmul       mm4,mm0			// a13*b21 | a13*b11
      punpckhdq   mm2,mm2			// a23 | a23
      pfmul       mm0,[edx+8]		// a13*b41 | a13*b31
      movq        mm7,[edx+48]	// b24 | b14
      pfmul       mm5,mm2			// a23*b22 | a23*b12
      punpckhdq   mm3,mm3			// a43 | a43
      pfmul       mm2,[edx+24]	// a23*b42 | a23*b32
      pfmul       mm6,mm1			// a33*b23 | a33*b13
      pfadd       mm5,mm4			// a23*b22 + a13*b21 | a23*b12 + a13*b11
      pfmul       mm1,[edx+40]	// a33*b43 | a33*b33 
      pfadd       mm2,mm0			// a13*b41 + a23*b42 | a13*b31 + a23*b32
      pfmul       mm7,mm3			// a43*b24 | a43*b14
      pfadd       mm6,mm5			// a33*b23 + a23*b22 + a13*b21 | a33*b13 + a23*b12 + a13*b11
      pfmul       mm3,[edx+56]	// a43*b44 | a43*b34
      pfadd       mm2,mm1			// a33*b43*a13*b41 + a23*b42 | a33*b33 + a13*b31 + a23*b32
      pfadd       mm7,mm6			// a43*b24 + a33*b23 + a23*b22 + a13*b21 | a43*b14 + a33*b13 + a23*b12 + a13*b11
      movq        mm0,[ecx+48]	// a24 | a14
      pfadd       mm3,mm2			// a43*b44 + a33*b43*a13*b41 + a23*b42 | a43*b34 + a33*b33 + a13*b31 + a23*b32
      movq        mm1,[ecx+56]	// a44 | a34
      movq        [eax+32],mm7	// r23 | r13
      movq        mm4,[edx]		// b21 | b11
      movq        [eax+40],mm3	// r43 | r33

      punpckhdq   mm2,mm0			// a24 | XXX
      movq        mm5,[edx+16]	// b22 | b12
      punpckhdq   mm3,mm1			// a44 | XXX
      movq        mm6,[edx+32]	// b23 | b13
      punpckldq   mm0,mm0			// a14 | a14
      punpckldq   mm1,mm1			// a34 | a34
      pfmul       mm4,mm0			// a14*b21 | a14*b11
      punpckhdq   mm2,mm2			// a24 | a24
      pfmul       mm0,[edx+8]		// a14*b41 | a14*b31
      movq        mm7,[edx+48]	// b24 | b14
      pfmul       mm5,mm2			// a24*b22 | a24*b12
      punpckhdq   mm3,mm3			// a44 | a44
      pfmul       mm2,[edx+24]	// a24*b 42 | a24*b32
      pfmul       mm6,mm1			// a34*b23 | a34*b13
      pfadd       mm5,mm4			// a14*b21 + a24*b22 | a14*b11 + a24*b12
      pfmul       mm1,[edx+40]	// a34*b43 | a34*b33
      pfadd       mm2,mm0			// a14*b41 + a24*b 42 | a14*b31 + a24*b32
      pfmul       mm7,mm3			// a44*b24 | a44*b14
      pfadd       mm6,mm5			// a34*b23 + a14*b21 + a24*b22 | a34*b13 + a14*b11 + a24*b12
      pfmul       mm3,[edx+56]	// a44*b44 | a44*b34
      pfadd       mm2,mm1			// a34*b43 + a14*b41 + a24*b 42 | a34*b33 + a14*b31 + a24*b32
      pfadd       mm7,mm6			// a44*b24 + a14*b23 + a24*b 42 | a44*b14 + a14*b31 + a24*b32
      pfadd       mm3,mm2			// a44*b44 + a34*b43 + a14*b41 + a24*b42 | a44*b34 + a34*b33 + a14*b31 + a24*b32
      movq        [eax+48],mm7	// r24 | r14
      movq        [eax+56],mm3	// r44 | r34
      femms
   }
}


#if 0
void Athlon_MatrixF_x_VectorF(const F32 *matrix, const F32 *vector, F32 *result)
{
   __asm {
      femms
      mov         eax,result
      mov         ecx,vector
      mov         edx,matrix

      // Here's what we're doing:      
      // result[0] = M[0] * v[0]    +  M[1] * v[1]    +  M[2] * v[2];
      // result[1] = M[4] * v[0]    +  M[5] * v[1]    +  M[6] * v[2];
      // result[2] = M[8] * v[0]    +  M[9] * v[1]    +  M[10]* v[2];

      movq        mm0,[ecx]         //     y   |  x
      movd        mm1,[ecx+8]       //     0   |  z
      movd        mm4,[edx+8]       //     0   | m_13
      movq        mm3,mm0           //     y   |  x
      movd        mm2,[edx+40]      //     0   | m_33 (M[10])
      punpckldq   mm0,mm0           //     x   |  x
      punpckldq   mm4,[edx+20]      //    m_31 | m_23
      pfmul       mm0,[edx]         //     x * m_12 | x * m_11
      punpckhdq   mm3,mm3           //     y   |  y
      pfmul       mm2,mm1           //     0   |  z * m_33
      punpckldq   mm1,mm1           //     z   |  z
      pfmul       mm4,[ecx]         //    y * m_31 | x * m_23
      pfmul       mm3,[edx+12]      //    y * m_22 | y * m_21
      pfmul       mm1,[edx+24]      //    z * m_32 | z * m_32
      pfacc       mm4,mm4           //     ?   | y * m_31 + x * m_23
      pfadd       mm3,mm0           //     x * m_12 + y * m_22 | x * m_11 + y * m_21
      pfadd       mm4,mm2           //     ?   | y * m_31 + x * m_23 + z * m_33
      pfadd       mm3,mm1           //     x * m_12 + y * m_22 + z * m_32 | x * m_11 + y * m_21 + z * m_32
      movd        [eax+8],mm4       //    r_z
      movq        [eax],mm3         //    r_y  | r_x
      femms
   }
}
#endif


void mInstall_AMD_Math()
{
   m_matF_x_matF           = Athlon_MatrixF_x_MatrixF;
   // m_matF_x_point3F = Athlon_MatrixF_x_Point3F;
   // m_matF_x_vectorF = Athlon_MatrixF_x_VectorF;
}
