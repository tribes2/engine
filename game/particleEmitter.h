//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _H_PARTICLEEMISSIONDUMMY
#define _H_PARTICLEEMISSIONDUMMY

#ifndef _GAMEBASE_H_
#include "game/gameBase.h"
#endif

class ParticleEmitterData;
class ParticleEmitter;

// -------------------------------------------------------------------------
class ParticleEmissionDummyData : public GameBaseData
{
   typedef GameBaseData Parent;

  protected:
   bool onAdd();

   //-------------------------------------- Console set variables
  public:
   F32 timeMultiple;

   //-------------------------------------- load set variables
  public:

  public:
   ParticleEmissionDummyData();
   ~ParticleEmissionDummyData();

   void packData(BitStream*);
   void unpackData(BitStream*);
   bool preload(bool server, char errorBuffer[256]);

   DECLARE_CONOBJECT(ParticleEmissionDummyData);
   static void initPersistFields();
};


// -------------------------------------------------------------------------
class ParticleEmissionDummy : public GameBase
{
   typedef GameBase Parent;

  private:
   ParticleEmissionDummyData* mDataBlock;

  protected:
   bool onAdd();
   void onRemove();
   bool onNewDataBlock(GameBaseData*);

   ParticleEmitterData* mEmitterDatablock;
   S32                  mEmitterDatablockId;

   ParticleEmitter* mEmitter;
   F32              mVelocity;

  public:
   ParticleEmissionDummy();
   ~ParticleEmissionDummy();

   // Time/Move Management
  public:
   void advanceTime(F32);

   DECLARE_CONOBJECT(ParticleEmissionDummy);
   static void initPersistFields();
   static void consoleInit();

   U32  packUpdate(NetConnection*, U32 mask, BitStream* stream);
   void unpackUpdate(NetConnection*, BitStream* stream);
};

#endif // _H_PARTICLEEMISSIONDUMMY

