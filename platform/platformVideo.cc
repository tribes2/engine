//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "Platform/platformVideo.h"
#include "GUI/guiCanvas.h"
#include "console/console.h"
#include "Platform/gameInterface.h"

extern void GameDeactivate( bool noRender );
extern void GameReactivate();

// Static class data:
Vector<DisplayDevice*>  Video::smDeviceList;
DisplayDevice*          Video::smCurrentDevice;
bool					Video::smCritical = false;
bool					Video::smNeedResurrect = false;

Resolution  DisplayDevice::smCurrentRes;
bool        DisplayDevice::smIsFullScreen;

//--------------------------------------------------------------------------
static bool cSetDisplayDevice( SimObject*, S32 argc, const char** argv )
{
	Resolution currentRes = Video::getResolution();

	U32 width = ( argc > 2 ) ? dAtoi( argv[2] ) : currentRes.w;
	U32 height =  ( argc > 3 ) ? dAtoi( argv[3] ) : currentRes.h;
	U32 bpp = ( argc > 4 ) ? dAtoi( argv[4] ) : currentRes.bpp;
	bool fullScreen = ( argc > 5 ) ? dAtob( argv[5] ) : Video::isFullScreen();

   return( Video::setDevice( argv[1], width, height, bpp, fullScreen ) );
}


//--------------------------------------------------------------------------
static bool cSetScreenMode( SimObject*, S32, const char** argv )
{
   return( Video::setScreenMode( dAtoi( argv[1] ), dAtoi( argv[2] ), dAtoi( argv[3] ), dAtob( argv[4] ) ) );
}


//------------------------------------------------------------------------------
static bool cFullScreenToggle( SimObject*, S32, const char** )
{
   return( Video::toggleFullScreen() );
}


//------------------------------------------------------------------------------
static bool cIsFullScreen( SimObject*, S32, const char** )
{
   return( Video::isFullScreen() );
}


//--------------------------------------------------------------------------
static bool cSwitchBitDepth( SimObject*, S32, const char** )
{
   if ( !Video::isFullScreen() )
   {
      Con::warnf( ConsoleLogEntry::General, "Can only switch bit depth in full-screen mode!" );
      return( false );
   }

   Resolution res = Video::getResolution();
   return( Video::setResolution( res.w, res.h, ( res.bpp == 16 ? 32 : 16 ) ) );
}


//--------------------------------------------------------------------------
static bool cPrevRes( SimObject*, S32, const char** )
{
   return( Video::prevRes() );
}                                   


//--------------------------------------------------------------------------
static bool cNextRes( SimObject*, S32, const char** )
{
   return( Video::nextRes() );
}


//--------------------------------------------------------------------------
static const char* cGetResolution( SimObject*, S32, const char** )
{
   static char resBuf[16];
   Resolution res = Video::getResolution();
   dSprintf( resBuf, sizeof(resBuf), "%d %d %d", res.w, res.h, res.bpp );
   return( resBuf );
}


//--------------------------------------------------------------------------
static bool cSetResolution( SimObject*, S32 argc, const char** argv )
{
   U32 width = dAtoi( argv[1] );
   U32 height = dAtoi( argv[2] );
   U32 bpp = 0;
   if ( argc == 4 )
   {
      bpp = dAtoi( argv[3] );
      if ( bpp != 16 && bpp != 32 )
         bpp = 0;
   }

   return( Video::setResolution( width, height, bpp ) );
}

//------------------------------------------------------------------------------
static const char* cVideoGetDeviceList( SimObject*, S32, const char** )
{
	return( Video::getDeviceList() );	
}


//------------------------------------------------------------------------------
static const char* cVideoGetResList( SimObject*, S32, const char** argv )
{
	DisplayDevice* device = Video::getDevice( argv[1] );
	if ( !device )
	{
		Con::warnf( ConsoleLogEntry::General, "\"%s\" display device not found!", argv[1] );
		return( NULL );
	}
	
	return( device->getResolutionList() );		
}


