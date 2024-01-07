//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "platformMacCarb/platformMacCarb.h"
#include "platform/platformInput.h"
#include "platform/event.h"
#include "console/console.h"
#include "platform/gameInterface.h"


Boolean noDirectMouse = TRUE; // set to FALSE for ISp enable.  TRUE for debug.

#if __APPLE__ // Mach-O

#else // carbon build.

#define CALL_IN_SPOCKETS_BUT_NOT_IN_CARBON      1
#include <InputSprocket.h>

#define MAX_MOUSE_ELS   16   // more than enough for lots of axes and buttons...

bool gISpAround = TRUE; // until we say otherwise...

OSStatus status = noErr;

ISpDeviceReference mouseRef = NULL;
ISpElementListReference mouseElList;
UInt32 mouseElCount = 0;
ISpElementReference mouseEl[MAX_MOUSE_ELS];
ISpElementInfo mouseElInfo[MAX_MOUSE_ELS];

ISpDeviceReference kbRef = NULL;

#endif


// Static class variables:
InputManager* Input::smManager;
bool           Input::smActive;

bool gInputEnabled = false;
bool gMouseEnabled = false;
bool gKBEnabled = false;
bool gMouseActive = false;
bool gKBActive = false;


//------------------------------------------------------------------------------
// Helper functions.  Should migrate to an InputManager object at some point.
bool enableKeyboard(void);
void disableKeyboard(void);
bool activateKeyboard(void);
void deactivateKeyboard(void);
bool enableMouse(void);
void disableMouse(void);
bool activateMouse(void);
void deactivateMouse(void);



static void fillAsciiTable();

//------------------------------------------------------------------------------
//
// This function gets the standard ASCII code corresponding to our key code
// and the existing modifier key state.
//
//------------------------------------------------------------------------------
struct AsciiData
{
   struct KeyData
   {
      U16   ascii;
      bool  isDeadChar;
   };

   KeyData upper;
   KeyData lower;
   KeyData goofy;
};


#define NUM_KEYS ( KEY_OEM_102 + 1 )
#define KEY_FIRST KEY_ESCAPE
static AsciiData AsciiTable[NUM_KEYS];


//--------------------------------------------------------------------------
void Input::init()
{
   Con::printf( "Input Init:" );
   destroy();

   smManager = NULL;
   smActive = false;

#if __APPLE__
#else
   if (ISpInit)
   {
// if we have no needs, do we bother calling ISpInit?? !!!!!TBD
//      ispErr = ISpInit();
      
      // setup the mouse. we only currently look for the very first one.
      UInt32 count;
      status = ISpDevices_ExtractByClass((UInt32)kISpDeviceClass_Mouse, (UInt32)1, &count, &mouseRef);
      if (status!=noErr) goto initError;
      if (count!=1)
      {
         Con::warnf("Error: InputSprocket detects %d mice.", count);
         goto initError;
      }
      
      // retrieve all the buttons & axes.
      // first, grab the list reference.
      status = ISpDevice_GetElementList(mouseRef, &mouseElList);
      if (status!=noErr) goto initError;

      // then, extract the elements from the list into an array.
      status = ISpElementList_Extract(mouseElList, (UInt32)MAX_MOUSE_ELS, &mouseElCount, mouseEl);
      if (status!=noErr) goto initError;
      
      // then, save away the static info on each element
      for (int i=0; i<mouseElCount; i++)
      {
         status = ISpElement_GetInfo(mouseEl[i], &(mouseElInfo[i]));
         if (status!=noErr)
            mouseElInfo[i].theKind = 0; // clear the kind, so we won't process this guy.
         else
         {
            // print something nice to console.
            if (mouseElInfo[i].theKind==kISpElementKind_Axis
            ||  mouseElInfo[i].theKind==kISpElementKind_Delta)
               Con::printf("   > Found mouse axis: %s.", p2str(mouseElInfo[i].theString));
            else
            if (mouseElInfo[i].theKind==kISpElementKind_Button)
               Con::printf("   > Found mouse button: %s.", p2str(mouseElInfo[i].theString));
            else
               Con::printf("   > Found mouse element: %s.", p2str(mouseElInfo[i].theString));
         }
      }
      
      Con::printf( "   InputSprocket enabled - setup complete." );

      // we exit as we're all set.
      return;
   }

initError:
   Con::printf( "   InputSprocket not enabled." );
   gISpAround = false;

#endif
   
/*
         smManager = new DInputManager;
         if ( !smManager->enable() )
         {
            Con::printf( "   DirectInput not enabled." );
            delete smManager;
            smManager = NULL;
         }
         else
         {
            DInputManager::init();
            Con::printf( "   DirectInput enabled." );
         }
*/

//   fillAsciiTable();
   Con::printf( "" );
}

