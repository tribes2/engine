//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "PlatformWin32/platformGL.h"
#include "PlatformPPC/platformPPC.h"
#include "PlatformPPC/ppcOGLVideo.h"
#include "console/console.h"
#include <math.h>

//extern Win32PlatState winState;
extern PPCPlatState ppcState;


//------------------------------------------------------------------------------
OpenGLDevice::OpenGLDevice()
{
   initDevice();
}


OpenGLDevice::~OpenGLDevice()
{
}


//------------------------------------------------------------------------------
void OpenGLDevice::initDevice()
{
   #pragma message("todo: enumerate available display modes");
   // Set the device name:
   mDeviceName = "OpenGL";

   // Set some initial conditions:
   mResolutionList.clear();

   Resolution newRes( 640, 480, 32 );
   mResolutionList.push_back( newRes );
   return;
   
/*   
   // Enumerate all available resolutions:
   DEVMODE devMode;
   U32 modeNum = 0;
   U32 stillGoing = true;
   while ( stillGoing )
   {
      dMemset( &devMode, 0, sizeof( devMode ) );
      devMode.dmSize = sizeof( devMode );

      stillGoing = EnumDisplaySettings( NULL, modeNum++, &devMode );
      if ( devMode.dmPelsWidth >= 640 && devMode.dmPelsHeight >= 480
        && ( devMode.dmBitsPerPel == 16 || devMode.dmBitsPerPel == 32 ) )
      {
         // Only add this resolution if it is not already in the list:
         bool alreadyInList = false;
         for ( U32 i = 0; i < mResolutionList.size(); i++ )
         {
            if ( devMode.dmPelsWidth == mResolutionList[i].w
              && devMode.dmPelsHeight == mResolutionList[i].h
              && devMode.dmBitsPerPel == mResolutionList[i].bpp )
            {
               alreadyInList = true;
               break;
            }
         }

         if ( !alreadyInList )
         {
            Resolution newRes( devMode.dmPelsWidth, devMode.dmPelsHeight, devMode.dmBitsPerPel );
            mResolutionList.push_back( newRes );
         }
      }
   }
*/
}


//------------------------------------------------------------------------------
bool OpenGLDevice::activate()
{
   // If the rendering context exists, delete it:
   if (ppcState.ctx)
   {
      Con::printf( "Making the rendering context not current..." );
      aglSetCurrentContext(NULL);
      aglSetDrawable(ppcState.ctx, NULL);
      Con::printf( "Deleting the rendering context..." );
      aglDestroyContext(ppcState.ctx);
      ppcState.ctx = NULL;
   }

   //if ( winState.hGLRC )
   //{
   //   qwglMakeCurrent( NULL, NULL );
   //   qwglDeleteContext( winState.hGLRC );
   //   winState.hGLRC = NULL;
   //}

   // If OpenGL library already loaded, shut it down:
   //if ( winState.hinstOpenGL )
   //   QGL_Shutdown();

   //if ( 0 /* TODO -- figure out when we will need to get a new window */ )
   //{
   //   // Release the device context and kill the window:
   //   ReleaseDC( winState.appWindow, winState.appDC );
   //   winState.appDC = NULL;
   //   DestroyWindow( winState.appWindow );
   //}

   // If the app window does not exist, create it:
/*
   if ( ppcState.appWindow == NULL )
   {
      // Create the new window:
      ppcState.appWindow = CreateOpenGLWindow( smIsFullScreen );
      if ( ppcState.appWindow == NULL )
      {
         Platform::AlertOK( "OpenGLDevice::activate", "Failed to create a window!" );
         return false;
      }

      setResolution( smCurrentRes, true );
      
      // Set the new window to the foreground:
      ShowWindow( ppcState.appWindow );
      
      //ShowWindow( winState.appWindow, SW_SHOW );
      //SetForegroundWindow( winState.appWindow );
      //SetFocus( winState.appWindow );

   }
*/
   GLint attrib[] = { AGL_RGBA, AGL_DOUBLEBUFFER, AGL_DEPTH_SIZE, 16, AGL_NONE };  
	
   // Never unload a code module
   aglConfigure(AGL_RETAIN_RENDERERS, GL_TRUE);

   ppcState.fmt = aglChoosePixelFormat(NULL, 0, attrib);
   if(ppcState.fmt == NULL) 
   {
      Platform::AlertOK( "OpenGLDevice::activate", "Failed to set the pixel format of the device context!" );
      return false;
   }
   // Con::printf( ??? );  // Spit out pixel format details?

   //QGL_Init( "opengl32", "glu32" );

   //PIXELFORMATDESCRIPTOR pfd;
   //CreatePixelFormat( &pfd, smCurrentRes.bpp, 24, 8, false );

   //S32 pixelFormat = ChooseBestPixelFormat( winState.appDC, &pfd );
   //qwglDescribePixelFormat( winState.appDC, pixelFormat, sizeof( pfd ), &pfd );
   //if ( !SetPixelFormat( winState.appDC, pixelFormat, &pfd ) )
   //{
   //   Platform::AlertOK( "OpenGLDevice::activate", "Failed to set the pixel format of the device context!" );
   //   return false;
   //}
   
   // Create an AGL context
   Con::printf( "Creating a new rendering context..." );
   ppcState.ctx = aglCreateContext(ppcState.fmt, NULL);
   if(ppcState.ctx == NULL)
   {
      Platform::AlertOK( "OpenGLDevice::activate", "Failed to create a GL rendering context!" );
      return false;
   }

   //winState.hGLRC = qwglCreateContext( winState.appDC );
   //if ( winState.hGLRC == NULL )
   //{
   //   Platform::AlertOK( "OpenGLDevice::activate", "Failed to create a GL rendering context!" );
   //   return false;
   //}

   // Attach the context to the window
   Con::printf( "Attaching the rendering context to the window..." );
   if( !aglSetDrawable(ppcState.ctx, (CGrafPtr) ppcState.appWindow) )
   {
      Platform::AlertOK( "OpenGLDevice::activate", "Failed to make the rendering context current!" );
      return false;
   }
   // Make this context current
   aglSetCurrentContext(ppcState.ctx);

   //if ( !qwglMakeCurrent( winState.appDC, winState.hGLRC ) )
   //{
   //   Platform::AlertOK( "OpenGLDevice::activate", "Failed to make the rendering context current!" );
   //   return false;
   //}

   // Output some driver info to the console?

   QGL_EXT_Init();

   return true;
}


