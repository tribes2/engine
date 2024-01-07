//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>

#include "platformLinux/platformLinux.h"

// dMalloc heap.
char* dStrdup_r( const char* s, const char *fileName, U32 lineNumber )
{
	char* sp = (char*) dMalloc_r( dStrlen( s ) + 1, fileName, lineNumber );
	dStrcpy( sp, s );
	return sp;
}

char* dStrcat( char* d, const char* s )
{
	return strcat( d, s );
}

char* dStrncat( char* d, const char* s, size_t n )
{
	return strncat( d, s, n );
}

S32 dStrcmp( const char* s1, const char* s2 )
{
	return strcmp( s1, s2 );
}

S32 dStricmp( const char* s1, const char* s2 )
{
	return strcasecmp( s1, s2 );
}

S32 dStrncmp( const char* s1, const char* s2, U32 n )
{
	return strncmp( s1, s2, n );
}

S32 dStrnicmp( const char* s1, const char* s2, U32 n )
{
	return strncasecmp( s1, s2, n );
}

char* dStrcpy( char* d, const char* s )
{
	return strcpy( d, s );
}

char* dStrncpy( char* d, const char* s, U32 n )
{
	return strncpy( d, s, n );
}

U32 dStrlen( const char* s )
{
	return strlen( s );
}

char* dStrupr( char* s )
{
	U32 l = dStrlen( s );

	for( int i = 0; i < l; i++ ) {
		s[i] = toupper( s[i] );
	}

	return s;
}

char* dStrlwr( char* s )
{
	U32 l = dStrlen( s );

	for( int i = 0; i < l; i++ ) {
		s[i] = tolower( s[i] );
	}

	return s;
}

char* dStrchr( char* s, S32 c )
{
	return strchr( s, c );
}

const char* dStrchr( const char* s, S32 c )
{
	return strchr( s, c );
}

char* dStrrchr( char* s, S32 c )
{
	return strrchr( s, c );
}

const char* dStrrchr( const char* s, S32 c )
{
	return strrchr( s, c );
}

U32 dStrspn( const char* s, const char* t )
{
	return strspn( s, t );
}

U32 dStrcspn( const char* s, const char* t )
{
	return strcspn( s, t );
}

char* dStrstr( char* s1, char* s2 )
{
	return strstr( s1, s2 );
}

const char* dStrstr( const char* s1, const char* s2 )
{
	return strstr( s1, s2 );
}

char* dStrtok( char* s, const char* t )
{
	return strtok( s, t );
}

S32 dAtoi( const char* s )
{
	return static_cast<S32>( atoi( s ) );
}

F32 dAtof( const char* s )
{
	double f = atof( s );

	if( isinf( f ) ) {
		return 0.0f;
	}

	return static_cast<F32>( f );
}

bool dAtob( const char* s )
{
	return !dStricmp( s, "true" ) || dAtof( s );
}

bool dIsalnum( const char c )
{
	return isalnum( c );
}

bool dIsalpha( const char c )
{
	return isalpha( c );
}

bool dIsspace( const char c )
{
	return isspace( c );
}

bool dIsdigit( const char c )
{
	return isdigit( c );
}

void dPrintf( const char* f, ... )
{
	va_list a;
	va_start( a, f );
	vprintf( f, a );
	va_end( a );
}

S32 dVprintf( const char* f, void* a )
{
	S32 l = vprintf( f, (va_list) a );
	// supposedly we're going to do something neat here.
	return l;
}

S32 dSprintf( char* b, U32 n, const char* f, ... )
{
	va_list a;
	va_start( a, f );
	S32 l = vsnprintf( b, n, f, a );
	va_end( a );
	return l;
}

S32 dVsprintf( char* b, U32 n, const char* f, void* a )
{
	S32 l = vsnprintf( b, n, f, (va_list) a );
	return l;
}

S32 dSscanf( const char* b, const char* f, ... )
{
	va_list a;
	va_start( a, f );
	S32 l = vsscanf( b, f, a );
	va_end( a );
	return l;
}

S32 dFflushStdout( void )
{
	return fflush( stdout );
}

S32 dFflushStderr( void )
{
	return fflush( stderr );
}

void dQsort( void* b, U32 n, U32 w, S32 (QSORT_CALLBACK* f)( const void*, const void* ) )
{
	qsort( b, n, w, f );
}
