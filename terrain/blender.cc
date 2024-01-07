//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "terrain/blender.h"


#define USE_ALPHATAB        (!BLENDER_USE_ASM)

#define SRC_BMP_SHIFT       8
#define DST_BMP_SHIFT       7
#define ALPHA_SHIFT         8
#define ALPHA_WID           (1 << ALPHA_SHIFT)
#define SRC_WID             (1 << SRC_BMP_SHIFT)
#define DST_WID             (1 << DST_BMP_SHIFT)
#define BMP_SIZE            (SRC_WID * SRC_WID)
#define DST_SIZE            (DST_WID * DST_WID)
#define LIGHTMAPSHIFT       9
#define BLOCKSHIFT          SRC_BMP_SHIFT
#define BLOCKMASK           ((1 << BLOCKSHIFT)-1)


#if BLENDER_USE_ASM

#ifdef __MWERKS__
#define _asm asm
#endif

struct QWORD
{
    unsigned short a, b, c, d;
};

static QWORD delta_a, delta_b, delta_c, delta_d;
static QWORD alpha_a0, alpha_b0, alpha_c0, alpha_d0;
static QWORD alpha_a1, alpha_b1, alpha_c1, alpha_d1;
static QWORD alpha_a2, alpha_b2, alpha_c2, alpha_d2;
static QWORD alpha_a3, alpha_b3, alpha_c3, alpha_d3;
static QWORD ldelt_a, ldelt_b, ldelt_c, ldelt_d;
static QWORD rdelt_a, rdelt_b, rdelt_c, rdelt_d;
static QWORD zero = { 0, 0, 0, 0 };
static QWORD redLightMask   = { 0xf800, 0, 0, 0 };
static QWORD greenLightMask = { 0x07c0, 0, 0, 0 };
static QWORD blueLightMask  = { 0x003e, 0, 0, 0 };

static QWORD delta2, delta3;
static QWORD rdelt_x2, ldelt_x2, leftq, rightq;
static QWORD mask_3e   = { 0x3e, 0, 0, 0 };
static QWORD mask_7c0 = { 0x07c0, 0, 0, 0 };
static QWORD mask_f8 = { 0x00f8, 0, 0, 0 };
static QWORD mask_f800000000 = { 0, 0, 0x00f8, 0 };
static QWORD rdeltq, ldeltq;
static QWORD mulfact = { 0x2000, 0x0008, 0x2000, 0x0008 };
static QWORD redblue = { 0x00f8, 0x00f8, 0x00f8, 0x00f8 };
static QWORD green =   { 0xf800, 0, 0xf800, 0 };
static QWORD alpha =   { 0x0001, 0x0001, 0, 0 };
static QWORD mask_0000ffff0000ffff = { 0xffff, 0, 0xffff, 0 };
static QWORD mask_00007fff00007fff = { 0x7fff, 0, 0x7fff, 0 };
// pass a few bits of data through mem for now.
//  most of them are constant for the entire blend() call.
#endif


static U32 lpoints[4];
static U32 texelsPerLumelShift;
static U32 texelsPerLumel;
static U32 texelsPerLumelDiv2;
static U32 nextsrcrow;
static U32 nextdstrow;
static U32 mip0_dstrowadd;
static U32 mip1_dstrowadd;
static U32 minus1srcrowsPlus8;
static U32 srcrows_x2_MinusTPL;


#if BLENDER_USE_ASM


