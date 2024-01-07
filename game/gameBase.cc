//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "game/gameBase.h"
#include "console/consoleTypes.h"
#include "console/consoleInternal.h"
#include "core/bitStream.h"
#include "sim/netConnection.h"
#include "game/gameConnection.h"
#include "game/targetManager.h"
#include "game/sensor.h"

//----------------------------------------------------------------------------
// Ghost update relative priority values

static F32 sUpFov       = 1.0;
static F32 sUpDistance  = 0.4;
static F32 sUpVelocity  = 0.4;
static F32 sUpSkips     = 0.2;
static F32 sUpOwnership = 0.2;
static F32 sUpInterest  = 0.2;


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

IMPLEMENT_CO_DATABLOCK_V1(GameBaseData);

GameBaseData::GameBaseData()
{
   catagory = ""; 
   className = "";
   packed = false;
   beacon = false;
   beaconType = GameBase::enemyBeacon;
}

bool GameBaseData::onAdd()
{
   if (!Parent::onAdd())
      return false;

   // Link our object name to the datablock class name and
   // then onto our C++ class name.
   const char* name = getName();
   if (name && name[0] && getClassRep()) {
      Namespace *parent = getClassRep()->getNameSpace();
      if (className && className[0] && dStricmp(className, parent->mName)) {
         Con::linkNamespaces(parent->mName,className);
         Con::linkNamespaces(className,name);
      }
      else
         Con::linkNamespaces(parent->mName,name);
      mNameSpace = Con::lookupNamespace(name);
   }

   // If no className was specified, set it to our C++ class name
   if (!className || !className[0])
      className = getClassRep()->getClassName();

   return true;
}

static EnumTable::Enums beaconEnums[] = 
{
   { GameBase::enemyBeacon,   "enemy"   },
   { GameBase::friendBeacon,  "friend"  },
   { GameBase::vehicleBeacon, "vehicle" }
};
static EnumTable gBeaconTypeTable(3, &beaconEnums[0]); 

void GameBaseData::initPersistFields()
{
   Parent::initPersistFields();
   addField("catagory",   TypeCaseString,          Offset(catagory,   GameBaseData));
   addField("className",  TypeString,              Offset(className,  GameBaseData));
   addField("beacon",     TypeBool,                Offset(beacon,     GameBaseData));
   addField("beaconType", TypeEnum,                Offset(beaconType, GameBaseData), 1 , &gBeaconTypeTable);
}

bool GameBaseData::preload(bool server, char errorBuffer[256])
{
   if (!Parent::preload(server, errorBuffer))
      return false;
   packed = false;
   return true;
}

void GameBaseData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);
   packed = true;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

bool GameBase::gShowBoundingBox;
StringTableEntry GameBase::smEnemyBeaconText;
StringTableEntry GameBase::smFriendBeaconText;
StringTableEntry GameBase::smVehicleBeaconText;

//----------------------------------------------------------------------------
IMPLEMENT_CO_NETOBJECT_V1(GameBase);

GameBase::GameBase()
{
   mNetFlags.set(Ghostable);
   mTypeMask |= GameBaseObjectType;

   mProcessLink.next = mProcessLink.prev = this;
   mAfterObject = 0;
   mProcessTag = 0;
   mLastDelta = 0;
   mDataBlock = 0;
   mProcessTick = true;
   mNameTag = "";
   mBeacon = false;
   mBeaconType = enemyBeacon;
   
   mTargetInfo = NULL;
   mTargetId = -1;

   mLockCount = 0;
   mHomingCount = 0;
}

GameBase::~GameBase()
{
   plUnlink();
}


//----------------------------------------------------------------------------

bool GameBase::onAdd()
{
   if (!Parent::onAdd() || !mDataBlock)
      return false;

   if (isClientObject()) {
      // Client datablock are initialized by the initial update
      gClientProcessList.addObject(this);
   }
   else {
      // Datablock must be initialized on the server
      if (!onNewDataBlock(mDataBlock))
         return false;
      gServerProcessList.addObject(this);

      setBeacon(mDataBlock->beacon);
      setBeaconType(mDataBlock->beaconType);
   }
   return true;
}

void GameBase::onRemove()
{
   plUnlink();
   Parent::onRemove();
}

