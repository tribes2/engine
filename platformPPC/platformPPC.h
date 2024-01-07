//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _PLATFORMPPC_H_
#define _PLATFORMPPC_H_


//#if defined(__MWERKS__)
//#  include <ansi_prefix.win32.h>
//#else
//#  include <stdio.h>
//#  include <string.h>
//#endif

#include <macwindows.h>
#include <Files.h>
#include <agl.h>
#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif
#ifndef _PPCUTILS_H_
#include "PlatformPPC/ppcUtils.h"
#endif

struct PPCPlatState
{
    GDHandle    hDisplay;
	WindowPtr	appWindow;
	char        appWindowTitle[255];
	bool quit;
	
	AGLPixelFormat fmt;
	AGLContext     ctx;
	
	S32 volRefNum;	// current working volume/drive reference number
	S32 dirID;		// current working directory id

//   FILE *log_fp;
//   HINSTANCE hinstOpenGL;
//   HINSTANCE hinstGLU;
//   HWND appWindow;
//   HDC appDC;
//   HINSTANCE appInstance;
//   HGLRC hGLRC;

   S32 desktopBitsPixel;
   S32 desktopWidth;
   S32 desktopHeight;
   U32 currentTime;
      
   PPCPlatState();
};

U32 GetMilliseconds();

extern PPCPlatState ppcState;

U8* str2p(const char *str);
U8* str2p(const char *str, U8 *dst_p);

char* p2str(U8 *p);
char* p2str(U8 *p, char *dst_str);


#endif //_PLATFORMPPC_H_