static void doSquare4( U32 *dst, int sq_shift, int *aoff, U32 **bmp_ptrs, 
                       U8 **alpha_ptrs )
{
        int iy = 1 << sq_shift;
        int ix = iy >> 1;

_asm
{
        movd    mm1, sq_shift
        // get alpha values for the corners of the square for each texture type.
        //  replicate the values into 4 words of the qwords.  Also calc vertical
        //  stepping values for the alpha values on left and right edges.
        // load alpha value into bh to mul by 256 for precision. then
        // punpcklwd mm0, mm0 followed by punpckldq mm0, mm0 
        //  to replicate the low word into all words of mm0.
        // shift down difference by sqshift to divide by pixels per square to get
        //  increment.

        mov     esi, aoff
        mov     edi, alpha_ptrs
        mov     eax, [edi]
        mov     edx, eax
        add     eax, [esi]
        xor     ebx,ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm2, ebx
        punpcklwd mm2, mm2
        add     eax, [esi+8]
        punpckldq mm2, mm2
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm0, ebx
        punpcklwd mm0, mm0
        punpckldq mm0, mm0
        movq    alpha_a0, mm2
        psubw   mm0, mm2
        add     eax, [esi+4]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm4, ebx
        punpcklwd mm4, mm4
        add     eax, [esi+12]
        punpckldq mm4, mm4
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        movd    mm3, ebx
        movq    alpha_a1, mm4
        punpcklwd mm3, mm3
        punpckldq mm3, mm3
        psraw   mm0, mm1
        psubw   mm3, mm4
        movq    ldelt_a, mm0
        psraw   mm3, mm1
        movq    rdelt_a, mm3

        mov     eax, [edi+4]
        mov     edx, eax
        add     eax, [esi]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm2, ebx
        punpcklwd mm2, mm2
        add     eax, [esi+8]
        punpckldq mm2, mm2
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm0, ebx
        punpcklwd mm0, mm0
        punpckldq mm0, mm0
        movq    alpha_b0, mm2
        psubw   mm0, mm2
        add     eax, [esi+4]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm4, ebx
        punpcklwd mm4, mm4
        add     eax, [esi+12]
        punpckldq mm4, mm4
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        movd    mm3, ebx
        movq    alpha_b1, mm4
        punpcklwd mm3, mm3
        punpckldq mm3, mm3
        psraw   mm0, mm1
        psubw   mm3, mm4
        movq    ldelt_b, mm0
        psraw   mm3, mm1
        movq    rdelt_b, mm3

        mov     eax, [edi+8]
        mov     edx, eax
        add     eax, [esi]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm2, ebx
        punpcklwd mm2, mm2
        add     eax, [esi+8]
        punpckldq mm2, mm2
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm0, ebx
        punpcklwd mm0, mm0
        punpckldq mm0, mm0
        movq    alpha_c0, mm2
        movq    alpha_c2, mm0
        psubw   mm0, mm2
        add     eax, [esi+4]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm4, ebx
        punpcklwd mm4, mm4
        add     eax, [esi+12]
        punpckldq mm4, mm4
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        movd    mm3, ebx
        movq    alpha_c1, mm4
        punpcklwd mm3, mm3
        punpckldq mm3, mm3
        psraw   mm0, mm1
        psubw   mm3, mm4
        movq    ldelt_c, mm0
        psraw   mm3, mm1
        movq    rdelt_c, mm3

        mov     eax, [edi+12]
        mov     edx, eax
        add     eax, [esi]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm2, ebx
        punpcklwd mm2, mm2
        add     eax, [esi+8]
        punpckldq mm2, mm2
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm0, ebx
        punpcklwd mm0, mm0
        punpckldq mm0, mm0
        movq    alpha_d0, mm2
        movq    alpha_d2, mm0
        psubw   mm0, mm2
        add     eax, [esi+4]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm4, ebx
        punpcklwd mm4, mm4
        add     eax, [esi+12]
        punpckldq mm4, mm4
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        movd    mm3, ebx
        movq    alpha_d1, mm4
        punpcklwd mm3, mm3
        punpckldq mm3, mm3
        psraw   mm0, mm1
        psubw   mm3, mm4
        movq    ldelt_d, mm0
        psraw   mm3, mm1
        movq    rdelt_d, mm3

        mov     esi, bmp_ptrs
        mov     eax, [esi]
        mov     ebx, [esi+4]
        mov     ecx, [esi+8]
        mov     edx, [esi+12]

        movq    mm0, alpha_a1
        movq    mm2, alpha_b1
        movq    mm3, alpha_c1
        movq    mm4, alpha_a0
        movq    mm5, alpha_b0
        movq    mm6, alpha_c0
        movq    mm7, alpha_d0
        mov     edi, dst

yloop:
        // mm1 should be sq_shift at this point

        // calculate alpha step increments...word-size steps are replicated
        //  to fill qword.
        psubw   mm0, mm4
        psraw   mm0, mm1            ;mm0 = (right-left) >> sq_shift
        movq    delta_a, mm0         ;delta = ainc ainc ainc ainc
        
        psubw   mm2, mm5
        psraw   mm2, mm1            ;mm0 = (right-left) >> sq_shift
        movq    delta_b, mm2         ;delta = ainc ainc ainc ainc
        
        psubw   mm3, mm6
        psraw   mm3, mm1            ;mm0 = (right-left) >> sq_shift
        movq    delta_c, mm3         ;delta = ainc ainc ainc ainc
        
        movq    mm0, alpha_d1
        psubw   mm0, mm7
        psraw   mm0, mm1            ;mm0 = (right-left) >> sq_shift
        movq    delta_d, mm0         ;delta = ainc ainc ainc ainc

        mov     esi, ix
        pxor    mm2, mm2

xloop:
        movq    mm0, [eax]
        movq    mm1, mm0
        punpcklbw mm0, mm2
        pmulhw  mm0, mm4
        paddw   mm4, delta_a
        punpckhbw mm1, mm2
        pmulhw  mm1, mm4
        paddw   mm4, delta_a
        packuswb mm0, mm1

        movq    mm3, [ebx]
        movq    mm1, mm3
        punpcklbw mm3, mm2
        pmulhw  mm3, mm5
        paddw   mm5, delta_b
        punpckhbw mm1, mm2
        pmulhw  mm1, mm5
        paddw   mm5, delta_b
        packuswb mm3, mm1
        paddb   mm0, mm3

        movq    mm3, [ecx]
        movq    mm1, mm3
        punpcklbw mm3, mm2
        pmulhw  mm3, mm6
        paddw   mm6, delta_c
        punpckhbw mm1, mm2
        pmulhw  mm1, mm6
        paddw   mm6, delta_c
        packuswb mm3, mm1
        paddb   mm0, mm3

        movq    mm3, [edx]
        movq    mm1, mm3
        punpcklbw mm3, mm2
        pmulhw  mm3, mm7
        paddw   mm7, delta_d
        punpckhbw mm1, mm2
        pmulhw  mm1, mm7
        paddw   mm7, delta_d
        packuswb mm3, mm1
        paddb   mm0, mm3

        // double result, to make up for alpha vals being signed (max = 127)
        // so our math turns out a bit short, example:
        //  (0x7f00 * 0xff) >> 16 = 0x7e....* 2 = 252...not quite 255
        // would have been (0xff00 * 0xff) >> 16 = 0xfe = 254, 
        //  if I could do an unsigned pmulhw...
        // pmulhuw is in an intel document I found, but doesn't compile....
        paddb   mm0, mm0        

        movq    [edi], mm0

        add     eax, 8
        add     ebx, 8
        add     ecx, 8
        add     edx, 8
        add     edi, 8

        dec     esi
        jnz     xloop

        movq    mm4, alpha_a0
        paddw   mm4, ldelt_a
        movq    alpha_a0, mm4

        movq    mm5, alpha_b0
        paddw   mm5, ldelt_b
        movq    alpha_b0, mm5

        movq    mm6, alpha_c0
        paddw   mm6, ldelt_c
        movq    alpha_c0, mm6

        movq    mm7, alpha_d0
        paddw   mm7, ldelt_d
        movq    alpha_d0, mm7

        movq    mm0, alpha_d1
        paddw   mm0, rdelt_d
        movq    alpha_d1, mm0

        movq    mm2, alpha_b1
        paddw   mm2, rdelt_b
        movq    alpha_b1, mm2

        movq    mm3, alpha_c1
        paddw   mm3, rdelt_c
        movq    alpha_c1, mm3

        movq    mm0, alpha_a1
        paddw   mm0, rdelt_a
        movq    alpha_a1, mm0

        movd    mm1, sq_shift       // top of loop expects this

        dec     iy
        jnz     yloop

        emms
}
}





