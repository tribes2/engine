//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "platform/platformAssert.h"
#include "platformWIN32/platformGL.h"
#include "platform/platform.h"
#include "core/tVector.h"
#include "core/resManager.h"
#include "dgl/gBitmap.h"
#include "dgl/gPalette.h"
#include "dgl/gTexManager.h"
#include "console/console.h"
#include "console/consoleInternal.h"
#include "console/consoleTypes.h"
#include "dgl/gChunkedTexManager.h"

//------------------------------------------------------------------------------

bool gDGLRender = true;

bool sgResurrect = false;
bool sgForcePalettedTexture = false;
bool sgForce16BitTexture    = false;


#define  ENABLE_HOLDING    1

#ifdef GATHER_METRICS
U32 TextureManager::smTextureSpaceLoaded = 0;
U32 TextureManager::smTextureCacheMisses = 0;
#endif

bool TextureManager::smUseSmallTextures = false;

bool TextureManager::smIsZombie = false;

//--------------------------------------------------------------------------
//-------------------------------------- Texture detailing control variables
//                                        0: Highest
//                                        1: ...
//                                        2: ...
//                                        3: Lowest

namespace {

struct Forced16BitMapping
{
   GLenum wanted;
   GLenum forced;
   bool   end;
};

Forced16BitMapping sg16BitMappings[] =
{
   { GL_RGB,  GL_RGB5,  false },
   { GL_RGBA, GL_RGBA4, false },
   { 0, 0, true }
};


U32    sgTextureDetailLevel         = 0;
U32    sgSkyTextureDetailLevel      = 0;
U32    sgInteriorTextureDetailLevel = 0;
bool   sgAllowTexCompression  = false;
GLenum sgCompressionHint      = GL_FASTEST;
F32    sgTextureAnisotropy    = 0.0;
bool   sgDisableSubImage      = false;
// valid texture extensions
#define EXT_ARRAY_SIZE 6
static const char* extArray[EXT_ARRAY_SIZE] = { "", ".jpg", ".png", ".gif", ".bmp", "" };
static const char* extArray_8[EXT_ARRAY_SIZE] = { "", ".bm8", ".bmp", ".jpg", ".png", ".gif" };

ConsoleFunction(setOpenGLMipReduction, void, 2, 2, "setOpenGLMipReduction(0-5);")
{
   argc;
   S32 val = dAtoi(argv[1]);
   if (val < 0)
      val = 0;
   else if (val > 5)
      val = 5;

   sgTextureDetailLevel = val;
}

ConsoleFunction(setOpenGLSkyMipReduction, void, 2, 2, "setOpenGLSkyMipReduction(0-5);")
{
   argc;
   S32 val = dAtoi(argv[1]);
   if (val < 0)
      val = 0;
   else if (val > 5)
      val = 5;

   sgSkyTextureDetailLevel = val;
}

ConsoleFunction(setOpenGLInteriorMipReduction, void, 2, 2, "setOpenGLInteriorMipReduction(0-5);")
{
   argc;
   S32 val = dAtoi(argv[1]);
   if (val < 0)
      val = 0;
   else if (val > 5)
      val = 5;

   sgInteriorTextureDetailLevel = val;
}

ConsoleFunction(setOpenGLTextureCompressionHint, void, 2, 2, "setTextureCompressionHint(GL_DONT_CARE|GL_FASTEST|GL_NICEST);")
{
   argc;

   GLenum newHint        = GL_DONT_CARE;
   const char* newString = "GL_DONT_CARE";

   if (dStricmp(argv[1], "GL_FASTEST") == 0) {
      newHint = GL_FASTEST;
      newString = "GL_FASTEST";
   } else if (dStricmp(argv[1], "GL_NICEST") == 0) {
      GLenum newHint = GL_NICEST;
      newString = "GL_NICEST";
   }

   sgCompressionHint = newHint;

   if (dglDoesSupportTextureCompression())
      glHint(GL_TEXTURE_COMPRESSION_HINT_ARB, sgCompressionHint);
}

ConsoleFunction(setOpenGLAnisotropy, void, 2, 2, "setOpenGLAnisotropy(0-1);")
{
   argc;
   F32 val = dAtof(argv[1]);
   if (val < 0.0)
      val = 0.0;
   if (val > 1.0)
      val = 1.0;
   sgTextureAnisotropy = val;
}

} // namespace {}


//--------------------------------------
struct TextureDictionary
{
   static TextureObject **smTable;
   static TextureObject *smTOList;
   static U32 smHashTableSize;
   
   static void create();
   static void preDestroy();
   static void destroy();
   
   static void insert(TextureObject *object);
   static TextureObject *find(StringTableEntry name, TextureHandleType type, bool clamp);
   static void remove(TextureObject *object);
   static S32 clearHolds();
};

TextureObject **TextureDictionary::smTable = NULL;
TextureObject *TextureDictionary::smTOList = NULL;
U32 TextureDictionary::smHashTableSize = 0;

//--------------------------------------
void TextureDictionary::create()
{
   smTOList = NULL;
   smHashTableSize = 1023;
   smTable = new TextureObject *[smHashTableSize];
   for(U32 i = 0; i < smHashTableSize; i++)
      smTable[i] = NULL;

   //Con::addVariable("$pref::OpenGL::mipReduction", texDetailLevelCB, "0");
   //Con::addVariable("$pref::OpenGL::anisotropy", anisotropyCB, "0");

   Con::addVariable("$pref::OpenGL::force16BitTexture",    TypeBool, &sgForce16BitTexture);
   Con::addVariable("$pref::OpenGL::forcePalettedTexture", TypeBool, &sgForcePalettedTexture);
   Con::addVariable("$pref::OpenGL::allowCompression",     TypeBool, &sgAllowTexCompression);
   Con::addVariable("$pref::OpenGL::disableSubImage",      TypeBool, &sgDisableSubImage);
}


