//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _V12GAME_H_
#define _V12GAME_H_

#ifndef _GAMEINTERFACE_H_
#include "platform/gameInterface.h"
#endif

class V12Game : public GameInterface
{
public:
   void textureKill();
   void textureResurrect();
   void refreshWindow();

   int main(int argc, const char **argv);
   
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