//------------------------------------------------------------------------------
ConsoleFunction( isJoystickDetected, bool, 1, 1, "isJoystickDetected()" )
{
/*
   argc; argv;
   return( DInputDevice::joystickDetected() );
*/
   return(false);
}

//------------------------------------------------------------------------------
ConsoleFunction( getJoystickAxes, const char*, 2, 2, "getJoystickAxes( instance )" )
{
/*
   argc;
   DInputManager* mgr = dynamic_cast<DInputManager*>( Input::getManager() );
   if ( mgr )
      return( mgr->getJoystickAxesString( dAtoi( argv[1] ) ) );
*/
   return( "" );
}

//------------------------------------------------------------------------------
static void fillAsciiTable()
{
/*
#ifdef LOG_INPUT
   char buf[256];
   Input::log( "--- Filling the ASCII table! ---\n" );
#endif

   //HKL   layout = GetKeyboardLayout( 0 );
   U8    state[256];
   U16   ascii[2];
   U32   dikCode, vKeyCode, keyCode;
   S32   result;

   dMemset( &AsciiTable, 0, sizeof( AsciiTable ) );
   dMemset( &state, 0, sizeof( state ) );

   for ( keyCode = KEY_FIRST; keyCode < NUM_KEYS; keyCode++ )
   {
      ascii[0] = ascii[1] = 0;
      dikCode  = Key_to_DIK( keyCode );
      if ( dikCode )
      {
         //vKeyCode = MapVirtualKeyEx( dikCode, 1, layout );
         vKeyCode = MapVirtualKey( dikCode, 1 );
#ifdef LOG_INPUT
         dSprintf( buf, sizeof( buf ), "KC: %#04X DK: %#04X VK: %#04X\n",
               keyCode, dikCode, vKeyCode );
         Input::log( buf );
#endif

         // Lower case:
         ascii[0] = ascii[1] = 0;
         //result = ToAsciiEx( vKeyCode, dikCode, state, ascii, 0, layout );
         result = ToAscii( vKeyCode, dikCode, state, ascii, 0 );
#ifdef LOG_INPUT
         dSprintf( buf, sizeof( buf ), "  LOWER- R: %d A[0]: %#06X A[1]: %#06X\n",
               result, ascii[0], ascii[1] );
         Input::log( buf );
#endif
         if ( result == 2 )
            AsciiTable[keyCode].lower.ascii = ascii[1] ? ascii[1] : ( ascii[0] >> 8 );
         else if ( result == 1 )
            AsciiTable[keyCode].lower.ascii = ascii[0];
         else if ( result < 0 )
         {
            AsciiTable[keyCode].lower.ascii = ascii[0];
            AsciiTable[keyCode].lower.isDeadChar = true;
            // Need to clear the dead character from the keyboard layout:
            //ToAsciiEx( vKeyCode, dikCode, state, ascii, 0, layout );
            ToAscii( vKeyCode, dikCode, state, ascii, 0 );
         }

         // Upper case:
         ascii[0] = ascii[1] = 0;
         state[VK_SHIFT] = 0x80;
         //result = ToAsciiEx( vKeyCode, dikCode, state, ascii, 0, layout );
         result = ToAscii( vKeyCode, dikCode, state, ascii, 0 );
#ifdef LOG_INPUT
         dSprintf( buf, sizeof( buf ), "  UPPER- R: %d A[0]: %#06X A[1]: %#06X\n",
               result, ascii[0], ascii[1] );
         Input::log( buf );
#endif
         if ( result == 2 )
            AsciiTable[keyCode].upper.ascii = ascii[1] ? ascii[1] : ( ascii[0] >> 8 );
         else if ( result == 1 )
            AsciiTable[keyCode].upper.ascii = ascii[0];
         else if ( result < 0 )
         {
            AsciiTable[keyCode].upper.ascii = ascii[0];
            AsciiTable[keyCode].upper.isDeadChar = true;
            // Need to clear the dead character from the keyboard layout:
            //ToAsciiEx( vKeyCode, dikCode, state, ascii, 0, layout );
            ToAscii( vKeyCode, dikCode, state, ascii, 0 );
         }
         state[VK_SHIFT] = 0;
         
         // Foreign mod case:
         ascii[0] = ascii[1] = 0;
         state[VK_CONTROL] = 0x80;
         state[VK_MENU] = 0x80;
         //result = ToAsciiEx( vKeyCode, dikCode, state, ascii, 0, layout );
         result = ToAscii( vKeyCode, dikCode, state, ascii, 0 );
#ifdef LOG_INPUT
         dSprintf( buf, sizeof( buf ), "  GOOFY- R: %d A[0]: %#06X A[1]: %#06X\n",
               result, ascii[0], ascii[1] );
         Input::log( buf );
#endif
         if ( result == 2 )
            AsciiTable[keyCode].goofy.ascii = ascii[1] ? ascii[1] : ( ascii[0] >> 8 );
         else if ( result == 1 )
            AsciiTable[keyCode].goofy.ascii = ascii[0];
         else if ( result < 0 )
         {
            AsciiTable[keyCode].goofy.ascii = ascii[0];
            AsciiTable[keyCode].goofy.isDeadChar = true;
            // Need to clear the dead character from the keyboard layout:
            //ToAsciiEx( vKeyCode, dikCode, state, ascii, 0, layout );
            ToAscii( vKeyCode, dikCode, state, ascii, 0 );
         }
         state[VK_CONTROL] = 0;
         state[VK_MENU] = 0;
      }
   }

#ifdef LOG_INPUT
   Input::log( "--- Finished filling the ASCII table! ---\n\n" );
#endif
*/
}

