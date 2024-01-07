//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------
#include "game/aiPlayer.h"
#include "core/realcomp.h"
#include "math/mMatrix.h"
#include "game/player.h"
#include "game/moveManager.h"

IMPLEMENT_CONOBJECT( AIPlayer );

/**
 * Constructor
 */
AIPlayer::AIPlayer() {
   mMoveMode = ModeStop;
   mMoveDestination.set( 0.0f, 0.0f, 0.0f );
   mAimLocation.set( 0.0f, 0.0f, 0.0f );
   mMoveSpeed = 0.0f;
   mMoveTolerance = 0.25f;

   // Clear the triggers
   for( int i = 0; i < MaxTriggerKeys; i++ )
      mTriggers[i] = false;
 
   mAimToDestination = true;

   mRotation.set( 0.0f, 0.0f, 0.0f );
   mLocation.set( 0.0f, 0.0f, 0.0f );
   mPlayer = NULL;
}

/**
 * Sets the object the bot is targeting
 *
 * @param targetObject The object to target
 */
void AIPlayer::setTargetObject( ShapeBase *targetObject ) {   

   if ( !targetObject || !bool( mTargetObject ) || targetObject->getId() != mTargetObject->getId() )
      mTargetInSight = false;
   
   mTargetObject = targetObject;
}

/**
 * Returns the target object
 *
 * @return Object bot is targeting
 */
S32 AIPlayer::getTargetObject() {
   if( bool( mTargetObject ) )
      return mTargetObject->getId();
   else
      return -1;
}

/**
 * Sets the speed at which this AI moves
 *
 * @param speed Speed to move, default player was 10
 */
void AIPlayer::setMoveSpeed( F32 speed ) {
   if( speed <= 0.0f )
      mMoveSpeed = 0.0f;
   else
      mMoveSpeed = getMin( 1.0f, speed );
}

/**
 * Sets the movement mode for this AI
 *
 * @param mode Movement mode, see enum
 */
void AIPlayer::setMoveMode( S32 mode ) {
   if( mode < 0 || mode >= ModeCount )
      mode = 0;

   mMoveMode = mode;
}

/**
 * Sets how far away from the move location is considered
 * "on target"
 * 
 * @param tolerance Movement tolerance for error
 */
void AIPlayer::setMoveTolerance( F32 tolerance ) {
   mMoveTolerance = getMax( 0.1f, tolerance );
}

/**
 * Sets the location for the bot to run to
 *
 * @param location Point to run to
 */
void AIPlayer::setMoveDestination( const Point3F &location ) {
   // Ok, here's the story...we're going to aim where we are going UNLESS told otherwise
   if( mAimToDestination ) {
      mAimLocation = location;
      mAimLocation.z = 0.0f;
   }

   mMoveDestination = location;

   // TEST CODE
   RayInfo dummy;

   if( mPlayer ) {
      if( !mPlayer->getContainer()->castRay( mLocation, location, InteriorObjectType | 
                                                StaticShapeObjectType | StaticObjectType |
                                                TerrainObjectType, &dummy ) )
         Con::printf( "I can see the target." );
      else
         Con::printf( "I can't see my target!! AAAAHHHHH!" );
   }
}

/**
 * Sets the location for the bot to aim at
 * 
 * @param location Point to aim at
 */
void AIPlayer::setAimLocation( const Point3F &location ) {
   mAimLocation = location;
   mAimToDestination = false;
}

/**
 * Clears the aim location and sets it to the bot's
 * current destination so he looks where he's going
 */
void AIPlayer::clearAim() {
   mAimLocation = Point3F( 0.0f, 0.0f, 0.0f );
   mAimToDestination = true;
}

/**
 * This method gets the move list for an object, in the case
 * of the AI, it actually calculates the moves, and then
 * sends them down the pipe.
 *
 * @param movePtr Pointer to move the move list into
 * @param numMoves Number of moves in the move list
 */
