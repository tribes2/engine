//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _H_TRACERPROJECTILE
#define _H_TRACERPROJECTILE

#ifndef _LINEARPROJECTILE_H_
#include "game/linearProjectile.h"
#endif
#ifndef _COLOR_H_
#include "Core/color.h"
#endif

struct DebrisData;

// -------------------------------------------------------------------------
class TracerProjectileData : public LinearProjectileData
{
   typedef LinearProjectileData Parent;

   enum Constants
   {
      NUM_TEX = 2,
   };

  protected:
   bool onAdd();

   //-------------------------------------- Console set variables
  public:
   F32      tracerLength;
   bool     tracerAlpha;
   F32      tracerMinPixels;
   ColorF   tracerColor;
   F32      tracerWidth;
   F32      crossViewAng;
   F32      crossSize;
   bool     renderCross;
         
   StringTableEntry  textureName[NUM_TEX];
   TextureHandle     textureHandle[NUM_TEX];

  public:
   TracerProjectileData();
   ~TracerProjectileData();

   void packData(BitStream*);
   void unpackData(BitStream*);
   bool preload(bool server, char errorBuffer[256]);

   DECLARE_CONOBJECT(TracerProjectileData);
   static void initPersistFields();
};


// -------------------------------------------------------------------------
class TracerProjectile : public LinearProjectile
{
   typedef LinearProjectile Parent;

  private:
   TracerProjectileData* mDataBlock;

  protected:
   bool onAdd();
   void onRemove();
   bool onNewDataBlock(GameBaseData*);

   // Rendering
  protected:
   bool     prepRenderImage(SceneState*, const U32, const U32, const bool);
   void     renderObject(SceneState*, SceneRenderImage*);
   void     renderProjectile( const Point3F &camPos );
   void     renderCrossSection( const Point3F &camPos );

   F32           mCurrLength;

  public:
   TracerProjectile();
   ~TracerProjectile();

   // Time/Move Management
  public:
   void advanceTime(F32);

   DECLARE_CONOBJECT(TracerProjectile);
};

#endif // _H_TRACERPROJECTILE