//------------------------------------------------------------------------------
U16 Input::getKeyCode( U16 asciiCode )
{
   U16 keyCode = 0;
   U16 i;
   
   // This is done three times so the lowerkey will always
   // be found first. Some foreign keyboards have duplicate
   // chars on some keys.
   for ( i = KEY_FIRST; i < NUM_KEYS && !keyCode; i++ )
   {
      if ( AsciiTable[i].lower.ascii == asciiCode )
      {
         keyCode = i;
         break;
      };
   }

   for ( i = KEY_FIRST; i < NUM_KEYS && !keyCode; i++ )
   {
      if ( AsciiTable[i].upper.ascii == asciiCode )
      {
         keyCode = i;
         break;
      };
   }

   for ( i = KEY_FIRST; i < NUM_KEYS && !keyCode; i++ )
   {
      if ( AsciiTable[i].goofy.ascii == asciiCode )
      {
         keyCode = i;
         break;
      };
   }

   return( keyCode );
}

//------------------------------------------------------------------------------
U16 Input::getAscii( U16 keyCode, KEY_STATE keyState )
{
   if ( keyCode >= NUM_KEYS )
      return 0;

   switch ( keyState )
   {
      case STATE_LOWER:
         return AsciiTable[keyCode].lower.ascii;
      case STATE_UPPER:
         return AsciiTable[keyCode].upper.ascii;
      case STATE_GOOFY:
         return AsciiTable[keyCode].goofy.ascii;
      default:
         return(0);
            
   }
}

//------------------------------------------------------------------------------
void Input::destroy()
{
#ifdef LOG_INPUT
   if ( gInputLog )
   {
      log( "*** CLOSING LOG ***\n" );
      CloseHandle( gInputLog );
      gInputLog = NULL;
   }
#endif

   // turn us off.
   if (gInputEnabled)
      disable();
   
/*
   if ( smManager && smManager->isEnabled() )
   {
      smManager->disable();
      delete smManager;
      smManager = NULL;
   }
*/
}

