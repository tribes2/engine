//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _BLENDER_H_
#define _BLENDER_H_

#define PAULS_TEST_CODE      0


#if PAULS_TEST_CODE

typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;
typedef float F32;

#define GRIDFLAGS( x, y )   (grid_mats[ yp ][ xp ])
#define MATERIALSTART       1
#define BLENDER_USE_ASM     1

#else

#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif
#ifndef _TERRDATA_H_
#include "terrain/terrData.h"
#endif
#ifndef _TERRRENDER_H_
#include "terrain/terrRender.h"
#endif

#ifdef USEASSEMBLYTERRBLEND
#define BLENDER_USE_ASM             1
#else
#define BLENDER_USE_ASM             0
#endif

#define GRIDFLAGS( x, y )   (TerrainRender::mCurrentBlock->findSquare( 0, Point2I( xp, yp ) )->flags)
#define MATERIALSTART       (GridSquare::MaterialStart)

#endif



class Blender
{
    // pointer to big buffer of source textures and mipmaps
    U32 *bmp_alloc_ptr;

    // One square buffer used for blending...
    U32 *blendbuffer;

    // List of pointers into bmp buffer.  Grouped by mip level, 
    //  so first X pointers are textures 0-X, mip 0.
    U32 **bmpdata;

    // List of pointers to alpha data for the bmp types.
    U8 **alpha_data;

    // Number of bmp types
    int num_src_bmps;

    // Mip levels (including top detail) for each bmp type
    int num_mip_levels;

public:
    // mips_per_bmp should include top level (always >= 1)
    // alphadata is 8 bit 256x256
    Blender( int num_bmp_types, int mips_per_bmp, U8 **alphadata );
    ~Blender();

    // blends into 5551 format.  X and Y are in blocks (same resolution as
    //  alpha table, i.e. at high detail, there are 4x4 blocks covered by
    //  the 128x128 destination bmp.
    // lightmap should be 16 bit 512x512.
    // destmips is an array of pointers to the bitmap and it's mips to be
    //  filled in by this function
    void blend( int x, int y, int level, const U16 *lightmap, U16 **destmips );

    // bmps is an array of pointers to the bitmap and it's mips
    //  highest detail first.  Should be in 24bit format.
    // Call this once per bmp type.  It copies the bmp into it's own format,
    //  so you can then delete your versions of the bmp and mips.
    void addSourceTexture( int bmp_type, const U8 **bmps );
};

#endif
