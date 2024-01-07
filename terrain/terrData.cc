//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "terrain/terrData.h"
#include "dgl/gBitmap.h"
#include "math/mMath.h"			  
#include "console/consoleTypes.h"
#include "core/fileStream.h"
#include "terrain/terrRender.h"
#include "dgl/materialList.h"
#include "scenegraph/sceneGraph.h"
#include "scenegraph/sceneState.h"
#include "platformWIN32/platformGL.h"
#include "dgl/dgl.h"
#include "core/bitStream.h"
#include "dgl/gTexManager.h"
#include "math/mathIO.h"
#include "terrain/blender.h"
#include "sim/netConnection.h"
#include "dgl/materialPropertyMap.h"


extern bool gDGLRender;


IMPLEMENT_CO_NETOBJECT_V1(TerrainBlock);

namespace {

void terrainTextureEventCB(const U32 eventCode, const U32 userData)
{
   TerrainBlock* pTerr = reinterpret_cast<TerrainBlock*>(userData);
   pTerr->processTextureEvent(eventCode);
}

Point3F sgTexGenS;
Point3F sgTexGenT;
Point3F sgLMGenS;
Point3F sgLMGenT;

} // namespace {}



//--------------------------------------
TerrainBlock::TerrainBlock()
{
   squareSize = 8;
   
   mBlender = NULL;
   lightMap = 0;
   
   for(S32 i = BlockShift; i >= 0; i--)
      gridMap[i] = NULL;
   
   heightMap = NULL;
   materialMap = NULL;
   mBaseMaterialMap = NULL;
   mMaterialFileName = NULL;
   mTypeMask = TerrainObjectType | StaticObjectType | StaticRenderedObjectType;
   mNetFlags.set(Ghostable | ScopeAlways);
   mCollideEmpty = false;
   mDetailTextureName = NULL;
   mCRC = 0;
   flagMap = 0;
	mVertexBuffer = -1;
}


//--------------------------------------
TerrainBlock::~TerrainBlock()
{
   delete lightMap;
}


//--------------------------------------
void TerrainBlock::setFile(Resource<TerrainFile> terr)
{
   mFile = terr;
   for(U32 i = 0; i <= BlockShift; i++)
      gridMap[i] = mFile->mGridMap[i];
      
   mBaseMaterialMap = mFile->mBaseMaterialMap;
   mMaterialFileName= mFile->mMaterialFileName;
   mChunkMap = mFile->mChunkMap;
   materialMap = mFile->mMaterialMap;
   heightMap   = mFile->mHeightMap;
   flagMap = mFile->mFlagMap;
}


//--------------------------------------------------------------------------
bool TerrainBlock::save(const char *filename)
{
   return mFile->save(filename);
}   


//--------------------------------------
static U16 calcDev(PlaneF &pl, Point3F &pt)
{
   F32 z = (pl.d + pl.x * pt.x + pl.y * pt.y) / -pl.z;
   F32 diff = z - pt.z;
   if(diff < 0)
      diff = -diff;
      
   if(diff > 0xFFFF)
      return 0xFFFF;
   else
      return U16(diff);
}

static U16 Umax(U16 u1, U16 u2)
{
   return u1 > u2 ? u1 : u2;
}

//------------------------------------------------------------------------------

bool TerrainBlock::unpackEmptySquares()
{
   U32 size = BlockSquareWidth * BlockSquareWidth;

   U32 i;
   for(i = 0; i < size; i++)
      materialMap[i].flags &= ~Material::Empty;

   // walk the pairs
   for(i = 0; i < mEmptySquareRuns.size(); i++)
   {
      U32 offset = mEmptySquareRuns[i] & 0xffff;
      U32 run = U32(mEmptySquareRuns[i]) >> 16;

      //
      for(U32 j = 0; j < run; j++)
      {
         if((offset+j) >= size)
         {
            Con::errorf(ConsoleLogEntry::General, "TerrainBlock::unpackEmpties: invalid entry.");
            return(false);
         }
         materialMap[offset+j].flags |= Material::Empty;
      }
   }

   rebuildEmptyFlags();
   return(true);
}

void TerrainBlock::packEmptySquares()
{  
   AssertFatal(isServerObject(), "TerrainBlock::packEmptySquares: client!");
   mEmptySquareRuns.clear();

   // walk the map
   U32 run = 0;
   U32 offset;

   U32 size = BlockSquareWidth * BlockSquareWidth;
   for(U32 i = 0; i < size; i++)
   {
      if(materialMap[i].flags & Material::Empty)
      {
         if(!run)
            offset = i;
         run++;
      }
      else if(run)
      {
         mEmptySquareRuns.push_back((run << 16) | offset);
         run = 0;

         // cap it
         if(mEmptySquareRuns.size() == MaxEmptyRunPairs)
            break;
      }
   }

   //
   if(run && mEmptySquareRuns.size() != MaxEmptyRunPairs)
      mEmptySquareRuns.push_back((run << 16) | offset);

   if(mEmptySquareRuns.size() == MaxEmptyRunPairs)
      Con::warnf(ConsoleLogEntry::General, "TerrainBlock::packEmptySquares: More than %d run pairs encountered.  Extras will not persist.", MaxEmptyRunPairs);

   //
   mNetFlags |= EmptyMask;
}

//------------------------------------------------------------------------------

