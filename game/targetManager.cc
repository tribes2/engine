//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "game/targetManager.h"
#include "sim/netConnection.h"
#include "core/bitStream.h"
#include "game/gameConnection.h"
#include "game/sensor.h"
#include "game/shapeBase.h"
#include "console/consoleTypes.h"
#include "game/player.h"

//------------------------------------------------------------------------------
TargetManager *   gTargetManager = NULL;
HUDTargetList *   gTargetList = NULL;

//------------------------------------------------------------------------------
// Client target notification system:
//------------------------------------------------------------------------------
TargetManagerNotify::TargetManagerNotify()
{
   AssertISV(gTargetManager, "TargetManagerNotify:: no target manager present!");
   gTargetManager->addNotify(this);
}

TargetManagerNotify::~TargetManagerNotify()
{
   if(gTargetManager)
      gTargetManager->removeNotify(this);
}

void TargetManager::addNotify(TargetManagerNotify * notify)
{
   for(S32 i = notifyList.size() - 1; i >= 0; i--)
      if(notifyList[i] == notify)
      {
         Con::errorf(ConsoleLogEntry::General, "TargetManager::addNotify: object already being notified!");
         return;
      }
   notifyList.push_back(notify);
}

void TargetManager::removeNotify(TargetManagerNotify * notify)
{
   for(S32 i = notifyList.size() - 1; i >= 0; i--)
      if(notifyList[i] == notify)
      {
         notifyList.erase(i);
         return;
      }
   Con::errorf(ConsoleLogEntry::General, "TargetManager::removeNotify: object not found in notify list!");
}

void TargetManager::notifyTargetAdded(U32 target)
{
   for(S32 i = notifyList.size() - 1; i >= 0; i--)
      notifyList[i]->targetAdded(target);
}

void TargetManager::notifyTargetRemoved(U32 target)
{
   for(S32 i = notifyList.size() - 1; i >= 0; i--)
      notifyList[i]->targetRemoved(target);
}

void TargetManager::notifyTargetChanged(U32 target)
{
   for(S32 i = notifyList.size() - 1; i >= 0; i--)
      notifyList[i]->targetChanged(target);
}

void TargetManager::notifyTargetsCleared()
{
   for(S32 i = notifyList.size() - 1; i >= 0; i--)
      notifyList[i]->targetsCleared();
}

//------------------------------------------------------------------------------
// Client target free notification:
//------------------------------------------------------------------------------
class TargetFreeEvent : public NetEvent
{
public:
   S32 mTarget;
   TargetFreeEvent(S32 target = 0)
   {
      mTarget = target;
   }
   void pack(NetConnection *, BitStream *bstream)
   {
      bstream->writeInt(mTarget, TargetManager::TargetIdBitSize);
   }
   void write(NetConnection *conn, BitStream *bstream)
   {
      pack(conn, bstream);
   }
   void unpack(NetConnection *, BitStream *bstream)
   {
      mTarget = bstream->readInt(TargetManager::TargetIdBitSize);
   }
   void process(NetConnection *conn)
   {
      GameConnection *gc = (GameConnection *) conn;
      TargetInfo *targ = gTargetManager->getClientTarget(mTarget);

      if(gTargetManager->mClientAudioHandles[mTarget] != NULL_AUDIOHANDLE)
      {
         alxStop(gTargetManager->mClientAudioHandles[mTarget]);
         gTargetManager->mClientAudioHandles[mTarget] = 0;
      }

      if(targ->allocated)
         gTargetManager->notifyTargetRemoved(mTarget);   

      if(bool(targ->targetObject))
         targ->targetObject->targetInfoChanged(0);

      targ->clear(false);
   }
   DECLARE_CONOBJECT(TargetFreeEvent);
};
IMPLEMENT_CO_CLIENTEVENT_V1(TargetFreeEvent);

//------------------------------------------------------------------------------
// Client new/changed target info event:
//------------------------------------------------------------------------------
class TargetInfoEvent : public NetEvent
{
public:
   S32 mTarget;
   S32 mNameTag;
   S32 mSkinTag;
   S32 mVoiceTag;
   S32 mTypeTag;
   S32 mSkinPrefTag;
   S32 mSensorGroup;
   S32 mDataBlockId;
   S32 mRenderFlags;
   F32 mVoicePitch;
   
   TargetInfoEvent(S32 target = 0, S32 nameTag = 0, S32 skinTag = 0, S32 voiceTag = 0,
                   S32 typeTag = 0, S32 sensorGroup = 0, S32 dataBlockId = 0, S32 renderFlags = 0, F32 voicePitch = -1.0f, S32 prefSkin = -1)
   {
      mTarget = target;
      mNameTag = nameTag;
      mSkinTag = skinTag;
      mVoiceTag = voiceTag;
      mTypeTag = typeTag;
      mSensorGroup = sensorGroup;
      mDataBlockId = dataBlockId;
      mSkinPrefTag = prefSkin;
      mRenderFlags = renderFlags;
      mVoicePitch = voicePitch;
   }
   
   void pack(NetConnection *, BitStream *bstream)
   {
      bstream->writeInt(mTarget, TargetManager::TargetIdBitSize);
      if(bstream->writeFlag(mNameTag != -1))
         if(bstream->writeFlag(mNameTag))
            bstream->writeInt(mNameTag, NetStringTable::StringIdBitSize);
      if(bstream->writeFlag(mSkinTag != -1))
         if(bstream->writeFlag(mSkinTag))
            bstream->writeInt(mSkinTag, NetStringTable::StringIdBitSize);
      if(bstream->writeFlag(mSkinPrefTag != -1))
         if(bstream->writeFlag(mSkinPrefTag))
            bstream->writeInt(mSkinPrefTag, NetStringTable::StringIdBitSize);
      if(bstream->writeFlag(mVoiceTag != -1))
         if(bstream->writeFlag(mVoiceTag))
            bstream->writeInt(mVoiceTag, NetStringTable::StringIdBitSize);
      if(bstream->writeFlag(mTypeTag != -1))
         if(bstream->writeFlag(mTypeTag))
            bstream->writeInt(mTypeTag, NetStringTable::StringIdBitSize);
      if(bstream->writeFlag(mSensorGroup != -1))
         bstream->writeInt(mSensorGroup, 5);
      if(bstream->writeFlag(mDataBlockId != -1))
         if(bstream->writeFlag(mDataBlockId))
            bstream->writeRangedU32(mDataBlockId, DataBlockObjectIdFirst, DataBlockObjectIdLast);
      if(bstream->writeFlag(mRenderFlags != -1))
         bstream->writeInt(mRenderFlags, TargetInfo::NumRenderBits);
      if(bstream->writeFlag(mVoicePitch != -1))
      {
         //convert the voice pitch from range [0.5, 2.0] to [0.0, 1.0]
         if (mVoicePitch < 0.5f || mVoicePitch > 2.0f)
            mVoicePitch = 1.0f;
         F32 packFloat = (mVoicePitch - 0.5f) / 1.5f;
         bstream->writeFloat(packFloat, 7);
      }
   }

   void write(NetConnection *conn, BitStream *bstream)
   {
      pack(conn, bstream);
   }
   
   void unpack(NetConnection *, BitStream *bstream)
   {
      mTarget = bstream->readInt(TargetManager::TargetIdBitSize);
      mNameTag = mSkinTag = mVoiceTag = mTypeTag = mSensorGroup = mDataBlockId = mRenderFlags = -1;
      mSkinPrefTag = -1;
      mVoicePitch = -1.0f;
      if(bstream->readFlag())
      {
         if(bstream->readFlag())
            mNameTag = bstream->readInt(NetStringTable::StringIdBitSize);
         else
            mNameTag = 0;
      }
      if(bstream->readFlag())
      {
         if(bstream->readFlag())
            mSkinTag = bstream->readInt(NetStringTable::StringIdBitSize);
         else
            mSkinTag = 0;
      }
      if(bstream->readFlag())
      {
         if(bstream->readFlag())
            mSkinPrefTag = bstream->readInt(NetStringTable::StringIdBitSize);
         else
            mSkinPrefTag = 0;
      }
      if(bstream->readFlag())
      {
         if(bstream->readFlag())
            mVoiceTag = bstream->readInt(NetStringTable::StringIdBitSize);
         else
            mVoiceTag = 0;
      }
      if(bstream->readFlag())
      {
         if(bstream->readFlag())
            mTypeTag = bstream->readInt(NetStringTable::StringIdBitSize);
         else
            mTypeTag = 0;
      }
      if(bstream->readFlag())
         mSensorGroup = bstream->readInt(5);

      if(bstream->readFlag())
         if(bstream->readFlag())
            mDataBlockId = bstream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);
            
      if(bstream->readFlag())
         mRenderFlags = bstream->readInt(TargetInfo::NumRenderBits);

      if(bstream->readFlag())
      {
         //convert the voice pitch from range [0.0, 1.0] back to [0.5, 2.0]
         F32 unpackFloat = bstream->readFloat(7);
         mVoicePitch = (unpackFloat * 1.5f) + 0.5f;
      }
   }
   
   void process(NetConnection *conn)
   {
      GameConnection *gc = (GameConnection *) conn;

      // translate all the strings   
      TargetInfo *targ = gTargetManager->getClientTarget(mTarget);
      if(mNameTag != -1)
      {
         targ->sNameTag = mNameTag;
         targ->nameTag = conn->translateRemoteStringId(mNameTag);
      }
      if(mSkinTag != -1)
      {
         targ->sSkinTag = mSkinTag;
         targ->skinTag = conn->translateRemoteStringId(mSkinTag);
      }
      if(mSkinPrefTag != -1)
      {
         targ->sSkinPrefTag = mSkinPrefTag;
         targ->skinPrefTag = conn->translateRemoteStringId(mSkinPrefTag);
      }
      if(mVoiceTag != -1)
      {
         targ->sVoiceTag = mVoiceTag;
         targ->voiceTag = conn->translateRemoteStringId(mVoiceTag);
      }
      if(mTypeTag != -1)
      {
         targ->sTypeTag = mTypeTag;
         targ->typeTag = conn->translateRemoteStringId(mTypeTag);
      }

      U32 oldGroup = targ->sensorGroup;
      if(mSensorGroup != -1)
         targ->sensorGroup = mSensorGroup;

      if(mRenderFlags != -1)
         targ->renderFlags = mRenderFlags;

      // team targets do not get notifications or objects
      if(mTarget >= 32)
      {
         if(mDataBlockId >= 0)
            targ->shapeBaseData = mDataBlockId ? dynamic_cast<ShapeBaseData*>(Sim::findObject(mDataBlockId)) : 0;

         if(targ->allocated)
         {
            gTargetManager->notifyTargetChanged(mTarget);
         }
         else
         {
            // created a new client target
            targ->allocated = true;
            gTargetManager->notifyTargetAdded(mTarget);
         }

         if(bool(targ->targetObject))
            targ->targetObject->targetInfoChanged(targ);
      }

      if(mVoicePitch != -1.0f)
         targ->voicePitch = mVoicePitch;
   }
   DECLARE_CONOBJECT(TargetInfoEvent);
};

IMPLEMENT_CO_CLIENTEVENT_V1(TargetInfoEvent);

//------------------------------------------------------------------------------
// Targets get one audio handle associated with them
//------------------------------------------------------------------------------
static F32 SoundPosAccuracy = 0.5;

class SimTargetAudioEvent : public NetEvent
{
   private:
      S32                  mTargetId;
      S32                  mFileTag;
      S32                  mDescriptionId;
   
      bool                 mHasPosition;
      Point3F              mPosition;
      
      bool                 mUpdateSound;

   public:   
      SimTargetAudioEvent(S32 targetId = -1, S32 fileTag = 0, S32 descId = 0, Point3F * pos = 0, bool update = false);
      void pack(NetConnection * con, BitStream * bstream);
      void write(NetConnection * con, BitStream * bstream);
      void unpack(NetConnection * con, BitStream * bstream);
      void process(NetConnection *);
      DECLARE_CONOBJECT(SimTargetAudioEvent);
};
IMPLEMENT_CO_CLIENTEVENT_V1(SimTargetAudioEvent);

SimTargetAudioEvent::SimTargetAudioEvent(S32 targetId, S32 fileTag, S32 descId, Point3F * pos, bool update)
{
   mTargetId = targetId;
   mFileTag = fileTag;
   mDescriptionId = descId;
   mUpdateSound = update;

   if(pos)
   {
      mHasPosition = true;
      mPosition = *pos;
   }
   else
      mHasPosition = false;
}

void SimTargetAudioEvent::pack(NetConnection * con, BitStream * bstream)
{
   bstream->writeInt(mTargetId, TargetManager::TargetIdBitSize);
   bstream->writeInt(mFileTag, NetStringTable::StringIdBitSize);
   bstream->writeRangedU32(mDescriptionId, DataBlockObjectIdFirst, DataBlockObjectIdLast);
   if(bstream->writeFlag(mHasPosition))
      con->writeCompressed(bstream, mPosition, SoundPosAccuracy);
      
  bstream->writeFlag(mUpdateSound);    
}

void SimTargetAudioEvent::write(NetConnection * con, BitStream * bstream)
{
   pack(con, bstream);
}

