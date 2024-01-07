//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "terrain/waterBlock.h"
#include "console/consoleTypes.h"
#include "sceneGraph/sceneGraph.h"
#include "sceneGraph/sceneState.h"
#include "Core/bitStream.h"
#include "Math/mBox.h"
#include "dgl/dgl.h"
#include "Core/color.h"
#include "terrain/terrData.h"
#include "terrain/terrRender.h"
#include "Math/mathIO.h"
#include "sceneGraph/sgUtil.h"
#include "audio/audioDataBlock.h"

//==============================================================================

IMPLEMENT_CO_NETOBJECT_V1(WaterBlock);

//==============================================================================

bool           WaterBlock::mCameraSubmerged = false;
U32            WaterBlock::mSubmergedType   = 0;
TextureHandle  WaterBlock::mSubmergeTexture[WC_NUM_SUBMERGE_TEX];

//==============================================================================
// I know this is a bit of a hack.  I've done this in order to avoid a coupling
// between the fluid and the rest of the system.
// 
static SceneState* pSceneState = NULL;

static F32 FogFunction( F32 Distance, F32 DeltaZ )
{
    return( pSceneState->getHazeAndFog( Distance, DeltaZ ) );
}

//==============================================================================

WaterBlock::WaterBlock()
{
    mNetFlags.set(Ghostable | ScopeAlways);
    mTypeMask = WaterObjectType;

    mObjBox.min.set( 0, 0, 0 );
    mObjBox.max.set( 1, 1, 1 );

    mLiquidType      = eOceanWater;
    mDensity         =  1; 
    mViscosity       = 15; 
    mWaveMagnitude   = 1.0f; 
    mSurfaceTexture  = TextureHandle();
    mSurfaceOpacity  = 0.75f; 
    mEnvMapTexture   = TextureHandle();
    mEnvMapIntensity = 1.0f;
    mRemoveWetEdges  = true;
    mAudioEnvironment = 0;
    
    // lets be good little programmers and initialize our data!
    mSurfaceName = NULL;
    mEnvMapName = NULL; 

    dMemset( mSubmergeName, 0, sizeof( mSubmergeName ) );

    mpTerrain        = NULL;
    mSurfaceZ        = 0.0f;
    
    mFluid.SetFogFn( FogFunction );
}

//==============================================================================

WaterBlock::~WaterBlock()
{
}

//==============================================================================

void WaterBlock::SnagTerrain( SceneObject* sceneObj, S32 key )
{
    WaterBlock* pWater = (WaterBlock*)key;
    pWater->mpTerrain  = dynamic_cast<TerrainBlock*>(sceneObj);
}

//==============================================================================

void WaterBlock::UpdateFluidRegion( void )
{
    MatrixF M;
    Point3F P;

    P    = mObjToWorld.getPosition();
    P.x += 1024.0f;
    P.y += 1024.0f;

    mSurfaceZ = P.z + mObjScale.z;

    mFluid.SetInfo( P.x,         P.y,
                    mObjScale.x, mObjScale.y,
                    mSurfaceZ,
                    mWaveMagnitude,
                    mSurfaceOpacity,
                    mEnvMapIntensity,                    
                    mRemoveWetEdges );

    P.x -= 1024.0f;
    P.y -= 1024.0f;

    M.identity();
    M.setPosition( P );

    Parent::setTransform( M );

    resetWorldBox();

    if( isServerObject() )
        setMaskBits(1);
}

//==============================================================================

bool WaterBlock::onAdd()
{
    if( !isClientObject() ) 
    {
        // Make sure that the Fluid has had a chance to tweak the values.
        UpdateFluidRegion();
    }

    if( !Parent::onAdd() )
        return false;
        
    if(!mRemoveWetEdges)
    {
       mObjBox.min.set(-1e4, -1e4, 0);
       mObjBox.max.set( 1e4,  1e4, 1);
    }
    
    resetWorldBox();
    addToScene();

    if( isClientObject() ) 
    { 
        // load textures
        mSurfaceTexture  = TextureHandle( mSurfaceName,  MeshTexture );
        mEnvMapTexture   = TextureHandle( mEnvMapName,   MeshTexture );

        for( int i=0; i<WC_NUM_SUBMERGE_TEX; i++ )
        {
           if( mSubmergeName[i] && mSubmergeName[i][0] )
           {
              mLocalSubmergeTexture[i] = TextureHandle( mSubmergeName[i], MeshTexture );
              mSubmergeTexture[i] = mLocalSubmergeTexture[i];
           }
        }
        
        // Register these textures with the Fluid.
        mFluid.SetTextures( mSurfaceTexture,
                            mEnvMapTexture ); 
    }

    return( true );
}

