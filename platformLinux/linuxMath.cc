//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "Platform/platform.h"
#include "console/console.h"
#include "Math/mMath.h"

#include <stdlib.h>

extern "C" {
	void m_quatF_set_matF_ASM( F32 x, F32 y, F32 z, F32 w, F32* m );
	F32 m_matF_determinant_ASM( const F32* m );
	void m_matF_inverse_ASM( F32* m );
	void m_matF_affineInverse_ASM( F32* m );
	void m_matF_x_matF_ASM( const F32* a, const F32* b, F32* m );
	void m_matF_x_vectorF_ASM( const F32* m, const F32* v, F32* r );
	U32 mSolveQuadratic_ASM(F32 A, F32 B, F32 C, F32* x);
	U32 mSolveCubic_ASM(F32 A, F32 B, F32 C, F32 D, F32* x);
	U32 mSolveQuartic_ASM( F32 A, F32 B, F32 C, F32 D, F32 E, F32* x );
	S32 m_mulDivS32_ASM(S32 a, S32 b, S32 c);
	U32 m_mulDivU32_ASM(S32 a, S32 b, U32 c);

	// Added lots more math functions optimized by Visual C++
	void m_point2F_normalize_ASM(F32 *p);
	void m_point2F_normalize_f_ASM(F32 *p, F32 val);
	void m_point2D_normalize_ASM(F64 *p);
	void m_point2D_normalize_f_ASM(F64 *p, F64 val);
	void m_point3F_normalize_ASM(F32 *p);
	void m_point3F_normalize_f_ASM(F32 *p, F32 val);
	void m_point3F_interpolate_ASM(const F32 *from, const F32 *to, F32 factor, F32 *result );
	void m_point3D_normalize_ASM(F64 *p);
	void m_point3D_interpolate_ASM(const F64 *from, const F64 *to, F64 factor, F64 *result );
	void m_point3F_bulk_dot_ASM(const F32* refVector,
                                    const F32* dotPoints,
                                    const U32  numPoints,
                                    const U32  pointStride,
                                    F32*       output);
	void m_point3F_bulk_dot_indexed_ASM(const F32* refVector,
                                            const F32* dotPoints,
                                            const U32  numPoints,
                                            const U32  pointStride,
                                            const U32* pointIndices,
                                            F32*       output);
	void m_matF_identity_ASM(F32 *m);
	void m_matF_set_euler_ASM(const F32 *e, F32 *result);
	void m_matF_set_euler_point_ASM(const F32 *e, const F32 *p, F32 *result);
	void m_matF_transpose_ASM(F32 *m);
	void m_matF_scale_ASM(F32 *m, const F32 *p);
	void m_matF_normalize_ASM(F32 *m);
	void m_matF_x_point4F_ASM(const F32 *m, const F32 *p, F32 *presult);
}

extern void mInstallLibrary_C( void );

// drivers
static void m_quatF_set_matF_D( F32 x, F32 y, F32 z, F32 w, F32* m )
{
	AssertISV( !isnan( x ), "x == NaN in QuatF::setMatrix" );
	AssertISV( !isnan( y ), "y == NaN in QuatF::setMatrix" );
	AssertISV( !isnan( z ), "z == NaN in QuatF::setMatrix" );
	AssertISV( !isnan( w ), "w == NaN in QuatF::setMatrix" );

	m_quatF_set_matF_ASM( x, y, z, w, m );

	for( int i = 0; i < 16; i++ ) {
		AssertISV( !isnan( m[i] ), "m[i] == NaN in QuatF::setMatrix" );
	}

}

static F32 m_matF_determinant_D( const F32* m )
{

	for( int i = 0; i < 16; i++ ) {
		AssertISV( !isnan( m[i] ), "m[i] == NaN in MatrixF::determinant" );
	}

	F32 d =  m_matF_determinant_ASM( m );

	AssertISV( !isnan( d ), "d == NaN in MatrixF::determinant" );

	return d;
}

static void m_matF_inverse_D( F32* m )
{

	for( int i = 0; i < 16; i++ ) {
		AssertISV( !isnan( m[i] ), "m[i] == NaN in MatrixF::inverse" );
	}

	m_matF_inverse_ASM( m );

	for( int i = 0; i < 16; i++ ) {
		AssertISV( !isnan( m[i] ), "m[i] == NaN in MatrixF::inverse" );
	}


}

