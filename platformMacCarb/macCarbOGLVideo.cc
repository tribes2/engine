//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "platformMacCarb/platformGL.h"
#include "platformMacCarb/platformMacCarb.h"
#include "platformMacCarb/maccarbOGLVideo.h"
#include "platform/platformAudio.h"
#include "console/console.h"
#include "math/mPoint.h"
#include "platform/event.h"
#include "platform/gameInterface.h"
#include "console/consoleInternal.h"
#include "console/ast.h"
#include "core/fileStream.h"

// !!!!!!!TBD
// Note 1: I found some comments at one point that noted that under OSX, using DSp AT ALL
// was worse than JUST using the AGL_FULLSCREEN system.  Right now, code uses a combo
// of the two under OSX, so we'll want to performance-test the two options...
// Note 2: Card Profiling code isn't doing anything.
// Note 3: Screen-res list building is hardcoded, and should tap DSp.
// Note 4: Gamma support.

// flip this if we want smooth gamma-fades when switching screenres or in/out of DSp.
// generally, it's less nice but easier to find probs if we just leave it off, even for release.
#define FADE_ON_SWITCH			0

#define CALL_IN_SPOCKETS_BUT_NOT_IN_CARBON   1
#if __APPLE__
#include <Carbon.h>
#else
#include <DrawSprocket.h>
#include <Gestalt.h>    
#include <Devices.h>
#endif

//-----------------------------------------------------------------------------------------
// prototypes and globals -- !!!!!!TBD - globals should mostly go away, into platState. 
//-----------------------------------------------------------------------------------------

static UInt32 CheckMacOSX (void);
void CToPStr (StringPtr outString, const char *inString);
void ReportError (char * strError);
OSStatus DSpDebugStr (OSStatus error);
GLenum aglDebugStr (void);

CGrafPtr SetupDSp (GDHandle *phGD, short *numDevices, int width, int height, int bpp);
void ShutdownDSp (CGrafPtr pDSpPort);

AGLContext SetupAGL (GDHandle hGD, AGLDrawable win);
AGLContext SetupAGLFullScreen (short display, int width, int height, int bpp);
void CleanupAGL (AGLContext ctx);

const RGBColor rgbBlack = { 0x0000, 0x0000, 0x0000 };

NumVersion gVersionDSp;
DSpContextAttributes gContextAttributes;
DSpContextReference gContext = 0;

AGLDrawable gpDSpPort = NULL; // will be NULL for full screen under X
Rect gRectPort = {0, 0, 0, 0};

GDHandle hGD;
short numDevices = 0;


//-----------------------------------------------------------------------------------------
// are we running on Mac OS X
// returns 0 if < Mac OS X or version number of Mac OS X (10.0 for GM)
//-----------------------------------------------------------------------------------------
static UInt32 CheckMacOSX (void)
{
   UInt32 response;
    
   if ((Gestalt(gestaltSystemVersion, (SInt32 *) &response) == noErr) && (response >= 0x01000))
      return response;
   else
      return 0;
}


//-----------------------------------------------------------------------------------------
// Copy C string to Pascal string
//-----------------------------------------------------------------------------------------
void CToPStr (StringPtr outString, const char *inString)
{   
   unsigned char x = 0;
   do
      *(((char*)outString) + x) = *(inString + x++);
   while ((*(inString + x) != 0)  && (x < 256));
   *((char*)outString) = (char) x;                           
}


//-----------------------------------------------------------------------------------------
// display errors -- for the moment, dumps to the console.  !!!!TBD
//-----------------------------------------------------------------------------------------
void ReportError (char * strError)
{
#if 1 // for now, dump all errors to console.
   Con::printf(strError);
#else
   char errMsgCStr [256];
   Str255 strErr;

   sprintf (errMsgCStr, "%s", strError); 

   // out as debug string
   CToPStr (strErr, errMsgCStr);
   DebugStr (strErr);
#endif
}


//-----------------------------------------------------------------------------------------
// translate DSp codes to strings.
//-----------------------------------------------------------------------------------------
OSStatus DSpDebugStr (OSStatus error)
{
   switch (error)
   {
      case noErr:
         break;
      case kDSpNotInitializedErr:
         ReportError ("DSp Error: Not initialized");
         break;
      case kDSpSystemSWTooOldErr:
         ReportError ("DSp Error: system Software too old");
         break;
      case kDSpInvalidContextErr:
         ReportError ("DSp Error: Invalid context");
         break;
      case kDSpInvalidAttributesErr:
         ReportError ("DSp Error: Invalid attributes");
         break;
      case kDSpContextAlreadyReservedErr:
         ReportError ("DSp Error: Context already reserved");
         break;
      case kDSpContextNotReservedErr:
         ReportError ("DSp Error: Context not reserved");
         break;
      case kDSpContextNotFoundErr:
         ReportError ("DSp Error: Context not found");
         break;
      case kDSpFrameRateNotReadyErr:
         ReportError ("DSp Error: Frame rate not ready");
         break;
      case kDSpConfirmSwitchWarning:
//         ReportError ("DSp Warning: Must confirm switch"); // removed since it is just a warning, add back for debugging
         return 0; // don't want to fail on this warning
         break;
      case kDSpInternalErr:
         ReportError ("DSp Error: Internal error");
         break;
      case kDSpStereoContextErr:
         ReportError ("DSp Error: Stereo context");
         break;
   }
   return error;
}


//-----------------------------------------------------------------------------------------
// if error dump agl errors to debugger string, return error
//-----------------------------------------------------------------------------------------
GLenum aglDebugStr (void)
{
   GLenum err = aglGetError();
   if (AGL_NO_ERROR != err)
      ReportError ((char *)aglErrorString(err));
   return err;
}

#pragma mark -

