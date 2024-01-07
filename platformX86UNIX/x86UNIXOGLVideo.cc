//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "platformX86UNIX/platformX86UNIX.h"
//#include "console/ast.h"
#include "console/console.h"
//#include "console/consoleInternal.h"
//#include "core/fileStream.h"
//#include "math/mPoint.h"

#include "platformX86UNIX/platformGL.h"
#include "platform/event.h"
#include "platform/gameInterface.h"
#include "platformX86UNIX/x86UNIXOGLVideo.h"
#include "platform/platformAudio.h"

#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>

/* our windowing/gl vars */
GLXContext  ctx;
Display     *display;
int         screen_num;
Window      win;
Screen      *screen_ptr;

//------------------------------------------------------------------------------

bool OpenGLDevice::smCanSwitchBitDepth = true;

//------------------------------------------------------------------------------
OpenGLDevice::OpenGLDevice()
{
   initDevice();
}


//------------------------------------------------------------------------------
void OpenGLDevice::initDevice()
{
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
           && ( devMode.dmBitsPerPel == 16 || devMode.dmBitsPerPel == 32 ) &&
           ( smCanSwitchBitDepth || devMode.dmBitsPerPel == winState.desktopBitsPixel ) )
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
bool OpenGLDevice::activate( U32 width, U32 height, U32 bpp, bool fullScreen )
{
   Con::printf( "Activating the OpenGL display device..." );

   bool needResurrect = false;

   // If the rendering context exists, delete it:
   if ( ctx )
   {
      Con::printf( "Killing the texture manager..." );
      Game->textureKill();
      needResurrect = true;

      Con::printf( "Making the rendering context not current..." );
      if ( !glXMakeCurrent(display, None, NULL) )
      {
         AssertFatal( false, "OpenGLDevice::activate\nglXMakeCurrent(display, None, NULL) failed!" );
         return false;
      }

      Con::printf( "Deleting the rendering context ..." );
      glXDestroyContext(display, ctx);
      ctx = NULL;
   }

   // GLX attributes
   static int attr[] = { GLX_RGBA, GLX_DOUBLEBUFFER, GLX_RED_SIZE, 4, GLX_GREEN_SIZE, 4, GLX_BLUE_SIZE, 4, GLX_DEPTH_SIZE, 16, None };

   // create a GLX instance
   Con::printf(" Creating a new rendering context..." );
   XVisualInfo *vi = glXChooseVisual(display, screen_num, attr);
   ctx = glXCreateContext(display, vi, 0, GL_TRUE);

   // attach the GLX context to the window
   glXMakeCurrent(display, win, ctx);

   // Output some driver info to the console:
   const char* vendorString   = (const char*) glGetString( GL_VENDOR );
   const char* rendererString = (const char*) glGetString( GL_RENDERER );
   const char* versionString  = (const char*) glGetString( GL_VERSION );
   Con::printf( "OpenGL driver information:" );
   if ( vendorString )
      Con::printf( "  Vendor: %s", vendorString );
   if ( rendererString )
      Con::printf( "  Renderer: %s", rendererString );
   if ( versionString )
      Con::printf( "  Version: %s", versionString );

   if ( needResurrect )
   {
      // Reload the textures:
      Con::printf( "Resurrecting the texture manager..." );
      Game->textureResurrect();
   }

   QGL_EXT_Init();

   return true;
}


//------------------------------------------------------------------------------
void OpenGLDevice::shutdown()
{
   Con::printf( "Shutting down the OpenGL display device..." );

   if ( ctx )
   {
      Con::printf( "Making the GL rendering context not current..." );
      if ( !glXMakeCurrent(display, None, NULL) )
      {
         AssertFatal( false, "OpenGLDevice::activate\nglXMakeCurrent(display, None, NULL) failed!" );
         return;
      }

      Con::printf( "Deleting the rendering context ..." );
      glXDestroyContext(display, ctx);
      ctx = NULL;
   }

//   if ( smIsFullScreen )
//   {
//      Con::printf( "Restoring the desktop display settings (%dx%dx%d)...", winState.desktopWidth, winState.desktopHeight, winState.desktopBitsPixel );
//      ChangeDisplaySettings( NULL, 0 );
//   }
}