void TerrainBlock::rebuildEmptyFlags()
{
   // rebuild entire maps empty flags!
   for(U32 y = 0; y < TerrainBlock::ChunkSquareWidth; y++)
   {
      for(U32 x = 0; x < TerrainBlock::ChunkSquareWidth; x++)
      {
         GridChunk &gc = *(mChunkMap + x + (y << TerrainBlock::ChunkShift));
         gc.emptyFlags = 0;
         U32 sx = x << TerrainBlock::ChunkDownShift;
         U32 sy = y << TerrainBlock::ChunkDownShift;
         for(U32 j = 0; j < 4; j++)
         {
            for(U32 i = 0; i < 4; i++)
            {
               TerrainBlock::Material *mat = getMaterial(sx + i, sy + j);
               if(mat->flags & TerrainBlock::Material::Empty)
                  gc.emptyFlags |= (1 << (j * 4 + i));
            }
         }
      }
   }

   for(S32 i = BlockShift; i >= 0; i--)
   {
      S32 squareCount = 1 << (BlockShift - i);
      S32 squareSize = (TerrainBlock::BlockSize) / squareCount;

      for(S32 squareX = 0; squareX < squareCount; squareX++)
      {
         for(S32 squareY = 0; squareY < squareCount; squareY++)
         {
            GridSquare *parent = NULL;
            if(i < BlockShift)
               parent = findSquare(i+1, Point2I(squareX * squareSize, squareY * squareSize));
            bool empty = true;
      
            for(S32 sizeX = 0; empty && sizeX <= squareSize; sizeX++)
            {
               for(S32 sizeY = 0; empty && sizeY <= squareSize; sizeY++)
               {
                  S32 x = squareX * squareSize + sizeX;
                  S32 y = squareY * squareSize + sizeY;

                  if(sizeX != squareSize && sizeY != squareSize)
                  {
                     TerrainBlock::Material *mat = getMaterial(x, y);
                     if(!(mat->flags & TerrainBlock::Material::Empty))
                        empty = false;
                  }
               }
            }
            GridSquare *sq = findSquare(i, Point2I(squareX * squareSize, squareY * squareSize));
            if(empty)
               sq->flags |= GridSquare::Empty;
            else
               sq->flags &= ~GridSquare::Empty;
         }
      }
   }
}

//------------------------------------------------------------------------------

void TerrainBlock::setHeight(const Point2I & pos, float height)
{
   // set the height
   U16 ht = floatToFixed(height);
   *((U16*)getHeightAddress(pos.x, pos.y)) = ht;
   
   // update the chunk deviance
   Point2I chunkPos(pos.x >> ChunkDownShift, pos.y >> ChunkDownShift);
   buildChunkDeviance(chunkPos.x, chunkPos.y);

   // update neighbors
   U32 chunkMask = ChunkSize - 1;
   U32 chunkSquareMask = ChunkSquareWidth - 1;

   for(S32 x = 0; x < ChunkSize; x += chunkMask)
   {
      bool xEdge = (pos.x & chunkMask) == x;
      if(xEdge)
         buildChunkDeviance((x ? (chunkPos.x + 1) : (chunkPos.x - 1)) & chunkSquareMask, chunkPos.y);

      for(S32 y = 0; y < ChunkSize; y += chunkMask)
      {
         bool yEdge = (pos.y & chunkMask) == y;

         if(yEdge)
         {
            buildChunkDeviance(chunkPos.x, (y ? (chunkPos.y + 1) : (chunkPos.y - 1)) & chunkSquareMask);

            if(xEdge)
               buildChunkDeviance((x ? (chunkPos.x + 1) : (chunkPos.x - 1)) & chunkSquareMask, 
                                  (y ? (chunkPos.y + 1) : (chunkPos.y - 1)) & chunkSquareMask);
         }
      }
   }

   Point3F p1, p2, p3, p4;
   PlaneF pl1, pl2, pl3, pl4;

   p1.set(0, 0, getHeight(pos.x, pos.y));
   p2.set(0, 1, getHeight(pos.x, pos.y + 1));
   p3.set(1, 1, getHeight(pos.x + 1, pos.y + 1));
   p4.set(1, 0, getHeight(pos.x + 1, pos.y));

   pl1.set(p1, p2, p3);
   pl2.set(p1, p3, p4);
   pl3.set(p1, p2, p4);
   pl4.set(p2, p3, p4);

   GridSquare * gs = findSquare(0, pos);

   U16 mindev45 = 0;
   U16 mindev135 = 0;

   for(U32 sizeX = 0; sizeX <= 1; sizeX++)
      for(U32 sizeY = 0; sizeY <= 1; sizeY++)
      {
         U32 x = pos.x + sizeX;
         U32 y = pos.y + sizeY;
         U16 ht = getHeight(x, y);

         if(ht < gs->minHeight)
            gs->minHeight = ht;
         if(ht > gs->maxHeight)
            gs->maxHeight = ht;

         Point3F pt(sizeX, sizeY, fixedToFloat(ht));
            
         U16 dev;
         if(sizeX < sizeY)
            dev = calcDev(pl1, pt);
         else if(sizeX > sizeY)
            dev = calcDev(pl2, pt);
         else
            dev = Umax(calcDev(pl1, pt), calcDev(pl2, pt));   
         
         if(dev > mindev45)
            mindev45 = dev;
            
         if((sizeX + sizeY) == 0)
            dev = calcDev(pl3, pt);
         else if((sizeX + sizeY) > 1)
            dev = calcDev(pl4, pt);
         else
            dev = Umax(calcDev(pl3, pt), calcDev(pl4, pt));

         if(dev > mindev135)
            mindev135 = dev;
      }

   //
   gs->flags &= GridSquare::Empty;

   if(((pos.x ^ pos.y) & 1) == 0)
   {
      gs->flags |= GridSquare::Split45;
      gs->heightDeviance = mindev45;
   }
   else
      gs->heightDeviance = mindev135;

   //
   for(U32 i = 1; i <= TerrainBlock::BlockShift; i++)
   {
      GridSquare * parent = findSquare(i, pos);
      
      if(parent->minHeight > gs->minHeight)
         parent->minHeight = gs->minHeight;
      if(parent->maxHeight < gs->maxHeight)
         parent->maxHeight = gs->maxHeight;
      if(parent->heightDeviance < gs->heightDeviance)
         parent->heightDeviance = gs->heightDeviance;
   }
}


