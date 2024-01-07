//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _TERRRENDER_H_
#define _TERRRENDER_H_

#ifndef _GTEXMANAGER_H_
#include "dgl/gTexManager.h"
#endif
#ifndef _COLOR_H_
#include "core/color.h"
#endif
#ifndef _WATERBLOCK_H_
#include "terrain/waterBlock.h"
#endif

struct EmitChunk;

struct AllocatedTexture {
   U32 level;
   S32 x, y;
   F32 distance;
   EmitChunk *list;
   TextureHandle handle;
   AllocatedTexture *next;
   AllocatedTexture *previous;
   AllocatedTexture *nextLink;
   U32 mipLevel;

   AllocatedTexture()
   {
      next = previous = NULL;
   }
   inline void unlink()
   {
      AssertFatal(next && previous, "Invalid unlink.");
      next->previous = previous;
      previous->next = next;
      next = previous = NULL;
   }
   inline void linkAfter(AllocatedTexture *t)
   {
      AssertFatal(next == NULL && previous == NULL, "Cannot link a non-null next & prev");

      next = t->next;
      previous = t;
      t->next->previous = this;
      t->next = this;
   }
};

struct Render2Point : public Point3F
{
   F32 d;
//   ColorI color;
};

struct EdgePoint : public Point3F
{
   ColorI detailColor;
   F32 haze;
   F32 distance;
	F32 fogRed;
   F32 fogGreen;
};

struct ChunkCornerPoint : public EdgePoint
{
   U32 pointIndex;
   U32 xfIndex;
};

struct EdgeParent
{
   ChunkCornerPoint *p1, *p2;
};

struct ChunkScanEdge : public EdgeParent
{
   ChunkCornerPoint *mp;
   EdgeParent *e1, *e2;
};

struct ChunkEdge : public EdgeParent
{
   U32 xfIndex;
   U32 pointIndex;
   U32 pointCount;
   
   EdgePoint pt[3];
   EmitChunk *c1, *c2;
};

struct EmitChunk
{
   ChunkEdge *edge[4];
   S32 subDivLevel;
   F32 growFactor;
   S32 x, y;
   S32 gridX, gridY;
   U32 emptyFlags;
   bool clip;
   U32 lightMask;
   EmitChunk *next;
   bool renderDetails;
};

struct SquareStackNode2
{
   U32 clipFlags;
   U32 lightMask;
   Point2I pos;
   U32  level;
   bool texAllocated;
};

struct SquareStackNode
{
   U32 clipFlags;
   U32 lightMask;
   Point2I pos;
   U32  level;
   bool  texAllocated;
   EdgeParent *top, *right, *bottom, *left;
};

struct TerrLightInfo
{
   Point3F pos; // world position
   F32 radius; // radius of the light
   F32 radiusSquared; // radius^2
   F32 r, g, b;

   F32 distSquared; // distance to camera
};

//struct ScanEdge
//{
//   U16 p1, p2, mp;
//   U16 firstSubEdge;  // two sub edges for each edge, mp = InvalidPointIndex if none
//};

enum EmptyFlags {
   SquareEmpty_0_0 = (1 << 0),
   SquareEmpty_1_0 = (1 << 1),
   SquareEmpty_2_0 = (1 << 2),
   SquareEmpty_3_0 = (1 << 3),
   SquareEmpty_0_1 = (1 << 4),
   SquareEmpty_1_1 = (1 << 5),
   SquareEmpty_2_1 = (1 << 6),
   SquareEmpty_3_1 = (1 << 7),
   SquareEmpty_0_2 = (1 << 8),
   SquareEmpty_1_2 = (1 << 9),
   SquareEmpty_2_2 = (1 << 10),
   SquareEmpty_3_2 = (1 << 11),
   SquareEmpty_0_3 = (1 << 12),
   SquareEmpty_1_3 = (1 << 13),
   SquareEmpty_2_3 = (1 << 14),
   SquareEmpty_3_3 = (1 << 15),
   CornerEmpty_0_0 = SquareEmpty_0_0 | SquareEmpty_1_0 | SquareEmpty_0_1 | SquareEmpty_1_1,
   CornerEmpty_1_0 = SquareEmpty_2_0 | SquareEmpty_3_0 | SquareEmpty_2_1 | SquareEmpty_3_1,
   CornerEmpty_0_1 = SquareEmpty_0_2 | SquareEmpty_1_2 | SquareEmpty_0_3 | SquareEmpty_1_3,
   CornerEmpty_1_1 = SquareEmpty_2_2 | SquareEmpty_3_2 | SquareEmpty_2_3 | SquareEmpty_3_3,
};   

