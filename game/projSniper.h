//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _H_SNIPERPROJECTILE
#define _H_SNIPERPROJECTILE

#ifndef _PROJECTILE_H_
#include "game/projectile.h"
#endif
#ifndef _COLOR_H_
#include "Core/color.h"
#endif
#ifndef _WEAPONBEAM_H_
#include "game/weaponBeam.h"
#endif

class SplashData;


// -------------------------------------------------------------------------
class SniperProjectileData : public ProjectileData
{
   typedef ProjectileData Parent;

  protected:
   bool onAdd();

   //-------------------------------------- Console set variables
  public:
   
   enum SnipeTextures
   {
      ST_FLARE = 0,
      ST_BEAM,
      ST_RIP1,
      ST_RIP2,
      ST_RIP3,
      ST_RIP4,
      ST_RIP5,
      ST_RIP6,
      ST_RIP7,
      ST_RIP8,
      ST_RIP9,
      ST_BEAM2,
      ST_NUM_TEX
   };

   F32      maxRifleRange;
   F32      rifleHeadMultiplier;
   ColorF   beamColor;
   F32      fadeTime;
   F32      startBeamWidth;
   F32      endBeamWidth;
   F32      beamFlareAngle;
   F32      minFlareSize;
   F32      maxFlareSize;
   F32      pulseBeamWidth;
   F32      pulseSpeed;
   F32      pulseLength;
   F32      lightRadius;
   ColorF   lightColor;

   StringTableEntry textureName[ST_NUM_TEX];

   //-------------------------------------- load set variables
  public:
   TextureHandle textureHandle[ST_NUM_TEX];


  public:
   SniperProjectileData();
   ~SniperProjectileData();

   

   bool calculateAim(const Point3F& targetPos,
                     const Point3F& targetVel,
                     const Point3F& sourcePos,
                     const Point3F& sourceVel,
                     Point3F*       outputVectorMin,
                     F32*           outputMinTime,
                     Point3F*       outputVectorMax,
                     F32*           outputMaxTime);

   void packData(BitStream*);
   void unpackData(BitStream*);
   bool preload(bool server, char errorBuffer[256]);

   DECLARE_CONOBJECT(SniperProjectileData);
   static void initPersistFields();
};


// -------------------------------------------------------------------------
class SniperProjectile : public Projectile
{
   typedef Projectile Parent;

  private:
   SniperProjectileData* mDataBlock;

  protected:

   WeaponBeam  mBeam;

   S32     mDeleteWaitTicks;
   F32     mEnergyPercentage;

   bool    mHitWater;
   bool    mTruncated;
   bool    mClientOwned;         // True if this ghost is on the client
                                 //  that fired the targeting laser

   // Client warping variables
   U32     mClientTotalWarpTicks;
   U32     mClientWarpTicks;
   Point3F mClientWarpFrom;
   Point3F mClientWarpTo;
   void setClientWarp(const Point3F&);

   F32 mClientTime;

  protected:
   bool onAdd();
   void onRemove();
   bool onNewDataBlock(GameBaseData*);
   void registerLights(LightManager * lightManager, bool lightingScene);

   // Rendering
  protected:
   bool prepRenderImage(SceneState*, const U32, const U32, const bool);
   void renderBeam( WeaponBeam &beam, F32 percentDone );
   void renderObject(SceneState*, SceneRenderImage*);
   void renderFlare( ColorF, F32 flareScale );
	bool calculateImpact(F32    simTime,
	                     Point3F& pointOfImpact,
                        F32&   impactTime);

   void  updateBeamData( BeamData &beamData, const Point3F &camPos );
   bool  checkForFlare( F32 &flareScale, const Point3F &camPos );
   F32 getUpdatePriority(CameraScopeQuery *camInfo, U32 updateMask, S32 updateSkips);

  public:


   SniperProjectile();
   ~SniperProjectile();

   // Time/Move Management
  public:
   void setEnergyPercentage(F32);
   void processTick(const Move*);
   void interpolateTick(F32);
   virtual void advanceTime(F32);

   DECLARE_CONOBJECT(SniperProjectile);

   U32  packUpdate(NetConnection*, U32 mask, BitStream* stream);
   void unpackUpdate(NetConnection*, BitStream* stream);
};

#endif // _H_SNIPERPROJECTILE