void GameBase::setBeacon(bool beacon)
{
   if(beacon ^ mBeacon)
   {
      SimSet * set = isServerObject() ? Sim::getServerTargetSet() : Sim::getClientTargetSet();
      beacon ? set->addObject(this) : set->removeObject(this);
      mBeacon = beacon;

      if(isServerObject())
         setMaskBits(ExtendedInfoMask);
   }
}

void GameBase::setBeaconType(S32 beaconType)
{
   mBeaconType = beaconType;
   if(isServerObject())
      setMaskBits(ExtendedInfoMask);
}

bool GameBase::onNewDataBlock(GameBaseData* dptr)
{
   mDataBlock = dptr;

   if (!mDataBlock)
      return false;

   if(isProperlyAdded() && isServerObject())
   {
      setBeacon(mDataBlock->beacon);
      setBeaconType(mDataBlock->beaconType);
   }   
   setMaskBits(DataBlockMask);
   return true;
}

void GameBase::inspectPostApply()
{
   setMaskBits(ExtendedInfoMask);
}

//----------------------------------------------------------------------------
void GameBase::processTick(const Move*)
{
   mLastDelta = 0;
}

void GameBase::interpolateTick(F32 delta)
{
   mLastDelta = delta;
}

void GameBase::advanceTime(F32)
{
   //
}


//----------------------------------------------------------------------------

F32 GameBase::getUpdatePriority(CameraScopeQuery *camInfo, U32 updateMask, S32 updateSkips)
{
   updateMask;

   // Calculate a priority used to decide if this object
   // will be updated on the client.  All the weights
   // are calculated 0 -> 1  Then weighted together at the
   // end to produce a priority.
   Point3F pos;
   getWorldBox().getCenter(&pos);
   pos -= camInfo->pos;
   F32 dist = pos.len();
   if (dist == 0.0f) dist = 0.001f;
   pos *= 1.0f / dist;

   // Weight based on linear distance, the basic stuff.
   F32 wDistance = (dist < camInfo->visibleDistance)?
      1.0f - (dist / camInfo->visibleDistance): 0.0f;

   // Weight by field of view, objects directly in front
   // will be weighted 1, objects behind will be 0
   F32 dot = mDot(pos,camInfo->orientation);
   bool inFov = dot > camInfo->cosFov;
   F32 wFov = inFov? 1.0f: 0;

   // Weight by linear velocity parallel to the viewing plane
   // (if it's the field of view, 0 if it's not).
   F32 wVelocity = 0.0f;
   if (inFov)
   {
      Point3F vec;
      mCross(camInfo->orientation,getVelocity(),&vec);
      wVelocity = (vec.len() * camInfo->fov) /
         (camInfo->fov * camInfo->visibleDistance);
      if (wVelocity > 1.0f)
         wVelocity = 1.0f;
   }

   // Weight by interest.
   F32 wInterest;
   if (getType() & PlayerObjectType)
      wInterest = 0.75f;
   else if (getType() & ProjectileObjectType)
   {
      // Projectiles are more interesting if they
      // are heading for us.
      wInterest = 0.30f;
      F32 dot = -mDot(pos,getVelocity());
      if (dot > 0.0f)
         wInterest += 0.20 * dot;
   }
   else
   {
      if (getType() & ItemObjectType)
         wInterest = 0.25f;
      else
         // Everything else is less interesting.
         wInterest = 0.0f;
   }

   // Weight by updateSkips
   F32 wSkips = updateSkips * 0.5;

   // Calculate final priority, should total to about 1.0f
   //
   return
      wFov       * sUpFov +
      wDistance  * sUpDistance +
      wVelocity  * sUpVelocity +
      wSkips     * sUpSkips +
      wInterest  * sUpInterest;
}


Point3F GameBase::getVelocity() const
{
   return Point3F(0, 0, 0);
}


//----------------------------------------------------------------------------
bool GameBase::setDataBlock(GameBaseData* dptr)
{
   if (isGhost() || isProperlyAdded()) {
      if (mDataBlock != dptr)
         return onNewDataBlock(dptr);
   }
   else
      mDataBlock = dptr;
   return true;
}


//--------------------------------------------------------------------------
void GameBase::scriptOnAdd()
{
   // Script onAdd() must be called by the leaf class after
   // everything is ready.
   if (!isGhost())
      Con::executef(mDataBlock,2,"onAdd",scriptThis());
}