//------------------------------------------------------------------------------
bool Input::enable()
{
//Con::printf( "[]Input::enable." );

   gInputEnabled = true;

//   if ( smManager && !smManager->isEnabled() )
//      return( smManager->enable() );

   enableMouse();
   //enableKeyboard();

   return( gInputEnabled );
}

//------------------------------------------------------------------------------
void Input::disable()
{
//Con::printf( "[]Input::disable." );

   gInputEnabled = false;

//  if ( smManager && smManager->isEnabled() )
//      smManager->disable();

   disableMouse();
   //disableKeyboard();
}

//------------------------------------------------------------------------------
void Input::activate()
{
//Con::printf( "[]Input::activate." );

   smActive = true;

#if __APPLE__ // Mach-O

#else // carbon.
   if (gISpAround)
   {
      ISpResume();
      
      enableMouse();
//      enableKeyboard());
   }
#endif

/*
   DInputDevice::resetModifierKeys();
   if ( !Con::getBoolVariable( "$enableDirectInput" ) )
      return;

   if ( smManager && smManager->isEnabled() && !smActive )
   {
      Con::printf( "Activating DirectInput..." );
#ifdef LOG_INPUT
      Input::log( "Activating DirectInput...\n" );
#endif
      smActive = true;
      DInputManager* dInputManager = dynamic_cast<DInputManager*>( smManager );
      if ( dInputManager )
      {
         if ( dInputManager->isKeyboardEnabled() )
            dInputManager->activateKeyboard();

         if ( Video::isFullScreen() )
         {
            // DirectInput Mouse Hook-Up:
            if ( dInputManager->isMouseEnabled() )
               dInputManager->activateMouse();
         }
         else
            dInputManager->deactivateMouse();

         if ( dInputManager->isJoystickEnabled() )
            dInputManager->activateJoystick();
      }
   }
*/
}

//------------------------------------------------------------------------------
void Input::deactivate()
{
//Con::printf( "[]Input::deactivate." );

#if __APPLE__ // Mach-O

#else // carbon.
   if (gISpAround)
   {
      deactivateMouse();
      //deactivateKeyboard();
      ISpSuspend();

// this is now extra...
// though, extra show can't hurt.
//      ShowCursor();
   }
#endif

   smActive = false;

/*
   if ( smManager && smManager->isEnabled() && smActive )
   {
#ifdef LOG_INPUT
      Input::log( "Deactivating DirectInput...\n" );
#endif
      DInputManager* dInputManager = dynamic_cast<DInputManager*>( smManager );
      if ( dInputManager )
      {
         dInputManager->deactivateKeyboard();
         dInputManager->deactivateMouse();
         dInputManager->deactivateJoystick();
      }

      smActive = false;
      Con::printf( "DirectInput deactivated." );
   }
*/
}

//------------------------------------------------------------------------------
void Input::reactivate()
{
   // don't think mac needs to do anything right now!!!!!! TBD
   
   // This is soo hacky...
//   SetForegroundWindow( winState.appWindow );
//   PostMessage( winState.appWindow, WM_ACTIVATE, WA_ACTIVE, NULL );
}

//------------------------------------------------------------------------------
bool Input::isEnabled()
{
//   if ( smManager )
//      return smManager->isEnabled();

   return(gInputEnabled);
}

//------------------------------------------------------------------------------
bool Input::isActive()
{
   return smActive;
}

