//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <assert.h>

#include <fpu_control.h>

#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include "loki_utils.h"
#include "sdl_utils.h"

#include "engine/platform/platformInput.h"
#include "engine/platformLinux/platformLinux.h"
#include "engine/platform/platform.h"
#include "engine/platformWIN32/platformGL.h"
#include "engine/platformWIN32/platformAL.h"
#include "engine/platform/platformVideo.h"
#include "engine/platformLinux/linuxOGLVideo.h"
#include "engine/platformLinux/linuxConsole.h"
#include "engine/platform/event.h"
#include "engine/console/console.h"
#include "engine/math/mPoint.h"
#include "engine/platform/gameInterface.h"
#include "game/src/t2Version.h"
#include "engine/core/fileio.h"
#include "engine/math/mRandom.h"

// random-ness
static MRandomLCG sgPlatRandom;

// platform-specific state
LinuxPlatformState linuxState;

// windowing state
static bool windowActive = false;
static bool windowLocked = false;
static Point2I windowSize;

// timing
static U32 lastTimeTick = 0;

// uber-cheesy modifier key var
static U32 modifierKeys = 0;

#ifdef DEDICATED
static bool shouldQuit = false;
#endif

void Platform::AlertOK( const char* title, const char* message )
{
	Con::warnf( ConsoleLogEntry::General, message );
}

bool Platform::AlertOKCancel( const char* title, const char* message )
{
#ifdef DEDICATED
	char response[32];

	fprintf( stderr, "tribes2: %s\n", message );

	while( 1 ) {
		fputs( "OK? > ", stderr );
		fgets( response, 32, stdin );

		if( strncasecmp( response, "yes", 3 ) == 0 ) {
			return true;
		} else if( strncasecmp( response, "no", 2 ) == 0 ) {
			return false;
		} else {
			fputs( "Please answer \"yes\" or \"no\"\n", stderr );
		}

	}
#else
	Con::warnf( ConsoleLogEntry::General, message );
	return false;
#endif
}

bool Platform::AlertRetry( const char* title, const char* message )
{
#ifdef DEDICATED
	char response[32];

	fprintf( stderr, "tribes2: %s\n", message );

	while( 1 ) {
		fputs( "Retry? > ", stderr );
		fgets( response, 32, stdin );

		if( strncasecmp( response, "yes", 3 ) == 0 ) {
			return true;
		} else if( strncasecmp( response, "no", 2 ) == 0 ) {
			return false;
		} else {
			fputs( "Please answer \"yes\" or \"no\"\n", stderr );
		}

	}
#else
	Con::warnf( ConsoleLogEntry::General, message );
	return false;
#endif
}

static void setMouseClipping( void )
{
#ifndef DEDICATED

	if( windowActive ) {
		SDL_ShowCursor( 0 );

		if( windowLocked ) {
			SDL_WM_GrabInput( SDL_GRAB_ON );
		} else {
			SDL_WM_GrabInput( SDL_GRAB_OFF );
		}

	} else {
		SDL_ShowCursor( 1 );
	}

#endif
}

static void InitInput( void )
{
	windowActive = true;
	setMouseClipping( );
}

//--------------------------------------
void Platform::enableKeyboardTranslation(void)
{
#ifndef DEDICATED
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,
	                    SDL_DEFAULT_REPEAT_INTERVAL);
	SDL_EnableUNICODE( 1 );
#endif
}


//--------------------------------------
void Platform::disableKeyboardTranslation(void)
{
#ifndef DEDICATED
	SDL_EnableKeyRepeat(0, 0);
	SDL_EnableUNICODE( 0 );
#endif
}

void Platform::setWindowLocked( bool locked )
{
	windowLocked = locked;
	setMouseClipping( );
}

void Platform::minimizeWindow()
{
#ifndef DEDICATED
	SDL_WM_IconifyWindow( );
#endif
}

#ifdef DEDICATED
void linuxPostQuitMessage( void )
{
	shouldQuit = true;
}
#endif

static U32 translateMods( SDLMod mods )
{

	if( mods == KMOD_NONE ) {
		return 0;
	}

	S32 out = 0;

	if( mods & KMOD_LSHIFT ) {
		out |= SI_LSHIFT;
	}

	if( mods & KMOD_RSHIFT ) {
		out |= SI_RSHIFT;
	}

	if( mods & KMOD_LCTRL ) {
		out |= SI_LCTRL;
	}

	if( mods & KMOD_RCTRL ) {
		out |= SI_RCTRL;
	}

	if( mods & KMOD_LALT ) {
		out |= SI_LALT;
	}

	if( mods & KMOD_RALT ) {
		out |= SI_RALT;
	}

	return out;
}

