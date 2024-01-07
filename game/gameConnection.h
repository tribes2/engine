//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GAMECONNECTION_H_
#define _GAMECONNECTION_H_

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif
#ifndef _NETCONNECTION_H_
#include "sim/netConnection.h"
#endif
#ifndef _MOVEMANAGER_H_
#include "game/moveManager.h"
#endif
#ifndef _BITVECTOR_H_
#include "core/bitVector.h"
#endif
#ifndef _TARGETMANAGER_H_
#include "game/targetManager.h"
#endif

enum {
   MaxClients = 126,
   DataBlockQueueCount = 16,
   ControlStateSkipAmount = 16,
};

class AudioProfile;
class MatrixF;
class MatrixF;
class Point3F;
class MoveManager;
class ShapeBase;
struct Move;
struct AuthInfo;

class GameConnection : public NetConnection
{
   typedef NetConnection Parent;

   enum Constants {
      MoveCountBits = 5,
      // MaxMoveCount should not exceed the MoveManager's
      // own maximum (MaxMoveQueueSize)
      MaxMoveCount = 30,
   };
   typedef Vector<Move> MoveList;
   
   SimObjectPtr<ShapeBase> mControlObject;
   bool mInCommanderMap;
   bool mPinged;
   bool mJammed;
   U32 mDataBlockSequence;
   char mDisconnectReason[256];
   
   U32  mMissionCRC;             // crc of the current mission file from the server

private:
   S32 mDataBlockModifiedKey;
   S32 mMaxDataBlockModifiedKey;

   // Client side first/third person
   static bool mFirstPerson;      // Currently first person or not
   bool mUpdateCameraFov;        // set to notify server of camera FOV change
   F32 mCameraFov;               // current camera fov (in degrees)
   F32 mCameraPos;               // Current camera pos (0-1)
   F32 mCameraSpeed;             // Camera in/out speed

   //
   void moveWritePacket(BitStream *bstream);
   void moveReadPacket(BitStream *bstream);
   enum {
      BlockTypeMove = NetConnectionBlockTypeCount,
      GameConnectionBlockTypeCount
   };

   static S32 smVoiceConnections[MaxClients];   // voice<->connection mapping
   U8        mVoiceID;                          // voice id for bitvectors
   BitVector mWouldListenTo;                    // willing to listen to these clients
   BitVector mListeningTo;                      // currently talking to me
   U8        mMaxVoicechannels;                 // maximum number of people to listen to at one time
   U8        mCurVoicechannels;                 // current number of people I'm listening to
   U32       mVoiceDecodingMask;                // the decoders this person can use
   S32       mVoiceEncodingLevel;               // what codec level this client is encoding at (-1 for not encoding)

   AUDIOHANDLE mLockAudioHandle;
   AUDIOHANDLE mHomingAudioHandle;
   AUDIOHANDLE mTargetLockedAudioHandle;
   AUDIOHANDLE mTargetingAudioHandle;

protected:
   struct GamePacketNotify : public NetConnection::PacketNotify
   {
      U32 xorVisibleMask[TargetManager::TargetFreeMaskSize];
      U32 controlObjectModifyKey;
      S32 cameraFov;
      GamePacketNotify();
   };
   PacketNotify *allocNotify();
   U32 mTargetVisibleMask[TargetManager::TargetFreeMaskSize];
   U32 mLastMoveAck;
   U32 mLastClientMove;
   U32 mFirstMoveIndex;
   U32 mMoveCredit;
   U32 mControlObjectModifyKey;
   U32 mLastSentControlObjectModifyKey;
   U32 mAckedControlObjectModifyKey;
   U32 mRemoteControlObjectModifyKey;
   U32 mControlStateSkipCount;

   MoveList mMoveList;
   bool mAIControlled;
   AuthInfo *mAuthInfo;

	static S32 mLagThresholdMS;
	S32 mLastPacketTime;
	bool mLagging;

   U32 mSensorGroup;
   S32 mTargetId;
   Point3F mTargetPos;

   F32 mDamageFlash;
   F32 mWhiteOut;
   bool mSelfLocked;
   bool mSelfHomed;
   bool mSeekerTracking;
   U32 mSeekerMode;
   SimObjectPtr<ShapeBase> mSeekerObject;

	//the black out vars are not network transmitted, they are for local connections only...
	F32 mBlackOut;
	S32 mBlackOutTimeMS;
	S32 mBlackOutStartTimeMS;
	bool mFadeToBlack;
      
   //
   void readPacket(BitStream *bstream);
   void writePacket(BitStream *bstream, PacketNotify *note);
   void packetReceived(PacketNotify *note);
   void packetDropped(PacketNotify *note);
   void connectionError(const char *errorString);

