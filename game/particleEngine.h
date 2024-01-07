//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _H_PARTICLEEMITTER
#define _H_PARTICLEEMITTER

#ifndef _GAMEBASE_H_
#include "game/gameBase.h"
#endif
#ifndef _COLOR_H_
#include "Core/color.h"
#endif

//-------------------------------------- Engine initialization...
//
namespace ParticleEngine {

   enum ParticleConsts
   {
      PC_COLOR_KEYS = 4,
      PC_SIZE_KEYS = 4,
   };

   void init();
   void destroy();
   
   extern Point3F windVelocity;
   inline void setWindVelocity(const Point3F & vel) { windVelocity = vel; }
   inline Point3F getWindVelocity() { return windVelocity; }
}


//--------------------------------------------------------------------------
//-------------------------------------- The data and the Emitter class
//                                        are all that the game should deal
//                                        with (other than initializing the
//                                        global engine pointer of course)
//
struct Particle;
class  ParticleData;

//--------------------------------------
class ParticleEmitterData : public GameBaseData {
   typedef GameBaseData Parent;

  public:
   ParticleEmitterData();
   DECLARE_CONOBJECT(ParticleEmitterData);
   static void initPersistFields();
   void packData(BitStream* stream);
   void unpackData(BitStream* stream);
   bool preload(bool server, char errorBuffer[256]);

   bool onAdd();

  public:
   S32   ejectionPeriodMS;
   S32   periodVarianceMS;

   F32   ejectionVelocity;
   F32   velocityVariance;
   F32   ejectionOffset;

   F32   thetaMin;
   F32   thetaMax;

   F32   phiReferenceVel;
   F32   phiVariance;

   U32   lifetimeMS;
   U32   lifetimeVarianceMS;

   bool  overrideAdvance;
   bool  orientParticles;
   bool  orientOnVelocity;
   bool  useEmitterSizes;
   bool  useEmitterColors;

   StringTableEntry      particleString;
   Vector<ParticleData*> particleDataBlocks;
   Vector<U32>           dataBlockIds;
};


//--------------------------------------
class ParticleEmitter : public GameBase
{
   typedef GameBase Parent;
   friend class PEngine;

  public:
   ParticleEmitter();
   ~ParticleEmitter();

   void setSizes( F32 *sizeList );
   void setColors( ColorF *colorList );
   ParticleEmitterData *getDataBlock(){ return mDataBlock; }
   bool onNewDataBlock(GameBaseData* dptr);

   // By default, a particle renderer will wait for it's owner to delete it.  When this
   //  is turned on, it will delete itself as soon as it's particle count drops to zero.
   void deleteWhenEmpty(); 

   // Main interface for creating particles.  The emitter does _not_ track changes
   //  in axis or velocity over the course of a single update, so this should be called
   //  at a fairly fine grain.  The emitter will potentially track the last particle
   //  to be created into the next call to this function in order to create a uniformly
   //  random time distribution of the particles.  If the object to which the emitter is
   //  attached is in motion, it should try to ensure that for call (n+1) to this
   //  function, start is equal to the end from call (n).  This will ensure a uniform
   //  spatial distribution.
   void emitParticles(const Point3F& start,
                      const Point3F& end,
                      const Point3F& axis,
                      const Point3F& velocity,
                      const U32      numMilliseconds);
   void emitParticles(const Point3F& point,
                      const bool     useLastPosition,
                      const Point3F& axis,
                      const Point3F& velocity,
                      const U32      numMilliseconds);
   void emitParticles(const Point3F& rCenter,
                      const Point3F& rNormal,
                      const F32      radius,
                      const Point3F& velocity,
                      S32 count);

   // Internal interface
  protected:
   void addParticle(const Point3F&, const Point3F&, const Point3F&, const Point3F&);
   void renderBillboardParticle( Particle &part, Point3F *basePnts, MatrixF &camView, F32 spinFactor );
   void renderOrientedParticle( Particle &part, const Point3F &camPos );
   void updateBBox();

  protected:
   bool onAdd();
   void onRemove();

   void processTick(const Move*);
   void advanceTime(F32);

   // Rendering
  protected:
   bool prepRenderImage(SceneState*, const U32, const U32, const bool);
   void renderObject(SceneState*, SceneRenderImage*);

   // PEngine interface
  private:
   void stealParticle(Particle*);

  private:
   ParticleEmitterData* mDataBlock;

   Particle* mParticleListHead;

   U32       mInternalClock;

   U32       mNextParticleTime;

   Point3F   mLastPosition;
   bool      mHasLastPosition;

   bool      mDeleteWhenEmpty;
   bool      mDeleteOnTick;

   S32       mLifetimeMS;
   S32       mElapsedTimeMS;

   F32       sizes[ParticleEngine::PC_SIZE_KEYS];
   ColorF    colors[ParticleEngine::PC_COLOR_KEYS];
};

#endif // _H_PARTICLEEMITTER

