//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "terrain/terrData.h"				   
#include "Math/mMath.h"
#include "dgl/dgl.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "dgl/gBitmap.h"
#include "terrain/terrRender.h"
#include "dgl/materialList.h"
#include "sceneGraph/sceneState.h"
#include "terrain/waterBlock.h"
#include "terrain/blender.h"
#include "Sim/frameAllocator.h"
#include "sceneGraph/sceneGraph.h"
#include "sceneGraph/sgUtil.h"

Render2Point gEmitPoints2[65536];
U16 gIndexArray2[131000];
Render2Point *mEmitPoints2;
U16 *mIndexArray2;
U32 mCurrPoint2;
U32 mPrimQ[16384];

struct PrimitiveList
{
   enum {
      MaxCount = 256,
   };
   U32 count;
   U16 *primList[MaxCount];
   PrimitiveList *next;
   static void alloc(PrimitiveList **ph)
   {
      PrimitiveList *next = *ph;
      *ph = (PrimitiveList *) FrameAllocator::alloc(sizeof(PrimitiveList));
      (*ph)->count = 0;
      (*ph)->next = next;
   }
};

PrimitiveList *blendList[TerrainBlock::MaterialGroups];
PrimitiveList *baseList[TerrainBlock::MaterialGroups];


inline void addPrimitive(U16 *start, PrimitiveList **lp)
{
   if(!(*lp) || ( (*lp)->count == PrimitiveList::MaxCount) )
      PrimitiveList::alloc(lp);
   (*lp)->primList[(*lp)->count++] = start;
}
#if 0
void addPrimitives(U32 flags, U16 *start)
{
   if(flags & GridSquare::Material0)
   {
      addPrimitive(start, &baseList[0]);
      goto testBlend1;
   }
   if(flags & GridSquare::Material1)
   {
      addPrimitive(start, &baseList[1]);
      goto testBlend2;
   }
   if(flags & GridSquare::Material2)
   {
      addPrimitive(start, &baseList[2]);
      goto testBlend3;
   }
   if(flags & GridSquare::Material3)
      addPrimitive(start, &baseList[3]);
   return;
testBlend1:
   if(flags & GridSquare::Material1)
      addPrimitive(start, &blendList[1]);
testBlend2:
   if(flags & GridSquare::Material2)
      addPrimitive(start, &blendList[2]);
testBlend3:
   if(flags & GridSquare::Material3)
      addPrimitive(start, &blendList[3]);
}
#else
#define addPrimitives(x,y)
#endif