//------------------------------------------------------------------------------
void OpenGLDevice::shutdown()
{
   /*if (ppcState.ctx)
   {
      aglSetCurrentContext(NULL);
      aglSetDrawable(ppcState.ctx, NULL);
      aglDestroyContext(ppcState.ctx);
      ppcState.ctx = NULL;
   }*/

//   if ( smIsFullScreen )
//      ChangeDisplaySettings( NULL, 0 );
}


//------------------------------------------------------------------------------
bool OpenGLDevice::setResolution( Resolution &res, bool forceIt )
{
   if ( !smIsFullScreen && ( res.w >= ppcState.desktopWidth || res.h >= ppcState.desktopHeight ) )
   {
      Con::printf( "OpenGLDevice::setResolution -- can't switch to resolution larger than desktop in windowed mode!" );
      return false;
   }
   
   if ( res.w < 640 || res.h < 480 )
   {
      Con::printf( "OpenGLDevice::setResolution -- can't go smaller than 640x480!" );
      return false;
   }

   // If full-screen, make the new resolution match one of the ones in the list:
   if ( smIsFullScreen )
   { 
      U32 resIndex = 0;
      U32 bestScore = 0, thisScore = 0;
      for ( int i = 0; i < mResolutionList.size(); i++ )
      {
         if ( res == mResolutionList[i] )
         {
            resIndex = i;
            break;
         }
         else
         {
            thisScore = (int) res.h - (int) mResolutionList[i].h;
            if (thisScore < 0) thisScore = -thisScore;
            thisScore = (int) res.w - (int) mResolutionList[i].w + thisScore; 
            if (thisScore < 0) thisScore = -thisScore;
            //thisScore = abs( (int) res.w - (int) mResolutionList[i].w ) + abs( (int) res.h - (int) mResolutionList[i].h );
            if ( ( !bestScore || thisScore < bestScore ) && res.bpp == mResolutionList[i].bpp )
            {
               bestScore = thisScore;
               resIndex = i;
            }
         }
      }

      res = mResolutionList[resIndex];
   }

   // Return if already at this resolution:
   if ( !forceIt && res == smCurrentRes )
      return true;

   Con::printf( "Changing resolution to %dx%dx%d", res.w, res.h, res.bpp );
   U32 test;
   if ( smIsFullScreen )
   {
      // aglSetFullScreen(    neds to know monitor freq...
#if 0   
      // Change the display settings:
      DEVMODE devMode;
      dMemset( &devMode, 0, sizeof( devMode ) );
      devMode.dmSize = sizeof( devMode );
      devMode.dmPelsWidth = res.w;
      devMode.dmPelsHeight = res.h;
      // TODO: whatever it takes to make bit depth switching work.
      //devMode.dmBitsPerPel = res.bpp
      devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT/* | DM_BITSPERPEL*/;

      if ( ChangeDisplaySettings( &devMode, CDS_FULLSCREEN ) != DISP_CHANGE_SUCCESSFUL )
      {
         ChangeDisplaySettings( NULL, 0 );
         Con::printf( "OpenGLDevice::setResolution -- ChangeDisplaySettings to %dx%dx%d failed.", res.w, res.h, res.bpp );
         return;
      }

      test = SetWindowPos( ppcState.appWindow, 0, 0, 0, res.w, res.h, SWP_NOZORDER );
      if ( !test )
      {
         Con::printf( "OpenGLDevice::setResolution -- SetWindowPos moving to ( 0, 0 ) and sizing to %dx%d failed.", res.w, res.h );
         return;
      }
#endif
   }
   else
   {
      //SizeWindow(ppcState.appWindow, res.w, res.h, true);     // change the window size
      #pragma message("todo: center window on resize");
      
      U32 xPos = 0;
      U32 yPos = 0;
      if (!aglSetDrawable(ppcState.ctx, (CGrafPtr) ppcState.appWindow))
      {
         Con::printf( "OpenGLDevice::setResolution: SizeWindow moving to ( %d, %d ) and sizing to %dx%d failed.", xPos, yPos, res.w, res.h );
         return false;
      }
      if (!aglUpdateContext(ppcState.ctx))   // tell OpenGL the window changed
      {
         Con::printf("OpenGLDevice::setResolution:  Unable to resize render buffer");
         return false;
      }
      
//      // Adjust the window rect to compensate for the window style:
//      
//      RECT windowRect;
//      windowRect.left = windowRect.top = 0;
//      windowRect.right = res.w;
//      windowRect.bottom = res.h;
//      
//      AdjustWindowRect( &windowRect, GetWindowLong( ppcState.appWindow, GWL_STYLE ), false );
//      U32 adjWidth = windowRect.right - windowRect.left;
//      U32 adjHeight = windowRect.bottom - windowRect.top;
//
//      // Center the window again:
//      U32 xPos = ( ppcState.desktopWidth - adjWidth ) / 2;
//      U32 yPos = ( ppcState.desktopHeight - adjHeight ) / 2;
//      test = SetWindowPos( ppcState.appWindow, 0, xPos, yPos, adjWidth, adjHeight, SWP_NOZORDER );
//      if ( !test )
//      {
//         Con::printf( "OpenGLDevice::setResolution -- SetWindowPos moving to ( %d, %d ) and sizing to %dx%d failed.", xPos, yPos, res.w, res.h );
//         return;
//      }
   }

   smCurrentRes = res;
   Platform::setWindowSize( res.w, res.h );
   char tempBuf[15];
   dSprintf( tempBuf, sizeof( tempBuf ), "%dx%dx%d", smCurrentRes.w, smCurrentRes.h, smCurrentRes.bpp );
   Con::setVariable( "$pref::Video::resolution", tempBuf );

   return true;
}


