//-----------------------------------------------------------------------------
// Torque Game Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "platformX86UNIX/platformX86UNIX.h"
#include "console/consoleTypes.h"
#include "platform/event.h"
#include "platform/gameInterface.h"
#include "platformX86UNIX/x86UNIXState.h"
#include "platformX86UNIX/x86UNIXInputManager.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#include <SDL/SDL.h>

// ascii table
AsciiData AsciiTable[NUM_KEYS];

// keymap table
static const U32 SDLtoTKeyMapSize = SDLK_LAST;
static U8 SDLtoTKeyMap[SDLtoTKeyMapSize];
static bool keyMapsInitialized = false;

// helper functions
static void MapKey(SDLKey SDLkey, U8 tkey, KeySym xkeysym);
static void InitKeyMaps();
static inline U8 TranslateSDLKeytoTKey(SDLKey keysym);

// JMQTODO: these should go in platform state
extern bool windowLocked;
extern bool windowActive;

// unix platform state
extern x86UNIXPlatformState * x86UNIXState;

// constants

static const U32 MouseMask = SDL_MOUSEEVENTMASK;
static const U32 KeyboardMask = SDL_KEYUPMASK | SDL_KEYDOWNMASK;
static const U32 JoystickMask = SDL_JOYEVENTMASK;

static const U32 AllInputEvents = MouseMask | KeyboardMask | JoystickMask;

//==============================================================================
// Static helper functions
//==============================================================================
static void MapKey(SDLKey SDLkey, U8 tkey, KeySym xkeysym)
{ 
   SDLtoTKeyMap[SDLkey] = tkey; 

   if (xkeysym == 0)
      return;

   XKeyPressedEvent fooKey;
   const int keybufSize = 256;
   char keybuf[keybufSize];

   // find the x keycode for the keysym
   KeyCode xkeycode = XKeysymToKeycode(
      x86UNIXState->GetDisplayPointer(), xkeysym);

//   Display *dpy = XOpenDisplay(NULL);
//    KeyCode xkeycode = XKeysymToKeycode(
//       dpy, xkeysym);

   if (!xkeycode)
      return;     

   // create an event with the keycode
   dMemset(&fooKey, 0, sizeof(fooKey));
   fooKey.type = KeyPress;
   fooKey.display = x86UNIXState->GetDisplayPointer();
   fooKey.window = DefaultRootWindow(x86UNIXState->GetDisplayPointer());
   fooKey.time = CurrentTime;
   fooKey.keycode = xkeycode;

   // translate the event with no modifiers (yields lowercase)
   KeySym dummyKeySym;
   int numChars = XLookupString(
      &fooKey, keybuf, keybufSize, &dummyKeySym, NULL);
   if (numChars)
   {
      //Con::printf("assigning lowercase string %c", *keybuf);
      // ignore everything but first char
      AsciiTable[tkey].lower.ascii = *keybuf;
      AsciiTable[tkey].goofy.ascii = *keybuf;
   }
         
   // translate the event with shift modifier (yields uppercase)
   fooKey.state |= ShiftMask;
   numChars = XLookupString(&fooKey, keybuf, keybufSize, &dummyKeySym, NULL);
   if (numChars)
   {
      //Con::printf("assigning uppercase string %c", *keybuf);
      // ignore everything but first char
      AsciiTable[tkey].upper.ascii = *keybuf;
   }    
   // JMQTODO: how to get goofy modifiers?
}