void TerrainRender::tesselate3(SquareStackNode2 *n)
{
   // generate 9x9 array of points, and emit the 16 fans for the block
   U32 i = 0, x, y;
   Render2Point * ep = mEmitPoints2 + mCurrPoint2;
   F32 xp, yp, step = mSquareSize;

   yp = n->pos.y * mSquareSize + mBlockPos.y;
   F32 xs = n->pos.x * mSquareSize + mBlockPos.x;

   U16 *flp = mCurrentBlock->getFlagMapPtr(n->pos.x, n->pos.y);

   for(y = 0; y < 9; y++)
   {
      xp = xs;
      U16 *hp = mCurrentBlock->getHeightAddress(n->pos.x, n->pos.y + y);
      for(x = 0; x < 8; x++)
      {
         ep->x = xp;
         ep->y = yp;
         ep->z = fixedToFloat(*hp++);
         ep++;
         xp += step;
      }
      ep->x = xp;
      ep->y = yp;
      ep->z = fixedToFloat(mCurrentBlock->getHeight(n->pos.x + x, n->pos.y + y));
      ep++;
      if((~y) & 1)
      {
         ep[-9].d = (ep[-9] - mCamPos).len();
         ep[-7].d = (ep[-7] - mCamPos).len();
         ep[-5].d = (ep[-5] - mCamPos).len();
         ep[-3].d = (ep[-3] - mCamPos).len();
         ep[-1].d = (ep[-1] - mCamPos).len();
         ep[-8].d = (ep[-9].d + ep[-7].d) * 0.5;
         ep[-6].d = (ep[-7].d + ep[-5].d) * 0.5;
         ep[-4].d = (ep[-5].d + ep[-3].d) * 0.5;
         ep[-2].d = (ep[-3].d + ep[-1].d) * 0.5;
         if(y > 1)
         {
            Render2Point *r1 = ep - 27;
            Render2Point *r2 = ep - 18;
            Render2Point *r3 = ep - 9;
            r2[0].d = (r1[0].d + r3[0].d) * 0.5;
            r2[2].d = (r1[2].d + r3[2].d) * 0.5;
            r2[4].d = (r1[4].d + r3[4].d) * 0.5;
            r2[6].d = (r1[6].d + r3[6].d) * 0.5;
            r2[8].d = (r1[8].d + r3[8].d) * 0.5;

            r2[1].d = (r2[0].d + r2[2].d) * 0.5;
            r2[3].d = (r2[2].d + r2[4].d) * 0.5;
            r2[5].d = (r2[4].d + r2[6].d) * 0.5;
            r2[7].d = (r2[6].d + r2[8].d) * 0.5;
         }
      }
      yp += step;
   }
   for(y = 0; y < 4; y++)
   {
      for(x = 0; x < 4; x++)
      {
         addPrimitives(*flp, mIndexArray2);
         flp++;

         mIndexArray2[0] = GL_TRIANGLE_FAN;
         mIndexArray2[1] = 10;
         mIndexArray2[2] = mCurrPoint2 + 10;
         mIndexArray2[3] = mCurrPoint2 + 18;
         mIndexArray2[4] = mCurrPoint2 + 19;
         mIndexArray2[5] = mCurrPoint2 + 20;
         mIndexArray2[6] = mCurrPoint2 + 11;
         mIndexArray2[7] = mCurrPoint2 + 2;
         mIndexArray2[8] = mCurrPoint2 + 1;
         mIndexArray2[9] = mCurrPoint2;
         mIndexArray2[10] = mCurrPoint2 + 9;
         mIndexArray2[11] = mCurrPoint2 + 18;
         mIndexArray2 += 12;
         mCurrPoint2 += 2;
      }
      flp += TerrainBlock::FlagMapWidth - 4;
      mCurrPoint2 += 10;
   }
   mCurrPoint2 += 9;
}

void TerrainRender::tesselate2(SquareStackNode2 *n)
{
   U32 i = 0, x, y;
   Render2Point * ep = mEmitPoints2 + mCurrPoint2;
   F32 xp, yp, step = mSquareSize;
   F32 xs = n->pos.x * mSquareSize + mBlockPos.x;

   yp = n->pos.y * mSquareSize + mBlockPos.y;
   U16 *flp = mCurrentBlock->getFlagMapPtr(n->pos.x, n->pos.y);

   for(y = 0; y < 5; y++)
   {
      xp = xs;
      for(x = 0; x < 5; x++)
      {
         ep->x = xp;
         ep->y = yp;
         ep->z = fixedToFloat(mCurrentBlock->getHeight(n->pos.x + x, n->pos.y + y));
         ep->d = (*ep - mCamPos).len();
         ep++;
         xp += step;
      }
      yp += step;
   }
   for(y = 0; y < 2; y++)
   {
      for(x = 0; x < 2; x++)
      {
         addPrimitives(*flp, mIndexArray2);
         flp++;

         mIndexArray2[0] = GL_TRIANGLE_FAN;
         mIndexArray2[1] = 10;
         mIndexArray2[2] = mCurrPoint2 + 6;
         mIndexArray2[3] = mCurrPoint2 + 10;
         mIndexArray2[4] = mCurrPoint2 + 11;
         mIndexArray2[5] = mCurrPoint2 + 12;
         mIndexArray2[6] = mCurrPoint2 + 7;
         mIndexArray2[7] = mCurrPoint2 + 2;
         mIndexArray2[8] = mCurrPoint2 + 1;
         mIndexArray2[9] = mCurrPoint2;
         mIndexArray2[10] = mCurrPoint2 + 5;
         mIndexArray2[11] = mCurrPoint2 + 10;
         mIndexArray2 += 12;
         mCurrPoint2 += 2;
      }
      mCurrPoint2 += 6;
      flp += TerrainBlock::FlagMapWidth - 2;
   }
   mCurrPoint2 += 5;
}

