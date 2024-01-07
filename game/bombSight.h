//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _BOMBSIGHT_H_
#define _BOMBSIGHT_H_

#ifndef _SCENEOBJECT_H_
#include "sim/sceneObject.h"
#endif
#ifndef _RESMANAGER_H_
#include "core/resManager.h"
#endif
class TSShape;
class TSShapeInstance;

class BombSight : public SceneObject
{
   typedef SceneObject Parent;
   Resource<TSShape> mMoveShape;
   TSShapeInstance* mShapeInstance;
   StringTableEntry mDtsFileName;
   bool mRenderObject;
   F32 mAlpha;   
  protected:
   bool onAdd();
   void onRemove();

   // Rendering
  protected:
   bool prepRenderImage(SceneState*, const U32, const U32, const bool);
   void renderObject(SceneState*, SceneRenderImage*);

  public:
   BombSight();
   ~BombSight();
   
   void setDtsShape(StringTableEntry);
   void setRender(const bool , const F32 alpha = 0.0f);
   
   DECLARE_CONOBJECT(BombSight);
};

#endif // _H_BOMBSIGHT

