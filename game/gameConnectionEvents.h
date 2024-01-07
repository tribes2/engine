//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GAMECONNECTIONEVENTS_H_
#define _GAMECONNECTIONEVENTS_H_

class QuitEvent : public SimEvent
{
   void process(SimObject *)
   {
      Platform::postQuitMessage(0);
   }
};

class SetSensorGroupEvent : public NetEvent
{
   U32 mSensorGroup;
  public:
   SetSensorGroupEvent(U32 sensGrp = 0)
   {
      mSensorGroup = sensGrp;
   }
   void pack(NetConnection *, BitStream *bstream)
   {
      bstream->writeInt(mSensorGroup, 5);
   }
   void write(NetConnection *con, BitStream *bstream)
   {
      pack(con, bstream);
   }
   void unpack(NetConnection *, BitStream *bstream)
   {
      mSensorGroup = bstream->readInt(5);
   }
   void process(NetConnection *con)
   {
      if(con->isServerConnection())
         ((GameConnection *) con)->setSensorGroup(mSensorGroup);
   }
   DECLARE_CONOBJECT(SetSensorGroupEvent);
};

class SetServerTargetEvent : public NetEvent
{
   S32 mTargetId;
   Point3F mTargetPos;
   
  public:
   SetServerTargetEvent(S32 targetId = -1, Point3F targetPos = Point3F(0,0,0))
   {
      mTargetId = targetId;
      mTargetPos = targetPos;
   }

   void pack(NetConnection *, BitStream *bstream)
   {
      if(bstream->writeFlag(mTargetId != -1))
         bstream->writeInt(mTargetId, TargetManager::TargetIdBitSize);
      bstream->write(mTargetPos.x);
      bstream->write(mTargetPos.y);
      bstream->write(mTargetPos.z);
   }
   
   void write(NetConnection *conn, BitStream *bstream)
   {
      pack(conn, bstream);
   }
   void unpack(NetConnection *, BitStream *bstream)
   {
      if(bstream->readFlag())
         mTargetId = bstream->readInt(TargetManager::TargetIdBitSize);
      bstream->read(&mTargetPos.x);
      bstream->read(&mTargetPos.y);
      bstream->read(&mTargetPos.z);
   }
   void process(NetConnection* conn)
   {
      ((GameConnection *)conn)->setServerTarget(mTargetId, mTargetPos);
   }
   DECLARE_CONOBJECT(SetServerTargetEvent);
};

class TargetToEvent : public NetEvent {
  public:
   S32 mTargetType;
   S32 mTargetId;
   bool mAssign;
   Point3F mTargetPos;
   SimObjectPtr<GameBase> mTarget;
   
   TargetToEvent(S32 targetId = -1, Point3F targetPos = Point3F(0,0,0), bool assign = false)
   {
      mTargetId = targetId;
      mTargetPos = targetPos;
      mAssign = assign;
   }
   void pack(NetConnection *conn, BitStream *bstream)
   {
      S32 ghostIndex = -1;
      if(mTargetId != -1)
      {
         TargetInfo *inf = gTargetManager->getServerTarget(mTargetId);
         if(bool(inf->targetObject))
            ghostIndex = conn->getGhostIndex(inf->targetObject);
      }
      if(bstream->writeFlag(mTargetId != -1))
         bstream->writeInt(mTargetId, TargetManager::TargetIdBitSize);

      if(bstream->writeFlag(ghostIndex == -1))
      {
         bstream->write(mTargetPos.x);
         bstream->write(mTargetPos.y);
         bstream->write(mTargetPos.z);
      }
      bstream->writeFlag(mAssign);
   }
   void write(NetConnection *conn, BitStream *bstream)
   {
      conn; //unused
      if(mTarget)
      {
         bstream->writeFlag(true);
         bstream->writeInt(mTarget->getNetIndex(), NetConnection::GhostIdBitSize);
      }
      else
      {
         bstream->write(mTargetPos.x);
         bstream->write(mTargetPos.y);
         bstream->write(mTargetPos.z);
      }
      bstream->writeFlag(mAssign);
   }
   void unpack(NetConnection *conn, BitStream *bstream)
   {
      if(!conn->isServerConnection())
      {
         conn->setLastError("Invalid Packet (TargetToEvent::unpack() connection id)");
         return;
      }
      if(bstream->readFlag())
      {
         mTargetId = bstream->readInt(TargetManager::TargetIdBitSize);
         TargetInfo *clTarget = gTargetManager->getClientTarget(mTargetId);
         mTarget = clTarget->targetObject;
      }
      if(bstream->readFlag())
      {
         bstream->read(&mTargetPos.x);
         bstream->read(&mTargetPos.y);
         bstream->read(&mTargetPos.z);
      }
      else if(bool(mTarget))
      {
         Box3F box = mTarget->getWorldBox();
         mTargetPos = box.min + (box.max - box.min) * 0.5;
      }
      mAssign = bstream->readFlag();
   }
   void process(NetConnection* conn)
   {
      conn; // unused
      ClientTarget *targetObject = new ClientTarget(mTargetType, mTargetId, mTargetPos);
      targetObject->registerObject();
      conn->addObject(targetObject);
      targetObject->mType = mAssign ? ClientTarget::AssignedTask : ClientTarget::PotentialTask;
      targetObject->process();
   }
   DECLARE_CONOBJECT(TargetToEvent);
};