//-----------------------------------------------------------------------------------------
// Set up DSp screens, handles multi-monitor correctly
// side effect: sets both gpDSpWindow and gpPort
//-----------------------------------------------------------------------------------------
CGrafPtr SetupDSp (GDHandle *phGD, short *numDevices, int width, int height, int bpp)
{
   GDHandle hDevice;
   DSpContextAttributes foundAttributes;
   DisplayIDType displayID;
   
   *numDevices = 0;
   *phGD = NULL;

   // check for DSp
   if ((Ptr) kUnresolvedCFragSymbolAddress == (Ptr) DSpStartup) 
      ReportError ("DSp not installed");

   if (noErr != DSpDebugStr (DSpStartup()))
      return NULL;

   if ((Ptr) kUnresolvedCFragSymbolAddress == (Ptr) DSpGetVersion) 
      return NULL;
   else
      gVersionDSp = DSpGetVersion ();

   if ((gVersionDSp.majorRev == 0x01) && (gVersionDSp.minorAndBugRev < 0x99))
   {
      // this version of DrawSprocket is not completely functional on Mac OS X
      if (CheckMacOSX ())
         return NULL;
   }
 
   hDevice = DMGetFirstScreenDevice (true);                        // check number of screens
   do
   {
      (*numDevices)++;
      hDevice = DMGetNextScreenDevice (hDevice, true);
   }
   while (hDevice);
         
   // Note: DSp < 1.7.3 REQUIRES the back buffer attributes even if only one buffer is required
   dMemset(&gContextAttributes, 0, sizeof (DSpContextAttributes));
   gContextAttributes.displayWidth         = width;
   gContextAttributes.displayHeight      = height;
   gContextAttributes.colorNeeds         = kDSpColorNeeds_Require;
   gContextAttributes.displayBestDepth      = bpp;
   gContextAttributes.backBufferBestDepth   = bpp;
   gContextAttributes.displayDepthMask      = kDSpDepthMask_All;
   gContextAttributes.backBufferDepthMask   = kDSpDepthMask_All;
   gContextAttributes.pageCount         = 1;                        // only the front buffer is needed
   
   if (noErr != DSpDebugStr (DSpFindBestContext(&gContextAttributes, &gContext)))
   {
      ReportError ("DSpFindBestContext() had an error.");
      return NULL;
   }

   if (noErr != DSpDebugStr (DSpContext_GetAttributes (gContext, &foundAttributes))) // see what we actually found
   {
      ReportError ("DSpContext_GetAttributes() had an error.");
      return NULL;
   }

   // reset width and height to full screen and handle our own centering
   // HWA will not correctly center less than full screen size contexts
   gContextAttributes.displayWidth    = foundAttributes.displayWidth;
   gContextAttributes.displayHeight    = foundAttributes.displayHeight;
   gContextAttributes.pageCount      = 1;                           // only the front buffer is needed
   gContextAttributes.contextOptions   = 0 | kDSpContextOption_DontSyncVBL;   // no page flipping and no VBL sync needed

   DSpSetBlankingColor(&rgbBlack);
   if (noErr !=  DSpDebugStr (DSpContext_GetDisplayID(gContext, &displayID)))    // get our device for future use
   {
      ReportError ("DSpContext_GetDisplayID() had an error.");
      return NULL;
   }
   
   if (noErr !=  DMGetGDeviceByDisplayID (displayID, phGD, false)) // get GDHandle for ID'd device
   {
      ReportError ("DMGetGDeviceByDisplayID() had an error.");
      return NULL;
   }
   
   if (noErr !=  DSpDebugStr (DSpContext_Reserve ( gContext, &gContextAttributes))) // reserve our context
   {
      ReportError ("DSpContext_Reserve() had an error.");
      return NULL;
   }
 
//   HideCursor ();

#if FADE_ON_SWITCH
   DSpDebugStr (DSpContext_FadeGammaOut (NULL, NULL)); // fade display, remove for debug
#endif
   if (noErr != DSpDebugStr (DSpContext_SetState (gContext, kDSpContextState_Active))) // activate our context
   {
      ReportError ("DSpContext_SetState() had an error.");
      return NULL;
   }


   if ((CheckMacOSX ()) && !((gVersionDSp.majorRev > 0x01) || ((gVersionDSp.majorRev == 0x01) && (gVersionDSp.minorAndBugRev >= 0x99))))// DSp should be supported in version after 1.98
   {
      ReportError ("Mac OS X with DSp < 1.99 does not support DrawSprocket for OpenGL full screen");
      return NULL;
   }
   else if (CheckMacOSX ()) // DSp should be supported in versions 1.99 and later
   {
      CGrafPtr pPort;
      // use DSp's front buffer on Mac OS X
      if (noErr != DSpDebugStr (DSpContext_GetFrontBuffer (gContext, &pPort)))
      {
         ReportError ("DSpContext_GetFrontBuffer() had an error.");
         return NULL;
      }
      // there is a problem in Mac OS X GM CoreGraphics that may not size the port pixmap correctly
      // this will check the vertical sizes and offset if required to fix the problem
      // this will not center ports that are smaller then a particular resolution
      {
         long deltaV, deltaH;
         Rect portBounds;
         PixMapHandle hPix = GetPortPixMap (pPort);
         Rect pixBounds = (**hPix).bounds;
         GetPortBounds (pPort, &portBounds);
         deltaV = (portBounds.bottom - portBounds.top) - (pixBounds.bottom - pixBounds.top) +
                  (portBounds.bottom - portBounds.top - height) / 2;
         deltaH = -(portBounds.right - portBounds.left - width) / 2;
         if (deltaV || deltaH)
         {
            GrafPtr pPortSave;
            GetPort (&pPortSave);
            SetPort ((GrafPtr)pPort);
            // set origin to account for CG offset and if requested drawable smaller than screen rez
            SetOrigin (deltaH, deltaV);
            SetPort (pPortSave);
         }
      }
#if FADE_ON_SWITCH
      DSpDebugStr (DSpContext_FadeGammaIn (NULL, NULL));
#endif
      platState.appWindow = GetWindowFromPort(pPort);
      return pPort;
   }
   else // Mac OS 9 or less
   {
      WindowPtr pWindow;
      Rect rectWin;
      RGBColor rgbSave;
      GrafPtr pGrafSave;
      // create a new window in our context 
      // note: OpenGL is expecting a window so it can enumerate the devices it spans, 
      // center window in our context's gdevice
      rectWin.top  = (short) ((***phGD).gdRect.top + ((***phGD).gdRect.bottom - (***phGD).gdRect.top) / 2);        // h center
      rectWin.top  -= (short) (height / 2);
      rectWin.left  = (short) ((***phGD).gdRect.left + ((***phGD).gdRect.right - (***phGD).gdRect.left) / 2);   // v center
      rectWin.left  -= (short) (width / 2);
      rectWin.right = (short) (rectWin.left + width);
      rectWin.bottom = (short) (rectWin.top + height);
      
      pWindow = NewCWindow (NULL, &rectWin, "\p", 0, plainDBox, (WindowPtr)-1, 0, 0);

      // paint back ground black before fade in to avoid white background flash
      ShowWindow(pWindow);
      GetPort (&pGrafSave);
      SetPortWindowPort (pWindow);
      GetForeColor (&rgbSave);
      RGBForeColor (&rgbBlack);
      {
         Rect paintRect;
         GetWindowPortBounds (pWindow, &paintRect);
         PaintRect (&paintRect);
      }
      RGBForeColor (&rgbSave);      // ensure color is reset for proper blitting
      SetPort (pGrafSave);
#if FADE_ON_SWITCH
      DSpDebugStr (DSpContext_FadeGammaIn (NULL, NULL));
#endif
      platState.appWindow = pWindow;
      return (GetWindowPort (pWindow));
   }
}


//-----------------------------------------------------------------------------------------
// clean up DSp
//-----------------------------------------------------------------------------------------
void ShutdownDSp (CGrafPtr pDSpPort)
{
#if FADE_ON_SWITCH
   DSpContext_FadeGammaOut(NULL, NULL);
#endif
   if ((NULL != pDSpPort))
   {
      WindowPtr w = GetWindowFromPort(pDSpPort);
      if (w == platState.appWindow) // then clear.
         platState.appWindow = NULL;
      if (!CheckMacOSX()) // then we actually created a window.
         DisposeWindow(w);
   }
   DSpContext_SetState(gContext, kDSpContextState_Inactive);
#if FADE_ON_SWITCH
   DSpContext_FadeGammaIn(NULL, NULL);
#endif
   ShowCursor();
   DSpContext_Release(gContext);
   DSpShutdown();
   
   gContext = NULL;
   gpDSpPort = NULL;
}

#pragma mark -

