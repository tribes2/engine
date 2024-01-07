//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "terrain/terrData.h"
#include "Math/mMath.h"
#include "dgl/dgl.h"
#include "dgl/gBitmap.h"
#include "terrain/terrRender.h"

void terrMipBlit_asm( U16* dest, U32 destStride, U32 squareSize,
		      const U8* sourcePtr, U32 sourceStep, U32 sourceRowAdd )
{

#if 0
	for( U32 k = 0; k < squareSize; k++ ) {

		for( U32 l = 0; l < squareSize; l++ ) {
			dest[l] = *( (U16*) sourcePtr );
			sourcePtr += sourceStep;
		}

		dest -= destStride;
		sourcePtr += sourceRowAdd;
	}
#endif

	if( sourceStep == 2 ) {
		destStride <<= 1;
		sourceRowAdd += squareSize << 1;

		__asm__ __volatile__( "pushl %%eax                    \n\t"
				      "pushl %%ebx                    \n\t"
				      "pushl %%ecx                    \n\t"
				      "pushl %%edx                    \n\t"
				      "pushl %%edi                    \n\t"
				      "pushl %%esi                    \n\t"
				      "                               \n\t"
				      "movl %0, %%edx                 \n\t"
				      "movl %1, %%edi                 \n\t"
				      "movl %2, %%esi                 \n\t"
				      "shrl $1, %%edx                 \n\t"
				      "movl $0, %%ecx                 \n\t"
				      "movl $0, %%ebx                 \n\t"
				      "                               \n"
				      "pixelLoop2:                    \n\t"
				      "movl (%%esi,%%ebx,4), %%eax    \n\t"
				      "movl %%eax, (%%edi,%%ebx,4)    \n\t"
				      "incl %%ebx                     \n\t"
				      "cmpl %%edx, %%ebx              \n\t"
				      "jnz pixelLoop2                 \n\t"
				      "                               \n\t"
				      "movl $0, %%ebx                 \n\t"
				      "incl %%ecx                     \n\t"
				      "subl %3, %%edi                 \n\t"
				      "addl %4, %%esi                 \n\t"
				      "cmpl %5, %%ecx                 \n\t"
				      "jl pixelLoop2                  \n\t"
				      "                               \n\t"
				      "popl %%esi                     \n\t"
				      "popl %%edi                     \n\t"
				      "popl %%edx                     \n\t"
				      "popl %%ecx                     \n\t"
				      "popl %%ebx                     \n\t"
				      "popl %%eax                     \n\t"
				      : /* no outputs */
				      : "g" (squareSize ), "g" (dest), "g" (sourcePtr),
				      "g" (destStride), "g" (sourceRowAdd), "g" (squareSize)
				      : "eax", "ebx", "ecx", "edx", "edi", "esi", "cc", "memory" );
	} else if( sourceStep == -2 ) {
		destStride <<= 1;
		__asm__ __volatile__( "pushl %%eax                    \n\t"
				      "pushl %%ebx                    \n\t"
				      "pushl %%ecx                    \n\t"
				      "pushl %%edx                    \n\t"
				      "pushl %%edi                    \n\t"
				      "pushl %%esi                    \n\t"
				      "                               \n\t"
				      "movl %0, %%edx                 \n\t"
				      "movl %1, %%edi                 \n\t"
				      "movl %2, %%esi                 \n\t"
				      "shrl $1, %%edx                 \n\t"
				      "movl $0, %%ecx                 \n\t"
				      "movl $0, %%ebx                 \n\t"
				      "                               \n"
				      "pixelLoopNeg2:                 \n\t"
				      "movl -2(%%esi), %%eax          \n\t"
				      "subl $4, %%esi                 \n\t"
				      "rorl $16, %%eax                \n\t"
				      "movl %%eax, (%%edi,%%ebx,$4)   \n\t"
				      "incl %%ebx                     \n\t"
				      "cmpl %%edx, %%ebx              \n\t"
				      "jnz pixelLoopNeg2              \n\t"
				      "                               \n\t"
				      "movl $0, %%ebx                 \n\t"
				      "incl %%ecx                     \n\t"
				      "subl %3, %%edi                 \n\t"
				      "addl %4, %%esi                 \n\t"
				      "cmpl %5, %%ecx                 \n\t"
				      "jl pixelLoopNeg2               \n\t"
				      "                               \n\t"
				      "popl %%esi                     \n\t"
				      "popl %%edi                     \n\t"
				      "popl %%edx                     \n\t"
				      "popl %%ecx                     \n\t"
				      "popl %%ebx                     \n\t"
				      "popl %%eax                     \n\t"
				      : /* no outputs */
				      : "g" (squareSize), "g" (dest), "g" (sourcePtr),
				      "g" (destStride), "g" (sourceRowAdd), "g" (squareSize)
				      : "eax", "ebx", "ecx", "edx", "esi", "edi", "cc", "memory" );
	} else {
		destStride = ( destStride + squareSize ) << 1;
		__asm__ __volatile__( "pushl %%eax                    \n\t"
				      "pushl %%ebx                    \n\t"
				      "pushl %%ecx                    \n\t"
				      "pushl %%edx                    \n\t"
				      "pushl %%edi                    \n\t"
				      "pushl %%esi                    \n\t"
				      "                               \n\t"
				      "movl %0, %%eax                 \n\t"
				      "movl %1, %%edi                 \n\t"
				      "movl %2, %%esi                 \n\t"
				      "leal (%%edi,%%eax,$2), %%edx   \n\t"
				      "movl $0, %%ecx                 \n\t"
				      "movl %3, %%ebx                 \n\t"
				      "                               \n"
				      "pixelLoop:                     \n\t"
				      "movw (%%esi,%%ebx), %%ax       \n\t"
				      "shll $16, %%eax                \n\t"
				      "addl $4, %%edi                 \n\t"
				      "movw (%%esi), %%ax             \n\t"
				      "leal (%%esi,%%ebx,$2), %%esi   \n\t"
				      "movl %%eax, -4(%%edi)          \n\t"
				      "cmpl %%edx, %%edi              \n\t"
				      "jnz pixelLoop                  \n\t"
				      "                               \n\t"
				      "incl %%ecx                     \n\t"
				      "subl %4, %%edi                 \n\t"
				      "movl %0, %%eax                 \n\t"
				      "addl %5, %%esi                 \n\t"
				      "leal (%%edi,%%eax,$2), %%edx   \n\t"
				      "cmpl %4, %%ecx                 \n\t"
				      "jl pixelLoop                   \n\t"
				      "                               \n\t"
				      "popl %%esi                     \n\t"
				      "popl %%edi                     \n\t"
				      "popl %%edx                     \n\t"
				      "popl %%ecx                     \n\t"
				      "popl %%ebx                     \n\t"
				      "popl %%eax                     \n\t"
				      : /* no outputs */
				      : "g" (squareSize), "g" (dest), "g" (sourcePtr),
				      "g" (sourceStep), "g" (destStride), "g" (sourceRowAdd)
				      : "eax", "ebx", "ecx", "edx", "esi", "edi", "cc", "memory" );
	}

}