//--------------------------------------
bool TerrainBlock::getHeight(const Point2F & pos, float * height)
{
   float invSquareSize = 1.0f / (float)squareSize;
   float xp = pos.x * invSquareSize;
   float yp = pos.y * invSquareSize;
   int x = (S32)mFloor(xp);
   int y = (S32)mFloor(yp);
   xp -= (float)x;
   yp -= (float)y;
   x &= BlockMask;
   y &= BlockMask;
   GridSquare * gs = findSquare(0, Point2I(x,y));

   if (gs->flags & GridSquare::Empty)
      return false;   

   float zBottomLeft = fixedToFloat(getHeight(x, y));
   float zBottomRight = fixedToFloat(getHeight(x + 1, y));
   float zTopLeft = fixedToFloat(getHeight(x, y + 1));
   float zTopRight = fixedToFloat(getHeight(x + 1, y + 1));

   if(gs->flags & GridSquare::Split45)
   {
      if (xp>yp)
         // bottom half
         *height = zBottomLeft + xp * (zBottomRight-zBottomLeft) + yp * (zTopRight-zBottomRight);
      else
         // top half
         *height = zBottomLeft + xp * (zTopRight-zTopLeft) + yp * (zTopLeft-zBottomLeft);
   }
   else
   {
      if (1.0f-xp>yp)
         // bottom half
         *height = zBottomRight + (1.0f-xp) * (zBottomLeft-zBottomRight) + yp * (zTopLeft-zBottomLeft);
      else
         // top half
         *height = zBottomRight + (1.0f-xp) * (zTopLeft-zTopRight) + yp * (zTopRight-zBottomRight);
   }
   return true;
}

bool TerrainBlock::getNormal(const Point2F & pos, Point3F * normal, bool normalize)
{
   float invSquareSize = 1.0f / (float)squareSize;
   float xp = pos.x * invSquareSize;
   float yp = pos.y * invSquareSize;
   int x = (S32)mFloor(xp);
   int y = (S32)mFloor(yp);
   xp -= (float)x;
   yp -= (float)y;
   x &= BlockMask;
   y &= BlockMask;
   GridSquare * gs = findSquare(0, Point2I(x,y));

   if (gs->flags & GridSquare::Empty)
      return false;   

   float zBottomLeft = fixedToFloat(getHeight(x, y));
   float zBottomRight = fixedToFloat(getHeight(x + 1, y));
   float zTopLeft = fixedToFloat(getHeight(x, y + 1));
   float zTopRight = fixedToFloat(getHeight(x + 1, y + 1));

   if(gs->flags & GridSquare::Split45)
   {
      if (xp>yp)
         // bottom half
         normal->set(zBottomLeft-zBottomRight,zBottomRight-zTopRight,squareSize);
      else
         // top half
         normal->set(zTopLeft-zTopRight,zBottomLeft-zTopLeft,squareSize);
   }
   else
   {
      if (1.0f-xp>yp)
         // bottom half
         normal->set(zBottomLeft-zBottomRight,zBottomLeft-zTopLeft,squareSize);
      else
         // top half
         normal->set(zTopLeft-zTopRight,zBottomRight-zTopRight,squareSize);
   }
   if (normalize)
      normal->normalize();
   return true;
}

bool TerrainBlock::getNormalAndHeight(const Point2F & pos, Point3F * normal, F32 * height, bool normalize)
{
   float invSquareSize = 1.0f / (float)squareSize;
   float xp = pos.x * invSquareSize;
   float yp = pos.y * invSquareSize;
   int x = (S32)mFloor(xp);
   int y = (S32)mFloor(yp);
   xp -= (float)x;
   yp -= (float)y;
   x &= BlockMask;
   y &= BlockMask;
   GridSquare * gs = findSquare(0, Point2I(x,y));

   if (gs->flags & GridSquare::Empty)
      return false;   

   float zBottomLeft = fixedToFloat(getHeight(x, y));
   float zBottomRight = fixedToFloat(getHeight(x + 1, y));
   float zTopLeft = fixedToFloat(getHeight(x, y + 1));
   float zTopRight = fixedToFloat(getHeight(x + 1, y + 1));

   if(gs->flags & GridSquare::Split45)
   {
      if (xp>yp)
      {
         // bottom half
         normal->set(zBottomLeft-zBottomRight,zBottomRight-zTopRight,squareSize);
         *height = zBottomLeft + xp * (zBottomRight-zBottomLeft) + yp * (zTopRight-zBottomRight);
      }
      else
      {
         // top half
         normal->set(zTopLeft-zTopRight,zBottomLeft-zTopLeft,squareSize);
         *height = zBottomLeft + xp * (zTopRight-zTopLeft) + yp * (zTopLeft-zBottomLeft);
      }
   }
   else
   {
      if (1.0f-xp>yp)
      {
         // bottom half
         normal->set(zBottomLeft-zBottomRight,zBottomLeft-zTopLeft,squareSize);
         *height = zBottomRight + (1.0f-xp) * (zBottomLeft-zBottomRight) + yp * (zTopLeft-zBottomLeft);
      }
      else
      {
         // top half
         normal->set(zTopLeft-zTopRight,zBottomRight-zTopRight,squareSize);
         *height = zBottomRight + (1.0f-xp) * (zTopLeft-zTopRight) + yp * (zTopRight-zBottomRight);
      }
   }
   if (normalize)
      normal->normalize();
   return true;
}

//--------------------------------------


//--------------------------------------
void TerrainBlock::setBaseMaterials(S32 argc, const char *argv[])
{
   for (S32 i = 0; i < argc; i++)
      mMaterialFileName[i] = StringTable->insert(argv[i]);   
   for (S32 j = argc; j < MaterialGroups; j++)
      mMaterialFileName[j] = NULL;
}   

//------------------------------------------------------------------------------

//--------------------------------------

void TerrainBlock::buildMipMap()
{
//   TerrainRender::mCurrentBlock = this;
//   for(S32 i = 0; i < 16; i++)
//   {
//      AllocatedTexture tex;
//      tex.x = (i & 3) * 64;
//      tex.y = (i >> 2) * 64;
//      tex.level = 6;
//      tex.mipLevel = 7;

//      TerrainRender::buildBlendMap(&tex);
//      baseTextures[i] = tex.handle;
//   }
}

//--------------------------------------

bool TerrainBlock::buildMaterialMap()
{
   TerrainRender::flushCache();
#if 0
   if (1)
   {
      for (i = 0; i < MaterialGroups; i++)
      {
         if(!mMaterialFileName[i] || !*mMaterialFileName[i] || !getMaterialAlphaMap(i))
            break;
         char fileBuf[256];
         dSprintf(fileBuf, sizeof(fileBuf), "terrain/%s", mMaterialFileName[i]);
         mBaseMaterials[i] = TextureHandle(fileBuf, MeshTexture);
         if(!mBaseMaterials[i])
            return false;
         U8 *amap = getMaterialAlphaMap(i);
         GBitmap *bmp = new GBitmap(256, 256, false, GBitmap::Luminance);
         U8 *dest = bmp->getWritableBits();
         dMemcpy(dest, amap, 256 * 256);
         mAlphaMaterials[i] = TextureHandle(NULL, bmp);
      }
      if(i == 0)
         return false;
   }
#endif
   if (initMMXBlender() == false)
      return false;
   if (gDGLRender)
      buildMipMap();

   return true;
}   