//--------------------------------------
TextureObject *TextureDictionary::find(StringTableEntry name, TextureHandleType type, bool clamp)
{
   U32 key = HashPointer(name) % smHashTableSize;
   TextureObject *walk = smTable[key];
   for(; walk; walk = walk->hashNext)
      if(walk->texFileName == name && walk->type == type && walk->clamp == clamp)
         break;
   return walk;
}


//--------------------------------------
void TextureDictionary::remove(TextureObject *object)
{
   if(object->next)
      object->next->prev = object->prev;
   
   if(object->prev)
      object->prev->next = object->next;
   else
      smTOList = object->next;

   if(!object->texFileName)
      return;
      
   U32 key = HashPointer(object->texFileName) % smHashTableSize;
   TextureObject **walk = &smTable[key];
   while(*walk)
   {
      if(*walk == object)
      {
         *walk = object->hashNext;
         break;
      }
      walk = &((*walk)->hashNext);
   }
}


//--------------------------------------
void TextureDictionary::insert(TextureObject *object)
{
   object->next = smTOList;
   object->prev = NULL;
   if(smTOList)
      smTOList->prev = object;
   smTOList = object;
   
   if(object->texFileName)
   {
      U32 key = HashPointer(object->texFileName) % smHashTableSize;
   
      object->hashNext = smTable[key];
      smTable[key] = object;
   }
}

//--------------------------------------
void TextureDictionary::preDestroy()
{
   // This is a horrid hack, but it will have to do for now.  (DMM, aided
   //  and abetted by MF.)
   TextureObject* walk = smTOList;
   while (walk)
   {
      if((gDGLRender || sgResurrect) && walk->texGLName)
         glDeleteTextures(1, (const GLuint*)&walk->texGLName);
      if((gDGLRender || sgResurrect) && walk->smallTexGLName)
         glDeleteTextures(1, (const GLuint*)&walk->smallTexGLName);
      delete walk->bitmap;
      walk->texGLName = 0;
      walk->smallTexGLName = 0;
      walk->bitmap = NULL;

      walk = walk->next;
   }
}

//--------------------------------------
void TextureDictionary::destroy()
{
   // This is a horrid hack, but it will have to do for now.  (DMM, aided
   //  and abetted by MF.)
   while(smTOList)
      TextureManager::freeTexture(smTOList);
   delete[] smTable;
}

//--------------------------------------
S32 TextureDictionary::clearHolds()
{
   Vector<TextureObject *>    holds;
   
   // Find held textures to delete.  Clear holding flag too so they're free
   // to go away.  
   for (TextureObject * walk = smTOList; walk; walk = walk->next)
      if (walk->holding) 
         if (!walk->refCount)
            holds.push_back(walk);
         else
            walk->holding = false;
   
   // Remove them- 
   for (S32 i = 0; i < holds.size(); i++)
      TextureManager::freeTexture(holds[i]);
      
   return holds.size();
}

namespace {
ConsoleFunction(clearTextureHolds, S32, 1, 1, "clearTextureHolds();")
{
   argc; argv;
   return TextureDictionary::clearHolds();
}
}//namespace


//--------------------------------------------------------------------------
//--------------------------------------
//
struct EventCallbackEntry
{
   TextureEventCallback callback;
   U32                  userData;
   U32                  key;
};
static U32                        sgCurrCallbackKey = 0;
static Vector<EventCallbackEntry> sgEventCallbacks(__FILE__, __LINE__);

U32  TextureManager::registerEventCallback(TextureEventCallback callback, const U32 userData)
{
   sgEventCallbacks.increment();
   sgEventCallbacks.last().callback = callback;
   sgEventCallbacks.last().userData = userData;
   sgEventCallbacks.last().key      = sgCurrCallbackKey++;

   return sgEventCallbacks.last().key;
}

void TextureManager::unregisterEventCallback(const U32 callbackKey)
{
   for (U32 i = 0; i < sgEventCallbacks.size(); i++)
      if (sgEventCallbacks[i].key == callbackKey) {
         sgEventCallbacks.erase(i);
         return;
      }
}

void TextureManager::postTextureEvent(const U32 eventCode)
{
   for (U32 i = 0; i < sgEventCallbacks.size(); i++)
      (sgEventCallbacks[i].callback)(eventCode, sgEventCallbacks[i].userData);
}


void TextureManager::create()
{
   TextureDictionary::create();
}    

void TextureManager::preDestroy()
{
   TextureDictionary::preDestroy();
}

void TextureManager::destroy()
{
   TextureDictionary::destroy();

   AssertFatal(sgEventCallbacks.size() == 0,
               "Error, some object didn't unregister it's texture event callback function!");
}


//--------------------------------------
void TextureManager::makeZombie()
{
   if (smIsZombie == true)
      return;
   smIsZombie = true;
   
   postTextureEvent(BeginZombification);
   ChunkedTextureManager::makeZombie();
   // Publish flush event?

   Vector<GLuint> deleteNames(4096);

   TextureObject* probe = TextureDictionary::smTOList;
   while (probe) {
      AssertFatal(probe->type != TerrainTexture, "Error, all the terrain textureobjects should be gone by now!");
      if (probe->type == BitmapNoDownloadTexture)
      {
         probe = probe->next;
         continue;
      }
      if (probe->texGLName != 0)
         deleteNames.push_back(probe->texGLName);
      if (probe->smallTexGLName != 0)
         deleteNames.push_back(probe->smallTexGLName);

#ifdef GATHER_METRICS
      AssertFatal(probe->textureSpace <= smTextureSpaceLoaded, "Error, that shouldn't happen!");
      smTextureSpaceLoaded -= probe->textureSpace;
      probe->textureSpace   = 0;
#endif

      probe->texGLName      = 0;
      probe->smallTexGLName = 0;
      
      probe = probe->next;
   }

   glDeleteTextures(deleteNames.size(), deleteNames.address());
}