//------------------------------------------------------------------------------
// This is the real workhorse function of the DisplayDevice...
//
bool OpenGLDevice::setScreenMode( U32 width, U32 height, U32 bpp, bool fullScreen, bool forceIt, bool repaint )
{
#if 0
   HWND curtain = NULL;
   char errorMessage[256];
   Resolution newRes( width, height, bpp );
   bool newFullScreen = fullScreen;
   bool safeModeOn = Con::getBoolVariable( "$pref::Video::safeModeOn" );

   if ( !newFullScreen && mFullScreenOnly )
   {
      Con::warnf( ConsoleLogEntry::General, "OpenGLDevice::setScreenMode - device or desktop color depth does not allow windowed mode!" );
      newFullScreen = true;
   }

   if ( !newFullScreen && ( newRes.w >= winState.desktopWidth || newRes.h >= winState.desktopHeight ) )
   {
      Con::warnf( ConsoleLogEntry::General, "OpenGLDevice::setScreenMode -- can't switch to resolution larger than desktop in windowed mode!" );
      // Find the largest standard resolution that will fit on the desktop:
      U32 resIndex;
      for ( resIndex = mResolutionList.size() - 1; resIndex > 0; resIndex-- )
      {
         if ( mResolutionList[resIndex].w < winState.desktopWidth 
           && mResolutionList[resIndex].h < winState.desktopHeight )
            break;  
      }
      newRes = mResolutionList[resIndex];
   }

   if ( newRes.w < 640 || newRes.h < 480 )
   {
      Con::warnf( ConsoleLogEntry::General, "OpenGLDevice::setScreenMode -- can't go smaller than 640x480!" );
      return false;
   }

   if ( newFullScreen )
   {
      if (newRes.bpp != 16 && mFullScreenOnly)
         newRes.bpp = 16;
    
      // Match the new resolution to one in the list:
      U32 resIndex = 0;
      U32 bestScore = 0, thisScore = 0;
      for ( int i = 0; i < mResolutionList.size(); i++ )
      {
         if ( newRes == mResolutionList[i] )
         {
            resIndex = i;
            break;
         }
         else
         {
            thisScore = abs( S32( newRes.w ) - S32( mResolutionList[i].w ) ) 
                      + abs( S32( newRes.h ) - S32( mResolutionList[i].h ) ) 
                      + ( newRes.bpp == mResolutionList[i].bpp ? 0 : 1 );

            if ( !bestScore || ( thisScore < bestScore ) )
            {
               bestScore = thisScore;
               resIndex = i;
            }
         }
      }

      newRes = mResolutionList[resIndex];
   }
   else
   {
      // Basically ignore the bit depth parameter:
      newRes.bpp = winState.desktopBitsPixel;
   }

   // Return if already at this resolution:
   if ( !forceIt && newRes == smCurrentRes && newFullScreen == smIsFullScreen )
      return true;

   Con::printf( "Setting screen mode to %dx%dx%d (%s)...", newRes.w, newRes.h, newRes.bpp, ( newFullScreen ? "fs" : "w" ) );

   bool needResurrect = false;

   if ( ( newRes.bpp != smCurrentRes.bpp ) || ( safeModeOn && ( ( smIsFullScreen != newFullScreen ) || newFullScreen ) ) )
   {
      // Delete the rendering context:
      if ( winState.hGLRC )
      {
         if (!Video::smNeedResurrect)
			{
         	Con::printf( "Killing the texture manager..." );
         	Game->textureKill();
         	needResurrect = true;
			}

         Con::printf( "Making the rendering context not current..." );
         if ( !qwglMakeCurrent( NULL, NULL ) )
         {
            AssertFatal( false, "OpenGLDevice::setScreenMode\nqwglMakeCurrent( NULL, NULL ) failed!" );
            return false;
         }

         Con::printf( "Deleting the rendering context..." );
         if ( Con::getBoolVariable("$pref::Video::deleteContext",true) &&
         	  !qwglDeleteContext( winState.hGLRC ) )
         {
            AssertFatal( false, "OpenGLDevice::setScreenMode\nqwglDeleteContext failed!" );
            return false;
         }
         winState.hGLRC = NULL;
      }

      // Release the device context:
      if ( winState.appDC )
      {
         Con::printf( "Releasing the device context..." );
         ReleaseDC( winState.appWindow, winState.appDC );
         winState.appDC = NULL;
      }

      // Destroy the window:
      if ( winState.appWindow )
      {
         Con::printf( "Destroying the window..." );
         DestroyWindow( winState.appWindow );
         winState.appWindow = NULL;
      }
   }
   else if ( smIsFullScreen != newFullScreen )
   {
      // Change the window style:
      Con::printf( "Changing the window style..." );
      S32 windowStyle = WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
      if ( newFullScreen )
         windowStyle |= ( WS_POPUP | WS_MAXIMIZE );
      else
         windowStyle |= ( WS_OVERLAPPED | WS_CAPTION );

      if ( !SetWindowLong( winState.appWindow, GWL_STYLE, windowStyle ) )
         Con::errorf( "SetWindowLong failed to change the window style!" );
   }

   U32 test;
   if ( newFullScreen )
   {
      // Change the display settings:
      DEVMODE devMode;
      dMemset( &devMode, 0, sizeof( devMode ) );
      devMode.dmSize       = sizeof( devMode );
      devMode.dmPelsWidth  = newRes.w;
      devMode.dmPelsHeight = newRes.h;
      devMode.dmBitsPerPel = newRes.bpp;
      devMode.dmFields     = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;

      Con::printf( "Changing the display settings to %dx%dx%d...", newRes.w, newRes.h, newRes.bpp );
      curtain = CreateCurtain( newRes.w, newRes.h );
      test = ChangeDisplaySettings( &devMode, CDS_FULLSCREEN );
      if ( test != DISP_CHANGE_SUCCESSFUL )
      {
         smIsFullScreen = false;
         Con::setBoolVariable( "$pref::Video::fullScreen", false );
         ChangeDisplaySettings( NULL, 0 );
         Con::errorf( ConsoleLogEntry::General, "OpenGLDevice::setScreenMode - ChangeDisplaySettings failed." );
         switch( test )
         {
            case DISP_CHANGE_RESTART:
               Platform::AlertOK( "Display Change Failed", "You must restart your machine to get the specified mode." );
               break;

            case DISP_CHANGE_BADMODE:
               Platform::AlertOK( "Display Change Failed", "The specified mode is not supported by this device." );
               break;

            default: 
               Platform::AlertOK( "Display Change Failed", "Hardware failed to switch to the specified mode." );
               break;
         };
         DestroyWindow( curtain );
         return false;
      }
      else
         smIsFullScreen = true;
   }
   else if ( smIsFullScreen )
   {
      Con::printf( "Changing to the desktop display settings (%dx%dx%d)...", winState.desktopWidth, winState.desktopHeight, winState.desktopBitsPixel );
      ChangeDisplaySettings( NULL, 0 );
      smIsFullScreen = false;
   }
   Con::setBoolVariable( "$pref::Video::fullScreen", smIsFullScreen );

   bool newWindow = false;
   if ( !winState.appWindow )
   {
      Con::printf( "Creating a new %swindow...", ( fullScreen ? "full-screen " : "" ) );
      winState.appWindow = CreateOpenGLWindow( newRes.w, newRes.h, newFullScreen );
      if ( !winState.appWindow )
      {
         AssertFatal( false, "OpenGLDevice::setScreenMode\nFailed to create a new window!" );
         return false;
      }
      newWindow = true;
   }

   // Move the window:
   if ( !newFullScreen )
   {
      // Adjust the window rect to compensate for the window style:
      RECT windowRect;
      windowRect.left = windowRect.top = 0;
      windowRect.right = newRes.w;
      windowRect.bottom = newRes.h;
      
      AdjustWindowRect( &windowRect, GetWindowLong( winState.appWindow, GWL_STYLE ), false );
      U32 adjWidth = windowRect.right - windowRect.left;
      U32 adjHeight = windowRect.bottom - windowRect.top;

      // Center the window on the desktop:
      U32 xPos = ( winState.desktopWidth - adjWidth ) / 2;
      U32 yPos = ( winState.desktopHeight - adjHeight ) / 2;
      test = SetWindowPos( winState.appWindow, 0, xPos, yPos, adjWidth, adjHeight, SWP_NOZORDER );
      if ( !test )
      {
         dSprintf( errorMessage, 255, "OpenGLDevice::setScreenMode\nSetWindowPos failed trying to move the window to (%d,%d) and size it to %dx%d.", xPos, yPos, newRes.w, newRes.h ); 
         AssertFatal( false, errorMessage );
         return false;
      }
   }
   else if ( !newWindow )
   {
      // Move and size the window to take up the whole screen:
      if ( !SetWindowPos( winState.appWindow, 0, 0, 0, newRes.w, newRes.h, SWP_NOZORDER ) )
      {
         dSprintf( errorMessage, 255, "OpenGLDevice::setScreenMode\nSetWindowPos failed to move the window to (0,0) and size it to %dx%d.", newRes.w, newRes.h );
         AssertFatal( false, errorMessage );
         return false;
      }
   }

   bool newDeviceContext = false;
   if ( !winState.appDC )
   {
      // Get a new device context:
      Con::printf( "Acquiring a new device context..." );
      winState.appDC = GetDC( winState.appWindow );
      if ( !winState.appDC )
      {
         AssertFatal( false, "OpenGLDevice::setScreenMode\nGetDC failed to get a valid device context!" );
         return false;
      }
      newDeviceContext = true;
   }

   if ( newWindow )
   { 
      // Set the pixel format of the new window:
      PIXELFORMATDESCRIPTOR pfd;
      CreatePixelFormat( &pfd, newRes.bpp, 24, 8, false );
      S32 chosenFormat = ChooseBestPixelFormat( winState.appDC, &pfd );
      if ( !chosenFormat )
      {
         AssertFatal( false, "OpenGLDevice::setScreenMode\nNo valid pixel formats found!" );
         return false;
      }
      qwglDescribePixelFormat( winState.appDC, chosenFormat, sizeof( pfd ), &pfd );
      if ( !SetPixelFormat( winState.appDC, chosenFormat, &pfd ) )
      {
         AssertFatal( false, "OpenGLDevice::setScreenMode\nFailed to set the pixel format!" );
         return false;
      }
      Con::printf( "Pixel format set:" );
      Con::printf( "  %d color bits, %d depth bits, %d stencil bits", pfd.cColorBits, pfd.cDepthBits, pfd.cStencilBits );
   }

   if ( !winState.hGLRC )
   {
      // Create a new rendering context:
      Con::printf( "Creating a new rendering context..." );
      winState.hGLRC = qwglCreateContext( winState.appDC );
      if ( !winState.hGLRC )
      {
         AssertFatal( false, "OpenGLDevice::setScreenMode\nqwglCreateContext failed to create an OpenGL rendering context!" );
         return false;
      }   

      // Make the new rendering context current:
      Con::printf( "Making the new rendering context current..." );
      if ( !qwglMakeCurrent( winState.appDC, winState.hGLRC ) )
      {
			AssertFatal( false, "OpenGLDevice::setScreenMode\nqwglMakeCurrent failed to make the rendering context current!" );
			return false;
      }

      // Just for kicks.  Seems a relatively central place to put this...
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

      if ( needResurrect )
      {
         // Reload the textures:
         Con::printf( "Resurrecting the texture manager..." );
         Game->textureResurrect();
      }
   }

   // Just for kicks.  Seems a relatively central place to put this...
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

   if ( newDeviceContext && gGLState.suppSwapInterval )
      setVerticalSync( !Con::getBoolVariable( "$pref::Video::disableVerticalSync" ) );

   smCurrentRes = newRes;
   Platform::setWindowSize( newRes.w, newRes.h );
   char tempBuf[15];
   dSprintf( tempBuf, sizeof( tempBuf ), "%d %d %d", smCurrentRes.w, smCurrentRes.h, smCurrentRes.bpp );
   Con::setVariable( "$pref::Video::resolution", tempBuf );

   if ( curtain )
      DestroyWindow( curtain );

   // Doesn't hurt to do this even it isn't necessary:
   ShowWindow( winState.appWindow, SW_SHOW );
   SetForegroundWindow( winState.appWindow );
   SetFocus( winState.appWindow );

   if ( repaint )
      Con::evaluate( "resetCanvas();" );

   return true;
#endif
}