static void processKeyMessage( SDL_keysym* keysym, bool down )
{
	InputEvent event;

	S32 key = translateSDLToKeyCode( keysym->sym );
#if (defined DEBUG) && !(defined DEDICATED)

	if( key == KEY_NULL ) {
		fprintf( stderr, "Keyboard weirdness: %d / %d / %d / %d / %s\n",
			 keysym->scancode, keysym->sym, keysym->mod, keysym->unicode,
			 SDL_GetKeyName( keysym->sym ) );
	}

#endif
	U32 mods = translateMods( keysym->mod );
	modifierKeys = mods;

	event.deviceInst = 0;
	event.deviceType = KeyboardDeviceType;
	event.objType = SI_KEY;
	event.objInst = key;
	event.action = down ? SI_MAKE : SI_BREAK;
	event.modifier = mods;
	event.ascii = keysym->unicode;
	event.fValue = down ? 1.0 : 0.0;

	Game->postEvent( event );
}

static void processDeltaMessage( int x, int y )
{
	InputEvent event;
	float xp = static_cast<float>( x );
	float yp = static_cast<float>( y );

	event.deviceType = MouseDeviceType;
	// always 0 for mouse, only one mouse
	event.deviceInst = 0;
	// always 0 for mouse, only one axis of given type
	event.objInst = 0;
	event.modifier = modifierKeys;
	event.action = SI_MOVE;

	event.objType = SI_XAXIS;
	event.fValue = xp;
	Game->postEvent( event );

	event.objType = SI_YAXIS;
	event.fValue = yp;
	Game->postEvent( event );
}

KeyCodes translateMouseButton( int button )
{
	KeyCodes key;

	switch( button ) {
	case SDL_BUTTON_LEFT:
		key = KEY_BUTTON0;
		break;
	case SDL_BUTTON_MIDDLE:
		key = KEY_BUTTON2;
		break;
	case SDL_BUTTON_RIGHT:
		key = KEY_BUTTON1;
		break;
	case 4:
	case 5:
		/* This should never happen
		   - Buttons 4 and 5 are interpreted as mouse wheel
		 */
	default:
		key = KEY_BUTTON0 + button - 1;
	}
	return key;
}

static void processMouseMessage( int button, int state )
{
	InputEvent event;

	event.deviceInst = 0;
	event.deviceType = MouseDeviceType;
	event.objType = SI_BUTTON;
	event.objInst = translateMouseButton( button );
	event.action = state ? SI_MAKE : SI_BREAK;
	event.modifier = modifierKeys;
	event.ascii = 0;
	event.fValue = ( event.action == SI_MAKE ) ? 1.0 : 0.0;

	Game->postEvent( event );
}

static void processWheelMessage( int button, int state )
{

	if( !state ) {
		// we don't want button "down"
		// events for the wheel
		return;
	}

	InputEvent event;
	F32 delta = 0.0f;

	if( button == 4 ) {
		delta = 5.0f;
	} else { 
		delta = -5.0f;
	}

	event.deviceInst = 0;
	event.deviceType = MouseDeviceType;
	event.objType = SI_ZAXIS;
	event.objInst = 0;
	event.action = SI_MOVE;
	event.modifier = modifierKeys;
	event.ascii = 0;
	event.fValue = delta;

	Game->postEvent( event );
}

static JoystickCodes translatePOV( U8 hat )
{
	JoystickCodes array[] = { SI_XPOV, SI_YPOV, SI_UPOV,
				  SI_DPOV, SI_LPOV, SI_RPOV };
	hat = ( hat < 6 ) ? hat : 5;
	return array[hat];
}