static void doSquare3( U32 *dst, int sq_shift, int *aoff, U32 **bmp_ptrs, 
                       U8 **alpha_ptrs )
{
        int iy = 1 << sq_shift;
        int ix = iy >> 1;

_asm
{
        movd    mm1, sq_shift
        // get alpha values for the corners of the square for each texture type.
        //  replicate the values into 4 words of the qwords.  Also calc vertical
        //  stepping values for the alpha values on left and right edges.
        // load alpha value into bh to mul by 256 for precision. then
        // punpcklwd mm0, mm0 followed by punpckldq mm0, mm0 
        //  to replicate the low word into all words of mm0.
        // shift down difference by sqshift to divide by pixels per square to get
        //  increment.

        mov     esi, aoff
        mov     edi, alpha_ptrs
        mov     eax, [edi]
        mov     edx, eax
        add     eax, [esi]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm2, ebx
        punpcklwd mm2, mm2
        add     eax, [esi+8]
        punpckldq mm2, mm2
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm0, ebx
        punpcklwd mm0, mm0
        punpckldq mm0, mm0
        movq    alpha_a0, mm2
        psubw   mm0, mm2
        add     eax, [esi+4]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm4, ebx
        punpcklwd mm4, mm4
        add     eax, [esi+12]
        punpckldq mm4, mm4
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        movd    mm3, ebx
        movq    alpha_a1, mm4
        punpcklwd mm3, mm3
        punpckldq mm3, mm3
        psraw   mm0, mm1
        psubw   mm3, mm4
        movq    ldelt_a, mm0
        psraw   mm3, mm1
        movq    rdelt_a, mm3

        mov     eax, [edi+4]
        mov     edx, eax
        add     eax, [esi]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm2, ebx
        punpcklwd mm2, mm2
        add     eax, [esi+8]
        punpckldq mm2, mm2
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm0, ebx
        punpcklwd mm0, mm0
        punpckldq mm0, mm0
        movq    alpha_b0, mm2
        psubw   mm0, mm2
        add     eax, [esi+4]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm4, ebx
        punpcklwd mm4, mm4
        add     eax, [esi+12]
        punpckldq mm4, mm4
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        movd    mm3, ebx
        movq    alpha_b1, mm4
        punpcklwd mm3, mm3
        punpckldq mm3, mm3
        psraw   mm0, mm1
        psubw   mm3, mm4
        movq    ldelt_b, mm0
        psraw   mm3, mm1
        movq    rdelt_b, mm3

        mov     eax, [edi+8]
        mov     edx, eax
        add     eax, [esi]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm2, ebx
        punpcklwd mm2, mm2
        add     eax, [esi+8]
        punpckldq mm2, mm2
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm0, ebx
        punpcklwd mm0, mm0
        punpckldq mm0, mm0
        movq    alpha_c0, mm2
        movq    alpha_c2, mm0
        psubw   mm0, mm2
        add     eax, [esi+4]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm4, ebx
        punpcklwd mm4, mm4
        add     eax, [esi+12]
        punpckldq mm4, mm4
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        movd    mm3, ebx
        movq    alpha_c1, mm4
        punpcklwd mm3, mm3
        punpckldq mm3, mm3
        psraw   mm0, mm1
        psubw   mm3, mm4
        movq    ldelt_c, mm0
        psraw   mm3, mm1
        movq    rdelt_c, mm3

        mov     esi, bmp_ptrs
        mov     eax, [esi]
        mov     ebx, [esi+4]
        mov     ecx, [esi+8]

        movq    mm0, alpha_a1
        movq    mm2, alpha_b1
        movq    mm3, alpha_c1
        movq    mm4, alpha_a0
        movq    mm5, alpha_b0
        movq    mm6, alpha_c0
        mov     edi, dst

yloop:
        // mm1 should be sq_shift at this point
        // mm0 should be alpha_a1
        // mm2 should be alpha_b1
        // mm3 should be alpha_c1

        // calculate alpha step increments...word-size steps are replicated
        //  to fill qword.
        psubw   mm0, mm4
        psraw   mm0, mm1            ;mm0 = (right-left) >> sq_shift
        movq    delta_a, mm0         ;delta = ainc ainc ainc ainc
        
        psubw   mm2, mm5
        psraw   mm2, mm1            ;mm0 = (right-left) >> sq_shift
        movq    delta_b, mm2         ;delta = ainc ainc ainc ainc
        
        psubw   mm3, mm6
        psraw   mm3, mm1            ;mm0 = (right-left) >> sq_shift
        movq    delta_c, mm3         ;delta = ainc ainc ainc ainc
        
        mov     esi, ix
        pxor    mm2, mm2


        movq    mm7, delta_a
xloop:
        movq    mm0, [eax]
        movq    mm1, mm0
        punpcklbw mm0, mm2
        pmulhw  mm0, mm4
        paddw   mm4, mm7
        punpckhbw mm1, mm2
        pmulhw  mm1, mm4
        paddw   mm4, mm7
        packuswb mm0, mm1

        movq    mm3, [ebx]
        movq    mm1, mm3
        punpcklbw mm3, mm2
        pmulhw  mm3, mm5
        paddw   mm5, delta_b
        punpckhbw mm1, mm2
        pmulhw  mm1, mm5
        paddw   mm5, delta_b
        packuswb mm3, mm1
        paddb   mm0, mm3

        movq    mm3, [ecx]
        movq    mm1, mm3
        punpcklbw mm3, mm2
        pmulhw  mm3, mm6
        paddw   mm6, delta_c
        punpckhbw mm1, mm2
        pmulhw  mm1, mm6
        paddw   mm6, delta_c
        packuswb mm3, mm1
        paddb   mm0, mm3
        paddb   mm0, mm0

        movq    [edi], mm0

        add     eax, 8
        add     ebx, 8
        add     ecx, 8
        add     edi, 8

        dec     esi
        jnz     xloop

        movq    mm4, alpha_a0
        paddw   mm4, ldelt_a
        movq    alpha_a0, mm4

        movq    mm5, alpha_b0
        paddw   mm5, ldelt_b
        movq    alpha_b0, mm5

        movq    mm6, alpha_c0
        paddw   mm6, ldelt_c
        movq    alpha_c0, mm6

        movq    mm2, alpha_b1
        paddw   mm2, rdelt_b
        movq    alpha_b1, mm2

        movq    mm3, alpha_c1
        paddw   mm3, rdelt_c
        movq    alpha_c1, mm3

        movq    mm0, alpha_a1
        paddw   mm0, rdelt_a
        movq    alpha_a1, mm0

        movd    mm1, sq_shift       // top of loop expects this

        dec     iy
        jnz     yloop

        emms
}
}

