//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "PlatformWin32/platformWin32.h"

void Platform::postQuitMessage(const U32 in_quitVal)
{
   PostQuitMessage(in_quitVal);
}

void Platform::debugBreak()
{
   DebugBreak();
}

void Platform::forceShutdown(S32 returnValue)
{
   ExitProcess(returnValue);
}   