void SimTargetAudioEvent::unpack(NetConnection * con, BitStream * bstream)
{
   mTargetId = bstream->readInt(TargetManager::TargetIdBitSize);
   mFileTag = bstream->readInt(NetStringTable::StringIdBitSize);
   mDescriptionId = bstream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);

   mHasPosition = bstream->readFlag();
   if(mHasPosition)
      con->readCompressed(bstream, &mPosition, SoundPosAccuracy);
      
   mUpdateSound = bstream->readFlag();   
}

void SimTargetAudioEvent::process(NetConnection * con)
{
   TargetInfo * targInfo = gTargetManager->getClientTarget(mTargetId);
   if(!targInfo)
      return;
   
   AudioDescription * description = dynamic_cast<AudioDescription*>(Sim::findObject(mDescriptionId));
   if(!description)
      return;

   if(!mHasPosition)
   {
      if(!bool(targInfo->targetObject))
         return;
      targInfo->targetObject->getTransform().getColumn(3, &mPosition);
   }
   
   mFileTag = con->translateRemoteStringId(mFileTag);

   if(!mFileTag || !targInfo->voiceTag)
      return;

   const char * name = gNetStringTable->lookupString(mFileTag);
   const char * voice = gNetStringTable->lookupString(targInfo->voiceTag);

   if(!name || !voice || !name[0] || !voice[0])
      return;

   char buf[256];
   dSprintf(buf, sizeof(buf), "voice/%s/%s.wav", voice, name);

   MatrixF mat(true);
   mat.setColumn(3, mPosition);
   
   if( mUpdateSound )
   {
      AUDIOHANDLE & handle = gTargetManager->mClientAudioHandles[mTargetId];
      if(handle != NULL_AUDIOHANDLE)
         alxStop(handle);
      handle = alxCreateSource(description, buf, &mat);

      if (targInfo->voicePitch != 1.0f)
         alxSourcef(handle, AL_PITCH, targInfo->voicePitch);
      alxPlay(handle);
   }
   else
   {
      AUDIOHANDLE handle = alxCreateSource(description, buf, &mat);
      if (targInfo->voicePitch != 1.0)
         alxSourcef(handle, AL_PITCH, targInfo->voicePitch);
      alxPlay(handle);
   }
}

bool TargetManager::playTargetAudio(S32 targetId, S32 fileTag, AudioDescription * description, bool update)
{
   // check args: must have target, file, description
   if((targetId < 0) || (targetId >= MaxTargets))
      return(false);

   if(fileTag <= 0)
      return(false);

   if(!description)
      return(false);

   // get position of target
   TargetInfo * targInfo = getServerTarget(targetId);
   if(!targInfo)
      return(false);

   if(!bool(targInfo->targetObject))
      return(false);

   Point3F pos;
   targInfo->targetObject->getTransform().getColumn(3, &pos);

   // send to whomever can hear...
   for(NetConnection * con = NetConnection::getConnectionList(); con; con = con->getNext())
   {
      if(con->isServerConnection())
         continue;

      if (dynamic_cast<GameConnection*>(con) && static_cast<GameConnection*>(con)->isAIControlled())
         continue;
      
      ShapeBase * controlObj = static_cast<GameConnection*>(con)->getControlObject();
      if(!controlObj)
         continue;

      Point3F ear;
      controlObj->getTransform().getColumn(3, &ear);
      if((ear - pos).magnitudeSafe() >= description->mDescription.mMaxDistance)
         continue;

      // send position if not scoped on client
      bool sendPos = con->getGhostIndex(targInfo->targetObject) == -1;

      // send it
      con->checkString(fileTag);
      con->postNetEvent(new SimTargetAudioEvent(targetId, fileTag, description->getId(), sendPos ? &pos : 0, update));
   }

   return(true);
}

//--------------------------------------------------------------------------
TargetManager::TargetManager()
{
   VECTOR_SET_ASSOCIATION(notifyList);
   clear();
}

void TargetManager::clear()
{
   mFreeCount = MaxTargets;
   mLastSensedObject = 0;
   mSensorGroupCount = 0;
   U32 i;
   for(i = 0; i < TargetFreeMaskSize; i++)
      mFreeMask[i] = 0;
   for(i = 0; i < MaxTargets; i++)
   {
      mTargets[i].clear();
      mClientTargets[i].clear();
      mClientAudioHandles[i] = NULL_AUDIOHANDLE;
   }

   // reserve team targets
   mFreeCount -= 32;
   mFreeMask[0] = 0xffffffff;

   // setup the sensorgroups and sensorgroup masks
   for(i = 0; i < 32; i++)
   {
      mSensorInfoArray[i].clear();

      // team targets
      mTargets[i].sensorGroup = mClientTargets[i].sensorGroup = i;

      // default teams to be friendly, visible, and can listen to themselves
      mSensorGroupListenMask[i] = mSensorGroupAlwaysVisMask[i] = mSensorGroupFriendlyMask[i] = (1 << i);
      mSensorGroupNeverVisMask[i] = 0;
   }
}

//-------------------------------------------------------------------------
class SensorGroupColorEvent : public NetEvent
{
   public:

      U32         mUpdateMask;
      U32         mSensorGroup;
      ColorI      mColors[32];

      SensorGroupColorEvent(U32 sensorGroup = 0, U32 updateMask = 0)
      {
         AssertFatal(sensorGroup < 32, "SensorGroupColorEvent:: invalid sensor group");
         
         mSensorGroup = sensorGroup;
         mUpdateMask = updateMask;
         dMemcpy(&mColors, &gTargetManager->mSensorInfoArray[mSensorGroup].groupColor, 32 * sizeof(ColorI));
      }
   
      void pack(NetConnection *, BitStream *bstream)
      {
         bstream->writeInt(mSensorGroup, 5);
         bstream->write(mUpdateMask);
         
         for(U32 i = 0; i < 32; i++)
            if((1<<i) & mUpdateMask)
               if(bstream->writeFlag(mColors[i] != TargetManager::SensorInfo::smDefaultColor))
               {
                  bstream->write(mColors[i].red);
                  bstream->write(mColors[i].green);
                  bstream->write(mColors[i].blue);
                  bstream->write(mColors[i].alpha);
               }
      }

      void write(NetConnection *conn, BitStream *bstream)
      {
         pack(conn, bstream);
      }
   
      void unpack(NetConnection *, BitStream *bstream)
      {
         mSensorGroup = bstream->readInt(5);
         bstream->read(&mUpdateMask);
         
         for(U32 i = 0; i < 32; i++)
            if((1<<i) & mUpdateMask)
            {
               if(bstream->readFlag())
               {
                  bstream->read(&mColors[i].red);
                  bstream->read(&mColors[i].green);
                  bstream->read(&mColors[i].blue);
                  bstream->read(&mColors[i].alpha);
               }
               else
                  mColors[i] = TargetManager::SensorInfo::smDefaultColor;
            }
      }
   
      void process(NetConnection *)
      {
         TargetManager::SensorInfo * sensorInfo = &gTargetManager->mSensorInfoArray[mSensorGroup];
         for(U32 i = 0; i < 32; i++)
            if((1<<i) & mUpdateMask)
               sensorInfo->groupColor[i] = mColors[i];
      }
      
      DECLARE_CONOBJECT(SensorGroupColorEvent);
};

IMPLEMENT_CO_CLIENTEVENT_V1(SensorGroupColorEvent);

void TargetManager::clientSensorGroupChanged(NetConnection * client, U32 newGroup)
{
   if(dynamic_cast<GameConnection*>(client) && static_cast<GameConnection*>(client)->isAIControlled())
      return;
   client->postNetEvent(new SensorGroupColorEvent(newGroup, 0xffffffff));
}

ColorI TargetManager::getSensorGroupColor(U32 sensorGroup, U32 colorGroup)
{
   AssertFatal((sensorGroup < 32) && (colorGroup < 32), "TagetManager:: invalidSensorGroup");
   return(mSensorInfoArray[sensorGroup].groupColor[colorGroup]);
}

void TargetManager::setSensorGroupColor(U32 sensorGroup, U32 updateMask, ColorI & color)
{
   AssertFatal((sensorGroup >= 0) && (sensorGroup < 32), "TargetManager:: invalid sensor group");
   SensorInfo * sensorInfo = &mSensorInfoArray[sensorGroup];
   
   for(U32 i = 0; i < 32; i++)
      if((1 << i) & updateMask)
         sensorInfo->groupColor[i] = color;

   for(NetConnection * con = NetConnection::getConnectionList(); con; con = con->getNext())
   {
      if(con->isServerConnection())
         continue;

      GameConnection * gc = dynamic_cast<GameConnection*>(con);
      if(!gc)
         continue;
         
      if(gc->isAIControlled() || (gc->getSensorGroup() != sensorGroup))
         continue;
         
      gc->postNetEvent(new SensorGroupColorEvent(sensorGroup, updateMask));
   }
}

//-----------------------------------------------------------------------------
// an '_' as the first char for the string excludes it
bool TargetManager::getGameName(S32 target, char * buf, S32 bufSize, bool server)
{
   if(target < 0 || target >= MaxTargets)
      return(false);

   TargetInfo * targInfo = server ? gTargetManager->getServerTarget(target) :
                                    gTargetManager->getClientTarget(target);

   if(!targInfo->allocated)
      return(false);

   const char * name = gNetStringTable->lookupString(targInfo->nameTag);
   const char * type = gNetStringTable->lookupString(targInfo->typeTag);

   // game name = 'name type'
   if(name && name[0] && (name[0] != '_'))
   {
      if(type && type[0] && (type[0] != '_'))
         dSprintf(buf, bufSize, "%s %s", name, type);
      else
         dSprintf(buf, bufSize, "%s", name);
   }
   else if(type && type[0] && (type[0] != '_'))
      dSprintf(buf, bufSize, "%s", type);

   return(true);
}

//-----------------------------------------------------------------------------
void TargetManager::newClient(NetConnection *client)
{
   if(dynamic_cast<GameConnection*>(client) && static_cast<GameConnection*>(client)->isAIControlled())
      return;

   for(U32 i = 0; i < TargetFreeMaskSize; i++)
   {
      if(mFreeMask[i] == 0)
         continue;

      for(U32 j = 0; j < 32; j++)
      {
         if(mFreeMask[i] & (1 << j))
         {
            TargetInfo &targ = mTargets[(i << 5) + j];
            client->checkString(targ.nameTag);
            client->checkString(targ.skinTag);
            client->checkString(targ.voiceTag);
            client->checkString(targ.typeTag);

            client->postNetEvent(new TargetInfoEvent( (i << 5) + j, targ.nameTag, 
                    targ.skinTag, targ.voiceTag, targ.typeTag, targ.sensorGroup, 
                    bool(targ.shapeBaseData) ? targ.shapeBaseData->getId() : 0, targ.renderFlags, targ.voicePitch));
         }
      }
   }
}

S32 TargetManager::allocTarget(U32 nameTag, U32 skinTag, U32 voiceTag, U32 typeTag, U32 sensorGroup, U32 dataBlockId, F32 voicePitch, U32 prefSkin)
{
   AssertFatal(sensorGroup < 32, "TargetManager::allocTarget: invalid sensorGroup");

   // find a free mask
   if(mFreeCount == 0)
      return -1;
   S32 targ = -1;
   for(U32 i = 0; i < TargetFreeMaskSize; i++)
   {
      if(mFreeMask[i] == 0xFFFFFFFF)
         continue;
      for(U32 j = 0; j < 32; j++)
      {
         if(!( mFreeMask[i] & (1 << j) ))
         {
            targ = (i << 5) + j;
            mFreeMask[i] |= (1 << j);
            goto done;
         }
      }
   }
done:
   AssertFatal(targ != -1, "Invalid target index.");
   mFreeCount--;

   mTargets[targ].allocated = true;   
   mTargets[targ].nameTag = nameTag;
   mTargets[targ].skinTag = skinTag;
   mTargets[targ].voiceTag = voiceTag;
   mTargets[targ].typeTag = typeTag;
   mTargets[targ].skinPrefTag = prefSkin;
   mTargets[targ].sensorGroup = sensorGroup;
   mTargets[targ].renderFlags = 0;
   mTargets[targ].voicePitch = voicePitch;

   // vis/friend masks default to team values
   mTargets[targ].sensorAlwaysVisMask = mSensorGroupAlwaysVisMask[sensorGroup];
   mTargets[targ].sensorNeverVisMask = mSensorGroupNeverVisMask[sensorGroup];
   mTargets[targ].sensorFriendlyMask = mSensorGroupFriendlyMask[sensorGroup];

   mTargets[targ].sensorFlags = 0;
   mTargets[targ].sensorVisMask = 0;

   mTargets[targ].shapeBaseData = dataBlockId ? dynamic_cast<ShapeBaseData*>(Sim::findObject(dataBlockId)) : 0;

   updateTarget(targ, nameTag, skinTag, voiceTag, typeTag, sensorGroup, dataBlockId, 0, voicePitch, prefSkin);
   return targ;
}