bool TerrainBlock::initMMXBlender()
{
   // DMMNOTE: come back to this
   delete mBlender;
   mBlender = NULL;

   char fileBuf[256];

   U32 validMaterials = 0;
   S32 i;
   for (i = 0; i < MaterialGroups; i++) {
      if (mMaterialFileName[i] && *mMaterialFileName[i])
         validMaterials++;
      else
         break;
   }
   AssertFatal(validMaterials != 0, "Error, must have SOME materials here!");

   // Submit alphamaps
   U8* alphaMaterials[MaterialGroups];
   dMemset(alphaMaterials, 0, sizeof(alphaMaterials));
   for (i = 0; i < validMaterials; i++) {
      if (getMaterialAlphaMap(i) == NULL) {
         AssertFatal(getMaterialAlphaMap(i) != NULL, "Error, need an alpha map here!");
         return false;
      }
      alphaMaterials[i] = getMaterialAlphaMap(i);
   }

   mBlender = new Blender(validMaterials, 5, alphaMaterials);

   // Ok, we have validMaterials set correctly
   for(i = 0; i < validMaterials; i++) {
      AssertFatal(mMaterialFileName[i] && *mMaterialFileName[i], "Error, something wacky here");
      StringTableEntry fn = mMaterialFileName[i];
      if(!dStrncmp(fn, "terrain.", 8))
         fn += 8;
      dSprintf(fileBuf, sizeof(fileBuf), "textures/terrain/%s.png", fn);

      // DMM
      Stream* pMaterialStream = ResourceManager->openStream(fileBuf);
      if (pMaterialStream == NULL)
         pMaterialStream = ResourceManager->openStream("textures/terrain/Default.png");
      AssertFatal(pMaterialStream != NULL, "Error, must have a stream at this point!");

      GBitmap* pBitmap = new GBitmap;
      if (pBitmap->readPNG(*pMaterialStream) == false) {
         AssertFatal(false, "Must be a png at this point!");
         return false;
      }
      ResourceManager->closeStream(pMaterialStream);
      pBitmap->extrudeMipLevels();

      // Submit as material for group i
      const U8* pMipmaps[5];
      for (U32 j = 0; j < 5; j++)
         pMipmaps[j] = pBitmap->getBits(j);

      mBlender->addSourceTexture(i, pMipmaps);

      delete pBitmap;
   }

   return true;
}   

//------------------------------------------------------------------------------

void TerrainBlock::refreshMaterialLists()
{
}

//------------------------------------------------------------------------------

void TerrainBlock::onEditorEnable()
{
   // need to load in the material base material lists
   if(isClientObject())
      refreshMaterialLists();
}

void TerrainBlock::onEditorDisable()
{
   
}

//------------------------------------------------------------------------------

bool TerrainBlock::onAdd()
{
   if(!Parent::onAdd())
      return false;

   setPosition(Point3F(-squareSize * (BlockSize >> 1), -squareSize * (BlockSize >> 1), 0));

   char fileBuf[256];
   dSprintf(fileBuf, sizeof(fileBuf), "terrains/%s", mTerrFileName);
   
   Resource<TerrainFile> terr = ResourceManager->load(fileBuf, true);
   if(!bool(terr))
   {
      if(isClientObject())
         NetConnection::setLastError("You are missing a file needed to play this mission: %s", mTerrFileName);
      return false;
   }

   setFile(terr);
   char nameBuff[256] = "terrain/";
   MaterialPropertyMap* pMatMap = static_cast<MaterialPropertyMap*>(Sim::findObject("MaterialPropertyMap"));
   
   StringTableEntry fn = mMaterialFileName[0];
   if(!dStrncmp(fn, "terrain.", 8))
      fn += 8;

   dStrcat(nameBuff,fn); 
   mMPMIndex[0] = pMatMap->getIndexFromName(nameBuff);
   
   mObjBox.min.set(-1e8, -1e8, -1e8);
   mObjBox.max.set( 1e8,  1e8,  1e8);
   resetWorldBox();
   setRenderTransform(mObjToWorld);

   if (isClientObject())
   {
      if(mCRC != terr.getCRC())
      {
         NetConnection::setLastError("Your terrain file doesn't match the version that is running on the server.");
         return false;
      }

      if(mDetailTextureName && mDetailTextureName[0])
         mDetailTextureHandle = TextureHandle(mDetailTextureName, DetailTexture);
      
      lightMap = new GBitmap(LightmapSize, LightmapSize, false, GBitmap::RGB5551);

      if (!buildMaterialMap())
         return false;

      mTextureCallbackKey = TextureManager::registerEventCallback(terrainTextureEventCB, U32(this));

      mDynLightTexture = TextureHandle("special/lightFalloffMono", BitmapTexture, true);

		if (dglDoesSupportVertexBuffer())
			mVertexBuffer = glAllocateVertexBufferEXT(VertexBufferSize,GL_TRIBESMTVFMT_EXT,true);
		else
			mVertexBuffer = -1;
   }
   else
      mCRC = terr.getCRC();

   addToScene();

   if(!unpackEmptySquares())
      return(false);

   return true;
}

//--------------------------------------
void TerrainBlock::onRemove()
{
   delete mBlender;
   mBlender = NULL;

   removeFromScene();

	if (mVertexBuffer != -1)
	{
		if (dglDoesSupportVertexBuffer())
			glFreeVertexBufferEXT(mVertexBuffer);
		else
			AssertFatal(false,"Vertex buffer should have already been freed!");
		mVertexBuffer = -1;
	}

	TerrainRender::flushCache();
   TextureManager::unregisterEventCallback(mTextureCallbackKey);
   Parent::onRemove();
}