static bool findHatDelta( U8 oldHat, U8 newHat, JoystickCodes* hat, U8* action )
{
	bool wasUp = ( oldHat & SDL_HAT_UP );
	bool isUp = ( newHat & SDL_HAT_UP );
	bool wasDown = ( oldHat & SDL_HAT_DOWN );
	bool isDown = ( newHat & SDL_HAT_DOWN );
	bool wasLeft = ( oldHat & SDL_HAT_LEFT );
	bool isLeft = ( newHat & SDL_HAT_LEFT );
	bool wasRight = ( oldHat & SDL_HAT_RIGHT );
	bool isRight = ( newHat & SDL_HAT_RIGHT );

	if( hat == 0 || action == 0 ) {
		return false;
	}

	*action = SI_BREAK;

	if( wasUp != isUp ) {
		*hat = SI_UPOV;

		if( isUp ) {
			*action = SI_MAKE;
		}

		return true;
	}

	if( wasDown != isDown ) {
		*hat = SI_DPOV;

		if( isDown ) {
			*action = SI_MAKE;
		}

		return true;
	}

	if( wasLeft != isLeft ) {
		*hat = SI_LPOV;

		if( isLeft ) {
			*action = SI_MAKE;
		}

		return true;
	}

	if( wasRight != isRight ) {
		*hat = SI_RPOV;

		if( isRight ) {
			*action = SI_MAKE;
		}

		return true;
	}

	return false;
}

static void processJoyHatMessage( U8 hat, U8 newValue )
{
	InputEvent event;
	U8 oldValue = getHatState( hat );
	JoystickCodes pov;
	U8 action;

	// this is a little more complicated than I'd like
	// because SDL doesn't report hat events very well.
	// it just says "hey, the hat changed", and you have
	// to manage the on/off-ness of it.
	// also, I'm assuming I'll only see one delta per
	// packet, which Sam says is an okay assumption
	// for now.

	event.deviceInst = 0;
	event.deviceType = JoystickDeviceType;
	// which hat
	event.objInst = hat;

	if( findHatDelta( oldValue, newValue, &pov, &action ) == false ) {
		return;
	}

	setHatState( hat, newValue );
	// set to SI_RPOV, SI_LPOV, etc.
	event.objType = pov;
	// set to SI_MAKE, SI_BREAK
	event.action = action;

	event.modifier = modifierKeys;
	event.ascii = 0;
	event.fValue = ( event.action == SI_MAKE ) ? 1.0 : 0.0;

	Game->postEvent( event );
}

static KeyCodes translateJoyButton( int button )
{

	if( button < 32 ) {
		return static_cast<KeyCodes>( KEY_BUTTON0 + button );
	} else {
		return KEY_NULL;
	}

}

static void processJoyButtonMessage( int button, int state )
{
	InputEvent event;

	event.deviceInst = 0;
	event.deviceType = JoystickDeviceType;
	event.objType = SI_BUTTON;
	event.objInst = translateJoyButton( button );
	event.action = state ? SI_MAKE : SI_BREAK;
	event.modifier = modifierKeys;
	event.ascii = 0;
	event.fValue = ( event.action == SI_MAKE ) ? 1.0 : 0.0;

	Game->postEvent( event );
}

static bool processMessages( void )
{
#ifdef DEDICATED
	return ( shouldQuit == false );
#else
	SDL_Event event;
	bool cont = true;

	while( SDL_PollEvent( &event ) ) {

		switch( event.type ) {
		case SDL_ACTIVEEVENT:
			if ( event.active.state & SDL_APPACTIVE ) {
				windowActive = event.active.gain;

				if( windowActive ) {
					Video::reactivate( );
					Con::evaluate( "resetCanvas();" );
					Input::activate( );
				} else {
					Input::deactivate( );
					Video::deactivate( );
				}
				setMouseClipping( );
			}
			break;
		case SDL_VIDEOEXPOSE:
			Con::evaluate( "resetCanvas();" );
			break;
		case SDL_KEYDOWN:
			processKeyMessage( &event.key.keysym, true );
			break;
		case SDL_KEYUP:
			processKeyMessage( &event.key.keysym, false );
			break;
		case SDL_MOUSEMOTION:

			if( windowLocked ) {
				processDeltaMessage( event.motion.xrel, event.motion.yrel );
			} else {
				MouseMoveEvent move;

				move.xPos = event.motion.x;
				move.yPos = event.motion.y;
				move.modifier = modifierKeys;
				Game->postEvent( move );
			}

			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:

			if( event.button.button == 4 ||
			    event.button.button == 5 ) {
				processWheelMessage( event.button.button, event.button.state );
			} else {
				processMouseMessage( event.button.button, event.button.state  );
			}

			break;
		case SDL_JOYAXISMOTION:
			// we ignore this because it needs to be reported
			// continusously, but this is only discrete
			// see Input::process()
			break;
		case SDL_JOYHATMOTION:
			processJoyHatMessage( event.jhat.hat, event.jhat.value );
			break;
		case SDL_JOYBUTTONDOWN:
			processJoyButtonMessage( event.jbutton.button, true );
			break;
		case SDL_JOYBUTTONUP:
			processJoyButtonMessage( event.jbutton.button, false );
			break;
		case SDL_JOYBALLMOTION:
			processDeltaMessage( event.jball.xrel, event.jball.yrel );
			break;
		case SDL_QUIT:
			cont = false;
			break;
		}

	}

	return cont;
#endif
}

