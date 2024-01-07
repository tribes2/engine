//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _LINUXOGLVIDEO_H_
#define _LINUXOGLVIDEO_H_

#ifndef _PLATFORMVIDEO_H_
#include "Platform/platformVideo.h"
#endif

class OpenGLDevice : public DisplayDevice
{
 public:
	OpenGLDevice( );

	void initDevice( void );
	bool activate( U32 width, U32 height, U32 bpp, bool fullScreen );
	void shutdown( void );
	void destroy( void );
	bool setScreenMode( U32 width,
			    U32 height,
			    U32 bpp,
			    bool fullScree,
			    bool forceIt = false,
			    bool repaint = true );
	void swapBuffers( void );

	const char* getDriverInfo( void );
	bool getGammaCorrection( F32& gamma );
	bool setGammaCorrection( F32 gamma );
   bool setVerticalSync( bool on );

	static DisplayDevice* create( void );
};

#endif
