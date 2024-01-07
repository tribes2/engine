//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _TARGETMANAGER_H_
#define _TARGETMANAGER_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif
#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif
#ifndef _GAMEBASE_H_
#include "game/gameBase.h"
#endif
#ifndef _SHAPEBASE_H_
#include "game/shapeBase.h"
#endif
#ifndef _COLOR_H_
#include "core/color.h"
#endif
#ifndef _DNET_H_
#include "core/dnet.h"
#endif
#ifndef _AUDIO_H_
#include "audio/audio.h"
#endif

struct SensorData;
class ResizeBitStream;
class BitStream;
struct ShapeBaseData;
class ShapeBase;

struct TargetInfo
{
   void clear(bool clearFlags = true) {
      nameTag = 0;
      skinTag = 0;
      skinPrefTag = 0;
      voiceTag = 0;
      typeTag = 0;
      voicePitch = 1.0;

      sNameTag = 0;
      sSkinTag = 0;
      sSkinPrefTag = 0;
      sVoiceTag = 0;
      sTypeTag = 0;

      sensorGroup = 0;
      allocated = false;
      renderFlags = 0;

      sensorVisMask = 0;
      sensorAlwaysVisMask = 0;
      sensorNeverVisMask = 0;
      sensorFriendlyMask = 0;

      // clients can get out of sync if these get cleared
      //  : a freed target can be reallocated and sensed before the connection
      //  : has a chance to update
      if(clearFlags)
         sensorFlags = 0;

      targetObject = 0;
      sensorData = 0;
      shapeBaseData = 0;
   }

   enum {
      HudRenderStart       = 0,
      NumHudRenderImages   = 8,
      HudRenderMask        = BIT(NumHudRenderImages) - 1,
      CommanderListRender  = BIT(NumHudRenderImages),
      NumRenderBits        = NumHudRenderImages + 1,
   };

   enum {
      VisibleToSensor      = BIT(0),
      SensorPinged         = BIT(1),
      SensorJammed         = BIT(2),
      EnemySensorJammed    = BIT(3),
      SensorCloaked        = BIT(4),
   };

   // Client/Server
   U16                           nameTag;
   U16                           skinTag;
   U16                           skinPrefTag;
   U16                           voiceTag;
   U16                           typeTag;
   F32                           voicePitch;

   U16                           sNameTag;  // source tag (pre translate)
   U16                           sSkinTag;  // used for writing the demo start
   U16                           sSkinPrefTag;
   U16                           sVoiceTag; // block
   U16                           sTypeTag;

   U32                           sensorGroup;
   U32                           renderFlags;
   
   bool                          allocated;

   SimObjectPtr<GameBase>        targetObject;
   SimObjectPtr<ShapeBaseData>   shapeBaseData;

   // Server
   U32                           sensorVisMask;
   U32                           sensorAlwaysVisMask;
   U32                           sensorNeverVisMask;
   U32                           sensorFriendlyMask;
   U32                           sensorFlags;
   SimObjectPtr<SensorData>      sensorData;
};

// Client target notification system
class TargetManagerNotify
{
   public:
      TargetManagerNotify();
      ~TargetManagerNotify();

      virtual void targetAdded(U32) {}
      virtual void targetRemoved(U32) {}
      virtual void targetChanged(U32) {}
      virtual void targetsCleared() {}
};

struct TargetManager
{
   // client target notification system
   Vector<TargetManagerNotify*> notifyList;
   void addNotify(TargetManagerNotify*);
   void removeNotify(TargetManagerNotify*);
   void notifyTargetAdded(U32);
   void notifyTargetRemoved(U32);
   void notifyTargetChanged(U32);
   void notifyTargetsCleared();

   enum {
      MaxTargets = 512,
      TargetIdBitSize = 9,
      TargetFreeMaskSize = MaxTargets >> 5,
   };
   struct SensorInfo
   {
      U32                  targetPingMask[TargetFreeMaskSize];
      ColorI               groupColor[32];
      static ColorI        smDefaultColor;

