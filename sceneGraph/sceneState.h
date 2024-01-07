//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _SCENESTATE_H_
#define _SCENESTATE_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif
#ifndef _PLATFORMASSERT_H_
#include "platform/platformAssert.h"
#endif
#ifndef _TVECTOR_H_
#include "core/tVector.h"
#endif
#ifndef _MRECT_H_
#include "math/mRect.h"
#endif
#ifndef _MMATRIX_H_
#include "math/mMatrix.h"
#endif
#ifndef _COLOR_H_
#include "core/color.h"
#endif
#ifndef _GTEXMANAGER_H_
#include "dgl/gTexManager.h"
#endif

class SceneObject;

class SceneRenderImage
{
  public:
   enum SortType
   {
      Sky     = 0,
      Terrain = 1,
      Normal  = 2,
      // Only valid when isTranslucent == true
      Point,   // If point sort, poly[0] is the sort point
      Plane,
      EndSort,
      BeginSort
   };

   SceneRenderImage()
      : sortType(Normal),
        isTranslucent(false),
        tieBreaker(false),
        useSmallTextures(false),
        textureSortKey(0)
   {
      //
   }
   virtual ~SceneRenderImage();

   SceneObject* obj;

   bool     isTranslucent;
   bool     tieBreaker;
   bool     useSmallTextures;

   SortType sortType;
   PlaneF   plane;
   Point3F  poly[4];
   F32 polyArea;
   F32 pointDistSq;

   U32 textureSortKey;
   
   // NEVER set this.
   SceneRenderImage* pNext;
};

//--------------------------------------------------------------------------
//-------------------------------------- SceneState
//

struct FogVolume;

class SceneState
{
   friend class SceneGraph;

  public:
   struct FogBand
   {
      bool isFog;
      float cap;
      float factor;
      ColorF color;
   };

   struct ZoneState {
      bool  render;
      F64   frustum[4];    // l r b t
      RectI viewport;

      bool   clipPlanesValid;
      PlaneF clipPlanes[5];
   };
   void setupClipPlanes(ZoneState&);
   
   struct TransformPortal {
      SceneObject* owner;
      U32          portalIndex;
      U32          globalZone;
      Point3F      traverseStart;
      bool         flipCull;
   };

  public:
   SceneState(SceneState*    parent,
              const U32      numZones,
              F64            left,
              F64            right,
              F64            bottom,
              F64            top,
              F64            nearPlane,
              F64            farPlane,
              RectI          viewport,
              const Point3F& camPos,
              const MatrixF& modelview,
              F32            fogDistance,
              F32            visibleDistance,
              ColorF         fogColor,
              U32            numFogVolumes,
              FogVolume      *fogVolumes,
              TextureHandle  environmentMap,
              F32            visFactor);
              
   ~SceneState();
   void setPortal(SceneObject*, const U32);

   void setImageRefPoint(SceneObject*, SceneRenderImage*) const;
   
   const Point3F& getCameraPosition() const { return mCamPosition; }
   F64            getNearPlane()      const { return mNearPlane;   }
   F64            getFarPlane()       const { return mFarPlane;    }

   const ZoneState& getBaseZoneState() const;
   ZoneState& getBaseZoneStateNC();

   const ZoneState& getZoneState(const U32 zoneId) const;
   ZoneState&       getZoneStateNC(const U32 zoneId);

   void insertRenderImage(SceneRenderImage*);
   void insertTransformPortal(SceneObject* owner, U32 portalIndex,
                              U32 globalZone,     const Point3F& traversalStartPoint,
                              const bool flipCull);

   void enableTerrainOverride();
   bool isTerrainOverridden() const;

   void renderCurrentImages();

   // Utility functions to be used in rendering.  SetupZoneProjection
   //  sets the viewport and projection up for the given zone.  setupObjectProjection
   //  unions all of an objects zones frustums.
   bool isObjectRendered(const SceneObject*);

   void setupZoneProjection(const U32 zone);
   void setupObjectProjection(const SceneObject*);
   void setupBaseProjection();
   F32 getVisibleDistance();
   F32 getFogDistance();
   ColorF getFogColor();

