//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include <SDL/SDL.h>

#include "Platform/platform.h"
#include "Platform/event.h"

static KeyCodes SDLCodeMap0_127[] =
{
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_BACKSPACE,        // 8
	KEY_TAB,              // 9
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_RETURN,           // 13
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_PAUSE,            // 19
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_ESCAPE,           // 27
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_SPACE,            // 32
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_APOSTROPHE,       // 39
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_COMMA,            // 44 
	KEY_MINUS,            // 45
	KEY_PERIOD,
	KEY_SLASH,
	KEY_0,                // 48
	KEY_1,                // 49
	KEY_2,                // 50
	KEY_3,                // 51
	KEY_4,                // 52
	KEY_5,                // 53
	KEY_6,                // 54
	KEY_7,                // 55
	KEY_8,                // 56
	KEY_9,                // 57
	KEY_NULL,
	KEY_SEMICOLON,        // 59
	KEY_NULL,
	KEY_EQUALS,           // 61
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_LBRACKET,         // 91
	KEY_BACKSLASH,        // 92
	KEY_RBRACKET,         // 93
	KEY_NULL,
	KEY_NULL,
	KEY_TILDE,
	KEY_A,                // 97
	KEY_B,                // 98
	KEY_C,                // 99
	KEY_D,                // 100
	KEY_E,                // 101
	KEY_F,                // 102
	KEY_G,                // 103
	KEY_H,                // 104
	KEY_I,                // 105
	KEY_J,                // 106
	KEY_K,                // 107
	KEY_L,                // 108
	KEY_M,                // 109
	KEY_N,                // 110
	KEY_O,                // 111
	KEY_P,                // 112
	KEY_Q,                // 113
	KEY_R,                // 114
	KEY_S,                // 115
	KEY_T,                // 116
	KEY_U,                // 117
	KEY_V,                // 118
	KEY_W,                // 119
	KEY_X,                // 120
	KEY_Y,                // 121
	KEY_Z,                // 122
	KEY_NULL,             // 123
	KEY_NULL,             // 124
	KEY_NULL,             // 125
	KEY_NULL,             // 126
	KEY_DELETE,           // 127
};

static KeyCodes SDLCodeMap256_319[] =
{
	KEY_NUMPAD0,          // 256
	KEY_NUMPAD1,          // 257
	KEY_NUMPAD2,          // 258
	KEY_NUMPAD3,          // 259
	KEY_NUMPAD4,          // 260
	KEY_NUMPAD5,          // 261
	KEY_NUMPAD6,          // 262
	KEY_NUMPAD7,          // 263
	KEY_NUMPAD8,          // 264
	KEY_NUMPAD9,          // 265
	KEY_DECIMAL,          // 266
	KEY_DIVIDE,           // 267
	KEY_MULTIPLY,         // 268
	KEY_SUBTRACT,         // 269
	KEY_ADD,              // 270
	KEY_NUMPADENTER,      // 271
	KEY_EQUALS,           // 272
	KEY_UP,               // 273
	KEY_DOWN,             // 274
	KEY_RIGHT,            // 275
	KEY_LEFT,             // 276
	KEY_INSERT,
	KEY_HOME,
	KEY_END,
	KEY_PAGE_UP,
	KEY_PAGE_DOWN,
	KEY_F1,               // 282
	KEY_F2,               // 283
	KEY_F3,               // 284
	KEY_F4,               // 285
	KEY_F5,               // 286
	KEY_F6,               // 287
	KEY_F7,               // 288
	KEY_F8,               // 289
	KEY_F9,               // 290
	KEY_F10,              // 291
	KEY_F11,              // 292
	KEY_F12,              // 293
	KEY_F13,              // 294
	KEY_F14,              // 295
	KEY_F15,              // 296
	KEY_NULL,
	KEY_NULL,
	KEY_NULL,
	KEY_NUMLOCK,          // 300
	KEY_CAPSLOCK,         // 301
	KEY_SCROLLLOCK,       // 302
	KEY_RSHIFT,           // 303
	KEY_LSHIFT,           // 304
	KEY_RCONTROL,         // 305
	KEY_LCONTROL,         // 306
	KEY_RALT,             // 307
	KEY_LALT,             // 308
	KEY_NULL,
	KEY_NULL,
	KEY_WIN_LWINDOW,      // 311
	KEY_WIN_RWINDOW,      // 312
	KEY_OEM_102,          // 313
	KEY_NULL,
	KEY_HELP,             // 315
	KEY_PRINT,            // 316
	KEY_NULL,
	KEY_NULL,
	KEY_WIN_APPS,         // 319
};

SDLKey translateKeyCodeToSDL( KeyCodes code )
{

	for( S32 i = 0; i <= 127; i++ ) {

		if( SDLCodeMap0_127[i] == code ) {
			return (SDLKey) i;
		}

	}

	for( S32 i = 256; i <= 319; i++ ) {

		if( SDLCodeMap256_319[ i - 256 ] == code ) {
			return (SDLKey) i;
		}

	}

	return SDLK_UNKNOWN;
}

KeyCodes translateSDLToKeyCode( SDLKey sym )
{
	S32 i_sym = static_cast<S32>( sym );

	if( i_sym >= 0 && i_sym <= 127 ) {
		return SDLCodeMap0_127[i_sym];
	}

	if( i_sym >= 256 && i_sym <= 319 ) {
		i_sym -= 256;
		return SDLCodeMap256_319[i_sym];
	}

	return KEY_NULL;
}