static void m_matF_affineInverse_D( F32* m )
{

	for( int i = 0; i < 16; i++ ) {
		AssertISV( !isnan( m[i] ), "m[i] == NaN in MatrixF::affineInverse" );
	}

	m_matF_affineInverse_ASM( m );

	for( int i = 0; i < 16; i++ ) {
		AssertISV( !isnan( m[i] ), "m[i] == NaN in MatrixF::affineInverse" );
	}

}

static void m_matF_x_matF_D( const F32* a, const F32* b, F32* m )
{

	for( int i = 0; i < 16; i++ ) {
		AssertISV( !isnan( a[i] ), "a[i] == NaN in MatrixF::mul" );
		AssertISV( !isnan( b[i] ), "b[i] == NaN in MatrixF::mul" );
	}

	m_matF_x_matF_ASM( a, b, m );

	for( int i = 0; i < 16; i++ ) {
		AssertISV( !isnan( m[i] ), "m[i] == NaN in MatrixF::mul" );
	}

}

static void m_matF_x_vectorF_D( const F32* m, const F32* v, F32* r )
{

	for( int i = 0; i < 16; i++ ) {
		AssertISV( !isnan( m[i] ), "m[i] == NaN in MatrixF::mul( VectorF )" );
	}

	for( int i = 0; i < 3; i++ ) {
		AssertISV( !isnan( v[i] ), "v[i] == NaN in MatrixF::mul( VectorF )" );
	}

	m_matF_x_vectorF_ASM( m, v, r );

	for( int i = 0; i < 3; i++ ) {
		AssertISV( !isnan( r[i] ), "r[i] == NaN in MatrixF::mul( VectorF )" );
	}

}

static U32 mSolveQuartic_D( F32 A, F32 B, F32 C, F32 D, F32 E, F32* x )
{
	AssertISV( !isnan( A ), "A == NaN in mSolveQuartic_D" );
	AssertISV( !isnan( B ), "B == NaN in mSolveQuartic_D" );
	AssertISV( !isnan( C ), "C == NaN in mSolveQuartic_D" );
	AssertISV( !isnan( D ), "D == NaN in mSolveQuartic_D" );
	AssertISV( !isnan( E ), "E == NaN in mSolveQuartic_D" );

	U32 n = mSolveQuartic_ASM( A, B, C, D, E, x );

	for( int i = 0; i < n; i++ ) {
		AssertISV( !isnan( x[i] ), "x[i] == NaN in mSolveQuartic_D" );
	}

	return n;
}

static void m_matF_transpose_D(F32 *m)
{
	for( int i = 0; i < 16; i++ ) {
		AssertISV( !isnan( m[i] ), "m[i] == NaN in MatrixF::transpose()" );
	}

	m_matF_transpose_ASM( m );

	for( int i = 0; i < 16; i++ ) {
		AssertISV( !isnan( m[i] ), "m[i] == NaN in MatrixF::transpose()" );
	}
}