void GameBase::scriptOnNewDataBlock()
{
   // Script onNewDataBlock() must be called by the leaf class
   // after everything is loaded.
   if (!isGhost())
      Con::executef(mDataBlock,2,"onNewDataBlock",scriptThis());
}

void GameBase::scriptOnRemove()
{
   // Script onRemove() must be called by leaf class while
   // the object state is still valid.
   if (!isGhost() && mDataBlock)
      Con::executef(mDataBlock,2,"onRemove",scriptThis());
}


//--------------------------------------------------------------------------
bool GameBase::getTarget(Point3F* p, U32* tid)
{
   if (mBeacon && getTargetInfo()) {
      getTransform().getColumn(3, p);
      *tid = getTargetInfo()->sensorGroup;
      return true;
   } else {
      return false;
   }
}

TargetInfo *GameBase::getTargetInfo()
{
   return mTargetInfo;
}

void GameBase::setTarget(S32 targetId)
{
   if(mTargetId == targetId)
      return;

   mTargetId = targetId;
   if(mTargetId == -1)
      mTargetInfo = 0;
   else
      mTargetInfo = isServerObject() ? gTargetManager->getServerTarget(mTargetId) :
                                       gTargetManager->getClientTarget(mTargetId);

   // set the targets object as this (only non-team targets can have objects)
   if(mTargetId >= 32)
      gTargetManager->setTargetObject(mTargetInfo, this);

   targetInfoChanged(mTargetInfo);

   if(isServerObject())
      setMaskBits(ExtendedInfoMask);
}

void GameBase::setBeaconGameNames(const char * enemyName, const char * friendName, const char * vehicleName)
{
   smEnemyBeaconText = StringTable->insert(enemyName);
   smFriendBeaconText = StringTable->insert(friendName);
   smVehicleBeaconText = StringTable->insert(vehicleName);
}

bool GameBase::getGameName(char * buf, S32 bufSize)
{
   if(gTargetManager->getGameName(getTarget(), buf, bufSize, isServerObject()))
      return(true);

   // special cases:
   // beacon:
   if(mBeacon)
   {
      const char * text = 0;
      switch(mBeaconType)
      {
         case enemyBeacon:
            text = smEnemyBeaconText;
            break;
         case friendBeacon:
            text = smFriendBeaconText;
            break;
         case vehicleBeacon:
            text = smVehicleBeaconText;
            break;
         default:
            AssertFatal(false, "Unknown beacon type");
            return(false);
      }
      dSprintf(buf, bufSize, "%s", text);
      return(true);
   }

   return(false);
}

void GameBase::setTransform(const MatrixF & mat)
{
   if(isClientObject() && mTargetId >= 0) 
   {
      AUDIOHANDLE & handle = gTargetManager->mClientAudioHandles[mTargetId];
      if(handle != NULL_AUDIOHANDLE)
      {
         if(alxIsPlaying(handle))
            alxSourceMatrixF(handle, &mat);
         else
            handle = NULL_AUDIOHANDLE;
      }
   }
   Parent::setTransform(mat);
}

//----------------------------------------------------------------------------
void GameBase::plUnlink()
{
   mProcessLink.next->mProcessLink.prev = mProcessLink.prev;
   mProcessLink.prev->mProcessLink.next = mProcessLink.next;
   mProcessLink.next = mProcessLink.prev = this;
}

void GameBase::plLinkAfter(GameBase* obj)
{
   // Link this after obj
   mProcessLink.next = obj->mProcessLink.next;
   mProcessLink.prev = obj;
   obj->mProcessLink.next = this;
   mProcessLink.next->mProcessLink.prev = this;
}

void GameBase::plLinkBefore(GameBase* obj)
{
   // Link this before obj
   mProcessLink.next = obj;
   mProcessLink.prev = obj->mProcessLink.prev;
   obj->mProcessLink.prev = this;
   mProcessLink.prev->mProcessLink.next = this;
}

void GameBase::plJoin(GameBase* head)
{
   GameBase *tail1 = head->mProcessLink.prev;
   GameBase *tail2 = mProcessLink.prev;
   tail1->mProcessLink.next = this;
   mProcessLink.prev = tail1;
   tail2->mProcessLink.next = head;
   head->mProcessLink.prev = tail2;
}