struct RenderPoint : public Point3F
{
   F32 dist;
   F32 haze; // also used as grow factor
};

enum {
   MaxClipPlanes = 8, // left, right, top, bottom - don't need far tho...
   MaxTerrainMaterials = 256,

   EdgeStackSize = 1024, // value for water/terrain edge stack size.
   MaxWaves = 8,
   MaxDetailLevel = 9,
   MaxMipLevel = 8,
   MaxTerrainLights = 64,
   MaxVisibleLights = 31,
   ClipPlaneMask = (1 << MaxClipPlanes) - 1, 
   FarSphereMask = 0x80000000,
   FogPlaneBoxMask = 0x40000000,
   VertexBufferSize = 65 * 65 + 1000,
   AllocatedTextureCount = 16 + 64 + 256 + 1024 + 4096,
   
   SmallMipLevel = 6
};

struct Color
{
   S32 r, g, b;
   F32 z;
};

class SceneState;

struct TerrainRender
{
   static MatrixF mCameraToObject;
   static AllocatedTexture mTextureFrameListHead;
   static AllocatedTexture mTextureFrameListTail;
   static AllocatedTexture mTextureFreeListHead;
   static AllocatedTexture mTextureFreeListTail;
   static AllocatedTexture mTextureFreeBigListHead;
   static AllocatedTexture mTextureFreeBigListTail;
   static U32 mTextureSlopSize;
   static Vector<TextureHandle> mTextureFreeList;
   static S32 mTextureMinSquareSize;
   
   static SceneState *mSceneState;
   static AllocatedTexture *mCurrentTexture;
   
   static TerrainBlock *mCurrentBlock;
   static S32 mSquareSize;
   static F32 mScreenSize;
   static U32 mFrameIndex;
   static U32 mNumClipPlanes;
   static AllocatedTexture *mTextureGrid[AllocatedTextureCount];
   static AllocatedTexture **mTextureGridPtr[5];
   
   static Point2F mBlockPos;
   static Point2I mBlockOffset;
   static Point2I mTerrainOffset;
   
   static PlaneF mClipPlane[MaxClipPlanes];
   static Point3F mCamPos;
   static TextureHandle* mGrainyTexture;
   static U32 mDynamicLightCount;
   static bool mEnableTerrainDetails;
   static bool mEnableTerrainDynLights;

   static F32 mPixelError;

   static TerrLightInfo mTerrainLights[MaxTerrainLights];
   static F32 mScreenError;
   static F32 mMinSquareSize;
   static F32 mFarDistance;
   static S32 mDynamicTextureCount;
   static S32 mTextureSpaceUsed;
   static S32 mLevelZeroCount;
   static S32 mFullMipCount;
   static S32 mStaticTextureCount;
   static S32 mUnusedTextureCount;
   static S32 mStaticTSU;
   static S32 mSquareSeqAdd[256];
   static U32 mNewGenTextureCount;
   static F32 mInvFarDistance;
   static F32 mInvHeightRange;
   static U32 mMipCap;
   static bool mRenderingCommander;

   static ColorF mFogColor;

   static bool mRenderOutline;
   static U32  mMaterialCount;

   static GBitmap* mBlendBitmap;

   static void (*transformPoint)(U32 point, U32 p1, U32 p2);