//------------------------------------------------------------------------------
void InitKeyMaps()
{
   dMemset( &AsciiTable, 0, sizeof( AsciiTable ) );
   dMemset(SDLtoTKeyMap, KEY_NULL, SDLtoTKeyMapSize);
   
   // set up the X to Torque key map
   // stuff
   MapKey(SDLK_BACKSPACE, KEY_BACKSPACE, XK_BackSpace);
   MapKey(SDLK_TAB, KEY_TAB, XK_Tab);
   MapKey(SDLK_RETURN, KEY_RETURN, XK_Return);
   MapKey(SDLK_PAUSE, KEY_PAUSE, XK_Pause);
   MapKey(SDLK_CAPSLOCK, KEY_CAPSLOCK, XK_Caps_Lock);
   MapKey(SDLK_ESCAPE, KEY_ESCAPE, XK_Escape);

   // more stuff
   MapKey(SDLK_SPACE, KEY_SPACE, XK_space);
   MapKey(SDLK_PAGEDOWN, KEY_PAGE_DOWN, XK_Page_Down);
   MapKey(SDLK_PAGEUP, KEY_PAGE_UP, XK_Page_Up);
   MapKey(SDLK_END, KEY_END, XK_End);
   MapKey(SDLK_HOME, KEY_HOME, XK_Home);
   MapKey(SDLK_LEFT, KEY_LEFT, XK_Left);
   MapKey(SDLK_UP, KEY_UP, XK_Up);
   MapKey(SDLK_RIGHT, KEY_RIGHT, XK_Right);
   MapKey(SDLK_DOWN, KEY_DOWN, XK_Down);
   MapKey(SDLK_PRINT, KEY_PRINT, XK_Print);
   MapKey(SDLK_INSERT, KEY_INSERT, XK_Insert);
   MapKey(SDLK_DELETE, KEY_DELETE, XK_Delete);
   
   S32 keysym;
   S32 tkeycode;
   KeySym xkey;
   // main numeric keys
   for (keysym = SDLK_0, tkeycode = KEY_0, xkey = XK_0;
        keysym <= SDLK_9; 
        ++keysym, ++tkeycode, ++xkey)
      MapKey(static_cast<SDLKey>(keysym), tkeycode, xkey);
   
   // lowercase letters
   for (keysym = SDLK_a, tkeycode = KEY_A, xkey = XK_a; 
        keysym <= SDLK_z; 
        ++keysym, ++tkeycode, ++xkey)
      MapKey(static_cast<SDLKey>(keysym), tkeycode, xkey);

   // various punctuation
   MapKey(SDLK_BACKQUOTE, KEY_TILDE, XK_grave);
   MapKey(SDLK_MINUS, KEY_MINUS, XK_minus);
   MapKey(SDLK_EQUALS, KEY_EQUALS, XK_equal);
   MapKey(SDLK_LEFTBRACKET, KEY_LBRACKET, XK_bracketleft);
   MapKey(SDLK_RIGHTBRACKET, KEY_RBRACKET, XK_bracketright);
   MapKey(SDLK_BACKSLASH, KEY_BACKSLASH, XK_backslash);
   MapKey(SDLK_SEMICOLON, KEY_SEMICOLON, XK_semicolon);
   MapKey(SDLK_QUOTE, KEY_APOSTROPHE, XK_apostrophe);
   MapKey(SDLK_COMMA, KEY_COMMA, XK_comma);
   MapKey(SDLK_PERIOD, KEY_PERIOD, XK_period);
   MapKey(SDLK_SLASH, KEY_SLASH, XK_slash); 

   // numpad numbers
   for (keysym = SDLK_KP0, tkeycode = KEY_NUMPAD0, xkey = XK_KP_0; 
        keysym <= SDLK_KP9; 
        ++keysym, ++tkeycode, ++xkey)
      MapKey(static_cast<SDLKey>(keysym), tkeycode, xkey);

   // other numpad stuff
   MapKey(SDLK_KP_MULTIPLY, KEY_MULTIPLY, XK_KP_Multiply);
   MapKey(SDLK_KP_PLUS, KEY_ADD, XK_KP_Add);
   MapKey(SDLK_KP_EQUALS, KEY_SEPARATOR, XK_KP_Separator);
   MapKey(SDLK_KP_MINUS, KEY_SUBTRACT, XK_KP_Subtract);
   MapKey(SDLK_KP_PERIOD, KEY_DECIMAL, XK_KP_Decimal);
   MapKey(SDLK_KP_DIVIDE, KEY_DIVIDE, XK_KP_Divide);
   MapKey(SDLK_KP_ENTER, KEY_NUMPADENTER, XK_KP_Enter);

   // F keys
   for (keysym = SDLK_F1, tkeycode = KEY_F1, xkey = XK_F1; 
        keysym <= SDLK_F15; 
        ++keysym, ++tkeycode, ++xkey)
      MapKey(static_cast<SDLKey>(keysym), tkeycode, xkey);

   // various modifiers
   MapKey(SDLK_NUMLOCK, KEY_NUMLOCK, XK_Num_Lock);
   MapKey(SDLK_SCROLLOCK, KEY_SCROLLLOCK, XK_Scroll_Lock);
   MapKey(SDLK_LCTRL, KEY_LCONTROL, XK_Control_L);
   MapKey(SDLK_RCTRL, KEY_RCONTROL, XK_Control_R);
   MapKey(SDLK_LALT, KEY_LALT, XK_Alt_L);
   MapKey(SDLK_RALT, KEY_RALT, XK_Alt_R);
   MapKey(SDLK_LSHIFT, KEY_LSHIFT, XK_Shift_L);
   MapKey(SDLK_RSHIFT, KEY_RSHIFT, XK_Shift_R);
   MapKey(SDLK_LSUPER, KEY_WIN_LWINDOW, 0);
   MapKey(SDLK_RSUPER, KEY_WIN_RWINDOW, 0);

   // these are unsupported
//    case KEY_WIN_APPS:      return DIK_APPS;
//    case KEY_OEM_102:       return DIK_OEM_102;

   keyMapsInitialized = true;
};