//------------------------------------------------------------------------------
static const char* cVideoGetDriverInfo( SimObject*, S32, const char** )
{
	return( Video::getDriverInfo() );
}


//------------------------------------------------------------------------------
static bool cIsDeviceFullScreenOnly( SimObject*, S32, const char** argv )
{
	DisplayDevice* device = Video::getDevice( argv[1] );
	if ( !device )
	{
		Con::warnf( ConsoleLogEntry::General, "\"%s\" display device not found!", argv[1] );
		return( false );
	}
	
	return( device->isFullScreenOnly() );		
}
    

//------------------------------------------------------------------------------
static F32 sgOriginalGamma = -1.0;
static F32 sgGammaCorrection = 0.0;

ConsoleFunction(videoSetGammaCorrection, void, 2, 2, "setGammaCorrection(gamma);")
{
   argc;
   F32 g = mClampF(dAtof(argv[1]),0.0,1.0);
   F32 d = -(g - 0.5);
   
   if (d != sgGammaCorrection &&
   	 (sgOriginalGamma != -1.0 || Video::getGammaCorrection(sgOriginalGamma)))
      Video::setGammaCorrection(sgOriginalGamma+d);
	sgGammaCorrection = d;
}    

//------------------------------------------------------------------------------
void Video::init()
{
   destroy();

	// Add console commands:
   Con::addCommand( "setDisplayDevice",         cSetDisplayDevice,         "setDisplayDevice( deviceName{, width{, height{, bpp{, fullScreen}}}}} );", 2, 6 );
   Con::addCommand( "setScreenMode",			   cSetScreenMode,            "setScreenMode( width, height, bpp, fullScreen );", 5, 5 );
   Con::addCommand( "toggleFullScreen",         cFullScreenToggle,         "toggleFullScreen();", 1, 1 );
   Con::addCommand( "isFullScreen",             cIsFullScreen,             "isFullScreen();", 1, 1 );
   Con::addCommand( "switchBitDepth",			   cSwitchBitDepth,           "switchBitDepth();", 1, 1 );
   Con::addCommand( "prevResolution", 			   cPrevRes,                  "prevResolution();", 1, 1 );
   Con::addCommand( "nextResolution", 			   cNextRes,                  "nextResolution();", 1, 1 );
   Con::addCommand( "getResolution", 			   cGetResolution,            "getResolution();", 1, 1 );
   Con::addCommand( "setResolution", 			   cSetResolution,            "setResolution( width, height, bpp );", 3, 4 );
   Con::addCommand( "setRes", 					   cSetResolution,            "setRes( width, height, bpp );", 3, 4 );
   Con::addCommand( "getDisplayDeviceList",     cVideoGetDeviceList,       "getDisplayDeviceList();", 1, 1 );
   Con::addCommand( "getResolutionList",		   cVideoGetResList,          "getResolutionList( deviceName );", 2, 2 );
   Con::addCommand( "getVideoDriverInfo",		   cVideoGetDriverInfo,       "getVideoDriverInfo();", 1, 1 );
   Con::addCommand( "isDeviceFullScreenOnly",   cIsDeviceFullScreenOnly,   "isDeviceFullScreenOnly( deviceName );", 2, 2 );
}


//------------------------------------------------------------------------------
void Video::destroy()
{
   if ( smCurrentDevice )
   {
      smCritical = true;
      smCurrentDevice->shutdown();
      smCritical = false;
   }

   smCurrentDevice = NULL;

   for ( U32 i = 0; i < smDeviceList.size(); i++ )
      delete smDeviceList[i];

   smDeviceList.clear();
}


//------------------------------------------------------------------------------
bool Video::installDevice( DisplayDevice *dev )
{
   if ( dev )
   {
      smDeviceList.push_back( dev );
      return true;
   }
   return false;
}   