   static void init();
   static void shutdown();

//   static void allocReset();
//   static void* alloc(U32 byteSize);
   static void allocRenderEdges(U32 edgeCount, EdgeParent **dest, bool renderEdge);
   static void subdivideChunkEdge(ChunkScanEdge *e, Point2I pos, bool chunkEdge);
   static void processCurrentBlock2(SceneState* state, EdgeParent *topEdge, EdgeParent *rightEdge, EdgeParent *bottomEdge, EdgeParent *leftEdge);
   static ChunkCornerPoint *allocInitialPoint(Point3F pos);
   static ChunkCornerPoint *allocPoint(Point2I pos);
   static void emitTerrChunk(SquareStackNode *n, F32 squareDistance, U32 lightMask, bool farClip, bool useDetails);
   static void renderChunkOutline(EmitChunk *chunk);
   static void renderChunkCommander(EmitChunk *chunk);
   static void fixEdge(ChunkEdge *edge, S32 x, S32 y, S32 dx, S32 dy);
   static void drawTriFan(U32 vCount, U32 *indexBuffer);
   static U32 constructPoint(S32 x, S32 y);
   static U32 interpPoint(U32 p1, U32 p2, S32 x, S32 y, F32 growFactor);
   static void addEdge(ChunkEdge *edge);
   static void clipStart(EdgePoint *pt, U32 index);
   static void clipInsert(EdgePoint *pt, U32 index);
   static void clipEnd();
   static void clip(U32 triFanStart);

   static F32 getScreenError()            { return(mScreenError); }
   static void setScreenError(F32 error)  { mScreenError = error; }

   static void flushCache();
   static void transformTerrainPoint(U32 point, U32 p1, U32 p2);
   
   static U8 getClipFlags(Point2F &pos, U8 clipMask);

   static void allocTerrTexture(Point2I pos, U32 level, U32 mipLevel, bool vis, F32 distance);
   static void freeTerrTexture(AllocatedTexture *texture);
   static void buildBlendMap(AllocatedTexture *texture);

   static U32 TestSquareLights(GridSquare *sq, S32 level, Point2I pos, U32 lightMask);
   static S32 TestSquareVisibility(Point3F &min, Point3F &max, S32 clipMask, F32 expand);

   static S32  allocEdges(S32 count);
   static void clampEdge(S32 edge);
   static void subdivideEdge(S32 edge, Point2I pos);
   static F32 getSquareDistance(const Point3F& minPoint, const Point3F& maxPoint,
                                F32* zDiff);
   static bool subdivideSquare(GridSquare *sq, S32 level, Point2I pos);

   static void emitTerrSquare(SquareStackNode *n, U32 flags);
   static void emitFullMipSquare(SquareStackNode *n, U32 flags);
   static void emitNonzeroSquare(SquareStackNode *n, U32 flags, U32 mipLevel);
   static void emitLevelZeroSquare(SquareStackNode *n, U32 flags, U32 material, U32 mipLevel);
   
   static void buildTextureCoor();
   static void renderCurrentBlock(S32 firstSquare, S32 lastSquare);

   static void textureRecurse(SquareStackNode *n);
   static void processCurrentBlock(S32 topEdge, S32 rightEdge, S32 bottomEdge, S32 leftEdge);
   static void buildLightArray();
   static void buildClippingPlanes(bool flipClipPlanes);
   static void buildDetailTable();
   
   static void renderXFCache();
   static void renderBlock(TerrainBlock *, SceneState *state);

   static void textureRecurse2(SquareStackNode2 *n);
   static void tesselate0(SquareStackNode2 *n, U32 flags);
   static void tesselate1(SquareStackNode2 *n);
   static void tesselate2(SquareStackNode2 *n);
   static void tesselate3(SquareStackNode2 *n);
   static void farclip(SquareStackNode2 *n, U32 flags);
   static void render2();
   static void processBlockStack(SquareStackNode2 *stack, S32 curStackSize);

protected:
   static void doesWaterBlockSubmergeCamera(SceneObject *sceneObj, S32 key);
};

#endif