//------------------------------------------------------------------------------
U8 TranslateSDLKeytoTKey(SDLKey keysym)
{
   if (!keyMapsInitialized)
   {
      Con::printf("WARNING: SDLkeysymMap is not initialized");
      return 0;
   }
   if (keysym < 0 || 
      static_cast<U32>(keysym) >= SDLtoTKeyMapSize)
   {
      Con::printf("WARNING: invalid keysym: %d", keysym);
      return 0;
   }
   return SDLtoTKeyMap[keysym];
}

//------------------------------------------------------------------------------
// this shouldn't be used, use TranslateSDLKeytoTKey instead
U8 TranslateOSKeyCode(U8 vcode)
{
   Con::printf("WARNING: TranslateOSKeyCode is not supported in unix");
   return 0;
}

//==============================================================================
// UInputManager
//==============================================================================
UInputManager::UInputManager()
{
   mActive = false;
   mEnabled = false;
   mKeyboardEnabled = mMouseEnabled = mJoystickEnabled = false;
   mKeyboardActive = mMouseActive = mJoystickActive = false;
}

//------------------------------------------------------------------------------
void UInputManager::init()
{
   Con::addVariable( "pref::Input::KeyboardEnabled",  
      TypeBool, &mKeyboardEnabled );
   Con::addVariable( "pref::Input::MouseEnabled",     
      TypeBool, &mMouseEnabled );
   Con::addVariable( "pref::Input::JoystickEnabled",  
      TypeBool, &mJoystickEnabled );
}

//------------------------------------------------------------------------------
bool UInputManager::enable()
{
   disable();
#ifdef LOG_INPUT
   Input::log( "Enabling Input...\n" );
#endif

   mModifierKeys = 0;
   dMemset( mMouseButtonState, 0, sizeof( mMouseButtonState ) );
   dMemset( mKeyboardState, 0, 256 );

   InitKeyMaps();

   mJoystickEnabled = false;
   initJoystick();

   mEnabled = true;
   mMouseEnabled = true;
   mKeyboardEnabled = true;

   return true;     
}

//------------------------------------------------------------------------------
void UInputManager::disable()
{
   deactivate();
   mEnabled = false;
   return;
}

void UInputManager::initJoystick()
{
   // initialize SDL joystick system
   if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0)
   {
      Con::warnf("   Unable to initialize joystick: %s", SDL_GetError());
      return;
   }

   int numJoysticks = SDL_NumJoysticks();
   if (numJoysticks == 0)
      Con::printf("   No joysticks found.");

   // enable joystick events
   SDL_JoystickEventState(SDL_ENABLE);

   // install joysticks
   for(int i = 0; i < numJoysticks; i++ ) 
   {
      JoystickInputDevice* newDevice = new JoystickInputDevice(i);
      addObject(newDevice);
      Con::printf("   %s: %s", 
         newDevice->getDeviceName(), newDevice->getName());
   }

   mJoystickEnabled = true;
}

//------------------------------------------------------------------------------
void UInputManager::activate()
{
   if (mEnabled && !isActive())
   {
      mActive = true;
      SDL_ShowCursor(SDL_DISABLE);
      resetInputState();
      activateMouse();
      activateKeyboard();
      activateJoystick();
      if (windowLocked)
         lockInput();
   }
}

//------------------------------------------------------------------------------
void UInputManager::deactivate()
{
   if (mEnabled && isActive())
   {
      unlockInput();
      deactivateKeyboard();
      deactivateMouse();
      deactivateJoystick();
      resetInputState();
      SDL_ShowCursor(SDL_ENABLE);
      mActive = false;
   }
}

