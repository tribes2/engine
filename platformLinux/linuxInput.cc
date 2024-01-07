//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include <SDL/SDL.h>
#include <SDL/SDL_joystick.h>
#include "sdl_utils.h"

#include "engine/platformLinux/platformLinux.h"
#include "engine/platform/platformInput.h"
#include "engine/platform/event.h"
#include "engine/console/console.h"
#include "engine/platform/gameInterface.h"

#define USE_X11_TRANSLATE

#ifdef USE_X11_TRANSLATE
static void fillAsciiTable( void );
#endif

InputManager* Input::smManager = 0;
bool Input::smActive = false;

static SDL_Joystick* joystick = 0;
static int joyNumButtons = 0;
static int joyNumAxes = 0;
static int joyNumBalls = 0;
static int joyNumHats = 0;
static U8* joyHatStates = 0;

static bool cIsJoystickDetected( SimObject* obj, S32 argc, const char** argv )
{
	return ( joystick != 0 );
}

static const char* cGetJoystickAxes( SimObject* obj, S32 argc, const char** argv )
{
	const char* axisNames[] = { "\tX", "\tY", "\tZ", "\tR", "\tU", "\tV" };

	if( joystick == 0 ) {
		return "";
	}

	char buffer[64];

	dSprintf( buffer, sizeof( buffer ), "%d", joyNumAxes );

	for( U32 i = 0; i < joyNumAxes; i++ ) {
		dStrcat( buffer, axisNames[i] );
	}

	char* returnString = Con::getReturnBuffer( dStrlen( buffer ) + 1 );
	dStrcpy( returnString, buffer );

	return returnString;
}

// Small helpers.
U8 getHatState( U8 hat )
{

	if( joyHatStates == 0 ) {
		return 0;
	}

	if( hat >= joyNumHats ) {
		return 0;
	}

	return joyHatStates[hat];
}

void setHatState( U8 hat, U8 state )
{

	if( joyHatStates == 0 ) {
		return;
	}

	if( hat >= joyNumHats ) {
		return;
	}

	joyHatStates[hat] = state;
}

void Input::init( void )
{
	Con::printf( "Input init: " );
	smActive = false;

	Con::addCommand( "isJoystickDetected", cIsJoystickDetected, "isJoystickDetected();", 1, 1 );
	Con::addCommand( "getJoystickAxes", cGetJoystickAxes, "getJoystickAxes( device );", 2, 2 );
#ifndef DEDICATED
#ifdef USE_X11_TRANSLATE
	fillAsciiTable( );
#endif
	sdl_InitClipboard();

	Con::printf( "    Clipboard initialized" );
	Con::printf( "    Keyboard initialized" );
	Con::printf( "    Mouse initialized" );

	if( SDL_Init( SDL_INIT_JOYSTICK ) < 0 ) {
		Con::printf( "    Joystick not initialized" );
		return;
	}

	Con::printf( "    Joystick initialized" );
	int numJoysticks = SDL_NumJoysticks( );

	if( numJoysticks <= 0 ) {
		Con::printf( "    No joysticks detected" );
		return;
	}

	int i;

	for( i = 0; i < numJoysticks; i++ ) {
		SDL_Joystick* temp = SDL_JoystickOpen( i );

		if( temp ) {
			joystick = temp;
			break;
		}

	}

	if( !joystick ) {
		Con::printf( "    No openable joysticks found" );
	}

	joyNumButtons = SDL_JoystickNumButtons( joystick );
	joyNumAxes = SDL_JoystickNumAxes( joystick );
	joyNumBalls = SDL_JoystickNumBalls( joystick );
	joyNumHats = SDL_JoystickNumHats( joystick );

	if( joyNumHats ) {
		joyHatStates = new U8[joyNumHats];
		dMemset( joyHatStates, 0, joyNumHats );
	}

	Con::printf( "    Found joystick: %s", SDL_JoystickName( i ) );
	Con::printf( "    Buttons: %d", joyNumButtons );
	Con::printf( "    Axes: %d", joyNumAxes );
	Con::printf( "    Balls: %d", joyNumBalls );
	Con::printf( "    Hats: %d", joyNumHats );
#endif
}

