//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _FLUID_H_
#define _FLUID_H_

//==============================================================================
//  TO DO LIST
//==============================================================================
//  - ARB support?
//  - Optimize the fog given water is horizontal
//  - New fog system
//  - Attempt to reject fully fogger low LOD blocks
//==============================================================================
//  - Designate specular map?
//  - Turn off specular?  (for lava)
//==============================================================================

//==============================================================================
//  ASSUMPTIONS:
//   - A single repetition of the terrain (or a "rep") is 256x256 squares.
//   - A terrain square is 8 meters on a side.
//   - The fluid can NOT be rotated on any axis.
//   - The "anchor point" for the fluid can be on any discrete square corner.
//   - The size of the fluid is constrained to be either a multiple of 4 or 8
//     squares.
//==============================================================================
//  DISCUSSION:
//   - If the overall size of the fluid is less than a quarter of a terrain rep,
//     then the fluid will use a "double resolution" mode.  Thus, when the fluid
//     is sufficiently small, it gets more detailed. 
//   - The fluid renders blocks in one of two sizes, LOW and HIGH.
//   - Block are 8 squares in normal resolution mode, and 4 in high.
//   - A quad tree is used to quickly cull down to the blocks.
//   - Reject and accept masks are used by the quad tree.
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef _TYPES_H_
#include "Platform/types.h"
#endif
#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif
#ifndef _GTEXMANAGER_H_
#include "dgl/gTexManager.h"
#endif
#ifndef _MMATH_H_
#include "Math/mMath.h"
#endif

//==============================================================================
//  DEFINES
//==============================================================================

#define s32     S32
#define u32     U32
#define s16     S16
#define u16     U16
#define s8      S8 
#define u8      U8 
#define byte    U8
#define f32     F32

#define PI	(3.141592653589793238462643f)

#define SECONDS         ((f32)(Platform::getVirtualMilliseconds()) * 0.001f)
#define MALLOC          dMalloc
#define REALLOC         dRealloc
#define FREE            dFree
#define MEMSET          dMemset
#define TWO_PI          (PI * 2.0f)
#define FMOD(a,b)       (f32)fmod( a, b )
#define LENGTH3(a,b,c)  (f32)sqrt( (a)*(a) + (b)*(b) + (c)*(c) )
#define SINE(a)         (float)sin( a )
#define COSINE(a)       (float)cos( a )

#define DISTANCE(x,y,z) LENGTH3( (x)-m_Eye.X, (y)-m_Eye.Y, (z)-m_Eye.Z )

#define ASSERT(exp)     

//==============================================================================
//  TYPES
//==============================================================================
//
//  Order for [4] element arrays...
//
//     2----3      Y
//     |    |      |
//     |    |      |
//     0----1      0----X
//
//==============================================================================

class fluid
{
//------------------------------------------------------------------------------
//  Public Types
//
public:
    typedef f32 compute_fog_fn( f32 DeltaZ, f32 D );

//------------------------------------------------------------------------------
//  Public Functions
//
public:
            fluid               ( void );
           ~fluid               ();

    //
    // Render (in FluidRender.cpp):
    //    
    void    Render              ( bool& EyeSubmerged );

    //
    // Setup per frame (in FluidSupport.cpp):
    //
    void    SetEyePosition      ( f32 X, f32 Y, f32 Z );
    void    SetFrustrumPlanes   ( f32* pFrustrumPlanes );

    //
    // Setup at initialization (in FluidSupport.cpp):
    //
    void    SetInfo             ( f32& X0, 
                                  f32& Y0,
                                  f32& SizeX,
                                  f32& SizeY,
                                  f32  SurfaceZ,
                                  f32  WaveAmplitude,
                                  f32& Opacity,
                                  f32& EnvMapIntensity,
                                  s32  RemoveWetEdges );

    void    SetTerrainData      ( u16* pTerrainData );

    void    SetTextures         ( TextureHandle Base,
                                  TextureHandle EnvMap );

    void    SetLightMapTexture  ( TextureHandle LightMapTexture );

    void    SetFogParameters    ( f32 R, f32 G, f32 B, f32 VisibleDistance );
    void    SetFogFn            ( compute_fog_fn* pFogFn );

    //
    // Run time interrogation (in FluidSupport.cpp):
    //
    s32     IsFluidAtXY         ( f32 X, f32 Y )    const;                                  

//------------------------------------------------------------------------------
//  Private Types
//
private:
    struct plane { f32 A, B, C, D; };
    struct rgba  { f32 R, G, B, A; };
    struct xyz   { f32 X, Y, Z;    };
    struct uv    { f32 U, V;       };

    struct node
    {
        s32     Level;
        s32     MaskIndexX, MaskIndexY;
        s32     BlockX0, BlockY0;       // World Block
        f32     X0,      Y0;            // World Position
        f32     X1,      Y1;            // World Position
        byte    ClipBits[4];
    };

    struct block
    {
        f32     X0, Y0;
        f32     X1, Y1;
        f32     Distance[4];    // Distance from eye
        f32     LOD     [4];    // Level of detail
    };

    // Rendering phases:
    //  Phase 1 - Base texture (two passes of cross faded textures)
    //  Phase 2 - Shadow map
    //  Phase 3 - Environment map / Specular
    //  Phase 4 - Fog