void TextureManager::resurrect()
{
   if (smIsZombie == false)
      return;
   smIsZombie = false;
   
   sgResurrect = true;

   TextureObject* probe = TextureDictionary::smTOList;

   while (probe) {
      // reload texture...
      AssertFatal(probe->type != TerrainTexture, "Error, all the terrain textureobjects should be gone by now!");
      if (probe->type == BitmapNoDownloadTexture)
      {
         probe = probe->next;
         continue;
      }
      if (probe->bitmap != NULL) {
         if(probe->type == BitmapKeepTexture)
         {
            delete probe->bitmap;
            probe->bitmap = NULL;
         }
         else
         {
            if (probe->type == RegisteredTexture) {
               createGLName(probe->bitmap, probe->clamp, 0, probe->type, probe);
            } else {
               TextureObject* refreshed = registerTexture(probe->texFileName, probe->bitmap,
                                                          probe->type, probe->clamp);
               AssertFatal(refreshed == probe, "Error, new texture object returned.  This should not happen in resurrect");
            }
            probe = probe->next;
            continue;
         }
      }

      // Ok, what we have here is the object, with the right name, we need to load the
      //  bitmap, and register the texture
      GBitmap *bmp = NULL;
      for (U32 i = 0; i < EXT_ARRAY_SIZE && bmp == NULL; i++) {
         char extendedBuffer[260];
         dStrcpy(extendedBuffer, probe->texFileName);

         if (sgForcePalettedTexture == true && dglDoesSupportPalettedTexture())
            dStrcat(extendedBuffer, extArray_8[i]);
         else
            dStrcat(extendedBuffer, extArray[i]);

         bmp = (GBitmap*)ResourceManager->loadInstance(extendedBuffer);
      }
      AssertISV(bmp != NULL, "Error resurrecting the texture cache.\n"
                "Possible cause: a bitmap was deleted during the course of gameplay.");

      TextureObject* refreshed = registerTexture(probe->texFileName, bmp,
                                                 probe->type, probe->clamp);
      AssertFatal(refreshed == probe, "Error, new texture object returned.  This should not happen in resurrect");

      probe = probe->next;          
   }

   ChunkedTextureManager::resurrect();
   postTextureEvent(CacheResurrected);

   sgResurrect = false;
}

void TextureManager::flush()
{
   makeZombie();
   resurrect();
}


#ifdef GATHER_METRICS
void TextureManager::dumpStats()
{
   TextureObject* probe = TextureDictionary::smTOList;

   Con::errorf("aaa Texture dump");
   while (probe)
   {
      Con::errorf("aaa    %d: (%d, %s) %d (%s)", probe->type, probe->refCount, probe->holding ? "yes" : "no", probe->textureSpace, probe->texFileName ? probe->texFileName : "nil");
      probe = probe->next;
   }
}
#endif


//------------------------------------------------------------------------------
GBitmap* TextureManager::createPaddedBitmap(GBitmap* pBitmap)
{
   if (isPow2(pBitmap->getWidth()) && isPow2(pBitmap->getHeight()))
      return pBitmap;

   AssertFatal(pBitmap->getNumMipLevels() == 1,
               "Cannot have non-pow2 bitmap with miplevels");

   U32 newWidth  = getNextPow2(pBitmap->getWidth());
   U32 newHeight = getNextPow2(pBitmap->getHeight());

   GBitmap* pReturn = new GBitmap(newWidth, newHeight, false, pBitmap->getFormat());

   for (U32 i = 0; i < pBitmap->getHeight(); i++) {
      U8*       pDest = (U8*)pReturn->getAddress(0, i);
      const U8* pSrc  = (const U8*)pBitmap->getAddress(0, i);

      dMemcpy(pDest, pSrc, pBitmap->getWidth() * pBitmap->bytesPerPixel);
   }
   if (pBitmap->getFormat() == GBitmap::Palettized)
   {
      pReturn->pPalette = new GPalette;
      dMemcpy(pReturn->pPalette->getColors(), pBitmap->pPalette->getColors(), sizeof(ColorI) * 256);
      pReturn->pPalette->setPaletteType(pBitmap->pPalette->getPaletteType());
   }
   return pReturn;
}


//------------------------------------------------------------------------------
GBitmap* TextureManager::createMipBitmap(const GBitmap* pBitmap)
{
   AssertFatal(pBitmap != NULL, "Error, no bitmap");
   AssertFatal(pBitmap->getNumMipLevels() != 1, "Error, no mips to maintain");

   GBitmap* pRetBitmap = new GBitmap(pBitmap->getWidth(1),
                                     pBitmap->getHeight(1),
                                     true,
                                     pBitmap->getFormat());

   for (U32 i = 1; i < pBitmap->getNumMipLevels(); i++) {
      void* pDest      = pRetBitmap->getWritableBits(i - 1);
      const void* pSrc = pBitmap->getBits(i);

      dMemcpy(pDest, pSrc, (pBitmap->getWidth(i)  *
                            pBitmap->getHeight(i) *
                            pBitmap->bytesPerPixel));
   }

   return pRetBitmap;
}