//------------------------------------------------------------------------------
void Input::process()
{
   if (!smActive || !gInputEnabled)
      return;
      
#if __APPLE__ // Mach-O

#else // carbon.
   if (gISpAround)
   if (gMouseEnabled && gMouseActive)
   {
      ISpElementReference currEl;
      ISpElementKind currKind;
      ISpElementLabel currLabel;
      ISpElementEvent mouseMove;
      Boolean gotEvent;
      UInt32 simple;
      UInt32 btnNum = 0;
      static int btnState[8] = {0,0,0,0,0,0,0,0};
      bool btnChange;

//#define MOUSE_DATA_INC      (256.0)
#define MOUSE_DATA_INC      (128.0)
#define INV_MOUSERANGE      (1.0/MOUSE_DATA_INC)

#define MOUSE_START_VAL      -111
      static int mouseX = MOUSE_START_VAL;
      static int mouseY = MOUSE_START_VAL;
      static int mouseZ = MOUSE_START_VAL;

      InputEvent event;
      
      // loop through the mouse's elements that we extracted, and
      for (int i=0; i<mouseElCount; i++)
      {
         // aliases for cleaner code...
         currEl = mouseEl[i];
         currKind = mouseElInfo[i].theKind;
         currLabel = mouseElInfo[i].theLabel;
         
         if (currKind == kISpElementKind_Delta
         ||  currKind == kISpElementKind_Axis)
         {
            while (1)
            {
               status = ISpElement_GetNextEvent(currEl, sizeof(mouseMove), &mouseMove, &gotEvent);
               if (status!=noErr) break;
               if (!gotEvent) break;
               
               event.deviceInst = 0;
               event.deviceType = MouseDeviceType;
               event.objInst = 0;
               event.modifier = 0; //modifierKeys; //!!!!!!TBD how to check.
               event.ascii = 0;
               event.action = SI_MOVE;
               if (currKind == kISpElementKind_Delta)
               {
                  event.objType = (currLabel==kISpElementLabel_Delta_X) ? SI_XAXIS : SI_YAXIS;
                  event.fValue = F32((S32)mouseMove.data)*INV_MOUSERANGE*((event.objType==SI_YAXIS)?-1.0:1.0);
               }
               else // currKind == kISpElementKind_Axis)
               {
//                  event.objType = (currLabel==kISpElementLabel_Axis_XAxis) ? SI_XAXIS : SI_YAXIS;
//                  event.fValue = F32((S32)mouseMove.data - kISpAxisMiddle);
               }
//               if (event.objType==SI_XAXIS)
//                   Con::printf( "EVENT: Mouse move (%.1f, 0.0).\n", event.fValue );
               
               if (event.fValue != 0.0)
                  Game->postEvent(event);
            }
         }
         else
         if (currKind == kISpElementKind_Button)
         {
            btnChange = false;
            status = ISpElement_GetSimpleState(currEl, &simple);
            if (status==noErr)
            {
               event.deviceInst = 0;
               event.deviceType = MouseDeviceType;
               event.objType    = SI_BUTTON;
               event.objInst    = KEY_BUTTON0 + btnNum;
               event.modifier = 0;
               event.ascii    = 0;
               if (simple == kISpButtonDown)
               {
                  if (!btnState[btnNum])
                  {
                     event.action = SI_MAKE;
                     event.fValue = 1.0;
                     btnState[btnNum] = 1;
                     btnChange = true;
                  }
               }
               else
               {
                  if (btnState[btnNum])
                  {
                     event.action = SI_BREAK;
                     event.fValue = 0.0;
                     btnState[btnNum] = 0;
                     btnChange = true;
                  }
               }
               
               if (btnChange)
                  Game->postEvent(event);
            }
            
            // we need to keep a count of which button is which, in order.
            // !!!!!TBD can we look at the info string to figure out which is which?
            btnNum++;
         }
         // else we don't handle it.

         // we want to flush the ISp event queue for this els.
         ISpElement_Flush(currEl);
      }
   }
#endif


//   if ( smManager && smManager->isEnabled() && smActive )
//      smManager->process();
}

//------------------------------------------------------------------------------
InputManager* Input::getManager()
{
   return( smManager );
}