void Platform::process( void )
{
	linuxConsole->process( );

	if( !processMessages( ) ) {
		Event quitEvent;
		quitEvent.type = QuitEventType;
		Game->postEvent( quitEvent );
	}

#ifdef DEDICATED
	usleep( 1000 );
#else
	Input::process( );
#endif
}

// FIXME: I'm not sure exactly what this is supposed to do.
// Does the CD have to be in the drive at all times?  Just for the server?
// The Win32 code looks like it's looking for a stub executable of some kind.
// eh? :)  For now, return true...
bool Platform::doCDCheck()
{
	return true;
}

void GetDesktopState( void )
{
#ifndef DEDICATED

	if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		Con::warnf( ConsoleLogEntry::General, "SDL video init failed..." );
		return;
	}

	const SDL_VideoInfo* info = SDL_GetVideoInfo( );

	if( info == 0 ) {
		return;
	}

	SDL_PixelFormat* format = info->vfmt;
	int bpp = format->BitsPerPixel;

	int width = 0;
	int height = 0;

	if( sdl_GetScreenSize( &width, &height ) == 0 ) {
		return;
	}

	linuxState.bpp = bpp;
	linuxState.width = width;
	linuxState.height = height;
#endif
}

const Point2I& Platform::getWindowSize( void )
{
	return windowSize;
}

void Platform::setWindowSize( U32 w, U32 h )
{
	windowSize.set( w, h );
}

static void InitWindow( const Point2I& initialSize )
{
	windowSize = initialSize;
}

static void InitOpenGL( void )
{
#ifdef DEDICATED
	Con::printf( "Ignoring OpenGL initialization." );
#else
	DisplayDevice::init( );

	int width = 640;
	int height = 480;
	int bpp = 16;
	bool fullScreen = true;

	const char* resolution = Con::getVariable( "$pref::Video::resolution" );

	if( resolution[0] != '\0' ) {
		sscanf( resolution, "%d %d %d", &width, &height, &bpp );
	}

	// default to fullscreen
	fullScreen = Con::getBoolVariable( "$pref::Video::fullScreen", true );

	// we don't have a separate voodoo 2 driver
	if( !Video::setDevice( "OpenGL", width, height, bpp, fullScreen ) ) {
		Con::errorf( ConsoleLogEntry::General,
			     "Couldn't activate OpenGL, exiting..." );
		Platform::forceShutdown( 1 );
		return;
	}

	// hack from Win32 stuff
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
#endif
}

static char cBuffer[512];

static const char* cGetDesktopResolution( SimObject*, S32, const char** )
{
	dSprintf( cBuffer, sizeof( cBuffer ), "%d %d %d", linuxState.width, linuxState.height, linuxState.bpp );
	char* returnString = Con::getReturnBuffer( dStrlen( cBuffer ) + 1 );
	dStrcpy( returnString, cBuffer );
	return returnString;
}

void Platform::init( void )
{
	// Set the platform variable for the scripts
	Con::setVariable( "$platform", "linux" );

	LinuxConsole::create( );
#ifndef DEDICATED
	GetDesktopState( );
	installRedBookDevices( );

	Con::printf( "Video Init:" );
	Video::init( );

	if( Video::installDevice( OpenGLDevice::create( ) ) ) {
		Con::printf( "    OpenGL display device created." );
	} else {
		Con::printf( "    OpenGL display device *not* created." );
	}

	Input::init( );
	InitInput( );

	Con::addCommand( "getDesktopResolution", cGetDesktopResolution, "getDesktopResolution();", 1, 1 );
#endif
}

void Platform::shutdown( void )
{
	LinuxConsole::destroy( );
#ifndef DEDICATED
	setWindowLocked( false );
	Input::destroy( );
	Video::destroy( );
	SDL_Quit();
#endif
}