void TargetManager::updateTarget(S32 targ, S32 nameTag, S32 skinTag, S32 voiceTag, S32 typeTag, S32 sensorGroup, S32 dataBlockId, S32 renderFlags, F32 voicePitch, U32 prefSkin)
{
   for(NetConnection *conn = NetConnection::getConnectionList(); conn; conn = conn->getNext())
   {
      if(conn->isServerConnection())
         continue;

      if (dynamic_cast<GameConnection*>(conn) && static_cast<GameConnection*>(conn)->isAIControlled())
         continue;

      if(nameTag != -1)
         conn->checkString(nameTag);
      if(skinTag != -1)
         conn->checkString(skinTag);
      if(voiceTag != -1)
         conn->checkString(voiceTag);
      if(typeTag != -1)
         conn->checkString(typeTag);
      if(prefSkin != -1)
         conn->checkString(prefSkin);
      conn->postNetEvent(new TargetInfoEvent(targ, nameTag, skinTag, voiceTag, typeTag, sensorGroup, dataBlockId, renderFlags, voicePitch, prefSkin));
   }
}

void TargetManager::freeTarget(S32 target)
{
   AssertFatal(target >= 32 && target < MaxTargets, "Invalid target id.");

   for(NetConnection *conn = NetConnection::getConnectionList(); conn; conn = conn->getNext())
   {
      if(conn->isServerConnection())
         continue;
      if (dynamic_cast<GameConnection*>(conn) && static_cast<GameConnection*>(conn)->isAIControlled())
         continue;
      conn->postNetEvent(new TargetFreeEvent(target));
   }

   for(U32 i = 0; i < 32; i++)
      mSensorInfoArray[i].setSensorVisible(target, false);

   // notify the target object
   if(bool(mTargets[target].targetObject))
      mTargets[target].targetObject->targetInfoChanged(0);

   mTargets[target].clear(false);

   // update the free mask/count
   U32 index = target >> 5;
   U32 bit = target & 0x1F;
   if(!(mFreeMask[index] & (1 << bit)))
      return;
   mFreeMask[index] &= ~(1 << bit);
   mFreeCount++;
}

//--------------------------------------------------------------------------
// Class ResetClientTargetsEvent: HUDTargetList will be notified of clear
// - overloaded for HUDTargetList use as well
// - needs to reset the connections visible target list on pack because
//   the connection gets packed (and can change the visibility of items) before
//   the event goes through
//--------------------------------------------------------------------------
class ResetClientTargetsEvent : public NetEvent
{
   private:
      bool mClientTargetsOnly;
      
   public:   
      ResetClientTargetsEvent(bool clientTargetsOnly = false)  { mClientTargetsOnly = clientTargetsOnly; }
      void pack(NetConnection *con , BitStream * bstream)
         { 
            bstream->writeFlag(mClientTargetsOnly); 
            if(!mClientTargetsOnly && dynamic_cast<GameConnection*>(con))
               static_cast<GameConnection*>(con)->resetVisibleMasks();
         }
      void write(NetConnection * con, BitStream * bstream)     { pack(con, bstream); }
      void unpack(NetConnection *, BitStream * bstream)        { mClientTargetsOnly = bstream->readFlag(); }
      void process(NetConnection * con)                        { mClientTargetsOnly ? gTargetList->targetsCleared() : gTargetManager->resetClient(con); }

      DECLARE_CONOBJECT(ResetClientTargetsEvent);
};
IMPLEMENT_CO_CLIENTEVENT_V1(ResetClientTargetsEvent);

void TargetManager::resetClient(NetConnection * con)
{
   // client?
   if(con->isServerConnection())
   {
      // clear all the target infos
      for(U32 i = 0; i < MaxTargets; i++)
      {
         // clear objects
         if(bool(mClientTargets[i].targetObject))
            mClientTargets[i].targetObject->targetInfoChanged(0);

         // clear audio
         if(mClientAudioHandles[i] != NULL_AUDIOHANDLE)
            alxStop(mClientAudioHandles[i]);
         mClientAudioHandles[i] = NULL_AUDIOHANDLE;

         mClientTargets[i].clear();
      }

      // notify objects that targets have been cleared
      notifyTargetsCleared();
   }
   else
      con->postNetEvent( new ResetClientTargetsEvent(false) );
}

void TargetManager::reset()
{
   // notify the clients to clear their targets
   for(NetConnection * con = NetConnection::getConnectionList(); con; con = con->getNext())
   {
      if(con->isServerConnection())
         continue;

      if (dynamic_cast<GameConnection*>(con) && static_cast<GameConnection*>(con)->isAIControlled())
         continue;

      resetClient(con);
   }
   // clear objects
   for(U32 i = 0; i < MaxTargets; i++)
      if(bool(mTargets[i].targetObject))
         mTargets[i].targetObject->targetInfoChanged(0);

   // reset all the targetmanager info
   clear();
}

//--------------------------------------------------------------------------
TargetInfo *TargetManager::getClientTarget(S32 target)
{
   AssertFatal(target >= 0 && target < MaxTargets, "Invalid target id.");
   return mClientTargets + target;
}

TargetInfo *TargetManager::getServerTarget(S32 target)
{
   AssertFatal(target >= 0 && target < MaxTargets, "Invalid target id.");
   return mTargets + target;
}

// TargetObject: ------------------------------------------------------------
// team targets can have multiple objects associtated with them (though, they
// will never have the target->obj set)
void TargetManager::setTargetObject(TargetInfo * target, GameBase *object)
{
   AssertFatal(target, "TargetManager::setTargetObject: invalid target object");
   
   // don't notify current object that it's target has gone bye-bye
   target->targetObject = object;
}

// Name: --------------------------------------------------------------------
S32 TargetManager::getTargetName(S32 target)
{
   AssertFatal(target >= 0 && target < MaxTargets, "Invalid target id.");
   return(mTargets[target].nameTag);
}

void TargetManager::setTargetName(S32 target, U32 nameTag)
{
   AssertFatal(target >= 0 && target < MaxTargets, "Invalid target id.");
   if(mTargets[target].nameTag == nameTag)
      return;
   mTargets[target].nameTag = nameTag;
   updateTarget(target, nameTag, -1, -1, -1, -1, -1, -1, -1, -1);
}

// Skin: --------------------------------------------------------------------
S32 TargetManager::getTargetSkin(S32 target)
{
   AssertFatal(target >= 0 && target < MaxTargets, "Invalid target id.");
   return(mTargets[target].skinTag);
}

void TargetManager::setTargetSkin(S32 target, U32 skinTag)
{
   AssertFatal(target >= 0 && target < MaxTargets, "Invalid target id.");
   if(mTargets[target].skinTag == skinTag)
      return;
   mTargets[target].skinTag = skinTag;
   updateTarget(target, -1, skinTag, -1, -1, -1, -1, -1, -1, -1);
}

// Voice: --------------------------------------------------------------------
S32 TargetManager::getTargetVoice(S32 target)
{
   AssertFatal(target >= 0 && target < MaxTargets, "Invalid target id.");
   return(mTargets[target].voiceTag);
}

void TargetManager::setTargetVoice(S32 target, U32 voiceTag)
{
   AssertFatal(target >= 0 && target < MaxTargets, "Invalid target id.");
   if(mTargets[target].voiceTag == voiceTag)
      return;
   mTargets[target].voiceTag = voiceTag;
   updateTarget(target, -1, -1, voiceTag, -1, -1, -1, -1, -1, -1);
}

// Type: --------------------------------------------------------------------
S32 TargetManager::getTargetType(S32 target)
{
   AssertFatal(target >= 0 && target < MaxTargets, "Invalid target id.");
   return(mTargets[target].typeTag);
}

void TargetManager::setTargetType(S32 target, U32 typeTag)
{
   AssertFatal(target >= 0 && target < MaxTargets, "Invalid target id.");
   if(mTargets[target].typeTag == typeTag)
      return;
   mTargets[target].typeTag = typeTag;
   updateTarget(target, -1, -1, -1, typeTag, -1, -1, -1, -1, -1);
}

// SensorGroup: -------------------------------------------------------------
S32 TargetManager::getTargetSensorGroup(S32 target)
{
   AssertFatal(target >= 0 && target < MaxTargets, "Invalid target id.");
   return(mTargets[target].sensorGroup);
}

void TargetManager::setTargetSensorGroup(S32 target, U32 sensorGroup)
{
   AssertFatal(target >= 32 && target < MaxTargets, "Invalid target id.");
   AssertFatal(sensorGroup < 32, "Invalid sensor group.");
   if(mTargets[target].sensorGroup == sensorGroup)
      return;
   mTargets[target].sensorGroup = sensorGroup;

   mTargets[target].sensorAlwaysVisMask = mSensorGroupAlwaysVisMask[sensorGroup];
   mTargets[target].sensorNeverVisMask = mSensorGroupNeverVisMask[sensorGroup];
   mTargets[target].sensorFriendlyMask = mSensorGroupFriendlyMask[sensorGroup];
   updateTarget(target, -1, -1, -1, -1, sensorGroup, -1, -1, -1, -1);
}

// voicePitch: --------------------------------------------------------------
F32 TargetManager::getTargetVoicePitch(S32 target)
{
   AssertFatal(target >= 0 && target < MaxTargets, "Invalid target id.");
   return(mTargets[target].voicePitch);
}

void TargetManager::setTargetVoicePitch(S32 target, F32 voicePitch)
{
   AssertFatal(target >= 0 && target < MaxTargets, "Invalid target id.");
   if(mTargets[target].voicePitch == voicePitch)
      return;
   if (voicePitch < 0.5f || voicePitch > 2.0f)
      mTargets[target].voicePitch = 1.0f;
   else
      mTargets[target].voicePitch = voicePitch;
   updateTarget(target, -1, -1, -1, -1, -1, -1, -1, voicePitch, -1);
}

// Misc: --------------------------------------------------------------------
void TargetManager::setTargetRenderMask(S32 target, U32 mask)
{
   AssertFatal(target >= 32 && target < MaxTargets, "Invalid target id.");
   mTargets[target].renderFlags = mask & ((1 << TargetInfo::NumRenderBits) - 1);
   updateTarget(target, -1, -1, -1, -1, -1, -1, mTargets[target].renderFlags, -1, -1);
}

void TargetManager::setTargetShapeBaseData(S32 target, ShapeBaseData * data)
{
   AssertFatal(target >= 0 && target < MaxTargets, "Invalid target id.");
   mTargets[target].shapeBaseData = data;
   updateTarget(target, -1, -1, -1, -1, -1, data ? data->getId() : 0, -1, -1, -1);
}

// TargetAlwaysVisMask: -----------------------------------------------------
U32 TargetManager::getTargetAlwaysVisMask(S32 target)
{
   AssertFatal(target >= 0 && target < MaxTargets, "Invalid target id.");
   return(mTargets[target].sensorAlwaysVisMask);
}

void TargetManager::setTargetAlwaysVisMask(S32 target, U32 mask)
{
   AssertFatal(target >= 0 && target < MaxTargets, "Invalid target id.");
   mTargets[target].sensorAlwaysVisMask = mask;
}

// TargetNeverVisMask: ------------------------------------------------------
U32 TargetManager::getTargetNeverVisMask(S32 target)
{
   AssertFatal(target >= 0 && target < MaxTargets, "Invalid target id.");
   return(mTargets[target].sensorNeverVisMask);
}

void TargetManager::setTargetNeverVisMask(S32 target, U32 mask)
{
   AssertFatal(target >= 0 && target < MaxTargets, "Invalid target id.");
   mTargets[target].sensorNeverVisMask = mask;
}

// TargetFriendlyMask: ------------------------------------------------------
U32 TargetManager::getTargetFriendlyMask(S32 target)
{
   AssertFatal(target >= 0 && target < MaxTargets, "Invalid target id.");
   return(mTargets[target].sensorFriendlyMask);
}

void TargetManager::setTargetFriendlyMask(S32 target, U32 mask)
{
   AssertFatal(target >= 0 && target < MaxTargets, "Invalid target id.");
   mTargets[target].sensorFriendlyMask = mask;
}

// all targets inherit group mask changes
void TargetManager::updateSensorGroupMask(U32 sensorGroup, U32 mask, U32 maskType)
{
   for(U32 i = 0; i < TargetFreeMaskSize; i++)
   {
      if(mFreeMask[i] == 0)
         continue;

      for(U32 j = 0; j < 32; j++)
      {
         if(mFreeMask[i] & (1 << j))
         {
            TargetInfo &targ = mTargets[(i << 5) + j];
            if(targ.sensorGroup == sensorGroup)
            {
               U32 * targMask = &targ.sensorAlwaysVisMask;
               if(maskType == NeverVisMask)
                  targMask = &targ.sensorNeverVisMask;
               else if(maskType == FriendlyMask)
                  targMask = &targ.sensorFriendlyMask;
               *targMask = mask;
            }
         }
      }
   }
}

// SensorGroupAlwaysVisMask: ------------------------------------------------
U32 TargetManager::getSensorGroupAlwaysVisMask(U32 sensorGroup)
{
   AssertFatal(sensorGroup < 32, "TargetManager::getSensorGroupAlwaysVisMask: invalid sensor group");
   return(mSensorGroupAlwaysVisMask[sensorGroup]);
}

void TargetManager::setSensorGroupAlwaysVisMask(U32 sensorGroup, U32 mask)
{
   AssertFatal(sensorGroup < 32, "TargetManager::setSensorGroupAlwaysVisMask: invalid sensor group");
   updateSensorGroupMask(sensorGroup, mask, AlwaysVisMask);
   mSensorGroupAlwaysVisMask[sensorGroup] = mask;
}

