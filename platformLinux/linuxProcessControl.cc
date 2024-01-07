//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include <SDL/SDL.h>
#include <stdlib.h>
#include <unistd.h>

#include "platformLinux/platformLinux.h"
#include "console/console.h"

void Platform::postQuitMessage( const U32 value )
{
#ifdef DEDICATED
	linuxPostQuitMessage( );
#else
	SDL_Event event;
	event.type = SDL_QUIT;
	SDL_PushEvent( &event );
#endif
}

void Platform::debugBreak( void )
{
	__asm__( "int $03" );
}

void Platform::forceShutdown( S32 value )
{
	// We have to use _exit() because of really bad
	// global constructor/destructor mojo they have.
#ifdef DEDICATED
	_exit( value );
#else
	SDL_Quit( );
	_exit( value );
#endif
}
