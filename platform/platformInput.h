//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _PLATFORMINPUT_H_
#define _PLATFORMINPUT_H_

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif


//------------------------------------------------------------------------------
U8 TranslateOSKeyCode( U8 vcode );

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class InputDevice : public SimObject
{
   public:
      struct ObjInfo
      {
         U16   mType;
         U16   mInst;
         S32   mMin, mMax;
      };

   protected:
      char mName[30];

   public:
      const char* getDeviceName();
      virtual bool process() = NULL;
};


//------------------------------------------------------------------------------
inline const char* InputDevice::getDeviceName()
{
   return mName;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class InputManager : public SimGroup
{
   protected:
      bool  mEnabled;

   public:
      bool  isEnabled();

      virtual bool enable() = NULL;
      virtual void disable() = NULL;

      virtual void process() = NULL;
};


//------------------------------------------------------------------------------
inline bool InputManager::isEnabled()
{
   return mEnabled;
}

enum KEY_STATE
{
   STATE_LOWER,
   STATE_UPPER,
   STATE_GOOFY
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class Input
{
   protected:
      static InputManager* smManager;
      static bool smActive;

   public:
      static void init();
      static void destroy();

      static bool enable();
      static void disable();

      static void activate();
      static void deactivate();
      static void reactivate();
      
      static U16  getAscii( U16 keyCode, KEY_STATE keyState );
      static U16  getKeyCode( U16 asciiCode );

      static bool isEnabled();
      static bool isActive();

      static void process();

      static InputManager* getManager();

#ifdef LOG_INPUT
      static void log( const char* format, ... );
#endif
};

#endif // _H_PLATFORMINPUT_
