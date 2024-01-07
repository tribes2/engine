//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "platformLinux/platformLinux.h"

#include <memory.h>
#include <stdlib.h>

void* dMemcpy( void* d, const void* s, U32 n )
{
	return memcpy( d, s, n );
}

void* dMemmove( void* d, const void* s, U32 n )
{
	return memmove( d, s, n );
}

void* dMemset( void* d, S32 c, U32 n )
{
	return memset( d, c, n );
}

S32 dMemcmp( const void* p1, const void* p2, U32 n )
{
	return memcmp( p1, p2, n );
}

void* dRealMalloc( dsize_t n )
{
	return malloc( n );
}

void dRealFree( void* p )
{
	free( p );
}