//-----------------------------------------------------------------------------------------
// OpenGL Setup, for any case where we already have a drawable (whether windowed or DSp-screen)
//-----------------------------------------------------------------------------------------
AGLContext SetupAGL (GDHandle hGD, AGLDrawable drawable)
{
// software renderer = AGL_RENDERER_ID + AGL_RENDERER_GENERIC_ID + AGL_ALL_RENDERERS
// any renderer = AGL_ALL_RENDERERS instead of AGL_ACCELERATED
// OpenGL compliant ATI renderer = AGL_RENDERER_ID + AGL_RENDERER_ATI_ID + AGL_ACCELERATED

   short               i = 0;
   GLint               attrib[64];
   AGLPixelFormat    fmt;
   AGLContext        ctx;

   if ((Ptr) kUnresolvedCFragSymbolAddress == (Ptr) aglChoosePixelFormat) // check for existance of OpenGL
   {
      ReportError ("OpenGL not installed");
      return NULL;
   }   

   attrib [i++] = AGL_RGBA; // red green blue and alpha
   attrib [i++] = AGL_DOUBLEBUFFER; // double buffered
   attrib [i++] = AGL_ACCELERATED; // HWA pixel format only
   attrib [i++] = AGL_NO_RECOVERY; // HWA pixel format only
   attrib [i++] = AGL_DEPTH_SIZE; // MUST request a depth buffer.
   attrib [i++] = 16; // !!!!!!!TBD this should be from a pref variable.

//   attrib [i++] = AGL_ALL_RENDERERS; // choose even non-compliant renderers
//   attrib [i++] = AGL_RENDERER_ID; // choose only renderer type specified in next parameter
//   attrib [i++] = AGL_RENDERER_ATI_ID; // ATI renderer
//   attrib [i++] = AGL_RENDERER_GENERIC_ID; // generic renderer

// !!!!!TBD -- do we need to have a pixelsize in here?
//   attrib [i++] = AGL_FULLSCREEN;
//   attrib [i++] = AGL_PIXEL_SIZE;
//   attrib [i++] = bpp;

   // terminate the list.
   attrib [i++] = AGL_NONE;

   if (hGD)
      fmt = aglChoosePixelFormat (&hGD, 1, attrib); // get an appropriate pixel format
   else
      fmt = aglChoosePixelFormat(NULL, 0, attrib); // get an appropriate pixel format
   aglDebugStr ();
   if (NULL == fmt) 
   {
      ReportError("Could not find valid pixel format");
      return NULL;
   }

   ctx = aglCreateContext (fmt, NULL); // Create an AGL context
   aglDebugStr ();
   if (NULL == ctx)
   {
      ReportError ("Could not create context");
      return NULL;
   }

   if (!aglSetDrawable (ctx, drawable)) // attach the window to the context
   {
      ReportError ("SetDrawable failed");
      aglDebugStr ();
      return NULL;
   }


   if (!aglSetCurrentContext (ctx)) // make the context the current context
   {
      aglDebugStr ();
      aglSetDrawable (ctx, NULL);
      return NULL;
   }

   aglDestroyPixelFormat(fmt); // pixel format is no longer needed

   return ctx;
}

//-----------------------------------------------------------------------------------------------------------------------

// OpenGL Setup

AGLContext SetupAGLFullScreen (short display, int width, int height, int bpp)
{
   GLint         attrib[64];
   AGLPixelFormat    fmt;
   AGLContext        ctx;
   
// different possible pixel format choices for different renderers 
// basics requirements are RGBA and double buffer
// OpenGL will select acclerated context if available

   short i = 0;
   attrib [i++] = AGL_RGBA; // red green blue and alpha
   attrib [i++] = AGL_DOUBLEBUFFER; // double buffered
   attrib [i++] = AGL_ACCELERATED; // HWA pixel format only
   attrib [i++] = AGL_NO_RECOVERY; // HWA pixel format only
   attrib [i++] = AGL_DEPTH_SIZE;
   attrib [i++] = 16;

//   attrib [i++] = AGL_ALL_RENDERERS; // choose even non-compliant renderers
//   attrib [i++] = AGL_RENDERER_ID; // choose only renderer type specified in next parameter
//   attrib [i++] = AGL_RENDERER_ATI_ID; // ATI renderer
//   attrib [i++] = AGL_RENDERER_GENERIC_ID; // generic renderer

   attrib [i++] = AGL_FULLSCREEN;
   attrib [i++] = AGL_PIXEL_SIZE;
   attrib [i++] = bpp;

   attrib [i++] = AGL_NONE;

   if ((Ptr) kUnresolvedCFragSymbolAddress == (Ptr) aglChoosePixelFormat) // check for existance of OpenGL
   {
      ReportError ("OpenGL not installed");
      return NULL;
   }   

   fmt = aglChoosePixelFormat(NULL, 0, attrib); // this may fail if looking for acclerated across multiple monitors
   if (NULL == fmt) 
   {
      ReportError("Could not find valid pixel format");
      aglDebugStr ();
      return NULL;
   }

   ctx = aglCreateContext (fmt, NULL); // Create an AGL context
   if (NULL == ctx)
   {
      ReportError ("Could not create context");
      aglDebugStr ();
      return NULL;
   }

   static int hzrate = 60;
   if (!aglSetFullScreen (ctx, width, height, hzrate, display)) // attach fulls screen device to the context
   {
      ReportError ("SetFullScreen failed");
      aglDebugStr ();
      return NULL;
   }


   if (!aglSetCurrentContext (ctx)) // make the context the current context
   {
      ReportError ("SetCurrentContext failed");
      aglDebugStr ();
      aglSetDrawable (ctx, NULL); // turn off full screen
      return NULL;
   }

   aglDestroyPixelFormat(fmt); // pixel format is no longer needed

   return ctx;
}

//-----------------------------------------------------------------------------------------------------------------------

// OpenGL Cleanup

void CleanupAGL(AGLContext ctx)
{
   aglSetCurrentContext(NULL);
   aglSetDrawable(ctx, NULL);
   aglDestroyContext(ctx);
}

#pragma mark -

//===============================================================================

Boolean sCanDoFullscreen = TRUE;

struct CardProfile
{
   const char *vendor;     // manufacturer
   const char *renderer;   // driver name

   bool safeMode;          // destroy rendering context for resolution change
   bool lockArray;         // allow compiled vertex arrays
   bool subImage;          // allow glTexSubImage*
   bool fogTexture;        // require bound texture for combine extension
   bool noEnvColor;        // no texture environment color
   bool clipHigh;          // clip high resolutions
   bool deleteContext;      // delete rendering context
   bool texCompress;         // allow texture compression
   bool interiorLock;      // lock arrays for Interior render
   bool skipFirstFog;      // skip first two-pass fogging (dumb 3Dfx hack)
   bool only16;            // inhibit 32-bit resolutions
   bool noArraysAlpha;   // don't use glDrawArrays with a GL_ALPHA texture

   const char *proFile;      // explicit profile of graphic settings
};

struct OSCardProfile
{
   const char *vendor;     // manufacturer
   const char *renderer;   // driver name
};

static Vector<CardProfile> sCardProfiles(__FILE__, __LINE__);
static Vector<OSCardProfile> sOSCardProfiles(__FILE__, __LINE__);

struct ProcessorProfile
{
    U16 clock;  // clock range max
    U16 adjust; // CPU adjust   
};