void bitmapExtrude5551_asm( const void* srcMip, void* mip, U32 height, U32 width )
{
	const U16* src = static_cast<const U16*>( srcMip );
	U16* dst = static_cast<U16*>( mip );
	U32 stride = width << 1;

	for( U32 y = 0; y < height; y++ ) {

		for( U32 x = 0; x < width; x++ ) {
			U32 a = src[0];
			U32 b = src[1];
			U32 c = src[stride];
			U32 d = src[ stride + 1 ];
			dst[x] = ( ( ( ( a >> 11 ) + ( b >> 11 ) + ( c >> 11 ) + ( d >> 11 ) ) >> 2 ) << 11 ) |
				( ( ( ( ( a >> 6 ) & 0x1f ) + ( ( b >> 6 ) & 0x1f ) + ( ( c >> 6 ) & 0x1f ) +
				      ( ( d >> 6 ) & 0x1F ) ) >> 2 ) << 6 ) |
				( ( ( ( ( a >> 1 ) & 0x1F ) + ( ( b >> 1 ) & 0x1F ) + ( ( c >> 1 ) & 0x1f ) +
				      ( ( d >> 1 ) & 0x1f ) ) >> 2 ) << 1 );
			src += 2;
		}

		src += stride;
		dst += width;
	}

}