//------------------------------------------------------------------------------
void TextureManager::freeTexture(TextureObject *to)
{
#ifdef GATHER_METRICS
   AssertFatal(to->textureSpace <= smTextureSpaceLoaded, "Error, that shouldn't happen!");
   smTextureSpaceLoaded -= to->textureSpace;
#endif

   if((gDGLRender || sgResurrect) && to->texGLName)
      glDeleteTextures(1, (const GLuint*)&to->texGLName);
   if((gDGLRender || sgResurrect) && to->smallTexGLName)
      glDeleteTextures(1, (const GLuint*)&to->smallTexGLName);

   delete to->bitmap;
   TextureDictionary::remove(to);
   delete to;
}


//------------------------------------------------------------------------------
static void getSourceDestByteFormat(GBitmap *pBitmap, U32 *sourceFormat, U32 *destFormat, U32 *byteFormat)
{
   *byteFormat = GL_UNSIGNED_BYTE;
   
   switch(pBitmap->getFormat()) {
      case GBitmap::Intensity:
         *sourceFormat = GL_INTENSITY;
         break;

      case GBitmap::Palettized:
         *sourceFormat = GL_COLOR_INDEX;
         break;

      case GBitmap::Luminance:
         *sourceFormat = GL_LUMINANCE;
         break;
      case GBitmap::RGB:
         *sourceFormat = GL_RGB;
         break;
      case GBitmap::RGBA:
         *sourceFormat = GL_RGBA;
         break;
      case GBitmap::Alpha:
         *sourceFormat = GL_ALPHA;
         break;
      case GBitmap::RGB565:
      case GBitmap::RGB5551:
         *sourceFormat = GL_RGBA;
         *byteFormat   = 0x8034;
         break;
   };
   
   if(*byteFormat == GL_UNSIGNED_BYTE)
   {
      if (*sourceFormat != GL_COLOR_INDEX)
         *destFormat = *sourceFormat;
      else
         *destFormat = GL_COLOR_INDEX8_EXT;
      
      if (pBitmap->getNumMipLevels() > 1 &&
          pBitmap->getFormat() != GBitmap::Palettized &&
          (sgAllowTexCompression && dglDoesSupportTextureCompression()))
      {
         if (*sourceFormat == GL_RGB)
            *destFormat = GL_COMPRESSED_RGB_ARB;
         else if (*sourceFormat == GL_RGBA)
            *destFormat = GL_COMPRESSED_RGBA_ARB;
      }
   } else
   {
      *destFormat = GL_RGB5_A1;
   }

   if (sgForce16BitTexture)
   {
      for (U32 i = 0; sg16BitMappings[i].end != true; i++)
      {
         if (*destFormat == sg16BitMappings[i].wanted)
         {
            *destFormat = sg16BitMappings[i].forced;
            return;
         }
      }
   }
}


//--------------------------------------
void TextureManager::refresh(TextureObject *to)
{
   if (!(gDGLRender || sgResurrect))
      return;

   U32 sourceFormat, destFormat, byteFormat;
   GBitmap *pBitmap = to->bitmap;

   getSourceDestByteFormat(pBitmap, &sourceFormat, &destFormat, &byteFormat);

   if (!to->texGLName)
      glGenTextures(1,&to->texGLName);

   glBindTexture(GL_TEXTURE_2D, to->texGLName);
   GBitmap *pDL = createPaddedBitmap(pBitmap);
   
   U32 maxDownloadMip = pDL->getNumMipLevels();
   if (to->type == BitmapTexture ||
       to->type == BitmapKeepTexture ||
       to->type == BitmapNoDownloadTexture)
   {
      maxDownloadMip = 1;
   }

   if (pDL->getFormat() == GBitmap::Palettized)
   {
      glColorTableEXT(GL_TEXTURE_2D,
                      pDL->getPalette()->getPaletteType() == GPalette::RGB ? GL_RGB : GL_RGBA,
                      256,
                      GL_RGBA,
                      GL_UNSIGNED_BYTE,
                      pDL->getPalette()->getColors());
   }
   if (sgDisableSubImage)
   {
      for (U32 i = 0; i < maxDownloadMip; i++)
      {
         glTexImage2D(GL_TEXTURE_2D,
                      i,
                      destFormat,
                      pDL->getWidth(i), pDL->getHeight(i),
                      0,
                      sourceFormat,
                      byteFormat,
                      pDL->getBits(i));
      }
   }
   else
   {
      for (U32 i = 0; i < maxDownloadMip; i++)
      {
         glTexSubImage2D(GL_TEXTURE_2D,
                         i,
                         0, 0,
                         pDL->getWidth(i), pDL->getHeight(i),
                         sourceFormat,
                         byteFormat,
                         pDL->getBits(i));
      }
   }

   if ((to->type == InteriorTexture || to->type == MeshTexture) &&
       pDL->getNumMipLevels() > 4)
   {
      // 
      if (!to->smallTexGLName)
         glGenTextures(1,&to->smallTexGLName);
      
      glBindTexture(GL_TEXTURE_2D, to->smallTexGLName);
      if (pDL->getFormat() == GBitmap::Palettized)
      {
         glColorTableEXT(GL_TEXTURE_2D,
                         pDL->getPalette()->getPaletteType() == GPalette::RGB ? GL_RGB : GL_RGBA,
                         256,
                         GL_RGBA,
                         GL_UNSIGNED_BYTE,
                         pDL->getPalette()->getColors());
      }
      if (sgDisableSubImage)
      {
         for (U32 i = 4; i < maxDownloadMip; i++)
         {
            glTexImage2D(GL_TEXTURE_2D,
                         i - 4,
                         destFormat,
                         pDL->getWidth(i), pDL->getHeight(i),
                         0,
                         sourceFormat,
                         byteFormat,
                         pDL->getBits(i));
         }
      }
      else
      {
         for (U32 i = 4; i < maxDownloadMip; i++)
         {
            glTexSubImage2D(GL_TEXTURE_2D,
                            i - 4,
                            0, 0,
                            pDL->getWidth(i), pDL->getHeight(i),
                            sourceFormat,
                            byteFormat,
                            pDL->getBits(i));
         }
      }
   }
   else
   {
      if (to->smallTexGLName != 0)
         glDeleteTextures(1, &to->smallTexGLName);
      to->smallTexGLName = 0;
   }
   
   if(pDL != pBitmap)
      delete pDL;
}

