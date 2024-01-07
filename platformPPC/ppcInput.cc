//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "PlatformPPC/platformPPC.h"
#include "Platform/platformInput.h"
#include "ppcInput.h"
#include "Platform/event.h"
#include "console/console.h"


// Static class variables:
InputManager* Input::smManager;


//--------------------------------------------------------------------------
void Input::init()
{
   Con::printf( "Input Init:" );
   smManager = NULL;
}


//--------------------------------------------------------------------------
static U8 VcodeRemap[256] =
{
KEY_A,                     // 0x00 
KEY_A,                     // 0x01 
KEY_D,                     // 0x02 
KEY_F,                     // 0x03 
KEY_H,                     // 0x04 
KEY_G,                     // 0x05 
KEY_Z,                     // 0x06 
KEY_X,                     // 0x07 
KEY_C,                     // 0x08 
KEY_V,                     // 0x09 
KEY_Y,                     // 0x0A 
KEY_B,                     // 0x0B 
KEY_Q,                     // 0x0C 
KEY_W,                     // 0x0D 
KEY_E,                     // 0x0E 
KEY_R,                     // 0x0F 
KEY_Y,                     // 0x10 
KEY_T,                     // 0x11 
KEY_1,                     // 0x12 
KEY_2,                     // 0x13 
KEY_3,                     // 0x14 
KEY_4,                     // 0x15 
KEY_6,                     // 0x16 
KEY_5,                     // 0x17 
0,                         // 0x18 
KEY_9,                     // 0x19 
KEY_7,                     // 0x1A 
0,                         // 0x1B 
KEY_8,                     // 0x1C 
KEY_0,                     // 0x1D 
0,                         // 0x1E 
KEY_O,                     // 0x1F 
KEY_U,                     // 0x20 
0,                         // 0x21 
KEY_I,                     // 0x22 
KEY_P,                     // 0x23 
KEY_RETURN,                // 0x24 
KEY_L,                     // 0x25 
KEY_J,                     // 0x26 
0,                         // 0x27 
KEY_K,                     // 0x28 
0,                         // 0x29 
0,                         // 0x2A 
0,                         // 0x2B 
0,                         // 0x2C 
KEY_N,                     // 0x2D 
KEY_M,                     // 0x2E 
0,                         // 0x2F 

KEY_TAB,                   // 0x30 
0,                         // 0x31 
0,                         // 0x32 
KEY_BACKSPACE,             // 0x33 
0,                         // 0x34 
KEY_ESCAPE,                // 0x35 
0,                         // 0x36 
KEY_ALT,                   // 0x37 
KEY_LSHIFT,                // 0x38 
KEY_CAPSLOCK,              // 0x39 
0,                         // 0x3A 
KEY_CONTROL,               // 0x3B 
KEY_RSHIFT,                // 0x3C 
0,                         // 0x3D 
0,                         // 0x3E 
0,                         // 0x3F 
0,                         // 0x40 
   
KEY_MULTIPLY,              // 0x41 
0,                         // 0x42 
0,                         // 0x43 
0,                         // 0x44 
KEY_ADD,                   // 0x45 
0,                         // 0x46 
KEY_NUMLOCK,               // 0x47 
0,                         // 0x48 
0,                         // 0x49 
0,                         // 0x4A 
0,                         // 0x4B 
KEY_RETURN,                // 0x4C 
KEY_DIVIDE,                // 0x4D 
KEY_SUBTRACT,              // 0x4E 
0,                         // 0x4F 
0,                         // 0x50 
0,                         // 0x51 
KEY_NUMPAD0,               // 0x52 
KEY_NUMPAD1,               // 0x53 
KEY_NUMPAD2,               // 0x54 
KEY_NUMPAD3,               // 0x55 
KEY_NUMPAD4,               // 0x56 
KEY_NUMPAD5,               // 0x57 
KEY_NUMPAD6,               // 0x58 
KEY_NUMPAD7,               // 0x59 
0,                         // 0x5A 
   

KEY_NUMPAD8,               // 0x5B 
KEY_NUMPAD9,               // 0x5C 
0,                         // 0x5D 
0,                         // 0x5E 
0,                         // 0x5F 

KEY_F5,                    // 0x60 
KEY_F6,                    // 0x61 
KEY_F7,                    // 0x62 
KEY_F3,                    // 0x63 
KEY_F8,                    // 0x64 
KEY_F9,                    // 0x65 
0,                         // 0x66 
KEY_F11,                   // 0x67 
0,                         // 0x68 
KEY_PRINT,                 // 0x69 
0,                         // 0x6A 
KEY_SCROLLLOCK,            // 0x6B 
0,                         // 0x6C 
KEY_F10,                   // 0x6D 
0,                         // 0x6E 
KEY_F12,                   // 0x6F 
0,                         // 0x70 
0,                         // 0x71 
KEY_INSERT,                // 0x72 
KEY_HOME,                  // 0x73 
KEY_PAGE_UP,               // 0x74 
KEY_DELETE,                // 0x75 
KEY_F4,                    // 0x76 
KEY_END,                   // 0x77 
KEY_F2,                    // 0x78 
KEY_PAGE_DOWN,             // 0x79 
KEY_F1,                    // 0x7A 
KEY_LEFT,                  // 0x7B 
KEY_RIGHT,                 // 0x7C 
KEY_DOWN,                  // 0x7D 
KEY_UP,                    // 0x7E 
0,                         // 0x7F 
0,                         // 0x80 
0,                         // 0x81 
0,                         // 0x82 
0,                         // 0x83 
0,                         // 0x84 
0,                         // 0x85 
0,                         // 0x86 
0,                         // 0x87 
0,                         // 0x88 
0,                         // 0x89 
0,                         // 0x8A 
0,                         // 0x8B 
0,                         // 0x8C 
0,                         // 0x8D 
0,                         // 0x8E 
0,                         // 0x8F 

0,                         // 0x90 
0,                         // 0x91 
0,                         // 0x92 
0,                         // 0x93 
0,                         // 0x94 
0,                         // 0x95 
0,                         // 0x96 
0,                         // 0x97 
0,                         // 0x98 
0,                         // 0x99 
0,                         // 0x9A 
0,                         // 0x9B 
0,                         // 0x9C 
0,                         // 0x9D 
0,                         // 0x9E 
0,                         // 0x9F 

0,                         // 0xA0 
0,                         // 0xA1 
0,                         // 0xA2 
0,                         // 0xA3 
0,                         // 0xA4 
0,                         // 0xA5 
0,                         // 0xA6 
0,                         // 0xA7 
0,                         // 0xA8 
0,                         // 0xA9 
0,                         // 0xAA 
0,                         // 0xAB 
0,                         // 0xAC 
0,                         // 0xAD 
0,                         // 0xAE 
0,                         // 0xAF 
0,                         // 0xB0 
0,                         // 0xB1 
0,                         // 0xB2 
0,                         // 0xB3 
0,                         // 0xB4 
0,                         // 0xB5 
0,                         // 0xB6 
0,                         // 0xB7 
0,                         // 0xB8 
0,                         // 0xB9 
0,                         // 0xBA 
0,                         // 0xBB 
0,                         // 0xBC 
0,                         // 0xBD 
0,                         // 0xBE 
0,                         // 0xBF 
0,                         // 0xC0 
0,                         // 0xC1 
0,                         // 0xC2 
0,                         // 0xC3 
0,                         // 0xC4 
0,                         // 0xC5 
0,                         // 0xC6 
0,                         // 0xC7 
0,                         // 0xC8 
0,                         // 0xC9 
0,                         // 0xCA 
0,                         // 0xCB 
0,                         // 0xCC 
0,                         // 0xCD 
0,                         // 0xCE 
0,                         // 0xCF 
0,                         // 0xD0 
0,                         // 0xD1 
0,                         // 0xD2 
0,                         // 0xD3 
0,                         // 0xD4 
0,                         // 0xD5 
0,                         // 0xD6 
0,                         // 0xD7 
0,                         // 0xD8 
0,                         // 0xD9 
0,                         // 0xDA 
0,                         // 0xDB 
0,                         // 0xDC 
0,                         // 0xDD 
0,                         // 0xDE 
0,                         // 0xDF 
0,                         // 0xE0 
0,                         // 0xE1 
0,                         // 0xE2 
0,                         // 0xE3 
0,                         // 0xE4 

0,                         // 0xE5 

0,                         // 0xE6 
0,                         // 0xE7 
0,                         // 0xE8 
0,                         // 0xE9 
0,                         // 0xEA 
0,                         // 0xEB 
0,                         // 0xEC 
0,                         // 0xED 
0,                         // 0xEE 
0,                         // 0xEF 
   
0,                         // 0xF0 
0,                         // 0xF1 
0,                         // 0xF2 
0,                         // 0xF3 
0,                         // 0xF4 
0,                         // 0xF5 
   
0,                         // 0xF6 
0,                         // 0xF7 
0,                         // 0xF8 
0,                         // 0xF9 
0,                         // 0xFA 
0,                         // 0xFB 
0,                         // 0xFC 
0,                         // 0xFD 
0,                         // 0xFE 
0                          // 0xFF 
};   


U8 TranslateOSKeyCode(U8 vcode)
{
   return VcodeRemap[vcode];   
}   