// SensorGroupNeverVisMask: -------------------------------------------------
U32 TargetManager::getSensorGroupNeverVisMask(U32 sensorGroup)
{
   AssertFatal(sensorGroup < 32, "TargetManager::getSensorGroupNeverVisMask: invalid sensor group");
   return(mSensorGroupNeverVisMask[sensorGroup]);
}

void TargetManager::setSensorGroupNeverVisMask(U32 sensorGroup, U32 mask)
{
   AssertFatal(sensorGroup < 32, "TargetManager::setSensorGroupNeverVisMask: invalid sensor group");
   updateSensorGroupMask(sensorGroup, mask, NeverVisMask);
   mSensorGroupNeverVisMask[sensorGroup] = mask;
}

// SensorGroupFriendlyMask: -------------------------------------------------
U32 TargetManager::getSensorGroupFriendlyMask(U32 sensorGroup)
{
   AssertFatal(sensorGroup < 32, "TargetManager::getSensorGroupFriendlyMask: invalid sensor group");
   return(mSensorGroupFriendlyMask[sensorGroup]);
}

void TargetManager::setSensorGroupFriendlyMask(U32 sensorGroup, U32 mask)
{
   AssertFatal(sensorGroup < 32, "TargetManager::setSensorGroupFriendlyMask: invalid sensor group");
   updateSensorGroupMask(sensorGroup, mask, FriendlyMask);
   mSensorGroupFriendlyMask[sensorGroup] = mask;
}

// Listen state: ------------------------------------------------------------
U32 TargetManager::getSensorGroupListenMask(U32 sensorGroup)
{
   AssertFatal(sensorGroup < 32, "TargetManager::getSensorGroupListenMask: invalid sensor group");
   return(mSensorGroupListenMask[sensorGroup]);
}

void TargetManager::setSensorGroupListenMask(U32 sensorGroup, U32 mask)
{
   AssertFatal(sensorGroup < 32, "TargetManager::setSensorGroupListenMask: invalid sensor group");
   mSensorGroupListenMask[sensorGroup] = mask;
}

// IsTarget(...): -----------------------------------------------------------
bool TargetManager::isTargetFriendly(S32 target, U32 sensorGroup)
{
   AssertFatal(target >= 0 && target < MaxTargets, "Invalid target id.");
   AssertFatal(sensorGroup < 32, "Invalid sensor group");
   return((mTargets[target].sensorFriendlyMask & (1 << sensorGroup)) != 0);
}

bool TargetManager::isTargetVisible(S32 target, U32 sensorGroup)
{
   AssertFatal(target >= 0 && target < MaxTargets, "Invalid target id.");
   AssertFatal(sensorGroup < 32, "Invalid sensor group");

   U32 groupMask = (1 << sensorGroup);

   if(mTargets[target].sensorNeverVisMask & groupMask)
      return(false);

   if(mTargets[target].sensorAlwaysVisMask & groupMask)
      return(true);

   return((mTargets[target].sensorVisMask & groupMask) != 0);
}

//---------------------------------------------------------------------------
// TargetManager console access:
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Helpers...
static S32 getValidTarget(S32 target, const char * funcStr, bool excludeTeam)
{
   if((target < 0) || (target >= TargetManager::MaxTargets))
   {
      Con::errorf(ConsoleLogEntry::General, "TargetManager::%s: invalid target index [%d]", funcStr, target);
      return(-1);
   }

   if(excludeTeam && target < 32)
   {
      Con::errorf(ConsoleLogEntry::General, "TargetManager::%s: cannot change attribute on team target [%d]", funcStr, target);
      return(-1);
   }

   TargetInfo * targ = gTargetManager->getServerTarget(target);
   if(!targ->allocated && (target >= 32))
   {
      Con::errorf(ConsoleLogEntry::General, "TargetManager::%s: cannot change attribute on unallocated target [%d]", funcStr, target);
      return(-1);
   }

   return(target);
}

static S32 getValidSensorGroup(S32 group, const char * funcStr)
{
   if(group < 0 || group >= 32)
   {
      Con::errorf(ConsoleLogEntry::General, "TargetManager::%s: invalid sensor group [%d]", funcStr, group);
      return(-1);
   }
   return(group);
}

static F32 getValidVoicePitch(F32 pitch, const char * funcStr)
{
   if(pitch < 0.5f || pitch > 2.0f)
   {
      Con::errorf(ConsoleLogEntry::General, "TargetManager::%s: Invalid pitch [%d]", funcStr, pitch);
      return (1.0f);
   }
   else
      return(pitch);
}

//---------------------------------------------------------------------------
static void cReset(SimObject *, S32, const char **)
{
   gTargetManager->reset();
}

static void cResetClientTargets(SimObject *, S32, const char ** argv)
{
   NetConnection * client = dynamic_cast<NetConnection*>(Sim::findObject(dAtoi(argv[1])));
   if(client)
   {
      if(dynamic_cast<GameConnection*>(client) && static_cast<GameConnection*>(client)->isAIControlled())
         return;

      client->postNetEvent( new ResetClientTargetsEvent(dAtob(argv[2])) );
   }
}

static void cSendTargetsToClient(SimObject *, S32, const char ** argv)
{
   NetConnection * client = dynamic_cast<NetConnection*>(Sim::findObject(dAtoi(argv[1])));
   if(client)
      gTargetManager->newClient(client);
}

static S32 cAllocTarget(SimObject *, S32 argc, const char **argv)
{
   U32 nameTag = 0;
   U32 skinTag = 0;
   U32 voiceTag = 0;
   U32 typeTag = 0;
   U32 prefSkinTag = 0;

   // check if tagprefix has already been removed
   if(argv[1][0] == StringTagPrefixByte)
      nameTag = dAtoi(argv[1] + 1);
   else if(argv[1][0])
      nameTag = dAtoi(argv[1]);
   if(argv[2][0] == StringTagPrefixByte)
      skinTag = dAtoi(argv[2] + 1);
   else if(argv[2][0])
      skinTag = dAtoi(argv[2]);
   if(argv[3][0] == StringTagPrefixByte)
      voiceTag = dAtoi(argv[3] + 1);
   else if(argv[3][0])
      voiceTag = dAtoi(argv[3]);
   if(argv[4][0] == StringTagPrefixByte)
      typeTag = dAtoi(argv[4] + 1);
   else if(argv[4][0])
      typeTag = dAtoi(argv[4]);
   U32 sensorGroup = dAtoi(argv[5]);
   U32 dataBlockId = dAtoi(argv[6]);
   F32 voicePitch = dAtof(argv[7]);
   if (voicePitch < 0.5 || voicePitch > 2.0)
      voicePitch = 1.0;
   if(argc == 9)
   {
      if(argv[8][0] == StringTagPrefixByte)
         prefSkinTag = dAtoi(argv[8] + 1);
      else if(argv[8][0])
         prefSkinTag = dAtoi(argv[8]);
   }
   return gTargetManager->allocTarget(nameTag, skinTag, voiceTag, typeTag, sensorGroup, dataBlockId, voicePitch, prefSkinTag);
}

static void cFreeTarget(SimObject *, S32, const char **argv)
{
   if(!gTargetManager)
      return;
   S32 target = getValidTarget(dAtoi(argv[1]), "cFreeTarget", true);
   if(target == -1)
      return;
   gTargetManager->freeTarget(target);
}

// Name: --------------------------------------------------------------------
// - queries the server's game name for this target
static const char * cGetTargetGameName(SimObject *, S32, const char ** argv)
{
   S32 target = getValidTarget(dAtoi(argv[1]), "cGetTargetGameName", false);
   if(target == -1)
      return("");

   char * ret = Con::getReturnBuffer(128);
   if(!gTargetManager->getGameName(target, ret, 128, true))
      return("");

   return(ret);
}

static S32 cGetTargetName(SimObject *, S32, const char ** argv)
{
   S32 target = getValidTarget(dAtoi(argv[1]), "cGetTargetName", false);
   if(target == -1)
      return(-1);
   return(gTargetManager->getTargetName(target));
}

static void cSetTargetName(SimObject *, S32, const char ** argv)
{
   S32 target = getValidTarget(dAtoi(argv[1]), "cSetTargetName", false);
   if(target == -1)
      return;
   if ( argv[2][0] == StringTagPrefixByte )
      gTargetManager->setTargetName(target, dAtoi(argv[2] + 1));
   else
      gTargetManager->setTargetName(target, dAtoi(argv[2]));
}

// Skin: --------------------------------------------------------------------
static S32 cGetTargetSkin(SimObject *, S32, const char ** argv)
{
   S32 target = getValidTarget(dAtoi(argv[1]), "cGetTargetSkin", false);
   if(target == -1)
      return(-1);
   return(gTargetManager->getTargetSkin(target));
}

static void cSetTargetSkin(SimObject *, S32, const char ** argv)
{
   S32 target = getValidTarget(dAtoi(argv[1]), "cSetTargetSkin", false);
   if(target == -1)
      return;
   if ( argv[2][0] == StringTagPrefixByte )
      gTargetManager->setTargetSkin(target, dAtoi(argv[2] + 1));
   else
      gTargetManager->setTargetSkin(target, dAtoi(argv[2]));
}

// Voice: -------------------------------------------------------------------
static S32 cGetTargetVoice(SimObject *, S32, const char ** argv)
{
   S32 target = getValidTarget(dAtoi(argv[1]), "cGetTargetVoice", false);
   if(target == -1)
      return(-1);
   return(gTargetManager->getTargetVoice(target));
}

static void cSetTargetVoice(SimObject *, S32, const char ** argv)
{
   S32 target = getValidTarget(dAtoi(argv[1]), "cSetTargetVoice", false);
   if(target == -1)
      return;
   if ( argv[2][0] == StringTagPrefixByte )
      gTargetManager->setTargetVoice(target, dAtoi(argv[2] + 1));
   else
      gTargetManager->setTargetVoice(target, dAtoi(argv[2]));
}

// Type: --------------------------------------------------------------------
static S32 cGetTargetType(SimObject *, S32, const char ** argv)
{
   S32 target = getValidTarget(dAtoi(argv[1]), "cGetTargetType", false);
   if(target == -1)
      return(-1);
   return(gTargetManager->getTargetType(target));
}

static void cSetTargetType(SimObject *, S32, const char ** argv)
{
   S32 target = getValidTarget(dAtoi(argv[1]), "cSetTargetType", false);
   if(target == -1)
      return;
   if ( argv[2][0] == StringTagPrefixByte )
      gTargetManager->setTargetType(target, dAtoi(argv[2] + 1));
   else
      gTargetManager->setTargetType(target, dAtoi(argv[2]));
}

// SensorGroup: -------------------------------------------------------------
static S32 cGetTargetSensorGroup(SimObject *, S32, const char ** argv)
{
   S32 target = getValidTarget(dAtoi(argv[1]), "cGetTargetSensorGroup", false);
   if(target == -1)
      return(-1);
   return(gTargetManager->getTargetSensorGroup(target));
}

static void cSetTargetSensorGroup(SimObject *, S32, const char ** argv)
{
   S32 target = getValidTarget(dAtoi(argv[1]), "cSetTargetSensorGroup", true);
   if(target == -1)
      return;
   S32 group = getValidSensorGroup(dAtoi(argv[2]), "cSetTargetSensorGroup");
   if(group == -1)
      return;
   gTargetManager->setTargetSensorGroup(target, group);
}

// VoicePitch: -------------------------------------------------------------
static F32 cGetTargetVoicePitch(SimObject *, S32, const char ** argv)
{
   S32 target = getValidTarget(dAtoi(argv[1]), "cGetTargetVoicePitch", false);
   if(target == -1)
      return(-1);
   return(gTargetManager->getTargetVoicePitch(target));
}

static void cSetTargetVoicePitch(SimObject *, S32, const char ** argv)
{
   S32 target = getValidTarget(dAtoi(argv[1]), "cSetTargetVoicePitch", true);
   if(target == -1)
      return;
   F32 pitch = getValidVoicePitch(dAtof(argv[2]), "cSetTargetVoicePitch");
   gTargetManager->setTargetVoicePitch(target, pitch);
}

// SensorGroupCount: --------------------------------------------------------
static S32 cGetSensorGroupCount(SimObject *, S32, const char **)
{
   return(gTargetManager->getSensorGroupCount());
}

static void cSetSensorGroupCount(SimObject *, S32, const char ** argv)
{
   S32 groupCount = dAtoi(argv[1]);
   if(groupCount < 0 || groupCount > 32)
   {
      Con::errorf(ConsoleLogEntry::General, "TargetManager::cSetSensorGroupCount: invalid group count [%d]", groupCount);
      return;
   }
   gTargetManager->setSensorGroupCount(groupCount);
}

// TargetAlwaysVisMask: -----------------------------------------------------
static S32 cGetTargetAlwaysVisMask(SimObject *, S32, const char ** argv)
{
   S32 target = getValidTarget(dAtoi(argv[1]), "cGetTargetAlwaysVisMask", false);
   if(target == -1)
      return(0);
   return(gTargetManager->getTargetAlwaysVisMask(target));   
}

