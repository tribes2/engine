//-----------------------------------------------------------------------------
// Torque Game Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

// $Id: x86UNIXInputManager.h,v 1.1 2002/01/26 23:58:06 jmquigs Exp $

#ifndef _X86UNIXINPUTMANAGER_H_
#define _X86UNIXINPUTMANAGER_H_

#include "platform/platformInput.h"
#include "platformX86UNIX/platformX86UNIX.h"

#include <SDL/SDL_events.h>

// JMQTODO: make these not be defines
#define NUM_KEYS ( KEY_OEM_102 + 1 )
#define KEY_FIRST KEY_ESCAPE

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

typedef struct _SDL_Joystick;

//------------------------------------------------------------------------------
class JoystickInputDevice : public InputDevice
{   
  public:
    JoystickInputDevice(U8 deviceID);
    ~JoystickInputDevice();
    
    bool activate();
    bool deactivate();
    bool isActive() { return( mActive ); }
    
    U8 getDeviceType() { return( JoystickDeviceType ); }
    U8 getDeviceID() { return( mDeviceID ); }
    const char* getName();
    const char* getJoystickAxesString();

    bool process();
    
  private:
    bool mActive;
    U8 mDeviceID;
    SDL_Joystick* mStick;
};

//------------------------------------------------------------------------------
class UInputManager : public InputManager
{
   public:
      UInputManager();

      void init();
      bool enable();
      void disable();
      void activate();
      void deactivate();
      void setWindowLocked(bool locked);
      bool isActive()               { return( mActive ); }

      void onDeleteNotify( SimObject* object );
      bool onAdd();
      void onRemove();

      void process();

      bool enableKeyboard();
      void disableKeyboard();
      bool isKeyboardEnabled()      { return( mKeyboardEnabled ); }
      bool activateKeyboard();
      void deactivateKeyboard();
      bool isKeyboardActive()       { return( mKeyboardActive ); }

      bool enableMouse();
      void disableMouse();
      bool isMouseEnabled()         { return( mMouseEnabled ); }
      bool activateMouse();
      void deactivateMouse();
      bool isMouseActive()          { return( mMouseActive ); }          

      bool enableJoystick();
      void disableJoystick();
      bool isJoystickEnabled()      { return( mJoystickEnabled ); }
      bool activateJoystick();
      void deactivateJoystick();
      bool isJoystickActive()       { return( mJoystickActive ); }          

      const char* getJoystickAxesString( U32 deviceID );
   private:
      typedef SimGroup Parent;

      bool mKeyboardEnabled;
      bool mMouseEnabled;
      bool mJoystickEnabled;

      bool mKeyboardActive;
      bool mMouseActive;
      bool mJoystickActive;

      bool mActive;

      // Device state variables
      S32 mModifierKeys;
      bool mKeyboardState[256];
      bool mMouseButtonState[3];

      // last mousex and y are maintained when window is unlocked
      S32 mLastMouseX;
      S32 mLastMouseY;

      void initJoystick();

      void resetKeyboardState();
      void resetMouseState();
      void resetInputState();

      void lockInput();
      void unlockInput();

      void mouseButtonEvent(const SDL_Event& event);
      void mouseMotionEvent(const SDL_Event& event);
      void keyEvent(const SDL_Event& event);
      bool processKeyEvent(InputEvent &event);
};

#endif  // _H_X86UNIXINPUTMANAGER_