      void clear();
      void setSensorVisible(S32 targetId, bool vis);
      bool getSensorVisible(S32 targetId);
   };
   SensorInfo mSensorInfoArray[32];
   U32 mFreeCount;
   U32 mLastSensedObject;
   U32 mSensorGroupCount;
   
   U32 mSensorGroupAlwaysVisMask[32];
   U32 mSensorGroupNeverVisMask[32];
   U32 mSensorGroupFriendlyMask[32];
   U32 mSensorGroupListenMask[32];

   U32 mFreeMask[TargetFreeMaskSize];
   TargetInfo mTargets[MaxTargets];

   TargetInfo mClientTargets[MaxTargets];
   AUDIOHANDLE mClientAudioHandles[MaxTargets];

   static void create();
   static void destroy();
   
   TargetManager();
   void clear();

   void writeDemoStartBlock(ResizeBitStream *stream, GameConnection *conn);
   void readDemoStartBlock(BitStream *stream, GameConnection *conn);
   
   void newClient(NetConnection *client);
   S32 allocTarget(U32 nameTag, U32 skinTag, U32 voiceTag, U32 typeTag, U32 sensorGroup, U32 dataBlockId, F32 voicePitch, U32 prefSkin);
   void freeTarget(S32 targetId);

   void reset();
   void resetClient(NetConnection * con);

   void setTargetObject(TargetInfo * target, GameBase *object);
   bool getGameName(S32 target, char * buf, S32 bufSize, bool server);

   S32 getTargetName(S32 target);
   void setTargetName(S32 target, U32 nameTag);
   S32 getTargetSkin(S32 target);
   void setTargetSkin(S32 target, U32 skinTag);
   S32 getTargetVoice(S32 target);
   void setTargetVoice(S32 target, U32 voiceTag);
   F32 getTargetVoicePitch(S32 target);
   void setTargetVoicePitch(S32 target, F32 voicePitch);
   S32 getTargetType(S32 target);
   void setTargetType(S32 target, U32 typeTag);
   S32 getTargetSensorGroup(S32 target);
   void setTargetSensorGroup(S32 target, U32 sensorGroup);

   U32 getTargetAlwaysVisMask(S32 target);
   void setTargetAlwaysVisMask(S32 target, U32 mask);
   U32 getTargetNeverVisMask(S32 target);
   void setTargetNeverVisMask(S32 target, U32 mask);
   U32 getTargetFriendlyMask(S32 target);
   void setTargetFriendlyMask(S32 target, U32 mask);

   enum MaskType {
      AlwaysVisMask = 0,
      NeverVisMask,
      FriendlyMask
   };
   void updateSensorGroupMask(U32 sensorGroup, U32 mask, U32 maskType);

   U32 getSensorGroupAlwaysVisMask(U32 sensorGroup);
   void setSensorGroupAlwaysVisMask(U32 sensorGroup, U32 mask);
   U32 getSensorGroupNeverVisMask(U32 sensorGroup);
   void setSensorGroupNeverVisMask(U32 sensorGroup, U32 mask);
   U32 getSensorGroupFriendlyMask(U32 sensorGroup);
   void setSensorGroupFriendlyMask(U32 sensorGroup, U32 mask);
   
   U32 getSensorGroupListenMask(U32 sensorGroup);
   void setSensorGroupListenMask(U32 sensorGroup, U32 mask);

   bool isTargetFriendly(S32 target, U32 sensorGroup);
   bool isTargetVisible(S32 target, U32 sensorGroup);
   
   S32 getSensorGroupCount() {return(mSensorGroupCount);}
   void setSensorGroupCount(S32 groupCount);

   void setTargetRenderMask(S32 target, U32 renderMask);
   void setTargetShapeBaseData(S32 target, ShapeBaseData * data);