//--------------------------------------------------------------------------
#pragma message("input remap table might need tweaking - rumors of ibooks having diff virt keycodes, might need intermediate remap...")
static U8 VcodeRemap[256] =
{
KEY_A,                     // 0x00 
KEY_S,                     // 0x01 
KEY_D,                     // 0x02 
KEY_F,                     // 0x03 
KEY_H,                     // 0x04 
KEY_G,                     // 0x05 
KEY_Z,                     // 0x06 
KEY_X,                     // 0x07 
KEY_C,                     // 0x08 
KEY_V,                     // 0x09 
KEY_Y,                     // 0x0A       // this is questionable - not normal Y code
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
KEY_EQUALS,                // 0x18 
KEY_9,                     // 0x19 
KEY_7,                     // 0x1A 
KEY_MINUS,                 // 0x1B 
KEY_8,                     // 0x1C 
KEY_0,                     // 0x1D 
KEY_RBRACKET,              // 0x1E 
KEY_O,                     // 0x1F 
KEY_U,                     // 0x20 
KEY_LBRACKET,              // 0x21 
KEY_I,                     // 0x22 
KEY_P,                     // 0x23 
KEY_RETURN,                // 0x24 
KEY_L,                     // 0x25 
KEY_J,                     // 0x26 
KEY_APOSTROPHE,            // 0x27 
KEY_K,                     // 0x28 
KEY_SEMICOLON,             // 0x29 
KEY_BACKSLASH,             // 0x2A 
KEY_COMMA,                 // 0x2B 
KEY_SLASH,                 // 0x2C 
KEY_N,                     // 0x2D 
KEY_M,                     // 0x2E 
KEY_PERIOD,                // 0x2F 
KEY_TAB,                   // 0x30 
KEY_SPACE,                 // 0x31 
KEY_TILDE,                 // 0x32 
KEY_BACKSPACE,             // 0x33 
0,                         // 0x34 //?
KEY_ESCAPE,                // 0x35 
0,                         // 0x36 //?
KEY_ALT,                   // 0x37 // best mapping for mac Cmd key
KEY_LSHIFT,                // 0x38 
KEY_CAPSLOCK,              // 0x39 
KEY_MAC_OPT,               // 0x3A // direct map mac Option key -- better than KEY_WIN_WINDOWS
KEY_CONTROL,               // 0x3B 
KEY_RSHIFT,                // 0x3C 
0,                         // 0x3D 
0,                         // 0x3E 
0,                         // 0x3F 
0,                         // 0x40 
KEY_DECIMAL,               // 0x41 
0,                         // 0x42 
KEY_MULTIPLY,              // 0x43 
0,                         // 0x44 
KEY_ADD,                   // 0x45 
KEY_SUBTRACT,              // 0x46 // secondary code?
KEY_NUMLOCK,               // 0x47 // also known as Clear on mac...
KEY_SEPARATOR,             // 0x48 // secondary code? for KPEqual
0,                         // 0x49 
0,                         // 0x4A 
KEY_DIVIDE,                // 0x4B 
KEY_NUMPADENTER,           // 0x4C 
KEY_DIVIDE,                // 0x4D // secondary code?
KEY_SUBTRACT,              // 0x4E 
0,                         // 0x4F 
0,                         // 0x50 
KEY_SEPARATOR,             // 0x51 // WHAT IS SEP?  This is KPEqual on mac.
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
KEY_PAUSE,                 // 0x71 
KEY_INSERT,                // 0x72 // also known as mac Help
KEY_HOME,                  // 0x73 
KEY_PAGE_UP,               // 0x74 
KEY_DELETE,                // 0x75 // FwdDel
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


//-----------------------------------------------------------------------------
// Clipboard functions
const char* Platform::getClipboard()
{
/*
   HGLOBAL hGlobal;
   LPVOID  pGlobal;

   //make sure we can access the clipboard
   if (!IsClipboardFormatAvailable(CF_TEXT)) 
      return ""; 
   if (!OpenClipboard(NULL))
      return "";

   hGlobal = GetClipboardData(CF_TEXT);
   pGlobal = GlobalLock(hGlobal);
   S32 cbLength = strlen((char *)pGlobal);
   char  *returnBuf = Con::getReturnBuffer(cbLength + 1);
   strcpy(returnBuf, (char *)pGlobal);
   returnBuf[cbLength] = '\0';
   GlobalUnlock(hGlobal);
   CloseClipboard();
*/
   char  *returnBuf = Con::getReturnBuffer(16);
   returnBuf[0] = 0;
   
   //note - this function never returns NULL
   return returnBuf;
}

//-----------------------------------------------------------------------------
bool Platform::setClipboard(const char *text)
{
/*
   if (!text)
      return false;

   //make sure we can access the clipboard
   if (!OpenClipboard(NULL))
      return false;

   S32 cbLength = strlen(text);

   HGLOBAL hGlobal;
   LPVOID  pGlobal;

   hGlobal = GlobalAlloc(GHND, cbLength + 1);
   pGlobal = GlobalLock (hGlobal);

   strcpy((char *)pGlobal, text);

   GlobalUnlock(hGlobal);

   EmptyClipboard();
   SetClipboardData(CF_TEXT, hGlobal);
   CloseClipboard();

   return true;
*/
      return false;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
bool enableKeyboard()
{
#if __APPLE__
#else
   if ( !gISpAround ) return(false);
#endif

   if ( !gInputEnabled )
      return( false );

   if ( gKBEnabled && gKBActive )
      return( true );

   gKBEnabled = true;
   if ( Input::isActive() )
      gKBEnabled = activateKeyboard();

   if ( gKBEnabled )
   {
      Con::printf( "Hardware-direct keyboard enabled." );
#ifdef LOG_INPUT
      Input::log( "Keyboard enabled.\n" );
#endif
   }
   else
   {
      Con::warnf( "Hardware-direct keyboard failed to enable!" );
#ifdef LOG_INPUT
      Input::log( "Keyboard failed to enable!\n" );
#endif
   }

   return( gKBEnabled );
}

//------------------------------------------------------------------------------
void disableKeyboard()
{
#if __APPLE__
#else
   if ( !gISpAround ) return;
#endif

   if ( !gInputEnabled || !gKBEnabled )
      return;

   deactivateKeyboard();
   gKBEnabled = false;

   Con::printf( "Hardware-direct keyboard disabled." );
#ifdef LOG_INPUT
   Input::log( "Keyboard disabled.\n" );
#endif
}

//------------------------------------------------------------------------------
bool activateKeyboard()
{
#if __APPLE__
#else
   if ( !gISpAround ) return(false);
#endif

   if ( !gInputEnabled || !Input::isActive() || !gKBEnabled )
      return( false );

#if __APPLE__
#else
   status = ISpDevices_Activate(1, &kbRef);
   if (status==noErr)
      gKBActive = true;
#endif

#ifdef LOG_INPUT
   Input::log( gKBActive ? "Keyboard activated.\n" : "Keyboard failed to activate!\n" );
#endif
   return( gKBActive );
}

//------------------------------------------------------------------------------
void deactivateKeyboard()
{
#if __APPLE__
#else
   if ( !gISpAround ) return;
#endif

   if ( gInputEnabled && gKBActive )
   {
#if __APPLE__
#else
      status = ISpDevices_Deactivate(1, &kbRef);
//      if (status!=noErr) goto initError;
      gKBActive = false;
#endif

#ifdef LOG_INPUT
      Input::log( "Keyboard deactivated.\n" );
#endif
   }
}

//------------------------------------------------------------------------------
bool enableMouse()
{
#if __APPLE__
#else
   if ( !gISpAround ) return (false);
#endif

//   Con::printf( ">> em one." );

   if ( !gInputEnabled || noDirectMouse )
      return( false );

//   Con::printf( ">> em two." );

   if ( gMouseEnabled && gMouseActive )
      return( true );

//   Con::printf( ">> em three." );

   gMouseEnabled = true; //i.e., allowed.  needed to call activate...
   gMouseEnabled = activateMouse();

   if ( gMouseEnabled )
   {
      Con::printf( "Hardware-direct mouse enabled." );
#ifdef LOG_INPUT
      Input::log( "Mouse enabled.\n" );
#endif
   }
   else
   {
      Con::warnf( "Hardware-direct mouse failed to enable!" );
#ifdef LOG_INPUT
      Input::log( "Mouse failed to enable!\n" );
#endif
   }

   return( gMouseEnabled );
}

//------------------------------------------------------------------------------
void disableMouse()
{
#if __APPLE__
#else
   if ( !gISpAround ) return;
#endif

   if ( !gInputEnabled || !gMouseEnabled )
      return;

   deactivateMouse();
   gMouseEnabled = false;

   Con::printf( "Hardware-direct mouse disabled." );
#ifdef LOG_INPUT
   Input::log( "Mouse disabled.\n" );
#endif
}

//------------------------------------------------------------------------------
bool activateMouse()
{
#if __APPLE__
#else
   if ( !gISpAround ) return(false);
#endif

//   Con::printf( ">> am one. %s %s %s", gInputEnabled?"true":"false", Input::isActive()?"true":"false", gMouseEnabled?"true":"false");
   if ( !gInputEnabled || !Input::isActive() || !gMouseEnabled )
      return( false );
   
   if (gMouseActive)
      return(true); // we're already set up.

//   Con::printf( ">> am two." );
#if __APPLE__
#else
   status = ISpDevices_Activate(1, &mouseRef);
   if (status==noErr)
   {
      gMouseActive = true;
      HideCursor();
//      Con::printf( ">> am three." );
   }
#endif

//   Con::printf( ">> am four." );

#ifdef LOG_INPUT
   Input::log( gMouseActive ? "Mouse activated.\n" : "Mouse failed to activate!\n" );
#endif
   return( gMouseActive );
}

//------------------------------------------------------------------------------
void deactivateMouse()
{
#if __APPLE__
#else
   if ( !gISpAround ) return;
#endif

//   Con::printf( ">> dm one. %s %s %s", gInputEnabled?"true":"false", gMouseActive?"true":"false");
   if ( !gInputEnabled || !gMouseActive ) return;
   
#if __APPLE__
#else
   status = ISpDevices_Deactivate(1, &mouseRef);
//   if (status!=noErr) goto initError;
   gMouseActive = false;
   ShowCursor();  
#endif

//   Con::printf( ">> dm two." );

#ifdef LOG_INPUT
   Input::log( "Mouse deactivated.\n" );
#endif
}


//------------------------------------------------------------------------------
ConsoleFunction( enableMouse, bool, 1, 1, "enableMouse()" )
{
   return( enableMouse() );
}

//------------------------------------------------------------------------------
ConsoleFunction( disableMouse, void, 1, 1, "disableMouse()" )
{
   disableMouse();
}

//------------------------------------------------------------------------------
ConsoleFunction( permDisableMouse, void, 1, 1, "permDisableMouse()" )
{
   disableMouse();
   noDirectMouse = TRUE;
}

//------------------------------------------------------------------------------
void printInputState(void)
{
#if __APPLE__
#else
   if ( !gISpAround ) return;
#endif

   if ( gInputEnabled )
   {
#if __APPLE__
#else
      Con::printf( "InputSprocket is enabled %s.", Input::isActive() ? "and active" : "but inactive" );
#endif
      Con::printf( "- Keyboard is %sabled and %sactive.", 
            gKBEnabled ? "en" : "dis",
            gKBActive ? "" : "in" );
      Con::printf( "- Mouse is %sabled and %sactive.", 
            gMouseEnabled ? "en" : "dis",
            gMouseActive ? "" : "in" );
/*
      Con::printf( "- Joystick is %sabled and %sactive.", 
            gJoystickEnabled() ? "en" : "dis",
            gJoystickActive() ? "" : "in" );
*/
   }
   else
#if __APPLE__
#else
      Con::printf( "InputSprocket is not currently enabled." );
#endif
}

//------------------------------------------------------------------------------
ConsoleFunction( echoInputState, void, 1, 1, "echoInputState()" )
{
   printInputState();
}

//------------------------------------------------------------------------------
ConsoleFunction( toggleInputState, void, 1, 1, "toggleInputState()" )
{
   if (gInputEnabled)
      Input::disable();
   else
      Input::enable();

   printInputState();
}