//==============================================================================

void WaterBlock::onRemove()
{

    // clear static texture handles
    for( int i=0; i<WC_NUM_SUBMERGE_TEX; i++ )
    {
        mSubmergeTexture[i] = NULL;
    }
   
    removeFromScene();
    Parent::onRemove();
}

//==============================================================================

bool WaterBlock::onSceneAdd( SceneGraph* pGraph )
{
    if( Parent::onSceneAdd(pGraph) )
    {
        // Attempt to get the terrain.
        if( (mpTerrain == NULL) && (mContainer != NULL) )
        {
            mContainer->findObjects( mWorldBox, (U32)TerrainObjectType, SnagTerrain, (S32)this );
            if( mpTerrain )
                mFluid.SetTerrainData( mpTerrain->heightMap );
        }
   
        return( true );
    }
    return( false );
}

//==============================================================================
// The incoming matrix transforms the water into the WORLD.  This includes any
// offset in the terrain, so the translation can be negative.  We need to get 
// the translation to be expressed in "terrain square" terms.  The terrain 
// offset is always -1024,-1024.

void WaterBlock::setTransform( const MatrixF &mat )
{
    mObjToWorld = mat;
    UpdateFluidRegion();
}

//==============================================================================

void WaterBlock::setScale( const VectorF & scale )
{
    mObjScale = scale;
    UpdateFluidRegion();
}

//==============================================================================

bool WaterBlock::prepRenderImage( SceneState* state, 
                                  const U32   stateKey,
                                  const U32, 
                                  const bool )
{
    // Attempt to get the terrain.
    if( (mpTerrain == NULL) && (mContainer != NULL) )
    {
        mContainer->findObjects( mWorldBox, (U32)TerrainObjectType, SnagTerrain, (S32)this );
        if( mpTerrain )
            mFluid.SetTerrainData( mpTerrain->heightMap );
    }


   if (isLastState(state, stateKey))
      return false;
   setLastState(state, stateKey);

   // This should be sufficient for most objects that don't manage zones, and
   //  don't need to return a specialized RenderImage...
   if (state->isObjectRendered(this)) {
      SceneRenderImage* image = new SceneRenderImage;
      image->obj = this;
      image->isTranslucent = true;

      image->sortType = SceneRenderImage::Plane;
      image->plane    = PlaneF(0, 0, 1, -mSurfaceZ);
      image->poly[0]  = Point3F(mObjBox.min.x, mObjBox.min.y, 1);
      image->poly[1]  = Point3F(mObjBox.min.x, mObjBox.max.y, 1);
      image->poly[2]  = Point3F(mObjBox.max.x, mObjBox.max.y, 1);
      image->poly[3]  = Point3F(mObjBox.max.x, mObjBox.min.y, 1);
      for (U32 i = 0; i < 4; i++)
      {
         image->poly[i].convolve(mObjScale);
         getTransform().mulP(image->poly[i]);
      }
      
      // Calc the area of this poly
      Point3F intermed;
      mCross(image->poly[2] - image->poly[0], image->poly[3] - image->poly[1], &intermed);
      image->polyArea = intermed.len() * 0.5;

      state->insertRenderImage(image);
   }

   return false;
}

//==============================================================================

