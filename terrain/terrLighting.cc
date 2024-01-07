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
#include "dgl/gTexManager.h"
#include "terrain/terrRender.h"

//void TerrainRender::setupColorAdder(F32 rSquared, F32 r, F32 g, F32 b)
//{
//   mCurrentR = r;
//   mCurrentG = g;
//   mCurrentB = b;
//   mCurrentRSq = rSquared;
//}
//
//void TerrainRender::addColor(Color &c, F32 dSquared)
//{
//   if(dSquared > mCurrentRSq)
//      return;
//   F32 scale = 255.0f * ( 1 - (dSquared / mCurrentRSq) );
//   c.r += (S32)(mCurrentR * scale);
//   c.g += (S32)(mCurrentG * scale);
//   c.b += (S32)(mCurrentB * scale);
//}
//
//inline U16 TerrainRender::convertColorBack(Color &c)
//{
//   U16 ret;
//   if(c.r > 255)
//      ret = 0xF800;
//   else
//      ret = (c.r >> 3) << 11;
//   if(c.g > 255)
//      ret |= 0x07E0;
//   else
//      ret |= (c.g >> 2) << 5;
//   
//   if(c.b > 255)
//      ret |= 0x001F;
//   else
//      ret |= c.b >> 3;
//      
//   return ret;
//}
//
//inline void TerrainRender::convertColor(Color &c, U16 lix)
//{
//   S32 r = lix >> 11;
//   S32 g = (lix >> 5) & 0x3F;
//   S32 b = lix & 0x1F;
//   
//   c.r = (r << 3) | ( r >> 2);
//   c.g = (g << 2) | ( g >> 4);
//   c.b = (b << 3) | ( b >> 2);
//}
//
//void TerrainRender::lightExpandHorz(F32 x, F32 ySquared, F32 dx, S32 j, S32 i, S32 shift)
//{
//   S32 size = 1 << shift;
//   F32 scale = 1 / F32(size);
//
//   F32 z = mLMABuffer[j][i].z;
//   F32 dz = (mLMABuffer[j][i+size].z - z) * scale;
//
//   for(S32 count = 1; count < size; count++)
//   {
//      x += dx;
//      z += dz;
//      
//      addColor(mLMABuffer[j][i + count], x * x + z * z + ySquared);
//   }
//}
//
//void TerrainRender::lightExpandVert(F32 y, F32 xSquared, F32 dy, S32 j, S32 i, S32 shift)
//{
//   S32 size = 1 << shift;
//   F32 scale = 1 / F32(size);
//
//   F32 z = mLMABuffer[j][i].z;
//   F32 dz = (mLMABuffer[j+size][i].z - z) * scale;
//
//   for(S32 count = 1; count < size; count++)
//   {
//      y += dy;
//      z += dz;
//      
//      mLMABuffer[j+count][i].z = z;
//      addColor(mLMABuffer[j+count][i], y * y + z * z + xSquared);
//   }
//}
//
//void TerrainRender::lmExpandHorz(S32 y, S32 x, S32 shift)
//{
//   S32 size = 1 << shift;
//   Color d;
//   Color c = mLMABuffer[y][x];
//   d.r = (mLMABuffer[y][x+size].r - c.r) >> shift;
//   d.g = (mLMABuffer[y][x+size].g - c.g) >> shift;
//   d.b = (mLMABuffer[y][x+size].b - c.b) >> shift;
//   
//   for(S32 i = 1; i < size; i++)
//   {
//      c.r += d.r;
//      c.g += d.g;
//      c.b += d.b;
//      
//      mLMABuffer[y][x+i] = c;
//   }
//}
//
//void TerrainRender::lmExpandVert(S32 y, S32 x, S32 shift)
//{
//   S32 size = 1 << shift;
//   Color d;
//   Color c = mLMABuffer[y][x];
//   d.r = (mLMABuffer[y+size][x].r - c.r) >> shift;
//   d.g = (mLMABuffer[y+size][x].g - c.g) >> shift;
//   d.b = (mLMABuffer[y+size][x].b - c.b) >> shift;
//   
//   for(S32 i = 1; i < size; i++)
//   {
//      c.r += d.r;
//      c.g += d.g;
//      c.b += d.b;
//      
//      mLMABuffer[y+i][x] = c;
//   }
//}

