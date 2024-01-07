//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "game/aiConnection.h"

IMPLEMENT_CONOBJECT( AIConnection );


//-----------------------------------------------------------------------------

AIConnection::AIConnection() {
   mAIControlled = true;
   mMove = NullMove;
}


//-----------------------------------------------------------------------------

void AIConnection::clearMoves( U32 )
{
   // Clear the pending move list. This connection generates moves
   // on the fly, so there are never any pending moves.
}

void AIConnection::setMove(Move* m)
{
   mMove = *m;
}

const Move& AIConnection::getMove()
{
   return mMove;
}   

/// Retrive the pending moves
/**
 * The GameConnection base class queues moves for delivery to the
 * controll object.  This function is normally used to retrieve the
 * queued moves recieved from the client.  The AI connection does not
 * have a connected client and simply generates moves on-the-fly
 * base on it's current state.
 */
void AIConnection::getMoveList( Move **lngMove, U32 *numMoves )
{
   *numMoves = 1;
   *lngMove = &mMove;
}


//-----------------------------------------------------------------------------
// Console functions & methods
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

static inline F32 moveClamp(F32 v)
{
   // Support function to convert/clamp the input into a move rotation
   // which only allows 0 -> M_2PI.
   F32 a = mClampF(v,-M_PI,M_PI);
   return (a < 0)? a + M_2PI: a;
}


//-----------------------------------------------------------------------------
/// Construct and connect an AI connection object
/**
 * Construct and registers a new AI connection. No control object
 * is set.
 */
ConsoleFunction(aiConnect, S32 , 2, 20, "aiConnect(val 0...n);")
{
   // Create the connection
   AIConnection *aiConnection = new AIConnection();
   aiConnection->registerObject();
   
   // Add the connection to the client group
   SimGroup *g = Sim::getClientGroup();
   g->addObject( aiConnection );

   // Prep the arguments for the console exec...
   // Make sure and leav args[1] empty.
   const char* args[21];
   args[0] = "onConnect";
   for (S32 i = 1; i < argc; i++)
      args[i + 1] = argv[i];

   // Execute the connect console function, this is the same 
   // onConnect function invoked for normal client connections
   Con::execute(aiConnection, argc + 1, args);
   return aiConnection->getId();
}


//-----------------------------------------------------------------------------
/// Console interface function to set the current move
/**
 */
ConsoleMethod(AIConnection,setMove,void,4, 4,"conn.setMove([x,y,z,yaw,pitch,roll],value);")
{
   AIConnection *ai = static_cast<AIConnection *>(object);
   Move move = ai->getMove();

   // Ok, a little slow for now, but this is just an example..
   if (!dStricmp(argv[2],"x"))
      move.x = mClampF(dAtof(argv[3]),-1,1);
      else
   if (!dStricmp(argv[2],"y"))
      move.y = mClampF(dAtof(argv[3]),-1,1);
      else
   if (!dStricmp(argv[2],"z"))
      move.z = mClampF(dAtof(argv[3]),-1,1);
      else
   if (!dStricmp(argv[2],"yaw"))
      move.yaw = moveClamp(dAtof(argv[3]));
      else
   if (!dStricmp(argv[2],"pitch"))
      move.pitch = moveClamp(dAtof(argv[3]));
      else
   if (!dStricmp(argv[2],"roll"))
      move.roll = moveClamp(dAtof(argv[3]));

   //
   ai->setMove(&move);
}

ConsoleMethod(AIConnection,getMove,F32,3, 3,"value conn.getMove([x,y,z,yaw,pitch,roll]);")
{
   AIConnection *ai = static_cast<AIConnection *>(object);
   const Move& move = ai->getMove();
   if (!dStricmp(argv[2],"x"))
      return move.x;
   if (!dStricmp(argv[2],"y"))
      return move.y;
   if (!dStricmp(argv[2],"z"))
      return move.z;
   if (!dStricmp(argv[2],"yaw"))
      return move.yaw;
   if (!dStricmp(argv[2],"pitch"))
      return move.pitch;
   if (!dStricmp(argv[2],"roll"))
      return move.roll;
   return 0;
}


//-----------------------------------------------------------------------------

ConsoleMethod(AIConnection,setFreeLook,void,3, 3,"conn.setFreeLook(bool);")
{
   AIConnection *ai = static_cast<AIConnection *>(object);
   Move move = ai->getMove();
   move.freeLook = dAtob(argv[2]);
   ai->setMove(&move);
}

ConsoleMethod(AIConnection,getFreeLook,bool,2, 2,"bool conn.getFreeLook();")
{
   AIConnection *ai = static_cast<AIConnection *>(object);
   return ai->getMove().freeLook;
}


//-----------------------------------------------------------------------------

ConsoleMethod(AIConnection,setTrigger,void,4, 4,"conn.setTrigger(trigger#,bool);")
{
   AIConnection *ai = static_cast<AIConnection *>(object);
   S32 idx = dAtoi(argv[2]);
   if (idx >= 0 && idx < MaxTriggerKeys)  {
      Move move = ai->getMove();
      move.trigger[idx] = dAtob(argv[3]);
      ai->setMove(&move);
   }
}

ConsoleMethod(AIConnection,getTrigger,bool,4, 4,"bool conn.getTrigger(trigger#);")
{
   AIConnection *ai = static_cast<AIConnection *>(object);
   S32 idx = dAtoi(argv[2]);
   if (idx >= 0 && idx < MaxTriggerKeys)
      return ai->getMove().trigger[idx];
   return false;
}


//-----------------------------------------------------------------------------

ConsoleMethod(AIConnection,getAddress,const char*,2, 2,"bool conn.getAddress();")
{
   // Override the netConnection method to return to indicate
   // this is an ai connection.
   return "ai:local";
}
