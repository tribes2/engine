//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GAMEBASE_H_
#define _GAMEBASE_H_

#ifndef _SCENEOBJECT_H_
#include "sim/sceneObject.h"
#endif
#ifndef _RESMANAGER_H_
#include "core/resManager.h"
#endif
#ifndef _GTEXMANAGER_H_
#include "dgl/gTexManager.h"
#endif

class NetConnection;
class ProcessList;
struct Move;
struct SensorData;
struct TargetInfo;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

struct GameBaseData : public SimDataBlock {
  private:
   typedef SimDataBlock Parent;

  public:
   bool packed;
   StringTableEntry catagory;
   StringTableEntry className;

   bool beacon;                     // will show a beacon on this object through the current connection sensorgroup
   S32 beaconType;

   // sensor data
   bool onAdd();
   
   // The derived class should provide the following:
   DECLARE_CONOBJECT(GameBaseData);
   GameBaseData();
   static void initPersistFields();
   bool preload(bool server, char errorBuffer[256]);
   void unpackData(BitStream* stream);
};

//----------------------------------------------------------------------------
class GameConnection;

class GameBase : public SceneObject
{
  private:
   typedef SceneObject Parent;
   friend class ProcessList;

   // Datablock
  private:
   GameBaseData*     mDataBlock;
   bool              mBeacon;
   S32               mBeaconType;
   StringTableEntry  mNameTag;

   // Processing interface
  private:
   void plUnlink();
   void plLinkAfter(GameBase*);
   void plLinkBefore(GameBase*);
   void plJoin(GameBase*);
   struct Link {
      GameBase *next;
      GameBase *prev;
   };
   U32  mProcessTag;                      // Tag used during sort
   Link mProcessLink;                     // Ordered process queue
   SimObjectPtr<GameBase> mAfterObject;

   // Collision Notification
  public:
   enum gBeaconType
   {
      enemyBeacon = 0,
      friendBeacon,
      vehicleBeacon   
   };

   struct CollisionTimeout {
      CollisionTimeout* next;
      GameBase*         object;
      U32               objectNumber;
      SimTime           expireTime;
   };
  private:
   CollisionTimeout* mTimeoutList;
  
  public:
   static bool gShowBoundingBox;
  protected:
   bool mProcessTick;                // Tick or no tick?
   F32  mLastDelta;
   TargetInfo *mTargetInfo;
   S32 mTargetId;
   
   GameBase *mNextSensor;
   GameBase *mPrevSensor;
   
   static      StringTableEntry     smEnemyBeaconText;
   static      StringTableEntry     smFriendBeaconText;
   static      StringTableEntry     smVehicleBeaconText;

   S32                     mLockCount;
   S32                     mHomingCount;

   //
   F32 getUpdatePriority(CameraScopeQuery *focusObject, U32 updateMask, S32 updateSkips);
   
  public:
   GameBase();
   ~GameBase();

   enum GameBaseMasks {
      InitialUpdateMask =     1 << 0,
      DataBlockMask =         1 << 1,
      ExtendedInfoMask =      1 << 2,
      NextFreeMask =          ExtendedInfoMask << 1
   };

   // Init
   bool onAdd();
   void onRemove();
   void inspectPostApply();
   static void consoleInit();
   static void initPersistFields();

   // Data block
   bool          setDataBlock(GameBaseData* dptr);
   GameBaseData* getDataBlock()  { return mDataBlock; }
   virtual bool  onNewDataBlock(GameBaseData* dptr);

   // The scriptOnXX methods must be invoked by the leaf classes
   void scriptOnAdd();
   void scriptOnNewDataBlock();
   void scriptOnRemove();

   // Processing
   void setProcessTick(bool t) { mProcessTick = t; }
   void processAfter(GameBase*);
   void clearProcessAfter();
   GameBase* getProcessAfter() { return mAfterObject; }
   void removeFromProcessList() { plUnlink(); }
   virtual void processTick(const Move*);
   virtual void interpolateTick(F32 backDelta);
   virtual void advanceTime(F32 dt);

   virtual Point3F getVelocity() const;

   virtual void setHeat(const F32);
   virtual F32  getHeat() const;

   // Targeting
   virtual bool getTarget(Point3F* pTarget, U32* pTeamId);
   virtual void targetInfoChanged(TargetInfo *targ);
   void setTarget(S32 targetId);
   TargetInfo *getTargetInfo();
   S32 getTarget() { return mTargetId; }
   void sendTargetTo(NetConnection *conn);

   void setTransform(const MatrixF&);

   bool isBeacon() { return(mBeacon); }
   void setBeacon(bool beacon);
   void setBeaconType(S32);
   S32 getBeaconType() { return mBeaconType; }   
   static void setBeaconGameNames(const char * target, const char * marker, const char * vehicle);
   
   S32 getLockCount() { return mLockCount; }
   S32 getHomingCount() { return mHomingCount; }
   void incHomingCount();
   void decHomingCount();
   void incLockCount();
   void decLockCount();

   // Sensor
   SensorData *getSensorData();
   bool isSensorVisible();
   U32  getSensorGroup();
   bool getGameName(char * buf, S32 bufSize);

   // Network
   U32  packUpdate(NetConnection *, U32 mask, BitStream *stream);
   void unpackUpdate(NetConnection *, BitStream *stream);
   // writePacketData returns false if it needs to write more... for
   // example if its control object hasn't been written across yet.
   virtual bool writePacketData(GameConnection *, BitStream *stream);
   virtual void readPacketData(GameConnection *, BitStream *stream);

   DECLARE_CONOBJECT(GameBase);
};

inline void GameBase::incHomingCount()
{
   mHomingCount++;
   
   if( mLockCount > 0 )
      decLockCount();
}

inline void GameBase::decHomingCount()
{
   if (mHomingCount)
      mHomingCount--;
}

inline void GameBase::incLockCount()
{
   mLockCount++;
}

inline void GameBase::decLockCount()
{
   if (mLockCount)
      mLockCount--;
}


//-----------------------------------------------------------------------------

#define TickShift   5
#define TickMs      (1 << TickShift)
#define TickSec     (F32(1) / TickMs)
#define SecTick     (F32(1) / TickSec)
#define TickMask    (TickMs - 1)

class ProcessList
{
   GameBase head;
   U32 mCurrentTag;
   SimTime mLastTick;
   SimTime mLastTime;
   SimTime mLastDelta;
   bool mDirty;

   void orderList();
   void advanceObjects();

public:
   SimTime getLastTime() { return mLastTime; }
   ProcessList();
   void markDirty()  { mDirty = true; }
   bool isDirty()  { return mDirty; }
   void addObject(GameBase* obj) {
      obj->plLinkBefore(&head);
   }
   void advanceServerTime(SimTime timeDelta);
   void advanceClientTime(SimTime timeDelta);
};

extern ProcessList gClientProcessList;
extern ProcessList gServerProcessList;

#endif