void TerrainRender::tesselate1(SquareStackNode2 *n)
{
   U32 i = 0, x, y;
   Render2Point * ep = mEmitPoints2 + mCurrPoint2;
   F32 xp, yp, step = mSquareSize;
   F32 xs = n->pos.x * mSquareSize + mBlockPos.x;

   yp = n->pos.y * mSquareSize + mBlockPos.y;

   for(y = 0; y < 3; y++)
   {
      xp = xs;
      for(x = 0; x < 3; x++)
      {
         ep->x = xp;
         ep->y = yp;
         ep->z = fixedToFloat(mCurrentBlock->getHeight(n->pos.x + x, n->pos.y + y));
         ep->d = (*ep - mCamPos).len();
         ep++;
         xp += step;
      }
      yp += step;
   }
   addPrimitives( *(mCurrentBlock->getFlagMapPtr(n->pos.x, n->pos.y)), mIndexArray2);
   mIndexArray2[0] = GL_TRIANGLE_FAN;
   mIndexArray2[1] = 10;
   mIndexArray2[2] = mCurrPoint2 + 4;
   mIndexArray2[3] = mCurrPoint2 + 6;
   mIndexArray2[4] = mCurrPoint2 + 7;
   mIndexArray2[5] = mCurrPoint2 + 8;
   mIndexArray2[6] = mCurrPoint2 + 5;
   mIndexArray2[7] = mCurrPoint2 + 2;
   mIndexArray2[8] = mCurrPoint2 + 1;
   mIndexArray2[9] = mCurrPoint2;
   mIndexArray2[10] = mCurrPoint2 + 3;
   mIndexArray2[11] = mCurrPoint2 + 6;
   mIndexArray2 += 12;
   mCurrPoint2 += 9;
}

void TerrainRender::tesselate0(SquareStackNode2 *n, U32 flags)
{
}

void TerrainRender::farclip(SquareStackNode2 *n, U32 flags)
{
}

