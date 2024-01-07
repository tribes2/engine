//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "game/particleEmitter.h"
#include "game/particleEngine.h"
#include "Core/bitStream.h"
#include "console/consoleTypes.h"
#include "console/objectTypes.h"
#include "Math/mathIO.h"

IMPLEMENT_CO_DATABLOCK_V1(ParticleEmissionDummyData);
IMPLEMENT_CO_NETOBJECT_V1(ParticleEmissionDummy);

//--------------------------------------------------------------------------
//--------------------------------------
//
ParticleEmissionDummyData::ParticleEmissionDummyData()
{
   timeMultiple = 1.0;
}

ParticleEmissionDummyData::~ParticleEmissionDummyData()
{

}


//--------------------------------------------------------------------------
void ParticleEmissionDummyData::initPersistFields()
{
   Parent::initPersistFields();

   addField("timeMultiple", TypeF32, Offset(timeMultiple, ParticleEmissionDummyData));
}


//--------------------------------------------------------------------------
bool ParticleEmissionDummyData::onAdd()
{
   if(!Parent::onAdd())
      return false;

   if (timeMultiple < 0.01 || timeMultiple > 100) {
      Con::warnf("ParticleEmissionDummyData::onAdd(%s): timeMultiple must be between 0.01 and 100", getName());
      timeMultiple = timeMultiple < 0.01 ? 0.01 : 100;
   }

   return true;
}


bool ParticleEmissionDummyData::preload(bool server, char errorBuffer[256])
{
   if (Parent::preload(server, errorBuffer) == false)
      return false;

   return true;
}


//--------------------------------------------------------------------------
void ParticleEmissionDummyData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->write(timeMultiple);
}

void ParticleEmissionDummyData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   stream->read(&timeMultiple);
}


//--------------------------------------------------------------------------
//--------------------------------------
//
ParticleEmissionDummy::ParticleEmissionDummy()
{
   // Todo: ScopeAlways?
   mNetFlags.set(Ghostable);
   mTypeMask |= EnvironmentObjectType;

   mEmitterDatablock   = NULL;
   mEmitterDatablockId = 0;
   mEmitter            = NULL;
   mVelocity           = 1.0;
}

ParticleEmissionDummy::~ParticleEmissionDummy()
{
   //
}

//--------------------------------------------------------------------------
void ParticleEmissionDummy::initPersistFields()
{
   Parent::initPersistFields();

   addField("emitter",  TypeParticleEmitterDataPtr, Offset(mEmitterDatablock, ParticleEmissionDummy));
   addField("velocity", TypeF32,                    Offset(mVelocity,         ParticleEmissionDummy));
}


void ParticleEmissionDummy::consoleInit()
{
   //
}


//--------------------------------------------------------------------------
bool ParticleEmissionDummy::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   if (!mEmitterDatablock && mEmitterDatablockId != 0) {
      if (Sim::findObject(mEmitterDatablockId, mEmitterDatablock) == false)
         Con::errorf(ConsoleLogEntry::General, "ParticleEmissionDummy::onAdd: Invalid packet, bad datablockId(mEmitterDatablock): %d", mEmitterDatablockId);
   }

   if (mEmitterDatablock == NULL)
      return false;

   if (isClientObject()) {
      ParticleEmitter* pEmitter = new ParticleEmitter;
      pEmitter->onNewDataBlock(mEmitterDatablock);
      if (pEmitter->registerObject() == false) {
         Con::warnf(ConsoleLogEntry::General, "Could not register base emitter for particle of class: %s", mDataBlock->getName());
         delete pEmitter;
         return false;
      }
      mEmitter = pEmitter;
   }

   mObjBox.min.set(-0.5, -0.5, -0.5);
   mObjBox.max.set( 0.5,  0.5,  0.5);
   resetWorldBox();
   addToScene();

   return true;
}


void ParticleEmissionDummy::onRemove()
{
   removeFromScene();
   if (isClientObject()) {
      mEmitter->deleteWhenEmpty();
      mEmitter = NULL;
   }

   Parent::onRemove();
}


bool ParticleEmissionDummy::onNewDataBlock(GameBaseData* dptr)
{
   mDataBlock = dynamic_cast<ParticleEmissionDummyData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr))
      return false;

   // Todo: Uncomment if this is a "leaf" class
   scriptOnNewDataBlock();
   return true;
}


//--------------------------------------------------------------------------
void ParticleEmissionDummy::advanceTime(F32 dt)
{
   Parent::advanceTime(dt);

   Point3F emitPoint, emitVelocity;
   Point3F emitAxis(0, 0, 1);
   getTransform().mulV(emitAxis);
   getTransform().getColumn(3, &emitPoint);
   emitVelocity = emitAxis * mVelocity;

   mEmitter->emitParticles(emitPoint, emitPoint,
                           emitAxis,
                           emitVelocity, (dt * mDataBlock->timeMultiple * 1000.0f));
}


//--------------------------------------------------------------------------
U32 ParticleEmissionDummy::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   mathWrite(*stream, getTransform());
   mathWrite(*stream, getScale());
   if (stream->writeFlag(mEmitterDatablock != NULL)) {
      stream->writeRangedU32(mEmitterDatablock->getId(), DataBlockObjectIdFirst,
                                                         DataBlockObjectIdLast);
   }

   return retMask;
}

void ParticleEmissionDummy::unpackUpdate(NetConnection* con, BitStream* stream)
{
   Parent::unpackUpdate(con, stream);

   MatrixF temp;
   Point3F tempScale;
   mathRead(*stream, &temp);
   mathRead(*stream, &tempScale);

   if (stream->readFlag()) {
      mEmitterDatablockId = stream->readRangedU32(DataBlockObjectIdFirst,
                                                  DataBlockObjectIdLast);
   } else {
      mEmitterDatablockId = 0;
   }

   setScale(tempScale);
   setTransform(temp);
}

