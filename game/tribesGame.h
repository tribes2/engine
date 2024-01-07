//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _TRIBESGAME_H_
#define _TRIBESGAME_H_

#ifndef _GAMEINTERFACE_H_
#include "platform/gameInterface.h"
#endif

class TribesGame : public GameInterface
{
public:
   void textureKill();
   void textureResurrect();
   void refreshWindow();

   S32 main(S32 argc, const char **argv);
   
   void processPacketReceiveEvent(PacketReceiveEvent *event);
   void processMouseMoveEvent(MouseMoveEvent *event);
   void processInputEvent(InputEvent *event);
   void processQuitEvent();
   void processTimeEvent(TimeEvent *event);
   void processConsoleEvent(ConsoleEvent *event);
   void processConnectedAcceptEvent(ConnectedAcceptEvent *event);
   void processConnectedReceiveEvent(ConnectedReceiveEvent *event);
   void processConnectedNotifyEvent(ConnectedNotifyEvent *event);
};

#endif