//--------------------------------------
void TerrainBlock::processTextureEvent(const U32 eventCode)
{
   if (eventCode == TextureManager::BeginZombification) {
      // Delete any textures we have built from the dmls...
      TerrainRender::flushCache();
      for(S32 i = 0; i < 16; i++)
         baseTextures[i] = NULL;
		if (mVertexBuffer != -1)
		{
			if (dglDoesSupportVertexBuffer())
				glFreeVertexBufferEXT(mVertexBuffer);
			else
				AssertFatal(false,"Vertex buffer should have already been freed!");
			mVertexBuffer = -1;
		}
   } else if (eventCode == TextureManager::CacheResurrected) {
      buildMipMap();
		if (dglDoesSupportVertexBuffer())
			mVertexBuffer = glAllocateVertexBufferEXT(VertexBufferSize,GL_TRIBESMTVFMT_EXT,true);
   }
}

//--------------------------------------------------------------------------
bool TerrainBlock::prepRenderImage(SceneState* state, const U32 stateKey,
                                   const U32 /*startZone*/, const bool /*modifyBaseState*/)
{
   if (isLastState(state, stateKey))
      return false;
   setLastState(state, stateKey);

   // This should be sufficient for most objects that don't manage zones, and
   //  don't need to return a specialized RenderImage...
   bool render = false;
   if (state->isTerrainOverridden() == false)
      render = state->isObjectRendered(this);
   else
      render = true;

   if (render == true) {
      SceneRenderImage* image = new SceneRenderImage;
      image->obj = this;
      image->sortType = SceneRenderImage::Terrain;
      state->insertRenderImage(image);
   }

   return false;
}

void TerrainBlock::buildChunkDeviance(S32 x, S32 y)
{
   mFile->buildChunkDeviance(x, y);
}

void TerrainBlock::buildGridMap()
{
   mFile->buildGridMap();
}

//------------------------------------------------------------------------------
void TerrainBlock::setTransform(const MatrixF & mat)
{
   Parent::setTransform(mat);

   // Since the terrain is a static object, it's render transform changes 1 to 1
   //  with it's collision transform
   setRenderTransform(mat);
}

void TerrainBlock::renderObject(SceneState* state, SceneRenderImage*)
{
   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on entry");

   RectI viewport;
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   dglGetViewport(&viewport);

   if (state->isTerrainOverridden() == false) {
      state->setupObjectProjection(this);
   } else {
      state->setupBaseProjection();
   }

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   dglMultMatrix(&mRenderObjToWorld);
   glScalef(mObjScale.x, mObjScale.y, mObjScale.z);

   TerrainRender::renderBlock(this, state);

   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();

   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   dglSetViewport(viewport);

   dglSetCanonicalState();
   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on exit");
}

//--------------------------------------
void TerrainBlock::initPersistFields()
{
   Parent::initPersistFields();
   addField("detailTexture",    TypeString, Offset(mDetailTextureName, TerrainBlock));
   addField("terrainFile",    TypeString, Offset(mTerrFileName, TerrainBlock));
   addField("squareSize",     TypeS32,    Offset(squareSize, TerrainBlock));
   addField("emptySquares",   TypeS32Vector, Offset(mEmptySquareRuns, TerrainBlock));
   removeField("position");
}

//--------------------------------------
U32 TerrainBlock::packUpdate(NetConnection *, U32 mask, BitStream *stream)
{
   if(stream->writeFlag(mask & InitMask))
   {
      stream->write(mCRC);
      stream->writeString(mTerrFileName);
      stream->writeString(mDetailTextureName);
      stream->write(squareSize);

      // write out the empty rle vector
      stream->write(mEmptySquareRuns.size());
      for(U32 i = 0; i < mEmptySquareRuns.size(); i++)   
         stream->write(mEmptySquareRuns[i]);
   }
   else // normal update
   {
      if(stream->writeFlag(mask & EmptyMask))
      {
         // write out the empty rle vector
         stream->write(mEmptySquareRuns.size());
         for(U32 i = 0; i < mEmptySquareRuns.size(); i++)   
            stream->write(mEmptySquareRuns[i]);
      }
   }

   return 0;
}

void TerrainBlock::unpackUpdate(NetConnection *, BitStream *stream)
{
   if(stream->readFlag())  // init
   {
      stream->read(&mCRC);
      mTerrFileName = stream->readSTString();
      mDetailTextureName = stream->readSTString();
      stream->read(&squareSize);
      
      // read in the empty rle
      U32 size;   
      stream->read(&size);
      mEmptySquareRuns.setSize(size);
      for(U32 i = 0; i < size; i++)
         stream->read(&mEmptySquareRuns[i]);
   }
   else // normal update
   {
      if(stream->readFlag())  // empty
      {
         // read in the empty rle
         U32 size;   
         stream->read(&size);
         mEmptySquareRuns.setSize(size);
         for(U32 i = 0; i < size; i++)
            stream->read(&mEmptySquareRuns[i]);

         //
         if(materialMap)
            unpackEmptySquares();
      }
   }
}

//--------------------------------------
static void cMakeTestTerrain(SimObject*, S32 argc, const char **argv)
{
   TerrainFile *file = new TerrainFile;
   S32 nMaterialTypes;
   argc -= 2;
   if (argc)
   {
      nMaterialTypes = argc;
      for (S32 i=0; i<TerrainBlock::MaterialGroups && i < argc; i++)
      {
         char filename[256];
         char *ext;
         dStrcpy(filename, argv[i+2]);
         ext = dStrrchr(filename, '.');
         if (ext)
            *ext = 0;
         file->mMaterialFileName[i] = StringTable->insert(filename);
      }
   }
   else
   {
      nMaterialTypes = 1;
      file->mMaterialFileName[0] = StringTable->insert("Default");
   }

   // create circular cone in the middle of the map:
   S32 i, j;
   for(i = 0; i < TerrainBlock::BlockSize; i++)
   {
      for(j = 0; j < TerrainBlock::BlockSize; j++)
      {
         S32 x = i & 0x7f;
         S32 y = j & 0x7f;
         
         F32 dist = mSqrt((64 - x) * (64 - x) + (64 - y) * (64 - y));
         dist /= 64.0f;
      
         if(dist > 1)
            dist = 1;

         U32 offset = i + (j << TerrainBlock::BlockShift);
         file->mHeightMap[offset] = (U16)((1 - dist) * (1 - dist) * 20000);
         file->mBaseMaterialMap[offset] = 0;
      }
   }
   
   char filename[256];
   char *ext;
   dSprintf(filename, sizeof(filename), "base/terrains/%s", argv[1]);
   ext = dStrrchr(filename, '.');
   if (!ext || dStricmp(ext, ".ter") != 0)
      dStrcat(filename, ".ter");
   file->save(filename);

   delete file;
}