//----------------------------------------------------------------------------
void GameBase::processAfter(GameBase* obj)
{
   mAfterObject = obj;
   if ((const GameBase*)obj->mAfterObject == this)
      obj->mAfterObject = 0;
   if (isGhost())
      gClientProcessList.markDirty();
   else
      gServerProcessList.markDirty();
}

void GameBase::clearProcessAfter()
{
   mAfterObject = 0;
}


//----------------------------------------------------------------------------

bool GameBase::writePacketData(GameConnection*, BitStream*)
{
   return true;
}

void GameBase::readPacketData(GameConnection*,BitStream*)
{
   //
}

U32 GameBase::packUpdate(NetConnection *, U32 mask, BitStream *stream)
{
   if (stream->writeFlag((mask & DataBlockMask) && mDataBlock != NULL)) {
      stream->writeRangedU32(mDataBlock->getId(),
                             DataBlockObjectIdFirst,
                             DataBlockObjectIdLast);
   }
   
   if(stream->writeFlag(mask & ExtendedInfoMask))
   {
      if(stream->writeFlag(mTargetId != -1))
         stream->writeInt(mTargetId, TargetManager::TargetIdBitSize);

      stream->writeFlag(mBeacon);
      stream->write(mBeaconType);
   }

   return 0;
}

void GameBase::unpackUpdate(NetConnection *con, BitStream *stream)
{
   if (stream->readFlag()) {
      GameBaseData* dptr = 0;
      SimObjectId id = stream->readRangedU32(DataBlockObjectIdFirst,
                                             DataBlockObjectIdLast);
                                             
      if (!Sim::findObject(id,dptr) || !setDataBlock(dptr))
         con->setLastError("Invalid packet GameBase::unpackUpdate()");
   }
   
   // ExtendedInfo
   if(stream->readFlag())     
   {
      // target
      S32 targetId = -1;
      if(stream->readFlag())
         targetId = stream->readInt(TargetManager::TargetIdBitSize);

      if(targetId != mTargetId)
         setTarget(targetId);
      
      // beacon
      setBeacon(stream->readFlag());
      stream->read(&mBeaconType);
   }
}

//----------------------------------------------------------------------------
U32 GameBase::getSensorGroup()
{
   if(!mTargetInfo)
      return 0;
   else
      return mTargetInfo->sensorGroup;
}

//----------------------------------------------------------------------------
static int cGetDataBlock(SimObject *obj, S32, const char **)
{
   GameBase* ptr = static_cast<GameBase*>(obj);
   return ptr->getDataBlock()? ptr->getDataBlock()->getId(): 0;
}

//----------------------------------------------------------------------------
static bool cSetDataBlock(SimObject *obj, S32, const char **argv)
{
   GameBaseData* data;
   if (Sim::findObject(argv[2],data)) {
      GameBase* ptr = static_cast<GameBase*>(obj);
      return ptr->setDataBlock(data);
   }
   Con::printf("Could not find data block \"%s\"",argv[2]);
   return false;
}

//----------------------------------------------------------------------------

static void cSetTargetId(SimObject *obj, S32, const char **argv)
{
   GameBase* ptr = static_cast<GameBase*>(obj);
   if(!ptr->isServerObject())
      return;

   // check for valid targetId
   S32 targetId = dAtoi(argv[2]);
   if((targetId < -1) || (targetId >= TargetManager::MaxTargets))
   {
      Con::errorf(ConsoleLogEntry::General, "GameBase::cSetTargetId: invalid target id [%s]", argv[2]);
      return;
   }
   ptr->setTarget(targetId);
}

static S32 cGetTargetId(SimObject *obj, S32, const char **)
{
   GameBase* ptr = static_cast<GameBase*>(obj);
   return ptr->getTarget();
}

static bool cIsBeacon(SimObject * obj, S32, const char **)
{
   GameBase * gameObj = static_cast<GameBase*>(obj);
   return(gameObj->isBeacon());
}

static void cSetBeacon(SimObject * obj, S32, const char ** argv)
{
   GameBase * gameObj = static_cast<GameBase*>(obj);
   if(gameObj->isServerObject())
      gameObj->setBeacon(dAtoi(argv[2]));
}

