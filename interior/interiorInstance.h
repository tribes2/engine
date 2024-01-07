//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _INTERIORINSTANCE_H_
#define _INTERIORINSTANCE_H_

//Includes
#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif
#ifndef _SCENEOBJECT_H_
#include "sim/sceneObject.h"
#endif
#ifndef _RESMANAGER_H_
#include "core/resManager.h"
#endif
#ifndef _INTERIORRES_H_
#include "interior/interiorRes.h"
#endif
#ifndef _INTERIORLMMANAGER_H_
#include "interior/interiorLMManager.h"
#endif

#ifndef _BITVECTOR_H_
#include "core/bitVector.h"
#endif
#ifndef _COLOR_H_
#include "core/color.h"
#endif

class AbstractPolyList;
class LightUpdateGrouper;
class InteriorSubObject;
class InteriorResTrigger;
class MaterialList;
class TextureObject;
class FloorPlan;
class Convex;
class AudioProfile;
class AudioEnvironment;

//--------------------------------------------------------------------------
class InteriorInstance : public SceneObject
{
   typedef SceneObject Parent;
   friend class SceneLighting;
   friend class FloorPlan;

  public:
   InteriorInstance();
   ~InteriorInstance();

   static void init();
   static void destroy();

   // Collision
  public:
   bool buildPolyList(AbstractPolyList*, const Box3F&, const SphereF&);
   bool castRay(const Point3F&, const Point3F&, RayInfo*);
   virtual void setTransform(const MatrixF&);

   void buildConvex(const Box3F& box,Convex* convex);
  private:
   Convex* mConvexList;

   // Lighting control
  public:
   bool inAlarmState() {return(mAlarmState);}
   void setAlarmMode(const bool alarm);
   void activateLight(const char* pLightName);
   void deactivateLight(const char* pLightName);
   void echoTriggerableLights();

   // Subobject access interface
  public:
   U32 getNumDetailLevels();
   Interior* getDetailLevel(const U32);
   void setDetailLevel(S32 level = -1) { mForcedDetailLevel = level; }

   // Material management for overlays
  public:
   void renewOverlays();
   void setSkinBase(const char*);

  public:
   static bool smDontRestrictOutside;
   static F32  smDetailModification;

   
   DECLARE_CONOBJECT(InteriorInstance);
   static void initPersistFields();
   static void consoleInit();

   bool readLightmaps(GBitmap**** lightmaps);
   
  protected:
   bool onAdd();
   void onRemove();

   void inspectPreApply();
   void inspectPostApply();

   static U32 smLightUpdatePeriod;
   static bool smRenderDynamicLights;

   U32  mLightUpdatedTime;
   void setLightUpdatedTime(const U32);
   U32  getLightUpdatedTime() const;

   bool onSceneAdd(SceneGraph*);
   void onSceneRemove();
   U32  getPointZone(const Point3F& p);
   bool getOverlappingZones(SceneObject* obj, U32* zones, U32* numZones);

   U32  calcDetailLevel(SceneState*, const Point3F&);
   bool prepRenderImage(SceneState*, const U32, const U32, const bool);
   void renderObject(SceneState*, SceneRenderImage*);
   bool scopeObject(const Point3F&        rootPosition,
                    const F32             rootDistance,
                    bool*                 zoneScopeState);

  public:
   void addChildren();
   static bool getRenderDynamicLights() { return(smRenderDynamicLights); }
   static void setRenderDynamicLights(bool val) { smRenderDynamicLights = val; }

  private:
   void activateLight(const U32 detail, const U32 lightIndex);
   void deactivateLight(const U32 detail, const U32 lightIndex);

   U32  packUpdate(NetConnection *, U32 mask, BitStream *stream);
   void unpackUpdate(NetConnection *, BitStream *stream);


   enum UpdateMaskBits {
      InitMask       = 1 << 0,
      TransformMask  = 1 << 1,
      AlarmMask      = 1 << 2,

      // Reserved for light updates (8 bits for now)
      _lightupdate0  = 1 << 3,
      _lightupdate1  = 1 << 4,
      _lightupdate2  = 1 << 5,
      _lightupdate3  = 1 << 6,
      _lightupdate4  = 1 << 7,
      _lightupdate5  = 1 << 8,
      _lightupdate6  = 1 << 9,
      _lightupdate7  = 1 << 10,

      SkinBaseMask   = 1 << 11,
      AudioMask      = 1 << 12,
      NextFreeMask   = 1 << 13
   };
   enum Constants {
      LightUpdateBitStart = 3,
      LightUpdateBitEnd   = 10
   };