//------------------------------------------------------------------------------
void UInputManager::resetKeyboardState()
{
   // unpress any pressed keys; in the future we may want
   // to actually sync with the keyboard state
   for (int i = 0; i < 256; ++i)
   {
      if (mKeyboardState[i])
      {
         InputEvent event;
         
         event.deviceInst = 0;
         event.deviceType = KeyboardDeviceType;
         event.objType = SI_KEY;
         event.objInst = i;
         event.action = SI_BREAK;
         event.fValue = 0.0;
         Game->postEvent(event);
      }
   }
   dMemset(mKeyboardState, 0, 256);

   // clear modifier keys
   mModifierKeys = 0;
}

//------------------------------------------------------------------------------
void UInputManager::resetMouseState()
{
   // unpress any buttons; in the future we may want
   // to actually sync with the mouse state
   for (int i = 0; i < 3; ++i)
   {
      if (mMouseButtonState[i])
      {
         // add KEY_BUTTON0 to the index to get the real
         // button ID
         S32 buttonID = i + KEY_BUTTON0;
         InputEvent event;
        
         event.deviceInst = 0;
         event.deviceType = MouseDeviceType;
         event.objType = SI_BUTTON;
         event.objInst = buttonID;
         event.action = SI_BREAK;
         event.fValue = 0.0;
         Game->postEvent(event);
      }
   }

   dMemset(mMouseButtonState, 0, 3);
}

//------------------------------------------------------------------------------
void UInputManager::resetInputState()
{
   resetKeyboardState();
   resetMouseState();

   // JMQTODO: make event arrays be members
   // dispose of any lingering SDL input events
   static const int MaxEvents = 255;
   static SDL_Event events[MaxEvents];
   SDL_PumpEvents();
   SDL_PeepEvents(events, MaxEvents, SDL_GETEVENT, 
      AllInputEvents);
   // JMQTODO: joystick events
}

//------------------------------------------------------------------------------
void UInputManager::lockInput()
{
   if (windowActive && windowLocked && 
      SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_OFF)
      SDL_WM_GrabInput(SDL_GRAB_ON);
}

//------------------------------------------------------------------------------
void UInputManager::unlockInput()
{
   if (SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_ON)
      SDL_WM_GrabInput(SDL_GRAB_OFF);
}

//------------------------------------------------------------------------------
void UInputManager::onDeleteNotify( SimObject* object )
{
   Parent::onDeleteNotify( object );
}

//------------------------------------------------------------------------------
bool UInputManager::onAdd()
{
   if ( !Parent::onAdd() )
      return false;

   return true;
}

//------------------------------------------------------------------------------
void UInputManager::onRemove()
{
   deactivate();
   Parent::onRemove();
}

//------------------------------------------------------------------------------
void UInputManager::mouseMotionEvent(const SDL_Event& event)
{
//    Con::printf("motion event: %d %d %d %d",
//       event.motion.xrel, event.motion.yrel,
//       event.motion.x, event.motion.y);
   if (windowLocked)
   {
      InputEvent ievent;
      ievent.deviceInst = 0;
      ievent.deviceType = MouseDeviceType;
      ievent.objInst = 0;
      ievent.modifier = mModifierKeys;
      ievent.ascii = 0;
      ievent.action = SI_MOVE;
            
      // post events if things have changed
      if (event.motion.xrel != 0)
      {
         ievent.objType = SI_XAXIS;
         ievent.fValue = event.motion.xrel;
         Game->postEvent(ievent);
      }
      if (event.motion.yrel != 0)
      {
         ievent.objType = SI_YAXIS;
         ievent.fValue = event.motion.yrel; 
         Game->postEvent(ievent);
      }
   }
   else
   {
      MouseMoveEvent mmevent;
      mmevent.xPos = mLastMouseX = event.motion.x;
      mmevent.yPos = mLastMouseY = event.motion.y;
      mmevent.modifier = mModifierKeys;
      Game->postEvent(mmevent);
   }
}