//------------------------------------------------------------------------------
bool Video::setDevice( const char *renderName, U32 width, U32 height, U32 bpp, bool fullScreen )
{
   S32 deviceIndex = NO_DEVICE;
	S32 iOpenGL = -1;
	S32 iD3D = -1;

   for ( S32 i = 0; i < smDeviceList.size(); i++ )
   {
      if ( dStrcmp( smDeviceList[i]->mDeviceName, renderName ) == 0 )
         deviceIndex = i;

		if ( dStrcmp( smDeviceList[i]->mDeviceName, "OpenGL" ) == 0 )
			iOpenGL = i;
		if ( dStrcmp( smDeviceList[i]->mDeviceName, "D3D" ) == 0 )
			iD3D = i;
   }

   if ( deviceIndex == NO_DEVICE )
   {
      Con::warnf( ConsoleLogEntry::General, "\"%s\" display device not found!", renderName );
      return false;
   }

   // Change the display device:
   if ( smDeviceList[deviceIndex] != NULL )
   {      
      if (smCurrentDevice && smCurrentDevice != smDeviceList[deviceIndex])
		{
			Con::printf( "Deactivating the previous display device..." );
			Game->textureKill();
			smNeedResurrect = true;
			smCurrentDevice->shutdown();
		}
      
      Con::printf( "Activating the %s display device...", renderName );
      smCurrentDevice = smDeviceList[deviceIndex];
      
      smCritical = true;
      bool result = smCurrentDevice->activate( width, height, bpp, fullScreen );
	   smCritical = false;

	   if ( result )
	   {
			if (smNeedResurrect)
			{
				Game->textureResurrect();
         	smNeedResurrect = false;
         }
         if (sgOriginalGamma != -1.0 || Video::getGammaCorrection(sgOriginalGamma))
         	Video::setGammaCorrection(sgOriginalGamma + sgGammaCorrection);
	      Con::evaluate("resetCanvas();");
	   }

		if (iOpenGL != -1 && !Con::getBoolVariable("$pref::Video::allowOpenGL"))
		{
			// change to D3D, delete OpenGL in the recursive call
			if (dStrcmp(renderName,"OpenGL") == 0)
			{
				U32 w, h, d;
				
				dSscanf(Con::getVariable("$pref::Video::resolution"), "%d %d %d", &w, &h, &d);

				return setDevice("D3D",w,h,d,Con::getBoolVariable("$pref::Video::fullScreen",true));
			}
			else
			{
				delete smDeviceList[iOpenGL];
				smDeviceList.erase(iOpenGL);
			}
		}
		else if (iD3D != -1 && !Con::getBoolVariable("$pref::Video::allowD3D"))
		{
			// change to OpenGL, delete D3D in the recursive call
			if (dStrcmp(renderName,"D3D") == 0)
			{
				U32 w, h, d;
				
				dSscanf(Con::getVariable("$pref::Video::resolution"), "%d %d %d", &w, &h, &d);

				return setDevice("OpenGL",w,h,d,Con::getBoolVariable("$pref::Video::fullScreen",true));
			}
			else
			{
				delete smDeviceList[iD3D];
				smDeviceList.erase(iD3D);
			}
		}
		else if (iD3D != -1 &&
					dStrcmp(renderName,"OpenGL") == 0 &&
					!Con::getBoolVariable("$pref::Video::preferOpenGL") &&
					!Con::getBoolVariable("$pref::Video::appliedPref"))
		{
			U32 w, h, d;
				
			dSscanf(Con::getVariable("$pref::Video::resolution"), "%d %d %d", &w, &h, &d);
			Con::setBoolVariable("$pref::Video::appliedPref", true);

			return setDevice("D3D",w,h,d,Con::getBoolVariable("$pref::Video::fullScreen",true));
		}
		else
			Con::setBoolVariable("$pref::Video::appliedPref", true); 

      // The video mode activate may have failed above, return that status
	   return( result );
   }

   return( false );
}


//------------------------------------------------------------------------------
bool Video::setScreenMode( U32 width, U32 height, U32 bpp, bool fullScreen )
{
   if ( smCurrentDevice )
   {
      smCritical = true;
      bool result = smCurrentDevice->setScreenMode( width, height, bpp, fullScreen );		
   	smCritical = false;

	   return( result );
   }

   return( false );	
}