//------------------------------------------------------------------------------
void OpenGLDevice::swapBuffers()
{
   glXSwapBuffers(display, win);
}


//------------------------------------------------------------------------------
const char* OpenGLDevice::getDriverInfo()
{
#if 0
   // Output some driver info to the console:
   const char* vendorString   = (const char*) glGetString( GL_VENDOR );
   const char* rendererString = (const char*) glGetString( GL_RENDERER );
   const char* versionString  = (const char*) glGetString( GL_VERSION );
   const char* extensionsString = (const char*) glGetString( GL_EXTENSIONS );
   
   U32 bufferLen = ( vendorString ? dStrlen( vendorString ) : 0 )
                 + ( rendererString ? dStrlen( rendererString ) : 0 )
                 + ( versionString  ? dStrlen( versionString ) : 0 )
                 + ( extensionsString ? dStrlen( extensionsString ) : 0 )
                 + 4;

   char* returnString = Con::getReturnBuffer( bufferLen );
   dSprintf( returnString, bufferLen, "%s\t%s\t%s\t%s", 
         ( vendorString ? vendorString : "" ),
         ( rendererString ? rendererString : "" ),
         ( versionString ? versionString : "" ),
         ( extensionsString ? extensionsString : "" ) );
   
   return( returnString );   
#endif
}