static void cSetTargetAlwaysVisMask(SimObject *, S32, const char ** argv)
{
   S32 target = getValidTarget(dAtoi(argv[1]), "cSetTargetAlwaysVisMask", false);
   if(target == -1)
      return;
   gTargetManager->setTargetAlwaysVisMask(target, dAtoi(argv[2]));
}


// TargetNeverVisMask: ------------------------------------------------------
static S32 cGetTargetNeverVisMask(SimObject *, S32, const char ** argv)
{
   S32 target = getValidTarget(dAtoi(argv[1]), "cGetTargetNeverVisMask", false);
   if(target == -1)
      return(0);
   return(gTargetManager->getTargetNeverVisMask(target));
}

static void cSetTargetNeverVisMask(SimObject *, S32, const char ** argv)
{
   S32 target = getValidTarget(dAtoi(argv[1]), "cSetTargetNeverVisMask", false);
   if(target == -1)
      return;
   gTargetManager->setTargetNeverVisMask(target, dAtoi(argv[2]));
}

// TargetFriendlyMask: ------------------------------------------------------
static S32 cGetTargetFriendlyMask(SimObject *, S32, const char ** argv)
{
   S32 target = getValidTarget(dAtoi(argv[1]), "cGetTargetFriendlyMask", false);
   if(target == -1)
      return(0);
   return(gTargetManager->getTargetFriendlyMask(target));
}

static void cSetTargetFriendlyMask(SimObject *, S32, const char ** argv)
{
   S32 target = getValidTarget(dAtoi(argv[1]), "cSetTargetFriendlyMask", false);
   if(target == -1)
      return;
   gTargetManager->setTargetFriendlyMask(target, dAtoi(argv[2]));
}

// SensorGroupAlwaysVisMask: ------------------------------------------------
static S32 cGetSensorGroupAlwaysVisMask(SimObject *, S32, const char ** argv)
{
   S32 group = getValidSensorGroup(dAtoi(argv[1]), "cGetSensorGroupAlwaysVisMask");
   if(group == -1)
      return(0);
   return(gTargetManager->getSensorGroupAlwaysVisMask(group));
}

static void cSetSensorGroupAlwaysVisMask(SimObject *, S32, const char ** argv)
{
   S32 group = getValidSensorGroup(dAtoi(argv[1]), "cSetSensorGroupAlwaysVisMask");
   if(group == -1)
      return;
   gTargetManager->setSensorGroupAlwaysVisMask(group, dAtoi(argv[2]));
}

// SensorGroupNeverVisMask: -------------------------------------------------
static S32 cGetSensorGroupNeverVisMask(SimObject *, S32, const char ** argv)
{
   S32 group = getValidSensorGroup(dAtoi(argv[1]), "cGetSensorGroupNeverVisMask");
   if(group == -1)
      return(0);
   return(gTargetManager->getSensorGroupNeverVisMask(group));
}

static void cSetSensorGroupNeverVisMask(SimObject *, S32, const char ** argv)
{
   S32 group = getValidSensorGroup(dAtoi(argv[1]), "cSetSensorGroupNeverVisMask");
   if(group == -1)
      return;
   gTargetManager->setSensorGroupNeverVisMask(group, dAtoi(argv[2]));
}

// SensorGroupFriendlyMask: -------------------------------------------------
static S32 cGetSensorGroupFriendlyMask(SimObject *, S32, const char ** argv)
{
   S32 group = getValidSensorGroup(dAtoi(argv[1]), "cGetSensorGroupFriendlyMask");
   if(group == -1)
      return(0);
   return(gTargetManager->getSensorGroupFriendlyMask(group));
}

static void cSetSensorGroupFriendlyMask(SimObject *, S32, const char ** argv)
{
   S32 group = getValidSensorGroup(dAtoi(argv[1]), "cSetSensorGroupFriendlyMask");
   if(group == -1)
      return;
   gTargetManager->setSensorGroupFriendlyMask(group, dAtoi(argv[2]));
}

// SensorGroupListenMask: ---------------------------------------------------
static S32 cGetSensorGroupListenMask(SimObject *, S32, const char ** argv)
{
   S32 group = getValidSensorGroup(dAtoi(argv[1]), "cGetSensorGroupListenMask");
   if(group == -1)
      return(0);
   return(gTargetManager->getSensorGroupListenMask(group));
}

static void cSetSensorGroupListenMask(SimObject *, S32, const char ** argv)
{
   S32 group = getValidSensorGroup(dAtoi(argv[1]), "cSetSensorGroupListenMask");
   if(group == -1)
      return;
   gTargetManager->setSensorGroupListenMask(group, dAtoi(argv[2]));
}

// IsTarget(...): -----------------------------------------------------------
static bool cIsTargetFriendly(SimObject *, S32, const char ** argv)
{
   S32 target = getValidTarget(dAtoi(argv[1]), "cIsTargetFriendly", false);
   if(target == -1)
      return(false);
   S32 group = getValidSensorGroup(dAtoi(argv[2]), "cIsTargetFriendly");
   if(group == -1)
      return(false);
   return(gTargetManager->isTargetFriendly(target, group));
}

static bool cIsTargetVisible(SimObject *, S32, const char ** argv)
{
   S32 target = getValidTarget(dAtoi(argv[1]), "cIsTargetVisible", false);
   if(target == -1)
      return(false);
   S32 group = getValidSensorGroup(dAtoi(argv[2]), "cIsTargetVisible");
   if(group == -1)
      return(false);
   return(gTargetManager->isTargetVisible(target, group));
}

// Others: ------------------------------------------------------------------
static void cSetTargetSensorData(SimObject *, S32, const char **argv)
{
   S32 target = dAtoi(argv[1]);
   if(target < 0 || target >= TargetManager::MaxTargets)
      return;

   SensorData * data = dynamic_cast<SensorData*>(Sim::findObject(argv[2]));
   TargetInfo *targ = gTargetManager->getServerTarget(target);
   targ->sensorData = data;
}

static S32 cGetTargetSensorData(SimObject *, S32, const char ** argv)
{
   S32 target = dAtoi(argv[1]);
   if(target < 0 || target >= TargetManager::MaxTargets)
      return(-1);

   TargetInfo * targ = gTargetManager->getServerTarget(target);
   if(bool(targ->sensorData))
      return(targ->sensorData->getId());
   return(-1);
}

static const char * cGetTargetObject(SimObject *, S32, const char **argv)
{
   char * buf = Con::getReturnBuffer(12);
   S32 target = dAtoi(argv[1]);
   if(target < 0 || target >= TargetManager::MaxTargets || !bool(gTargetManager->mTargets[target].targetObject))
      dStrcpy(buf, "-1");
   else
      dSprintf(buf, 12, "%d", gTargetManager->mTargets[target].targetObject->getId());
   return(buf);
}

// Color info: ------------------------------------------------------------
static const char * cGetSensorGroupColor(SimObject *, S32, const char ** argv)
{
   S32 sensorGroup = getValidSensorGroup(dAtoi(argv[1]), "cGetSensorGroupColor");
   if(sensorGroup == -1)
      return("");
   
   S32 colorGroup = getValidSensorGroup(dAtoi(argv[2]), "cGetSensorGroupColor");
   if(colorGroup == -1)
      return("");
      
   char * buf = Con::getReturnBuffer(100);

   ColorI col = gTargetManager->getSensorGroupColor(sensorGroup, colorGroup);
   dSprintf(buf, 100, "%d %d %d %d", col.red, col.green, col.blue, col.alpha);
   return(buf);
}

static void cSetSensorGroupColor(SimObject *, S32, const char ** argv)
{
   S32 sensorGroup = getValidSensorGroup(dAtoi(argv[1]), "cSetSensorGroupColor");
   if(sensorGroup == -1)
      return;

   U32 r,g,b,a;
   dSscanf(argv[3], "%d %d %d %d", &r, &g, &b, &a);

   ColorI col(r,g,b,a);
   gTargetManager->setSensorGroupColor(sensorGroup, dAtoi(argv[2]), col);         
}

// DataBlock info: ----------------------------------------------------------
static S32 cGetTargetDataBlock(SimObject *, S32, const char ** argv)
{
   S32 target = getValidTarget(dAtoi(argv[1]), "cGetTargetDataBlock", true);
   if(target == -1)
      return(-1);

   TargetInfo * targ = gTargetManager->getServerTarget(target);
   if(!bool(targ->shapeBaseData))
      return(0);
   
   return(targ->shapeBaseData->getId());
}

static void cSetTargetDataBlock(SimObject *, S32, const char ** argv)
{
   S32 target = getValidTarget(dAtoi(argv[1]), "cSetTargetDataBlock", true);
   if(target == -1)
      return;
   
   ShapeBaseData * data = dynamic_cast<ShapeBaseData*>(Sim::findObject(dAtoi(argv[2])));
   gTargetManager->setTargetShapeBaseData(target, data);
}

// TargetRender: ------------------------------------------------------------
static S32 cGetTargetRenderMask(SimObject *, S32, const char ** argv)
{
   S32 target = getValidTarget(dAtoi(argv[1]), "cGetTargetRender", false);
   if(target == -1)
      return(-1);
   
   return(gTargetManager->getServerTarget(target)->renderFlags);
}

static void cSetTargetRenderMask(SimObject *, S32, const char ** argv)
{
   S32 target = getValidTarget(dAtoi(argv[1]), "cSetTargetRender", false);
   if(target == -1)
      return;
      
   gTargetManager->setTargetRenderMask(target, dAtoi(argv[2]));
}

// Audio: -------------------------------------------------------------------
static void cPlayTargetAudio(SimObject *, S32 argc, const char ** argv)
{
   AudioDescription * desc = dynamic_cast<AudioDescription*>(Sim::findObject(argv[3]));
   if(!desc || (desc->getId() < DataBlockObjectIdFirst) || (desc->getId() > DataBlockObjectIdLast))
   {
      Con::warnf("Invalid audio description '%s'.", argv[5]);
      return;
   }
   
   bool update;
   if(argc < 5)
      update = false;
   else
      update = dAtob(argv[4]);  

   S32 fileTag = (argv[2][0] == StringTagPrefixByte) ? dAtoi(argv[2] + 1) : dAtoi(argv[2]);
   if(!gTargetManager->playTargetAudio(dAtoi(argv[1]), fileTag, desc, update))
      Con::warnf("Failed to send target audio event to clients");
}