static void checkASM( void )
{
	float i[] = { 1, 0, 0, 0,
		      0, 1, 0, 0,
		      0, 0, 1, 0,
		      0, 0, 0, 1 };
	float aff[] = { 0.1, 0, 0, 0.333,
			0, 0.2, 0, 0.666,
			0, 0, 0.3, 0.999,
			0, 0, 0, 1.0 };
	float inv[] = { 1, 3, 1, 1,
			2, 5, 2, 2,
			1, 3, 8, 9,
			1, 3, 2, 2 };
	float m[] = { 1, 2, 3, 4,
		      5, 6, 7, 8,
		      9, 10, 11, 12,
		      13, 14, 15, 16 };
	float v[] = { 1, 2 ,3 };
	float v2[] = { -1, 2, -0.333 };
	float a[16];
	float c[16];
	int j, k;

	for( int j = 0; j < 16; j++ ) {
		a[j] = 0.0f;
		c[j] = 0.0f;
	}

	m_matF_x_vectorF_ASM( inv, v, a );
	m_matF_x_vectorF( inv, v, c );

	AssertISV( dMemcmp( a, c, sizeof( a ) ) == 0, "m_matF_x_vectorF" );

	m_matF_x_vectorF_ASM( inv, v2, a );
	m_matF_x_vectorF( inv, v2, c );

	AssertISV( dMemcmp( a, c, sizeof( a ) ) == 0, "m_matF_x_vectorF" );

	dMemcpy( a, inv, sizeof( a ) );
	dMemcpy( c, inv, sizeof( c ) );
	m_matF_inverse_ASM( a );
	m_matF_inverse( c );

	AssertISV( dMemcmp( a, c, sizeof( a ) ) == 0, "m_matF_inverse" );

	m_quatF_set_matF_ASM( 1, 2, 3, 4, a );
	m_quatF_set_matF( 1, 2, 3, 4, c );

	AssertISV( dMemcmp( a, c, sizeof( a ) ) == 0, "m_quatF_set_matF" );

	a[0] = m_matF_determinant_ASM( inv );
	c[0] = m_matF_determinant_ASM( inv );

	AssertISV( a[0] == c[0], "m_matF_determinant" );

	dMemcpy( a, aff, sizeof( a ) );
	dMemcpy( c, aff, sizeof( c ) );
	m_matF_affineInverse_ASM( a );
	m_matF_affineInverse( c );

	AssertISV( dMemcmp( a, c, sizeof( a ) ) == 0, "m_matF_affineInverse" );

	m_matF_x_matF_ASM( m, m, a );
	m_matF_x_matF( m, m, c );

	AssertISV( dMemcmp( a, c, sizeof( a ) ) == 0, "m_matF_x_matF" );

#if 0 // Code generated with Visual C++ gives slighly different results
	j = mSolveQuartic_ASM( 1.0, 2.0, 3.0, 4.0, 5.0, a );
	k = mSolveQuartic( 1.0, 2.0, 3.0, 4.0, 5.0, c );

	AssertISV( dMemcmp( a, c, sizeof( a ) ) == 0, "mSolveQuartic" );
	AssertISV( j == k, "mSolveQuartic" );

	j = mSolveQuartic_ASM( 0.0, 1.0, 2.0, 3.0, 4.0, a );
	k = mSolveQuartic( 0.0, 1.0, 2.0, 3.0, 4.0, c );

	AssertISV( dMemcmp( a, c, sizeof( a ) ) == 0, "mSolveQuartic" );
	AssertISV( j == k, "mSolveQuartic" );
#endif
}

void mInstallLibrary_ASM( void )
{
	// Experimentally determined by 10,000,000 iteration benchmarks
	U32 asm_flags = 0x027FC4F3;
	U32 dbg_flags = 0x00000000;

	if ( getenv("TRIBES2_ASM_FLAGS") ) {
		asm_flags = strtol(getenv("TRIBES2_ASM_FLAGS"), NULL, 0);
		if ( ! asm_flags ) {
			return;
		}
	}
	if ( getenv("TRIBES2_ASM_DEBUG") ) {
		dbg_flags = strtol(getenv("TRIBES2_ASM_DEBUG"), NULL, 0);
		if ( dbg_flags ) {
			checkASM( );
		}
	}

	if ( asm_flags & (1<<0) ) {
		m_mulDivS32 = m_mulDivS32_ASM;
		m_mulDivU32 = m_mulDivU32_ASM;
	}
	if ( asm_flags & (1<<1) ) {
		if ( dbg_flags & (1<<1) )
			m_quatF_set_matF = m_quatF_set_matF_D;
		else
			m_quatF_set_matF = m_quatF_set_matF_ASM;
	}
	if ( asm_flags & (1<<2) ) {
		if ( dbg_flags & (1<<2) )
			m_matF_determinant = m_matF_determinant_D;
		else
			m_matF_determinant = m_matF_determinant_ASM;
	}
	if ( asm_flags & (1<<3) ) {
		if ( dbg_flags & (1<<3) )
			m_matF_inverse = m_matF_inverse_D;
		else
			m_matF_inverse = m_matF_inverse_ASM;
	}
	if ( asm_flags & (1<<4) ) {
		if ( dbg_flags & (1<<4) )
			m_matF_affineInverse = m_matF_affineInverse_D;
		else
			m_matF_affineInverse = m_matF_affineInverse_ASM;
	}
	if ( asm_flags & (1<<5) ) {
		if ( dbg_flags & (1<<5) )
			m_matF_x_matF = m_matF_x_matF_D;
		else
			m_matF_x_matF = m_matF_x_matF_ASM;
	}
	if ( asm_flags & (1<<6) ) {
		mSolveQuadratic = mSolveQuadratic_ASM;
		mSolveCubic = mSolveCubic_ASM;
	}
	if ( asm_flags & (1<<7) ) {
		if ( dbg_flags & (1<<7) )
			mSolveQuartic = mSolveQuartic_D;
		else
			mSolveQuartic = mSolveQuartic_ASM;
	}

	// Added lots more math functions optimized by Visual C++
	if ( asm_flags & (1<<8) )
		m_point2F_normalize     = m_point2F_normalize_ASM;
	if ( asm_flags & (1<<9) )
		m_point2F_normalize_f   = m_point2F_normalize_f_ASM;
	if ( asm_flags & (1<<10) )
		m_point2D_normalize     = m_point2D_normalize_ASM;
	if ( asm_flags & (1<<11) )
		m_point2D_normalize_f   = m_point2D_normalize_f_ASM;
	if ( asm_flags & (1<<12) )
		m_point3F_normalize     = m_point3F_normalize_ASM;
	if ( asm_flags & (1<<13) )
		m_point3F_normalize_f   = m_point3F_normalize_f_ASM;
	if ( asm_flags & (1<<14) )
		m_point3F_interpolate   = m_point3F_interpolate_ASM;

	if ( asm_flags & (1<<15) )
		m_point3D_normalize     = m_point3D_normalize_ASM;
	if ( asm_flags & (1<<16) )
		m_point3D_interpolate   = m_point3D_interpolate_ASM;

	if ( asm_flags & (1<<17) )
		m_point3F_bulk_dot      = m_point3F_bulk_dot_ASM;
	if ( asm_flags & (1<<18) )
		m_point3F_bulk_dot_indexed = m_point3F_bulk_dot_indexed_ASM;

	if ( asm_flags & (1<<19) )
		m_matF_set_euler        = m_matF_set_euler_ASM;
	if ( asm_flags & (1<<20) )
		m_matF_set_euler_point  = m_matF_set_euler_point_ASM;
	if ( asm_flags & (1<<21) )
		m_matF_identity         = m_matF_identity_ASM;
	if ( asm_flags & (1<<22) ) {
		if ( dbg_flags & (1<<22) )
			m_matF_transpose        = m_matF_transpose_D;
		else
			m_matF_transpose        = m_matF_transpose_ASM;
	}
	if ( asm_flags & (1<<23) )
		m_matF_scale            = m_matF_scale_ASM;
	if ( asm_flags & (1<<24) )
		m_matF_normalize        = m_matF_normalize_ASM;
	if ( asm_flags & (1<<25) )
		m_matF_x_point4F        = m_matF_x_point4F_ASM;
}