static U8 sNumProcessors = 4;
static ProcessorProfile sProcessorProfiles[] =
{ 
    {  400,  0 },
    {  600,  5 },
    {  800, 10 },
    { 1000, 15 },
};

struct SettingProfile
{
    U16 performance;        // metric range max
    const char *settings;   // default file
};    

static U8 sNumSettings = 3;
static SettingProfile sSettingProfiles[] =
{
    {  33, "LowProfile.cs" },
    {  66, "MediumProfile.cs" },
    { 100, "HighProfile.cs" }, 
};

//------------------------------------------------------------------------------

static void cAddCardProfile(SimObject *, S32, const char **argv)
{
   CardProfile profile;

   profile.vendor = dStrdup(argv[1]);
   profile.renderer = dStrdup(argv[2]);

   profile.safeMode = dAtob(argv[3]);   
   profile.lockArray = dAtob(argv[4]);
   profile.subImage = dAtob(argv[5]);
   profile.fogTexture = dAtob(argv[6]);
   profile.noEnvColor = dAtob(argv[7]);
   profile.clipHigh = dAtob(argv[8]);
   profile.deleteContext = dAtob(argv[9]);
   profile.texCompress = dAtob(argv[10]);
   profile.interiorLock = dAtob(argv[11]);
   profile.skipFirstFog = dAtob(argv[12]);
   profile.only16 = dAtob(argv[13]);
   profile.noArraysAlpha = dAtob(argv[14]);

   if (dStrcmp(argv[15],""))
      profile.proFile = dStrdup(argv[15]);
   else
      profile.proFile = NULL;

   sCardProfiles.push_back(profile);
}

static void cAddOSCardProfile(SimObject *, S32, const char **argv)
{
   OSCardProfile profile;

   profile.vendor = dStrdup(argv[1]);
   profile.renderer = dStrdup(argv[2]);

/*
   profile.allowOpenGL = dAtob(argv[3]);
   profile.allowD3D = dAtob(argv[4]);
   profile.preferOpenGL = dAtob(argv[5]);
*/

   sOSCardProfiles.push_back(profile);
}

static void clearCardProfiles()
{
   while (sCardProfiles.size())
   {
      dFree((char *) sCardProfiles.last().vendor);
      dFree((char *) sCardProfiles.last().renderer);

      dFree((char *) sCardProfiles.last().proFile);

      sCardProfiles.decrement();
   }
}

static void clearOSCardProfiles()
{
   while (sOSCardProfiles.size())
   {
      dFree((char *) sOSCardProfiles.last().vendor);
      dFree((char *) sOSCardProfiles.last().renderer);

      sOSCardProfiles.decrement();
   }
}

static void execScript(const char *scriptFile)
{
   // execute the script
   FileStream str;

   if (!str.open(scriptFile, FileStream::Read))
      return;

   U32 size = str.getStreamSize();
   char *script = new char[size + 1];

   str.read(size, script);
   str.close();

   script[size] = 0;
   Con::executef(2, "eval", script);
   delete[] script;
}

static void profileSystem(const char *vendor, const char *renderer)
{
   Con::addCommand("addCardProfile", cAddCardProfile, "addCardProfile(vendor,renderer,safeMode,lockArray,subImage,fogTexture,noEnvColor,clipHigh,deleteContext,texCompress,interiorLock,skipFirstFog,only16,noArraysAlpha,proFile);", 16, 16); 
   Con::addCommand("addOSCardProfile", cAddOSCardProfile, "addOSCardProfile(vendor,renderer,allowOpenGL,allowD3D,preferOpenGL);", 6, 6);

   //Con::executef(2, "exec", "scripts/CardProfiles.cs");
   execScript("CardProfiles.cs");

   const char *os = NULL;
   char osProfiles[64];

#pragma message("todo: implement full profile checking")
   os = "MACCARB";
   
   if ( os != NULL )
   {
      dSprintf(osProfiles,64,"%s%sCardProfiles.cs","PPC",os);
      //Con::executef(2, "exec", osProfiles);
      execScript(osProfiles);
   }

   const char *proFile = NULL;
   U32 i;
   
   for (i = 0; i < sCardProfiles.size(); ++i)
      if (dStrstr(vendor, sCardProfiles[i].vendor) &&
          (!dStrcmp(sCardProfiles[i].renderer, "*") ||
           dStrstr(renderer, sCardProfiles[i].renderer)))
      {         
         Con::setBoolVariable("$pref::Video::safeModeOn", sCardProfiles[i].safeMode);
         Con::setBoolVariable("$pref::OpenGL::disableEXTCompiledVertexArray", !sCardProfiles[i].lockArray);
         Con::setBoolVariable("$pref::OpenGL::disableSubImage", !sCardProfiles[i].subImage);
         Con::setBoolVariable("$pref::TS::fogTexture", sCardProfiles[i].fogTexture);
         Con::setBoolVariable("$pref::OpenGL::noEnvColor", sCardProfiles[i].noEnvColor);
         Con::setBoolVariable("$pref::Video::clipHigh", sCardProfiles[i].clipHigh);
         if (!sCardProfiles[i].deleteContext)
         {
               Con::setBoolVariable("$pref::Video::deleteContext", false);
/*
            // HACK: The Voodoo3/5 on W2K crash when deleting a rendering context
            // So we're not deleting it.
            // Oh, and the Voodoo3 returns a Banshee renderer string under W2K
            dMemset( &OSVersionInfo, 0, sizeof( OSVERSIONINFO ) );
            OSVersionInfo.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
            if ( GetVersionEx( &OSVersionInfo ) &&
                 OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT &&
                 OSVersionInfo.dwMajorVersion == 5)
               Con::setBoolVariable("$pref::Video::deleteContext", false);
            else
               Con::setBoolVariable("$pref::Video::deleteContext", true);
*/
         }
         else
            Con::setBoolVariable("$pref::Video::deleteContext", true);
         Con::setBoolVariable("$pref::OpenGL::disableARBTextureCompression", !sCardProfiles[i].texCompress);
         Con::setBoolVariable("$pref::Interior::lockArrays", sCardProfiles[i].interiorLock);
         Con::setBoolVariable("$pref::TS::skipFirstFog", sCardProfiles[i].skipFirstFog);
         Con::setBoolVariable("$pref::Video::only16", sCardProfiles[i].only16);
         Con::setBoolVariable("$pref::OpenGL::noDrawArraysAlpha", sCardProfiles[i].noArraysAlpha);

         proFile = sCardProfiles[i].proFile;

         break;   
      }

   // defaults
   U16 glProfile;

   if (!proFile)
   {
      // no driver GL profile -- make one via weighting GL extensions
      glProfile = 25;

      glProfile += gGLState.suppARBMultitexture * 25;
      glProfile += gGLState.suppLockedArrays * 15;
      glProfile += gGLState.suppVertexArrayRange * 10;
      glProfile += gGLState.suppTextureEnvCombine * 5;
      glProfile += gGLState.suppPackedPixels * 5;
      glProfile += gGLState.suppTextureCompression * 5;
      glProfile += gGLState.suppS3TC * 5;
      glProfile += gGLState.suppFXT1 * 5;

      Con::setBoolVariable("$pref::Video::safeModeOn", true);
      Con::setBoolVariable("$pref::OpenGL::disableEXTCompiledVertexArray", false);
      Con::setBoolVariable("$pref::OpenGL::disableSubImage", false); 
      Con::setBoolVariable("$pref::TS::fogTexture", false);
      Con::setBoolVariable("$pref::OpenGL::noEnvColor", false);
      Con::setBoolVariable("$pref::Video::clipHigh", false);
      Con::setBoolVariable("$pref::Video::deleteContext", true);
      Con::setBoolVariable("$pref::OpenGL::disableARBTextureCompression", false);
      Con::setBoolVariable("$pref::Interior::lockArrays", true);
      Con::setBoolVariable("$pref::TS::skipFirstFog", false);
      Con::setBoolVariable("$pref::Video::only16", false);
      Con::setBoolVariable("$pref::OpenGL::noDrawArraysAlpha", false);
   }

   Con::setVariable("$pref::Video::profiledVendor", vendor);
   Con::setVariable("$pref::Video::profiledRenderer", renderer);

   if (!Con::getBoolVariable("$DisableSystemProfiling") &&
       ( dStrcmp(vendor, Con::getVariable("$pref::Video::defaultsVendor")) ||
          dStrcmp(renderer, Con::getVariable("$pref::Video::defaultsRenderer")) ))
   {
      if (proFile)
      {
         char settings[64];

         dSprintf(settings,64,"%s.cs",proFile);
         //Con::executef(2, "exec", settings);
         execScript(settings);
      }
      else
      {
         U16 adjust;

         // match clock with profile
         for (i = 0; i < sNumProcessors; ++i)
         {
            adjust = sProcessorProfiles[i].adjust;

            if (Platform::SystemInfo.processor.mhz < sProcessorProfiles[i].clock) break;
         }

         const char *settings;

         // match performance metric with profile
         for (i = 0; i < sNumSettings; ++i)
         {
            settings = sSettingProfiles[i].settings;

            if (glProfile+adjust <= sSettingProfiles[i].performance) break;
         }

         //Con::executef(2, "exec", settings);
         execScript(settings);
      }

      bool match = false;

      for (i = 0; i < sOSCardProfiles.size(); ++i)
         if (dStrstr(vendor, sOSCardProfiles[i].vendor) &&
             (!dStrcmp(sOSCardProfiles[i].renderer, "*") ||
              dStrstr(renderer, sOSCardProfiles[i].renderer)))
         {

//            Con::setBoolVariable("$pref::Video::allowOpenGL", sOSCardProfiles[i].allowOpenGL);
//            Con::setBoolVariable("$pref::Video::allowD3D", sOSCardProfiles[i].allowD3D);
//            Con::setBoolVariable("$pref::Video::preferOpenGL", sOSCardProfiles[i].preferOpenGL);
         
            match = true;

            break;
         }

      if (!match)
      {
         Con::setBoolVariable("$pref::Video::allowOpenGL", true);
//         Con::setBoolVariable("$pref::Video::allowD3D", true);
         Con::setBoolVariable("$pref::Video::preferOpenGL", true);
      }

      Con::setVariable("$pref::Video::defaultsVendor", vendor);
      Con::setVariable("$pref::Video::defaultsRenderer", renderer);
   }

   // write out prefs
   gEvalState.globalVars.exportVariables("$pref::*", "prefs/ClientPrefs.cs", false);

   clearCardProfiles();
   clearOSCardProfiles();
}    