void TargetManager::create()
{
   gTargetManager = new TargetManager;
   gTargetList = new HUDTargetList;

   Con::addCommand("resetTargets",           cReset,                 "resetTargets()",                               1, 1);
   Con::addCommand("resetClientTargets",     cResetClientTargets,    "resetClientTargets(connection, tasksOnly)",    3, 3);

   Con::addCommand("sendTargetsToClient",    cSendTargetsToClient,   "sendTargetsToClient(connection)",              2, 2);
   Con::addCommand("allocTarget",            cAllocTarget,           "allocTarget(nameTag, skinTag, voiceTag, typeTag, sensorGroup, dataBlockId, voicePitch, [prefskin])", 8, 9);
   Con::addCommand("freeTarget",             cFreeTarget,            "freeTarget(targetId)",                         2, 2);

   Con::addCommand("getTargetGameName",      cGetTargetGameName,     "getTargetGameName(targetId)",                  2, 2);
   Con::addCommand("getTargetName",          cGetTargetName,         "getTargetName(targetId)",                      2, 2);
   Con::addCommand("setTargetName",          cSetTargetName,         "setTargetName(targetId, nameTag)",             3, 3);
   Con::addCommand("getTargetSkin",          cGetTargetSkin,         "getTargetSkin(targetId)",                      2, 2);
   Con::addCommand("setTargetSkin",          cSetTargetSkin,         "setTargetSkin(targetId, skinTag)",             3, 3);
   Con::addCommand("getTargetVoice",         cGetTargetVoice,        "getTargetVoice(targetId)",                     2, 2);
   Con::addCommand("setTargetVoice",         cSetTargetVoice,        "setTargetVoice(targetId, voiceTag)",           3, 3);
   Con::addCommand("getTargetVoicePitch",    cGetTargetVoicePitch,   "getTargetVoicePitch(targetId)",                2, 2);
   Con::addCommand("setTargetVoicePitch",    cSetTargetVoicePitch,   "setTargetVoice(targetId, voicePitch)",         3, 3);
   Con::addCommand("getTargetType",          cGetTargetType,         "getTargetType(targetId)",                      2, 2);
   Con::addCommand("setTargetType",          cSetTargetType,         "setTargetType(targetId, typeTag)",             3, 3);
   Con::addCommand("getTargetSensorGroup",   cGetTargetSensorGroup,  "getTargetSensorGroup(targetId)",               2, 2);
   Con::addCommand("setTargetSensorGroup",   cSetTargetSensorGroup,  "setTargetSensorGroup(targetId, sensorGroup)",  3, 3);

   Con::addCommand("getTargetAlwaysVisMask", cGetTargetAlwaysVisMask,   "getTargetAlwaysVisMask(target)",            2, 2);
   Con::addCommand("setTargetAlwaysVisMask", cSetTargetAlwaysVisMask,   "setTargetAlwaysVisMask(target, mask)",      3, 3);
   Con::addCommand("getTargetNeverVisMask",  cGetTargetNeverVisMask,    "getTargetNeverVisMask(target)",             2, 2);
   Con::addCommand("setTargetNeverVisMask",  cSetTargetNeverVisMask,    "setTargetNeverVisMask(target, mask)",       3, 3);
   Con::addCommand("getTargetFriendlyMask",  cGetTargetFriendlyMask,    "getTargetFriendlyMask(target)",             2, 2);            
   Con::addCommand("setTargetFriendlyMask",  cSetTargetFriendlyMask,    "setTargetFriendlyMask(target, mask)",       3, 3);

   Con::addCommand("getSensorGroupAlwaysVisMask",  cGetSensorGroupAlwaysVisMask, "getSensorGroupAlwaysVisMask(sensorGroup)",        2, 2);
   Con::addCommand("setSensorGroupAlwaysVisMask",  cSetSensorGroupAlwaysVisMask, "setSensorGroupAlwaysVisMask(sensorGroup, mask)",  3, 3);
   Con::addCommand("getSensorGroupNeverVisMask",   cGetSensorGroupNeverVisMask,  "getSensorGroupNeverVisMask(sensorGroup)",         2, 2);
   Con::addCommand("setSensorGroupNeverVisMask",   cSetSensorGroupNeverVisMask,  "setSensorGroupNeverVisMask(sensorGroup, mask)",   3, 3);
   Con::addCommand("getSensorGroupFriendlyMask",   cGetSensorGroupFriendlyMask,  "getSensorGroupFriendlyMask(sensorGroup)",         2, 2);
   Con::addCommand("setSensorGroupFriendlyMask",   cSetSensorGroupFriendlyMask,  "setSensorGroupFriendlyMask(sensorGroup, mask)",   3, 3);

   Con::addCommand("getSensorGroupListenMask",     cGetSensorGroupListenMask,    "getSensorGroupListenMask(sensorGroup)",           2, 2);
   Con::addCommand("setSensorGroupListenMask",     cSetSensorGroupListenMask,    "setSensorGroupListenMask(sensorGroup, mask)",     3, 3);

   Con::addCommand("isTargetFriendly",             cIsTargetFriendly,            "isTargetFriendly(target, sensorGroup)",           3, 3);
   Con::addCommand("isTargetVisible",              cIsTargetVisible,             "isTargetVisible(target, sensorGroup)",            3, 3);

   Con::addCommand("getSensorGroupCount",    cGetSensorGroupCount,   "getSensorGroupCount()",                                 1, 1);
   Con::addCommand("setSensorGroupCount",    cSetSensorGroupCount,   "setSensorGroupCount(count)",                            2, 2);   

   Con::addCommand("setTargetSensorData",    cSetTargetSensorData,   "setTargetSensorData(targetId, sensorData)",             3, 3);
   Con::addCommand("getTargetSensorData",    cGetTargetSensorData,   "getTargetSensorData(targetId)",                         2, 2);
   Con::addCommand("getTargetObject",        cGetTargetObject,       "getTargetObject(targetId)",                             2, 2);

   Con::addCommand("getSensorGroupColor",    cGetSensorGroupColor,   "getSensorGroupColor(sensorGroup, colorGroup)",          3, 3);
   Con::addCommand("setSensorGroupColor",    cSetSensorGroupColor,   "setSensorGroupColor(sensorGroup, groupMask, color)",    4, 4);

   Con::addCommand("setTargetDataBlock",     cSetTargetDataBlock,    "setTargetDataBlock(targetId, dataBlockId)",             3, 3);
   Con::addCommand("getTargetDataBlock",     cGetTargetDataBlock,    "getTargetDataBlock(targetId)",                          2, 2);

   Con::addCommand("getTargetRenderMask",    cGetTargetRenderMask,   "getTargetRender(targetId)",                             2, 2);
   Con::addCommand("setTargetRenderMask",    cSetTargetRenderMask,   "setTargetRender(targetId, mask)",                       3, 3);

   Con::setIntVariable("$TargetInfo::HudRenderStart",       TargetInfo::HudRenderStart);
   Con::setIntVariable("$TargetInfo::NumHudRenderImages",   TargetInfo::NumHudRenderImages);
   Con::setIntVariable("$TargetInfo::CommanderListRender",  TargetInfo::CommanderListRender);

   Con::addCommand("playTargetAudio",        cPlayTargetAudio,       "playTargetAudio(target, fileTag, desc, update)",        5, 5);
}

void TargetManager::destroy()
{
   delete gTargetList;
   gTargetList = 0;

   delete gTargetManager;
   gTargetManager = 0;
}

void TargetManager::writeDemoStartBlock(ResizeBitStream *stream, GameConnection *)
{
   TargetInfo *targ = mClientTargets;
   for(U32 i = 0; i < MaxTargets; i++)
   {
      if(stream->writeFlag(targ->allocated))
      {
         if(stream->writeFlag(targ->sNameTag))
            stream->write(targ->sNameTag);
         if(stream->writeFlag(targ->sSkinTag))
            stream->write(targ->sSkinTag);
         if(stream->writeFlag(targ->sSkinPrefTag))
            stream->write(targ->sSkinPrefTag);
         if(stream->writeFlag(targ->sVoiceTag))
            stream->write(targ->sVoiceTag);
         if(stream->writeFlag(targ->sTypeTag))
            stream->write(targ->sTypeTag);
         stream->writeInt(targ->sensorGroup, 5);
         stream->writeInt(targ->renderFlags, TargetInfo::NumRenderBits);
         if(i >= 32)
         {
            if(stream->writeFlag(targ->shapeBaseData))
               stream->writeRangedU32(targ->shapeBaseData->getId(), DataBlockObjectIdFirst, DataBlockObjectIdLast);
         }
         //convert the voice pitch from range [0.5, 2.0] to [0.0, 1.0]
         F32 packFloat;
         if (targ->voicePitch < 0.5f || targ->voicePitch > 2.0f)
            packFloat = (1.0f - 0.5f) / 1.5f;
         else
            packFloat = (targ->voicePitch - 0.5f) / 1.5f;
         stream->writeFloat(packFloat, 7);
         stream->validate();
      }
   }
}

void TargetManager::readDemoStartBlock(BitStream *stream, GameConnection *conn)
{
   TargetInfo *targ = mClientTargets;
   for(U32 i = 0; i < MaxTargets; i++)
   {
      if(stream->readFlag())
      {
         if(stream->readFlag())
         {
            stream->read(&targ->sNameTag);
            targ->nameTag = conn->translateRemoteStringId(targ->sNameTag);
         }  
         if(stream->readFlag())
         {
            stream->read(&targ->sSkinTag);
            targ->skinTag = conn->translateRemoteStringId(targ->sSkinTag);
         }
         if(stream->readFlag())
         {
            stream->read(&targ->sVoiceTag);
            targ->voiceTag = conn->translateRemoteStringId(targ->sVoiceTag);
         }
         if(stream->readFlag())
         {
            stream->read(&targ->sTypeTag);
            targ->typeTag = conn->translateRemoteStringId(targ->sTypeTag);
         }
         targ->sensorGroup = stream->readInt(5);

         targ->renderFlags = stream->readInt(TargetInfo::NumRenderBits);
         targ->allocated = true;
         if(i >= 32)
         {
            if(stream->readFlag())
            {
               U32 id;
               id = stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);
               targ->shapeBaseData = dynamic_cast<ShapeBaseData*>(Sim::findObject(id));
            }
            gTargetManager->notifyTargetAdded(i);
            if(bool(targ->targetObject))
               targ->targetObject->targetInfoChanged(targ);
         }

         //convert the voice pitch from range [0.0, 1.0] back to [0.5, 2.0]
         F32 unpackFloat = stream->readFloat(7);
         targ->voicePitch = (unpackFloat * 1.5) + 0.5f;
         if (targ->voicePitch < 0.5f)
            targ->voicePitch = 0.5f;
         else if (targ->voicePitch > 2.0f)
            targ->voicePitch = 2.0f;
      }
   }
}

//----------------------------------------------------------------------------
// HUDTargetListNotify: client target notification system, passes handles
// to the actual entries
//----------------------------------------------------------------------------
HUDTargetListNotify::HUDTargetListNotify()
{
   AssertFatal(gTargetList, "HUDTargetListNotify:: No target list present");
   gTargetList->addNotify(this);
}

HUDTargetListNotify::~HUDTargetListNotify()
{
   if(gTargetList)
      gTargetList->removeNotify(this);
}

void HUDTargetList::addNotify(HUDTargetListNotify * notify)
{
   for(S32 i = hudNotifyList.size() - 1; i >= 0; i--)
      if(hudNotifyList[i] == notify)
      {
         Con::errorf(ConsoleLogEntry::General, "HUDTargetList::addNotify: object already being notified!");
         return;
      }
   hudNotifyList.push_back(notify);
}

void HUDTargetList::removeNotify(HUDTargetListNotify * notify)
{
   for(S32 i = hudNotifyList.size() - 1; i >= 0; i--)
      if(hudNotifyList[i] == notify)
      {
         hudNotifyList.erase(i);
         return;
      }
   Con::errorf(ConsoleLogEntry::General, "HUDTargetList::removeNotify: object not found in notify list!");
}

void HUDTargetList::notifyTargetAdded(U32 target)
{
   for(S32 i = hudNotifyList.size() - 1; i >= 0; i--)
      hudNotifyList[i]->hudTargetAdded(target);
}

void HUDTargetList::notifyTargetRemoved(U32 target)
{
   for(S32 i = hudNotifyList.size() - 1; i >= 0; i--)
      hudNotifyList[i]->hudTargetRemoved(target);
}

void HUDTargetList::notifyTargetsCleared()
{
   for(S32 i = hudNotifyList.size() - 1; i >= 0; i--)
      hudNotifyList[i]->hudTargetsCleared();
}


//----------------------------------------------------------------------------
// HUDTargetList:
//----------------------------------------------------------------------------
U32 HUDTargetList::smTargetTimeout = HUDTargetList::DefaultTimeout;

//----------------------------------------------------------------------------
HUDTargetList::HUDTargetList()
{
   VECTOR_SET_ASSOCIATION(hudNotifyList);

   mCount = 0;

   mNumAssignedTasks = 0;
   mNumPotentialTasks = 0;
   mNumWaypoints = 0;

   for(U32 i = 0; i < MaxTargets; i++)
      mHandle[i] = FREE_HANDLE;
}

//----------------------------------------------------------------------------
S32 HUDTargetList::findOldestEntry(S32 type)
{
   AssertFatal(mCount, "HUDTargetList::findOldestEntry: should not have 0 count");

   U32 oldTime = 0xffffffff;
   S32 oldest = -1;
   for(U32 i = 0; i < mCount; i++)
   {
      AssertFatal(bool(mList[i].target), "HUDTargetList::findOldestEntry: invalid target");
      if((mList[i].target->mType == type) && (mList[i].doneTime < oldTime))
      {
         oldest = i;
         oldTime = mList[i].doneTime;
      }
   }
   return(oldest);
}

//----------------------------------------------------------------------------
S32 HUDTargetList::getAddSlot(ClientTarget * target)
{
   S32 slot = mCount;
   switch(target->mType)
   {
      case ClientTarget::AssignedTask:
         if(mNumAssignedTasks == MaxAssignedTasks)
            slot = findOldestEntry(ClientTarget::AssignedTask);
         break;

      case ClientTarget::PotentialTask:
         if(mNumPotentialTasks == MaxPotentialTasks)
            slot = findOldestEntry(ClientTarget::PotentialTask);
         break;

      case ClientTarget::Waypoint:
         if(mNumWaypoints == MaxWaypoints)
            slot = findOldestEntry(ClientTarget::Waypoint);
         break;

      default:
         slot = -1;
         break;
   }
   return(slot);
}

S32 HUDTargetList::getFreeHandle()
{
   for(U32 i = 0; i < MaxTargets; i++)
      if(mHandle[i] == FREE_HANDLE)
         return(i);
   return(MaxTargets);
}

// Hudtarget ids are actually handles which are mapped above TargetManager::MaxTarget so
// that any target type still has a unique target id
S32 HUDTargetList::getHandleIndex(S32 handle)
{
   AssertFatal(handle >= TargetManager::MaxTargets && handle < TargetManager::MaxTargets + MaxTargets, "HUDTargetList::getNotifyEntry: invalid handle");
   return(mHandle[handle - TargetManager::MaxTargets]);
}

bool HUDTargetList::addTarget(ClientTarget * target, bool canTimeout, U32 doneTime)
{
   S32 slot = getAddSlot(target);
   if(slot == -1)
      return(false);

   // something there?
   if(slot != mCount)
   {
      AssertFatal(bool(mList[slot].target), "HUDTargetList::addTarget: invalid target in list");
      mList[slot].target->onDie();
   }

   // notify class' get a handle to this entry
   S32 handle = getFreeHandle();
   AssertFatal(handle != MaxTargets, "HudTargetList::addTarget: failed to get free handle");
   mHandle[handle] = mCount;

   // always add at end
   mList[mCount].target = target;
   mList[mCount].canTimeout = canTimeout;
   mList[mCount].doneTime = doneTime;
   mList[mCount].handle = handle;
   mCount++;

   // increment the type count
   if(target->mType == ClientTarget::AssignedTask)
      mNumAssignedTasks++;
   else if(target->mType == ClientTarget::PotentialTask)
      mNumPotentialTasks++;
   else if(target->mType == ClientTarget::Waypoint)
      mNumWaypoints++;
      
   // notify using unique target id's   
   notifyTargetAdded(handle + TargetManager::MaxTargets);
   return(true);
}