   TargetInfo *getClientTarget(S32 target);
   TargetInfo *getServerTarget(S32 target);
   void updateTarget(S32 targ, S32 nameTag, S32 skinTag, S32 voiceTag, S32 typeTag, S32 sensorGroup, S32 dataBlockId, S32 render, F32 voicePitch, U32 prefSkin);
   void tickSensorState();
   
   void clientSensorGroupChanged(NetConnection * con, U32 newGroup);
   void setSensorGroupColor(U32 sensorGroup, U32 updateMask, ColorI & color);
   ColorI getSensorGroupColor(U32 sensorGroup, U32 colorGroup);

   bool playTargetAudio(S32 targetId, S32 fileTag, AudioDescription * description, bool update);
};

inline void TargetManager::setSensorGroupCount(S32 groupCount)
{
   AssertFatal(groupCount <= 32, "TargetManager::setSensorGroupCount: invalid group count");
   mSensorGroupCount = groupCount;
}

extern TargetManager *gTargetManager;

//--------------------------------------------------------------------------
class ClientTarget : public SimObject
{
   typedef SimObject Parent;
public:
   enum 
   {
      AssignedTask = 0,    // what the player is currently doing   
      PotentialTask,       // what people want the player to do
      Waypoint,            // waypoints
      
      NumTypes
   };
   S32                     mType;
   S32                     mTargetId;
   Point3F                 mLastTargetPos;
   StringTableEntry        mText;

   ClientTarget(S32 type = -1, S32 targetId = -1, Point3F targetPos = Point3F(0,0,0));
   bool process();
   void setServerTarget();
   static void consoleInit();
   void onRemove();
   void onDie();

   const char * getText() {return(mText);}
   void setText(const char *);
   
   DECLARE_CONOBJECT(ClientTarget);
};

//--------------------------------------------------------------------------
class HUDTargetListNotify
{
   public:
      HUDTargetListNotify();
      ~HUDTargetListNotify();

      virtual void hudTargetAdded(U32) {}
      virtual void hudTargetRemoved(U32) {}
      virtual void hudTargetsCleared() {}
};

struct HUDTargetList : public TargetManagerNotify
{
   struct Entry
   {
      SimObjectPtr<ClientTarget> target;
      bool canTimeout;
      U32 doneTime;
      S32 handle;
   };
   enum
   {
      DefaultTimeout = 2000,
      MaxTargets = 32,

      MaxAssignedTasks     = 1,
      MaxPotentialTasks    = 15,
      MaxWaypoints         = 16,
   };
   U32   mNumAssignedTasks;
   U32   mNumPotentialTasks;
   U32   mNumWaypoints;

   Entry mList[MaxTargets];
   U32   mCount;
   static U32 smTargetTimeout;

   HUDTargetList();

   S32 getAddSlot(ClientTarget * target);
   S32 findOldestEntry(S32 type);
   void removeEntry(S32 entry);
      
   bool addTarget(ClientTarget * target, bool canTimeout, U32 doneTime);
   S32 getEntryByTarget(ClientTarget *);
   void removeEntryByTarget(ClientTarget *);
   void update(U32 newTime);
   Entry *getEntry(U32 index);

   void removeTargetsOfType(U32 type);

   // TargetManager notification
   void targetRemoved(U32);
   void targetsCleared();

   // notification stuff
   enum {
      FREE_HANDLE = -1
   };
   S32 mHandle[MaxTargets];
   S32 getFreeHandle();
   S32 getHandleIndex(S32 handle);

   Vector<HUDTargetListNotify*> hudNotifyList;
   void addNotify(HUDTargetListNotify*);
   void removeNotify(HUDTargetListNotify*);
   void notifyTargetAdded(U32);
   void notifyTargetRemoved(U32);
   void notifyTargetsCleared();
};

extern HUDTargetList * gTargetList;

#endif