void AIPlayer::getMoveList( Move **movePtr,U32 *numMoves ) {
   //initialize the move structure and return pointers
   mMove = NullMove;
   *movePtr = &mMove;
   *numMoves = 1;

   // Check if we got a player
   mPlayer = NULL;
   mPlayer = dynamic_cast<Player *>( getControlObject() );

   // We got a something controling us?
   if( !mPlayer )
      return;

   // If system is disabled, don't process
   if ( !gAISystemEnabled )
      return;
   
   // What is The Matrix?
   MatrixF moveMatrix;
   moveMatrix.set( EulerF( 0, 0, 0 ) );
   moveMatrix.setColumn( 3, Point3F( 0, 0, 0 ) );
   moveMatrix.transpose();
      
   // Position / rotation variables
   F32 curYaw, curPitch;
   F32 newYaw, newPitch;
   F32 xDiff, yDiff, zDiff;

   

   // Check if we are dead, if so, throw the script callback -- PUT THIS IN AIPLAYER
   //if( !dStricmp( mPlayer->getStateName(), "dead" ) ) {
      // TO-DO: Script callback
   //   return;
   //}

   F32 moveSpeed = mMoveSpeed;

   switch( mMoveMode ) {

   case ModeStop:
      return;     // Stop means no action, peroid
      break;

   case ModeIdle:
      // TO-DO: Insert callback for scripted idle stuff
      break;

   case ModeWalk:
      moveSpeed /= 2.0f; // Walking speed is half running speed
                         // Fall through to run
   case ModeRun:
   
      // Get my location
      MatrixF const& myTransform = mPlayer->getTransform();
      myTransform.getColumn( 3, &mLocation );
   
      // Set rotation variables
      mRotation = mPlayer->getRotation();
      Point3F headRotation = mPlayer->getHeadRotation();
      curYaw = mRotation.z;
      curPitch = headRotation.x;
      xDiff = mAimLocation.x - mLocation.x;
      yDiff = mAimLocation.y - mLocation.y;
   
      // first do Yaw
      if( !isZero( xDiff ) || !isZero( yDiff ) ) {
         // use the cur yaw between -Pi and Pi
         while( curYaw > M_2PI )
            curYaw -= M_2PI;
         while( curYaw < -M_2PI )
            curYaw += M_2PI;
      
         // find the new yaw
         newYaw = mAtan( xDiff, yDiff );
      
         // find the yaw diff 
         F32 yawDiff = newYaw - curYaw;
      
         // make it between 0 and 2PI
         if( yawDiff < 0.0f )
            yawDiff += M_2PI;
         else if( yawDiff >= M_2PI )
            yawDiff -= M_2PI;
      
         // now make sure we take the short way around the circle
         if( yawDiff > M_PI )
            yawDiff -= M_2PI;
         else if( yawDiff < -M_PI )
            yawDiff += M_2PI;
      
         mMove.yaw = yawDiff;
      
         // set up the movement matrix
         moveMatrix.set( EulerF( 0, 0, newYaw ) );
      }
      else
         moveMatrix.set( EulerF( 0, 0, curYaw ) );
   
      // next do pitch
      F32 horzDist = Point2F( mAimLocation.x, mAimLocation.y ).len();

      if( !isZero( horzDist ) ) {
         //we shoot from the gun, not the eye...
         F32 vertDist = mAimLocation.z;
      
         newPitch = mAtan( horzDist, vertDist ) - ( M_PI / 2.0f );
      
         F32 pitchDiff = newPitch - curPitch;
         mMove.pitch = pitchDiff;
      }
   
      // finally, mMove towards mMoveDestination
      xDiff = mMoveDestination.x - mLocation.x;
      yDiff = mMoveDestination.y - mLocation.y;

      // Check if we should mMove, or if we are 'close enough'
      if( ( ( mFabs( xDiff ) > mMoveTolerance ) || 
            ( mFabs( yDiff ) > mMoveTolerance ) ) && ( !isZero( mMoveSpeed ) ) )
      {
         if( isZero( xDiff ) )
            mMove.y = ( mLocation.y > mMoveDestination.y ? -moveSpeed : moveSpeed );
         else if( isZero( yDiff ) )
            mMove.x = ( mLocation.x > mMoveDestination.x ? -moveSpeed : moveSpeed );
         else if( mFabs( xDiff ) > mFabs( yDiff ) ) {
            F32 value = mFabs( yDiff / xDiff ) * mMoveSpeed;
            mMove.y = ( mLocation.y > mMoveDestination.y ? -value : value );
            mMove.x = ( mLocation.x > mMoveDestination.x ? -moveSpeed : moveSpeed );
         }
         else {
            F32 value = mFabs( xDiff / yDiff ) * mMoveSpeed;
            mMove.x = ( mLocation.x > mMoveDestination.x ? -value : value );
            mMove.y = ( mLocation.y > mMoveDestination.y ? -moveSpeed : moveSpeed );
         }
      
         //now multiply the mMove vector by the transpose of the object rotation matrix
         moveMatrix.transpose();
         Point3F newMove;
         moveMatrix.mulP( Point3F( mMove.x, mMove.y, 0 ), &newMove );
      
         //and sub the result back in the mMove structure
         mMove.x = newMove.x;
         mMove.y = newMove.y;
      }
      else {
         // Ok, we are close enough
         setMoveMode( ModeStop );
         Con::printf( "I'm close enough to my destination to stop." );
      }
      break;
   }
   
   // Copy over the trigger status
   for( int i = 0; i < MaxTriggerKeys; i++ ) {
      mMove.trigger[i] = mTriggers[i];
      mTriggers[i] = false;
   }
}

/**
 * This method is just called to stop the bots from running amuck
 * while the mission cycles
 */
void AIPlayer::missionCycleCleanup() {
   setMoveMode( ModeStop );
}

/**
 * Sets the move speed for an AI object, remember, walk is 1/2 run
 */
static void cAISetMoveSpeed( SimObject *obj, S32, const char** argv ) {
   AIPlayer *ai = static_cast<AIPlayer *>( obj );
   ai->setMoveSpeed( dAtoi( argv[2] ) );
}

