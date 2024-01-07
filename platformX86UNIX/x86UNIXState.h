//-----------------------------------------------------------------------------
// Torque Game Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//
// Initial revision: 12/13/01 Paul G. Allen, Random Logic Consulting
//-----------------------------------------------------------------------------

#include "math/mPoint.h"
#include "platformX86UNIX/platformGL.h"
#include "platformX86UNIX/x86UNIXGLX.h"

/******************************************************************************
*
*  Class:         x86UNIXPlatformState
*  Description:   Class used to store the state of things for X Windows based
*                 implementations.
*
*  Data Objects:
*                 Point2I  DesktopSize       hor. and vert. X desktop resolution
*                 Point2I  WindowSize        hor. and vert. X window resolution
*                 GLXContext  OpenGLContext  Current GLX context
*                 S32      Desktop_bpp       X desktop color depth
*                 Display  *display          Pointer to current X display
*                 Window   CurrentWindow     Current X window for game
*                 Screen   *ScreenPointer    Pointer to current screen on display
*                 int      ScreenNumber      Current X screen number
*                 Colormap CurrentColormap   Current X color map
*                 XVisualInfo *VisualInfo    Visual information stuct for GLX
*                 XSetWindowAttributes WindowAttributes
*                                            Attributes for CurrentWindow
*                 XSizeHints  SizeHints      Size hints for CurrentWindow
*                 bool     KeepAspect        true = keep aspect ratio of window
*                 bool     OverrideSettings  true = disable "Graphics" dialog
*                 char     WindowName[40]    CurrentWindow name (for title bar)
*
*                 bool     videoInitted      Video is initialized
*                 U32      currentTime       Current game time
*                 char     *DisplayHint      Display hints for display
*                  
*  Changelog:
*                 PGA 12/18/01: Initial revision
******************************************************************************/

class x86UNIXPlatformState
{
   private:
      Point2I              DesktopSize;
      Point2I              WindowSize;
      GLXContext           OpenGLContext;
      S32                  Desktop_bpp;
      Display              *display;
      Window               CurrentWindow;
      Screen               *ScreenPointer;
      int                  ScreenNumber;
      Colormap             CurrentColormap;
      XVisualInfo          *VisualInfo;
      XSetWindowAttributes WindowAttributes;
      XSizeHints           SizeHints;
      bool                 KeepAspect,
                           OverrideSettings;
      char                 WindowName[40];

   public:
      bool     videoInitted;
      U32      currentTime;
      char     *DisplayHint;


/******************************************************************************
*
*  Member routines:  Get and set XSetWindowAttributes private data member.
*
******************************************************************************/


      void SetWindowAttributes( XSetWindowAttributes NewAttributes )
      {
         WindowAttributes = NewAttributes;
      }

      XSetWindowAttributes * GetWindowAttributes()
      {
         return &WindowAttributes;
      }


/******************************************************************************
*
*  Member routines:  Get and set Colormap private data member.
*
******************************************************************************/

      void SetColormap( Colormap NewColormap )
      {
         CurrentColormap = NewColormap;
      }

      Colormap GetColormap()
      {
         return CurrentColormap;
      }


/******************************************************************************
*
*  Member routines:  Get and set XVisualInfo private data member.
*
******************************************************************************/

      void SetVisualInfo( XVisualInfo *NewInfo )
      {
         VisualInfo = NewInfo;
      }

      XVisualInfo * GetVisualInfo()
      {
         return VisualInfo;
      }


/******************************************************************************
*
*  Member routines:  Get and set X display Screen number private data member.
*
******************************************************************************/

      void SetScreenNumber( int NewNumber )
      {
         ScreenNumber = NewNumber;
      }

      int GetScreenNumber()
      {
         return ScreenNumber;
      }


/******************************************************************************
*
*  Member routines:  Get and set Screen pointer private data member.
*
******************************************************************************/

      void SetScreenPointer( Screen *NewScreenPointer )
      {
         ScreenPointer = NewScreenPointer;
      }
      
      Screen * GetScreenPointer()
      {
         return ScreenPointer;
      }


/******************************************************************************
*
*  Member routines:  Get and set Window private data member.
*
******************************************************************************/