    struct vertex
    {
        xyz     XYZ;        // All phases - Position
        rgba    RGBA1a;     // Phase 1a   - Base alpha, first pass
        rgba    RGBA1b;     // Phase 1b   - Base alpha, second pass
        uv      UV3;        // Phase 3    - EnvMap UV
        rgba    RGBA4;      // Phase 4    - Fog Color/Alpha
    };

//------------------------------------------------------------------------------
//  Private Variables
//
private:           
    s32     m_SquareX0,   m_SquareY0;       // Anchor in terrain squares
    s32     m_SquaresInX, m_SquaresInY;     // Number of squares in fluid region
    s32     m_BlocksInX,  m_BlocksInY;      // Number of blocks in fluid region
    f32     m_SurfaceZ;                     // Altitude of fluid surface
    s32     m_RemoveWetEdges;               // Dry fill all edges of the fluid block
    s32     m_HighResMode;                  // Blocks are 4x4 (high res) or 8x8 squares (normal)
    plane   m_Plane[6];                     // Frustrum clip planes: 0=T 1=B 2=L 3=R 4=N 5=F
    xyz     m_Eye;
    f32     m_Seconds;
    f32     m_BaseSeconds;
    rgba    m_FogColor;
    f32     m_VisibleDistance;
    f32     m_Opacity;
    f32     m_EnvMapIntensity;
    f32     m_WaveAmplitude;
    f32     m_WaveFactor;
    u16*    m_pTerrain;         // 256x256 data for the terrain

    TextureHandle     m_BaseTexture;
    TextureHandle     m_EnvMapTexture;
    TextureHandle     m_LightMapTexture;

    f32     m_Step[5];          // [0] =  0
                                // [1] = 1/4 block step
                                // [2] = 1/2 block step
                                // [3] = 3/4 block step
                                // [4] =  1  block step

    compute_fog_fn* m_pFogFn;

    // Bit masks to trivially accept or reject the progressive levels for the
    // quad tree recursion.  No need for an accept mask at the highest level
    // since it is just the exact opposite of the reject mask at that level.

    static  s32     m_MaskOffset[6];  // Offset for given level into masks.

    byte            m_RejectMask[ 1 + 1 + 2 + 8 + 32 + 128 ];
    byte            m_AcceptMask[ 1 + 1 + 2 + 8 + 32 ];

    //
    // Shared among instances of fluid.
    //

    static  vertex* m_pVertex;
    static  s32     m_VAllocated;
    static  s32     m_VUsed;

    static  s16*    m_pIndex;
    static  s16*    m_pINext;
    static  s16     m_IOffset;
    static  s32     m_IAllocated;
    static  s32     m_IUsed;
    
    static  s32     m_Instances;

    //
    // Debug variables.
    //
public:
    s32     m_ShowWire;
    s32     m_ShowNodes;
    s32     m_ShowBlocks;
    s32     m_ShowBaseA;
    s32     m_ShowBaseB;
    s32     m_ShowLightMap;
    s32     m_ShowEnvMap;
    s32     m_ShowFog;

//------------------------------------------------------------------------------
//  Private Functions
//
private:

    //
    // Functions in FluidSupport.cpp:
    //
    s32     GetAcceptBit            ( s32 Level, s32 IndexX, s32 IndexY ) const;
    s32     GetRejectBit            ( s32 Level, s32 IndexX, s32 IndexY ) const;
    void    SetAcceptBit            ( s32 Level, s32 IndexX, s32 IndexY, s32 Value );
    void    SetRejectBit            ( s32 Level, s32 IndexX, s32 IndexY, s32 Value );
    void    BuildLowerMasks         ( void );
    void    RebuildMasks            ( void );
    void    FloodFill               ( u8* pGrid, s32 x, s32 y, s32 SizeX, s32 SizeY );
    
    //
    // Functions in FluidQuadTree.cpp
    //
    void    RunQuadTree             ( bool& EyeSubmerged );

    f32     ComputeLOD              ( f32 Distance );
    byte    ComputeClipBits         ( f32 X, f32 Y, f32 Z );

    void    ProcessNode             ( node&  Node  );
    void    ProcessBlock            ( block& Block );

    void    ProcessBlockLODHigh     ( block& Block );
    void    ProcessBlockLODMorph    ( block& Block );
    void    ProcessBlockLODTrans    ( block& Block );
    void    ProcessBlockLODLow      ( block& Block );

    void    SetupVert               ( f32 X, f32 Y, f32 Distance, vertex* pV );

    void    InterpolateVerts        ( vertex* pV0, 
                                      vertex* pV1, 
                                      vertex* pV2, 
                                      vertex* pV3, 
                                      vertex* pV4,
                                      f32     LOD0,
                                      f32     LOD4 );

    void    InterpolateVert         ( vertex* pV0, 
                                      vertex* pV1, 
                                      vertex* pV2, 
                                      f32     LOD );

//  void    BuildFogTable           ( void );

    void    ReleaseVertexMemory     ( void );
    vertex* AcquireVertices         ( s32 Count );
    void    AddTriangleIndices      ( s16 I1, s16 I2, s16 I3 );
};

//==============================================================================
#endif // FLUID_HPP
//==============================================================================