static void doSquare2( U32 *dst, int sq_shift, int *aoff, U32 **bmp_ptrs, 
                       U8 **alpha_ptrs )
{
        int iy = 1 << sq_shift;
        int ix = iy >> 1;

_asm
{
        movd    mm1, sq_shift
        // get alpha values for the corners of the square for each texture type.
        //  replicate the values into 4 words of the qwords.  Also calc vertical
        //  stepping values for the alpha values on left and right edges.
        // punpcklwd mm0, mm0 followed by punpckldq mm0, mm0 
        //  to replicate the low word into all words of mm0.
        // shift down difference by sqshift to divide by pixels per square to get
        //  increment.

        mov     esi, aoff
        mov     edi, alpha_ptrs
        mov     eax, [edi]
        mov     edx, eax
        add     eax, [esi]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm2, ebx
        punpcklwd mm2, mm2
        add     eax, [esi+8]
        punpckldq mm2, mm2
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm0, ebx
        punpcklwd mm0, mm0
        punpckldq mm0, mm0
        movq    alpha_a0, mm2
        psubw   mm0, mm2
        add     eax, [esi+4]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm4, ebx
        punpcklwd mm4, mm4
        add     eax, [esi+12]
        punpckldq mm4, mm4
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        movd    mm3, ebx
        movq    alpha_a1, mm4
        punpcklwd mm3, mm3
        punpckldq mm3, mm3
        psraw   mm0, mm1
        psubw   mm3, mm4
        movq    ldelt_a, mm0
        psraw   mm3, mm1
        movq    rdelt_a, mm3

        mov     eax, [edi+4]
        mov     edx, eax
        add     eax, [esi]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm2, ebx
        punpcklwd mm2, mm2
        add     eax, [esi+8]
        punpckldq mm2, mm2
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm0, ebx
        punpcklwd mm0, mm0
        punpckldq mm0, mm0
        movq    alpha_b0, mm2
        psubw   mm0, mm2
        add     eax, [esi+4]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm4, ebx
        punpcklwd mm4, mm4
        add     eax, [esi+12]
        punpckldq mm4, mm4
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        movd    mm3, ebx
        movq    alpha_b1, mm4
        punpcklwd mm3, mm3
        punpckldq mm3, mm3
        psraw   mm0, mm1
        psubw   mm3, mm4
        movq    ldelt_b, mm0
        psraw   mm3, mm1
        movq    rdelt_b, mm3

        mov     esi, bmp_ptrs
        mov     eax, [esi]
        mov     ebx, [esi+4]

        movq    mm0, alpha_a1
        movq    mm2, alpha_b1
        movq    mm4, alpha_a0
        movq    mm5, alpha_b0
        mov     edi, dst

yloop:
        // mm1 should be sq_shift at this point
        // mm0 should be alpha_a1
        // mm2 should be alpha_b1

        // calculate alpha step increments...word-size steps are replicated
        //  to fill qword.
        psubw   mm0, mm4
        psraw   mm0, mm1            ;mm0 = (right-left) >> sq_shift
        movq    delta_a, mm0         ;delta = ainc ainc ainc ainc
        
        psubw   mm2, mm5
        psraw   mm2, mm1            ;mm0 = (right-left) >> sq_shift
        movq    delta_b, mm2         ;delta = ainc ainc ainc ainc
        
        mov     esi, ix
        pxor    mm2, mm2

        movq    mm6, delta_a
        movq    mm7, delta_b

xloop:
        movq    mm0, [eax]
        movq    mm3, [ebx]

        movq    mm1, mm0
        punpcklbw mm0, mm2
        pmulhw  mm0, mm4
        paddw   mm4, mm6
        punpckhbw mm1, mm2
        pmulhw  mm1, mm4
        paddw   mm4, mm6
        packuswb mm0, mm1

        movq    mm1, mm3
        punpcklbw mm3, mm2
        pmulhw  mm3, mm5
        paddw   mm5, mm7
        punpckhbw mm1, mm2
        pmulhw  mm1, mm5
        paddw   mm5, mm7
        packuswb mm3, mm1
        paddb   mm0, mm3
        paddb   mm0, mm0

        movq    [edi], mm0

        add     edi, 8
        add     eax, 8
        add     ebx, 8

        dec     esi
        jnz     xloop

        movq    mm4, alpha_a0
        paddw   mm4, ldelt_a
        movq    alpha_a0, mm4

        movq    mm5, alpha_b0
        paddw   mm5, ldelt_b
        movq    alpha_b0, mm5

        movq    mm2, alpha_b1
        paddw   mm2, rdelt_b
        movq    alpha_b1, mm2

        movq    mm0, alpha_a1
        paddw   mm0, rdelt_a
        movq    alpha_a1, mm0

        movd    mm1, sq_shift       // top of loop expects this

        dec     iy
        jnz     yloop

        emms
}
}