void TextureManager::refresh(TextureObject *to, GBitmap* bmp)
{
   if (!(gDGLRender || sgResurrect)) return;

   U32 sourceFormat, destFormat, byteFormat;
   GBitmap* pBitmap = bmp;

   getSourceDestByteFormat(pBitmap, &sourceFormat, &destFormat, &byteFormat);

   if (!to->texGLName)
      glGenTextures(1,&to->texGLName);

   glBindTexture(GL_TEXTURE_2D, to->texGLName);
   GBitmap* pDL = createPaddedBitmap(pBitmap);

   U32 maxDownloadMip = pDL->getNumMipLevels();
   if (to->type == BitmapTexture ||
       to->type == BitmapKeepTexture ||
       to->type == BitmapNoDownloadTexture)
   {
      maxDownloadMip = 1;
   }

   if (pDL->getFormat() == GBitmap::Palettized)
   {
      glColorTableEXT(GL_TEXTURE_2D,
                      pDL->getPalette()->getPaletteType() == GPalette::RGB ? GL_RGB : GL_RGBA,
                      256,
                      GL_RGBA,
                      GL_UNSIGNED_BYTE,
                      pDL->getPalette()->getColors());
   }
   if (sgDisableSubImage)
   {
      for (U32 i = 0; i < maxDownloadMip; i++)
      {
         glTexImage2D(GL_TEXTURE_2D,
                      i,
                      destFormat,
                      pDL->getWidth(i), pDL->getHeight(i),
                      0,
                      sourceFormat,
                      byteFormat,
                      pDL->getBits(i));
      }
   }
   else
   {
      for (U32 i = 0; i < maxDownloadMip; i++)
      {
         glTexSubImage2D(GL_TEXTURE_2D,
                         i,
                         0, 0,
                         pDL->getWidth(i), pDL->getHeight(i),
                         sourceFormat,
                         byteFormat,
                         pDL->getBits(i));
      }
   }
   
   if ((to->type == InteriorTexture || to->type == MeshTexture) &&
       pDL->getNumMipLevels() > 4)
   {
      // 
      if (!to->smallTexGLName)
         glGenTextures(1,&to->smallTexGLName);
      
      glBindTexture(GL_TEXTURE_2D, to->smallTexGLName);
      glBindTexture(GL_TEXTURE_2D, to->smallTexGLName);
      if (pDL->getFormat() == GBitmap::Palettized)
      {
         glColorTableEXT(GL_TEXTURE_2D,
                         pDL->getPalette()->getPaletteType() == GPalette::RGB ? GL_RGB : GL_RGBA,
                         256,
                         GL_RGBA,
                         GL_UNSIGNED_BYTE,
                         pDL->getPalette()->getColors());
      }
      if (sgDisableSubImage)
      {
         for (U32 i = 4; i < maxDownloadMip; i++)
         {
            glTexImage2D(GL_TEXTURE_2D,
                         i - 4,
                         destFormat,
                         pDL->getWidth(i), pDL->getHeight(i),
                         0,
                         sourceFormat,
                         byteFormat,
                         pDL->getBits(i));
         }
      }
      else
      {
         for (U32 i = 4; i < maxDownloadMip; i++)
         {
            glTexSubImage2D(GL_TEXTURE_2D,
                            i - 4,
                            0, 0,
                            pDL->getWidth(i), pDL->getHeight(i),
                            sourceFormat,
                            byteFormat,
                            pDL->getBits(i));
         }
      }
   }
   else
   {
      if (to->smallTexGLName != 0)
         glDeleteTextures(1, &to->smallTexGLName);
      to->smallTexGLName = 0;
   }

   if(pDL != pBitmap)
      delete pDL;
}

