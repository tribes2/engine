//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _WATERBLOCK_H_
#define _WATERBLOCK_H_

#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif
#ifndef _MPOINT_H_
#include "Math/mPoint.h"
#endif
#ifndef _SCENEOBJECT_H_
#include "Sim/sceneObject.h"
#endif
#ifndef _GTEXMANAGER_H_
#include "dgl/gTexManager.h"
#endif
#ifndef _COLOR_H_
#include "Core/color.h"
#endif
#ifndef _TERRDATA_H_
#include "terrain/terrData.h"
#endif

#ifndef _FLUID_H_
#include "terrain/Fluid.h"
#endif

//==============================================================================
class AudioEnvironment;

class WaterBlock : public SceneObject
{
    typedef SceneObject Parent;

public:

    enum EWaterType
    {
        eWater            = 0,
        eOceanWater       = 1,
        eRiverWater       = 2,
        eStagnantWater    = 3,
        eLava             = 4,
        eHotLava          = 5,
        eCrustyLava       = 6,
        eQuicksand        = 7,
    };

    enum WaterConst
    {
       WC_NUM_SUBMERGE_TEX = 2,
    };

private:

    fluid               mFluid;

    TextureHandle       mSurfaceTexture;
    TextureHandle       mEnvMapTexture;   
    PlaneF              mClipPlane[6];  // Frustrum clip planes: 0=T 1=B 2=L 3=R 4=N 5=F
    TerrainBlock*       mpTerrain;      // Terrain block
    F32                 mSurfaceZ;

    // Fields exposed to the editor.
    EWaterType          mLiquidType;    // Water?  Lava?  What?
    F32                 mDensity;
    F32                 mViscosity;
    F32                 mWaveMagnitude;
    StringTableEntry    mSurfaceName;         
    F32                 mSurfaceOpacity;
    StringTableEntry    mEnvMapName;       
    F32                 mEnvMapIntensity;
    StringTableEntry    mSubmergeName[WC_NUM_SUBMERGE_TEX];
    bool                mRemoveWetEdges;
    AudioEnvironment *  mAudioEnvironment;

    TextureHandle mLocalSubmergeTexture[WC_NUM_SUBMERGE_TEX];

    static TextureHandle mSubmergeTexture[WC_NUM_SUBMERGE_TEX];

public:

    WaterBlock();
    ~WaterBlock();

    bool onAdd                  ( void );
    void onRemove               ( void );
    bool onSceneAdd             ( SceneGraph* pGraph );
    void setTransform           ( const MatrixF& mat );
    void setScale               ( const VectorF& scale );
    bool prepRenderImage        ( SceneState*, const U32, const U32, const bool );
    void renderObject           ( SceneState*, SceneRenderImage* );
    void inspectPostApply       ( void );
    F32  getSurfaceHeight       ( void ) { return mSurfaceZ; }

    void UpdateFluidRegion      ( void );
    static void SnagTerrain     ( SceneObject* sceneObj, S32 key );

    static void cToggleWireFrame( SimObject*, S32, const char** );

    DECLARE_CONOBJECT(WaterBlock);

    static void initPersistFields();
    static void consoleInit();

//  bool setLiquidType(EWaterFlag liquidType);
    EWaterType getLiquidType() const                 { return mLiquidType; }
    static bool isWater      ( U32 liquidType );
    static bool isLava       ( U32 liquidType );
    static bool isQuicksand  ( U32 liquidType );

    F32 getViscosity() const { return mViscosity; }
    F32 getDensity()   const { return mDensity;   }

    U32  packUpdate  ( NetConnection*, U32 mask, BitStream *stream );
    void unpackUpdate( NetConnection*,           BitStream *stream );

    bool isPointSubmerged      ( const Point3F &pos, bool worldSpace = true ) const;
    bool isPointSubmergedSimple( const Point3F &pos, bool worldSpace = true ) const;

    AudioEnvironment * getAudioEnvironment() { return(mAudioEnvironment); }

    static bool             mCameraSubmerged;
    static U32              mSubmergedType;

    static TextureHandle getSubmergeTexture( U32 index ){ return mSubmergeTexture[index]; }

protected:
    bool castRay( const Point3F& start, const Point3F& end, RayInfo* info );
};

#endif