//--------------------------------------
static bool cTerrain_save(SimObject *obj, S32, const char **argv)
{
   char filename[256];
   char *ext;
   dSprintf(filename, sizeof(filename), "base/terrains/%s", argv[2]);
   ext = dStrrchr(filename, '.');
   if (!ext || dStricmp(ext, ".ter") != 0)
      dStrcat(filename, ".ter");
   return static_cast<TerrainBlock*>(obj)->save(filename);
}   

static F32 cTerrainHeight(SimObject *, S32, const char **argv)
{
   Point2F pos;
   F32 height = 0.0f;
   dSscanf(argv[1],"%f %f",&pos.x,&pos.y);
   TerrainBlock * terrain = dynamic_cast<TerrainBlock*>(Sim::findObject("Terrain"));
   if(terrain)
      if(terrain->isServerObject())
      {
         Point3F offset;
         terrain->getTransform().getColumn(3, &offset);
         pos -= Point2F(offset.x, offset.y);
         terrain->getHeight(pos, &height);
      }
   return height;   
}

//--------------------------------------
void TerrainBlock::consoleInit()
{
   Con::addCommand("makeTestTerrain", cMakeTestTerrain, "makeTestTerrain(fileName, {dml1, dml2, ...dml8} );", 2, 10);
   Con::addCommand("TerrainBlock", "save", cTerrain_save, "TerrainBlock.save(fileName);", 3, 3);
   Con::addCommand("getTerrainHeight", cTerrainHeight, "getTerrainHeight(pos);", 2, 2);
}

void TerrainBlock::flushCache()
{
   TerrainRender::flushCache();

}

//--------------------------------------
TerrainFile::TerrainFile()
{
   for(S32 i=0; i < TerrainBlock::MaterialGroups; i++)
   {
      mMaterialFileName[i] = NULL;
      mMaterialAlphaMap[i] = NULL;
   }
}   

TerrainFile::~TerrainFile()
{
   for(S32 i=0; i < TerrainBlock::MaterialGroups; i++) {
      delete[] mMaterialAlphaMap[i];
      mMaterialAlphaMap[i] = NULL;
   }
}   


//--------------------------------------
bool TerrainFile::save(const char *filename)
{
   FileStream writeFile;
   if(!writeFile.open(filename, FileStream::Write))
      return(false);

   // write the VERSION and HeightField
   writeFile.write((U8)FILE_VERSION);
   for (S32 i=0; i < (TerrainBlock::BlockSize * TerrainBlock::BlockSize); i++)
      writeFile.write(mHeightMap[i]);

   // write the material group map, after merging the flags...
   TerrainBlock::Material * materialMap = (TerrainBlock::Material*)mMaterialMap;
   for (S32 j=0; j < (TerrainBlock::BlockSize * TerrainBlock::BlockSize); j++)
   {
      U8 val = mBaseMaterialMap[j];
      val |= materialMap[j].flags & TerrainBlock::Material::PersistMask;
      writeFile.write(val);
   }
   
   // write the MaterialList Info
   S32 k;
   for(k=0; k < TerrainBlock::MaterialGroups; k++)
      writeFile.writeString(mMaterialFileName[k]);

   for(k=0; k < TerrainBlock::MaterialGroups; k++) {
      if(mMaterialFileName[k] && mMaterialFileName[k][0]) {
         AssertFatal(mMaterialAlphaMap[k] != NULL, "Error, must have a material map here!");
         writeFile.write(TerrainBlock::BlockSize * TerrainBlock::BlockSize, mMaterialAlphaMap[k]);
      }
   }

   return (writeFile.getStatus() == FileStream::Ok);
}   

//--------------------------------------

void TerrainFile::heightDevLine(U32 p1x, U32 p1y, U32 p2x, U32 p2y, U32 pmx, U32 pmy, U16 *devPtr)
{
   S32 h1 = getHeight(p1x, p1y);
   S32 h2 = getHeight(p2x, p2y);
   S32 hm = getHeight(pmx, pmy);
   S32 dev = ((h2 + h1) >> 1) - hm;
   if(dev < 0)
      dev = -dev;
   if(dev > *devPtr)
      *devPtr = dev;
}

