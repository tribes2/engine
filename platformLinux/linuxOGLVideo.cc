//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef DEDICATED

#include <stdlib.h>
#include <assert.h>

#include <SDL/SDL.h>

#include "PlatformWin32/platformGL.h"
#include "platformLinux/platformLinux.h"
#include "Platform/platformAudio.h"
#include "platformLinux/linuxOGLVideo.h"
#include "console/console.h"
#include "Math/mPoint.h"
#include "Platform/event.h"
#include "Platform/gameInterface.h"
#include "console/consoleInternal.h"
#include "console/ast.h"
#include "Core/fileStream.h"

//------------------------------------------------------------------------------

struct CardProfile
{
   const char *vendor;     // manufacturer
   const char *renderer;   // driver name

   bool safeMode;          // destroy rendering context for resolution change
   bool lockArray;         // allow compiled vertex arrays
   bool subImage;          // allow glTexSubImage*
   bool fogTexture;        // require bound texture for combine extension
   bool noEnvColor;        // no texture environment color
   bool clipHigh;          // clip high resolutions
   bool deleteContext;	   // delete rendering context
   bool texCompress;	   // allow texture compression
   bool interiorLock;	   // lock arrays for Interior render
   bool skipFirstFog;	   // skip first two-pass fogging (dumb 3Dfx hack)
   bool only16;		   // inhibit 32-bit resolutions
   bool noArraysAlpha;     // don't use glDrawArrays with a GL_ALPHA texture

   const char *proFile;	   // explicit profile of graphic settings
};

static Vector<CardProfile> sCardProfiles(__FILE__, __LINE__);

struct ProcessorProfile
{
    U16 clock;  // clock range max
    U16 adjust; // CPU adjust   
};

static U8 sNumProcessors = 4;
static ProcessorProfile sProcessorProfiles[] =
{ 
    {  400,  0 },
    {  600,  5 },
    {  800, 10 },
    { 1000, 15 },
};

struct SettingProfile
{
    U16 performance;        // metric range max
    const char *settings;   // default file
};    

static U8 sNumSettings = 3;
static SettingProfile sSettingProfiles[] =
{
    {  33, "LowProfile.cs" },
    {  66, "MediumProfile.cs" },
    { 100, "HighProfile.cs" }, 
};

//------------------------------------------------------------------------------

static void cAddCardProfile(SimObject *, S32, const char **argv)
{
   CardProfile profile;

   profile.vendor = dStrdup(argv[1]);
   profile.renderer = dStrdup(argv[2]);

   profile.safeMode = dAtob(argv[3]);   
   profile.lockArray = dAtob(argv[4]);
   profile.subImage = dAtob(argv[5]);
   profile.fogTexture = dAtob(argv[6]);
   profile.noEnvColor = dAtob(argv[7]);
   profile.clipHigh = dAtob(argv[8]);
   profile.deleteContext = dAtob(argv[9]);
   profile.texCompress = dAtob(argv[10]);
   profile.interiorLock = dAtob(argv[11]);
   profile.skipFirstFog = dAtob(argv[12]);
   profile.only16 = dAtob(argv[13]);
   profile.noArraysAlpha = dAtob(argv[14]);

   if (strcmp(argv[15],""))
      profile.proFile = dStrdup(argv[15]);
   else
      profile.proFile = NULL;

   sCardProfiles.push_back(profile);
}

static void clearCardProfiles()
{
   while (sCardProfiles.size())
   {
      dFree((char *) sCardProfiles.last().vendor);
      dFree((char *) sCardProfiles.last().renderer);

		dFree((char *) sCardProfiles.last().proFile);

      sCardProfiles.decrement();
   }
}

static void execScript(const char *scriptFile)
{
	// execute the script
   FileStream str;

   if (!str.open(scriptFile, FileStream::Read))
      return;

   U32 size = str.getStreamSize();
   char *script = new char[size + 1];

   str.read(size, script);
   str.close();

   script[size] = 0;
   Con::executef(2, "eval", script);
   delete[] script;
}