void TerrainRender::render2()
{
   glFrontFace(GL_CW);
   glEnable(GL_CULL_FACE);
   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(3, GL_FLOAT, sizeof(Render2Point), mEmitPoints2);
   TextureHandle hazeTexture = gClientSceneGraph->getFogTexture();

   F32 invLevel = 1 / F32(mSquareSize << 2);
   Point4F texGenS = Point4F(invLevel, 0, 0, 0);
   Point4F texGenT = Point4F(0, invLevel, 0, 0);
   glColor4f(1,1,1,1);

   glClientActiveTextureARB(GL_TEXTURE0_ARB);
   glActiveTextureARB(GL_TEXTURE0_ARB);
   glEnable(GL_TEXTURE_2D);
   glEnable(GL_TEXTURE_GEN_S);
   glEnable(GL_TEXTURE_GEN_T);
   glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
   glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
   glTexGenfv(GL_S, GL_OBJECT_PLANE, texGenS);
   glTexGenfv(GL_T, GL_OBJECT_PLANE, texGenT);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

   invLevel = 1 / F32(mSquareSize << 8);
   texGenS = Point4F(invLevel, 0, 0, 0);
   texGenT = Point4F(0, invLevel, 0, 0);
   
   glClientActiveTextureARB(GL_TEXTURE1_ARB);
   glActiveTextureARB(GL_TEXTURE1_ARB);
   glEnable(GL_TEXTURE_2D);
   glEnable(GL_TEXTURE_GEN_S);
   glEnable(GL_TEXTURE_GEN_T);
   glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
   glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
   glTexGenfv(GL_S, GL_OBJECT_PLANE, texGenS);
   glTexGenfv(GL_T, GL_OBJECT_PLANE, texGenT);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

   glBlendFunc(GL_ONE, GL_ONE);

   glLockArraysEXT(0, mCurrPoint2);

   //glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   //glTexCoordPointer(2, GL_FLOAT, sizeof(Render2Point), &mEmitPoints2->z);
   //glMatrixMode(GL_TEXTURE_MATRIX);

   for(S32 i = 0; i < TerrainBlock::MaterialGroups; i++)
   {
      TextureHandle t = mCurrentBlock->mBaseMaterials[i];
      TextureHandle a = mCurrentBlock->mAlphaMaterials[i];

      if(!bool(t))
         break;

      glActiveTextureARB(GL_TEXTURE0_ARB);
      glBindTexture(GL_TEXTURE_2D, t.getGLName());
      glActiveTextureARB(GL_TEXTURE1_ARB);
      glBindTexture(GL_TEXTURE_2D, a.getGLName());

      glDisable(GL_BLEND);
      //glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
      for(PrimitiveList *walk = baseList[i]; walk; walk = walk->next)
         for(S32 j = 0; j < walk->count; j++)
		      glDrawElements(walk->primList[j][0], walk->primList[j][1], GL_UNSIGNED_SHORT, walk->primList[j] + 2);
      
      glEnable(GL_BLEND);
      for(PrimitiveList *walk = blendList[i]; walk; walk = walk->next)
         for(S32 j = 0; j < walk->count; j++)
		      glDrawElements(walk->primList[j][0], walk->primList[j][1], GL_UNSIGNED_SHORT, walk->primList[j] + 2);
   }
   glUnlockArraysEXT();

   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   glClientActiveTextureARB(GL_TEXTURE1_ARB);
   glActiveTextureARB(GL_TEXTURE1_ARB);
   glDisable(GL_TEXTURE_GEN_S);
   glDisable(GL_TEXTURE_GEN_T);
   glDisable(GL_TEXTURE_2D);

   // last pass is the haze pass
   glClientActiveTextureARB(GL_TEXTURE0_ARB);
   glActiveTextureARB(GL_TEXTURE0_ARB);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
   glDisable(GL_TEXTURE_GEN_S);
   glDisable(GL_TEXTURE_GEN_T);
   glBindTexture(GL_TEXTURE_2D, hazeTexture.getGLName());


   static F32 texMatrix[16] = {
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1,
   };
   // get the height range from the root of the block map
   GridSquare *sq = mCurrentBlock->findSquare(TerrainBlock::BlockShift, Point2I(0,0));

   F32 heightOffset = fixedToFloat(sq->minHeight);
   F32 heightRange = fixedToFloat(sq->maxHeight) - heightOffset;
   texMatrix[4] = - 1 / mFarDistance;
   texMatrix[12] = 1;
   texMatrix[1] = 1 / heightRange;
   texMatrix[13] = -heightOffset / heightRange;
   glMatrixMode(GL_TEXTURE);
   glLoadMatrixf(texMatrix);

   glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   glTexCoordPointer(2, GL_FLOAT, sizeof(Render2Point), &(mEmitPoints2[0].z));
   glLockArraysEXT(0, mCurrPoint2);

   U16 *ar = gIndexArray2;
   while(ar != mIndexArray2)
   {
      glDrawElements(ar[0], ar[1], GL_UNSIGNED_SHORT, ar + 2);
      ar += ar[1] + 2;
   }
   glUnlockArraysEXT();

   glLoadIdentity();

   glDisable(GL_BLEND);
   //glDisableClientState(GL_TEXTURE_COORD_ARRAY);

   glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   glDisableClientState(GL_VERTEX_ARRAY);
   glDisable(GL_TEXTURE_2D);

   glDisable(GL_CULL_FACE);
}

static S32 getPower(S32 x)
{
	// Returns 2^n (the highest bit).
	S32 i = 0;
	if (x)
		do
			i++;
		while (x >>= 1);
	return i;
}