      void SetWindow( Window NewWindow )
      {
         CurrentWindow = NewWindow;
      }
      
      Window GetWindow()
      {
         return CurrentWindow;
      }


/******************************************************************************
*
*  Member routines:  Get and set desktop color depth private data member.
*
******************************************************************************/

      void SetDesktop_bpp( S32 bpp )
      {
         Desktop_bpp = bpp;
      }
      
      S32 GetDesktop_bpp()
      {
         return Desktop_bpp;
      }


/******************************************************************************
*
*  Member routines:  Get and set GLXContext private data member.
*
******************************************************************************/
      
      void SetGLContextPointer( GLXContext GLContext )
      {
         OpenGLContext = GLContext;
      }
      
      GLXContext GetGLContextPointer()
      {
         return OpenGLContext;
      }


/******************************************************************************
*
*  Member routines:  Get and set Display private data member.
*
******************************************************************************/
      
      void SetDisplayPointer( Display *DisplayPointer )
      {
         display = DisplayPointer;
      }
      
      Display * GetDisplayPointer()
      {
         return display;
      }


/******************************************************************************
*
*  Member routines:  Set Point2I window size private data members given two
*                    seperate values, one for each WindowSize data mamber.
*
******************************************************************************/

      void SetWindowSize (S32 horizontal, S32 vertical )
      {
         WindowSize.set ( horizontal, vertical );
      }


/******************************************************************************
*
*  Member routines:  Set Point2I window size private data class given a Point2I
*                    data object.
*
******************************************************************************/

      void SetWindowSize( Point2I Size )
      {
         WindowSize = Size;
      }


/******************************************************************************
*
*  Member routines:  Return both x and y data members of the WindowSize private
*                    data class.
*
******************************************************************************/
      
      Point2I& GetWindowSize()
      {
         return ( WindowSize );
      }


/******************************************************************************
*
*  Member routines:  Get and set the window name private data member.
*
******************************************************************************/
   
      void SetName (const char * Name)
      {
         strcpy( WindowName, "\0" );
         strncpy( WindowName, Name, sizeof( WindowName ) );
      }
      
      const char * GetWindowName()
      {
         return WindowName;
      }


/******************************************************************************
*
*  Member routines:  Get a pointer to XSizeHints private data member.
*
******************************************************************************/
      
      XSizeHints * GetSizeHints()
      {
         return ( &SizeHints );
      }


/******************************************************************************
*
*  Member routines:  Get and set the command line override private data member.
*
******************************************************************************/
      
      bool GetOverrideSetting()
      {
         return OverrideSettings;
      }
      
      void SetOverrideSetting( bool NewOverride )
      {
         OverrideSettings = NewOverride;
      }


/******************************************************************************
*
*  Member routines:  Get and set aspect ratio command line private data member.
*
******************************************************************************/

      bool GetAspectSetting()
      {
         return KeepAspect;
      }
      
      void SetAspectSetting( bool NewAspect )
      {
         KeepAspect = NewAspect;
      }


/******************************************************************************
*
*  Member routine:  Get the Point2I desktop size private class data member.
*
******************************************************************************/

      Point2I GetDesktopSize()
      {
         return DesktopSize;
      }


/******************************************************************************
*
*  Member routine:  Set the Point2I desktop size private class data member
*                    given a Point2I data object.
*
******************************************************************************/

      void SetDesktopSize( S32 horizontal, S32 vertical )
      {
         DesktopSize.set( horizontal, vertical );
      }

/******************************************************************************
*
*  Member routine:  Initialize the class to a known state.
*
******************************************************************************/

      x86UNIXPlatformState()
      {
         Desktop_bpp = 16;
         SizeHints.flags = None;
         SizeHints.x = 0;
         SizeHints.y = 0;
         videoInitted = false;
         currentTime = 0;
         DisplayHint = NULL;
         KeepAspect = true;
         OverrideSettings = false;
         strcpy( WindowName, "Torque" );
         DesktopSize.set( 0, 0 );
         WindowSize.set( 800, 600 );
         CurrentColormap = 0;
         VisualInfo = NULL;
      }
};

extern x86UNIXPlatformState  * x86UNIXState;
