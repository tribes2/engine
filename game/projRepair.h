//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _H_REPAIRPROJECTILE
#define _H_REPAIRPROJECTILE

#ifndef _PROJECTILE_H_
#include "game/projectile.h"
#endif
#ifndef _COLOR_H_
#include "Core/color.h"
#endif
#ifndef _MRANDOM_H_
#include "Math/mRandom.h"
#endif
#ifndef _MQUADPATCH_H_
#include "Math/mQuadPatch.h"
#endif

class Point3F;

//******************************************************************************
// Repair projectile data
//******************************************************************************
class RepairProjectileData : public ProjectileData
{
   typedef ProjectileData Parent;

  protected:
   bool onAdd();

  public:
   enum Constants {
      MaxLines = 8
   };

   //-------------------------------------- Console set variables
  public:

   enum RepairTextures
   {
      RT_BEAM = 0,
      RT_FLARE,
      RT_NUM_TEX
   };


   F32         beamRange;
   F32         beamWidth;
   U32         numSegments;
   F32         texRepeat;
   F32         beamSpeed;
   F32         blurFreq;
   F32         blurLifetime;
   F32         cutoffAngle;

   StringTableEntry textureNames[RT_NUM_TEX];
   


   //-------------------------------------- preload set variables
  public:
   TextureHandle  textureList[RT_NUM_TEX];

  public:
   RepairProjectileData();
   ~RepairProjectileData();

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

   DECLARE_CONOBJECT(RepairProjectileData);
   static void initPersistFields();
};


//******************************************************************************
// Repair Projectile
//******************************************************************************
class RepairProjectile : public Projectile
{
   typedef Projectile Parent;

   enum RPConst
   {
      RP_NUM_BLUR = 5,
   };

   struct BlurLine
   {
      F32         elapsedTime;
      F32         lifetime;
      F32         startAlpha;
      ColorF      color;
      QuadPatch   qPatch;

      BlurLine()
      {
         elapsedTime = 1.1;
         lifetime = 1.0;
         startAlpha = 0.5;
      }
   };


  private:
   RepairProjectileData*   mDataBlock;

   SimObjectPtr<ShapeBase> mRepairingObject;
   S32                     mRepairingObjectId;

   BlurLine                mBlurList[RP_NUM_BLUR];

   F32                     mElapsedTime;
   bool                    mInitialHit;
   F32                     mTimeSinceLastBlur;
   
   Point3F                 mCurrStartPoint;
   Point3F                 mCurrStartDir;
   Point3F                 mDesiredEndPoint;
   Point3F                 mCurrEndPoint;    // Endpoint is relative to the center of
                                             //  the objects bounding box...


  protected:
   bool onAdd();
   void onRemove();
   bool onNewDataBlock(GameBaseData*);


   // Rendering
  protected:
   bool     prepRenderImage(SceneState*, const U32, const U32, const bool);
   void     renderObject(SceneState*, SceneRenderImage*);

   void     renderFlare( F32 flareSize );
   bool     canRenderFlare( const Point3F &camPos );

   void     renderBeam( const Point3F &campPos);
   void     updateBlur( F32 dt );


  public:
   RepairProjectile();
   ~RepairProjectile();

   void advanceTime(F32);

   DECLARE_CONOBJECT(RepairProjectile);
   static void initPersistFields();
   static void consoleInit();

   U32  packUpdate(NetConnection*, U32 mask, BitStream* stream);
   void unpackUpdate(NetConnection*, BitStream* stream);
};

#endif // _H_REPAIRPROJECTILE