//------------------------------------------------------------------------------
void UInputManager::mouseButtonEvent(const SDL_Event& event)
{
   S32 action = (event.type == SDL_MOUSEBUTTONDOWN) ? SI_MAKE : SI_BREAK;
   S32 objInst = -1;
   // JMQTODO: support wheel delta like windows version?
   // JMQTODO: make this value configurable?
   S32 wheelDelta = 10;
   bool wheel = false;

   switch (event.button.button)
   {
      case SDL_BUTTON_LEFT:
         objInst = KEY_BUTTON0;
         break;
      case SDL_BUTTON_RIGHT:
         objInst = KEY_BUTTON1;
         break;
      case SDL_BUTTON_MIDDLE:
         objInst = KEY_BUTTON2;
         break;
      case Button4:
         wheel = true;
         break;
      case Button5:
         wheel = true;
         wheelDelta = -wheelDelta;
         break;
   }

   if (objInst == -1 && !wheel)
      // unsupported button
      return;

   InputEvent ievent;

   ievent.deviceInst = 0;
   ievent.deviceType = MouseDeviceType;
   ievent.modifier = mModifierKeys;
   ievent.ascii = 0;

   if (wheel)
   {
      // SDL generates a button press/release for each wheel move,
      // so ignore breaks to translate those into a single event
      if (action == SI_BREAK)
         return;
      ievent.objType = SI_ZAXIS;
      ievent.objInst = 0;
      ievent.action = SI_MOVE;
      ievent.fValue = wheelDelta;
   }
   else // regular button
   {
      S32 buttonID = (objInst - KEY_BUTTON0);
      if (buttonID < 3)
         mMouseButtonState[buttonID] = ( action == SI_MAKE ) ? true : false;

      ievent.objType = SI_BUTTON;
      ievent.objInst = objInst;
      ievent.action = action;
      ievent.fValue = (action == SI_MAKE) ? 1.0 : 0.0;
   }

   Game->postEvent(ievent);
}

//------------------------------------------------------------------------------
void UInputManager::keyEvent(const SDL_Event& event)
{
   S32 action = (event.type == SDL_KEYDOWN) ? SI_MAKE : SI_BREAK;
   InputEvent ievent;

   ievent.deviceInst = 0;
   ievent.deviceType = KeyboardDeviceType;
   ievent.objType = SI_KEY;
   ievent.objInst = TranslateSDLKeytoTKey(event.key.keysym.sym);
   ievent.action = action;
   ievent.fValue = (action == SI_MAKE) ? 1.0 : 0.0;

   processKeyEvent(ievent);
   Game->postEvent(ievent);
}

//------------------------------------------------------------------------------
// This function was ripped from DInputDevice almost entirely intact.  
bool UInputManager::processKeyEvent( InputEvent &event )
{
   if ( event.deviceType != KeyboardDeviceType || event.objType != SI_KEY )
      return false;

   bool modKey = false;
   U8 keyCode = event.objInst;

   if ( event.action == SI_MAKE )
   {
      // Maintain the key structure:
      mKeyboardState[keyCode] = true;

      switch ( event.objInst )
      {
         case KEY_LSHIFT:
            mModifierKeys |= SI_LSHIFT;
            modKey = true;
            break;

         case KEY_RSHIFT:
            mModifierKeys |= SI_RSHIFT;
            modKey = true;
            break;

         case KEY_LCONTROL:
            mModifierKeys |= SI_LCTRL;
            modKey = true;
            break;

         case KEY_RCONTROL:
            mModifierKeys |= SI_RCTRL;
            modKey = true;
            break;

         case KEY_LALT:
            mModifierKeys |= SI_LALT;
            modKey = true;
            break;

         case KEY_RALT:
            mModifierKeys |= SI_RALT;
            modKey = true;
            break;
      }
   }
   else
   {
      // Maintain the keys structure:
      mKeyboardState[keyCode] = false;

      switch ( event.objInst )
      {
         case KEY_LSHIFT:
            mModifierKeys &= ~SI_LSHIFT;
            modKey = true;
            break;

         case KEY_RSHIFT:
            mModifierKeys &= ~SI_RSHIFT;
            modKey = true;
            break;

         case KEY_LCONTROL:
            mModifierKeys &= ~SI_LCTRL;
            modKey = true;
            break;

         case KEY_RCONTROL:
            mModifierKeys &= ~SI_RCTRL;
            modKey = true;
            break;

         case KEY_LALT:
            mModifierKeys &= ~SI_LALT;
            modKey = true;
            break;

         case KEY_RALT:
            mModifierKeys &= ~SI_RALT;
            modKey = true;
            break;
      }
   }

   if ( modKey )
      event.modifier = 0;
   else
      event.modifier = mModifierKeys;

   // TODO: alter this getAscii call
   KEY_STATE state = STATE_LOWER;
   if (event.modifier & (SI_CTRL|SI_ALT) )
   {
      state = STATE_GOOFY;
   }
   if ( event.modifier & SI_SHIFT )
   {
      state = STATE_UPPER;
   }

   event.ascii = Input::getAscii( event.objInst, state );

   return modKey;
}