//------------------------------------------------------------------------------
bool OpenGLDevice::toggleFullScreen()
{
   // Change the window style:
//   U32 style = WS_VISIBLE;
//   if ( smIsFullScreen )
//      style |= ( WS_OVERLAPPED | WS_BORDER | WS_CAPTION );
//   else
//      style |= ( WS_POPUP | WS_MAXIMIZE );
//   SetWindowLong( ppcState.appWindow, GWL_STYLE, style );
//
//   U32 test = SetWindowPos( ppcState.appWindow, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED );
//   if ( !test )
//   {
//      Con::printf( "OpenGLDevice::toggleFullScreen -- SetWindowPos changing window style failed." );
//      return false;
//   }

   smIsFullScreen = !smIsFullScreen;
   Con::setVariable( "$pref::Video::fullScreen", ( smIsFullScreen ? "true" : "false" ) );

   if ( !smIsFullScreen )
   {
      // Change to windowed mode:
//      ChangeDisplaySettings( NULL, 0 );

      // Make sure the window size is not too big for the desktop:
      if ( smCurrentRes.w >= ppcState.desktopWidth || smCurrentRes.h >= ppcState.desktopHeight )
      {
         S32 resIndex;
         for ( resIndex = 0; resIndex < mResolutionList.size(); resIndex++ )
         {
            if ( smCurrentRes == mResolutionList[resIndex] )
               break;
         }

         if ( resIndex < mResolutionList.size() )
         {
            // Walk back in the list until we get to a res smaller than the desktop:
            while ( resIndex >= 0 && ( mResolutionList[resIndex].w >= ppcState.desktopWidth || mResolutionList[resIndex].h >= ppcState.desktopHeight ) )
               resIndex--;
            
            if ( resIndex >= 0 )
            {
               setResolution( mResolutionList[resIndex], true );
               return true;
            }
         }

         // Something is really screwy if you ever get to here, but let it fall through anyways.
         Con::printf( "OpenGLDevice::toggleFullScreen -- a suitable resolution could not be found!" );
      }
   }

   // Force the resolution change:
   if (setResolution( smCurrentRes, true ))
      return true;
   return false;
}


//------------------------------------------------------------------------------
void OpenGLDevice::swapBuffers()
{
   aglSwapBuffers(aglGetCurrentContext());
}


//------------------------------------------------------------------------------
DisplayDevice* OpenGLDevice::create()
{
   return new OpenGLDevice();
}   