//--------------------------------------------------------------------------
// Sent to remove a specific target type from the target list
//--------------------------------------------------------------------------
class RemoveClientTargetTypeEvent : public NetEvent
{
   private:
      U32 mTargetType;
      
   public:   
      RemoveClientTargetTypeEvent(U32 type = 0)                { mTargetType = type; }
      void pack(NetConnection *, BitStream * bstream)          { bstream->writeRangedU32(mTargetType, 0, ClientTarget::NumTypes); }
      void write(NetConnection *, BitStream * bstream)         { bstream->writeRangedU32(mTargetType, 0, ClientTarget::NumTypes); }
      void unpack(NetConnection *, BitStream * bstream)        { mTargetType = bstream->readRangedU32(0, ClientTarget::NumTypes); }
      void process(NetConnection *)                            { gTargetList->removeTargetsOfType(mTargetType); }

      DECLARE_CONOBJECT(RemoveClientTargetTypeEvent);
};
IMPLEMENT_CO_CLIENTEVENT_V1(RemoveClientTargetTypeEvent);

// remove any target with this type
void HUDTargetList::removeTargetsOfType(U32 type)
{
   AssertFatal(type < ClientTarget::NumTypes, "HudTargetList::removeTargetsOfType: invalid type id");

   // mCount gets updated on removing of an entry
   for(U32 i = 0; i < mCount; i++)
   {
      AssertFatal(bool(mList[i].target), "HUDTargetList::removeTargetsOfType: invalid target");
      if(mList[i].target->mType == type)
         removeEntry(i);
   }
}

void HUDTargetList::removeEntry(S32 entry)
{
   AssertFatal(entry < mCount, "HUDTargetList::removeEntry: invalid entry");
   notifyTargetRemoved(mList[entry].handle + TargetManager::MaxTargets);

   // dec the type count
   AssertFatal(bool(mList[entry].target), "HUDTargetList::removeEntry:: invalid target");
   ClientTarget * target = static_cast<ClientTarget*>(mList[entry].target);

   if(target->mType == ClientTarget::AssignedTask)
      mNumAssignedTasks--;
   else if(target->mType == ClientTarget::PotentialTask)
      mNumPotentialTasks--;
   else if(target->mType == ClientTarget::Waypoint)
      mNumWaypoints--;

   // update the notification handles as well
   mCount--;
   if(entry != mCount)
   {
      mHandle[mList[mCount].handle] = entry;
      mHandle[mList[entry].handle] = FREE_HANDLE;
      mList[entry] = mList[mCount];
   }
   else
      mHandle[mList[entry].handle] = FREE_HANDLE;

   mList[mCount].target = 0;
}

//---------------------------------------------------------------------------
// TargetManager notification: 
//---------------------------------------------------------------------------
void HUDTargetList::targetRemoved(U32 target)
{
   // multiple hudtargets could be mapped to this target...
   for(S32 i = mCount - 1; i >= 0; i--)
   {
      AssertFatal(bool(mList[i].target), "HUDTargetList::targetRemoved: invalid target in list!");
      if(target == mList[i].target->mTargetId)
         (static_cast<ClientTarget*>(mList[i].target))->deleteObject();
   }
}

void HUDTargetList::targetsCleared()
{
   for(S32 i = mCount - 1; i >= 0; i--)
   {
      AssertFatal(bool(mList[i].target), "HUDTargetList::targetRemoved: invalid target in list!");
      (static_cast<ClientTarget*>(mList[i].target))->deleteObject();
   }
   notifyTargetsCleared();
}

//---------------------------------------------------------------------------
void HUDTargetList::update(U32 newTime)
{
   for(S32 i = mCount - 1; i >= 0; i--)
   {
      AssertFatal(bool(mList[i].target), "HUDTargetList::updateTime: invalid target");

      // timeout
      if(mList[i].canTimeout && (mList[i].doneTime < newTime))
      {
         mList[i].target->onDie();
         continue;
      }

      // skip location targets (their position is already known)
      if(mList[i].target->mTargetId == -1)
         continue;

      // update position
      TargetInfo * targ = gTargetManager->getClientTarget(mList[i].target->mTargetId);
      if(targ->sensorFlags & TargetInfo::VisibleToSensor)
         if(bool(targ->targetObject))
            targ->targetObject->getRenderWorldBox().getCenter(&mList[i].target->mLastTargetPos);
   }
}

S32 HUDTargetList::getEntryByTarget(ClientTarget * target)
{
   if(!target)
      return(-1);

   for(U32 i = 0; i < mCount; i++)
      if(static_cast<ClientTarget*>(mList[i].target) == target)
         return(i);

   return(-1);
}

void HUDTargetList::removeEntryByTarget(ClientTarget * target)
{
   S32 entry = getEntryByTarget(target);
   if(entry == -1)
      return;

   removeEntry(entry);
}

HUDTargetList::Entry *HUDTargetList::getEntry(U32 index)
{
   if(index >= mCount)
      return(0);
   return(mList + index);
}

//--------------------------------------------------------------------------
ClientTarget::ClientTarget(S32 type, S32 targetId, Point3F targetPos)
{
   mType = type;
   mTargetId = targetId;
   mLastTargetPos = targetPos;
   mText = 0;
}

void ClientTarget::onRemove()
{
   if(gTargetList)
   {
      if(mType == Waypoint)
         Con::executef(this, 1, "waypointRemoved");
      gTargetList->removeEntryByTarget(this);
   }
   Parent::onRemove();
}

// being removed by the target list
void ClientTarget::onDie()
{
   const char * typeStr = 0;
   switch(mType)
   {
      case AssignedTask:
         typeStr = "AssignedTask";
         break;
      case PotentialTask:
         typeStr = "PotentialTask";
         break;   

      default:
         deleteObject();
         return;
   }

   SimObjectPtr<ClientTarget> safeThis = this;
      
   // give the console a chance to take possesion of this target   
   Con::executef(this, 2, "onDie", typeStr);
   
   if(!bool(safeThis))
      return;
      
   // removed now?
   gTargetList->removeEntryByTarget(this);
   if(!getGroup() || (getGroup() == NetConnection::getServerConnection()))
      deleteObject();
}

//---------------------------------------------------------------------------
// only ClientTarget created through event get processed (Tasks)
bool ClientTarget::process()
{
   S32 time = Sim::getCurrentTime() + (mType == PotentialTask ? HUDTargetList::smTargetTimeout : 0);
   if(gTargetList->addTarget(this, mType == PotentialTask, time))
   {
      Con::executef(this, 2, "onAdd", mType == AssignedTask ? "AssignedTask" : "PotentialTask");
      return(true);
   }
   return(false);
}

static void cTargetSendToServer(SimObject *target, S32, const char **)
{
   ClientTarget *targ = static_cast<ClientTarget *>(target);
   GameConnection *gc = GameConnection::getServerConnection();
   if(gc)
      gc->sendTargetToServer(targ->mTargetId, targ->mLastTargetPos);
}

// waypoints are created by the client
static void cCreateWaypoint(SimObject * obj, S32, const char ** argv)
{
   ClientTarget * target = static_cast<ClientTarget*>(obj);
   
   // can only create waypoints from untyped targets   
   if(target->mType != -1)
   {
      Con::errorf(ConsoleLogEntry::General, "ClientTarget::cCreateWaypoint: target already typed");
      return;
   }

   target->mText = StringTable->insert(argv[2]);   
   target->mType = ClientTarget::Waypoint;
   if(!gTargetList->addTarget(target, false, Sim::getCurrentTime()))
      Con::errorf(ConsoleLogEntry::General, "ClientTarget::cCreateWaypoint: unable to create waypoint");
}

static void cAddPotentialTask(SimObject * obj, S32, const char **)
{
   ClientTarget * target = static_cast<ClientTarget*>(obj);
   if(target->mType != ClientTarget::PotentialTask)
      return;

   // may already be in the list      
   S32 entry = gTargetList->getEntryByTarget(target);
   if(entry != -1)
      return;

   // add to the target-list   
   if(!gTargetList->addTarget(target, true, Sim::getCurrentTime() + HUDTargetList::smTargetTimeout))
      Con::errorf(ConsoleLogEntry::General, "ClientTarget::cAddPotentialTask: unable to add task");
}

static void cSetText(SimObject * obj, S32, const char ** argv)
{
   ClientTarget * target = static_cast<ClientTarget*>(obj);
   target->mText = StringTable->insert(argv[2]);
}

static S32 cGetTargetId(SimObject * obj, S32, const char **)
{
   ClientTarget * target = static_cast<ClientTarget*>(obj);
   if(!gTargetList)
      return(-1);
   S32 entry = gTargetList->getEntryByTarget(target);
   if(entry == -1)
      return(-1);
   return(TargetManager::MaxTargets + gTargetList->mList[entry].handle);
}

static S32 cCreateClientTarget(SimObject *, S32 argc, const char ** argv)
{
   GameConnection * con = GameConnection::getServerConnection();
   if(!con)
      return(-1);

   Point3F pos(0,0,0);
   if(argc == 3)
      dSscanf(argv[2],"%f %f %f", &pos.x, &pos.y, &pos.z);
   
   ClientTarget * target = new ClientTarget(-1, dAtoi(argv[1]), pos);
   target->registerObject();
   con->addObject(target);
   return(target->getId());
}

static U32 getTypeFromString(const char * str)
{
   if(!str)
      return(ClientTarget::NumTypes);
   
   if(!dStricmp(str, "AssignedTask"))
      return(ClientTarget::AssignedTask);
   else if(!dStricmp(str, "PotentialTask"))
      return(ClientTarget::PotentialTask);
   else if(!dStricmp(str, "Waypoint"))
      return(ClientTarget::Waypoint);

   return(ClientTarget::NumTypes);
}

static void cRemoveClientTargetType(SimObject *, S32, const char ** argv)
{
   U32 type = getTypeFromString(argv[2]);
   
   // make sure type in range
   if(type >= ClientTarget::NumTypes)
   {
      Con::errorf(ConsoleLogEntry::General, "HUDTargetList::removeClientTargetType: invalid type [%s]", argv[1]);
      return;
   }
   
   // only send to a non-ai connection
   NetConnection * client = dynamic_cast<NetConnection*>(Sim::findObject(dAtoi(argv[1])));
   if(client && !client->isServerConnection())
   {
      if(dynamic_cast<GameConnection*>(client) && static_cast<GameConnection*>(client)->isAIControlled())
         return;
      client->postNetEvent( new RemoveClientTargetTypeEvent(type) );
   }
   else
      Con::errorf(ConsoleLogEntry::General, "HUDTargetList::removeClientTargetType: invalid connection [%s]", argv[1]);
}

void ClientTarget::consoleInit()
{
   Con::addCommand("ClientTarget", "sendToServer",       cTargetSendToServer,       "target.sendToServer()",                  2, 2);
   Con::addCommand("ClientTarget", "createWaypoint",     cCreateWaypoint,           "target.createWaypoint(text)",            3, 3);
   Con::addCommand("ClientTarget", "addPotentialTask",   cAddPotentialTask,         "target.addPotentialTask()",              2, 2);
   Con::addCommand("ClientTarget", "setText",            cSetText,                  "target.setText(text)",                   3, 3);
   Con::addCommand("ClientTarget", "getTargetId",        cGetTargetId,              "target.getTargetId()",                   2, 2);

   Con::addCommand("createClientTarget",                 cCreateClientTarget,       "createClientTarget(targetId, <x y z>)",  2, 3);
   Con::addCommand("removeClientTargetType",             cRemoveClientTargetType,   "removeClientTargetType(client, type)",   3, 3);

   Con::addVariable("clientTargetTimeout", TypeS32, &HUDTargetList::smTargetTimeout);
}

IMPLEMENT_CONOBJECT(ClientTarget);

//---------------------------------------------------------------------------
// TargetManager::SensorInfo:
//---------------------------------------------------------------------------
ColorI TargetManager::SensorInfo::smDefaultColor(255, 0, 0, 255);

void TargetManager::SensorInfo::clear()
{
   U32 i;
   for(i = 0; i < TargetManager::TargetFreeMaskSize; i++)
      targetPingMask[i] = 0;
   for(i = 0; i < 32; i++)
      groupColor[i] = smDefaultColor;
}

void TargetManager::SensorInfo::setSensorVisible(S32 targetId, bool vis)
{
   if(targetId < 0 || targetId >= TargetManager::MaxTargets)
      return;
   if(vis)
      targetPingMask[targetId >> 5] |= (1 << (targetId & 0x1F));
   else
      targetPingMask[targetId >> 5] &= ~(1 << (targetId & 0x1F));
}

bool TargetManager::SensorInfo::getSensorVisible(S32 targetId)
{
   if(targetId < 0 || targetId >= TargetManager::MaxTargets)
      return false;
   return targetPingMask[targetId >> 5] & (1 << (targetId & 0x1F));
}