//------------------------------------------------------------------------------
void UInputManager::setWindowLocked(bool locked)
{
   if (locked)
      lockInput();
   else
   {
      unlockInput();
      // SDL keeps track of abs mouse position in fullscreen mode, which means
      // that if you switch to unlocked mode while fullscreen, the mouse will
      // suddenly warp to someplace unexpected on screen.  To fix this, we 
      // warp the mouse to the last known Torque abs mouse position.
      if (mLastMouseX != -1 && mLastMouseY != -1)
         SDL_WarpMouse(mLastMouseX, mLastMouseY);
   }
}

//------------------------------------------------------------------------------
void UInputManager::process()
{
   if (!mEnabled || !isActive())
      return;

   // JMQTODO: make these be class members
   static const int MaxEvents = 255;
   static SDL_Event events[MaxEvents];

   U32 mask = 0;

   if (mMouseActive)
      mask |= MouseMask;
   if (mKeyboardActive)
      mask |= KeyboardMask;
   if (mJoystickActive)
      mask |= JoystickMask;

   if (mask == 0)
      return;

   SDL_PumpEvents();
   S32 numEvents = SDL_PeepEvents(events, MaxEvents, SDL_GETEVENT, mask);
   if (numEvents == 0)
      return;

   for (int i = 0; i < numEvents; ++i)
   {
      switch (events[i].type) 
      {
         case SDL_MOUSEMOTION:
            mouseMotionEvent(events[i]);
            break;
         case SDL_MOUSEBUTTONUP:
         case SDL_MOUSEBUTTONDOWN:
            mouseButtonEvent(events[i]);
            break;
         case SDL_KEYDOWN:
         case SDL_KEYUP:
            keyEvent(events[i]);
            break;
         case SDL_JOYAXISMOTION:
            Con::printf("Axis: device: %d, axis: %d, value: %d",
               events[i].jaxis.which, events[i].jaxis.axis, events[i].jaxis.value);
            break;
         case SDL_JOYBUTTONUP:
            Con::printf("ButtonUp: device: %d, button: %d",
               events[i].jbutton.which, events[i].jbutton.button);
            break;
         case SDL_JOYBUTTONDOWN:
            Con::printf("ButtonDown: device: %d, button: %d",
               events[i].jbutton.which, events[i].jbutton.button);
            break;
         case SDL_JOYBALLMOTION:
            Con::printf("SDL_JOYBALLMOTION");
            break;
         case SDL_JOYHATMOTION: 
            Con::printf("SDL_JOYHATMOTION");
            break;
//          default:
//             Con::printf("Unknown input event: %d", events[i].type);
//             break;
      }
   }
}

//------------------------------------------------------------------------------
bool UInputManager::enableKeyboard()
{
   if ( !isEnabled() )
      return( false );

   if ( isKeyboardEnabled() && isKeyboardActive() )
      return( true );

   mKeyboardEnabled = true;
   if ( isActive() )
      mKeyboardEnabled = activateKeyboard();

   if ( mKeyboardEnabled )
   {
      Con::printf( "Keyboard enabled." );
#ifdef LOG_INPUT
      Input::log( "Keyboard enabled.\n" );
#endif
   }
   else
   {
      Con::warnf( "Keyboard failed to enable!" );
#ifdef LOG_INPUT
      Input::log( "Keyboard failed to enable!\n" );
#endif
   }
      
   return( mKeyboardEnabled );
}

//------------------------------------------------------------------------------
void UInputManager::disableKeyboard()
{
   if ( !isEnabled() || !isKeyboardEnabled())
      return;

   deactivateKeyboard();
   mKeyboardEnabled = false;
   Con::printf( "Keyboard disabled." );
#ifdef LOG_INPUT
   Input::log( "Keyboard disabled.\n" );
#endif
}

//------------------------------------------------------------------------------
bool UInputManager::activateKeyboard()
{
   if ( !isEnabled() || !isActive() || !isKeyboardEnabled() )
      return( false );

   mKeyboardActive = true;
#ifdef LOG_INPUT
   Input::log( mKeyboardActive ? "Keyboard activated.\n" : "Keyboard failed to activate!\n" );
#endif
   return( mKeyboardActive );
}

//------------------------------------------------------------------------------
void UInputManager::deactivateKeyboard()
{
   if ( isEnabled() && isKeyboardActive() )
   {
      mKeyboardActive = false;
#ifdef LOG_INPUT
      Input::log( "Keyboard deactivated.\n" );
#endif
   }
}