static void cSetBeaconType(SimObject * obj, S32, const char ** argv)
{
   GameBase * gameObj = static_cast<GameBase*>(obj);
   if(gameObj->isServerObject())
      for(S32 x = 0; x < 3; ++x)
         if(!dStrcmp(argv[2], beaconEnums[x].label))
         {
            gameObj->setBeaconType(x);
            return;            
         }
}

static bool cIsBeaconType(SimObject * obj, S32, const char ** argv)
{
   GameBase * gameObj = static_cast<GameBase*>(obj);
   if(gameObj->isServerObject())
      for(S32 x = 0; x < 3; ++x)
         if(!dStrcmp(argv[2], beaconEnums[x].label))
         {
            if(gameObj->getBeaconType() == x)
               return true;
            break;            
         }
   return false;
}

static void cSetBeaconNames(SimObject *, S32, const char ** argv)
{
   GameBase::setBeaconGameNames(argv[1], argv[2], argv[3]);
}

//----------------------------------------------------------------------------

IMPLEMENT_GETDATATYPE(GameBaseData)
IMPLEMENT_SETDATATYPE(GameBaseData)

void GameBase::initPersistFields()
{
   Parent::initPersistFields();

   // set some static membors
   smEnemyBeaconText = StringTable->insert("Target Beacon");
   smFriendBeaconText = StringTable->insert("Marker Beacon");
   smVehicleBeaconText = StringTable->insert("Bomb Target");

   Con::registerType(TypeGameBaseDataPtr, sizeof(GameBaseData*),
                     REF_GETDATATYPE(GameBaseData),
                     REF_SETDATATYPE(GameBaseData));

   addField("nameTag", TypeCaseString, Offset(mNameTag, GameBase));
   addField("dataBlock", TypeGameBaseDataPtr, Offset(mDataBlock, GameBase));

   addField("lockCount",   TypeS32, Offset(mLockCount,   GameBase));
   addField("homingCount", TypeS32, Offset(mHomingCount, GameBase));
}

void GameBase::consoleInit()
{
#ifdef DEBUG
   Con::addVariable("GameBase::boundingBox", TypeBool, &gShowBoundingBox);
#endif

   Con::addCommand("GameBase", "getDataBlock", cGetDataBlock, "obj.getDataBlock()", 2, 2);
   Con::addCommand("GameBase", "setDataBlock", cSetDataBlock, "obj.setDataBlock(DataBlock)", 3, 3);
   Con::addCommand("GameBase", "setTarget", cSetTargetId, "obj.setTarget(targetId)", 3, 3);
   Con::addCommand("GameBase", "getTarget", cGetTargetId, "obj.getTarget()", 2, 2);
   Con::addCommand("GameBase", "isBeacon", cIsBeacon, "obj.isBeacon()", 2, 2);
   Con::addCommand("GameBase", "setBeacon", cSetBeacon, "obj.setBeacon(bool)", 3, 3);
   Con::addCommand("GameBase", "setBeaconType", cSetBeaconType, "obj.setBeaconType(enemy or friend or vehicle)", 3, 3);
   Con::addCommand("GameBase", "isBeaconType", cIsBeaconType, "obj.isBeaconType(enemy or friend or vehicle)", 3, 3);
   Con::addCommand("setBeaconNames", cSetBeaconNames, "setBeaconNames(target, marker, vehicle)", 4, 4);
}



//--------------------------------------------------------------------------
// Sensor stuff...
SensorData* GameBase::getSensorData()
{
   if(mTargetInfo)
      return mTargetInfo->sensorData;
   else
      return NULL;
}

void GameBase::targetInfoChanged(TargetInfo *info)
{
   if(isClientObject())
   {
      if(info)
         Sim::getCommandTargetSet()->addObject(this);
      else
         Sim::getCommandTargetSet()->removeObject(this);
   }

   if(info)
   {
      if(mTargetId >= 32)
         gTargetManager->setTargetObject(info, this);
   }
   else
   {
      mTargetId = -1;
      mTargetInfo = 0;
   }
}


void GameBase::setHeat(const F32)
{
   // Do nothing
}

F32 GameBase::getHeat() const
{
   return 0.0f;
}

