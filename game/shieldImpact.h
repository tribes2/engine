//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _SHIELDIMPACT_H_
#define _SHIELDIMPACT_H_

#ifndef _GAMEBASE_H_
#include "game/gameBase.h"
#endif
#ifndef _SCENESTATE_H_
#include "sceneGraph/sceneState.h"
#endif

class ShapeBase;
class TSShapeInstance;
class SceneState;

//--------------------------------------------------------------------------
// ShieldImpact
//--------------------------------------------------------------------------
class ShieldImpact
{
  private:
   TextureHandle        mShieldTexture;
   TextureHandle        mOverrideTex;

   F32         mLifetime;
   F32         mElapsedTime;
   F32         mCurScale;
   F32         mShieldStrength;
   F32         mStartScale;
   F32         mEndScale;

  protected:
   SimObjectPtr<ShapeBase> mShieldedObj;


   // Rendering
  protected:
   void renderObj( ShapeBase *obj, SceneState* state, F32 alpha );
   void renderInstance( TSShapeInstance *shapeInst, SceneState* state, F32 alpha );

  public:
   ShieldImpact();
   ~ShieldImpact();

   void init();
   bool isActive(){ return (mElapsedTime <= mLifetime); }
   void renderObject(SceneState*, SceneRenderImage*);
   void start();
   void setShieldedObj( ShapeBase *obj ){ mShieldedObj = obj; }
   void setShieldStrength( F32 strength ){ mShieldStrength = strength; }
   void setLifetime( F32 lifetime ){ mLifetime = lifetime; }
   void update(F32 dt);
};

//--------------------------------------------------------------------------
// Scene Render Image for shield impact
//--------------------------------------------------------------------------
class ShieldImpactRenderImage : public SceneRenderImage
{
  public:
   void setup( SceneObject *object )
   {
      obj = object;
      isTranslucent = true;
      sortType = SceneRenderImage::Point;
      tieBreaker = true;
   }

};


#endif // _H_SHIELDIMPACT