//void TerrainRender::expandDynamicLight(TerrLightInfo &tl, S32 x, S32 y, S32 level)
//{
//   S32 i, j;
//   setupColorAdder(tl.radiusSquared, tl.r, tl.g, tl.b);
//   
//   if(DynamicLightMapShift - level <= 0)
//   {
//      S32 step = 1 << (level - DynamicLightMapShift);
//      S32 count = DynamicLightMapSize;
//      for(j = 0; j <= count; j++)
//      {
//         for(i = 0; i <= count; i++)
//         {
//            F32 xp = (x + i*step) * mSquareSize - tl.pos.x;
//            F32 yp = (y + j*step) * mSquareSize - tl.pos.y;
//            F32 zp = fixedToFloat(mCurrentBlock->getHeight((x + i) & TerrainBlock::BlockMask,
//                                                                (y + j) & TerrainBlock::BlockMask)) - tl.pos.z;
//
//            addColor(mLMABuffer[j][i], xp * xp + yp * yp + zp * zp);
//         }
//      }
//   }
//   else
//   {
//      S32 delt = DynamicLightMapShift - level;
//      S32 size = 1 << delt;
//      S32 count = 1 << level;
//
//      F32 delta = mSquareSize / F32(size);
//      
//      for(j = 0; j <= count; j++)
//      {
//         for(i = 0; i <= count; i++)
//         {
//            F32 xp = (x + i) * mSquareSize - tl.pos.x;
//            F32 yp = (y + j) * mSquareSize - tl.pos.y;
//            F32 zp = fixedToFloat(mCurrentBlock->getHeight((x + i) & TerrainBlock::BlockMask, (y + j) & TerrainBlock::BlockMask)) - tl.pos.z;
//            mLMABuffer[j << delt][i << delt].z = zp;
//
//            addColor(mLMABuffer[j << delt][i << delt], xp * xp + yp * yp + zp * zp);
//            if(i != 0)
//               lightExpandHorz(xp - mSquareSize, yp * yp, delta, j << delt, (i - 1) << delt, delt);
//            if(j != 0)
//               lightExpandVert(yp - mSquareSize, xp * xp, delta, (j-1) << delt, i << delt, delt);
//            if(i != 0 && j != 0)
//            {
//               F32 xp = (x + i - 1) * mSquareSize - tl.pos.x;
//               for(S32 k = 1; k < size; k++)
//               {
//                  F32 yp = (y + j - 1) * mSquareSize + k * delta - tl.pos.y;
//                  lightExpandHorz(xp, yp * yp, delta,  ((j - 1) << delt) + k, (i-1) << delt, delt);
//               }
//            }
//         }
//      }
//   }
//}

//void TerrainRender::expandLightMap(S32 /*x*/, S32 /*y*/, S32 /*level*/)
//{
//   S32 i, j;
//   x &= TerrainBlock::BlockMask;
//   y &= TerrainBlock::BlockMask;
//   GBitmap *lightMap = mCurrentBlock->lightMapHandle.getBitmap();
//   if(DynamicLightMapShift - level <= 0)
//   {
//      S32 step = (level - DynamicLightMapShift);
//      S32 count = DynamicLightMapSize;
//      S32 lightMask = lightMap->getWidth() - 1;
//      
//      for(j = 0; j <= count; j++)
//      {
//         for(i = 0; i <= count; i++)
//         {
//            U16 lix = * ((U16 *) lightMap->
//               getAddress( (x + (i << step)) & lightMask,
//                           (y + (j << step)) & lightMask ) );
//            convertColor(mLMABuffer[j][i], lix);
//         }
//      }
//   }
//   else
//   {
//      S32 delt = DynamicLightMapShift - level;
//      S32 size = 1 << delt;
//      S32 count = 1 << level;
//      
//      for(j = 0; j <= count; j++)
//      {
//         U16 *pixPtr = (U16 *) lightMap->getAddress(x, y + j);
//         for(i = 0; i <= count; i++)
//         {
//            convertColor(mLMABuffer[j << delt][i << delt], *pixPtr++);
//            if(i != 0)
//               lmExpandHorz(j << delt, (i-1) << delt, delt);
//            if(j != 0)
//               lmExpandVert((j-1) << delt, i << delt, delt);
//            if(i != 0 && j != 0)
//            {
//               for(S32 k = 1; k < size; k++)
//                  lmExpandHorz(((j-1) << delt) + k, (i-1) << delt, delt);
//            }
//         }
//      }
//   }
//}