class SimDataBlockEvent : public NetEvent
{
   SimObjectId id;
   SimDataBlock *mObj;
   U32 mIndex;
   U32 mTotal;
   U32 mMissionSequence;
   bool mProcess;
  public:
   ~SimDataBlockEvent();
   SimDataBlockEvent(SimDataBlock* obj = NULL, U32 index = 0, U32 total = 0, U32 missionSequence = 0);
   void pack(NetConnection *, BitStream *bstream);
   void write(NetConnection *, BitStream *bstream);
   void unpack(NetConnection *cptr, BitStream *bstream);
   void process(NetConnection*);
   void notifyDelivered(NetConnection *, bool);
#ifdef DEBUG_NET
   const char *getDebugName();
#endif
   DECLARE_CONOBJECT(SimDataBlockEvent);
};

class Sim2DAudioEvent: public NetEvent
{
  private:
   const AudioProfile *mProfile;

  public:
   Sim2DAudioEvent(const AudioProfile *profile=NULL);
   void pack(NetConnection *, BitStream *bstream);
   void write(NetConnection *, BitStream *bstream);
   void unpack(NetConnection *, BitStream *bstream);
   void process(NetConnection *);
   DECLARE_CONOBJECT(Sim2DAudioEvent);
};

class Sim3DAudioEvent: public NetEvent
{
  private:
   const AudioProfile *mProfile;
   MatrixF mTransform;

  public:
   Sim3DAudioEvent(const AudioProfile *profile=NULL,const MatrixF* mat=NULL);
   void pack(NetConnection *, BitStream *bstream);
   void write(NetConnection *, BitStream *bstream);
   void unpack(NetConnection *, BitStream *bstream);
   void process(NetConnection *);
   DECLARE_CONOBJECT(Sim3DAudioEvent);
};

//----------------------------------------------------------------------------
// - An event to set the active image slot for a control object.  This is needed
//   for the targeting system to determing which weapon image is currently
//   active on the server.
// - Clients reset the active image (set to 0) when control objects change 
//   (so the server only needs to send events on non-default image slots)
// - Not always the active image on THE control object.. could be down the control chain
//----------------------------------------------------------------------------
class SetObjectActiveImageEvent : public NetEvent
{
  private:
   U32   mImageSlot;
   U32   mObjectId;
   
  public:
   SetObjectActiveImageEvent(U32 objectId = 0, U32 imageSlot = 0)
   { mObjectId = objectId, mImageSlot = imageSlot; }
   void pack(NetConnection *, BitStream * bstream)
   {
      bstream->writeRangedU32(mObjectId, 0, NetConnection::MaxGhostCount-1);
      bstream->writeRangedU32(mImageSlot, 0, ShapeBase::MaxMountedImages); 
   }
   void write(NetConnection * con, BitStream * bstream)
   { pack(con, bstream); }
   void unpack(NetConnection *, BitStream * bstream)
   { 
      mObjectId = bstream->readRangedU32(0, NetConnection::MaxGhostCount-1);
      mImageSlot = bstream->readRangedU32(0, ShapeBase::MaxMountedImages); 
   }
   void process(NetConnection * con)
   {  
      ShapeBase * obj = dynamic_cast<ShapeBase*>(con->resolveGhost(mObjectId));
      if(obj)
         obj->setActiveImage(mImageSlot);
   }

   DECLARE_CONOBJECT(SetObjectActiveImageEvent);
};

//----------------------------------------------------------------------------
// used to set the crc for the current mission (mission lighting)
//----------------------------------------------------------------------------
class SetMissionCRCEvent : public NetEvent
{
   private:
      U32   mCrc;

   public:
      SetMissionCRCEvent(U32 crc = 0xffffffff)
         { mCrc = crc; }
      void pack(NetConnection *, BitStream * bstream)
         { bstream->write(mCrc); }
      void write(NetConnection * con, BitStream * bstream)
         { pack(con, bstream); }
      void unpack(NetConnection *, BitStream * bstream)
         { bstream->read(&mCrc); }
      void process(NetConnection * con)
         { static_cast<GameConnection*>(con)->setMissionCRC(mCrc); }

      DECLARE_CONOBJECT(SetMissionCRCEvent);
};

#endif