   TextureHandle getEnvironmentMap() { return mEnvironmentMap; }

   void setupFog();
   bool isBoxFogVisible(F32 dist, F32 top, F32 bottom);

   F32 getHazeAndFog(float dist, float deltaZ);
   F32 getFog(float dist, float deltaZ);
   F32 getFog(float dist, float deltaZ, S32 volKey);
   void getFogs(float dist, float deltaZ, ColorF *array, U32 &numFogs);   
   F32 getHaze(F32 dist);
   
  private:
   struct RenderBSPNode
   {
      PlaneF            plane;
      SceneRenderImage* riList;
      U16               frontIndex;
      U16               backIndex;
      SceneRenderImage* rimage;
   };
   Vector<RenderBSPNode> mTranslucentBSP;

   
   Vector<ZoneState>         mZoneStates;
   Vector<SceneRenderImage*> mRenderImages;
   Vector<SceneRenderImage*> mTranslucentPlaneImages;
   Vector<SceneRenderImage*> mTranslucentPointImages;
   Vector<SceneRenderImage*> mTranslucentBeginImages;
   Vector<SceneRenderImage*> mTranslucentEndImages;
   void sortRenderImages();
   void buildTranslucentBSP();
   void insertIntoNode(RenderBSPNode&, SceneRenderImage*, bool rendered = true);
   void renderNode(RenderBSPNode&);
   
   bool mTerrainOverride;

   // Closely related.  Transform portals are turned into sorted mSubsidiaries
   //  by the traversal process...
   Vector<SceneState*>       mSubsidiaries;
   Vector<TransformPortal>   mTransformPortals;

   Vector<FogBand> mPosFogBands;
   Vector<FogBand> mNegFogBands;

   ZoneState mBaseZoneState;
   Point3F   mCamPosition;

   F64       mNearPlane;
   F64       mFarPlane;

   F32       mVisFactor;
   
   SceneState* mParent;

   SceneObject* mPortalOwner;
   U32          mPortalIndex;

   ColorF   mFogColor;
   F32 mFogDistance;
   F32 mVisibleDistance;
   F32 mFogScale;
   
   U32 mNumFogVolumes;
   FogVolume *mFogVolumes;

   TextureHandle mEnvironmentMap;

  public:
   bool    mFlipCull;
   MatrixF mModelview;
   Vector<FogBand> *getPosFogBands() { return &mPosFogBands; }
   Vector<FogBand> *getNegFogBands() { return &mNegFogBands; }
};

inline F32 SceneState::getHaze(F32 dist)
{
   if(dist <= mFogDistance)
      return 0;

   if (dist > mVisibleDistance)
      return 1.0;

   F32 distFactor = (dist - mFogDistance) * mFogScale - 1.0;
   return 1.0 - distFactor * distFactor;
}

inline F32 SceneState::getVisibleDistance()
{
   return mVisibleDistance;
}

inline F32 SceneState::getFogDistance()
{
   return mFogDistance;
}

inline ColorF SceneState::getFogColor()
{
   return mFogColor;
}

inline const SceneState::ZoneState& SceneState::getBaseZoneState() const
{
   return mBaseZoneState;
}

inline SceneState::ZoneState& SceneState::getBaseZoneStateNC()
{
   return mBaseZoneState;
}

inline const SceneState::ZoneState& SceneState::getZoneState(const U32 zoneId) const
{
   AssertFatal(zoneId < mZoneStates.size(), "Error, out of bounds zone!");
   return mZoneStates[zoneId];
}

inline SceneState::ZoneState& SceneState::getZoneStateNC(const U32 zoneId)
{
   AssertFatal(zoneId < mZoneStates.size(), "Error, out of bounds zone!");
   return mZoneStates[zoneId];
}

inline void SceneState::enableTerrainOverride()
{
   mTerrainOverride = true;
}

inline bool SceneState::isTerrainOverridden() const
{
   return mTerrainOverride;
}


#endif  // _H_SCENESTATE_

