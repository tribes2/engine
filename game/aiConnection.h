//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _AICONNECTION_H_
#define _AICONNECTION_H_

#ifndef _GAMECONNECTION_H_
#include "game/gameConnection.h"
#endif
#ifndef _MOVEMANAGE_H_
#include "game/moveManager.h"
#endif

//-----------------------------------------------------------------------------

class AIConnection : public GameConnection 
{
   typedef GameConnection Parent;

protected:
   Move mMove;

public:
   AIConnection();
   DECLARE_CONOBJECT( AIConnection );

   // Interface
   const Move& getMove();
   void setMove(Move *m);

   // GameConnection overrides
   void clearMoves(U32 n); 
   virtual void getMoveList(Move **,U32 *numMoves);
};


#endif