//--------------------------------------
bool TextureManager::createGLName(GBitmap*          pBitmap,
                                  bool              clampToEdge,
                                  U32               firstMip,
                                  TextureHandleType type,
                                  TextureObject*    to)
{
   if (!(gDGLRender || sgResurrect))
      return 0;

   glGenTextures(1, &to->texGLName);
   glBindTexture(GL_TEXTURE_2D, to->texGLName);

   U32 sourceFormat, destFormat, byteFormat;

   getSourceDestByteFormat(pBitmap, &sourceFormat, &destFormat, &byteFormat);
 
   GBitmap *pDL = createPaddedBitmap(pBitmap);
   
   U32 maxDownloadMip = pDL->getNumMipLevels();
   if (type == BitmapTexture ||
       type == BitmapKeepTexture ||
       type == BitmapNoDownloadTexture)
   {
      maxDownloadMip = firstMip + 1;
   }

   if (pDL->getFormat() == GBitmap::Palettized)
   {
      glColorTableEXT(GL_TEXTURE_2D,
                      pDL->getPalette()->getPaletteType() == GPalette::RGB ? GL_RGB : GL_RGBA,
                      256,
                      GL_RGBA,
                      GL_UNSIGNED_BYTE,
                      pDL->getPalette()->getColors());
   }
   for (U32 i = firstMip; i < maxDownloadMip; i++)
   {
      glTexImage2D(GL_TEXTURE_2D,
                   i - firstMip,
                   destFormat,
                   pDL->getWidth(i), pDL->getHeight(i),
                   0,
                   sourceFormat,
                   byteFormat,
                   pDL->getBits(i));
   }
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   if(pBitmap->getNumMipLevels() != 1 &&
      type != BitmapTexture &&
      type != BitmapKeepTexture &&
      type != BitmapNoDownloadTexture) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
      if (dglDoesSupportTexAnisotropy()) {
         F32 val = 1.0 + sgTextureAnisotropy * dglGetMaxAnisotropy();
         glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, val);
      }
   } else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   }

   U32 clamp = GL_REPEAT;
   if (clampToEdge)
      clamp = dglDoesSupportEdgeClamp() ? GL_CLAMP_TO_EDGE_EXT : GL_CLAMP;
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp);

   
   if ((type == InteriorTexture || type == MeshTexture) &&
       (pDL->getNumMipLevels() - firstMip) > 4)
   {
      glGenTextures(1, &to->smallTexGLName);
      glBindTexture(GL_TEXTURE_2D, to->smallTexGLName);
      
      if (pDL->getFormat() == GBitmap::Palettized)
      {
         glColorTableEXT(GL_TEXTURE_2D,
                         pDL->getPalette()->getPaletteType() == GPalette::RGB ? GL_RGB : GL_RGBA,
                         256,
                         GL_RGBA,
                         GL_UNSIGNED_BYTE,
                         pDL->getPalette()->getColors());
      }
      for (U32 i = firstMip + 4; i < maxDownloadMip; i++)
      {
         glTexImage2D(GL_TEXTURE_2D,
                      i - (firstMip + 4),
                      destFormat,
                      pDL->getWidth(i), pDL->getHeight(i),
                      0,
                      sourceFormat,
                      byteFormat,
                      pDL->getBits(i));
      }
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
      if (dglDoesSupportTexAnisotropy()) {
         F32 val = 1.0 + sgTextureAnisotropy * dglGetMaxAnisotropy();
         glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, val);
      }

      U32 clamp = GL_REPEAT;
      if (clampToEdge)
         clamp = dglDoesSupportEdgeClamp() ? GL_CLAMP_TO_EDGE_EXT : GL_CLAMP;
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp);
   }

   if(pDL != pBitmap)
      delete pDL;

   return to->texGLName != 0;
}  


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
TextureObject* TextureManager::registerTexture(const char* textureName, const GBitmap* data, bool clampToEdge)
{
   // if there is no textureName, it isn't inserted into the hash
   // table... merely tracked by the texture manager
   
   TextureObject *ret = NULL;
   if(textureName)
   {
      textureName = StringTable->insert(textureName);
      ret         = TextureDictionary::find(textureName, RegisteredTexture, clampToEdge);
   }
   if(ret)
   {
      // Crucial conditionals for the flush case...
      if (ret->bitmap != data)
         delete ret->bitmap;
      if (ret->texGLName)
         glDeleteTextures(1, (const GLuint*)&ret->texGLName);
      if (ret->smallTexGLName)
         glDeleteTextures(1, (const GLuint*)&ret->smallTexGLName);
      ret->texGLName      = 0;
      ret->smallTexGLName = 0;

#ifdef GATHER_METRICS
      AssertFatal(ret->textureSpace <= smTextureSpaceLoaded, "Error, that shouldn't happen!");
      smTextureSpaceLoaded -= ret->textureSpace;
      ret->textureSpace     = 0;
#endif

   }
   else
   {
      ret = new TextureObject;
      ret->texFileName    = textureName;
      ret->texGLName      = 0;
      ret->smallTexGLName = 0;
      ret->refCount       = 0;
      ret->type = RegisteredTexture;
      ret->holding        = false;
   
      TextureDictionary::insert(ret);
   }
   ret->bitmap           = (GBitmap *) data;
   ret->bitmapWidth      = data->getWidth();
   ret->bitmapHeight     = data->getHeight();
   ret->texWidth         = getNextPow2(ret->bitmapWidth);
   ret->texHeight        = getNextPow2(ret->bitmapHeight);
   ret->downloadedWidth  = ret->texWidth;
   ret->downloadedHeight = ret->texHeight;
   ret->clamp            = clampToEdge;

#ifdef GATHER_METRICS
   ret->textureSpace     = ret->downloadedWidth * ret->downloadedHeight;
   smTextureSpaceLoaded += ret->textureSpace;
#endif

   createGLName(ret->bitmap, clampToEdge, 0, ret->type, ret);

   return ret;
}