S32 run( S32 argc, const char** argv )
{
#ifdef DEDICATED
	// start the timers
	linuxInitTicks( );
	lastTimeTick = Platform::getRealMilliseconds( );

	return Game->main( argc, argv );
#else

	// start the timers
	linuxInitTicks( );
	lastTimeTick = Platform::getRealMilliseconds( );
	createFontInit( );
	windowSize.set( 0, 0 );
	S32 ret = Game->main( argc, argv );
	createFontShutdown( );

	return ret;
#endif
}

void Platform::initWindow( const Point2I& initialSize, const char* name )
{
	InitWindow( initialSize );
	InitOpenGL( );
#ifndef DEDICATED
	SDL_WM_SetCaption( name, "tribes2" );
	SDL_WM_SetIcon(SDL_LoadBMP("icon.bmp"), NULL);
#endif
}

static void setupFPU( void )
{
	// The correct combination to catch NaNs is: IM, ZM, OM
	bool im = getenv( "TRIBES2_FPU_IM" );
	bool dm = getenv( "TRIBES2_FPU_DM" );
	bool zm = getenv( "TRIBES2_FPU_ZM" );
	bool om = getenv( "TRIBES2_FPU_OM" );
	bool um = getenv( "TRIBES2_FPU_UM" );

	fpu_control_t cw;

	_FPU_GETCW( cw );

	// triggers problem in NVIDIA drivers
	if( im ) {
		cw &= ~_FPU_MASK_IM;
	} else {
		cw |= _FPU_MASK_IM;
	}

	// triggers problems everywhere :)
	if( dm ) {
		cw &= ~_FPU_MASK_DM;
	} else {
		cw |= _FPU_MASK_DM;
	}

	// div0
	if( zm ) {
		cw &= ~_FPU_MASK_ZM;
	} else {
		cw |= _FPU_MASK_ZM;
	}

	// *numeric* overflow
	if( om ) {
		cw &= ~_FPU_MASK_OM;
	} else {
		cw |= _FPU_MASK_OM;
	}

	// *numeric* underflow
	if( um ) {
		cw &= ~_FPU_MASK_UM;
	} else {
		cw |= _FPU_MASK_UM;
	}

	_FPU_SETCW( cw );

	if ( getenv("TRIBES2_FPU_WINDOWS") ) {
		cw = 0x0f7f;
		_FPU_SETCW( cw );
	}
}

int main( int argc, char* argv[] )
{
	int dedicatedArgc = 0;
	int missionArgc = 0;
	char version[32];

	sprintf( version, "#%d", getTribes2VersionNumber( ) );
	loki_setgamename( "tribes2", version, "Tribes 2 for Linux" );
	loki_isdemo( 0 );
#ifndef DEDICATED
	loki_signalcleanup(SDL_Quit);
#endif

	loki_registeroption( "help", 'h', "Display this help message" );
	loki_registeroption( "version", 'v', "Display the game version" );
#ifndef DEDICATED
	loki_registeroption( "fullscreen", 'f', "Run the game fullscreen" );
	loki_registeroption( "windowed", 'w', "Run the game in a window" );
	loki_registeroption( "nosound", 's', "Do not access the soundcard" );
#endif
	loki_registeroption( "update", 'u', "Run the Loki auto-update tool" );
	loki_registeroption( "qagent", 'q', "Run the Loki QAgent support tool" );
#ifndef DEDICATED
	loki_registeroption( "gllibrary", 'g', "Select 3D rendering library" );
#endif

	loki_initialize_noparse( argc, argv );

	// HACK: path creation at startup
	Platform::createPath( "base/prefs/" );

	for( int i = 1; i < argc; i++ ) {
#define IS_ARG(x,l,s) ( !strcasecmp( x, l ) || !strcasecmp( x, s ) )

		if( IS_ARG( argv[i], "--help", "-h" ) ) {
			loki_printusage( argv[0], 0 );
			exit( 0 );
		} else if( IS_ARG( argv[i], "--version", "-v" ) ) {
			printf( "%s\nBuilt with glibc-%d.%d on %s\n",
				loki_getgamedescription( ),
				__GLIBC__, __GLIBC_MINOR__,
				loki_getarch( ) );
			exit( 0 );
		} else if( IS_ARG( argv[i], "--update", "-u" ) ) {
			loki_runupdate( argc, argv );
		} else if( IS_ARG( argv[i], "--qagent", "-q" ) ) {
			loki_runqagent( 0 );
		} else {

			// do our last check, and pass to engine
			if( strcasecmp( argv[i], "-dedicated" ) == 0 ) {
				dedicatedArgc = i;
			} else if( strcasecmp( argv[i], "-mission" ) == 0 ) {
				missionArgc = i;
			}

		}

#undef IS_ARG
	}

	chdir( loki_getdatapath( ) );

#if defined(DEDICATED) && !defined(BUILD_TOOLS)
	// make sure we have -dedicated
	if( dedicatedArgc ) {

		// if you specify mission, you must specify args,
		// but you don't need to specify it
		if( missionArgc ) {

			if( missionArgc + 2 >= argc ) {
				// uh oh, need two args
				loki_printusage( argv[0],
						 "\nFATAL: \"-mission\" requires "
						 "<map> and <type>\"" );
				exit( 1 );
			}

		}

	} else {
		loki_printusage( argv[0],
				 "\nFATAL: The dedicated server "
				 "*must* be run with \"-dedicated\"." );
		exit( 1 );
	}

	printf( "Dedicated server by:\n\tLoki Software, Inc.\n\thttp://www.lokigames.com\n" );
#else
	// make sure we don't have -dedicated
	if( dedicatedArgc ) {
		loki_printusage( argv[0],
				 "\nFATAL: The client "
				 "*may not* be run with \"-dedicated\"." );
		exit( 1 );
	}
#endif

	setupFPU( );

	int retval = run( argc, const_cast<const char**>( argv ) );

	return retval;
}