// This special version only works for 4x4 lumels or larger (details 2, 3, 4).
// Lights from source ptr, and Creates Mips 0, 1, and 2 in one pass.
// Mip 0 must be 128x128, all mips are 5551 format.
// Source texture is variable width (but power of 2).
static void doLumelPlus1Mip( U16 *dstmip0, U16 *dstmip1, U32 *srcptr )
{
_asm
{
        movd    mm7, texelsPerLumelShift

        movd    mm0, [lpoints]
        movq    mm4, mm0
        pand    mm0, redLightMask
        movq    mm5, mm4
        pand    mm4, greenLightMask
        psllq   mm0, 31
        pand    mm5, blueLightMask
        psllq   mm4, 20
        paddw   mm0, mm4
        psllq   mm5, 9
        paddw   mm0, mm5        // mm0 = 0000rrrrggggbbbb qword for lp[0]
        movq    leftq, mm0

        movd    mm1, [lpoints+8]    // get lp2
        movq    mm4, mm1
        pand    mm1, redLightMask
        movq    mm5, mm4
        pand    mm4, greenLightMask
        psllq   mm1, 31
        pand    mm5, blueLightMask
        psllq   mm4, 20
        paddw   mm1, mm4
        psllq   mm5, 9
        paddw   mm1, mm5        // mm1 = 0000rrrrggggbbbb qword for lp[2]

        psubw   mm1, mm0
        psraw   mm1, mm7
        movq    ldeltq, mm1
        psllw   mm1, 1
        movq    ldelt_x2, mm1


        movd    mm2, [lpoints+4]    // get lp[1]
        movq    mm4, mm2
        pand    mm2, redLightMask
        movq    mm5, mm4
        pand    mm4, greenLightMask
        psllq   mm2, 31
        pand    mm5, blueLightMask
        psllq   mm4, 20
        paddw   mm2, mm4
        psllq   mm5, 9
        paddw   mm2, mm5        // mm2 = 0000rrrrggggbbbb qword for lp[1]
        movq    rightq, mm2

        movd    mm3, [lpoints+12]    // get lp3
        movq    mm4, mm3
        pand    mm3, redLightMask
        movq    mm5, mm4
        pand    mm4, greenLightMask
        psllq   mm3, 31
        pand    mm5, blueLightMask
        psllq   mm4, 20
        paddw   mm3, mm4
        psllq   mm5, 9
        paddw   mm3, mm5        // mm3 = 0000rrrrggggbbbb qword for lp[3]

        psubw   mm3, mm2
        psraw   mm3, mm7
        movq    rdeltq, mm3
        psllw   mm3, 1
        movq    rdelt_x2, mm3

        mov     edi, dstmip0
        mov     esi, srcptr
        mov     edx, dstmip1
        pxor    mm6, mm6

        mov     ecx, texelsPerLumelDiv2 // yloop count
        movq    mm2, leftq
        movq    mm3, rightq

        // mm2 is left, mm3 is right
yloop:
        movd    mm7, texelsPerLumelShift
        movq    mm6, mm2
        movq    mm1, mm2        // mm1 is light1
        movq    mm5, mm3
        movq    mm4, mm3

        paddw   mm5, rdeltq     // right + rdelt
        psubw   mm4, mm6        // right - left
        paddw   mm6, ldeltq     // left + ldelt
        psraw   mm4, mm7        
        movq    delta2, mm4

        psubw   mm5, mm6        // mm6 is light2
        psraw   mm5, mm7
        movq    delta3, mm5

        mov     ebx, texelsPerLumelDiv2 // loop count
                      
        // do 4 source pixels per loop
        // mm1 is light1
        // mm6 is light2
        pxor    mm7, mm7

xloop:
        // get first of source, col 0 and 1
        movq    mm4, [esi]
        add     esi, nextsrcrow
        movq    mm5, mm4
        punpcklbw mm4, zero
        pmulhw  mm4, mm1        // mm1 is light factor for first row
        paddw   mm1, delta2
        punpckhbw mm5, zero
        pmulhw  mm5, mm1
        paddw   mm1, delta2

        movq    mm7, [esi]
        add     esi, minus1srcrowsPlus8

        movq    mm0, mm4
        paddw   mm0, mm5        // mm0 is the avg, for mip1[0,1]

        packuswb    mm4, mm5          // put both pixels in same qword
        paddw       mm4, mm4          // double it, because lighting mul halved it
        movq        mm5, mm4          // save the original data
        pand        mm4, redblue      // mask out all but the 5MSBits of red and blue
        pmaddwd     mm4, mulfact      // multiply each word by
                                      //   2^13, 2^3, 2^13, 2^3 and add results
        pand        mm5, green        // mask out all but the 5MSBits of green
        por         mm4, mm5          // combine the red, green, and blue bits
        psrld       mm4, 6            // shift into position
        packssdw    mm4, zero         // pack into single dword
        pslld       mm4, 1            // shift into final position
        por         mm4, alpha        // add the alpha bit

        // write 2 pixels to mip0
        movd    [edi], mm4

        // get second row, cols 0 and 1
        movq    mm5, mm7
        pxor    mm4, mm4
        punpcklbw mm7, mm4
        pmulhw  mm7, mm6        // mm6 is light factor for 2nd row
        paddw   mm6, delta3
        punpckhbw mm5, mm4
        pmulhw  mm5, mm6
        paddw   mm6, delta3
        paddw   mm0, mm7
        paddw   mm0, mm5
        psrlw   mm0, 1          // mm0 is mip1[0,1] average
        
        packuswb    mm7, mm5          // put both pixels in same qword
        paddw       mm7, mm7          // double it, because lighting mul halved it
        movq        mm5, mm7          // save the original data
        pand        mm7, redblue      // mask out all but the 5MSBits of red and blue
        pmaddwd     mm7, mulfact      // multiply each word by
                                      //   2^13, 2^3, 2^13, 2^3 and add results
        pand        mm5, green        // mask out all but the 5MSBits of green
        por         mm7, mm5          // combine the red, green, and blue bits
        psrld       mm7, 6            // shift into position
        packssdw    mm7, mm4         // pack into single dword
        pslld       mm7, 1            // shift into final position
        por         mm7, alpha        // add the alpha bit

        // write 2 16-bit pixels to mip0, 2nd row
        movd    [edi+0x100], mm7

        movq        mm5, mm0
        movq        mm4, mm0
        pand        mm4, mask_f8    // red
        psrlq       mm5, 13
        pand        mm5, mask_7c0   // green
        psllq       mm4, 8
        pand        mm0, mask_f800000000    // blue
        paddw       mm5, mm4
        psrlq       mm0, 34
        paddw       mm0, mm5

        // write 1 pixels to mip1
        movd        eax, mm0
        mov         [edx], ax

        // increment ptrs
        add     edx, 2      // mip1
        add     edi, 4      // mip0

        dec     ebx
        jnz     xloop

        add     esi, srcrows_x2_MinusTPL
        add     edx, mip1_dstrowadd
        add     edi, mip0_dstrowadd
        paddw   mm2, ldelt_x2       // mm2 is left
        paddw   mm3, rdelt_x2       // mm3 is right
        dec     ecx
        jnz     yloop

        emms
}
}

static void do1x1Lumel( U16 *dstptr, U32 *srcptr )
{
_asm
{
        movd    mm0, [lpoints]
        movq    mm4, mm0
        pand    mm0, redLightMask
        movq    mm5, mm4
        pand    mm4, greenLightMask
        psllq   mm0, 31
        pand    mm5, blueLightMask
        psllq   mm4, 20
        paddw   mm0, mm4
        psllq   mm5, 9
        paddw   mm0, mm5        // mm0 = 0000rrrrggggbbbb qword for lp[0]

        mov     edi, dstptr
        mov     esi, srcptr
        pxor    mm6, mm6

        movd    mm4, [esi]
        punpcklbw mm4, mm6      // mm6 is expected to be 0 here
        pmulhw  mm4, mm0
        paddw   mm4, mm4

        movq    mm7, mm4
        movq    mm6, mm4
        psrlq   mm4, 34
        pand    mm7, mask_f8
        psrlq   mm6, 13
        psllq   mm7, 8
        pand    mm6, mask_7c0
        paddw   mm4, mm7
        paddw   mm4, mm6
        movd    eax, mm4
        mov     [edi],ax
        emms
}
}


