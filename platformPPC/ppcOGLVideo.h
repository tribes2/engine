//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _PCCOGLVIDEO_H_
#define _PCCOGLVIDEO_H_

#ifndef _PLATFORMVIDEO_H_
#include "Platform/platformVideo.h"
#endif

class OpenGLDevice : public DisplayDevice
{
   public:
      OpenGLDevice();
      ~OpenGLDevice();

      void initDevice();
      bool activate();
      void shutdown();
      bool setResolution( Resolution &res, bool forceIt = false );
      bool toggleFullScreen();
      void swapBuffers();

      static DisplayDevice* create();
};

#endif // _H_WINOGLVIDEO
