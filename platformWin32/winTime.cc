//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "PlatformWin32/platformWin32.h"
#include "time.h"

//--------------------------------------
void Platform::getLocalTime(LocalTime &lt)
{
   struct tm *systime;
   time_t long_time;

   time( &long_time );                // Get time as long integer.
   systime = localtime( &long_time ); // Convert to local time.
   
   lt.sec      = systime->tm_sec;      
   lt.min      = systime->tm_min;      
   lt.hour     = systime->tm_hour;     
   lt.month    = systime->tm_mon;    
   lt.monthday = systime->tm_mday; 
   lt.weekday  = systime->tm_wday;  
   lt.year     = systime->tm_year;     
   lt.yearday  = systime->tm_yday;  
   lt.isdst    = systime->tm_isdst;    
}

U32 Platform::getTime()
{
   time_t long_time;
   time( &long_time );
   return long_time;
}   

U32 Platform::getRealMilliseconds()
{
   return GetTickCount();
}   

U32 Platform::getVirtualMilliseconds()
{
   return winState.currentTime;   
}   

void Platform::advanceTime(U32 delta)
{
   winState.currentTime += delta;
}   




//------------------------------------------------------------------------------
//-------------------------------------- Linux Implementation
//
//
// static bool   sg_initialized = false;
// static U32 sg_secsOffset  = 0;
//
// //--------------------------------------
// U32
// Platform::getTickCount()
// {
//    // TODO: What happens when crossing a day boundary?
//    //
//    timeval t;
//
//    if (sg_initialized == false) {
//       sg_initialized = true;
//
//       gettimeofday(&t, NULL);
//       sg_secsOffset = t.tv_sec;
//    }
//
//    gettimeofday(&t, NULL);
//
//    U32 secs  = t.tv_sec - sg_secsOffset;
//    U32 uSecs = t.tv_usec;
//
//    // Make granularity 1 ms
//    return (secs * 1000) + (uSecs / 1000);
// }