void Input::destroy( void )
{
	Con::printf( "Input shutdown" );
	smActive = false;
#ifndef DEDICATED

	SDL_JoystickClose( joystick );
	joystick = 0;
	joyNumButtons = 0;
	joyNumAxes = 0;
	joyNumBalls = 0;
	joyNumHats = 0;

	if( joyHatStates ) {
		delete[] joyHatStates;
	}
#endif
}

#ifdef USE_X11_TRANSLATE
typedef struct {
	U16 lower;
	U16 upper;
	U16 goofy;
} asciiTable_t;

#define NUM_KEYS (KEY_OEM_102+1)
#define KEY_FIRST KEY_ESCAPE

static asciiTable_t asciiTable[NUM_KEYS];

// special prototype for non-visible SDL function
extern "C" Uint16 X11_KeyToUnicode( SDLKey keysym, SDLMod modifiers );

static void fillAsciiTable( void )
{
#ifndef DEDICATED
	U32 keyCode = 0;

	dMemset( &asciiTable, 0, sizeof( asciiTable ) );

	for( keyCode = KEY_FIRST; keyCode < NUM_KEYS; keyCode++ ) {
		Uint16 key = 0;
		SDLKey sym = translateKeyCodeToSDL( keyCode );
		SDLMod mod = 0;

		// lower case
		key = X11_KeyToUnicode( sym, mod );
		asciiTable[keyCode].lower = key;
		// upper case
		mod = KMOD_SHIFT;
		key = X11_KeyToUnicode( sym, mod );
		asciiTable[keyCode].upper = key;
		// goofy (i18n) case
		mod = KMOD_MODE;
		key = X11_KeyToUnicode( sym, mod );
		asciiTable[keyCode].goofy = key;
	}
#endif
}

U16 Input::getKeyCode( U16 asciiCode )
{

	for( S32 i = KEY_FIRST; i < NUM_KEYS; i++ ) {

		if( asciiTable[i].lower == asciiCode ) {
			return i;
		} else if( asciiTable[i].upper == asciiCode ) {
			return i;
		} else if( asciiTable[i].goofy == asciiCode ) {
			return i;
		}

	}

	return 0;
}

U16 Input::getAscii( U16 keyCode, KEY_STATE keyState )
{

	if( keyCode >= NUM_KEYS ) {
		return 0;
	}

	switch( keyState ) {
	case STATE_LOWER:
		return asciiTable[keyCode].lower;
	case STATE_UPPER:
		return asciiTable[keyCode].upper;
	case STATE_GOOFY:
		return asciiTable[keyCode].goofy;
	default:
		return 0;
	}

}
#else
// FIXME: 'goofy' is always '0' because we suck
typedef struct {
	U16 code;
	U16 lower;
	U16 upper;
	U16 goofy;
} asciiTable_t;