void bitmapExtrudeRGB_mmx( const void* srcMip, void* mip, U32 srcHeight, U32 srcWidth )
{

	if( srcHeight == 1 || srcWidth == 1 ) {
		bitmapExtrudeRGB_c( srcMip, mip, srcHeight, srcWidth );
		return;
	}

	U32 width = srcWidth >> 1;
	U32 height = srcHeight >> 1;

	if( width <= 1 ) {
		bitmapExtrudeRGB_c( srcMip, mip, srcHeight, srcWidth );
		return;
	}

	// now that we've taken care of our pathological cases...
	U64 ZERO = 0;
	const U8* src = static_cast<const U8*>( srcMip );
	U8* dst = static_cast<U8*>( mip );
	U32 srcStride = ( width << 1 ) * 3;
	U32 dstStride = width * 3;

	for( U32 y = 0; y < height; y++ ) {
		__asm__ __volatile__( "movl %0, %%eax                 \n\t"
				      "movl %%eax, %%ebx              \n\t"
				      "addl %1, %%ebx                 \n\t"
				      "movl %2, %%ecx                 \n\t"
				      "movl %3, %%edx                 \n\t"
				      "                               \n"
				      "0:                             \n\t"
				      "                               \n\t"
				      "punpcklbw (%%eax), %%mm0       \n\t"
				      "psrlw $8, %%mm0                \n\t"
				      "                               \n\t"
				      "punpcklbw 3(%%eax), %%mm1      \n\t"
				      "psrlw $8, %%mm1                \n\t"
				      "paddw %%mm1, %%mm0             \n\t"
				      "                               \n\t"
				      "punpcklbw (%%ebx), %%mm1       \n\t"
				      "psrlw $8, %%mm1                \n\t"
				      "paddw %%mm1, %%mm0             \n\t"
				      "                               \n\t"
				      "punpcklbw 3(%%ebx), %%mm1      \n\t"
				      "psrlw $8, %%mm1                \n\t"
				      "paddw %%mm1, %%mm0             \n\t"
				      "                               \n\t"
				      "psrlw $2, %%mm0                \n\t"
				      "packuswb %4, %%mm0             \n\t"
				      "                               \n\t"
				      "movd  %%mm0, (%%ecx)           \n\t"
				      "addl $6, %%eax                 \n\t"
				      "addl $6, %%ebx                 \n\t"
				      "addl $3, %%ecx                 \n\t"
				      "decl %%edx                     \n\t"
				      "jnz 0b                         \n\t"
				      : /* no outputs */
				      : "g" (src), "g" (srcStride), "g" (dst),
				      "g" (width), "m" (ZERO)
				      : "eax", "ebx", "ecx", "edx", "cc", "memory" );
		src += srcStride + srcStride;
		dst += dstStride;
	}
	__asm__ __volatile__( "emms \n\t" );

}

void bitmapConvertRGB_to_5551_mmx( U8*src, U32 pixels )
{
	U64 MULFACT = 0x0008200000082000;
	U64 REDBLUE = 0x00f800f800f800f8;
	U64 GREEN = 0x0000f8000000f800;
	U64 ALPHA = 0x0000000000010001;
	U64 ZERO = 0x0000000000000000;

	U32 evenPixels = pixels >> 1;
	U32 oddPixels = pixels & 1;
	U16* dst = reinterpret_cast<U16*>( src );

	if( evenPixels ) {
		__asm__ __volatile__( "movl %0, %%eax                 \n\t"
				      "movl %1, %%ebx                 \n\t"
				      "movl %2, %%edx                 \n\t"
				      "                               \n"
				      "pixel_loop2:                   \n\t"
				      "movd  (%%eax), %%mm0           \n\t"
				      "movd  3(%%eax), %%mm1          \n\t"
				      "punpckldq %%mm1, %%mm0         \n\t"
				      "movq  %%mm0, %%mm1             \n\t"
				      "pand %3, %%mm0                 \n\t"
				      "pmaddwd %4, %%mm0              \n\t"
				      "                               \n\t"
				      "pand %5, %%mm1                 \n\t"
				      "por %%mm1, %%mm0               \n\t"
				      "psrld $6, %%mm0                \n\t"
				      "packssdw %6, %%mm0             \n\t"
				      "pslld $1, %%mm0                \n\t"
				      "por %7, %%mm0                  \n\t"
				      "movd %%mm0, (%%ebx)            \n\t"
				      "                               \n\t"
				      "addl $6, %%eax                 \n\t"
				      "addl $4, %%ebx                 \n\t"
				      "decl %%edx                     \n\t"
				      "jnz pixel_loop2                \n\t"
				      "                               \n\t"
				      "movl %%eax, %0                 \n\t"
				      "movl %%ebx, %1                 \n\t"
				      "emms                           \n\t"
				      : /* no outputs */
				      : "g" (src), "g" (dst), "g" (evenPixels),
				      "m" (REDBLUE), "m" (MULFACT), "m" (GREEN),
				      "m" (ZERO), "m" (ALPHA)
				      : "eax", "ebx", "edx", "cc", "memory" );
	}

	if( oddPixels ) {
		U32 r = src[0] >> 3;
		U32 g = src[1] >> 3;
		U32 b = src[2] >> 3;

		*dst = ( b << 1 ) | ( g << 6 ) | ( r << 11 ) | 1;
	}

}

void PlatformBlitInit( void )
{
	bitmapExtrude5551 = bitmapExtrude5551_asm;
	bitmapExtrudeRGB = bitmapExtrudeRGB_c;

	if( Platform::SystemInfo.processor.properties & CPU_PROP_MMX ) {
		bitmapExtrudeRGB = bitmapExtrudeRGB_mmx;
		bitmapConvertRGB_to_5551 = bitmapConvertRGB_to_5551_mmx;
	}

	// Per 10/04/2000 change to PlatformWin32/linuxAsmBlit.cc
	//	terrMipBlit = terrMipBlit_asm;
}