U32 TerrainRender::TestSquareLights(GridSquare *sq, S32 level, Point2I pos, U32 lightMask)
{
   U32 retMask = 0;
   F32 blockX = pos.x * mSquareSize + mBlockPos.x;
   F32 blockY = pos.y * mSquareSize + mBlockPos.y;
   F32 blockZ = fixedToFloat(sq->minHeight);

   F32 blockSize = mSquareSize * (1 << level);
   F32 blockHeight = fixedToFloat(sq->maxHeight - sq->minHeight);

   Point3F vec;

   for(S32 i = 0; (lightMask >> i) != 0; i++)
   {
      
      if(lightMask & (1 << i))
      {
         Point3F *pos = &mTerrainLights[i].pos;
         // test the visibility of this light to box
         // find closest point on box to light and test
         
         if(pos->z < blockZ)
            vec.z = blockZ - pos->z;
         else if(pos->z > blockZ + blockHeight)
            vec.z = pos->z - (blockZ + blockHeight);
         else
            vec.z = 0;
         
         if(pos->x < blockX)
            vec.x = blockX - pos->x;
         else if(pos->x > blockX + blockSize)
            vec.x = pos->x - (blockX + blockSize);
         else
            vec.x = 0;

         if(pos->y < blockY)
            vec.y = blockY - pos->y;
         else if(pos->y > blockY + blockSize)
            vec.y = pos->y - (blockY + blockSize);
         else
            vec.y = 0;
         F32 dist = vec.len();
         if(dist < mTerrainLights[i].radius)
            retMask |= (1 << i);
      }
   }
   return retMask;
}

//jff tmp
#include "sceneGraph/lightManager.h"
#include "sceneGraph/sceneGraph.h"

void TerrainRender::buildLightArray()
{
   static Vector<LightInfo*> lights;

   //
   lights.clear();
   gClientSceneGraph->getLightManager()->getBestLights(mClipPlane, mNumClipPlanes, mCamPos, lights, MaxTerrainLights);

   // create terrain lights from these...
   U32 curIndex = 0;
   for(U32 i = 0; i < lights.size(); i++)
   {
      if(lights[i]->mType != LightInfo::Point)
         continue;
   
      // set the 'fo
      TerrLightInfo & info = mTerrainLights[curIndex++];

      mCurrentBlock->getWorldTransform().mulP(lights[i]->mPos, &info.pos);
      info.radius = lights[i]->mRadius; //jff
      info.radiusSquared = info.radius * info.radius;
      
      //   
      info.r = lights[i]->mColor.red;
      info.g = lights[i]->mColor.green;
      info.b = lights[i]->mColor.blue;

      Point3F dVec = mCamPos - lights[i]->mPos;
      info.distSquared = mDot(dVec, dVec);
   }

   mDynamicLightCount = curIndex;

   /*const TMat3F & mat = mCamera->getTOW();
   TSSceneLighting * sceneLights = trs.renderContext->getLights();
   SphereF  instSphere ( mat.p, 1000000.0f );
   sceneLights->prepare ( instSphere, mat );

   S32 lightCount = 0;
      
   TSSceneLighting::iterator ptr;
   for ( ptr = sceneLights->begin(); ptr != sceneLights->end() && lightCount < MaxTerrainLights ; ptr++ )
   {
      TSLight *tsl = *ptr;
      if(tsl->fLight.fType == TS::Light::LightPoint && !tsl->isStaticLight())
      {
         mTerrainLights[lightCount].pos = tsl->fLight.fPosition;
         Point3F dVec = mCamPos - mTerrainLights[lightCount].pos;

         mTerrainLights[lightCount].distSquared = m_dot(dVec, dVec);
         mTerrainLights[lightCount].radius = tsl->fLight.fRange;
         mTerrainLights[lightCount].radiusSquared = tsl->fLight.fRange * tsl->fLight.fRange;
         mTerrainLights[lightCount].r = tsl->fLight.fRed;
         mTerrainLights[lightCount].g = tsl->fLight.fGreen;
         mTerrainLights[lightCount].b = tsl->fLight.fBlue;
         lightCount++;
      }
   }
   trs.dynamicLightCount = min(lightCount, (S32)MaxVisibleLights);
   */
}