void TerrainRender::textureRecurse2(SquareStackNode2 *stack)
{
   S32 curStackSize = 1;
   Point3F minPoint, maxPoint;
   F32 squareDistance;

   while(curStackSize)
   {
      SquareStackNode2 *n = stack + curStackSize - 1;
      // see if it's visible
      Point2I pos = n->pos;
      S32 squareSz = mSquareSize << n->level;
      GridSquare *sq = mCurrentBlock->findSquare(n->level, pos);
         
      minPoint.set(mSquareSize * pos.x + mBlockPos.x,
                   mSquareSize * pos.y + mBlockPos.y,
                   fixedToFloat(sq->minHeight));
      maxPoint.set(minPoint.x + (mSquareSize << n->level),
                   minPoint.y + (mSquareSize << n->level),
                   fixedToFloat(sq->maxHeight));

      F32 zDiff;
      squareDistance = getSquareDistance(minPoint, maxPoint, &zDiff);

	  // holes only in the primary terrain block
      if (squareDistance >= mFarDistance ||
      	  ((sq->flags & GridSquare::Empty) && mBlockPos.x == 0 && mBlockPos.y == 0))
      {
         curStackSize--;
         continue;
      }
      // first check the level - if its 3 or less, we have to just make a bitmap:
      // level 3 == 8x8 square - 8x8 * 16x16 == 128x128
      S32 mipLevel = 7;
      if(n->level > 6)
         goto norecalloc;
      if(n->level != mTextureMinSquareSize)
      {
         // get the mip level of the square and see if we're in range
         if(squareDistance > 0.001)
         {
            S32 size = S32(dglProjectRadius(squareDistance + (squareSz >> 1), squareSz));
            mipLevel = getPower(size * 0.75);
            //if(n->level >= 6)
            //{
            //   if(mipLevel - n->level > 2)
            //      goto norecalloc;
            //}
            //else
            if(mipLevel > 7) // too big for this square
               goto norecalloc;
         }
         else
            goto norecalloc;
      }
         
      allocTerrTexture(n->pos, n->level, mipLevel, false, squareDistance);
      curStackSize--;
      continue;
norecalloc:
      // split it up:
      S32 nextLevel = n->level - 1;
      for(S32 i = 0; i < 4; i++)
         n[i].level = nextLevel;

      S32 squareHalfSize = 1 << nextLevel;

      // push in reverse order of processing.
      n[3].pos = pos;
      n[2].pos.set(pos.x + squareHalfSize, pos.y);
      n[1].pos.set(pos.x, pos.y + squareHalfSize);
      n[0].pos.set(pos.x + squareHalfSize, pos.y + squareHalfSize);
      curStackSize += 3;
   }
}

      
void TerrainRender::processBlockStack(SquareStackNode2 *stack, S32 curStackSize)
{
   Point3F minPoint, maxPoint;
   F32 squareDistance;
   mCurrPoint2 = 0;
   mIndexArray2 = gIndexArray2;
   mEmitPoints2 = gEmitPoints2;
   U32 i;

   for(i = 0; i < TerrainBlock::MaterialGroups; i++)
   {
      blendList[i] = 0;
      baseList[i] = 0;
   }
   for(i = 0; i < curStackSize; i++)
      stack[i].texAllocated = false;

   F32 worldToScreenScale   = dglProjectRadius(1,1);
   F32 zeroFullMipDistance  = (mSquareSize * worldToScreenScale) / (1 << 3) - (mSquareSize >> 1);
   F32 zeroSmallMipDistance = (mSquareSize * worldToScreenScale) / (1 << 6) - (mSquareSize >> 1);
   F32 zeroDetailDistance   = (mSquareSize * worldToScreenScale) / (1 << 6) - (mSquareSize >> 1);

   while(curStackSize)
   {
      SquareStackNode2 *n = stack + curStackSize - 1;
      // see if it's visible
      GridSquare *sq = mCurrentBlock->findSquare(n->level, n->pos.x, n->pos.y);

      minPoint.set(mSquareSize * n->pos.x + mBlockPos.x,
                   mSquareSize * n->pos.y + mBlockPos.y,
                   fixedToFloat(sq->minHeight));
      maxPoint.set(minPoint.x + (mSquareSize << n->level),
                   minPoint.y + (mSquareSize << n->level),
                   fixedToFloat(sq->maxHeight));

	  // holes only in the primary terrain block
      if ((sq->flags & GridSquare::Empty) && mBlockPos.x == 0 && mBlockPos.y == 0)
      {
         curStackSize--;
         continue;
      }

      F32 zDiff;
      S32 nextClipFlags = 0;
      squareDistance = getSquareDistance(minPoint, maxPoint, &zDiff);
      
      if(n->clipFlags)
      {
         if(n->clipFlags & FarSphereMask)
         {
            if(squareDistance >= mFarDistance)
            {
               curStackSize--;
               continue;
            }

            S32 squareSz = mSquareSize << n->level;
            if(squareDistance + maxPoint.z - minPoint.z + squareSz + squareSz > mFarDistance)
               nextClipFlags |= FarSphereMask;
         }
         nextClipFlags |= TestSquareVisibility(minPoint, maxPoint, n->clipFlags, mSquareSize);
         if(nextClipFlags == -1)
         {
            //if(!n->texAllocated)
            //   textureRecurse(n);
            // trivially rejected, so pop it off the stack
            curStackSize--;
            continue;
         }
      }

      if(!n->texAllocated)
      {
         S32 squareSz = mSquareSize << n->level;
         // first check the level - if its 3 or less, we have to just make a bitmap:
         // level 3 == 8x8 square - 8x8 * 16x16 == 128x128
         if(n->level > 6)
            goto notexalloc;
            
         S32 mipLevel = 7;
         if(n->level != mTextureMinSquareSize)
         {
            // get the mip level of the square and see if we're in range
            if(squareDistance > 0.001)
            {
               S32 size = S32(dglProjectRadius(squareDistance + (squareSz >> 1), squareSz));
               mipLevel = getPower(size * 0.75);
               if(mipLevel > 7) // too big for this square
                  goto notexalloc;
            }
            else
               goto notexalloc;
         }
         allocTerrTexture(n->pos, n->level, mipLevel, true, squareDistance);
         n->texAllocated = true;
      }
notexalloc:

      if(n->lightMask)
         n->lightMask = TestSquareLights(sq, n->level, n->pos, n->lightMask);

      if(n->level <= 3)
      {
         bool noHoles = !(sq->flags & GridSquare::HasEmpty);
         if(n->level == 3)
         {
            if(nextClipFlags == 0 && noHoles)
            {
               tesselate3(n);
               curStackSize--;
               continue;
            }
         }
         else
         {
            if(!(nextClipFlags & FarSphereMask) && noHoles)
            {
               if(n->level == 2)
                  tesselate2(n);
               else if(n->level == 1)
                  tesselate1(n);
               else
                  tesselate0(n, sq->flags);
               curStackSize--;
               continue;
            }
         }
         if(n->level == 0)
         {
            // it's gotta be far clipped...
            farclip(n, sq->flags);
            curStackSize--;
            continue;
         }
      }
      Point2I pos = n->pos;
      // subdivide this square and throw it on the stack
      S32 nextLevel = n->level - 1;
      S32 squareHalfSize = 1 << nextLevel;
   
      n->level = nextLevel;
      n->clipFlags = nextClipFlags;

      for(S32 i = 1; i < 4; i++)
      {
         n[i].level = nextLevel;
         n[i].clipFlags = nextClipFlags;
         n[i].lightMask = n->lightMask;
      }
      // push in reverse order of processing.
      n[3].pos = pos;
      n[2].pos.set(pos.x + squareHalfSize, pos.y);
      n[1].pos.set(pos.x, pos.y + squareHalfSize);
      n[0].pos.set(pos.x + squareHalfSize, pos.y + squareHalfSize);
      curStackSize += 3;
   }
}