//--------------------------------------
TextureObject* TextureManager::registerTexture(const char* textureName, GBitmap* bmp, TextureHandleType type, bool clampToEdge)
{
   TextureObject *ret = NULL;
   if(textureName)
   {
      textureName = StringTable->insert(textureName);
      ret = TextureDictionary::find(textureName, type, clampToEdge);
   }

   if(ret)
   {
      // Crucial conditionals for the flush case...
      if (ret->bitmap != bmp)
         delete ret->bitmap;
      if (ret->texGLName)
         glDeleteTextures(1, (const GLuint*)&ret->texGLName);
      if (ret->smallTexGLName)
         glDeleteTextures(1, (const GLuint*)&ret->smallTexGLName);
      ret->texGLName      = 0;
      ret->smallTexGLName = 0;

#ifdef GATHER_METRICS
      AssertFatal(ret->textureSpace <= smTextureSpaceLoaded, "Error, that shouldn't happen!");
      smTextureSpaceLoaded -= ret->textureSpace;
      ret->textureSpace     = 0;
#endif
   }
   else
   {
      ret = new TextureObject;
      ret->texFileName    = textureName;
      ret->texGLName      = 0;
      ret->smallTexGLName = 0;
      ret->refCount       = 0;
      ret->type           = type;

      TextureDictionary::insert(ret);
   }

   ret->bitmap       = bmp;
   ret->bitmapWidth  = bmp->getWidth();
   ret->bitmapHeight = bmp->getHeight();
   ret->texWidth     = getNextPow2(ret->bitmapWidth);
   ret->texHeight    = getNextPow2(ret->bitmapHeight);
   ret->clamp        = clampToEdge;
   ret->holding      = (type == MeshTexture) && ENABLE_HOLDING;

   if (ret->type == DetailTexture &&
       bmp->getFormat() != GBitmap::Palettized)
      bmp->extrudeMipLevels();
   else if (ret->type != TerrainTexture &&
            ret->type != BitmapTexture && 
            ret->type != BitmapKeepTexture &&
            ret->type != BitmapNoDownloadTexture &&
            bmp->getFormat() != GBitmap::Palettized)
      bmp->extrudeMipLevels(ret->type==ZeroBorderTexture);

   if(!ret->texGLName) {
      U32 firstMip = 0;
      if (ret->bitmap->getNumMipLevels() > 1 &&
          type != DetailTexture &&
          type != TerrainTexture &&
          type != BitmapTexture &&
          type != BitmapKeepTexture &&
          type != BitmapNoDownloadTexture)
      {
         if (type == SkyTexture)
         {
            firstMip = getMin(sgSkyTextureDetailLevel, ret->bitmap->getNumMipLevels() - 1);
         }
         else if (type == InteriorTexture)
         {
            firstMip = getMin(sgInteriorTextureDetailLevel, ret->bitmap->getNumMipLevels() - 1);
         }
         else
         {
            firstMip = getMin(sgTextureDetailLevel, ret->bitmap->getNumMipLevels() - 1);
         }
      }

      ret->downloadedWidth  = ret->bitmapWidth  >> firstMip;
      ret->downloadedHeight = ret->bitmapHeight >> firstMip;
      if (ret->downloadedWidth  == 0) ret->downloadedWidth  = 1;
      if (ret->downloadedHeight == 0) ret->downloadedHeight = 1;

#ifdef GATHER_METRICS
      ret->textureSpace = 0;
      for (U32 i = firstMip; i < ret->bitmap->getNumMipLevels(); i++)
         ret->textureSpace += ret->bitmap->getWidth(i) * ret->bitmap->getHeight(i);
      smTextureSpaceLoaded += ret->textureSpace;
#endif

      if(ret->type != BitmapNoDownloadTexture)
         createGLName(bmp, clampToEdge, firstMip, ret->type, ret);
   }
   
   if (ret->type == BitmapKeepTexture || ret->type == BitmapNoDownloadTexture) {
      // do nothing
   } else if (ret->type == TerrainTexture) {
      // Don't delete the bitmap
      ret->bitmap = NULL;
   } else {
      delete ret->bitmap;
      ret->bitmap = NULL;
   }

   return ret;
}   


//--------------------------------------
static GBitmap *loadBitmapInstance(const char *textureName)
{
   char fileNameBuffer[512];
   dStrcpy(fileNameBuffer, textureName);
   
   GBitmap *bmp = NULL;

   U32 len = dStrlen(fileNameBuffer);
   // DMM: Major hack here until we decide what do to about this problem.
   for (U32 i = 0; i < EXT_ARRAY_SIZE && bmp == NULL; i++) {
      if (sgForcePalettedTexture == true && dglDoesSupportPalettedTexture())
         dStrcpy(fileNameBuffer + len, extArray_8[i]);
      else
         dStrcpy(fileNameBuffer + len, extArray[i]);
      bmp = (GBitmap*)ResourceManager->loadInstance(fileNameBuffer);
   }
   return bmp;   
}

//--------------------------------------

TextureObject *TextureManager::loadTexture(const char* textureName, TextureHandleType type, bool clampToEdge)
{
   // case of assigning texture to NULL
   if(!textureName)
      return NULL;
   
   textureName = StringTable->insert(textureName);
   
   TextureObject *ret = TextureDictionary::find(textureName, type, clampToEdge);

   if(ret)
      return ret;

   GBitmap *bmp = loadBitmapInstance(textureName);
   if(!bmp)
      return NULL;

   return registerTexture(textureName, bmp, type, clampToEdge);
}


//--------------------------------------
void TextureHandle::setClamp(const bool c)
{
   if (object)
   {
      object->clamp = c;
      if (object->texGLName != 0)
      {
         glBindTexture(GL_TEXTURE_2D, object->texGLName);
         GLenum clamp;
         if (c)
            clamp = dglDoesSupportEdgeClamp() ? GL_CLAMP_TO_EDGE_EXT : GL_CLAMP;
         else
            clamp = GL_REPEAT;
         
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp);
      }
      if (object->smallTexGLName != 0)
      {
         glBindTexture(GL_TEXTURE_2D, object->smallTexGLName);
         GLenum clamp;
         if (c)
            clamp = dglDoesSupportEdgeClamp() ? GL_CLAMP_TO_EDGE_EXT : GL_CLAMP;
         else
            clamp = GL_REPEAT;
         
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp);
      }
   }
}

void TextureHandle::lock()
{
   if(object)
      object->refCount++;
}

void TextureHandle::unlock()
{
   if(object)
   {
      object->refCount--;
      if (object->holding == false)
      {
         if(!object->refCount)
            TextureManager::freeTexture(object);
      }
      else
      {
         AssertISV(object->refCount >= 0, avar("Texture holding out of balance: %d (0x%x)",
                                               object->refCount, object->refCount));
      }
      
      object = NULL;
   }
}

void TextureHandle::refresh()
{
   TextureManager::refresh(object);
}