void WaterBlock::renderObject( SceneState* state, SceneRenderImage* )
{
    AssertFatal( dglIsInCanonicalState(), 
                 "Error, GL not in canonical state on entry" );

    RectI   viewport;
    Point3F Eye;    // Camera in water space.
    bool    CameraSubmergedFlag = false;

    dglGetViewport  ( &viewport );
    glMatrixMode    ( GL_PROJECTION );
    glPushMatrix    ();
    state->setupObjectProjection( this );

/****
    // Debug assist.
    // Render a wire outline around the base of the water block.
    if( 0 )
    {
        glMatrixMode    ( GL_MODELVIEW );
        glPushMatrix    ();
        dglMultMatrix   ( &mObjToWorld );

        F32 X0 = 0;
        F32 Y0 = 0;
        F32 X1 = mObjScale.x;
        F32 Y1 = mObjScale.y;
        F32 Z  = 0;

        glDisable       ( GL_TEXTURE_2D );
        glDisable       ( GL_DEPTH_TEST );
        glEnable        ( GL_BLEND );
        glBlendFunc     ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glPolygonMode   ( GL_FRONT_AND_BACK, GL_LINE );
        glColor4f       ( 0.6f, 0.6f, 0.0f, 0.5f );
        glBegin         ( GL_QUADS );

        glVertex3f      ( X0, Y0, Z );
        glVertex3f      ( X1, Y0, Z );
        glVertex3f      ( X1, Y1, Z );
        glVertex3f      ( X0, Y1, Z );

        glEnd           ();
        glEnable        ( GL_DEPTH_TEST );
        glColor4f       ( 0.8f, 0.8f, 0.8f, 0.8f );
        glBegin         ( GL_QUADS );

        glVertex3f      ( X0, Y0, Z );
        glVertex3f      ( X1, Y0, Z );
        glVertex3f      ( X1, Y1, Z );
        glVertex3f      ( X0, Y1, Z );

        glEnd           ();
        glPolygonMode   ( GL_FRONT_AND_BACK, GL_FILL );

        glPopMatrix     ();
        glMatrixMode    ( GL_MODELVIEW );
    }
****/

    // Handle the fluid.
    {
        // The water block lives in terrain space which is -1024,-1024.
        // To get into world space, we just add 1024,1024.
        // The fluid lives in world space.

        Point3F W2Lv(  1024.0f,  1024.0f, 0.0f );   // World to Local vector
        Point3F L2Wv( -1024.0f, -1024.0f, 0.0f );   // Local to World vector
        MatrixF L2Wm;                               // Local to World matrix

        L2Wm.identity();
        L2Wm.setPosition( L2Wv );

        glMatrixMode    ( GL_MODELVIEW );
        glPushMatrix    ();
        dglMultMatrix   ( &L2Wm );


        // We need the eye in water space.
        {
            Eye = state->getCameraPosition() + W2Lv;
            mFluid.SetEyePosition( Eye.x, Eye.y, Eye.z );
        }

        // We need the frustrum in water space.
        {
           MatrixF L2Cm;
           dglGetModelview( &L2Cm );
           L2Cm.inverse();
           
           Point3F Dummy;
           Dummy = L2Cm.getPosition();
           
           F64 frustumParam[6];
           dglGetFrustum(&frustumParam[0], &frustumParam[1],
                         &frustumParam[2], &frustumParam[3],
                         &frustumParam[4], &frustumParam[5]);
           
           sgComputeOSFrustumPlanes(frustumParam,
                                    L2Cm,
                                    Dummy,
                                    mClipPlane[1],
                                    mClipPlane[2],
                                    mClipPlane[3],
                                    mClipPlane[4],
                                    mClipPlane[5]);
           
           // near plane is needed as well...
           PlaneF p(0, 1, 0, -frustumParam[4]);
           mTransformPlane(L2Cm, Point3F(1,1,1), p, &mClipPlane[0]);
           
//            F64     left, right, bottom, top, near, far;
//             MatrixF L2Cm;

//             dglGetFrustum( &left, &right, &bottom, &top, &near, &far );

//             far = state->getVisibleDistance();

//             Point3F Origin      ( 0,     0,    0      );
//             Point3F FarOrigin   ( 0,     far,  0      );
//             Point3F UpperLeft   ( left,  near, top    );
//             Point3F UpperRight  ( right, near, top    );
//             Point3F LowerLeft   ( left,  near, bottom );
//             Point3F LowerRight  ( right, near, bottom );

//             dglGetModelview( &L2Cm );
// 	        L2Cm.inverse();

//             Point3F Dummy;
// 	        Dummy = L2Cm.getPosition();

//             L2Cm.mulP( Origin     );
//             L2Cm.mulP( FarOrigin  );
//             L2Cm.mulP( UpperLeft  );
//             L2Cm.mulP( UpperRight );
//             L2Cm.mulP( LowerLeft  );
//             L2Cm.mulP( LowerRight );

//             // Frustrum clip planes: 0=T 1=B 2=L 3=R 4=N 5=F

//             mClipPlane[0].set( UpperLeft,  Origin,    UpperRight ); 
//             mClipPlane[1].set( LowerRight, Origin,    LowerLeft  ); 
//             mClipPlane[2].set( LowerLeft,  Origin,    UpperLeft  ); 
//             mClipPlane[3].set( UpperRight, Origin,    LowerRight ); 
//             mClipPlane[4].set( UpperRight, UpperLeft, LowerRight ); 

//             // far plane is reverse of near plane vector
//             mClipPlane[5].x = -mClipPlane[4].x;
//             mClipPlane[5].y = -mClipPlane[4].y;
//             mClipPlane[5].z = -mClipPlane[4].z;
//             mClipPlane[5].d = -mClipPlane[4].d + far;

            mFluid.SetFrustrumPlanes( (F32*)mClipPlane );
        }

        // Fog stuff.
        {
            pSceneState = state;
            ColorF FogColor = state->getFogColor();
            mFluid.SetFogParameters( FogColor.red, 
                                     FogColor.green, 
                                     FogColor.blue,
                                     state->getVisibleDistance() );
        }

        // And RENDER!
        {
            mFluid.Render( CameraSubmergedFlag );
        }

        // Clean up.
        glPopMatrix     ();
        glMatrixMode    ( GL_MODELVIEW );
    }

    //
    // And now the closing ceremonies...
    //
    glMatrixMode    ( GL_PROJECTION );    
    glPopMatrix     ();
    glTexEnvi       ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
    dglSetViewport  ( viewport );

    //
    // Oh yes.  We have to set some state information for the scene...
    //

    if( CameraSubmergedFlag )
    {
        mCameraSubmerged = true;
        mSubmergedType   = mLiquidType;
    }
    
    AssertFatal( dglIsInCanonicalState(), 
                 "Error, GL not in canonical state on exit" );
}