//------------------------------------------------------------------------------
bool UInputManager::enableMouse()
{
   if ( !isEnabled() )
      return( false );

   if ( isMouseEnabled() && isMouseActive() )
      return( true );

   mMouseEnabled = true;
   if ( isActive() )
      mMouseEnabled = activateMouse();

   if ( mMouseEnabled )
   {
      Con::printf( "Mouse enabled." );
#ifdef LOG_INPUT
      Input::log( "Mouse enabled.\n" );
#endif
   }
   else
   {
      Con::warnf( "Mouse failed to enable!" );
#ifdef LOG_INPUT
      Input::log( "Mouse failed to enable!\n" );
#endif
   }

   return( mMouseEnabled );
}

//------------------------------------------------------------------------------
void UInputManager::disableMouse()
{
   if ( !isEnabled() || !isMouseEnabled())
      return;

   deactivateMouse();
   mMouseEnabled = false;
   Con::printf( "Mouse disabled." );
#ifdef LOG_INPUT
   Input::log( "Mouse disabled.\n" );
#endif
}

//------------------------------------------------------------------------------
bool UInputManager::activateMouse()
{
   if ( !isEnabled() || !isActive() || !isMouseEnabled() )
      return( false );

   mMouseActive = true;
#ifdef LOG_INPUT
   Input::log( mMouseActive ? 
      "Mouse activated.\n" : "Mouse failed to activate!\n" );
#endif
   return( mMouseActive );
}

//------------------------------------------------------------------------------
void UInputManager::deactivateMouse()
{
   if ( isEnabled() && isMouseActive() )
   {
      mMouseActive = false;
#ifdef LOG_INPUT
      Input::log( "Mouse deactivated.\n" );
#endif
   }
}

//------------------------------------------------------------------------------
bool UInputManager::enableJoystick()
{
   if ( !isEnabled() )
      return( false );

   if ( isJoystickEnabled() && isJoystickActive() )
      return( true );

   // JMQTODO: detect joystick

   mJoystickEnabled = true;
   if ( isActive() )
      mJoystickEnabled = activateJoystick();

   if ( mJoystickEnabled )
   {
      Con::printf( "Joystick enabled." );
#ifdef LOG_INPUT
      Input::log( "Joystick enabled.\n" );
#endif
   }
   else
   {
      Con::warnf( "Joystick failed to enable!" );
#ifdef LOG_INPUT
      Input::log( "Joystick failed to enable!\n" );
#endif
   }

   return( mJoystickEnabled );
}

//------------------------------------------------------------------------------
void UInputManager::disableJoystick()
{
   if ( !isEnabled() || !isJoystickEnabled())
      return;

   deactivateJoystick();
   mJoystickEnabled = false;
   Con::printf( "Joystick disabled." );
#ifdef LOG_INPUT
   Input::log( "Joystick disabled.\n" );
#endif
}

//------------------------------------------------------------------------------
bool UInputManager::activateJoystick()
{
   if ( !isEnabled() || !isActive() || !isJoystickEnabled() )
      return( false );

   mJoystickActive = false;
   JoystickInputDevice* dptr;
   for ( iterator ptr = begin(); ptr != end(); ptr++ )
   {
      dptr = dynamic_cast<JoystickInputDevice*>( *ptr );
      if ( dptr && dptr->getDeviceType() == JoystickDeviceType)
         if ( dptr->activate() )
            mJoystickActive = true;
   }
#ifdef LOG_INPUT
   Input::log( mJoystickActive ? 
      "Joystick activated.\n" : "Joystick failed to activate!\n" );
#endif
   return( mJoystickActive );
}

//------------------------------------------------------------------------------
void UInputManager::deactivateJoystick()
{
   if ( isEnabled() && isJoystickActive() )
   {
      mJoystickActive = false;
      JoystickInputDevice* dptr;
      for ( iterator ptr = begin(); ptr != end(); ptr++ )
      {
         dptr = dynamic_cast<JoystickInputDevice*>( *ptr );
         if ( dptr && dptr->getDeviceType() == JoystickDeviceType)
            dptr->deactivate();
      }
#ifdef LOG_INPUT
      Input::log( "Joystick deactivated.\n" );
#endif
   }
}

//------------------------------------------------------------------------------
const char* UInputManager::getJoystickAxesString( U32 deviceID )
{
//    DInputDevice* dptr;
//    for ( iterator ptr = begin(); ptr != end(); ptr++ )
//    {
//       dptr = dynamic_cast<DInputDevice*>( *ptr );
//       if ( dptr && ( dptr->getDeviceType() == JoystickDeviceType ) && ( dptr->getDeviceID() == deviceID ) )
//          return( dptr->getJoystickAxesString() );
//    }

   return( "" );
}