static void cheatmips( U16 *srcptr, U16 *dstmip0, U16 *dstmip1, int wid )
{
_asm
{
        mov     esi, srcptr
        mov     edi, dstmip0
        mov     edx, dstmip1

        mov     ecx, wid
        shr     ecx, 1
        mov     eax, ecx
        shr     eax, 3
        shl     wid, 1

        movq    mm6, mask_0000ffff0000ffff
        movq    mm7, mask_00007fff00007fff

yloop:
        mov     ebx, eax
xloop:
        movq    mm0, [esi]
        movq    mm1, [esi+8]
        movq    mm2, [esi+16]
        movq    mm3, [esi+24]

        pand    mm0, mm6
        psrlw   mm1, 1
        pand    mm1, mm7
        psrlw   mm0, 1
        pand    mm2, mm6
        psrlw   mm3, 1
        pand    mm3, mm7
        psrlw   mm2, 1
        packssdw mm0, mm1
        packssdw mm2, mm3
        psllw   mm0, 1          // mip1, qw 0
        movq    [edi], mm0
        psllw   mm2, 1          // mip1, qw 1
        movq    [edi+8], mm2

        test    ecx, 1
        jnz     nomip2

        movq    mm1, mm0
        movq    mm3, mm2
        pand    mm1, mm6
        psrlw   mm3, 1
        pand    mm3, mm7
        psrlw   mm1, 1
        packssdw mm1, mm3
        psllw   mm1, 1          // mip2, qw 0
        movq    [edx], mm1
        add     edx, 8
nomip2:

        add     esi, 32
        add     edi, 16

        dec     ebx
        jnz     xloop

        add     esi, wid
        dec     ecx
        jnz     yloop

        emms
}
}


static void cheatmips4x4( U16 *srcptr, U16 *dstmip0, U16 *dstmip1 )
{
_asm
{
        mov     esi, srcptr
        mov     edi, dstmip0
        mov     edx, dstmip1

        movq    mm0, [esi]
        movq    mm1, [esi+16]
        pand    mm0, mask_0000ffff0000ffff
        psrlw   mm1, 1
        pand    mm1, mask_00007fff00007fff
        psrlw   mm0, 1
        packssdw mm0, mm1
        psllw   mm0, 1          // mip1, qw 0
        movq    [edi], mm0

        movd    eax, mm0
        mov     [edx], ax

        emms
}
}
#endif

#if USE_ALPHATAB
static U8 alphaTable[16384];
#endif

#if PAULS_TEST_CODE
static unsigned char grid_mats[256][256];
#endif


// old C extruder
static void extrude5551( const U16 *srcMip, U16 *mip, U32 height, U32 width )
{
   const U16 *src = srcMip;
   U16 *dst = mip;
   U32 stride = width << 1;
   
   for(U32 y = 0; y < height; y++)
   {
      for(U32 x = 0; x < width; x++)
      {
         U32 a = src[0];
         U32 b = src[1];
         U32 c = src[stride];
         U32 d = src[stride+1];
         dst[x] = ((((a >> 11) + (b >> 11) + (c >> 11) + (d >> 11)) >> 2) << 11) |
                  (((  ((a >> 6) & 0x1f) + ((b >> 6) & 0x1f) + ((c >> 6) & 0x1f) + ((d >> 6) & 0x1F) ) >> 2) << 6) |
                  ((( ((a >> 1) & 0x1F) + ((b >> 1) & 0x1F) + ((c >> 1) & 0x1f) + ((d >> 1) & 0x1f)) >> 2) << 1);
         src += 2;
      }
      src += stride;
      dst += width;
   }
}


// Take first mip in array, and extrude rest into other entries of array
//  i.e. power is 7 for 128x128, but there should be power+1 entries in the
//  array.
static void extrude( U16 **mips, U32 power )
{
    U32 width = 1 << (power - 1);

    for ( U32 i = 0; i < power; i++ )
    {
        extrude5551( mips[i], mips[i+1], width, width );
        width >>= 1;
    }
}