static void profileSystem(const char *vendor, const char *renderer)
{
   Con::addCommand("addCardProfile", cAddCardProfile, "addCardProfile(vendor,renderer,safeMode,lockArray,subImage,fogTexture,noEnvColor,clipHigh,deleteContext,texCompress,interiorLock,skipFirstFog,only16,noArraysAlpha,proFile);", 16, 16); 

   execScript("CardProfiles.cs");

   const char *arch = "";
   const char *os = "Linux";
   char osProfiles[64];

   if ( os != NULL )
   {
      dSprintf(osProfiles,64,"%s%sCardProfiles.cs",arch,os);
      //Con::executef(2, "exec", osProfiles);
      execScript(osProfiles);
   }

   const char *proFile = NULL;
   U32 i;
   
   for (i = 0; i < sCardProfiles.size(); ++i)
      if (dStrstr(vendor, sCardProfiles[i].vendor) &&
          (!dStrcmp(sCardProfiles[i].renderer, "*") ||
           dStrstr(renderer, sCardProfiles[i].renderer)))
      {         
         Con::setBoolVariable("$pref::Video::safeModeOn", sCardProfiles[i].safeMode);
         Con::setBoolVariable("$pref::OpenGL::disableEXTCompiledVertexArray", !sCardProfiles[i].lockArray);
         Con::setBoolVariable("$pref::OpenGL::disableSubImage", !sCardProfiles[i].subImage);
         Con::setBoolVariable("$pref::TS::fogTexture", sCardProfiles[i].fogTexture);
         Con::setBoolVariable("$pref::OpenGL::noEnvColor", sCardProfiles[i].noEnvColor);
         Con::setBoolVariable("$pref::Video::clipHigh", sCardProfiles[i].clipHigh);
         Con::setBoolVariable("$pref::Video::deleteContext", true);
         Con::setBoolVariable("$pref::OpenGL::disableARBTextureCompression", !sCardProfiles[i].texCompress);
         Con::setBoolVariable("$pref::Interior::lockArrays", sCardProfiles[i].interiorLock);
         Con::setBoolVariable("$pref::TS::skipFirstFog", sCardProfiles[i].skipFirstFog);
         Con::setBoolVariable("$pref::Video::only16", sCardProfiles[i].only16);
         Con::setBoolVariable("$pref::OpenGL::noDrawArraysAlpha", sCardProfiles[i].noArraysAlpha);

         proFile = sCardProfiles[i].proFile;

         break;   
      }

   // defaults
   U16 glProfile;

   if (!proFile)
   {
      // no driver GL profile -- make one via weighting GL extensions
      glProfile = 25;

      glProfile += gGLState.suppARBMultitexture * 25;
      glProfile += gGLState.suppLockedArrays * 15;
      glProfile += gGLState.suppVertexArrayRange * 10;
      glProfile += gGLState.suppTextureEnvCombine * 5;
      glProfile += gGLState.suppPackedPixels * 5;
      glProfile += gGLState.suppTextureCompression * 5;
      glProfile += gGLState.suppS3TC * 5;
      glProfile += gGLState.suppFXT1 * 5;

      Con::setBoolVariable("$pref::Video::safeModeOn", true);
      Con::setBoolVariable("$pref::OpenGL::disableEXTCompiledVertexArray", false);
      Con::setBoolVariable("$pref::OpenGL::disableSubImage", false); 
      Con::setBoolVariable("$pref::TS::fogTexture", false);
      Con::setBoolVariable("$pref::OpenGL::noEnvColor", false);
      Con::setBoolVariable("$pref::Video::clipHigh", false);
      Con::setBoolVariable("$pref::Video::deleteContext", true);
      Con::setBoolVariable("$pref::OpenGL::disableARBTextureCompression", false);
      Con::setBoolVariable("$pref::Interior::lockArrays", true);
      Con::setBoolVariable("$pref::TS::skipFirstFog", false);
      Con::setBoolVariable("$pref::Video::only16", false);
      Con::setBoolVariable("$pref::OpenGL::noDrawArraysAlpha", false);
   }

   Con::setVariable("$pref::Video::profiledVendor", vendor);
   Con::setVariable("$pref::Video::profiledRenderer", renderer);

   if (!Con::getBoolVariable("$DisableSystemProfiling") &&
       ( dStrcmp(vendor, Con::getVariable("$pref::Video::defaultsVendor")) ||
          dStrcmp(renderer, Con::getVariable("$pref::Video::defaultsRenderer")) ))
   {
      if (proFile)
      {
         char settings[64];

         dSprintf(settings,64,"%s.cs",proFile);
         //Con::executef(2, "exec", settings);
         execScript(settings);
      }
      else
      {
         U16 adjust;

         // match clock with profile
         for (i = 0; i < sNumProcessors; ++i)
         {
            adjust = sProcessorProfiles[i].adjust;

            if (Platform::SystemInfo.processor.mhz < sProcessorProfiles[i].clock) break;
         }

         const char *settings;

         // match performance metric with profile
         for (i = 0; i < sNumSettings; ++i)
         {
            settings = sSettingProfiles[i].settings;

            if (glProfile+adjust <= sSettingProfiles[i].performance) break;
         }

         //Con::executef(2, "exec", settings);
         execScript(settings);
      }

      Con::setVariable("$pref::Video::defaultsVendor", vendor);
      Con::setVariable("$pref::Video::defaultsRenderer", renderer);
   }

   // write out prefs
   gEvalState.globalVars.exportVariables("$pref::*", "prefs/ClientPrefs.cs", false);

   clearCardProfiles();
}    

