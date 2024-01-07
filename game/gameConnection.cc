//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "core/dnet.h"
#include "console/consoleTypes.h"
#include "console/simBase.h"
#include "game/gameConnection.h"
#include "game/shapeBase.h"
#include "core/bitStream.h"
#include "sim/pathManager.h"
#include "audio/audio.h"
#include "game/targetManager.h"
#include "game/netDispatch.h"
#include "game/game.h"
#include "scenegraph/sceneGraph.h"
#include "game/gameConnectionEvents.h"
#include "game/auth.h"

//----------------------------------------------------------------------------
#define MAX_VOICE_CHANNELS    3
#define MAX_MOVE_PACKET_SENDS 4

//----------------------------------------------------------------------------

const U32 targetCollisionMask =  (TerrainObjectType     |
                                  InteriorObjectType    |
                                  PlayerObjectType      |
                                  VehicleObjectType     |
                                  StaticShapeObjectType |
                                  ForceFieldObjectType  |
                                  TurretObjectType);

void GameConnection::sendTargetToServer(S32 targetId, Point3F pos)
{
   AssertFatal((targetId >= -1) && (targetId < TargetManager::MaxTargets), "GameConnection::sendTargetToServer: invalid target id");
   postNetEvent(new SetServerTargetEvent(targetId, pos));
}

bool GameConnection::sendLOSTarget()
{
   MatrixF transform;
   Point3F pos;
   
   if(ShapeBase *cam = getControlObject())
   {
      cam->getEyeTransform(&transform);
      transform.getColumn(3, &pos);
      VectorF vec;
      transform.mulV(VectorF(0,1000,0), &vec);
   
      RayInfo rayInfo;
      Point3F endPos = pos + vec;
      cam->disableCollision();
      if (gClientContainer.castRay(pos, endPos, targetCollisionMask, &rayInfo))
      {
         cam->enableCollision();
         GameBase *obj = dynamic_cast<GameBase *>(rayInfo.object);
         S32 target = -1;
         if(obj)
            target = obj->getTarget();
         sendTargetToServer(target, rayInfo.point);
         return true;
      }
      cam->enableCollision();
   }
   return false;
}


void GameConnection::setServerTarget(S32 targetId, Point3F targetPos)
{
   AssertFatal((targetId >= -1) && (targetId < TargetManager::MaxTargets), "GameConnection::setServerTarget: invalid target id");
   mTargetId = targetId;
   mTargetPos = targetPos;
}