//------------------------------------------------------------------------------
void Video::deactivate()
{
   if ( smCritical ) return;

   GameDeactivate( DisplayDevice::isFullScreen() );
   if ( smCurrentDevice && DisplayDevice::isFullScreen() )
   {
	   smCritical = true;

      Game->textureKill();
      smCurrentDevice->shutdown();
      
      Platform::minimizeWindow();      
      smCritical = false;
   }
}    

//------------------------------------------------------------------------------
void Video::reactivate()
{
   if ( smCritical ) return;

   if ( smCurrentDevice && DisplayDevice::isFullScreen() )
   {
	   Resolution res = DisplayDevice::getResolution();

      smCritical = true;
      smCurrentDevice->activate(res.w,res.h,res.bpp,DisplayDevice::isFullScreen());
	   Game->textureResurrect();

	   smCritical = false;
      if (sgOriginalGamma != -1.0)
      	Video::setGammaCorrection(sgOriginalGamma + sgGammaCorrection);
   }
   GameReactivate();
}


//------------------------------------------------------------------------------
bool Video::setResolution( U32 width, U32 height, U32 bpp )
{
   if ( smCurrentDevice )
   {
      if ( bpp == 0 )
         bpp = DisplayDevice::getResolution().bpp;

	  smCritical = true;
      bool result = smCurrentDevice->setResolution( width, height, bpp );
	  smCritical = false;

	  return( result );
   }
   return( false );	
}


//------------------------------------------------------------------------------
bool Video::toggleFullScreen()
{
   if ( smCurrentDevice )
   {
      smCritical = true;
      bool result = smCurrentDevice->toggleFullScreen();
	  smCritical = false;
   
   	  return( result );
   }
   return( false );	
}


//------------------------------------------------------------------------------
DisplayDevice* Video::getDevice( const char* renderName )
{
   for ( S32 i = 0; i < smDeviceList.size(); i++ )
   {
      if ( dStrcmp( smDeviceList[i]->mDeviceName, renderName ) == 0 )
			return( smDeviceList[i] );
   }

	return( NULL );	
}


//------------------------------------------------------------------------------
bool Video::prevRes()
{
   if ( smCurrentDevice )
   {
      smCritical = true;
      bool result = smCurrentDevice->prevRes();
	  smCritical = false;

      return( result );
   }
   return( false );	
}


//------------------------------------------------------------------------------
bool Video::nextRes()
{
   if ( smCurrentDevice )
   {
      smCritical = true;
      bool result = smCurrentDevice->nextRes();
	  smCritical = false;

      return( result );
   }
   return( false );	
}


//------------------------------------------------------------------------------
Resolution Video::getResolution()
{
   return DisplayDevice::getResolution();
}


//------------------------------------------------------------------------------
const char* Video::getDeviceList()
{
	U32 deviceCount = smDeviceList.size();
	if ( deviceCount > 0 ) // It better be...
	{		
		U32 strLen = 0, i;
		for ( i = 0; i < deviceCount; i++ )
			strLen += ( dStrlen( smDeviceList[i]->mDeviceName ) + 1 );

		char* returnString = Con::getReturnBuffer( strLen );
		dStrcpy( returnString, smDeviceList[0]->mDeviceName );
		for ( i = 1; i < deviceCount; i++ )
		{
			dStrcat( returnString, "\t" );
			dStrcat( returnString, smDeviceList[i]->mDeviceName );
		}

		return( returnString );				
	}

	return( NULL );
}


//------------------------------------------------------------------------------
const char* Video::getResolutionList()
{
   if ( smCurrentDevice )
      return smCurrentDevice->getResolutionList();
   else
      return NULL;
}


//------------------------------------------------------------------------------
const char* Video::getDriverInfo()
{
   if ( smCurrentDevice )
      return smCurrentDevice->getDriverInfo();
   else
      return NULL;
}


//------------------------------------------------------------------------------
bool Video::isFullScreen()
{
   return DisplayDevice::isFullScreen();
}


//------------------------------------------------------------------------------
void Video::swapBuffers()
{
   if ( smCurrentDevice )
      smCurrentDevice->swapBuffers();
}


