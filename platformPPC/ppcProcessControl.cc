//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "PlatformPPC/platformPPC.h"

void Platform::postQuitMessage(const U32 in_quitVal)
{
   ppcState.quit = true;
}

void Platform::debugBreak()
{
   //DebugBreak();
}

void Platform::forceShutdown(S32 returnValue)
{
	//exit(returnValue);
}   
