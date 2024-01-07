//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _PLATFORMX86UNIX_H_
#define _PLATFORMX86UNIX_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif
#ifndef _EVENT_H_
#include "platform/event.h"
#endif

#include <stdio.h>
#include <string.h>

/* files needed for X11 */
//#include <X11/Xlib.h>
//#include <X11/Xutil.h>
//#include <X11/Xos.h>
//#include <X11/Xatom.h>
//#include <GL/glx.h>

class x86UNIXPlatformState
{
 public:
   S32 desktopBitsPixel;
   S32 desktopWidth;
   S32 desktopHeight;
   S32 width;
   S32 height;
   S32 bpp;
   bool videoInitted;
   U32 currentTime;
   struct __GLXcontextRec *ctx;
//   GLXContext ctx;

/* X11 specific */
//   Display   *display;
//   int       screen_num;
//   Window    win;
//   Screen    *screen_ptr;

   x86UNIXPlatformState( ) {
      width = 0;
      height = 0;
      bpp = 0;
      videoInitted = false;
      currentTime = 0;
   }
};

extern x86UNIXPlatformState x86UNIXState;

extern bool QGL_Init( const char* glName, const char* gluName );
extern bool QGL_EXT_Init( void );
extern void QGL_Shutdown( void );

extern void GetDesktopState( void );

extern char* dStrncat( char* d, const char* s, size_t n );

extern void x86UNIXInitTicks( void );

#ifdef DEDICATED
extern void x86UNIXPostQuitMessage( void );
#endif

extern void createFontInit( void );
extern void createFontShutdown( void );

extern void installRedBookDevices( void );

extern void PlatformBlitInit( void );

extern U8 getHatState( U8 hat );
extern void setHatState( U8 hat, U8 state );


#endif