//------------------------------------------------------------------------------
OpenGLDevice::OpenGLDevice()
{
   initDevice();
}


//------------------------------------------------------------------------------
void OpenGLDevice::initDevice()
{
   #pragma message("todo: enumerate available display modes");
   // Set the device name:
   mDeviceName = "OpenGL";

	// !!!!!TBD
	// PRE-TEST THAT GL IS AVAILABLE, REALLY.


   // Never unload a code module
   aglConfigure(AGL_RETAIN_RENDERERS, GL_TRUE);

   // Set some initial conditions:
   mResolutionList.clear();

   Resolution newRes1( 1024, 768, 16 );
   Resolution newRes2( 800, 600, 16 );
   Resolution newRes3( 640, 480, 16 );
   mResolutionList.push_back( newRes1 );
   mResolutionList.push_back( newRes2 );
   mResolutionList.push_back( newRes3 );

//   sCanDoFullscreen = FALSE;
// then we check DSp available.  could also check AGL_FULLSCREEN supported.
// .... TBD....

   
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
bool OpenGLDevice::activate( U32 width, U32 height, U32 bpp, bool fullScreen )
{
   Con::printf( "Activating the OpenGL display device..." );

   bool needResurrect = false;

#pragma message("todo: need to error check on return from agl funcs.")

  // If the rendering context exists, delete it:
/*
   if (platState.ctx)
   {
      Con::printf( "Killing the texture manager..." );
      Game->textureKill();
      needResurrect = true;

      Con::printf( "Making the rendering context not current..." );
      aglSetCurrentContext(NULL);
      aglSetDrawable(platState.ctx, NULL);

      Con::printf( "Deleting the rendering context..." );
      aglDestroyContext(platState.ctx);
      platState.ctx = NULL;
   }
*/

#pragma message("todo: figure out when we will need to get a new window")

   static bool onceAlready = false;
   bool profiled = false;

   // Set the resolution:
   if ( !setScreenMode( width, height, bpp, ( fullScreen || mFullScreenOnly ), true, false ) )
      return false;

   // Get original gamma ramp
//   mRestoreGamma = GetDeviceGammaRamp(platState.appDC, mOriginalRamp);

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

   Con::setVariable( "$pref::Video::displayDevice", mDeviceName );

/* !!!!!!TBD LATER!!!!!!!!
   // only do this for the first session
   if (!profiled &&
       !Con::getBoolVariable("$DisableSystemProfiling") &&
       (   dStrcmp(vendorString, Con::getVariable("$pref::Video::profiledVendor")) ||
          dStrcmp(rendererString, Con::getVariable("$pref::Video::profiledRenderer")) ))
   {
      profileSystem(vendorString, rendererString);
      profiled = true;
   }

   if (profiled)
   {      
      U32 width, height, bpp;

      if (Con::getBoolVariable("$pref::Video::clipHigh", false))
         for (S32 i = mResolutionList.size()-1; i >= 0; --i)
            if (mResolutionList[i].w > 1152 || mResolutionList[i].h > 864)
               mResolutionList.erase(i);

      if (Con::getBoolVariable("$pref::Video::only16", false))
         for (S32 i = mResolutionList.size()-1; i >= 0; --i)
            if (mResolutionList[i].bpp == 32)
               mResolutionList.erase(i);
      
      dSscanf(Con::getVariable("$pref::Video::resolution"), "%d %d %d", &width, &height, &bpp);
      setScreenMode(width, height, bpp,
                    Con::getBoolVariable("$pref::Video::fullScreen", true), false, false);
   }
*/

   // Do this here because we now know about the extensions:
   if ( gGLState.suppSwapInterval )
      setVerticalSync( !Con::getBoolVariable( "$pref::Video::disableVerticalSync" ) );
   Con::setBoolVariable("$pref::OpenGL::allowTexGen", true);

   return true;
}


//------------------------------------------------------------------------------
void OpenGLDevice::shutdown()
{
   OSStatus err = noErr;
   
   Con::printf( "Shutting down the OpenGL display device..." );

// temporarily don't have shutdown do anything.  setScreenMode internally does all this cleanup, so this is only
// useful for real shutdown.
// !!!!TBD!!!!!!
return;
// !!!!TBD!!!!!!

   // Delete the rendering context:
   if (platState.ctx)
   {
/*
      if (!Video::smNeedResurrect)
      {
         Con::printf( "Killing the texture manager..." );
          Game->textureKill();
          needResurrect = true;
      }
*/

// !!!!!TBD HANDLE GAMMA
//      if (mRestoreGamma)
//         SetDeviceGammaRamp(platState.appDC, mOriginalRamp);

      Con::printf( "Making darn sure the rendering context is not current..." );
      aglSetCurrentContext(NULL);
      aglSetDrawable(platState.ctx, NULL);

      if (0) // !!!!TBD error check
      {
         AssertFatal( false, "OpenGLDevice::setScreenMode\nqwglMakeCurrent( NULL, NULL ) failed!" );
         return;
      }

// !!!!!TBD
#pragma message("todo: ask Rick why we check pref here but not in activate()!!!!")
      if ( Con::getBoolVariable("$pref::Video::deleteContext", true) )
      {
         Con::printf( "Deleting the rendering context..." );
         aglDestroyContext(platState.ctx);
      }

      if (0) // !!!!TBD error check
      {
         AssertFatal( false, "OpenGLDevice::setScreenMode\nqwglDeleteContext failed!" );
         return;
      }
   
      platState.ctx = NULL;
   }

/* not sure shutdown should do this...  !!!!!!!TBD
   // Destroy the window:
   if ( platState.appWindow )
   {
      Con::printf( "Destroying the window..." );
      DisposeWindow( platState.appWindow );
      platState.appWindow = NULL;
   }
*/

   if ( smIsFullScreen )
   {
      Con::printf( "Restoring the desktop display settings (%dx%dx%d)...", platState.desktopWidth, platState.desktopHeight, platState.desktopBitsPixel );
      // shut down DSp.
      if (gpDSpPort)
         ShutdownDSp(gpDSpPort);
   }
}


//------------------------------------------------------------------------------
// This is the real workhorse function of the DisplayDevice...
//
bool OpenGLDevice::setScreenMode( U32 width, U32 height, U32 bpp, bool fullScreen, bool forceIt, bool repaint )
{
   WindowPtr curtain = NULL;
   char errorMessage[256];
   Resolution newRes( width, height, bpp );
   bool newFullScreen = fullScreen;
   bool safeModeOn = Con::getBoolVariable( "$pref::Video::safeModeOn" );
   OSStatus err;

//temp!!!!!TBD
// if not really changing, return immediately.
// this is sort-of an optimization, but also useful to prevent thrashing (which the
// current graphic startup would do otherwise...)
if (smCurrentRes.w == width && smCurrentRes.h == height && smCurrentRes.bpp == bpp && smIsFullScreen == newFullScreen)
   return(true);

   if ( !newFullScreen && mFullScreenOnly )
   {
      Con::warnf( ConsoleLogEntry::General, "OpenGLDevice::setScreenMode - device or desktop color depth does not allow windowed mode!" );
      newFullScreen = true;
   }

/*
   if ( !newFullScreen && ( newRes.w >= platState.desktopWidth || newRes.h >= platState.desktopHeight ) )
   {
      Con::warnf( ConsoleLogEntry::General, "OpenGLDevice::setScreenMode -- can't switch to resolution larger than desktop in windowed mode!" );
      // Find the largest standard resolution that will fit on the desktop:
      U32 resIndex;
      for ( resIndex = mResolutionList.size() - 1; resIndex > 0; resIndex-- )
      {
         if ( mResolutionList[resIndex].w < platState.desktopWidth 
           && mResolutionList[resIndex].h < platState.desktopHeight )
            break;  
      }
      newRes = mResolutionList[resIndex];
   }

   // DHC -- !!!!!TBD  -- Why the heck are we disallowing <640x480!?!?!  Consoles/other media devices could easily want!
   // THIS SAME ISSUE APPLIES TO PC CODE -- REINFORCES THAT THERE NEEDS TO BE MORE CODEPATH SHARING AMONGST PLATFORMS!!!!
   if ( newRes.w < 640 || newRes.h < 480 )
   {
      Con::warnf( ConsoleLogEntry::General, "OpenGLDevice::setScreenMode -- can't go smaller than 640x480!" );
      return false;
   }
*/


#pragma message("todo: how to handle picking res?")
#if 0 //!!!!!TBD how do we want to deal with this on the Mac???
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
            #define myAbs(x)   (((x)<0)?-(x):(x))
            S32 tw = S32( newRes.w ) - S32( mResolutionList[i].w );
            S32 th = S32( newRes.h ) - S32( mResolutionList[i].h );
            thisScore = myAbs(tw) + myAbs(th) 
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
      newRes.bpp = platState.desktopBitsPixel;
   }

   // Return if already at this resolution:
   if ( !forceIt && newRes == smCurrentRes && newFullScreen == smIsFullScreen )
      return true;
#endif

   Con::printf( "Setting screen mode to %dx%dx%d (%s)...", newRes.w, newRes.h, newRes.bpp, ( newFullScreen ? "fs" : "w" ) );

   bool needResurrect = false;

   if (gpDSpPort)
   {
      Con::printf("Shutting down DrawSprocket...");
      ShutdownDSp(gpDSpPort);
   }

// !!!!!!!TBD NOT SURE THAT THIS WILL DO THE RIGHT THING FOR MAC IN ALL CASES!
   if ( ( newRes.bpp != smCurrentRes.bpp ) || ( safeModeOn && ( ( smIsFullScreen != newFullScreen ) || newFullScreen ) ) )
   {
      // !!!!!TBD
      // NOTE THAT A LARGE CHUNK OF THIS CODE IS SHARED WITH shutdown(), AND THUS SHOULD BE >REALLY< SHARED CODE.
      // Delete the rendering context:
      if (platState.ctx)
      {
         if (!Video::smNeedResurrect)
         {
            Con::printf( "Killing the texture manager..." );
            Game->textureKill();
            needResurrect = true;
         }

         Con::printf( "Making darn sure the rendering context is not current..." );
         aglSetCurrentContext(NULL);
         aglSetDrawable(platState.ctx, NULL);
         if (0) // !!!!TBD error check
         {
            AssertFatal( false, "OpenGLDevice::setScreenMode\nqwglMakeCurrent( NULL, NULL ) failed!" );
            return false;
         }

         Con::printf( "Deleting the rendering context..." );
         // !!!! TBD WHY THE CHECK HERE & NOT IN ACTIVATE?
         if ( Con::getBoolVariable("$pref::Video::deleteContext",true) )
            aglDestroyContext(platState.ctx);
         if (0) // !!!!TBD error check
         {
            AssertFatal( false, "OpenGLDevice::setScreenMode\nqwglDeleteContext failed!" );
            return false;
         }

         // make sure it is null...
         platState.ctx = NULL;
      }
   }

   // on the mac, if the window is still around for any reason, dispose of it now.
   // DestroyGL or ShutdownDSp should have nuked it already if they created it...
   if ( platState.appWindow )
   {
      Con::printf( "Destroying the window..." );
      DisposeWindow( platState.appWindow );
      platState.appWindow = NULL;
   }

   Con::printf( "Changing the display settings to %s %dx%dx%d...", newFullScreen?"fullscreen":"windowed", newRes.w, newRes.h, newRes.bpp );

   // MAC DOESN'T NEED A CURTAIN -- prefers to use gamma fade out/in...
//   curtain = CreateCurtain( newRes.w, newRes.h );

   if ( newFullScreen )
   {
      smIsFullScreen = true;
   }
   else if ( smIsFullScreen )
   {
      Con::printf( "Changing to the desktop display settings (%dx%dx%d)...", platState.desktopWidth, platState.desktopHeight, platState.desktopBitsPixel );
      // this is automatic with the DestroyGL call or the ShutdownDSp call or the switching off fullscreen context.
      smIsFullScreen = false;
   }
   Con::setBoolVariable( "$pref::Video::fullScreen", smIsFullScreen );


   if (newFullScreen)
   {
      if (NULL == (gpDSpPort = SetupDSp (&hGD, &numDevices, newRes.w, newRes.h, newRes.bpp))) // Setup DSp for OpenGL
      {
         if (CheckMacOSX ()) // if setupDSp fails we can try SetaglFullScreen on Mac OS X
         {
            short display = 0; // need to define a GDHandle to display # mappinga
            if (NULL == (platState.ctx = SetupAGLFullScreen (display, newRes.w, newRes.h, newRes.bpp))) // Setup the OpenGL context
            {
               Con::printf("Failed to set up GL support");
               return false;
            }
            SetRect (&gRectPort, 0, 0, newRes.w, newRes.h); // l, t, r, b
            Con::printf("Successfully set up AGLFullscreen GL support [%d x %d x %d]", newRes.w, newRes.h, newRes.bpp);
         }
      }
      else
      {
         if (NULL == (platState.ctx = SetupAGL(hGD, gpDSpPort))) // Setup the OpenGL context
            return false;
         GetPortBounds (gpDSpPort, &gRectPort);
         if (hGD)
            Con::printf("Successfully set up DSpFullscreen GL support [%d x %d x %d]", gRectPort.right - gRectPort.left, gRectPort.bottom - gRectPort.top, (**(**hGD).gdPMap).pixelSize);
         else
            Con::printf("Successfully set up GL support [%d x %d]", gRectPort.right - gRectPort.left, gRectPort.bottom - gRectPort.top);
      }
   }
   else
   {
      //if we haven't yet created/assigned a plat window, need to do so now.
      if ( !newFullScreen && !platState.appWindow )
      {
         Con::printf( "Creating a new %swindow...", ( fullScreen ? "full-screen " : "" ) );
         platState.appWindow = CreateOpenGLWindow( newRes.w, newRes.h, newFullScreen );
         if ( !platState.appWindow )
         {
            AssertFatal( false, "OpenGLDevice::setScreenMode\nFailed to create a new window!" );
            return false;
         }
      }
      
      GLint attrib[32];
      int c = 0;
      attrib[c++] = AGL_RGBA;
      attrib[c++] = AGL_DOUBLEBUFFER;
      attrib[c++] = AGL_NO_RECOVERY;
      attrib[c++] = AGL_ACCELERATED;
      attrib[c++] = AGL_DEPTH_SIZE;
      attrib[c++] = 16; //!!!!TBD -- this should come from a pref!
// !!!!TBD
//      attrib[c++] = AGL_PIXEL_SIZE;
//      attrib[c++] = newRes.bpp;
      attrib[c++] = AGL_NONE;
      
      platState.fmt = aglChoosePixelFormat(NULL, 0, (const GLint *)attrib);
      if(platState.fmt == NULL) 
      {
         GLenum glerr = aglGetError();
         Platform::AlertOK("OpenGLDevice::setScreenMode Failure", (char *)aglErrorString(glerr));
         AssertFatal( false, "OpenGLDevice::setScreenMode\nNo valid pixel formats found!" );
         return false;
      }

//PRINT BACK THE FORMAT WE GOT!!!! !!!!TBD
//qwglDescribePixelFormat( platState.appDC, chosenFormat, sizeof( pfd ), &pfd );
//Con::printf( "Pixel format set:" );
//Con::printf( "  %d color bits, %d depth bits, %d stencil bits", platState.fmt.cColorBits, pfd.cDepthBits, pfd.cStencilBits );

      if ( !platState.ctx )
      {
         // Create a new rendering context:
         Con::printf( "Creating a new rendering context..." );
         platState.ctx = aglCreateContext(platState.fmt, NULL);
         if(platState.ctx == NULL)
         {
            AssertFatal( false, "OpenGLDevice::setScreenMode\naglCreateContext failed to create an OpenGL rendering context!" );
            return false;
         }

         // Attach the context to the window
         Con::printf( "Attaching the rendering context to the window..." );
         if( !aglSetDrawable(platState.ctx, GetWindowPort(platState.appWindow)) )
         {
            AssertFatal( false, "OpenGLDevice::setScreenMode\naglSetDrawable failed to attach rendering context to window!" );
            return false;
         }
      }
   }

   // Make the new rendering context current:
   Con::printf( "Making the new rendering context current..." );
   aglSetCurrentContext(platState.ctx);
   if (0) // need to add error check!!!!!TBD
   {
      AssertFatal( false, "OpenGLDevice::setScreenMode\naglSetCurrentContext failed to make the rendering context current!" );
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

   if ( gGLState.suppSwapInterval )
      setVerticalSync( !Con::getBoolVariable( "$pref::Video::disableVerticalSync" ) );

   smCurrentRes = newRes;
   Platform::setWindowSize( newRes.w, newRes.h );
   char tempBuf[15];
   dSprintf( tempBuf, sizeof( tempBuf ), "%d %d %d", smCurrentRes.w, smCurrentRes.h, smCurrentRes.bpp );
   Con::setVariable( "$pref::Video::resolution", tempBuf );

   if ( curtain )
      DisposeWindow( curtain );

   if ( repaint )
      Con::evaluate( "resetCanvas();" );

   return true;
}

//------------------------------------------------------------------------------
void OpenGLDevice::swapBuffers()
{
   // TBD!!!!! should this be setting to platState.ctx
   // how to in future multiple contexts like RIVET??
   
   //dhc - also adding in a sanity check here.
   AssertISV(platState.ctx, "OpenGLDevice::swapBuffers -- No GL context in place!");
   
   aglSwapBuffers(aglGetCurrentContext()); 
}


//------------------------------------------------------------------------------
const char* OpenGLDevice::getDriverInfo()
{
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
}


//------------------------------------------------------------------------------
bool OpenGLDevice::getGammaCorrection(F32 &g)
{
   U16 ramp[256*3];

#pragma message("todo: gamma")
/*
   if (!GetDeviceGammaRamp(platState.appDC, ramp))
      return false;
*/

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
}    

//------------------------------------------------------------------------------
bool OpenGLDevice::setGammaCorrection(F32 g)
{
   U16 ramp[256*3];

   for (U16 i = 0; i < 256; ++i)
      ramp[i] = mPow((F32) i/256.0f, g) * 65535.0f;
   dMemcpy(&ramp[256],ramp,256*sizeof(U16));
   dMemcpy(&ramp[512],ramp,256*sizeof(U16));      

   bool t = false;
#pragma message("todo: gamma")
//   t = SetDeviceGammaRamp(platState.appDC, ramp);
   return t;
}

//------------------------------------------------------------------------------
bool OpenGLDevice::setVerticalSync( bool on )
{
#pragma message("todo: gl swap interval ext")
/*
   if ( !gGLState.suppSwapInterval )
      return( false );

   return( qwglSwapIntervalEXT( on ? 1 : 0 ) );
*/
   return(false);
}

//------------------------------------------------------------------------------
DisplayDevice* OpenGLDevice::create()
{
   bool result = true;
   bool fullScreenOnly = false;
#pragma message("todo: ::create full gl availability test")
/*
   bool result = false;
   bool fullScreenOnly = false;

   OSVERSIONINFO OSVersionInfo;
   U32 switchedNT = false;

   dMemset( &OSVersionInfo, 0, sizeof( OSVERSIONINFO ) );
   OSVersionInfo.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
   if ( GetVersionEx( &OSVersionInfo ) &&
        OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
   {
      DEVMODE devMode;

      dMemset( &devMode, 0, sizeof( devMode ) );
      devMode.dmSize = sizeof( devMode );
      devMode.dmBitsPerPel = platState.desktopBitsPixel == 16 ? 32 : 16;
      devMode.dmFields = DM_BITSPERPEL;
      
      // attempt bit-depth change to test Windows 95B
      if ( ChangeDisplaySettings( &devMode, CDS_TEST ) != DISP_CHANGE_SUCCESSFUL )
         smCanSwitchBitDepth = false;
   }
   // switching NT to 32-bit desktop to determine if we have a 16-bit card
   else if (OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT &&
            OSVersionInfo.dwMajorVersion == 4 &&
            platState.desktopBitsPixel != 32)
   {
      DEVMODE devMode;

      dMemset( &devMode, 0, sizeof( devMode ) );
      devMode.dmSize       = sizeof( devMode );
      devMode.dmBitsPerPel = 32;
      devMode.dmFields     = DM_BITSPERPEL;

      switchedNT = ChangeDisplaySettings( &devMode, 0 ) == DISP_CHANGE_SUCCESSFUL;
   }

   // This shouldn't happen, but just to be safe...
   if ( platState.hinstOpenGL )
      QGL_Shutdown();

   if (!QGL_Init( "opengl32", "glu32" ))
   {
      if (switchedNT)
         ChangeDisplaySettings( NULL, 0 );

      return NULL;
   }

   // Create a test window to see if OpenGL hardware acceleration is available:
   WNDCLASS wc;
   dMemset(&wc, 0, sizeof(wc));   
   wc.style         = CS_OWNDC;
   wc.lpfnWndProc   = DefWindowProc;
   wc.hInstance     = platState.appInstance;
   wc.lpszClassName = "OGLTest";
   RegisterClass( &wc );

   HWND testWindow = CreateWindow( 
      "OGLTest", 
      "", 
      WS_POPUP, 
      0, 0, 640, 480, 
      NULL, 
      NULL, 
      platState.appInstance, 
      NULL );

   if ( testWindow )
   {
      HDC testDC = GetDC( testWindow );
      if ( testDC )
      {
         PIXELFORMATDESCRIPTOR pfd;
         CreatePixelFormat( &pfd, 16, 16, 8, false );
         U32 chosenFormat = ChooseBestPixelFormat( testDC, &pfd );
         if ( chosenFormat != 0 )
         {
            qwglDescribePixelFormat( testDC, chosenFormat, sizeof( pfd ), &pfd );

            result = !( pfd.dwFlags & PFD_GENERIC_FORMAT );
  
              if ( result && (platState.desktopBitsPixel == 16 || pfd.cColorBits == 16) )
               D3DDevice::smStay16 = true;
         
            if ( result && platState.desktopBitsPixel != 16 && !smCanSwitchBitDepth)
            {
               // If Windows 95 cannot switch bit depth, it should only attempt 16-bit cards
               // with a 16-bit desktop

               // See if we can get a 32-bit pixel format:
               PIXELFORMATDESCRIPTOR pfd;

               CreatePixelFormat( &pfd, 32, 24, 8, false );
               S32 chosenFormat = ChooseBestPixelFormat( testDC, &pfd );
               if ( chosenFormat != 0 )
               {
                  qwglDescribePixelFormat( platState.appDC, chosenFormat, sizeof( pfd ), &pfd );
      
                  if (pfd.cColorBits == 16)
                  {
                     Platform::AlertOK("Requires 16-Bit Desktop",
                                       "You must run in 16-bit color to play \"Tribes 2\".\nPlease quit the game, set your desktop color depth to 16-bit\nand then restart the application.");

                     result = false;
                  }
               }
            }
            // Don't allow 16-bit cards to do windowed mode on NT
            else if (OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT &&
                     OSVersionInfo.dwMajorVersion == 4 &&
                     result && pfd.cColorBits == 16)
               fullScreenOnly = true;
         }
         else if ( (platState.desktopBitsPixel != 16 || switchedNT) && smCanSwitchBitDepth )
         {
            // Try again after changing the display to 16-bit:
            ReleaseDC( testWindow, testDC );
            DestroyWindow( testWindow );

            DEVMODE devMode;
            dMemset( &devMode, 0, sizeof( devMode ) );
            devMode.dmSize       = sizeof( devMode );
            devMode.dmBitsPerPel = 16;
            devMode.dmFields     = DM_BITSPERPEL;

            U32 test = ChangeDisplaySettings( &devMode, 0 );
            if ( test == DISP_CHANGE_SUCCESSFUL )
            {
               testWindow = CreateWindow( 
                  "OGLTest", 
                  "", 
                  WS_OVERLAPPED | WS_CAPTION, 
                  0, 0, 640, 480, 
                  NULL, 
                  NULL, 
                  platState.appInstance, 
                  NULL );

               if ( testWindow )
               {
                  testDC = GetDC( testWindow );
                  if ( testDC )
                  {
                     CreatePixelFormat( &pfd, 16, 16, 8, false );
                     chosenFormat = ChooseBestPixelFormat( testDC, &pfd );
                     if ( chosenFormat != 0 )
                     {
                        qwglDescribePixelFormat( testDC, chosenFormat, sizeof( pfd ), &pfd );
 
                        result = !( pfd.dwFlags & PFD_GENERIC_FORMAT );
                        if ( result )
                           fullScreenOnly = true;
                     }
                  }
               }
            }
            ChangeDisplaySettings( NULL, 0 );
            switchedNT = false;
         }
         else if ( platState.desktopBitsPixel != 16 && !smCanSwitchBitDepth )
            Platform::AlertOK("Requires 16-Bit Desktop",
                              "You must run in 16-bit color to play \"Tribes 2\".\nPlease quit the game, set your desktop color depth to 16-bit\nand then restart the application.");

         ReleaseDC( testWindow, testDC );
      }
      DestroyWindow( testWindow );
   }

   UnregisterClass( "OGLTest", platState.appInstance );

   QGL_Shutdown();

   // Return NT to previous desktop bit depth
   if (switchedNT)
      ChangeDisplaySettings( NULL, 0 );
*/

   if ( result )
   {
      OpenGLDevice* newOGLDevice = new OpenGLDevice();
      if ( newOGLDevice )
      {
         newOGLDevice->mFullScreenOnly = fullScreenOnly;
         return (DisplayDevice*) newOGLDevice;
      }
      else 
         return NULL;
   }
   else
      return NULL;
}