void TerrainFile::buildChunkDeviance(S32 x, S32 y)
{
   GridChunk &gc = *(mChunkMap + x + (y << TerrainBlock::ChunkShift));
   gc.emptyFlags = 0;
   U32 sx = x << TerrainBlock::ChunkDownShift;
   U32 sy = y << TerrainBlock::ChunkDownShift;
   
   gc.heightDeviance[0] = 0;

   heightDevLine(sx, sy, sx + 2, sy, sx + 1, sy, &gc.heightDeviance[0]);
   heightDevLine(sx + 2, sy, sx + 4, sy, sx + 3, sy, &gc.heightDeviance[0]);

   heightDevLine(sx, sy + 2, sx + 2, sy + 2, sx + 1, sy + 2, &gc.heightDeviance[0]);
   heightDevLine(sx + 2, sy + 2, sx + 4, sy + 2, sx + 3, sy + 2, &gc.heightDeviance[0]);

   heightDevLine(sx, sy + 4, sx + 2, sy + 4, sx + 1, sy + 4, &gc.heightDeviance[0]);
   heightDevLine(sx + 2, sy + 4, sx + 4, sy + 4, sx + 3, sy + 4, &gc.heightDeviance[0]);

   heightDevLine(sx, sy, sx, sy + 2, sx, sy + 1, &gc.heightDeviance[0]);
   heightDevLine(sx + 2, sy, sx + 2, sy + 2, sx + 2, sy + 1, &gc.heightDeviance[0]);
   heightDevLine(sx + 4, sy, sx + 4, sy + 2, sx + 4, sy + 1, &gc.heightDeviance[0]);

   heightDevLine(sx, sy + 2, sx, sy + 4, sx, sy + 3, &gc.heightDeviance[0]);
   heightDevLine(sx + 2, sy + 2, sx + 2, sy + 4, sx + 2, sy + 3, &gc.heightDeviance[0]);
   heightDevLine(sx + 4, sy + 2, sx + 4, sy + 4, sx + 4, sy + 3, &gc.heightDeviance[0]);

   gc.heightDeviance[1] = gc.heightDeviance[0];
   
   heightDevLine(sx, sy, sx + 2, sy + 2, sx + 1, sy + 1, &gc.heightDeviance[1]);
   heightDevLine(sx + 2, sy, sx, sy + 2, sx + 1, sy + 1, &gc.heightDeviance[1]);

   heightDevLine(sx + 2, sy, sx + 4, sy + 2, sx + 3, sy + 1, &gc.heightDeviance[1]);
   heightDevLine(sx + 2, sy + 2, sx + 4, sy, sx + 3, sy + 1, &gc.heightDeviance[1]);

   heightDevLine(sx, sy + 2, sx + 2, sy + 4, sx + 1, sy + 3, &gc.heightDeviance[1]);
   heightDevLine(sx + 2, sy + 4, sx, sy + 2, sx + 1, sy + 3, &gc.heightDeviance[1]);

   heightDevLine(sx + 2, sy + 2, sx + 4, sy + 4, sx + 3, sy + 3, &gc.heightDeviance[1]);
   heightDevLine(sx + 2, sy + 4, sx + 4, sy + 2, sx + 3, sy + 3, &gc.heightDeviance[1]);

   gc.heightDeviance[2] = gc.heightDeviance[1];

   heightDevLine(sx, sy, sx + 4, sy, sx + 2, sy, &gc.heightDeviance[2]);
   heightDevLine(sx, sy + 4, sx + 4, sy + 4, sx + 2, sy + 4, &gc.heightDeviance[2]);
   heightDevLine(sx, sy, sx, sy + 4, sx, sy + 2, &gc.heightDeviance[2]);
   heightDevLine(sx + 4, sy, sx + 4, sy + 4, sx + 4, sy + 2, &gc.heightDeviance[2]);

   for(U32 j = 0; j < 4; j++)
   {
      for(U32 i = 0; i < 4; i++)
      {
         TerrainBlock::Material *mat = getMaterial(sx + i, sy + j);
         if(mat->flags & TerrainBlock::Material::Empty)
            gc.emptyFlags |= (1 << (j * 4 + i));
      }
   }
}

void TerrainFile::buildGridMap()
{
   S32 y;
   for(y = 0; y < TerrainBlock::ChunkSquareWidth; y++)
      for(U32 x = 0; x < TerrainBlock::ChunkSquareWidth; x++)
         buildChunkDeviance(x, y);

   GridSquare * gs = mGridMapBase;
   S32 i;
   for(i = TerrainBlock::BlockShift; i >= 0; i--)
   {
      mGridMap[i] = gs;
      gs += 1 << (2 * (TerrainBlock::BlockShift - i));
   }
   for(i = TerrainBlock::BlockShift; i >= 0; i--)
   {
      S32 squareCount = 1 << (TerrainBlock::BlockShift - i);
      S32 squareSize = (TerrainBlock::BlockSize) / squareCount;
      
      for(S32 squareX = 0; squareX < squareCount; squareX++)
      {
         for(S32 squareY = 0; squareY < squareCount; squareY++)
         {
            U16 min = 0xFFFF;
            U16 max = 0;
            U16 mindev45 = 0;
            U16 mindev135 = 0;
            
            Point3F p1, p2, p3, p4;

            // determine max error for both possible splits.
            PlaneF pl1, pl2, pl3, pl4;

            p1.set(0, 0, getHeight(squareX * squareSize, squareY * squareSize));
            p2.set(0, squareSize, getHeight(squareX * squareSize, squareY * squareSize + squareSize));
            p3.set(squareSize, squareSize, getHeight(squareX * squareSize + squareSize, squareY * squareSize + squareSize));
            p4.set(squareSize, 0, getHeight(squareX * squareSize + squareSize, squareY * squareSize));

            // pl1, pl2 = split45, pl3, pl4 = split135
            pl1.set(p1, p2, p3);
            pl2.set(p1, p3, p4);
            pl3.set(p1, p2, p4);
            pl4.set(p2, p3, p4);
            bool parentSplit45 = false;
            GridSquare *parent = NULL;
            if(i < TerrainBlock::BlockShift)
            {
               GridSquare *parent;
               parent = findSquare(i+1, Point2I(squareX * squareSize, squareY * squareSize));
               parentSplit45 = parent->flags & GridSquare::Split45;
            }
            bool empty = true;
            bool hasEmpty = false;
            
            for(S32 sizeX = 0; sizeX <= squareSize; sizeX++)
            {
               for(S32 sizeY = 0; sizeY <= squareSize; sizeY++)
               {
                  S32 x = squareX * squareSize + sizeX;
                  S32 y = squareY * squareSize + sizeY;

                  if(sizeX != squareSize && sizeY != squareSize)
                  {
                     TerrainBlock::Material *mat = getMaterial(x, y);
                     if(!(mat->flags & TerrainBlock::Material::Empty))
                        empty = false;
                     else 
                        hasEmpty = true;
                  }
                  U16 ht = getHeight(x, y);

                  if(ht < min)
                     min = ht;
                  if(ht > max)
                     max = ht;
                  Point3F pt(sizeX, sizeY, ht);
                  U16 dev;

                  if(sizeX < sizeY)
                     dev = calcDev(pl1, pt);
                  else if(sizeX > sizeY)
                     dev = calcDev(pl2, pt);
                  else
                     dev = Umax(calcDev(pl1, pt), calcDev(pl2, pt));

                  if(dev > mindev45)
                     mindev45 = dev;
                     
                  if(sizeX + sizeY < squareSize)
                     dev = calcDev(pl3, pt);
                  else if(sizeX + sizeY > squareSize)
                     dev = calcDev(pl4, pt);
                  else
                     dev = Umax(calcDev(pl3, pt), calcDev(pl4, pt));
                  
                  if(dev > mindev135)
                     mindev135 = dev;
               }
            }
            GridSquare *sq = findSquare(i, Point2I(squareX * squareSize, squareY * squareSize));
            sq->minHeight = min;
            sq->maxHeight = max;
            
            sq->flags = empty ? GridSquare::Empty : 0;
            if(hasEmpty)
               sq->flags |= GridSquare::HasEmpty;
            
            bool shouldSplit45 = ((squareX ^ squareY) & 1) == 0;
            bool split45;
   
            //split45 = shouldSplit45;          
            if(i == 0)
               split45 = shouldSplit45;
            else if(i < 4 && shouldSplit45 == parentSplit45)
               split45 = shouldSplit45;
            else
               split45 = mindev45 < mindev135;
            
            //split45 = shouldSplit45;
            if(split45)
            {
               sq->flags |= GridSquare::Split45;
               sq->heightDeviance = mindev45;
            }
            else
               sq->heightDeviance = mindev135;
            if(parent)
               if(parent->heightDeviance < sq->heightDeviance)
                  parent->heightDeviance = sq->heightDeviance;
         }
      }
   }
   for (y = 0; y < TerrainBlock::BlockSize; y++)
   {
      for (S32 x=0; x < TerrainBlock::BlockSize; x++)
      {
         GridSquare *sq = findSquare(0, Point2I(x, y));
         S32 xpl = (x + 1) & TerrainBlock::BlockMask;
         S32 ypl = (y + 1) & TerrainBlock::BlockMask;
         for(U32 i = 0; i < TerrainBlock::MaterialGroups; i++)
         {
            if (mMaterialAlphaMap[i] != NULL) {
               U32 mapVal = (mMaterialAlphaMap[i][(y << TerrainBlock::BlockShift) + x]     +
                             mMaterialAlphaMap[i][(ypl << TerrainBlock::BlockShift) + x]   +
                             mMaterialAlphaMap[i][(ypl << TerrainBlock::BlockShift) + xpl] +
                             mMaterialAlphaMap[i][(y << TerrainBlock::BlockShift) + xpl]);
               if(mapVal)
                  sq->flags |= (GridSquare::MaterialStart << i);
            }
         }
      }
   }
   for (y = 0; y < TerrainBlock::BlockSize; y += 2)
   {
      for (S32 x=0; x < TerrainBlock::BlockSize; x += 2)
      {
         GridSquare *sq = findSquare(1, Point2I(x, y));
         GridSquare *s1 = findSquare(0, Point2I(x, y));
         GridSquare *s2 = findSquare(0, Point2I(x+1, y));
         GridSquare *s3 = findSquare(0, Point2I(x, y+1));
         GridSquare *s4 = findSquare(0, Point2I(x+1, y+1));
         sq->flags |= (s1->flags | s2->flags | s3->flags | s4->flags) & ~(GridSquare::MaterialStart -1);
      }
   }
   GridSquare *s = findSquare(1, Point2I(0, 0));
   U16 *dflags = mFlagMap;
   U16 *eflags = mFlagMap + TerrainBlock::FlagMapWidth * TerrainBlock::FlagMapWidth;

   for(;dflags != eflags;s++,dflags++)
      *dflags = s->flags;
   
}