static asciiTable_t asciiTable[128] =
{
	{ KEY_NULL, 0, 0, 0 },
	{ KEY_NULL, 0, 0, 0 },
	{ KEY_NULL, 0, 0, 0 },
	{ KEY_NULL, 0, 0, 0 },
	{ KEY_NULL, 0, 0, 0 },
	{ KEY_NULL, 0, 0, 0 },
	{ KEY_NULL, 0, 0, 0 },
	{ KEY_NULL, 0, 0, 0 },
	{ KEY_BACKSPACE, 8, 8, 0 },
	{ KEY_TAB, 9, 9, 0 },
	{ KEY_NULL, 0, 0, 0 },
	{ KEY_NULL, 0, 0, 0 },
	{ KEY_NULL, 0, 0, 0 },
	{ KEY_RETURN, 13, 13, 0 },
	{ KEY_NULL, 0, 0, 0 },
	{ KEY_NULL, 0, 0, 0 },
	{ KEY_NULL, 0, 0, 0 },
	{ KEY_NULL, 0, 0, 0 },
	{ KEY_NULL, 0, 0, 0 },
	{ KEY_NULL, 0, 0, 0 },
	{ KEY_NULL, 0, 0, 0 },
	{ KEY_NULL, 0, 0, 0 },
	{ KEY_NULL, 0, 0, 0 },
	{ KEY_NULL, 0, 0, 0 },
	{ KEY_NULL, 0, 0, 0 },
	{ KEY_NULL, 0, 0, 0 },
	{ KEY_NULL, 0, 0, 0 },
	{ KEY_ESCAPE, 27, 27, 0 },
	{ KEY_NULL, 0, 0, 0 },
	{ KEY_NULL, 0, 0, 0 },
	{ KEY_NULL, 0, 0, 0 },
	{ KEY_NULL, 0, 0, 0 },
	{ KEY_SPACE, 32, 32, 0 },
	{ KEY_1, 49, 33, 0 },
	{ KEY_APOSTROPHE, 39, 34, 0 },
	{ KEY_3, 51, 35, 0 },
	{ KEY_4, 52, 36, 0 },
	{ KEY_5, 53, 37, 0 },
	{ KEY_7, 55, 38, 0 },
	{ KEY_APOSTROPHE, 39, 34, 0 },
	{ KEY_9, 58, 40, 0 },
	{ KEY_0, 48, 41, 0 },
	{ KEY_8, 56, 42, 0 },
	{ KEY_EQUALS, 61, 43, 0 },           // 43
	{ KEY_COMMA, 44, 60, 0 },
	{ KEY_MINUS, 45, 95, 0 },
	{ KEY_PERIOD, 46, 62, 0 },
	{ KEY_BACKSLASH, 47, 63, 0 },
	{ KEY_0, 48, 41, 0 },                // 48
	{ KEY_1, 49, 33, 0 },
	{ KEY_2, 50, 64, 0 },
	{ KEY_3, 51, 35, 0 },
	{ KEY_4, 52, 36, 0 },
	{ KEY_5, 53, 37, 0 },
	{ KEY_6, 54, 94, 0 },
	{ KEY_7, 54, 38, 0 },
	{ KEY_8, 56, 42, 0 },
	{ KEY_9, 57, 28, 0 },
	{ KEY_SEMICOLON, 59, 58, 0 },        // 58
	{ KEY_SEMICOLON, 59, 58, 0 },
	{ KEY_COMMA, 44, 60, 0 },
	{ KEY_EQUALS, 61, 43, 0 },
	{ KEY_PERIOD, 46, 62, 0 },
	{ KEY_BACKSLASH, 47, 63, 0 },        // 63
	{ KEY_2, 50, 64, 0 },
	{ KEY_A, 97, 65, 0 },                // 65
	{ KEY_B, 98, 66, 0 },
	{ KEY_C, 99, 67, 0 },
	{ KEY_D, 100, 68, 0 },
	{ KEY_E, 101, 69, 0 },
	{ KEY_F, 102, 70, 0 },
	{ KEY_G, 103, 71, 0 },
	{ KEY_H, 104, 72, 0 },
	{ KEY_I, 105, 73, 0 },
	{ KEY_J, 106, 74, 0 },
	{ KEY_K, 107, 75, 0 },
	{ KEY_L, 108, 76, 0 },
	{ KEY_M, 109, 77, 0 },
	{ KEY_N, 110, 78, 0 },
	{ KEY_O, 111, 79, 0 },
	{ KEY_P, 112, 80, 0 },
	{ KEY_Q, 113, 81, 0 },
	{ KEY_R, 114, 82, 0 },
	{ KEY_S, 115, 83, 0 },
	{ KEY_T, 116, 84, 0 },
	{ KEY_U, 117, 85, 0 },
	{ KEY_V, 118, 86, 0 },
	{ KEY_W, 119, 87, 0 },
	{ KEY_X, 120, 88, 0 },
	{ KEY_Y, 121, 89, 0 },
	{ KEY_Z, 122, 90, 0 },
	{ KEY_LBRACKET, 91, 123, 0 },         // 91
	{ KEY_SLASH, 92, 124, 0 },
	{ KEY_RBRACKET, 93, 125, 0 },
	{ KEY_6, 54, 94, 0 },
	{ KEY_MINUS, 45, 95, 0 },
	{ KEY_TILDE, 96, 126, 0 },
	{ KEY_A, 97, 65, 0 },                // 97
	{ KEY_B, 98, 66, 0 },
	{ KEY_C, 99, 67, 0 },
	{ KEY_D, 100, 68, 0 },
	{ KEY_E, 101, 69, 0 },
	{ KEY_F, 102, 70, 0 },
	{ KEY_G, 103, 71, 0 },
	{ KEY_H, 104, 72, 0 },
	{ KEY_I, 105, 73, 0 },
	{ KEY_J, 106, 74, 0 },
	{ KEY_K, 107, 75, 0 },
	{ KEY_L, 108, 76, 0 },
	{ KEY_M, 109, 77, 0 },
	{ KEY_N, 110, 78, 0 },
	{ KEY_O, 111, 79, 0 },
	{ KEY_P, 112, 80, 0 },
	{ KEY_Q, 113, 81, 0 },
	{ KEY_R, 114, 82, 0 },
	{ KEY_S, 115, 83, 0 },
	{ KEY_T, 116, 84, 0 },
	{ KEY_U, 117, 85, 0 },
	{ KEY_V, 118, 86, 0 },
	{ KEY_W, 119, 87, 0 },
	{ KEY_X, 120, 88, 0 },
	{ KEY_Y, 121, 89, 0 },
	{ KEY_Z, 122, 90, 0 },
	{ KEY_LBRACKET, 91, 123, 0 },         // 123
	{ KEY_SLASH, 92, 124, 0 },
	{ KEY_RBRACKET, 93, 125, 0 },
	{ KEY_TILDE, 94, 126, 0 },
	{ KEY_DELETE, 127, 127, 0 },           // 127
};

