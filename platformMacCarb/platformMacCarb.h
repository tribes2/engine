//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _PLATFORMMACCARB_H_
#define _PLATFORMMACCARB_H_

#if __APPLE__
#include <Carbon/Carbon.h>
#else
#include "platformMacCarb/macCarbHeaders.h"
#endif
#include <agl.h>

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

class MacCarbPlatState
{
public:
   GDHandle      hDisplay;
   WindowPtr     appWindow;
   AGLDrawable   drawable;
   
   char          appWindowTitle[256];
   bool          quit;
   
   AGLPixelFormat fmt;
   AGLContext     ctx;

   short         volRefNum;   // application volume/drive reference number
   long          dirID;      // application directory id
   char          absAppPath[2048]; // app path - make it big enough!

   S32           desktopBitsPixel;
   S32           desktopWidth;
   S32           desktopHeight;
   U32           currentTime;
      
   MacCarbPlatState();
};

extern MacCarbPlatState platState;

extern bool QGL_EXT_Init();


extern WindowPtr CreateOpenGLWindow( U32 width, U32 height, bool fullScreen );
extern WindowPtr CreateCurtain( U32 width, U32 height );

U32 GetMilliseconds();

U8* str2p(const char *str);
U8* str2p(const char *str, U8 *dst_p);

char* p2str(U8 *p);
char* p2str(U8 *p, char *dst_str);

U8 TranslateOSKeyCode(U8 vcode);

#endif //_PLATFORMMACCARB_H_
