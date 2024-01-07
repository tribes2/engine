//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "PlatformMacCarb/platformMacCarb.h"

void Platform::postQuitMessage(const U32 in_quitVal)
{
   platState.quit = true;
}

void Platform::debugBreak()
{
#pragma message("Platform::debugBreak NYI")
   //DebugBreak();
}

void Platform::forceShutdown(S32 returnValue)
{
#pragma message("Platform::forceShutdown NYI")
   //exit(returnValue);
}   