/**
 * Stops all AI movement, halt!
 */
static void cAIStop( SimObject *obj, S32, const char** ) {
   AIPlayer *ai = static_cast<AIPlayer *>( obj );
   ai->setMoveMode( AIPlayer::ModeStop );
}

/**
 * Tells the AI to aim at the location provided
 */
static void cAIAimAtLocation( SimObject *obj, S32 argc, const char** argv ) {
   AIPlayer *ai = static_cast<AIPlayer *>( obj );
   Point3F v( 0.0f,0.0f,0.0f );
   dSscanf( argv[2], "%f %f %f", &v.x, &v.y, &v.z );

   ai->setAimLocation( v );
}

/**
 * Returns the point the AI is aiming at
 */
static const char *cAIGetAimLocation( SimObject *obj, S32, const char** ) {
   AIPlayer *ai = static_cast<AIPlayer *>( obj );
   Point3F aimPoint = ai->getAimLocation();

   char* returnBuffer = Con::getReturnBuffer( 256 );
   dSprintf( returnBuffer, 256, "%f %f %f", aimPoint.x, aimPoint.y, aimPoint.z );

   return returnBuffer;
}

/**
 * Sets the bots target object
 */
static void cAISetTargetObject( SimObject *obj, S32 argc, const char **argv ) {
   AIPlayer *ai = static_cast<AIPlayer *>( obj );
   
   // Find the target
   ShapeBase *targetObject;
   if( Sim::findObject( argv[2], targetObject ) )
      ai->setTargetObject( targetObject );
   else
      ai->setTargetObject( NULL );
}

/**
 * Gets the object the AI is targeting
 */
static const char *cAIGetTargetObject( SimObject *obj, S32, const char ** ) {
   AIPlayer *ai = static_cast<AIPlayer *>( obj );
   S32 targetId = ai->getTargetObject();
   char* returnBuffer = Con::getReturnBuffer( 256 );
   dSprintf( returnBuffer, 256, "%d", targetId );

   return returnBuffer;
}

/**
 * Checks to see if the target is in sight
 */
static bool cAITargetInSight( SimObject *obj, S32, const char ** ) {
   AIPlayer *ai = static_cast<AIPlayer*>( obj );
   return ai->targetInSight();
}

/**
 * Tells the bot the mission is cycling
 */
static void cAIMissionCycleCleanup( SimObject *obj, S32, const char ** ) {
   AIPlayer *ai = static_cast<AIPlayer*>( obj );
   ai->missionCycleCleanup();
}

/**
 * Tells the AI to move forward 100 units...TEST FXN
 */
static void cAIMoveForward( SimObject *obj, S32, const char ** ) {
   
   AIPlayer *ai = static_cast<AIPlayer *>( obj );
   ShapeBase *player = ai->getControlObject();
   Point3F location;
   MatrixF const &myTransform = player->getTransform();
   myTransform.getColumn( 3, &location );

   location.y += 100.0f;
   
   ai->setMoveDestination( location );
} // *** /TEST FXN

/**
 * Sets the AI to walk mode
 */
static void cAIWalk( SimObject *obj, S32, const char ** ) {
   AIPlayer *ai = static_cast<AIPlayer *>( obj );
   ai->setMoveMode( AIPlayer::ModeWalk );
}

/**
 * Sets the AI to run mode
 */
static void cAIRun( SimObject *obj, S32, const char ** ) {
   AIPlayer *ai = static_cast<AIPlayer *>( obj );
   ai->setMoveMode( AIPlayer::ModeRun );
}


/**
 * Console init function
 */
void AIPlayer::consoleInit() {
   // TEST FXN
   Con::addCommand( "AIPlayer", "moveForward", cAIMoveForward, "ai.moveForward()", 2, 2 );

   Con::addCommand( "AIPlayer", "walk", cAIWalk, "ai.walk()", 2, 2 );
   Con::addCommand( "AIPlayer", "run", cAIRun, "ai.run()", 2, 2 );
   Con::addCommand( "AIPlayer", "stop", cAIStop, "ai.stop()", 2, 2 );
   Con::addCommand( "AIPlayer", "setMoveSpeed", cAISetMoveSpeed, "ai.setMoveSpeed( float )", 3, 3 );
   
   Con::addCommand( "AIPlayer", "setTargetObject", cAISetTargetObject, "ai.setTargetObject( object )", 3, 4 );
   Con::addCommand( "AIPlayer", "getTargetObject", cAIGetTargetObject, "ai.getTargetObject()", 2, 2 );
   Con::addCommand( "AIPlayer", "targetInSight", cAITargetInSight, "ai.targetInSight()", 2, 2 );
   Con::addCommand( "AIPlayer", "aimAt", cAIAimAtLocation, "ai.aimAt( point )", 3, 3 );
   Con::addCommand( "AIPlayer", "getAimLocation", cAIGetAimLocation, "ai.getAimLocation()", 2, 2 );
}