//------------------------------------------------------------------------------
bool OpenGLDevice::getGammaCorrection(F32 &g)
{
#if 0
   U16 ramp[256*3];

   if (!GetDeviceGammaRamp(winState.appDC, ramp))
      return false;

   F32 csum = 0.0;
   U32 ccount = 0;

   for (U16 i = 0; i < 256; ++i)
   {
      if (i != 0 && ramp[i] != 0 && ramp[i] != 65535)
      {
         F64 b = (F64) i/256.0;
         F64 a = (F64) ramp[i]/65535.0;
         F32 c = (F32) (mLog(a)/mLog(b));
         
         csum += c;
         ++ccount;  
      }
   }
   g = csum/ccount;

   return true;         
#endif
}    

//------------------------------------------------------------------------------
bool OpenGLDevice::setGammaCorrection(F32 g)
{
#if 0
   U16 ramp[256*3];

   for (U16 i = 0; i < 256; ++i)
      ramp[i] = mPow((F32) i/256.0f, g) * 65535.0f;
   dMemcpy(&ramp[256],ramp,256*sizeof(U16));
   dMemcpy(&ramp[512],ramp,256*sizeof(U16));      

   return SetDeviceGammaRamp(winState.appDC, ramp);
#endif
}

//------------------------------------------------------------------------------
bool OpenGLDevice::setVerticalSync( bool on )
{
#if 0
   if ( !gGLState.suppSwapInterval )
      return( false );

   return( qwglSwapIntervalEXT( on ? 1 : 0 ) );
#endif
}

//------------------------------------------------------------------------------
DisplayDevice* OpenGLDevice::create()
{
   return new OpenGLDevice();
}