OpenGLDevice::OpenGLDevice( void )
{
	initDevice( );
}

void OpenGLDevice::initDevice( void )
{
	mDeviceName = "OpenGL";
	mFullScreenOnly = false;

	// generate valid resolutions
	mResolutionList.clear( );

	if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		// Yes, of course this is bad. But we
		// need to support it for the dedicated
		// codepath in the full client, and we
		// do an SDL_Init( SDL_INIT_VIDEO ) in
		// the ::activate code, anyway.
		return;
	}

	// make sure we have desktop info
	GetDesktopState( );
	SDL_Rect** rects = SDL_ListModes( 0, 0 );

	if( rects == 0 ) {
		// eek
		AssertFatal( 0, "No video modes available" );
		Con::errorf( ConsoleLogEntry::General,
			     "No video modes available, exiting..." );
		Platform::forceShutdown( 1 );
	} else if( rects == (SDL_Rect**) -1 ) {
		// okay, we're not fullscreen, so technically any
		// dimension is possible, so let's just use some
		// nice defaults
		int defaults[5][2] = { { 640, 480 },
				       { 800, 600 },
				       { 1024, 768 },
				       { 1280, 1024 },
				       { 1600, 1200 } };

		for( int i = 0; i < 5; i++ ) {
			int w = defaults[i][0];
			int h = defaults[i][1];

			// but we don't want them too big
			if( w <= linuxState.width && h <= linuxState.height ) {
				Resolution rez( w, h, linuxState.bpp );
				mResolutionList.push_back( rez );
			}

		}

	} else {

		for( int i = 0; rects[i] != 0; i++ ) {
			Resolution rez( rects[i]->w, rects[i]->h, linuxState.bpp );
			mResolutionList.push_back( rez );
		}

	}

	if( getenv( "MESA_GLX_FX" ) == 0 ) {
		putenv( "MESA_GLX_FX=f" );
	}

	if( getenv( "MESA_FX_NO_SIGNALS" ) == 0 ) {
		putenv( "MESA_FX_NO_SIGNALS=1" );
	}

}

bool OpenGLDevice::activate( U32 width, U32 height, U32 bpp, bool fullScreen )
{
	const char* libraryName = Con::getVariable( "$pref::OpenGL::driver" );
	assert( libraryName );

	if( !dStrcmp( libraryName, "" ) ) {
		libraryName = "libGL.so.1";
	}

	{ static bool loaded_gl = false;

		if( ! loaded_gl && !QGL_Init( libraryName, "libGLU.so.1" ) ) {
			return false;
		}
		loaded_gl = true;
	}

	if( !setScreenMode( width, height, bpp, fullScreen, true, false ) ) {
		return false;
	}

	const char* vendorString = (const char*) glGetString( GL_VENDOR );
	const char* rendererString = (const char*) glGetString( GL_RENDERER );
	const char* versionString = (const char*) glGetString( GL_VERSION );

	Con::printf( "OpenGL driver information:" );

	if( vendorString ) {
		Con::printf( "    Vendor: %s", vendorString );
	}

	if( rendererString ) {
		Con::printf( "    Renderer: %s", rendererString );
	}

	if( versionString ) {
		Con::printf( "    Version: %s", versionString );
	}

	QGL_EXT_Init( );

	Con::setVariable( "$pref::Video::displayDevice", mDeviceName );

	if( Con::getBoolVariable( "$DisableSystemProfiling" ) ) {
		return true;
	}

	// profile on changed display device (including first time)
	if( dStrcmp( vendorString, Con::getVariable( "$pref::Video::profiledVendor" ) ) ||
	    dStrcmp( rendererString, Con::getVariable( "$pref::Video::profiledRenderer" ) ) ) {
		bool fullscreen = Con::getBoolVariable( "$pref::Video::fullScreen", true );

		profileSystem( vendorString, rendererString );

		U32 width, height, bpp;
		const char* resolution = Con::getVariable( "$pref::Video::resolution" );

		// restart the system now that we've possibly changed
		// things from the exec calls in profileSystem().
		dSscanf( resolution, "%d %d %d", &width, &height, &bpp );
		setScreenMode( width, height, bpp, fullscreen, false, false );
	}

	return true;
}

void OpenGLDevice::shutdown( void )
{
	Con::printf( "Shutting down video subsystem..." );

	linuxState.videoInitted = false;
}