//--------------------------------------
ResourceInstance *constructTerrainFile(Stream &stream)
{
   U8 version;
   stream.read(&version);
   if (version > TerrainFile::FILE_VERSION)
      return NULL;

   if (version != TerrainFile::FILE_VERSION) {
      Con::errorf(" ****************************************************");
      Con::errorf(" ****************************************************");
      Con::errorf(" ****************************************************");
      Con::errorf(" PLEASE RESAVE THE TERRAIN FILE FOR THIS MISSION!  THANKS!");
      Con::errorf(" ****************************************************");
      Con::errorf(" ****************************************************");
      Con::errorf(" ****************************************************");
   }

   TerrainFile* ret = new TerrainFile;
   // read the HeightField
   for (S32 i=0; i < (TerrainBlock::BlockSize * TerrainBlock::BlockSize); i++)
      stream.read(&ret->mHeightMap[i]);

   // read the material group map and flags...
   dMemset(ret->mMaterialMap, 0, sizeof(ret->mMaterialMap));
   TerrainBlock::Material * materialMap = (TerrainBlock::Material*)ret->mMaterialMap;

   AssertFatal(!(TerrainBlock::Material::PersistMask & TerrainFile::MATERIAL_GROUP_MASK),
               "Doh! We have flag clobberage...");

   for (S32 j=0; j < (TerrainBlock::BlockSize * TerrainBlock::BlockSize); j++)
   {
      U8 val;
      stream.read(&val);

      //
      ret->mBaseMaterialMap[j] = val & TerrainFile::MATERIAL_GROUP_MASK;
      materialMap[j].flags = val & TerrainBlock::Material::PersistMask;
   }

   // read the MaterialList Info
   for(S32 k=0; k < TerrainBlock::MaterialGroups; k++)
      ret->mMaterialFileName[k] = stream.readSTString();

   if(version == 1)
   {
      for(S32 j = 0; j < (TerrainBlock::BlockSize * TerrainBlock::BlockSize); j++) {
         if (ret->mMaterialAlphaMap[ret->mBaseMaterialMap[j]] == NULL) {
            ret->mMaterialAlphaMap[ret->mBaseMaterialMap[j]] = new U8[TerrainBlock::BlockSize * TerrainBlock::BlockSize];
            dMemset(ret->mMaterialAlphaMap[ret->mBaseMaterialMap[j]], 0, TerrainBlock::BlockSize * TerrainBlock::BlockSize);
         }

         ret->mMaterialAlphaMap[ret->mBaseMaterialMap[j]][j] = 255;
      }
   }
   else
   {
      for(S32 k=0; k < TerrainBlock::MaterialGroups; k++) {
         if(ret->mMaterialFileName[k] && ret->mMaterialFileName[k][0]) {
            AssertFatal(ret->mMaterialAlphaMap[k] == NULL, "Bad assumption.  There should be no alpha map at this point...");
            ret->mMaterialAlphaMap[k] = new U8[TerrainBlock::BlockSize * TerrainBlock::BlockSize];
            stream.read(TerrainBlock::BlockSize * TerrainBlock::BlockSize, ret->mMaterialAlphaMap[k]);
         }
      }
   }

   ret->buildGridMap();
   return ret;
}

void TerrainBlock::setBaseMaterial(U32 /*x*/, U32 /*y*/, U8 /*matGroup*/)
{

}

