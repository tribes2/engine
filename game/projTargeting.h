//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _H_TARGETPROJECTILE
#define _H_TARGETPROJECTILE

#ifndef _PROJECTILE_H_
#include "game/projectile.h"
#endif
#ifndef _COLOR_H_
#include "Core/color.h"
#endif
#ifndef _WEAPONBEAM_H_
#include "game/weaponBeam.h"
#endif

// -------------------------------------------------------------------------
class TargetProjectileData : public ProjectileData
{
   typedef ProjectileData Parent;

  protected:
   bool onAdd();

   //-------------------------------------- Console set variables
  public:

   enum TargetTextures
   {
      TT_BEAM = 0,
      TT_FLARE,
      TT_PULSE,
      TT_END_FLARE,
      TT_NUM_TEX
   };

   F32      maxRifleRange;
   ColorF   beamColor;

   F32      startBeamWidth;
   F32      beamFlareAngle;
   F32      minFlareSize;
   F32      maxFlareSize;
   F32      pulseBeamWidth;
   F32      pulseSpeed;
   F32      pulseLength;
   bool     coupleBeam;

   StringTableEntry textureName[TT_NUM_TEX];


   //-------------------------------------- load set variables
  public:
   TextureHandle textureHandle[TT_NUM_TEX];

  public:
   TargetProjectileData();
   ~TargetProjectileData();

	//aiming used by AI
	bool calculateAim(const Point3F& targetPos,
	                  const Point3F& targetVel,
	                  const Point3F& sourcePos,
	                  const Point3F& /*sourceVel*/,
	                  Point3F*       outputVectorMin,
	                  F32*           outputMinTime,
	                  Point3F*       outputVectorMax,
	                  F32*           outputMaxTime);

   void packData(BitStream*);
   void unpackData(BitStream*);
   bool preload(bool server, char errorBuffer[256]);

   DECLARE_CONOBJECT(TargetProjectileData);
   static void initPersistFields();
};


// -------------------------------------------------------------------------
class TargetProjectile : public Projectile
{
   typedef Projectile Parent;

  private:
   TargetProjectileData* mDataBlock;

  protected:
   enum TargetProjNetMasks {
      ResetEndPointMask = Parent::NextFreeMask,
      NextFreeMask      = Parent::NextFreeMask << 1
   };

  protected:

   WeaponBeam  mBeam;
         
   F32   mElapsedTime;
   bool  mTruncated;
   bool  mClientOwned;         // True if this ghost is on the client
                                 //  that fired the targeting laser


   // Client warping variables
   U32     mClientTotalWarpTicks;
   U32     mClientWarpTicks;
   Point3F mClientWarpFrom;
   Point3F mClientWarpTo;
   void setClientWarp(const Point3F&);

  protected:
   bool onAdd();
   void onRemove();
   bool onNewDataBlock(GameBaseData*);

   // Rendering
  protected:
   bool checkForFlare( F32 &flareScale, const Point3F &camPos );
   bool prepRenderImage(SceneState*, const U32, const U32, const bool);
   void renderBeam( WeaponBeam &beam, F32 percentDone );
   void renderFlare( ColorF flareColor, F32 flareScale );
   void renderObject(SceneState*, SceneRenderImage*);

	bool calculateImpact(float    simTime,
	                     Point3F& pointOfImpact,
                        float&   impactTime);

   void updateBeamData( BeamData &beamData, const Point3F &camPos );
   void renderEndFlare( F32 flareSize );
   bool canRenderEndFlare( const Point3F &camPos );



  public:
   TargetProjectile();
   ~TargetProjectile();

   bool getTarget(Point3F* pTarget, U32* pTeamId);

   // Time/Move Management
  public:
   void processTick(const Move*);
   void interpolateTick(F32);
   virtual void advanceTime(F32);

   DECLARE_CONOBJECT(TargetProjectile);
   static void initPersistFields();
   static void consoleInit();

   U32  packUpdate(NetConnection*, U32 mask, BitStream* stream);
   void unpackUpdate(NetConnection*, BitStream* stream);
};

#endif // _H_TARGETPROJECTILE