void TimeManager::process( void )
{
	U32 currentTime = Platform::getRealMilliseconds( );
	TimeEvent event;

	event.elapsedTime = currentTime - lastTimeTick;
	lastTimeTick = currentTime;

	Game->postEvent( event );
}

F32 Platform::getRandom( void )
{
	return sgPlatRandom.randF( );
}

bool Platform::openWebBrowser( const char* webAddress )
{
	if ( Video::isFullScreen() ) {
		minimizeWindow( );
	}
	loki_launchURL( webAddress );
}

const char* Platform::getLoginPassword()
{
	File file;

	file.open( "password", File::Read );

	if( file.getStatus( ) != File::Ok ) {
		return "";
	}

	U32 size = file.getSize( );

	if( !size ) {
		return "";
	}

	static char buffer[256];

	file.read( size, buffer );

	if( file.getStatus( ) != File::Ok ) {
		file.close( );
		return "";
	}

	file.close( );

	return buffer;
}

bool Platform::setLoginPassword( const char* password )
{
	File file;

	file.open( "password", File::Write );

	if( file.getStatus( ) != File::Ok ) {
		return false;
	}

	file.write( dStrlen( password ), password );

	if( file.getStatus( ) != File::Ok ) {
		return false;
	}

	file.close( );

	return true;
}

bool Platform::excludeOtherInstances( const char* mutexName )
{
	// foo on that
	return true;
}

//--------------------------------------
// Dedicated server launcher:
//--------------------------------------
ConsoleFunction( launchDedicatedServer, bool, 4, 4, "launchDedicatedServer( map, missionType, botCount )" )
{
   // run "tribes2 -dedicated -mission argv[1] argv[2] -bot dAtoi( argv[3] )"
   // Tell what we are doing
   char cmdLine[512];

   dSprintf( cmdLine, sizeof( cmdLine ), "tribes2d -dedicated -mission %s %s -bot %d", argv[1], argv[2], dAtoi( argv[3] ) ); 
   Con::errorf( "** launching dedicated server - command line = \"%s\" **", cmdLine );

   // Fork and run the server
   pid_t child = fork();
   if ( child == 0 ) {
      // The child runs the dedicated server
      int i;
      char *args[12];

      i = 0;
      // Always pop up an xterm so we can have console input
      args[i++] = "xterm";
      args[i++] = "-e";
      // The rest of the dedicated server command line goes here
      args[i++] = "./tribes2d";
      args[i++] = "-dedicated";
      args[i++] = "-mission";
      args[i++] = argv[1];
      args[i++] = argv[2];
      args[i++] = "-bot";
      args[i++] = argv[3];
      args[i++] = NULL;
      // Execute it!
      execvp(args[0], args);

      // Fall through only if exec() fails...
      perror(args[0]); _exit(255);
   }
   return( child != -1 );
}