   void writeDemoStartBlock(ResizeBitStream *stream);
   void readDemoStartBlock(BitStream *stream);
   void handleRecordedBlock(U32 type, U32 size, void *data);

public:
   DECLARE_CONOBJECT(GameConnection);
   static void consoleInit();
   void handleGhostMessage(S32 message, U32 sequence, U32 ghostCount);
   void setDisconnectReason(const char *reason);
   GameConnection(bool ghostFrom = false, bool ghostTo = false, bool sendEvents = false);
   ~GameConnection();

   U32 getDataBlockSequence() { return mDataBlockSequence; }
   void setDataBlockSequence(U32 seq) { mDataBlockSequence = seq; }

   bool onAdd();
   void onRemove();

   static GameConnection *getServerConnection() { return dynamic_cast<GameConnection*>(mServerConnection); }
   static GameConnection *getLocalClientConnection() { return dynamic_cast<GameConnection*>(mLocalClientConnection); }

   // Control object
   void setControlObject(ShapeBase *co);
   void setControlObjectDirty() { if(mControlObjectModifyKey == mLastSentControlObjectModifyKey) mControlObjectModifyKey++; }
   ShapeBase* getControlObject()  { return  mControlObject; }
   bool getControlCameraTransform(F32 dt,MatrixF* mat);
   bool getControlCameraVelocity(Point3F *vel);

   void setObjectActiveImage(ShapeBase * obj, U32 slot);

   bool getControlCameraFov(F32 * fov);
   bool setControlCameraFov(F32 fov);
   bool isValidControlCameraFov(F32 fov);
   void updateLockTones();
   void updateLockWarnings();
   void detectLag();

   // Datablock management
   S32 getDataBlockModifiedKey()  { return mDataBlockModifiedKey; }
   void setDataBlockModifiedKey(S32 key)  { mDataBlockModifiedKey = key; }
   S32 getMaxDataBlockModifiedKey()  { return mMaxDataBlockModifiedKey; }
   void setMaxDataBlockModifiedKey(S32 key)  { mMaxDataBlockModifiedKey = key; }

   // presentation of flash and seekers
   F32 getDamageFlash() { return mDamageFlash; }
   F32 getWhiteOut() { return mWhiteOut; }
   bool getSelfLocked() { return mSelfLocked; }
   bool getSelfHomed() { return mSelfHomed; }
   bool getSeekerTracking() { return mSeekerTracking; }
   U32 getSeekerMode() { return mSeekerMode; }
   ShapeBase *getSeekerObject() { return mSeekerObject; }

	void setBlackOut(bool fadeToBlack, S32 timeMS);
	F32 getBlackOut();

   // Move management
   void pushMove(const Move &mv);
   bool getNextMove(Move &curMove);
   bool isBacklogged();
   virtual void getMoveList(Move**,U32* numMoves);
   virtual void clearMoves(U32 count);
   void collectMove(U32 simTime);
   virtual bool areMovesPending();
   void incMoveCredit(U32 count);

   void setAuthInfo(const AuthInfo *info);
   const AuthInfo *getAuthInfo();

   // Sound
   U8 getVoiceID() { return(mVoiceID); }
   void play2D(const AudioProfile *profile);
   void play3D(const AudioProfile *profile, const MatrixF *transform);
   void listenTo(U8 clientId, bool tf);
   void listenToAll();
   void listenToNone();
   void setVoiceChannels(S32 num);
   U32 getVoiceDecodingMask() { return(mVoiceDecodingMask); }
   void setVoiceDecodingMask(U32 mask) { mVoiceDecodingMask = mask; }
   U32 getVoiceEncodingLevel() { return(mVoiceEncodingLevel); }
   void setVoiceEncodingLevel(S32 level) { mVoiceEncodingLevel = level; }
   
   bool listenEnabled() { return( mMaxVoicechannels > 0 ); }
   bool canListen(GameConnection *);
   bool willListen(U8 voiceId);
   void stopListening(U8 voiceId);
   bool isListening(U8 voiceId) { return mListeningTo.test(voiceId); }
   bool getListenState(U8 voiceId);

   // Misc.
   bool isFirstPerson()  { return mCameraPos == 0; }
   bool isAIControlled() { return mAIControlled; }

   bool isScopingCommanderMap() const { return(mInCommanderMap); }
   void scopeCommanderMap(const bool scope) {mInCommanderMap = scope;}

   void doneScopingScene();
   void demoPlaybackComplete();
   U32  getSensorGroup() { return mSensorGroup; }
   void setSensorGroup(U32 group);
   
   bool sendLOSTarget();
   void sendTargetToServer(S32 targetId, Point3F pos);
   void sendTargetTo(NetConnection *conn, bool assign);
   void setServerTarget(S32 targetId, Point3F targetPos);

   S32 getTargetId() { return(mTargetId); }
   Point3F getTargetPos() { return(mTargetPos); }
   void setTargetId(S32 id) { mTargetId = id; }
   void setTargetPos(const Point3F &pos) { mTargetPos = pos; }

   void resetVisibleMasks();

   void setMissionCRC(U32 crc)   { mMissionCRC = crc; }
   U32 getMissionCRC()           { return(mMissionCRC); }
};

#endif
