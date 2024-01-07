//-----------------------------------------------------------------------------
// Torque Game Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "platformX86UNIX/x86UNIXUtils.h"

#include <unistd.h>
#include <termios.h>
#include <stdio.h>

UnixUtils *UUtils = NULL;
UnixUtils utils; 

UnixUtils::UnixUtils()
{
   UUtils = this;
}

bool UnixUtils::inBackground()
{
   int terminalGroupId = tcgetpgrp(fileno(stdin));
   int myPid = getpid();
   if (terminalGroupId != myPid)
      return true;
   else
      return false;
}