// level is between 2 (high detail) and 5 (low detail) inclusive;
//  x and y are in alpha sized squares (not tex squares)
//  lmap is light map data, format is 5551
//  destmips is a list of 16-bit 5551 rgb buffers for the result.
void Blender::blend( int x, int y, int level, const U16 *lmap, U16 **destmips )
{
    int sq_shift = 7 - level;
    int squareShift = sq_shift;
    int double_sq_shift = sq_shift << 1;
    int squareCount = 1 << (DST_BMP_SHIFT - sq_shift);
    int miplevel = level - 2;       // 0 at high detail, 3 at low detail
    U32 squareSize = 1 << sq_shift;

    U32 lumelsPerSquareShift = (LIGHTMAPSHIFT - BLOCKSHIFT);
    U32 lumelsPerSquare = 1 << lumelsPerSquareShift;
    texelsPerLumelShift = squareShift - lumelsPerSquareShift;
    texelsPerLumel = 1 << texelsPerLumelShift;
    texelsPerLumelDiv2 = texelsPerLumel >> 1;
    U32 textureSizeShift = 7;
    U32 lightMapSizeMask = (1 << LIGHTMAPSHIFT) - 1;
    U32 tystep = (1 << textureSizeShift);
    U32 stystep = (1 << squareShift);
    U32 txstep = texelsPerLumel;

    nextsrcrow = ((stystep) << 2);
    nextdstrow = ((tystep) << 1);

#if BLENDER_USE_ASM
    U32 *bmpptrs[4];
    U8 *alphaptrs[4];
    mip0_dstrowadd = (nextdstrow << 1) - (texelsPerLumel << 1);
    mip1_dstrowadd = (nextdstrow >> 1) - (texelsPerLumel);
    minus1srcrowsPlus8 = 8 - nextsrcrow;
    srcrows_x2_MinusTPL = (nextsrcrow << 1) - (texelsPerLumel << 2);
#endif

    for ( int sy = 0; sy < squareCount; sy++ )
    {
        int yp = (y + sy) & BLOCKMASK;            
        int yp1 = (yp + 1) & BLOCKMASK;
        int py = yp & 7;
        int yp_shifted = yp << ALPHA_SHIFT;
        int yp1_shifted = yp1 << ALPHA_SHIFT;
        int sy_shifted = sy << DST_BMP_SHIFT;
        int py_shifted = py << (SRC_BMP_SHIFT - miplevel + sq_shift);
        int mip_off = miplevel * num_src_bmps;

        for ( int sx = 0; sx < squareCount; sx++ )
        {
            int aoffs[4];
            int xp = (x + sx) & BLOCKMASK;            
            int xp1 = (xp + 1) & BLOCKMASK;
            int px = xp & 7;
            aoffs[0] = (yp_shifted + xp);
            aoffs[1] = (yp_shifted + xp1);
            aoffs[2] = (yp1_shifted + xp);
            aoffs[3] = (yp1_shifted + xp1);
            U32 *dstptr = blendbuffer;

            int bmpoff = py_shifted + (px << double_sq_shift);
#if BLENDER_USE_ASM
            int cnt = 0;

            U32 gridflags = GRIDFLAGS( xp, yp );

            for ( int i = 0; i < num_src_bmps; i++ )
                if ( gridflags & (MATERIALSTART << i) )
                {
                    alphaptrs[ cnt ] = alpha_data[ i ];
                    bmpptrs[ cnt++ ] = bmpdata[ mip_off + i ] + bmpoff;

                    if ( cnt == 4 )
                        break;
                }

            switch( cnt )
            {
                case 1:
                    // don't copy the square over...just leave it and tell
                    //  lighting code to use src bmp as the source instead of
                    //  the blend_buffer;
                    dstptr = bmpptrs[ 0 ];
                    break;
                case 2:
                    doSquare2( dstptr, sq_shift, aoffs, bmpptrs, alphaptrs  );
                    break;
                case 3:
                    doSquare3( dstptr, sq_shift, aoffs, bmpptrs, alphaptrs  );
                    break;
                case 4:
                    doSquare4( dstptr, sq_shift, aoffs, bmpptrs, alphaptrs  );
                    break;
            }
#else
            bool cleared = false;

            U32 gridflags = GRIDFLAGS( xp, yp );

            for ( int i = 0; i < num_src_bmps; i++ )
                if ( gridflags & (MATERIALSTART << i) )
                {
                    U32 a0 = U32( alpha_data[ i ][ aoffs[0] ] ) << 8;
                    U32 a1 = U32( alpha_data[ i ][ aoffs[1] ] ) << 8;
                    U32 a2 = U32( alpha_data[ i ][ aoffs[2] ] ) << 8;
                    U32 a3 = U32( alpha_data[ i ][ aoffs[3] ] ) << 8;

                    U32 ldelt = (a2 - a0) >> squareShift;
                    U32 rdelt = (a3 - a1) >> squareShift;
        
                    U32 left = a0;
                    U32 right = a1;
                    U8 *sourcePtr = (U8 *)(bmpdata[ mip_off + i ] + bmpoff);
                    U8 *addr1 = (U8 *)dstptr;
                    
                    if (!cleared)
                    {
                        for(U32 iy = 0; iy < squareSize; iy++)
                        {
                           U32 delt = (right - left) >> squareShift;
                           U32 astart = left;
                           U8 *addr = addr1;
                           for(U32 ix = 0; ix < squareSize; ix++)
                           {
                              U32 index = (astart >> 2) & 0x3F00;
                           
                              addr[0] = alphaTable[index | sourcePtr[0]];
                              addr[1] = alphaTable[index | sourcePtr[1]];
                              addr[2] = alphaTable[index | sourcePtr[2]];
                              addr += 4;
                              sourcePtr += 4;
                              astart += delt;
                           }
                           left += ldelt;
                           right += rdelt;
                           addr1 += 1 << (squareShift + 2);
                        }
                        cleared = true;
                    }
                    else
                    {
                        for(U32 iy = 0; iy < squareSize; iy++)
                        {
                           U32 delt = (right - left) >> squareShift;
                           U32 astart = left;
                           U8 *addr = addr1;
                           for(U32 ix = 0; ix < squareSize; ix++)
                           {
                              U32 index = (astart >> 2) & 0x3F00;
                           
                              addr[0] += alphaTable[index | sourcePtr[0]];
                              addr[1] += alphaTable[index | sourcePtr[1]];
                              addr[2] += alphaTable[index | sourcePtr[2]];
                              addr += 4;
                              sourcePtr += 4;
                              astart += delt;
                           }
                           left += ldelt;
                           right += rdelt;
                           addr1 += 1 << (squareShift + 2);
                        }
                    }
                }
#endif

            // copy in the lighting info
            U32 lxstart = xp << lumelsPerSquareShift;
            U32 lystart = yp << lumelsPerSquareShift;
    
            U32 txstart = sx << squareShift;
            U32 tystart = sy << squareShift;
    
            U32 tstart = 0, ststart = 0;
            U32 idy = (tystart << DST_BMP_SHIFT);
            U16 *bits0 = &destmips[0][ idy + txstart ];
            U16 *bits1 = &destmips[1][ (idy >> 2) + (txstart >> 1) ];
            U16 *bits2 = &destmips[2][ (idy >> 4) + (txstart >> 2) ];
    
            U32 ly = lystart;
            for(U32 iy = 0; iy < lumelsPerSquare; iy++)
            {
               U32 lynext = (ly + 1) & lightMapSizeMask;
    
               U32 lx = lxstart;
               U32 txwalk = 0, stxwalk = 0;

               for(U32 ix = 0; ix < lumelsPerSquare; ix++)
               {
                  U32 lxnext = (lx + 1) & lightMapSizeMask;
                  U32 twalk = tstart + txwalk;
                  U32 stwalk = ststart + stxwalk;
    
                  lpoints[0] = (U32)lmap[lx + (ly << LIGHTMAPSHIFT)];
                  lpoints[1] = (U32)lmap[lxnext + (ly << LIGHTMAPSHIFT)];
                  lpoints[2] = (U32)lmap[lx + (lynext << LIGHTMAPSHIFT)];
                  lpoints[3] = (U32)lmap[lxnext + (lynext << LIGHTMAPSHIFT)];

#if BLENDER_USE_ASM
                  if ( texelsPerLumel > 1 )
                  {
                      doLumelPlus1Mip( &bits0[ twalk ], 
                                        &bits1[ (tstart >> 2) + (txwalk >> 1) ], 
                                        &dstptr[ stwalk ] );
                  }
                  else
                      do1x1Lumel( &bits0[ twalk ], &dstptr[ stwalk ] );
#else
                  U32 col[3][4];
    
                  U32 i;
                  for(i = 0; i < 4; i++)
                  {
                     col[0][i] = (lpoints[i] >> 11) << 11;
                     col[1][i] = ((lpoints[i] >> 6) & 0x1f) << 11;
                     col[2][i] = ((lpoints[i] >> 1) & 0x1f) << 11;
                  }
    
                  U32 ldelt[3];
                  U32 rdelt[3];
                  U32 left[3];
                  U32 right[3];
    
                  for(i = 0; i < 3; i++)
                  {
                     ldelt[i] = (col[i][2] - col[i][0]) >> texelsPerLumelShift;
                     rdelt[i] = (col[i][3] - col[i][1]) >> texelsPerLumelShift;
    
                     left[i] = col[i][0];
                     right[i] = col[i][1];
                  }
    
                  for(U32 ty = 0; ty < texelsPerLumel; ty++)
                  {
                     U32 delt[3];
                     U32 start[3];
                     
                     for(i = 0; i < 3; i++)
                     {
                        delt[i] = (right[i] - left[i]) >> texelsPerLumelShift;
                        start[i] = left[i];
                     }                     
                     
                     U16 *dstbits = &bits0[ twalk ];
                     U8 *srcbits = (U8 *)&dstptr[stwalk];

                     for(U32 tx = 0; tx < texelsPerLumel; tx++)
                     {
                        U16 dstcol[3];
                        for(i = 0; i < 3; i++)
                        {
                           U32 index = (start[i] >> 2) & 0x3F00;
                           dstcol[i] = alphaTable[index | srcbits[i]];
                           start[i] += delt[i];
                        }
                        U16 packed_col = (dstcol[0]>>3)<<11;
                        packed_col += (dstcol[1]>>3)<<6;
                        packed_col += (dstcol[2]>>3)<<1;
                        *dstbits++ = packed_col;
                        srcbits += 4;
                     }
    
                     for(i = 0; i < 3; i++)
                     {
                        left[i] += ldelt[i];
                        right[i] += rdelt[i];
                     }

                     
                     twalk += tystep;
                     stwalk += stystep;
                  }
#endif
                  txwalk += txstep;
                  stxwalk += txstep;
                  lx = lxnext;
               }
               ly = lynext;
               tstart += (tystep << texelsPerLumelShift);
               ststart += (stystep << texelsPerLumelShift);
            }
        }
    }

#if BLENDER_USE_ASM
    if ( texelsPerLumel == 1)
        extrude( destmips, DST_BMP_SHIFT );
    else
    {
        cheatmips( destmips[1], destmips[2], destmips[3], 64 );
        cheatmips( destmips[3], destmips[4], destmips[5], 16 );
        cheatmips4x4( destmips[5], destmips[6], destmips[7] );
    }
#else
    extrude( destmips, DST_BMP_SHIFT );
#endif
}