bool OpenGLDevice::setScreenMode( U32 width,
				  U32 height,
				  U32 bpp,
				  bool fullScreen,
				  bool forceIt,
				  bool repaint )
{
	bool needResurrect = false;

	// shutdown if we're already up
	if( linuxState.videoInitted ) {
		Con::printf( "Killing the texture manager..." );
		Game->textureKill( );
		needResurrect = true;
	}

	if( bpp != 16 && bpp != 32 ) {
		// Occurs because T2 passes 'Default' to atoi()
		// which is passed down as the bpp. Is usually
		// converted to '0', but we'll play it safe.
		bpp = linuxState.bpp;
	}

	int flags = SDL_OPENGL;

	if( fullScreen ) {
		flags |= SDL_FULLSCREEN;
	}

	Con::setBoolVariable( "$pref::Video::fullScreen", fullScreen );
	Con::printf( "Setting video mode to %d %d %d (%s)...",
		     width, height, bpp,
		     ( fullScreen ? "fs" : "w" ) );

	// these are the deafults in SDL, actually
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	if( SDL_SetVideoMode( width, height, bpp, flags ) == 0 ) {
		return false;
	}

	linuxState.videoInitted = true;

	if( needResurrect ) {
		Con::printf( "Resurrecting the texture manager..." );
		Game->textureResurrect( );
	}

	// Save the current resolution for fullscreen toggling
	if ( SDL_GetVideoSurface()->flags & SDL_FULLSCREEN ) {
		smIsFullScreen = true;
	} else {
		smIsFullScreen = false;
	}
	smCurrentRes = Resolution( width, height, bpp );

	Platform::setWindowSize( smCurrentRes.w, smCurrentRes.h );
	char buffer[16];
	dSprintf( buffer, sizeof( buffer ), "%d %d %d",
	          smCurrentRes.w, smCurrentRes.h, smCurrentRes.bpp );
	Con::setVariable( "$pref::Video::resolution", buffer );

	if( repaint ) {
		Con::evaluate( "resetCanvas();" );
	}

	return true;
}

void OpenGLDevice::swapBuffers( void )
{
	SDL_GL_SwapBuffers( );
}

const char* OpenGLDevice::getDriverInfo( void )
{
	const char* vendor = (const char*) glGetString( GL_VENDOR );
	const char* renderer = (const char*) glGetString( GL_RENDERER );
	const char* version = (const char*) glGetString( GL_VERSION );
	const char* extensions = (const char*) glGetString( GL_EXTENSIONS );

	U32 length = 4;

	if( vendor ) {
		length += dStrlen( vendor );
	}

	if( renderer ) {
		length += dStrlen( renderer );
	}

	if( version ) {
		length += dStrlen( version );
	}

	if( extensions ) {
		length += dStrlen( extensions );
	}

	char* info = Con::getReturnBuffer( length );

	dSprintf( info, length,  "%s\t%s\t%s\t%s",
		  ( vendor ? vendor : "" ),
		  ( renderer ? renderer : "" ),
		  ( version ? version : "" ),
		  ( extensions ? extensions : "" ) );

	return info;
}

extern "C" int SDL_GetGamma( float* red, float* green, float* blue );

bool OpenGLDevice::getGammaCorrection( F32& gamma )
{
#ifdef USE_GAMMA_RAMPS
	U16 red[256];
	U16 green[256];
	U16 blue[256];

	if( SDL_GetGammaRamp( red, green, blue ) == -1 ) {
		return false;
	}

	F32 sum = 0.0f;
	U32 count = 0;

	for( U16 i = 1; i < 256; i++ ) {

		if( red[i] != 0 && red[i] != 65535 ) {
			F64 b = i / 256.0;
			F64 a = red[i] / 65535.0;
			F32 c = mLog( a ) / mLog( b );

			sum += c;
			count++;
		}

	}

	gamma = sum / count;

	return true;
#else
	F32 red = 0;
	F32 green = 0;
	F32 blue = 0;

	int result = SDL_GetGamma( &red, &green, &blue );

	gamma = ( red + green + blue ) / 3.0f;

	return ( result != -1 );
#endif
}

bool OpenGLDevice::setGammaCorrection( F32 gamma )
{
#ifdef USE_GAMMA_RAMPS
	U16 red[256];
	U16 green[256];
	U16 blue[256];

	for( U16 i = 0; i < 256; i++ ) {
		red[i] = mPow( static_cast<F32>( i ) / 256.0f, gamma ) * 65535.0f;
	}

	dMemcpy( green, red, sizeof( green ) );
	dMemcpy( blue, red, sizeof( blue ) );

	int result = SDL_SetGammaRamp( red, green, blue );

	return ( result != -1 );
#else
	// no idea why these are foo
	gamma = 2.0f - gamma;
	return ( SDL_SetGamma( gamma, gamma, gamma ) );
#endif
}

bool OpenGLDevice::setVerticalSync( bool on )
{
   // Implement this is you want to...
   on;
   return( false );
}

DisplayDevice* OpenGLDevice::create( void )
{
	// NOTE: We're skipping all that awful hoodoo about checking
	// for a fullscreen only implementation.
	OpenGLDevice* device = new OpenGLDevice( );
	return device;
}

#endif