void GameConnection::sendTargetTo(NetConnection *conn, bool assign)
{
   if((mTargetId < -1) || (mTargetId >= TargetManager::MaxTargets))
      return;

   if(!isServerConnection())
      conn->postNetEvent(new TargetToEvent(mTargetId, mTargetPos, assign));
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(GameConnection);
bool GameConnection::mFirstPerson = true;
S32 GameConnection::mLagThresholdMS = 0;
S32 GameConnection::smVoiceConnections[MaxClients];

//----------------------------------------------------------------------------
GameConnection::GameConnection(bool ghostFrom, bool ghostTo, bool sendEvents) 
   : NetConnection(ghostFrom, ghostTo, sendEvents)
{
   mControlObject = NULL;
   mLastMoveAck = 0;
   mControlStateSkipCount = 0;
   mLastClientMove = 0;
   mFirstMoveIndex = 0;
   mMoveCredit = MaxMoveCount;
   mDataBlockModifiedKey = 0;
   mMaxDataBlockModifiedKey = 0;
   mAuthInfo = NULL;
   mControlObjectModifyKey = 0;
   mAckedControlObjectModifyKey = 0;
   mRemoteControlObjectModifyKey = 0;
   mLastSentControlObjectModifyKey = 0;
   mMissionCRC = 0xffffffff;

   mDamageFlash = mWhiteOut = 0;
   mSelfLocked = false;
   mSelfHomed = false;
   mSeekerTracking = false;
   mSeekerMode = ShapeBase::NotLocked;

   mCameraPos = 0;
   mCameraSpeed = 10;
   
   mCameraFov = 90.f;
   mUpdateCameraFov = false;
      
   mAIControlled = false;

   // voice stream
   mWouldListenTo.setSize(MaxClients);  
   mWouldListenTo.set();

   mListeningTo.setSize(MaxClients); 
   mListeningTo.clear();

   mMaxVoicechannels = 0;
   mCurVoicechannels = 0;
   mVoiceDecodingMask = 0;
   mVoiceEncodingLevel = -1;
   mPinged = false;
   mJammed = false;
   mDisconnectReason[0] = 0;
   for(U32 i = 0; i < TargetManager::TargetFreeMaskSize; i++)
      mTargetVisibleMask[i] = 0;

   mSensorGroup = 0;
   mInCommanderMap = false;
   mLockAudioHandle = NULL_AUDIOHANDLE;
   mHomingAudioHandle = NULL_AUDIOHANDLE;
   mTargetLockedAudioHandle = NULL_AUDIOHANDLE;
   mTargetingAudioHandle = NULL_AUDIOHANDLE;
   
   mTargetId = -1;
   mTargetPos.set(0.f, 0.f, 0.f);

   //blackout vars
   mBlackOut = 0.0f;
	mBlackOutTimeMS = 0;
	mBlackOutStartTimeMS = 0;
	mFadeToBlack = false;
}

GameConnection::~GameConnection()
{
   if (mLockAudioHandle != NULL_AUDIOHANDLE)
      alxStop(mLockAudioHandle);
   if (mHomingAudioHandle != NULL_AUDIOHANDLE)
      alxStop(mHomingAudioHandle);
   if (mTargetLockedAudioHandle != NULL_AUDIOHANDLE)
      alxStop(mTargetLockedAudioHandle);
   if (mTargetingAudioHandle != NULL_AUDIOHANDLE)
      alxStop(mTargetingAudioHandle);
   mLockAudioHandle = NULL_AUDIOHANDLE;
   mHomingAudioHandle = NULL_AUDIOHANDLE;
   mTargetLockedAudioHandle = NULL_AUDIOHANDLE;
   mTargetingAudioHandle = NULL_AUDIOHANDLE;

   delete mAuthInfo;
}

void GameConnection::setAuthInfo(const AuthInfo *info)
{
   mAuthInfo = new AuthInfo;
   *mAuthInfo = *info;
}

const AuthInfo *GameConnection::getAuthInfo()
{
   return mAuthInfo;
}


//----------------------------------------------------------------------------

void GameConnection::setControlObject(ShapeBase *obj)
{
   if(obj == mControlObject)
      return;

   // if this is a connection to a client, update the control object modify key:
   if(!isServerConnection())
      mControlObjectModifyKey++;
   if(mControlObject)
      mControlObject->setControllingClient(0);
   if(obj)
   {
      if (ShapeBase* coo = obj->getControllingObject())
         coo->setControlObject(0);
      if (GameConnection *con = obj->getControllingClient())
         con->setControlObject(0);
      obj->setControllingClient(this);
   }
   mControlObject = obj;
   setScopeObject(mControlObject);

   // if this is a client then set the fov and active image
   if(isServerConnection() && mControlObject)
   {
      F32 fov = mControlObject->getDefaultCameraFov();
      GameSetCameraFov(fov);
   }
}

static S32 sChaseQueueSize = 0;
static MatrixF* sChaseQueue = 0;
static S32 sChaseQueueHead = 0;
static S32 sChaseQueueTail = 0;

bool GameConnection::getControlCameraTransform(F32 dt, MatrixF* mat)
{
   ShapeBase* obj = getControlObject();
   if(!obj)
      return false;
   
   ShapeBase* cObj = obj;
   while((cObj = cObj->getControlObject()) != 0)
   {
      if(cObj->useObjsEyePoint())
         obj = cObj;
   }
   
   if (dt) {
      if (mFirstPerson || obj->onlyFirstPerson()) {
         if (mCameraPos > 0)
            if ((mCameraPos -= mCameraSpeed * dt) <= 0)
               mCameraPos = 0;
      }
      else {
         if (mCameraPos < 1)
            if ((mCameraPos += mCameraSpeed * dt) > 1)
               mCameraPos = 1;
      }
   }
   if (!sChaseQueueSize || mFirstPerson || obj->onlyFirstPerson())
      obj->getCameraTransform(&mCameraPos,mat);
   else {
      MatrixF& hm = sChaseQueue[sChaseQueueHead];
      MatrixF& tm = sChaseQueue[sChaseQueueTail];
      obj->getCameraTransform(&mCameraPos,&hm);
      *mat = tm;
      if (dt) {
         if ((sChaseQueueHead += 1) >= sChaseQueueSize)
            sChaseQueueHead = 0;
         if (sChaseQueueHead == sChaseQueueTail)
            if ((sChaseQueueTail += 1) >= sChaseQueueSize)
               sChaseQueueTail = 0;
      }
   }
   return true;
}

// bool GameConnection::getControlCameraFov(F32 * fov)
// {
//    if(ShapeBase * obj = getControlObject())
//    {
//       ShapeBase* cObj = obj;
//       while((cObj = cObj->getControlObject()) != 0)
//       {
//          if(cObj->useObjsEyePoint())
//             obj = cObj;
//       }
//       *fov = obj->getCameraFov();
//       return(true);   
//    }
//    return(false);
// }

bool GameConnection::getControlCameraFov(F32 * fov)
{
   //find the last control object in the chain (client->player->turret->whatever...)
   ShapeBase *obj = getControlObject();
   ShapeBase *cObj = NULL;
   while (obj)
   {
      cObj = obj;
      obj = obj->getControlObject();
   }
   if (cObj)
   {
      *fov = cObj->getCameraFov();
      return(true);   
   }
   return(false);
}

bool GameConnection::isValidControlCameraFov(F32 fov)
{
   //find the last control object in the chain (client->player->turret->whatever...)
   ShapeBase *obj = getControlObject();
   ShapeBase *cObj = NULL;
   while (obj)
   {
      cObj = obj;
      obj = obj->getControlObject();
   }
   if (cObj)
      return (cObj->isValidCameraFov(fov));
   return(true);
}

bool GameConnection::setControlCameraFov(F32 fov)
{
   //find the last control object in the chain (client->player->turret->whatever...)
   ShapeBase *obj = getControlObject();
   ShapeBase *cObj = NULL;
   while (obj)
   {
      cObj = obj;
      obj = obj->getControlObject();
   }
   if (cObj)
   {
      // allow shapebase to clamp fov to its datablock values
      cObj->setCameraFov(mClampF(fov, MinCameraFov, MaxCameraFov));
      fov = cObj->getCameraFov();

      // server fov of client has 1degree resolution
      if(S32(fov) != S32(mCameraFov))
         mUpdateCameraFov = true;
   
      mCameraFov = fov;
      return(true);
   }
   return(false);
}

bool GameConnection::getControlCameraVelocity(Point3F *vel)
{
   if (ShapeBase* obj = getControlObject()) {
      *vel = obj->getVelocity();
      return true;
   }
   return false;
}   


void GameConnection::setObjectActiveImage(ShapeBase * obj, U32 slot)
{
   AssertFatal(obj, "GameConnection::setControlObjectActiveImage: invalid obj ptr");
   AssertFatal(slot < ShapeBase::MaxMountedImages, "GameConnection::setControlObjectActiveImage: image slot out of range");

   if(isServerConnection())
      return;
      
   S32 gi = getGhostIndex(obj);
   if(gi == -1)
      return;

   postNetEvent(new SetObjectActiveImageEvent(U32(gi), slot));
}

//----------------------------------------------------------------------------
bool GameConnection::onAdd()
{
   if (!Parent::onAdd())
      return false;

   if (!isServerConnection())
   {
      // set the voice id
      AssertFatal(MaxClients < 255, "Must increase voice id size");
      for(U32 i = 1; i <= MaxClients; i++)
         if(!smVoiceConnections[i])
         {
            smVoiceConnections[i] = getId();
            mVoiceID = i;
            break;   
         }
   }

   return true;
}

void GameConnection::onRemove()
{
   if(isNetworkConnection())
   {
      Con::printf("Issuing Disconnect packet.");
      
      // send a disconnect packet...
      U32 serverConnectSequence, clientConnectSequence;
      getSequences(&clientConnectSequence, &serverConnectSequence);
      
      BitStream *out = BitStream::getPacketStream();
      out->write(U8(Disconnect));
      out->write(serverConnectSequence);
      out->write(clientConnectSequence);
      out->writeString(mDisconnectReason);
   
      BitStream::sendPacketStream(getNetAddress());
   }
   if(!isServerConnection())
      Con::executef(this, 2, "onDrop", mDisconnectReason);

   if (mControlObject)
      mControlObject->setControllingClient(0);
   Parent::onRemove();

   // clear the voice id
   smVoiceConnections[mVoiceID] = 0;
   mVoiceID = 0;
}

void GameConnection::setDisconnectReason(const char *str)
{
   dStrncpy(mDisconnectReason, str, sizeof(mDisconnectReason) - 1);
   mDisconnectReason[sizeof(mDisconnectReason) - 1] = 0;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void GameConnection::handleRecordedBlock(U32 type, U32 size, void *data)
{
   switch(type)
   {
      case BlockTypeMove:
         pushMove(*((Move *) data));
         if(isRecording()) // put it back into the stream
            recordBlock(Sim::getCurrentTime(), type, size, data);
         break;
      default:
         Parent::handleRecordedBlock(type, size, data);
         break;
   }
}

void GameConnection::writeDemoStartBlock(ResizeBitStream *stream)
{
   // write all the data blocks to the stream:

   for(SimObjectId i = DataBlockObjectIdFirst; i <= DataBlockObjectIdLast; i++)
   {
      SimDataBlock *data;
      if(Sim::findObject(i, data))
      {
         stream->writeFlag(true);
         SimDataBlockEvent evt(data);
         evt.pack(this, stream);
         stream->validate();
      }
   }
   stream->writeFlag(false);
   stream->write(mFirstPerson);
   stream->write(mCameraPos);
   stream->write(mCameraSpeed);
   stream->write(mLastMoveAck);
   stream->write(mLastClientMove);
   stream->write(mFirstMoveIndex);

   stream->write(U32(mMoveList.size()));
   for(U32 j = 0; j < mMoveList.size(); j++)
      mMoveList[j].pack(stream);
   Parent::writeDemoStartBlock(stream);
   gTargetManager->writeDemoStartBlock(stream, this);
}

void GameConnection::readDemoStartBlock(BitStream *stream)
{
   while(stream->readFlag())
   {
      SimDataBlockEvent evt;
      evt.unpack(this, stream);
      evt.process(this);
   }
   stream->read(&mFirstPerson);
   stream->read(&mCameraPos);
   stream->read(&mCameraSpeed);
   stream->read(&mLastMoveAck);
   stream->read(&mLastClientMove);
   stream->read(&mFirstMoveIndex);
   
   U32 size;
   Move mv;
   stream->read(&size);
   mMoveList.clear();
   while(size--)
   {
      mv.unpack(stream);
      pushMove(mv);
   }
   Parent::readDemoStartBlock(stream);
   gTargetManager->readDemoStartBlock(stream, this);
}


void GameConnection::demoPlaybackComplete()
{
   static const char *demoPlaybackArgv[1] = { "demoPlaybackComplete" };
   Sim::postCurrentEvent(Sim::getRootGroup(), new SimConsoleEvent(1, demoPlaybackArgv, false));
   Parent::demoPlaybackComplete();
}


//----------------------------------------------------------------------------

void GameConnection::readPacket(BitStream *bstream)
{
   char stringBuf[256];
   stringBuf[0] = 0;
   bstream->setStringBuffer(stringBuf);

   clearCompression();
   if (isServerConnection())
   {
      mLastMoveAck = bstream->readInt(32);
      if (mLastMoveAck < mFirstMoveIndex)
         mLastMoveAck = mFirstMoveIndex;
      if(mLastMoveAck > mLastClientMove)
         mLastClientMove = mLastMoveAck;
      while(mFirstMoveIndex < mLastMoveAck)
      {
         AssertFatal(mMoveList.size(), "Popping off too many moves!");
         mMoveList.pop_front();
         mFirstMoveIndex++;
      }

      mDamageFlash = 0;
      mWhiteOut = 0;
      if(bstream->readFlag())
      {
         if(bstream->readFlag())
            mDamageFlash = bstream->readFloat(7);
         if(bstream->readFlag())
            mWhiteOut = bstream->readFloat(7) * 1.5;
      }
      if(bstream->readFlag())
      {
         mSelfLocked = bstream->readFlag();
         mSelfHomed = bstream->readFlag();
      }
      else
      {
         mSelfLocked = false;
         mSelfHomed = false;
      }
      if(bstream->readFlag())
      {
         mSeekerTracking = bstream->readFlag();
         mSeekerMode = bstream->readRangedU32(ShapeBase::NotLocked, ShapeBase::LockPosition);
         if(mSeekerMode == ShapeBase::LockObject)
         {
            if(bstream->readFlag())
               mSeekerObject = (ShapeBase *) resolveGhost(bstream->readRangedU32(0, MaxGhostCount-1));
            else
               mSeekerObject = NULL;
         }
         else if( mSeekerMode == ShapeBase::LockPosition )
         {
            bstream->read(&mTargetPos.x);
            bstream->read(&mTargetPos.y);
            bstream->read(&mTargetPos.z);
         }
         else
            mSeekerObject = NULL;
      }
      else
      {
         mSeekerTracking = false;
         mSeekerMode = ShapeBase::NotLocked;
         mSeekerObject = NULL;
      }
      bool pinged = bstream->readFlag();
      if(pinged != mPinged)
      {
         Con::executef(this, 2, "sensorPing", Con::getIntArg(pinged));
         mPinged = pinged;         
      }

      bool jammed = bstream->readFlag();
      if(jammed != mJammed)
      {
         Con::executef(this, 2, "sensorJammed", Con::getIntArg(jammed));
         mJammed = jammed;
      }
         
      if (bstream->readFlag())
      {
         if(bstream->readFlag())
         {
            // the control object is dirty...
            // so we get an update:
            mLastClientMove = mLastMoveAck;
            bool callScript = false;
            if(mControlObject.isNull())
               callScript = true;
               
            S32 gIndex = bstream->readInt(10);
            ShapeBase* obj = static_cast<ShapeBase*>(resolveGhost(gIndex));
            if (obj != mControlObject)
               setControlObject(obj);
            obj->readPacketData(this, bstream);
            
            if(callScript)
               Con::executef(this, 2, "initialControlSet");
         }
         else
         {
            // read out the compression point
            Point3F pos;
            bstream->read(&pos.x);
            bstream->read(&pos.y);
            bstream->read(&pos.z);
            setCompressionPoint(pos);
         }
      }

      while(bstream->readFlag())
      {
         U32 index = bstream->readInt(4);
         U32 mask = bstream->readInt(32);
         U32 start = index << 5;
         while(mask)
         {
            if(mask & 1)
            {
               TargetInfo *ct = gTargetManager->getClientTarget(start);
               ct->sensorFlags ^= TargetInfo::VisibleToSensor;
            }
            mask >>= 1;
            start++;
         }
      }

      // server forcing a fov change?
      if(bstream->readFlag())
      {
         S32 fov = bstream->readInt(8);
         setControlCameraFov(fov);
         
         // don't bother telling the server if we were able to set the fov   
         F32 setFov;
         if(getControlCameraFov(&setFov) && (S32(setFov) == fov))
            mUpdateCameraFov = false;

         // update the games fov info
         GameSetCameraFov(fov);
      }
   }
   else
   {
      bool fp = bstream->readFlag();
      if(fp)
         mCameraPos = 0;
      else
         mCameraPos = 1;

      if(bstream->readFlag())
      {
         // the client wants a new control object state
         U32 remoteKey = bstream->readInt(5);
         if(remoteKey != mRemoteControlObjectModifyKey)
         {
            mControlObjectModifyKey++;
            mRemoteControlObjectModifyKey = remoteKey;
         }
      }
      moveReadPacket(bstream);

      // check fov change.. 1degree granularity on server
      if(bstream->readFlag())
      {
         S32 fov = mClamp(bstream->readInt(8), S32(MinCameraFov), S32(MaxCameraFov));
         setControlCameraFov(fov);

         // may need to force client back to a valid fov
         F32 setFov;
         if(getControlCameraFov(&setFov) && (S32(setFov) == fov))
            mUpdateCameraFov = false;
      }
   }
   Parent::readPacket(bstream);
   clearCompression();
   bstream->setStringBuffer(NULL);
}

void GameConnection::writePacket(BitStream *bstream, PacketNotify *note)
{
   char stringBuf[256];
   clearCompression();
   stringBuf[0] = 0;
   bstream->setStringBuffer(stringBuf);

   GamePacketNotify *gnote = (GamePacketNotify *) note;
   gnote->controlObjectModifyKey = 0;

   U32 startPos = bstream->getCurPos();
   if (isServerConnection())
   {
      bstream->writeFlag(mCameraPos == 0);

      if(bstream->writeFlag(mControlObjectModifyKey != mAckedControlObjectModifyKey))
      {
         gnote->controlObjectModifyKey = mControlObjectModifyKey;
         mLastSentControlObjectModifyKey = mControlObjectModifyKey;
         bstream->writeInt(mControlObjectModifyKey, 5);
      }

      moveWritePacket(bstream);

      // camera fov changed? (server fov resolution is 1 degree)
      if(bstream->writeFlag(mUpdateCameraFov))
      {
         bstream->writeInt(mClamp(S32(mCameraFov), S32(MinCameraFov), S32(MaxCameraFov)), 8);
         mUpdateCameraFov = false;
      }
      DEBUG_LOG(("PKLOG %d CLIENTMOVES: %d", getId(), bstream->getCurPos() - startPos));
   }
   else
   {
      // The only time mMoveList will not be empty at this
      // point is during a change in control object.

      bstream->writeInt(mLastMoveAck - mMoveList.size(),32);
      TargetInfo *info = NULL;

      S32 gIndex = -1;

      // get the ghost index of the control object, and write out
      // all the damage flash, white out and missile tracking information

      if (!mControlObject.isNull())
      {
         gIndex = getGhostIndex(mControlObject);
         info = mControlObject->getTargetInfo();

         F32 flash = mControlObject->getDamageFlash();
         F32 whiteOut = mControlObject->getWhiteOut();
         if(bstream->writeFlag(flash != 0 || whiteOut != 0))
         {
            if(bstream->writeFlag(flash != 0))
               bstream->writeFloat(flash, 7);
            if(bstream->writeFlag(whiteOut != 0))
               bstream->writeFloat(whiteOut/1.5, 7);
         }
         S32 lockCount = mControlObject->getLockCount(), 
            homingCount = mControlObject->getHomingCount();

         // get the lock and homing counts off our mount as well..
         ShapeBase *mount = mControlObject->getObjectMount();
         if(mount)
         {
            lockCount += mount->getLockCount();
            homingCount += mount->getHomingCount();
         }
         if(bstream->writeFlag(lockCount | homingCount))
         {
            bstream->writeFlag(lockCount > 0);
            bstream->writeFlag(homingCount > 0);
         }
         ShapeBase *co = mControlObject->getControlObject();
         if(!co)
            co = mControlObject;

         bool tracking = co->isTracking();
         ShapeBase::LockMode mode = co->getLockMode();
         ShapeBase *target = co->getLockedTarget();

         if(bstream->writeFlag(tracking || mode != ShapeBase::NotLocked))
         {
            bstream->writeFlag(tracking);
            bstream->writeRangedU32(mode, ShapeBase::NotLocked, ShapeBase::LockPosition);
            if(mode == ShapeBase::LockObject)
            {
               S32 gi = -1;
               if(target)
                  gi = getGhostIndex(target);
               if(bstream->writeFlag(gi != -1))
                  bstream->writeRangedU32(U32(gi), 0, MaxGhostCount - 1);
            }
            else if( mode == ShapeBase::LockPosition )
            {
               Point3F targetPos = co->getLockedPosition();
               bstream->write(targetPos.x);
               bstream->write(targetPos.y);
               bstream->write(targetPos.z);
            }
         }
      }
      else
      {
         bstream->writeFlag(false);
         bstream->writeFlag(false);
         bstream->writeFlag(false);
      }
      // always write out sensor ping stuff...
      bstream->writeFlag(info ? info->sensorFlags & TargetInfo::SensorPinged : false);
      bstream->writeFlag(info ? info->sensorFlags & TargetInfo::SensorJammed : false);

      if (bstream->writeFlag(gIndex != -1))
      {
         // assume that the control object will write in a compression point
         if(bstream->writeFlag(mControlObjectModifyKey != mAckedControlObjectModifyKey || mControlStateSkipCount >= ControlStateSkipAmount))
         {
            bstream->writeInt(gIndex,10);
            // write out the control object - if it returns false, that means we have
            // to send it again soon.

            if(mControlObject->writePacketData(this, bstream))
            {
               gnote->controlObjectModifyKey = mControlObjectModifyKey;
               mLastSentControlObjectModifyKey = mControlObjectModifyKey;
               mControlStateSkipCount = 0;
            }
         }
         else
         {
            // we'll have to use the control object's position as the compression point
            // should make this lower res for better space usage:
            mControlStateSkipCount++;
            Point3F coPos = mControlObject->getPosition();
            bstream->write(coPos.x);
            bstream->write(coPos.y);
            bstream->write(coPos.z);
            setCompressionPoint(coPos);
         }
      }
      DEBUG_LOG(("PKLOG %d CONTROLOBJECTSTATE: %d", getId(), bstream->getCurPos() - startPos));
      startPos = bstream->getCurPos();
      // now write the visible target mask
      U32 *pingMask = gTargetManager->mSensorInfoArray[mSensorGroup].targetPingMask;

      for(U32 i = 0; i < TargetManager::TargetFreeMaskSize; i++)
      {
         gnote->xorVisibleMask[i] = pingMask[i] ^ mTargetVisibleMask[i];
         mTargetVisibleMask[i] = pingMask[i];
         if(gnote->xorVisibleMask[i])
         {
            bstream->writeFlag(true);
            bstream->writeInt(i, 4);
            bstream->writeInt(gnote->xorVisibleMask[i], 32);
         }
      }
      bstream->writeFlag(false);

      // server forcing client fov?
      gnote->cameraFov = -1;
      if(bstream->writeFlag(mUpdateCameraFov))
      {
         gnote->cameraFov = mClamp(S32(mCameraFov), S32(MinCameraFov), S32(MaxCameraFov));
         bstream->writeInt(gnote->cameraFov, 8);
         mUpdateCameraFov = false;
      }
      DEBUG_LOG(("PKLOG %d PINGCAMSTATE: %d", getId(), bstream->getCurPos() - startPos));
   }
   Parent::writePacket(bstream, note);
   clearCompression();
   bstream->setStringBuffer(NULL);
}

void GameConnection::updateLockTones()
{
   // we've locked our target
   if( (mSeekerMode == ShapeBase::LockObject) || (mSeekerMode == ShapeBase::LockPosition) )
   {
      Con::executef(this, 2, "onTargetLocked", "true");
   } 
   else
   {
      Con::executef(this, 2, "onTargetLocked", "false");
   }
   
   // hot targets in our view
   if( mSeekerTracking )
   {
      Con::executef(this, 2, "onTrackingTarget", "true");
   } 
   else 
   {
      Con::executef(this, 2, "onTrackingTarget", "false");
   }
}

void GameConnection::updateLockWarnings()
{
   // someone has a lock on us
   if( mSelfLocked )
   {
      if(!mSelfHomed)
         Con::executef(this, 2, "onLockWarning", "true");
   } 
   else
   {
      Con::executef(this, 2, "onLockWarning", "false");
   }
   
   // missile in the air, homing in on us
   if( mSelfHomed )
   {
      Con::executef(this, 2, "onHomeWarning", "true");
   } 
   else
   {
      Con::executef(this, 2, "onHomeWarning", "false");
   }
}

void GameConnection::detectLag()
{
   //see if we're lagging...
   S32 curTime = Sim::getCurrentTime();
   if (curTime - mLastPacketTime > mLagThresholdMS)
   {
      if (!mLagging)
      {
         mLagging = true;
         Con::executef(this, 2, "setLagIcon", "true");
      }
   }
   else if (mLagging)
   {
      mLagging = false;
      Con::executef(this, 2, "setLagIcon", "false");
   }
}

GameConnection::GamePacketNotify::GamePacketNotify()
{
   // need to fill in empty notifes for demo start block
   cameraFov = 0;
   controlObjectModifyKey = 0;
   for(U32 i = 0; i < TargetManager::TargetFreeMaskSize; i++)
      xorVisibleMask[i] = 0;
}

NetConnection::PacketNotify *GameConnection::allocNotify()
{
   return new GamePacketNotify;
}

void GameConnection::packetReceived(PacketNotify *note)
{
   //record the time so we can tell if we're lagging...
   mLastPacketTime = Sim::getCurrentTime();
   GamePacketNotify *gnote = (GamePacketNotify *) note;
   if(gnote->controlObjectModifyKey)
      mAckedControlObjectModifyKey = gnote->controlObjectModifyKey;

   Parent::packetReceived(note);
}

void GameConnection::packetDropped(PacketNotify *note)
{
   Parent::packetDropped(note);
   GamePacketNotify *gnote = (GamePacketNotify *) note;
   for(U32 i = 0; i < TargetManager::TargetFreeMaskSize; i++)
      mTargetVisibleMask[i] ^= gnote->xorVisibleMask[i];
   if(gnote->cameraFov != -1)
      mUpdateCameraFov = true;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

void GameConnection::play2D(const AudioProfile* profile)
{
   postNetEvent(new Sim2DAudioEvent(profile));
}

void GameConnection::play3D(const AudioProfile* profile, const MatrixF *transform)
{
   if (transform) {
      if (mControlObject) {
         // Only post the event if it's within audible range
         // of the control object.
         Point3F ear,pos;
         transform->getColumn(3,&pos);
         mControlObject->getTransform().getColumn(3,&ear);
         if ((ear - pos).len() < profile->mDescriptionObject->mDescription.mMaxDistance)
            postNetEvent(new Sim3DAudioEvent(profile,transform));
      }
      else
         postNetEvent(new Sim3DAudioEvent(profile,transform));
   }
   else
      play2D(profile);
}

//--------------------------------------
bool GameConnection::getListenState(U8 voiceId)
{
   return(mWouldListenTo.test(voiceId));
}

void GameConnection::listenTo(U8 voiceId, bool tf)
{
   if (tf)
      mWouldListenTo.set(voiceId);
   else
   {
      // terminate any existing stream
      GameConnection *itr = static_cast<GameConnection*>(getConnectionList());
      while (itr != NULL)
      {
         if (!itr->isServerConnection())
            itr->stopListening(mVoiceID);
         itr = static_cast<GameConnection*>(itr->getNext());
      }

      // refuse any future request to talk to me
      mWouldListenTo.clear(voiceId);
   }
}   

//--------------------------------------
void GameConnection::listenToAll()
{
   mWouldListenTo.set();
}   

//--------------------------------------
void GameConnection::listenToNone()
{
   mWouldListenTo.clear();
}   

bool GameConnection::canListen(GameConnection * con)
{
   AssertFatal(con, "GameConnection::canListen: invalid connection passed!");

#ifndef DEBUG
   // never allow a connection to listen to self
   if(con == this)
      return(false);
#endif

   // Can't listen if no channels are available:
   if ( mMaxVoicechannels == 0 )
      return( false );

   // make sure encoder/decoder's match
   if(con->getVoiceEncodingLevel() < 0)
      return(false);

   if(!(mVoiceDecodingMask & (1 << con->getVoiceEncodingLevel())))
      return(false);

   // check the listen mask for this group
   U32 listenMask = gTargetManager->getSensorGroupListenMask(getSensorGroup());
   return(listenMask & (1 << con->getSensorGroup()));
}

//--------------------------------------
bool GameConnection::willListen(U8 voiceId)
{
   AssertFatal(voiceId < MaxClients, "GameConnection::willListen: invalid voice id");
   if(voiceId >= MaxClients)
      return false;
   
   // would we consider listening to this person?
   if (mWouldListenTo.test(voiceId))
   {
      // can we listen to another person?
      if (mCurVoicechannels < mMaxVoicechannels)
      {
         if (mListeningTo.test(voiceId))
            stopListening(voiceId);     // just in case
         mListeningTo.set(voiceId);
         mCurVoicechannels++;
         return true;
      }
      else
      {
         // notify client that someone attempted to talk
         Con::evaluatef("commandToClient(%d, 'playerStartTalking', %d, 0);", getId(), smVoiceConnections[voiceId]);
      }
   }
   return false;
}   

//--------------------------------------
void GameConnection::stopListening(U8 voiceId)
{
   AssertFatal(voiceId < MaxClients, "GameConnection::stopListening: invalid voice id");
   if(voiceId >= MaxClients)
      return;
   
   // if this client is talking to me quit listening and terminiate voice stream
   if (mListeningTo.test(voiceId))
   {
      mCurVoicechannels--;
      AssertWarn(0, "GameConnection::stopListening: Need to terminate existing stream & send event");
   }
   else
   {
      if (mWouldListenTo.test(voiceId))
      {
         // notify client that someone quit talking 
         Con::evaluatef("commandToClient(%d, 'playerStoppedTalking', %d, 0);", getId(), smVoiceConnections[voiceId]);
      }
   }

   mListeningTo.clear(voiceId);
}   


void GameConnection::setVoiceChannels(S32 num)
{
   mMaxVoicechannels = mClamp(num, 0, MAX_VOICE_CHANNELS);  // 0-MAX_VOICE_CHANNELS active talking channels   
}   

//----------------------------------------------------------------------------
// Object scoping callback
static void commanderScopeCallback(SceneObject* obj, S32 value)
{
   const U32 scopeMask = (TerrainObjectType |
                          InteriorObjectType |
                          WaterObjectType |
                          PlayerObjectType |
                          VehicleObjectType |
                          StaticShapeObjectType);
                           
   GameConnection * con = reinterpret_cast<GameConnection*>(value);
   if(obj->isScopeable() && (obj->getType() & scopeMask))
      con->objectInScope(obj);
}

void GameConnection::doneScopingScene()
{
   // scope all in commander map
   if(mInCommanderMap)
      gServerContainer.findObjects(-1, commanderScopeCallback, reinterpret_cast<S32>(this));
   else
   {
      // scope any visible object from sensorVisibleSet
      SimSet * scopeSet = Sim::getScopeSensorVisibleSet();
      for(SimSet::iterator itr = scopeSet->begin(); itr != scopeSet->end(); itr++)
      {
         AssertFatal(((*itr)->getType() & ShapeBaseObjectType), "GameConnection: invalid object in scope set");

         ShapeBase * obj = static_cast<ShapeBase*>(*itr);
         if(obj->isHidden() || (obj->getTarget() == -1) || !obj->isScopeable() ||
            !gTargetManager->isTargetVisible(obj->getTarget(), getSensorGroup()))
            continue;

         objectInScope(obj);
      }
   }
}

//----------------------------------------------------------------------------
void GameConnection::setSensorGroup(U32 group)
{
   if(group == mSensorGroup)
      return;
   
   if(!isServerConnection())
   {
      gTargetManager->clientSensorGroupChanged(this, group);
      postNetEvent(new SetSensorGroupEvent(group));
   }
   mSensorGroup = group;
}

void GameConnection::resetVisibleMasks()
{
   for(U32 i = 0; i < TargetManager::TargetFreeMaskSize; i++)
      mTargetVisibleMask[i] = 0;
}

//----------------------------------------------------------------------------
//localconnection only blackout functions
void GameConnection::setBlackOut(bool fadeToBlack, S32 timeMS)
{
   mFadeToBlack = fadeToBlack;
   mBlackOutStartTimeMS = Sim::getCurrentTime();
   mBlackOutTimeMS = timeMS;

   //if timeMS <= 0 set the value instantly
   if (mBlackOutTimeMS <= 0)
      mBlackOut = (mFadeToBlack ? 1.0f : 0.0f);
}

F32 GameConnection::getBlackOut()
{
   S32 curTime = Sim::getCurrentTime();

   //see if we're in the middle of a black out
   if (curTime < mBlackOutStartTimeMS + mBlackOutTimeMS)
   {
      S32 elapsedTime = curTime - mBlackOutStartTimeMS;
      F32 timePercent = F32(elapsedTime) / F32(mBlackOutTimeMS);
      mBlackOut = (mFadeToBlack ? timePercent : 1.0f - timePercent);
   }
   else
      mBlackOut = (mFadeToBlack ? 1.0f : 0.0f);
   
   //return the blackout time
   return mBlackOut;
}
 
void GameConnection::handleGhostMessage(S32 message, U32 sequence, U32 ghostCount)
{
   if(isServerConnection())
   {
      if(message == GhostAlwaysStarting)
      {
// Authentication removed.
//         if(!clientAuth.valid)
//            Sim::postEvent( Sim::getRootGroup(), new QuitEvent(), Sim::getTargetTime() + 10000 );
      }
   }
   Parent::handleGhostMessage(message, sequence, ghostCount);
}

//----------------------------------------------------------------------------
static void cTransmitDataBlocks(SimObject *obj, S32, const char **argv)
{

   GameConnection *cptr = (GameConnection *) obj;
   
   cptr->setDataBlockSequence(dAtoi(argv[2]));
   SimDataBlockGroup *g = Sim::getDataBlockGroup();
   //g->sort();
   
   // check for the early out:
   U32 groupCount = g->size();
   if(!groupCount)
      return;
   S32 key = cptr->getDataBlockModifiedKey();
   // find the first one we haven't sent:
   U32 i;
   for(i = 0; i < groupCount; i++)
      if(( (SimDataBlock *)(*g)[i])->getModifiedKey() > key)
         break;
   
   if(i == groupCount)
   {
      Con::executef(cptr, 2, "dataBlocksDone", Con::getIntArg(cptr->getDataBlockSequence()));
      return;
   }
   cptr->setMaxDataBlockModifiedKey(key);
   
   U32 max = getMin(i + DataBlockQueueCount, groupCount);
   for(;i < max; i++)
   {
      SimDataBlock *data = (SimDataBlock *)(*g)[i];
      cptr->postNetEvent(new SimDataBlockEvent(data, i, groupCount, cptr->getDataBlockSequence()));
   }
}

static void cActivateGhosting(SimObject *obj, S32, const char **)
{
   NetConnection *cptr = (NetConnection *) obj;
   cptr->activateGhosting();
}

static void cResetGhosting(SimObject *obj, S32, const char **)
{
   NetConnection *cptr = (NetConnection *) obj;
   cptr->resetGhosting();
}

static bool cSetControlObject(SimObject *obj, S32, const char **argv)
{
   GameConnection *cptr = (GameConnection *) obj;
   ShapeBase *gb;
   if(!Sim::findObject(argv[2], gb))
      return false;

   cptr->setControlObject(gb);
   return true;
}

static int cGetControlObject(SimObject *obj, S32, const char **argv)
{
   argv;
   GameConnection *cptr = (GameConnection *) obj;
   SimObject* cp = cptr->getControlObject();
   return cp? cp->getId(): 0;
}

static bool cIsAIControlled(SimObject *obj, S32, const char **)
{
   GameConnection *cptr = (GameConnection *) obj;
   return cptr->isAIControlled();
}

static bool cPlay2D(SimObject *obj, S32, const char **argv)
{
   GameConnection *cptr = (GameConnection *) obj;
   AudioProfile *profile;
   if(!Sim::findObject(argv[2], profile))
      return false;
   cptr->play2D(profile);
   return true;
}

static bool cPlay3D(SimObject *obj, S32, const char **argv)
{
   GameConnection *cptr = (GameConnection *) obj;
   AudioProfile *profile;
   if(!Sim::findObject(argv[2], profile))
      return false;

   Point3F pos(0,0,0);
   AngAxisF aa;
   aa.axis.set(0,0,1);
   aa.angle = 0;
   dSscanf(argv[3],"%f %f %f %f %f %f %f",
           &pos.x,&pos.y,&pos.z,&aa.axis.x,&aa.axis.y,&aa.axis.z,&aa.angle);
   MatrixF mat;
   aa.setMatrix(&mat);
   mat.setColumn(3,pos);

   cptr->play3D(profile,&mat);
   return true;
}

static bool cChaseCam(SimObject *obj, S32, const char **argv)
{
   GameConnection *cptr = (GameConnection *) obj;
   S32 size = dAtoi(argv[2]);
   if (size != sChaseQueueSize) {
      delete [] sChaseQueue;
      sChaseQueue = 0;
      sChaseQueueSize = size;
      sChaseQueueHead = sChaseQueueTail = 0;
      if (size) {
         sChaseQueue = new MatrixF[size];
         return true;
      }
   }
   return false;
}

static void cSetSensorGroup(SimObject *obj, S32, const char **argv)
{
   GameConnection *cptr = (GameConnection *) obj;
   if(!cptr->isServerConnection())
      cptr->setSensorGroup(dAtoi(argv[2]));
}

static S32 cGetSensorGroup(SimObject *obj, S32, const char **)
{
   GameConnection *con = static_cast<GameConnection*>(obj);
   return(S32(con->getSensorGroup()));
}

static bool cSendLOSTarget(SimObject *obj, S32, const char **)
{
   GameConnection *cptr = (GameConnection *) obj;
   return cptr->sendLOSTarget();
}

static void cSendTargetToServer(SimObject * obj, S32, const char ** argv)
{
   S32 targetId = dAtoi(argv[2]);
   if((targetId < -1) || (targetId >= TargetManager::MaxTargets))
   {
      Con::errorf(ConsoleLogEntry::General, "GameConnection::sendTargetToServer: invalid target id [%s]", argv[2]);
      return;
   }

   GameConnection * con = static_cast<GameConnection*>(obj);
   Point3F pos(0,0,0);
   dSscanf(argv[3], "%f %f %f", &pos.x, &pos.y, &pos.z);
   con->sendTargetToServer(targetId, pos);
}

static void cSendTargetTo(SimObject *obj, S32, const char **argv)
{
   GameConnection *cptr = (GameConnection *) obj;
   NetConnection *dest;
   if(Sim::findObject(argv[2], dest))
      cptr->sendTargetTo(dest, dAtob(argv[3]));
}

static S32 cGetTargetId(SimObject *obj, S32, const char **)
{
   GameConnection * con = static_cast<GameConnection*>(obj);
   if(con->isServerConnection())
      return(-1);
   
   return(con->getTargetId());
}

static S32 cGetHomingCount(SimObject *obj, S32, const char **)
{
   GameConnection * con = static_cast<GameConnection*>(obj);
   if(con->isServerConnection())
      return(-1);
   
   return(con->getSelfHomed());
}

static const char * cGetTargetPos(SimObject *obj, S32, const char **)
{
   GameConnection * con = static_cast<GameConnection*>(obj);
   if(con->isServerConnection())
      return("");

   Point3F pos = con->getTargetPos();

   char * buf = Con::getReturnBuffer(128);
   dSprintf(buf, 128, "%f %f %f", pos.x, pos.y, pos.z);
   return(buf);
}

static void cSetTargetId(SimObject *obj, S32, const char **argv)
{
   GameConnection *con = static_cast<GameConnection*>(obj);

   S32 targetId = dAtoi(argv[2]);
   if((targetId < -1) || (targetId >= TargetManager::MaxTargets))
   {
      Con::errorf(ConsoleLogEntry::General, "GameConnection::cSetTargetId: invalid target id [%d]", targetId);
      return;
   }
   con->setTargetId(targetId);
}

static void cSetTargetPos(SimObject *obj, S32, const char **argv)
{
   GameConnection * con = static_cast<GameConnection*>(obj);
   Point3F pos(0,0,0);
   dSscanf(argv[2],"%f %f %f", &pos.x, &pos.y, &pos.z);
   con->setTargetPos(pos);
}

static bool cIsScopingCommanderMap(SimObject *obj, S32, const char **)
{
   GameConnection * con = static_cast<GameConnection *>(obj);
   return(con->isScopingCommanderMap());
}

static void cScopeCommanderMap(SimObject *obj, S32, const char ** argv)
{
   GameConnection * con = static_cast<GameConnection*>(obj);
   con->scopeCommanderMap(dAtob(argv[2]));
}

// Camera: (fov in degrees) --------------------------------------------------
static void cSetControlCameraFov(SimObject * obj, S32, const char ** argv)
{
   GameConnection * con = static_cast<GameConnection*>(obj);
   con->setControlCameraFov(dAtoi(argv[2]));
}

static F32 cGetControlCameraFov(SimObject * obj, S32, const char **)
{
   GameConnection * con = static_cast<GameConnection*>(obj);
   F32 fov = 0.f;
   if(!con->getControlCameraFov(&fov))
      return(0.f);
   return(fov);
}

//--------------------------------------------------------------------------
static bool cListenEnabled( SimObject* obj, S32, const char** )
{
   return( static_cast<GameConnection*>( obj )->listenEnabled() );
}

static bool cGetListenState(SimObject *obj, S32, const char **argv)
{
   GameConnection * me = (GameConnection *) obj;
   GameConnection * him = dynamic_cast<GameConnection*>(Sim::findObject(argv[2]));
   if(!him)
      return(false);

   return(me->getListenState(him->getVoiceID()));
}

static bool cCanListenTo(SimObject *obj, S32, const char **argv)
{
   GameConnection* me = (GameConnection*) obj;
   GameConnection* him = dynamic_cast<GameConnection*>(Sim::findObject(argv[2]));
   if ( !him )
      return( false );

   return( me->canListen( him ) );
}

static void cListenTo(SimObject *obj, S32, const char **argv)
{
   GameConnection * me = (GameConnection *) obj;
   GameConnection * him = dynamic_cast<GameConnection*>(Sim::findObject(argv[2]));
   if(!him)
      return;

   me->listenTo(him->getVoiceID(), dAtob(argv[3]));
}

static void cListenToAll(SimObject *obj, S32, const char **argv)
{
   argv;
   GameConnection *cptr = (GameConnection *) obj;
   cptr->listenToAll();
}

static void cListenToNone(SimObject *obj, S32, const char **argv)
{
   argv;
   GameConnection *cptr = (GameConnection *) obj;
   cptr->listenToNone();
}

static void cSetVoiceChannels(SimObject *obj, S32, const char **argv)
{
   GameConnection *cptr = (GameConnection *) obj;
   cptr->setVoiceChannels(dAtoi(argv[2]));
}

static void cSetVoiceDecodingMask(SimObject *obj, S32, const char ** argv)
{
   GameConnection *cptr = (GameConnection *) obj;
   cptr->setVoiceDecodingMask(dAtoi(argv[2]));
}

static void cSetVoiceEncodingLevel(SimObject *obj, S32, const char ** argv)
{
   GameConnection *cptr = (GameConnection *) obj;
   cptr->setVoiceEncodingLevel(dAtoi(argv[2]));
}

//--------------------------------------------------------------------------
static void cSetObjectActiveImage(SimObject * ptr, S32, const char ** argv)
{
   GameConnection * con = static_cast<GameConnection*>(ptr);
   if(con->isServerConnection())
      return;

   ShapeBase * obj = dynamic_cast<ShapeBase*>(Sim::findObject(argv[2]));
   if(!obj)
   {
      Con::errorf(ConsoleLogEntry::General, "GameConnection::cSetObjectActiveImage: invalid object %s", argv[2]);
      return;
   }

   U32 imageSlot = U32(dAtoi(argv[3]));
   if(imageSlot >= ShapeBase::MaxMountedImages)
   {
      Con::errorf(ConsoleLogEntry::General, "GameConnection::cSetControlObjectActiveImage: image slot out of range [%d]", imageSlot);
      return;
   }

   con->setObjectActiveImage(obj, imageSlot);
}

static void cSetBlackOut(SimObject * ptr, S32, const char ** argv)
{
   GameConnection * con = static_cast<GameConnection*>(ptr);
   con->setBlackOut(dAtob(argv[2]), dAtoi(argv[3]));
}

static void cSetMissionCRC(SimObject * obj, S32, const char ** argv)
{
   GameConnection * con = static_cast<GameConnection*>(obj);
   if(con->isServerConnection())
      return;

   con->postNetEvent(new SetMissionCRCEvent(dAtoi(argv[2])));
}

//--------------------------------------------------------------------------
void GameConnection::consoleInit()
{
   Con::addVariable("firstPerson", TypeBool, &mFirstPerson);
   Con::addVariable("pref::Net::lagThreshold", TypeS32, &mLagThresholdMS);

   Con::addCommand("GameConnection", "chaseCam", cChaseCam, "conn.chaseCam(size)", 3, 3);

   Con::addCommand("GameConnection", "setControlCameraFov", cSetControlCameraFov, "conn.setControlCameraFov(fov)", 3, 3);
   Con::addCommand("GameConnection", "getControlCameraFov", cGetControlCameraFov, "conn.getControlCameraFov()", 2, 2);

   Con::addCommand("GameConnection", "setSensorGroup", cSetSensorGroup, "conn.setSensorGroup(groupId)", 3, 3);
   Con::addCommand("GameConnection", "getSensorGroup", cGetSensorGroup, "conn.getSensorGroup()", 2, 2);
   Con::addCommand("GameConnection", "transmitDataBlocks", cTransmitDataBlocks, "conn.transmitDataBlocks(seq)", 3, 3);
   Con::addCommand("GameConnection", "activateGhosting", cActivateGhosting, "conn.activateGhosting()", 2, 2);
   Con::addCommand("GameConnection", "resetGhosting", cResetGhosting, "conn.resetGhosting()", 2, 2);
   Con::addCommand("GameConnection", "setControlObject", cSetControlObject, "conn.setControlObject(%obj)", 3, 3);
   Con::addCommand("GameConnection", "getControlObject", cGetControlObject, "conn.getControlObject()", 2, 2);
   Con::addCommand("GameConnection", "isAIControlled", cIsAIControlled, "conn.isAIControlled()", 2, 2);
   Con::addCommand("GameConnection", "setObjectActiveImage", cSetObjectActiveImage, "conn.setObjectActiveImage(obj, imageSlot)", 4, 4);

   Con::addCommand("GameConnection", "play2D", cPlay2D, "conn.play2D(AudioProfile)", 3, 3);
   Con::addCommand("GameConnection", "play3D", cPlay3D, "conn.play3D(AudioProfile,Transform)", 4, 4);

   Con::addCommand("GameConnection", "sendLOSTarget", cSendLOSTarget, "conn.sendLOSTarget()", 2, 2);
   Con::addCommand("GameConnection", "sendTargetToServer", cSendTargetToServer, "conn.sendTargetToServer(id, pos)", 4, 4);
   Con::addCommand("GameConnection", "sendTargetTo", cSendTargetTo, "conn.sendTargetTo(conn, assign)", 4, 4);
   Con::addCommand("GameConnection", "getTargetId", cGetTargetId, "conn.getTargetId()", 2, 2);
   Con::addCommand("GameConnection", "getTargetPos", cGetTargetPos, "conn.getTargetPos()", 2, 2);
   Con::addCommand("GameConnection", "setTargetId", cSetTargetId, "conn.setTargetId(targetId)", 3, 3);
   Con::addCommand("GameConnection", "setTargetPos", cSetTargetPos, "conn.setTargetPos(Point3F)", 3, 3);
   Con::addCommand("GameConnection", "getHomingCount", cGetHomingCount, "conn.getHomingCount()", 2, 2);

   Con::addCommand("GameConnection", "isScopingCommanderMap", cIsScopingCommanderMap, "conn.isScopingCommanderMap()", 2, 2);
   Con::addCommand("GameConnection", "scopeCommanderMap", cScopeCommanderMap, "conn.scopeCommanderMap(bool)", 3, 3);

   Con::addCommand("GameConnection", "listenEnabled", cListenEnabled, "conn.listenEnabled()", 2, 2);
   Con::addCommand("GameConnection", "getListenState", cGetListenState, "conn.getListenState(clientId)", 3, 3);
   Con::addCommand("GameConnection", "canListenTo", cCanListenTo, "conn.canListen(clientId)", 3, 3);
   Con::addCommand("GameConnection", "listenTo", cListenTo, "conn.listenTo(clientId, true|false)", 4, 4);
   Con::addCommand("GameConnection", "listenToAll", cListenToAll, "conn.listenToAll()", 2, 2);
   Con::addCommand("GameConnection", "listenToNone", cListenToNone, "conn.listenToNone()", 2, 2);
   Con::addCommand("GameConnection", "setVoiceChannels", cSetVoiceChannels, "conn.setVoiceChannels(0-3)", 3, 3);
   Con::addCommand("GameConnection", "setVoiceDecodingMask", cSetVoiceDecodingMask, "conn.setVoiceDecodingMask(mask)", 3, 3);
   Con::addCommand("GameConnection", "setVoiceEncodingLevel", cSetVoiceEncodingLevel, "conn.setVoiceEncodingLevel(codecLevel)", 3, 3);
   
   Con::addVariable("specialFog", TypeBool, &SceneGraph::useSpecial);
   Con::addCommand("GameConnection", "setBlackOut", cSetBlackOut, "conn.setBlackOut(fadeTOBlackBool, timeMS)", 4, 4);
   Con::addCommand("GameConnection", "setMissionCRC", cSetMissionCRC, "conn.setMissionCRC(crc)", 3, 3);
}