//------------------------------------------------------------------------------
bool Video::getGammaCorrection(F32 &g)
{
   if (smCurrentDevice)
      return smCurrentDevice->getGammaCorrection(g);
 
   return false;	
}


//------------------------------------------------------------------------------
bool Video::setGammaCorrection(F32 g)
{
   if (smCurrentDevice)
      return smCurrentDevice->setGammaCorrection(g);
 
   return false;	
}

//------------------------------------------------------------------------------
bool Video::setVerticalSync( bool on )
{
   if ( smCurrentDevice )
      return( smCurrentDevice->setVerticalSync( on ) );

   return( false );
}

ConsoleFunction( setVerticalSync, bool, 2, 2, "setVerticalSync( <bool> )" )
{
   argc;
   return( Video::setVerticalSync( dAtob( argv[1] ) ) );
}

//------------------------------------------------------------------------------
DisplayDevice::DisplayDevice()
{
   mDeviceName = NULL;
}


//------------------------------------------------------------------------------
void DisplayDevice::init()
{
	smCurrentRes = Resolution( 0, 0, 0 );
	smIsFullScreen = false;
}


//------------------------------------------------------------------------------
bool DisplayDevice::prevRes()
{
   U32 resIndex;
   for ( resIndex = mResolutionList.size() - 1; resIndex > 0; resIndex-- )
   {
      if ( mResolutionList[resIndex].bpp == smCurrentRes.bpp 
        && mResolutionList[resIndex].w <= smCurrentRes.w 
        && mResolutionList[resIndex].h != smCurrentRes.h )
         break;
   }

   if ( mResolutionList[resIndex].bpp == smCurrentRes.bpp )
      return( Video::setResolution( mResolutionList[resIndex].w, mResolutionList[resIndex].h, mResolutionList[resIndex].bpp ) );

	return( false );	
}


//------------------------------------------------------------------------------
bool DisplayDevice::nextRes()
{
   U32 resIndex;
   for ( resIndex = 0; resIndex < mResolutionList.size() - 1; resIndex++ )
   {
      if ( mResolutionList[resIndex].bpp == smCurrentRes.bpp 
        && mResolutionList[resIndex].w >= smCurrentRes.w 
        && mResolutionList[resIndex].h != smCurrentRes.h )
         break;
   }

   if ( mResolutionList[resIndex].bpp == smCurrentRes.bpp )
      return( Video::setResolution( mResolutionList[resIndex].w, mResolutionList[resIndex].h, mResolutionList[resIndex].bpp ) );

	return( false );	
}


//------------------------------------------------------------------------------
// This function returns a string containing all of the available resolutions for this device
// in the format "<bit depth> <width> <height>", separated by tabs.
//
const char* DisplayDevice::getResolutionList()
{
   if (Con::getBoolVariable("$pref::Video::clipHigh", false))
		for (S32 i = mResolutionList.size()-1; i >= 0; --i)
			if (mResolutionList[i].w > 1152 || mResolutionList[i].h > 864)
				mResolutionList.erase(i);

	if (Con::getBoolVariable("$pref::Video::only16", false))
		for (S32 i = mResolutionList.size()-1; i >= 0; --i)
			if (mResolutionList[i].bpp == 32)
				mResolutionList.erase(i);
   
   U32 resCount = mResolutionList.size();
   if ( resCount > 0 )
   {
      char* tempBuffer = new char[resCount * 15];
      tempBuffer[0] = 0;
      for ( U32 i = 0; i < resCount; i++ )
      {
         char newString[15];
         dSprintf( newString, sizeof( newString ), "%d %d %d\t", mResolutionList[i].w, mResolutionList[i].h, mResolutionList[i].bpp );
         dStrcat( tempBuffer, newString );
      }
      tempBuffer[dStrlen( tempBuffer ) - 1] = 0;

      char* returnString = Con::getReturnBuffer( dStrlen( tempBuffer ) + 1 );
      dStrcpy( returnString, tempBuffer );
      delete [] tempBuffer;

      return returnString;
   }

   return NULL;
}