//==============================================================================
// JoystickInputDevice
//==============================================================================
JoystickInputDevice::JoystickInputDevice(U8 deviceID)
{
   mActive = false;
   mStick = NULL;
   mDeviceID = deviceID;
   dSprintf(mName, 29, "joystick%d", mDeviceID);
}

//------------------------------------------------------------------------------
JoystickInputDevice::~JoystickInputDevice()
{
   if (isActive())
      deactivate();
}

//------------------------------------------------------------------------------
bool JoystickInputDevice::activate()
{
   if (isActive())
      return true;

   mStick = SDL_JoystickOpen(mDeviceID);
   if (mStick == NULL)
   {
      Con::printf("Unable to activate %s: %s", getDeviceName(), SDL_GetError());
      return false;
   }

   mActive = true;
   return true;
}

//------------------------------------------------------------------------------
bool JoystickInputDevice::deactivate()
{
   if (!isActive())
      return true;

   if (mStick != NULL)
   {
      SDL_JoystickClose(mStick);
      mStick = NULL;
   }

   mActive = false;
   return true;
}

//------------------------------------------------------------------------------
const char* JoystickInputDevice::getJoystickAxesString()
{
   return "";
}

//------------------------------------------------------------------------------
const char* JoystickInputDevice::getName()
{
   return SDL_JoystickName(mDeviceID);
}


//------------------------------------------------------------------------------
bool JoystickInputDevice::process()
{
   if (!isActive())
      return false;
   return true;
}

//==============================================================================
// Console Functions
//==============================================================================
ConsoleFunction( activateKeyboard, bool, 1, 1, "activateKeyboard()" )
{
   UInputManager* mgr = dynamic_cast<UInputManager*>( Input::getManager() );
   if ( mgr )
      return( mgr->activateKeyboard() );

   return( false );
}

//------------------------------------------------------------------------------
ConsoleFunction( deactivateKeyboard, void, 1, 1, "deactivateKeyboard()" )
{
   UInputManager* mgr = dynamic_cast<UInputManager*>( Input::getManager() );
   if ( mgr )
      mgr->deactivateKeyboard();
}

//------------------------------------------------------------------------------
ConsoleFunction( enableMouse, bool, 1, 1, "enableMouse()" )
{
   UInputManager* mgr = dynamic_cast<UInputManager*>( Input::getManager() );
   if ( mgr )
      return( mgr->enableMouse() );

   return ( false );
}

//------------------------------------------------------------------------------
ConsoleFunction( disableMouse, void, 1, 1, "disableMouse()" )
{
   UInputManager* mgr = dynamic_cast<UInputManager*>( Input::getManager() );
   if ( mgr )
      mgr->disableMouse();
}

//------------------------------------------------------------------------------
ConsoleFunction( enableJoystick, bool, 1, 1, "enableJoystick()" )
{
   UInputManager* mgr = dynamic_cast<UInputManager*>( Input::getManager() );
   if ( mgr )
      return( mgr->enableJoystick() );

   return ( false );
}

//------------------------------------------------------------------------------
ConsoleFunction( disableJoystick, void, 1, 1, "disableJoystick()" )
{
   UInputManager* mgr = dynamic_cast<UInputManager*>( Input::getManager() );
   if ( mgr )
      mgr->disableJoystick();
}

//------------------------------------------------------------------------------
ConsoleFunction( echoInputState, void, 1, 1, "echoInputState()" )
{
   UInputManager* mgr = dynamic_cast<UInputManager*>( Input::getManager() );
   if ( mgr && mgr->isEnabled() )
   {
      Con::printf( "Input is enabled %s.", 
         mgr->isActive() ? "and active" : "but inactive" );
      Con::printf( "- Keyboard is %sabled and %sactive.", 
         mgr->isKeyboardEnabled() ? "en" : "dis",
         mgr->isKeyboardActive() ? "" : "in" );
      Con::printf( "- Mouse is %sabled and %sactive.", 
         mgr->isMouseEnabled() ? "en" : "dis",
         mgr->isMouseActive() ? "" : "in" );
      Con::printf( "- Joystick is %sabled and %sactive.", 
         mgr->isJoystickEnabled() ? "en" : "dis",
         mgr->isJoystickActive() ? "" : "in" );
   }
   else
      Con::printf( "Input is not enabled." );
}