void TextureHandle::refresh(GBitmap* bmp)
{
   AssertFatal(object->type == TerrainTexture, "Error, only terrain textures may be refreshed in this manner!");
   TextureManager::refresh(object, bmp);
}


#ifdef GATHER_METRICS
F32 TextureManager::getResidentFraction()
{
   U32 resident = 0;
   U32 total    = 0;

   Vector<GLuint> names;

   TextureObject* pProbe = TextureDictionary::smTOList;
   while (pProbe != NULL) {
      if (pProbe->texGLName != 0) {
         total++;
         names.push_back(pProbe->texGLName);
      }

      pProbe = pProbe->next;
   }

   if (total == 0)
      return 1.0f;

   Vector<GLboolean> isResident;
   isResident.setSize(names.size());

   glAreTexturesResident(names.size(), names.address(), isResident.address());
   for (U32 i = 0; i < names.size(); i++)
      if (isResident[i] == GL_TRUE)
         resident++;

   return (F32(resident) / F32(total));
}
#endif


ChunkedTextureObject *gChunkedTextureList = NULL;

ChunkedTextureObject* ChunkedTextureManager::loadTexture(const char *textureName)
{
   if(!textureName)
      return NULL;
   StringTableEntry tName = StringTable->insert(textureName);
   
   for(ChunkedTextureObject *walk = gChunkedTextureList; walk; walk = walk->next)
      if(walk->texFileName == tName)
         return walk;
   GBitmap *bmp = loadBitmapInstance(textureName);
   if(!bmp)
      return NULL;
   return registerTexture(textureName, bmp, false);
}

ChunkedTextureObject* ChunkedTextureManager::registerTexture(const char *textureName, GBitmap *data, bool keep)
{
   ChunkedTextureObject *ret = NULL;
   StringTableEntry tName = NULL;
   
   if(textureName)
   {
      tName = StringTable->insert(textureName);
      for(ChunkedTextureObject *walk = gChunkedTextureList; walk; walk = walk->next)
      {
         if(walk->texFileName == tName)
         {
            ret = walk;
            break;
         }
      }
   }
   if(ret && ret->bitmap)
   {
      delete ret->bitmap;
      ret->bitmap = data;
   }
   else
   {
      ret = new ChunkedTextureObject;
      ret->bitmap = data;
      ret->texFileName = tName;
      ret->next = gChunkedTextureList;
      gChunkedTextureList = ret;
      ret->texWidthCount = (data->getWidth() + 255) >> 8;
      ret->texHeightCount = (data->getHeight() + 255) >> 8;
      ret->width = data->getWidth();
      ret->height = data->getHeight();
      ret->textureHandles = NULL;
      ret->refCount = 0;
   }
   refresh(ret);
   if(!keep)
   {
      delete ret->bitmap;
      ret->bitmap = NULL;
   }
   return ret;
}

void ChunkedTextureManager::freeTexture(ChunkedTextureObject *to)
{
   // remove it from the linked list
   
   for(ChunkedTextureObject **walk = &gChunkedTextureList; *walk; walk = &((*walk)->next))
   {
      if(*walk == to)
      {
         *walk = to->next;
         delete[] to->textureHandles;
         delete to->bitmap;
         return;
      }
   }
}

void ChunkedTextureManager::refresh(ChunkedTextureObject *to)
{
   if(!to->bitmap)
      return;
      
   if(to->textureHandles)
   {
      delete[] to->textureHandles;
      to->textureHandles = NULL;
   }
   to->textureHandles = new TextureHandle[to->texWidthCount * to->texHeightCount];
   for(U32 j = 0; j < to->texHeightCount; j++)
   {
      U32 y = j * 256;
      U32 height = getMin(to->bitmap->getHeight() - y, U32(256));
      
      for(U32 i = 0; i < to->texWidthCount; i++)
      {
         U32 index = j * to->texWidthCount + i;
         U32 x = i * 256;
         U32 width = getMin(to->bitmap->getWidth() - x, U32(256));
         GBitmap *tempBitmap = new GBitmap(width, height, false, to->bitmap->getFormat());
         for(U32 lp = 0; lp < height; lp++)
         {
            const U8 *src = to->bitmap->getAddress(x, y + lp);
            U8 *dest = tempBitmap->getAddress(0, lp);
            dMemcpy(dest, src, width * to->bitmap->bytesPerPixel);
         }
         to->textureHandles[index] = TextureHandle(NULL, tempBitmap, BitmapTexture, true);
      }
   }
}

void ChunkedTextureManager::makeZombie()
{
   for(ChunkedTextureObject *walk = gChunkedTextureList; walk; walk = walk->next)
   {
      delete[] walk->textureHandles;
      walk->textureHandles = NULL;
   }
}

void ChunkedTextureManager::resurrect()
{
   for(ChunkedTextureObject *walk = gChunkedTextureList; walk; walk = walk->next)
   {
      GBitmap *bmp = walk->bitmap;
      if(!bmp)
         walk->bitmap = loadBitmapInstance(walk->texFileName);
      refresh(walk);
      if(!bmp)
      {
         delete walk->bitmap;
         walk->bitmap = NULL;
      }
   }
}

TextureHandle ChunkedTextureHandle::getSubTexture(U32 x, U32 y)
{
   if(!object || !object->textureHandles)
      return NULL;
   return object->textureHandles[x + y * object->texWidthCount];
}


void ChunkedTextureHandle::lock()
{
   if(object)
      object->refCount++;
}

void ChunkedTextureHandle::unlock()
{
   if(object)
   {
      object->refCount--;
      if(object->refCount == 0)
         ChunkedTextureManager::freeTexture(object);
   }
}