//--------------------------------------------------------------------------
static inline bool testLOS(GameBase * sensor, const Point3F & sensorPos, 
                           GameBase * target, const Point3F & targetPos)
{
   static U32 losMask = TerrainObjectType | InteriorObjectType | ShapeBaseObjectType;
   
   RayInfo info;

   // disable collision for the target/sensor and possible mount
   target->disableCollision();
   sensor->disableCollision();
   
   ShapeBase * mount = 0;
   if(target->getType() & ShapeBaseObjectType)
   {
      mount = static_cast<ShapeBase*>(target)->getObjectMount();
      if(mount)
         mount->disableCollision();
   }
   
   bool hasLOS = !gServerContainer.castRay(sensorPos, targetPos, losMask, &info);

   // enable the collisions for the objects..
   target->enableCollision();
   sensor->enableCollision();
   if(mount)
      mount->enableCollision();
   
   return(hasLOS);
}

void TargetManager::tickSensorState()
{
   U32 objectCount = 0;
   U32 totalCount = MaxTargets - mFreeCount;
   U32 pingCount = (totalCount >> 5) + 1;  // ping everything once a second
   U32 lastSensed = mLastSensedObject;

   for(U32 i = mLastSensedObject + 1; i - mLastSensedObject < MaxTargets; i++)
   {
      U32 index = i & (MaxTargets - 1);
      U32 maskPos = index >> 5;
      U32 maskShift = index & 0x1F;
      if((maskShift == 0) && mFreeMask[maskPos] == 0)
      {
         i += 31;
         continue;
      }
      if(!(mFreeMask[maskPos] & (1 << maskShift)))
         continue;
      TargetInfo *targetInfo = mTargets + index;
      
      if(!bool(targetInfo->targetObject))
         continue;
         
      GameBase * target = targetInfo->targetObject;
      
      // ok, we have an object
      // now loop through all the sensable objects
      U32 baseVisMask = 0;
      U32 activeJamVisMask = 0;
      U32 passiveJamVisMask = 0;
      U32 cloakVisMask = 0;
      bool pinged = false;
      bool jammed = false;
      bool enemyJammed = false;
      
      Point3F targetPos;
      
      // grab eye point if player...
      if(target->getType() & PlayerObjectType)
      {
         AssertFatal(dynamic_cast<Player*>(target), "Invalid player object.");
         Player * player = static_cast<Player*>(target);

         MatrixF eye;
         player->getEyeTransform(&eye);
         eye.getColumn(3, &targetPos);
      }
      else
         targetPos = target->getBoxCenter();
         
      Point3F targetVec;

      for(U32 sens = 0; sens < MaxTargets; sens++)
      {
         bool testedLOS = false;
         bool hasLOS = false;
         U32 smaskPos = sens >> 5;
         U32 smaskShift = sens & 0x1F;
         if((smaskShift == 0) && mFreeMask[smaskPos] == 0)
         {
            sens += 31;
            continue;
         }
         if(!(mFreeMask[smaskPos] & (1 << smaskShift)))
            continue;
         TargetInfo *sensorInfo = mTargets + sens;
         if(!bool(sensorInfo->targetObject))
            continue;
         GameBase *sensor = sensorInfo->targetObject;
         SensorData *sensorData = sensorInfo->sensorData;
         
         if(!sensor || !sensorData)
            continue;

         // can't detect its own bad self, but can jam...
         if(sens == index)
         {
            if(sensorData->jams)
               jammed = true;
            continue;
         }
         // sensors must be shapebase items (need damage state)
         if(!dynamic_cast<ShapeBase*>(sensor))
            continue;

         ShapeBase * sensorShape = static_cast<ShapeBase*>(sensor);
         if(sensorShape->getDamageState() != ShapeBase::Enabled)
            continue;

         Point3F sensorPos;

         bool jumpNoDetect = false;

         // jams? and is/not always visible?
         U32 sensorMask = 1 << sensorInfo->sensorGroup;
         if((targetInfo->sensorAlwaysVisMask | targetInfo->sensorNeverVisMask) & sensorMask)
         {
            if(sensorData->jams)
               jumpNoDetect = true;
            else
               continue;
         }

         // grab the sensor position (grab eye of players)
         // if this is a player then grab its eye...
         if(sensor->getType() & PlayerObjectType)
         {
            AssertFatal(dynamic_cast<Player*>(sensor), "Invalid player object.");

            MatrixF eye;
            sensorShape->getEyeTransform(&eye);
            eye.getColumn(3, &sensorPos);
         }
         else
            sensorPos = sensor->getBoxCenter();

         // jams but is visible?
         if(jumpNoDetect)
            goto nodetect;

         // see if the sensor detects stuff:
         if(!sensorData->detects)
            goto nodetect;

         U32 *mask;
         if(sensorData->detectsActiveJammed)       // everything
            mask = &activeJamVisMask;
         else if(sensorData->detectsCloaked)       // all but motion
            mask = &cloakVisMask;
         else if(sensorData->detectsPassiveJammed) // all but motion and los
            mask = &passiveJamVisMask;
         else
            mask = &baseVisMask;

         // see if we can skip this one:
         if((*mask & sensorMask) && (pinged == sensorData->detectionPings))
            goto nodetect;

         targetVec = targetPos - sensorPos;

         // check distance:
         if(targetVec.isZero())
            continue;
         
         // uncapped cylinder
         if(Point2F(targetVec.x, targetVec.y).lenSquared() > sensorData->detectRSquared)
            goto nodetect;

// normal sphere
//         if(targetVec.lenSquared() > sensorData->detectRSquared)
//            goto nodetect;

         // minvel:
         if(sensorData->detectMinVelocity != 0.f)
            if(target->getVelocity().lenSquared() < sensorData->detectMinVSquared)
               goto nodetect;

         // fov:
         if(sensorData->detectsFOVOnly)
         {
            MatrixF camMat;
            sensorShape->getEyeTransform(&camMat);

            VectorF camDir;
            camMat.mulV(VectorF(0,1,0), &camDir);
            targetVec.normalize();

            F32 dot = mClampF(mDot(targetVec, camDir), -1.f, 1.f);

            // check if interested in projected fov through this object
            if(sensorData->useObjectFOV)
            {
               F32 objectFov = mDegToRad(sensorShape->getCameraFov());
               F32 halfFovCos = mCos(objectFov / 2.f);
               if(dot < halfFovCos)
                  goto nodetect;

               if(sensorData->detectFOVPercent != 0.f)
               {
                  F32 objRadius = target->getWorldSphere().radius;
                  F32 distance = Point3F(targetPos - sensorPos).len();
               
                  F32 projRadius = distance * mTan(objectFov / 2.f);

                  if(((objRadius / projRadius) * 100.f) < sensorData->detectFOVPercent)
                     goto nodetect;
               }
            }
            else
               if(dot < sensorData->halfFovCos)
                  goto nodetect;
         }

         // los:
         if(sensorData->detectsUsingLOS)
         {
            testedLOS = true;
            hasLOS = testLOS(sensor, sensorPos, target, targetPos);
            if(!hasLOS)
               goto nodetect;
         }

         // it's detected
         *mask |= sensorMask;

         // friendly do not ping
         if(sensorData->detectionPings && !(sensorInfo->sensorFriendlyMask & (1 << targetInfo->sensorGroup)))
            pinged = true;

nodetect:
         
         // early out?
         if(!sensorData->jams || (jammed && enemyJammed))
            continue;

         if(sensorInfo->sensorGroup == targetInfo->sensorGroup)
         {
            if(jammed)
               continue;
         }
         else
         {
            if(enemyJammed && sensorData->jamsOnlyGroup)
               continue;
         }

         // normal sphere
         if((targetPos - sensorPos).lenSquared() > sensorData->jamRSquared)
            continue;

         // check los
         if(sensorData->jamsUsingLOS)
         {
            if(!testedLOS)
               hasLOS = testLOS(sensor, sensorPos, target, targetPos);

            if(!hasLOS)
               continue;
         }

         // set the jammed state
         if(sensorInfo->sensorGroup == targetInfo->sensorGroup)
            jammed = true;
         else
         {
            jammed = !sensorData->jamsOnlyGroup;
            enemyJammed = true;
         }
      }

      // check cloaked/passiveJammed: only ShapeBase objects
      bool cloaked = false;
      bool passiveJammed = false;
      if(dynamic_cast<ShapeBase*>(target))
      {
         cloaked = (static_cast<ShapeBase*>(target))->getCloakedState();
         passiveJammed = (static_cast<ShapeBase*>(target))->getPassiveJamState();
      }

      // check what could detect it: active->cloaked->passive->base
      U32 visMask;
      if(jammed)
         visMask = activeJamVisMask;
      else if(cloaked)
         visMask = activeJamVisMask | cloakVisMask;
      else if(passiveJammed)
         visMask = activeJamVisMask | cloakVisMask | passiveJamVisMask;
      else
         visMask = activeJamVisMask | cloakVisMask | passiveJamVisMask | baseVisMask;

      visMask |= targetInfo->sensorAlwaysVisMask;
      visMask &= ~targetInfo->sensorNeverVisMask;
               
      targetInfo->sensorVisMask = visMask;
      targetInfo->sensorFlags = 0;
      if(pinged)
         targetInfo->sensorFlags |= TargetInfo::SensorPinged;

      // if jammed, then notify the shapebase object
      if(jammed || enemyJammed)
      {
         if(jammed)
            targetInfo->sensorFlags |= TargetInfo::SensorJammed;
         
         if(enemyJammed)
         {
            targetInfo->sensorFlags |= TargetInfo::EnemySensorJammed;

            if(dynamic_cast<ShapeBase*>(static_cast<SimObject*>(targetInfo->targetObject)))
            {
               ShapeBase * shape = static_cast<ShapeBase*>(static_cast<SimObject*>(targetInfo->targetObject));

               // reason gets passed down into script.. (shapebase actually doesnt do anything)
               shape->forceUncloak("jammed");
            }
         }
      }

      for(U32 j = 0; j < mSensorGroupCount; j++, visMask >>= 1)
         mSensorInfoArray[j].setSensorVisible(index, (visMask & 1));

      objectCount++;
      lastSensed = i;
      if(objectCount >= pingCount)
         break;
   }
   mLastSensedObject = lastSensed;
}

//------------------------------------------------------------------------------
// debug list control: fills with target info
//------------------------------------------------------------------------------
#ifdef DEBUG
#include "gui/guiTextListCtrl.h"

class GuiTargetManagerListCtrl : public GuiTextListCtrl
{
   private:
      typedef GuiTextListCtrl Parent;
      bool  mServerTargets;

  public:

      enum {
         NumCategories = 14
      };
      
      GuiTargetManagerListCtrl();
      void onPreRender();

      static void initPersistFields();
      static void consoleInit();

      DECLARE_CONOBJECT(GuiTargetManagerListCtrl);
};
IMPLEMENT_CONOBJECT(GuiTargetManagerListCtrl);

GuiTargetManagerListCtrl::GuiTargetManagerListCtrl()
{
   mServerTargets = true;
}

void GuiTargetManagerListCtrl::onPreRender()
{
   RectI bounds = mBounds;

   clear();
   
   // add all the entries
   addEntry(0, "ID\tName\tType\tVoice\tSkin\tPitch\tGroup\tObj\tData\tSensor\tVisMask\tAlways\tNever\tFriend\tFlags");
   
   for(U32 i = 32; i < TargetManager::MaxTargets; i++)
   {
      TargetInfo * targInfo = mServerTargets ? gTargetManager->getServerTarget(i) : gTargetManager->getClientTarget(i);

      if(targInfo->allocated)
      {
         char buf[1024];
         buf[0] = 0;

         dSprintf(buf, sizeof(buf),
            "%d\t%s\t%s\t%s\t%s\t%f\t%d\t%d\t%s\t%s\t%x\t%x\t%x\t%x\t%x", i,
            gNetStringTable->lookupString(targInfo->nameTag),
            gNetStringTable->lookupString(targInfo->typeTag),
            gNetStringTable->lookupString(targInfo->voiceTag),
            gNetStringTable->lookupString(targInfo->skinTag),
            targInfo->voicePitch,
            targInfo->sensorGroup,
            bool(targInfo->targetObject) ? targInfo->targetObject->getId() : 0,
            bool(targInfo->shapeBaseData) ? targInfo->shapeBaseData->getName() : "<none>",
            bool(targInfo->sensorData) ? targInfo->sensorData->getName() : "<none>",
            targInfo->sensorVisMask,
            targInfo->sensorAlwaysVisMask,
            targInfo->sensorNeverVisMask,
            targInfo->sensorFriendlyMask,
            targInfo->sensorFlags);

         addEntry(i, buf);
      }
   }
   
   resize(bounds.point, mBounds.extent);
   Parent::onPreRender();
}

void GuiTargetManagerListCtrl::initPersistFields()
{
   Parent::initPersistFields();
   addField("serverTargets",  TypeBool, Offset(mServerTargets, GuiTargetManagerListCtrl));
}

static S32 cGetNumColumns(SimObject *, S32, const char **)
{
   return(GuiTargetManagerListCtrl::NumCategories);
}

void GuiTargetManagerListCtrl::consoleInit()
{
   Con::addCommand("GuiTargetManagerListCtrl", "getNumColumns", cGetNumColumns, "ctrl.getNumColumns()", 2, 2);
}
#endif
