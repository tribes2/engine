//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _SCENEROOT_H_
#define _SCENEROOT_H_

//Includes
#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif
#ifndef _SCENEOBJECT_H_
#include "Sim/sceneObject.h"
#endif

class SceneRoot : public SceneObject
{
   typedef SceneObject Parent;

  protected:
   bool onSceneAdd(SceneGraph*);
   void onSceneRemove();

   bool getOverlappingZones(SceneObject*, U32* zones, U32* numZones);

   bool prepRenderImage(SceneState*, const U32 stateKey, const U32 startZone,
                        const bool modifyBaseZoneState);

   bool scopeObject(const Point3F&        rootPosition,
                    const F32             rootDistance,
                    bool*                 zoneScopeState);
   
  public:
   SceneRoot();
   ~SceneRoot();
};

extern SceneRoot* gClientSceneRoot;
extern SceneRoot* gServerSceneRoot;

#endif //_SCENEROOT_H_