static void cMathInit( SimObject* obj, S32 argc, const char** argv )
{
	U32 properties = CPU_PROP_C;
   
	if( argc == 1 ) {
		Math::init( 0 );
		return;
	}

	for( argc--, argv++; argc; argc--, argv++ ) {

		if( dStricmp( *argv, "DETECT" ) == 0 ) { 
			Math::init( 0 );
			return;
		}

		if( dStricmp( *argv, "C" ) == 0 ) { 
			properties |= CPU_PROP_C; 
			continue; 
		}

		if( dStricmp( *argv, "FPU" ) == 0 ) { 
			properties |= CPU_PROP_FPU; 
			continue; 
		}

		if( dStricmp( *argv, "MMX" ) == 0 ) { 
			properties |= CPU_PROP_MMX; 
			continue; 
		}

		if( dStricmp( *argv, "3DNOW" ) == 0 ) { 
			properties |= CPU_PROP_3DNOW; 
			continue; 
		}

		if( dStricmp( *argv, "SSE" ) == 0 ) { 
			properties |= CPU_PROP_SSE; 
			continue;
		}

		Con::printf( "Error: MathInit(): ignoring unknown math extension '%s'", *argv );
	}

	Math::init( properties );
}

void Math::init( U32 properties )
{
	Con::addCommand( "MathInit", cMathInit, "MathInit( detect|C|FPU|MMX|3DNOW|SSE|... )", 1, 10 );

	if( !properties ) {
		properties = Platform::SystemInfo.processor.properties;
	} else {
		properties &= Platform::SystemInfo.processor.properties;
	}


	Con::printf( "Math Init:" );
	Con::printf( "    Installing Standard C extensions" );
	mInstallLibrary_C( );

	Con::printf( "    Installing Assembly extensions" );
	mInstallLibrary_ASM( );

	if( properties & CPU_PROP_FPU ) {
		Con::printf( "    Installing FPU extensions" );
	}

	if( properties & CPU_PROP_MMX ) {
		Con::printf( "    Installing MMX extensions" );
	}

	if( properties & CPU_PROP_3DNOW ) {
		Con::printf( "    Installing 3DNow! extensions" );
	}

	if( properties & CPU_PROP_SSE ) {
		Con::printf( "    Installing SSE extensions" );
	}

}
