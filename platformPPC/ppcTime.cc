//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "PlatformPPC/platformPPC.h"
#include <time.h>
#include <timer.h>

//--------------------------------------
void Platform::getLocalTime(LocalTime &lt)
{
    #pragma message("todo: Platform::getLocalTime")
//   struct tm *systime;
//   time_t long_time;
//
//   time( &long_time );                // Get time as long integer.
//   systime = localtime( &long_time ); // Convert to local time.
//   
//   lt.sec      = systime->tm_sec;      
//   lt.min      = systime->tm_min;      
//   lt.hour     = systime->tm_hour;     
//   lt.month    = systime->tm_mon;    
//   lt.monthday = systime->tm_mday; 
//   lt.weekday  = systime->tm_wday;  
//   lt.year     = systime->tm_year;     
//   lt.yearday  = systime->tm_yday;  
//   lt.isdst    = systime->tm_isdst;    

   lt.sec      = 0;      
   lt.min      = 0;      
   lt.hour     = 0;     
   lt.month    = 1;    
   lt.monthday = 1; 
   lt.weekday  = 0;  
   lt.year     = 1999;     
   lt.yearday  = 1;  
   lt.isdst    = 0;    
}   


U32 GetMilliseconds()
{
	UnsignedWide time;
	Microseconds(&time);
	return (time.hi*5000000) + (time.lo/1000);
}

U32 Platform::getRealMilliseconds()
{
   UnsignedWide time;
   Microseconds(&time);
   return (time.hi*5000000) + (time.lo/1000);
}   

U32 Platform::getVirtualMilliseconds()
{
   return ppcState.currentTime;   
}   

void Platform::advanceTime(U32 delta)
{
   ppcState.currentTime += delta;
}   