U16 Input::getKeyCode( U16 asciiCode )
{

	if( asciiCode < sizeof( asciiTable ) ) {
		return asciiTable[asciiCode].code;
	} else {
		return 0;
	}

}

U16 Input::getAscii( U16 keyCode, KEY_STATE whichOne )
{

	for( S32 i = 0; i < sizeof( asciiTable ); i++ ) {

		if( asciiTable[i].code == keyCode ) {

			switch( whichOne ) {
			case STATE_LOWER:
			case STATE_GOOFY:
				return asciiTable[i].lower;
			case STATE_UPPER:
				return asciiTable[i].upper;
			}

		}

	}

	return 0;
}
#endif

bool Input::enable( void )
{
   return true;
}

void Input::disable( void )
{
}

void Input::activate( void )
{
	smActive = true;
}

void Input::deactivate( void )
{
	smActive = false;
}

void Input::reactivate( void )
{
   // Sorry, needed this indirection for Windows...
	Input::activate();
}

bool Input::isEnabled( void )
{
	return true;
}

bool Input::isActive( void )
{
	return smActive;
}

// bool Input::isKeyboardActive( void )
// {
// 	return true;
// }
// 
// bool Input::isMouseActive( void )
// {
// 	return true;
// }
// 
// bool Input::isJoystickActive( void )
// {
// 	return ( joystick != 0 );
// }

static JoystickCodes translateAxis( U8 axis )
{
	JoystickCodes array[] = { SI_XAXIS, SI_YAXIS, SI_ZAXIS,
				  SI_RXAXIS, SI_RYAXIS, SI_RZAXIS };
	axis = ( axis < 6 ) ? axis : 5;
	return array[axis];
}

static void processAxes( void )
{
#ifndef DEDICATED
	InputEvent event;

	// we need continuous reporting for joystick axes
	for( S32 i = 0; i < joyNumAxes; i++ ) {
		S16 value = SDL_JoystickGetAxis( joystick, i );

		event.deviceInst = 0;
		event.deviceType = JoystickDeviceType;
		event.objType = translateAxis( i );
		event.objInst = 0;
		event.action = SI_MOVE;
		event.modifier = 0;
		event.ascii = 0;
		event.fValue = static_cast<float>( value ) / 32767.0f;

		Game->postEvent( event );
	}
#endif
}

void Input::process( void )
{
	// do our immediate processing here
	processAxes( );
}

//-----------------------------------------------------------------------------
// Clipboard functions
const char* Platform::getClipboard()
{
#ifdef DEDICATED
	return "";
#else
	return sdl_GetClipboard();
#endif
}

bool Platform::setClipboard(const char *text)
{
#ifdef DEDICATED
	return false;
#else
	if (!text)
		return false;

	sdl_PutClipboard(text);

	return true;
#endif
}