static U16 convertColor(ColorF color)
{
   if(color.red > 1)
      color.red = 1;
   if(color.green > 1)
      color.green = 1;
   if(color.blue > 1)
      color.blue = 1;
      
   return (U32(color.red * 31) << 11) | 
          (U32(color.green * 31) << 6) |
          (U32(color.blue * 31) << 1) | 1;
}

void TerrainBlock::relight(const ColorF &lightColor, const ColorF &ambient, const Point3F &lightDir)
{
   if(lightDir.x == 0 && lightDir.y == 0)
      return;
   
   if(!lightMap)
      return;

   S32 generateLevel = Con::getIntVariable("$pref::sceneLighting::terrainGenerateLevel", 0);
   generateLevel = mClamp(generateLevel, 0, 4);

   U32 generateDim = TerrainBlock::LightmapSize << generateLevel;
   U32 generateShift = TerrainBlock::LightmapShift + generateLevel;
   U32 generateMask = generateDim - 1;
   
   F32 zStep;
   F32 frac;
   
   Point2I blockColStep;
   Point2I blockRowStep;
   Point2I blockFirstPos;
   Point2I lmapFirstPos;
      
   F32 terrainDim = F32(getSquareSize()) * F32(TerrainBlock::BlockSize);
   F32 stepSize = F32(getSquareSize()) / F32(generateDim / TerrainBlock::BlockSize);

   if(mFabs(lightDir.x) >= mFabs(lightDir.y))
   {
      if(lightDir.x > 0)
      {
         zStep = lightDir.z / lightDir.x;
         frac = lightDir.y / lightDir.x;

         blockColStep.set(1, 0);
         blockRowStep.set(0, 1);
         blockFirstPos.set(0, 0);
         lmapFirstPos.set(0, 0);
      }
      else
      {
         zStep = -lightDir.z / lightDir.x;
         frac = -lightDir.y / lightDir.x;

         blockColStep.set(-1, 0);
         blockRowStep.set(0, 1);
         blockFirstPos.set(255, 0);
         lmapFirstPos.set(TerrainBlock::LightmapSize-1, 0);
      }
   }
   else
   {
      if(lightDir.y > 0)
      {
         zStep = lightDir.z / lightDir.y;
         frac = lightDir.x / lightDir.y;

         blockColStep.set(0, 1);
         blockRowStep.set(1, 0);
         blockFirstPos.set(0, 0);
         lmapFirstPos.set(0, 0);
      }
      else
      {
         zStep = -lightDir.z / lightDir.y;
         frac = -lightDir.x / lightDir.y;

         blockColStep.set(0, -1);
         blockRowStep.set(1, 0);
         blockFirstPos.set(0, 255);
         lmapFirstPos.set(0, TerrainBlock::LightmapSize-1);
      }
   }
   zStep *= stepSize;

   F32 * heightArray = new F32[generateDim];

   S32 fracStep = -1;
   if(frac < 0)
   {
      fracStep = 1;
      frac = -frac;
   }

   F32 * nextHeightArray = new F32[generateDim];
   F32 oneMinusFrac = 1 - frac;

   U32 blockShift = generateShift - TerrainBlock::BlockShift;
   U32 lightmapShift = generateShift - TerrainBlock::LightmapShift;

   U32 blockStep = 1 << blockShift;
   U32 blockMask = (1 << blockShift) - 1;
   U32 lightmapMask = (1 << lightmapShift) - 1;

   Point2I bp = blockFirstPos;
   F32 terrainHeights[2][TerrainBlock::BlockSize];
   U32 i;

   // get first set of heights
   for(i = 0; i < TerrainBlock::BlockSize; i++, bp += blockRowStep)
      terrainHeights[0][i] = fixedToFloat(getHeight(bp.x, bp.y));

   // get second set of heights
   bp = blockFirstPos + blockColStep;
   for(i = 0; i < TerrainBlock::BlockSize; i++, bp += blockRowStep)
      terrainHeights[1][i] = fixedToFloat(getHeight(bp.x, bp.y));

   F32 * pTerrainHeights[2];
   pTerrainHeights[0] = static_cast<F32*>(terrainHeights[0]);
   pTerrainHeights[1] = static_cast<F32*>(terrainHeights[1]);

   F32 heightStep = 1.f / blockStep;

   F32 terrainZRowStep[2][TerrainBlock::BlockSize];
   F32 terrainZColStep[TerrainBlock::BlockSize];

   // fill in the row steps
   for(i = 0; i < TerrainBlock::BlockSize; i++)
   {
      terrainZRowStep[0][i] = (terrainHeights[0][(i+1) & TerrainBlock::BlockMask] - terrainHeights[0][i]) * heightStep;
      terrainZRowStep[1][i] = (terrainHeights[1][(i+1) & TerrainBlock::BlockMask] - terrainHeights[1][i]) * heightStep;
      terrainZColStep[i] = (terrainHeights[1][i] - terrainHeights[0][i]) * heightStep;
   }
   
   // get first row of process heights   
   for(i = 0; i < generateDim; i++)
   {
      U32 bi = i >> blockShift;
      heightArray[i] = terrainHeights[0][bi] + (i & blockMask) * terrainZRowStep[0][bi];
   }

   bp = blockFirstPos;
   if(generateDim == TerrainBlock::BlockSize)
      bp += blockColStep;

   // generate the initial run
   U32 x, y;
   for(x = 1; x < generateDim; x++)
   {
      U32 xmask = x & blockMask;

      // generate new height step rows?
      if(!xmask)
      {
         F32 * tmp = pTerrainHeights[0];
         pTerrainHeights[0] = pTerrainHeights[1];
         pTerrainHeights[1] = tmp;

         bp += blockColStep;
         
         Point2I bwalk = bp;
         for(i = 0; i < TerrainBlock::BlockSize; i++, bwalk += blockRowStep)
            pTerrainHeights[1][i] = fixedToFloat(getHeight(bwalk.x, bwalk.y));

         // fill in the row steps
         for(i = 0; i < TerrainBlock::BlockSize; i++)
         {
            terrainZRowStep[0][i] = (pTerrainHeights[0][(i+1) & TerrainBlock::BlockMask] - pTerrainHeights[0][i]) * heightStep;
            terrainZRowStep[1][i] = (pTerrainHeights[1][(i+1) & TerrainBlock::BlockMask] - pTerrainHeights[1][i]) * heightStep;
            terrainZColStep[i] = (pTerrainHeights[1][i] - pTerrainHeights[0][i]) * heightStep;
         }
      }

      Point2I bwalk = bp - blockRowStep;
      for(y = 0; y < generateDim; y++)
      {
         U32 ymask = y & blockMask;
         if(!ymask)
            bwalk += blockRowStep;

         U32 bi = y >> blockShift;
         U32 binext = (bi + 1) & TerrainBlock::BlockMask;

         F32 height;

         // 135?
         if((bwalk.x ^ bwalk.y) & 1)
         {
            U32 xsub = blockStep - xmask;
            if(xsub > ymask) // bottom
               height = pTerrainHeights[0][bi] + xmask * terrainZColStep[bi] + 
                        ymask * terrainZRowStep[0][bi];
            else // top
               height = pTerrainHeights[1][bi] - xsub * terrainZColStep[binext] + 
                        ymask * terrainZRowStep[1][bi];
         }
         else
         {
            if(xmask > ymask) // bottom
               height = pTerrainHeights[0][bi] + xmask * terrainZColStep[bi] +
                        ymask * terrainZRowStep[1][bi];
            else // top
               height = pTerrainHeights[0][bi] + xmask * terrainZColStep[binext] +
                        ymask * terrainZRowStep[0][bi];
         }

         F32 intHeight = heightArray[y] * oneMinusFrac + heightArray[(y + fracStep) & generateMask] * frac + zStep;
         nextHeightArray[y] = getMax(height, intHeight);
      }

      // swap the height rows
      for(y = 0; y < generateDim; y++)
         heightArray[y] = nextHeightArray[y];
   }

   F32 squareSize = getSquareSize();
   F32 squaredSquareSize = squareSize * squareSize;
   F32 lexelDim = squareSize * F32(TerrainBlock::BlockSize) / F32(TerrainBlock::LightmapSize);

   // calculate normal runs
   Point3F normals[2][TerrainBlock::BlockSize];
   Point3F * pNormals[2];

   pNormals[0] = static_cast<Point3F*>(normals[0]);
   pNormals[1] = static_cast<Point3F*>(normals[1]);

   // calculate the normal lookup table
   F32 * normTable = new F32 [blockStep * blockStep * 4];

   Point2F corners[4] = {
      Point2F(0.f, 0.f),
      Point2F(1.f, 0.f),
      Point2F(1.f, 1.f),
      Point2F(0.f, 1.f)
   };

   U32 idx = 0;
   F32 step = 1.f / blockStep;
   F32 halfStep = step / 2.f;
   Point2F pos(halfStep, halfStep);

   // fill it
   for(x = 0; x < blockStep; x++, pos.x += step, pos.y = halfStep)
      for(y = 0; y < blockStep; y++, pos.y += step)
         for(U32 i = 0; i < 4; i++, idx++)
            normTable[idx] = 1.f - getMin(Point2F(pos - corners[i]).len(), 1.f);

   // fill first column
   bp = blockFirstPos;
   for(x = 0; x < TerrainBlock::BlockSize; x++)
   {
      terrainHeights[0][x] = fixedToFloat(getHeight(bp.x, bp.y));
      Point2F pos(bp.x * squareSize, bp.y * squareSize);
      getNormal(pos, &pNormals[1][x]);
      bp += blockRowStep;
   }

   // get swapped on first pass
   pTerrainHeights[0] = static_cast<F32*>(terrainHeights[1]);
   pTerrainHeights[1] = static_cast<F32*>(terrainHeights[0]);

   ColorF colors[TerrainBlock::LightmapSize];

   F32 ratio = F32(1 << lightmapShift);
   F32 inverseRatioSquared = 1.f / (ratio * ratio);
   
   // walk it...   
   bp = blockFirstPos - blockColStep;
   Point2I lp = lmapFirstPos - blockColStep;

   for(x = 0; x < generateDim; x++)
   {
      U32 xmask = x & blockMask;

      // process lightmap?
      if(!(x & lightmapMask))
      {
         dMemset(colors, 0, sizeof(ColorF) * TerrainBlock::LightmapSize);
         lp += blockColStep;
      }

      // generate new runs?
      if(!xmask)
      {
         bp += blockColStep;

         // do the normals
         Point3F * temp = pNormals[0];
         pNormals[0] = pNormals[1];
         pNormals[1] = temp;
      
         // fill the row
         Point2I bwalk = bp + blockColStep;
         for(U32 i = 0; i < TerrainBlock::BlockSize; i++)
         {
            Point2F pos(bwalk.x * squareSize, bwalk.y * squareSize);
            getNormal(pos, &pNormals[1][i]);
            bwalk += blockRowStep;
         }

         // do the heights   
         F32 * tmp = pTerrainHeights[0];
         pTerrainHeights[0] = pTerrainHeights[1];
         pTerrainHeights[1] = tmp;

         bwalk = bp + blockColStep;
         for(i = 0; i < TerrainBlock::BlockSize; i++, bwalk += blockRowStep)
            pTerrainHeights[1][i] = fixedToFloat(getHeight(bwalk.x, bwalk.y));

         // fill in the row steps
         for(i = 0; i < TerrainBlock::BlockSize; i++)
         {
            terrainZRowStep[0][i] = (pTerrainHeights[0][(i+1) & TerrainBlock::BlockMask] - pTerrainHeights[0][i]) * heightStep;
            terrainZRowStep[1][i] = (pTerrainHeights[1][(i+1) & TerrainBlock::BlockMask] - pTerrainHeights[1][i]) * heightStep;
            terrainZColStep[i] = (pTerrainHeights[1][i] - pTerrainHeights[0][i]) * heightStep;
         }
      }

      Point2I bwalk = bp - blockRowStep;
      for(y = 0; y < generateDim; y++)
      {
         U32 ymask = y & blockMask;
         if(!ymask)
            bwalk += blockRowStep;

         U32 bi = y >> blockShift;
         U32 binext = (bi + 1) & TerrainBlock::BlockMask;

         F32 height;

         // 135?
         if((bwalk.x ^ bwalk.y) & 1)
         {
            U32 xsub = blockStep - xmask;
            if(xsub > ymask) // bottom
               height = pTerrainHeights[0][bi] + xmask * terrainZColStep[bi] + 
                        ymask * terrainZRowStep[0][bi];
            else // top
               height = pTerrainHeights[1][bi] - xsub * terrainZColStep[binext] + 
                        ymask * terrainZRowStep[1][bi];
         }
         else
         {
            if(xmask > ymask) // bottom
               height = pTerrainHeights[0][bi] + xmask * terrainZColStep[bi] +
                        ymask * terrainZRowStep[1][bi];
            else // top
               height = pTerrainHeights[0][bi] + xmask * terrainZColStep[binext] +
                        ymask * terrainZRowStep[0][bi];
         }

         F32 intHeight = heightArray[y] * oneMinusFrac + heightArray[(y + fracStep) & generateMask] * frac + zStep;

         ColorF & col = colors[y >> lightmapShift];

         // non shadowed?
         if(height >= intHeight)
         {
            U32 idx = (xmask + (ymask << blockShift)) << 2;

            Point3F normal;
            normal = pNormals[0][bi] * normTable[idx++];
            normal += pNormals[0][binext] * normTable[idx++];
            normal += pNormals[1][binext] * normTable[idx++];
            normal += pNormals[1][bi] * normTable[idx];
            normal.normalize();

            nextHeightArray[y] = height;
            F32 colorScale = mDot(normal, lightDir);
            if(colorScale >= 0)
               col += ambient;
            else
               col += (ambient + lightColor * -colorScale);
         }
         else
         {
            nextHeightArray[y] = intHeight;
            col += ambient;
         }
      }

      for(y = 0; y < generateDim; y++)
         heightArray[y] = nextHeightArray[y];

      // do some lighting stuff?
      if(!((x+1) & lightmapMask))
      {
         Point2I lwalk = lp;
         U32 mask = TerrainBlock::LightmapSize - 1;
         for(U32 i = 0; i < TerrainBlock::LightmapSize; i++)
         {
            U16 * ptr = (U16*)lightMap->getAddress(lp.x & mask, lp.y & mask);
            *ptr = convertColor(colors[i]);
            lp += blockRowStep;
         }
      }
   }

   delete [] normTable;
   delete [] heightArray;
   delete [] nextHeightArray;
}

