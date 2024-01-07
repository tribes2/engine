//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _PLATFORMLINUX_H_
#define _PLATFORMLINUX_H_

#include <stdio.h>
#include <string.h>
#include <SDL/SDL.h>

#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif
#ifndef _EVENT_H_
#include "Platform/event.h"
#endif

class LinuxPlatformState
{
 public:
	S32 width;
	S32 height;
	S32 bpp;
	bool videoInitted;

	LinuxPlatformState( ) {
		width = 0;
		height = 0;
		bpp = 0;
		videoInitted = false;
	}
};

extern LinuxPlatformState linuxState;

extern bool QGL_Init( const char* glName, const char* gluName );
extern bool QGL_EXT_Init( void );
extern void QGL_Shutdown( void );

extern void GetDesktopState( void );

extern char* dStrncat( char* d, const char* s, size_t n );

extern void linuxInitTicks( void );

#ifdef DEDICATED
extern void linuxPostQuitMessage( void );
#endif

extern void createFontInit( void );
extern void createFontShutdown( void );

extern void installRedBookDevices( void );

extern void PlatformBlitInit( void );

extern U8 getHatState( U8 hat );
extern void setHatState( U8 hat, U8 state );

extern SDLKey translateKeyCodeToSDL( KeyCodes code );
extern KeyCodes translateSDLToKeyCode( SDLKey sym );

#endif
