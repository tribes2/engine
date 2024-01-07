//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include "platformLinux/platformLinux.h"

static U32 virtualTime = 0; 

#if !defined(__i386__)
#define USE_GETTIMEOFDAY
#endif

#ifdef USE_GETTIMEOFDAY

/* gettimeofday() based time implementation */

static struct timeval start;

void linuxInitTicks( void )
{
	gettimeofday( &start, 0 );
}

static inline U32 linuxGetTicks( void )
{
	struct timeval now;
	U32 ticks;

	gettimeofday( &now, 0 );
	ticks = ( now.tv_sec - start.tv_sec ) * 1000 + ( now.tv_usec - start.tv_usec ) / 1000;

	return ticks;
}

#else

/* rdtsc based time implementation */

static unsigned long long int start;
static unsigned long long int freq;

static unsigned int cpu_KHz(void)
{
	struct timeval t1, t2;
	unsigned long long int r1, r2;
	unsigned int z1, z2;
	unsigned int ms;
	int delta;
	const int max_delta = 1;
	int tries;
	const int max_tries = 20;

	tries = 0;
	z2 = 0;
	do {
		z1 = z2;
		gettimeofday(&t1, NULL);
		__asm__ __volatile__ ("rdtsc" : "=A" (r1));
		do {
			gettimeofday(&t2, NULL);
			ms = (t2.tv_sec-t1.tv_sec)*1000 + 
			     (t2.tv_usec-t1.tv_usec)/1000;
		} while ( ms < 100 );
		__asm__ __volatile__ ("rdtsc" : "=A" (r2));
		z2 = (r2-r1)/ms;
		if ( z2 > z1 ) {
			delta = (int)(z2 - z1);
		} else {
			delta = (int)(z1 - z2);
		}
	} while ( (delta > max_delta) && (tries++ < max_tries) );

	return(z2);
}

void linuxInitTicks( void )
{
	__asm__ ("rdtsc" : "=A" (start));
	freq = cpu_KHz();
}

static inline U32 linuxGetTicks( void )
{
	unsigned long long int ticks;

	__asm__ ("rdtsc" : "=A" (ticks));
	return (U32)((ticks-start)/freq);
}

#endif /* USE_GETTIMEOFDAY */


void Platform::getLocalTime( LocalTime &lt )
{
	struct tm *systime;
	time_t long_time;

	time( &long_time );
	systime = localtime( &long_time );

	lt.sec = systime->tm_sec;
	lt.min = systime->tm_min;
	lt.hour = systime->tm_hour;
	lt.month = systime->tm_mon;
	lt.monthday = systime->tm_mday;
	lt.weekday = systime->tm_wday;
	lt.year = systime->tm_year;
	lt.yearday = systime->tm_yday;
	lt.isdst = systime->tm_isdst;
}

U32 Platform::getTime( void )
{
	time_t when;
	time( &when );
	return static_cast<U32>( when );
}

U32 Platform::getRealMilliseconds( void )
{
	return linuxGetTicks( );
}

U32 Platform::getVirtualMilliseconds( void )
{
	return virtualTime;
}

void Platform::advanceTime( U32 delta )
{
	virtualTime += delta;
}