#define MAX_SQ_SHIFT        5
#define NUM_BLOCKS          (1 << (SRC_BMP_SHIFT - MAX_SQ_SHIFT))


void Blender::addSourceTexture( int bmp_type, const U8 **bmps )
{
    int sqwid = 1 << MAX_SQ_SHIFT;
    int mipwid = SRC_WID;

    for ( int j = 0; j < num_mip_levels; j++ )
    {
        U32 *dst = bmpdata[ j * num_src_bmps + bmp_type ];

        // copy the bmp data over, changing the format so each block
        //  is contiguous. 
        for ( int row = 0; row < NUM_BLOCKS; row++ )
        { 
            const U8 *srcptr = bmps[ j ] + row * mipwid * sqwid * 3;

            for ( int col = 0; col < NUM_BLOCKS; col++, srcptr += (sqwid * 3) )
                for ( int py = 0; py < sqwid; py++ )
                { 
                    const U8 *src2 = srcptr + py * mipwid * 3;

                    for ( int px = 0; px < sqwid; px++, src2 += 3 )
                        *dst++ = src2[0] + (U32(src2[1]) << 8) + (U32(src2[2]) << 16);
                }
        }

        sqwid >>= 1;
        mipwid >>= 1;
    }
}



#define BLEND_BUFFER_SIZE   (1 << (MAX_SQ_SHIFT * 2))
#define CACHE_ROUND_SHIFT   12
#define CACHE_ROUND_ADJUST  ((1 << CACHE_ROUND_SHIFT) - 1)
#define CACHE_ROUND_MASK    (~CACHE_ROUND_ADJUST)
#define DWORD_STAGGER       0// 256

static U32 *round_to_cache_start( U32 *ptr )
{
    return ( (U32 *) ((int(ptr) + CACHE_ROUND_ADJUST) & CACHE_ROUND_MASK) );
}


Blender::Blender( int num_src, int num_mips, U8 **alphas )
{
    int bmps_size = BLEND_BUFFER_SIZE;        // blending buffer (1 square)
    int mip_size = BMP_SIZE;
    int i, j;

    alpha_data = new U8*[num_src];
    for (i = 0; i < num_src; i++)
      alpha_data[i] = alphas[i];

    num_src_bmps = num_src;
    num_mip_levels = num_mips;

    bmpdata = new U32*[ num_src * num_mips ];

    for ( i = 0; i < num_mips; i++ )
    {
        bmps_size += (mip_size + DWORD_STAGGER) * num_src;
        mip_size >>= 2;
    }

    bmp_alloc_ptr = new U32[ bmps_size + CACHE_ROUND_ADJUST ];
    U32 *bmps = round_to_cache_start( bmp_alloc_ptr );

    // buffer that we'll be blending into, and lighting out of.
    blendbuffer = bmps;

    U32 *curbmp = blendbuffer + BLEND_BUFFER_SIZE;
    int bmp_size = BMP_SIZE;
    int bmpnum = 0;

    // initialize pointers into buffer for source textures.
    for ( j = 0; j < num_mips; j++ )
    {
        for ( i = 0; i < num_src; i++ )
        {
            bmpdata[ bmpnum ] = curbmp;
            U32 *bptr = curbmp;

            curbmp += (bmp_size + DWORD_STAGGER);
            bmpnum++;
        }

        bmp_size >>= 2;
    }


#if PAULS_TEST_CODE
    // now zip through the alpha grids, and setup our material bits.
    //  only needed for test code, as real code can use tribes' grid.
    int kbit = 1;

    for ( int k = 0; k < num_src; k++, kbit <<= 1 )
    {
        for ( i = 0; i < ALPHA_WID; i++ )
            for ( j = 0; j < ALPHA_WID; j++ )
            {
                int i2 = (i + 1) & 0xff;
                int j2 = (j + 1) & 0xff;

                if ( !k )
                    grid_mats[i][j] = 0;

                if ( alphas[k][i*ALPHA_WID+j] || alphas[k][i2*ALPHA_WID+j] ||
                     alphas[k][i*ALPHA_WID+j2] || alphas[k][i2*ALPHA_WID+j2] )
                    grid_mats[i][j] |= kbit;
            }
    }

    // make sure all squares have at least one bit set, so that it
    //  at least fills in with black.
    for ( i = 0; i < ALPHA_WID; i++ )
        for ( j = 0; j < ALPHA_WID; j++ )
            if ( !grid_mats[i][j] )
                grid_mats[i][j] = 1;
#endif

#if USE_ALPHATAB
    // build alpha blending table for C versions...
    for (i = 0; i < 16384; i++)
    {
        U32 pix = i & 0xFF;
        U32 alpha = i >> 8;
        U32 val = U32( pix * (F32(alpha) / 63.f) );
        alphaTable[i] = val;
    }
#endif
}

Blender::~Blender()
{
    if ( bmp_alloc_ptr )
        delete [] bmp_alloc_ptr;

    if ( bmpdata )
        delete [] bmpdata;

    if ( alpha_data )
        delete [] alpha_data;
}