//==============================================================================

void WaterBlock::inspectPostApply()
{
    resetWorldBox();
    setMaskBits(1);
}

//==============================================================================

static EnumTable::Enums gLiquidTypeEnums[] =
{
   { WaterBlock::eWater,         "Water"         },
   { WaterBlock::eOceanWater,    "OceanWater"    },
   { WaterBlock::eRiverWater,    "RiverWater"    },
   { WaterBlock::eStagnantWater, "StagnantWater" },
   { WaterBlock::eLava,          "Lava"          },
   { WaterBlock::eHotLava,       "HotLava"       },
   { WaterBlock::eCrustyLava,    "CrustyLava"    },
   { WaterBlock::eQuicksand,     "Quicksand"     }
};
static EnumTable gLiquidTypeTable( 8, gLiquidTypeEnums );

//------------------------------------------------------------------------------

void WaterBlock::initPersistFields()
{
    Parent::initPersistFields();

    addField( "liquidType",         TypeEnum,    Offset( mLiquidType,       WaterBlock ), 1, &gLiquidTypeTable );
    addField( "density",            TypeF32,     Offset( mDensity,          WaterBlock ) );
    addField( "viscosity",          TypeF32,     Offset( mViscosity,        WaterBlock ) );
    addField( "waveMagnitude",      TypeF32,     Offset( mWaveMagnitude,    WaterBlock ) );
    addField( "surfaceTexture",     TypeString,  Offset( mSurfaceName,      WaterBlock ) );
    addField( "surfaceOpacity",     TypeF32,     Offset( mSurfaceOpacity,   WaterBlock ) );
    addField( "envMapTexture",      TypeString,  Offset( mEnvMapName,       WaterBlock ) );
    addField( "envMapIntensity",    TypeF32,     Offset( mEnvMapIntensity,  WaterBlock ) );
    addField( "submergeTexture",    TypeString,  Offset( mSubmergeName,     WaterBlock ), WC_NUM_SUBMERGE_TEX );
    addField( "removeWetEdges",     TypeBool,    Offset( mRemoveWetEdges,   WaterBlock ) );
    addField( "audioEnvironment",   TypeAudioEnvironmentPtr, Offset( mAudioEnvironment, WaterBlock ) );
}    
    
//==============================================================================

void WaterBlock::cToggleWireFrame( SimObject* obj, S32, const char** )
{
    WaterBlock* pWaterBlock = static_cast<WaterBlock*>(obj);
    if( pWaterBlock )
    {
        pWaterBlock->mFluid.m_ShowWire = !(pWaterBlock->mFluid.m_ShowWire);
        if( pWaterBlock->mFluid.m_ShowWire )
            Con::printf( "WaterBlock wire frame ENABLED" );
        else
            Con::printf( "WaterBlock wire frame DISABLED" );
    }
}

//==============================================================================

void WaterBlock::consoleInit()
{
   Con::addCommand( "WaterBlock", "toggleWireFrame", cToggleWireFrame,  "waterBlock.toggleWireFrame()", 2, 2 );
}

//==============================================================================

U32 WaterBlock::packUpdate( NetConnection* c, U32 mask, BitStream* stream )
{
    U32 retMask = Parent::packUpdate( c, mask, stream );

    // No masking in here now.  
    // There's not too much data, and it doesn't change during normal game play.

    stream->writeAffineTransform( mObjToWorld );
    mathWrite( *stream, mObjScale );

    stream->writeString( mSurfaceName  );
    stream->writeString( mEnvMapName   );

    for( int i=0; i<WC_NUM_SUBMERGE_TEX; i++ )
    {
       stream->writeString( mSubmergeName[i] );
    }
    
    stream->write( (S32)mLiquidType );
    stream->write( mDensity         );
    stream->write( mViscosity       );
    stream->write( mWaveMagnitude   );
    stream->write( mSurfaceOpacity  );
    stream->write( mEnvMapIntensity );
    stream->write( mRemoveWetEdges  );

    // audio environment:
    if(stream->writeFlag(mAudioEnvironment))
       stream->writeRangedU32(mAudioEnvironment->getId(), DataBlockObjectIdFirst, DataBlockObjectIdLast);

    return( retMask );
}