  private:
   StringTableEntry                     mInteriorFileName;
   U32                                  mInteriorFileHash;
   Resource<InteriorResource>           mInteriorRes;
   Vector<MaterialList*>                mMaterialMaps;
   StringTableEntry                     mSkinBase;

   Vector< Vector<InteriorSubObject*> > mInteriorSubObjects;
   bool                                 mShowTerrainInside;
   LM_HANDLE                            mLMHandle;
   AudioProfile *                       mAudioProfile;
   AudioEnvironment *                   mAudioEnvironment;
   S32                                  mForcedDetailLevel;
   U32                                  mCRC;
   
  public:
   LM_HANDLE getLMHandle() { return(mLMHandle); }
   AudioProfile * getAudioProfile() { return(mAudioProfile); }
   AudioEnvironment * getAudioEnvironment() { return(mAudioEnvironment); }
   bool getPointInsideScale(const Point3F & pos, F32 * pScale);   // ~0: outside -> 1: inside

   // SceneLighting::InteriorProxy interface
   Resource<InteriorResource> & getResource() {return(mInteriorRes);}
   U32 getCRC() { return(mCRC); }

   Vector<Vector<ColorI>*> mVertexColorsNormal;
   Vector<Vector<ColorI>*> mVertexColorsAlarm;
   void rebuildVertexColors();
   Vector<ColorI>* getVertexColorsNormal(U32 detail);
   Vector<ColorI>* getVertexColorsAlarm(U32 detail);
   
   // Alarm state information
  private:
   enum AlarmState {
      Normal          = 0,
      Alarm           = 1
   };

   bool mAlarmState;

   // LightingAnimation information
  private:
   struct LightInfo {
      struct Light {
         U32    curState;
         U32    curTime;
         ColorI curColor;

         bool   active;
         bool   alarm;
      };
      struct StateDataInfo {
         ColorI curColor;
         U8*    curMap;
         bool   alarm;
      };

      Vector<Light> mLights;
      BitVector             mSurfaceInvalid;
      Vector<StateDataInfo> mStateDataInfo;
   };
   Vector<LightInfo>   mLightInfo;     // One for each detail level.
   LightUpdateGrouper* mUpdateGrouper; // Cuts down on net traffic

   static U32 makeUpdateKey(const U32 detail, const U32 lightIndex);
   static U32 detailFromUpdateKey(const U32 key);
   static U32 indexFromUpdateKey(const U32 key);

   // Animated light functions
   void updateLightTime(const U32 detail, const U32 lightIndex, const U32 ms);
   void downloadLightmaps(SceneState*, Interior*, LightInfo&);
   void installLight(const U32 detail, const U32 lightIndex);
   void updateLoopingLight(Interior*, LightInfo::Light&, const U32 lightIndex, const U32 ms);
   void updateFlickerLight(Interior*, LightInfo::Light&, const U32 lightIndex, const U32 ms);
   void updateRampLight(Interior*, LightInfo::Light&, const U32 lightIndex, const U32 ms);
   void updateAllLights(const U32);

   void updateLightMap(Interior*, LightInfo&, const U32 surfaceIndex);
   void intensityMapMerge(U8* lightMap,
                          const U32 width, const U32 height,
                          const U8* intensityMap, const ColorI& color);

  private:
   void createTriggerTransform(const InteriorResTrigger*, MatrixF*);
};

inline void InteriorInstance::setLightUpdatedTime(const U32 now)
{
   mLightUpdatedTime = now;
}

inline U32 InteriorInstance::getLightUpdatedTime() const
{
   return mLightUpdatedTime;
}

inline U32 InteriorInstance::makeUpdateKey(const U32 detail, const U32 lightIndex)
{
   AssertFatal(detail < (1 << 16) && lightIndex < (1 << 16), "Error, out of bounds key params");

   return (detail << 16) | (lightIndex & 0x0000FFFF);
}

inline U32 InteriorInstance::detailFromUpdateKey(const U32 key)
{
   return (key >> 16) & 0xFFFF;
}

inline U32 InteriorInstance::indexFromUpdateKey(const U32 key)
{
   return (key >> 0) & 0xFFFF;
}

inline Vector<ColorI>* InteriorInstance::getVertexColorsNormal(U32 detail)
{
   if (bool(mInteriorRes) == false || detail > mInteriorRes->getNumDetailLevels())
      return NULL;

   return mVertexColorsNormal[detail];
}

inline Vector<ColorI>* InteriorInstance::getVertexColorsAlarm(U32 detail)
{
   if (bool(mInteriorRes) == false || detail > mInteriorRes->getNumDetailLevels())
      return NULL;

   return mVertexColorsAlarm[detail];
}

#endif //_INTERIORBLOCK_H_