//==============================================================================

void WaterBlock::unpackUpdate( NetConnection* c, BitStream* stream )
{
    Parent::unpackUpdate( c, stream );

    U32 LiquidType;

    // No masking in here now.  
    // There's not too much data, and it doesn't change during normal game play.

    stream->readAffineTransform( &mObjToWorld );
    mathRead( *stream, &mObjScale );

    mSurfaceName  = stream->readSTString();
    mEnvMapName   = stream->readSTString();

    for( int i=0; i<WC_NUM_SUBMERGE_TEX; i++ )
    {
       mSubmergeName[i] = stream->readSTString();
    }

    
    stream->read( &LiquidType       );
    stream->read( &mDensity         );
    stream->read( &mViscosity       );
    stream->read( &mWaveMagnitude   );
    stream->read( &mSurfaceOpacity  );
    stream->read( &mEnvMapIntensity );
    stream->read( &mRemoveWetEdges  );

    // audio environment:
    if(stream->readFlag())
    {
       U32 profileId = stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);
       mAudioEnvironment = dynamic_cast<AudioEnvironment*>(Sim::findObject(profileId));
    }
    else
       mAudioEnvironment = 0;

    mLiquidType = (EWaterType)LiquidType;

    UpdateFluidRegion();
    
    if( !isProperlyAdded() )
        return;

    resetWorldBox();
}

//==============================================================================
// This method can take a point in world space, or water block space. The default
// assumes pos is in world space, and therefore must transform it to waterblock
// space.

bool WaterBlock::isPointSubmerged(const Point3F &pos, bool worldSpace) const
{
    return( isPointSubmergedSimple( pos, worldSpace ) );
}

//==============================================================================

bool WaterBlock::isPointSubmergedSimple(const Point3F &pos, bool worldSpace) const
{
    Point3F Pos = pos;

    if( Pos.z > mSurfaceZ )
        return( false );
                         
    if( worldSpace )
    {
        Pos.x += 1024.0f;
        Pos.y += 1024.0f;
    }

    return( mFluid.IsFluidAtXY( Pos.x, Pos.y ) );
}

//==============================================================================

bool WaterBlock::castRay( const Point3F& start, const Point3F& end, RayInfo* info )
{
    F32     t, x, y, X, Y;
    Point3F Pos;

    //
    // Looks like the incoming points are in parametric object space.  Great.
    //

    // The water surface is 1.0.  Bail if the ray does not cross the surface.

    if( (start.z > 1.0f) && (end.z > 1.0f) )        return( false );
    if( (start.z < 1.0f) && (end.z < 1.0f) )        return( false );

    // The ray crosses the surface plane.  Find out where.

    t = (start.z - 1.0f) / (start.z - end.z);
    x = start.x + (end.x - start.x) * t;
    y = start.y + (end.y - start.y) * t;

    Pos = mObjToWorld.getPosition();

    X = (x * mObjScale.x) + Pos.x + 1024.0f;
    Y = (y * mObjScale.y) + Pos.y + 1024.0f;

    if( mFluid.IsFluidAtXY( X, Y ) )
    {
        info->t = t;
        info->point.x  = x;
        info->point.y  = y;
        info->point.z  = 1.0f;
        info->normal.x = 0.0f;
        info->normal.y = 0.0f;
        info->normal.z = 1.0f;
        info->object   = this;
        info->material = 0;
        return( true );
    }

    // Hmm.  Guess we missed!
    return( false );
}

//==============================================================================

bool WaterBlock::isWater( U32 liquidType )
{
   EWaterType wType = EWaterType( liquidType );
   return( wType == eWater      || 
           wType == eOceanWater || 
           wType == eRiverWater || 
           wType == eStagnantWater );
}

//==============================================================================

bool WaterBlock::isLava( U32 liquidType )
{
   EWaterType wType = EWaterType( liquidType );
   return( wType == eLava    || 
           wType == eHotLava || 
           wType == eCrustyLava );
}

//==============================================================================

bool WaterBlock::isQuicksand( U32 liquidType )
{
   EWaterType wType = EWaterType( liquidType );
   return( wType == eQuicksand );
}

//==============================================================================
