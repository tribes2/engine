//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GTEXMANAGER_H_
#define _GTEXMANAGER_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

//-------------------------------------- Forward Decls.
class GBitmap;

//------------------------------------------------------------------------------
//-------------------------------------- TextureHandle
//

enum TextureHandleType
{
   BitmapTexture = 0,
   BitmapKeepTexture,
   BitmapNoDownloadTexture,
   RegisteredTexture,
   MeshTexture,
   TerrainTexture,
   SkyTexture,
   InteriorTexture,
   
   DetailTexture,
   ZeroBorderTexture
};

class TextureObject
{
  public:
   TextureObject *next;
   TextureObject *prev;
   TextureObject *hashNext;

   U32 texGLName;
   U32 smallTexGLName;

#ifdef GATHER_METRICS
   U32 textureSpace;
#endif

   StringTableEntry texFileName;
   GBitmap * bitmap;

   U32 texWidth;
   U32 texHeight;
   
   U32 bitmapWidth;
   U32 bitmapHeight;

   U32 downloadedWidth;
   U32 downloadedHeight;

   TextureHandleType type;
   bool              clamp;
   bool              holding;
   S32               refCount;
};

typedef void (*TextureEventCallback)(const U32 eventCode, const U32 userData);

struct TextureManager
{
   // additional functions for refreshing the textures, reloading larger
   // mip levels, etc, will go in here, as well as delay-load functions.
   friend class TextureHandle;
   friend class InteriorLMManager;
   friend struct TextureDictionary;

  private:
   static TextureObject* loadTexture(const char *textureName, TextureHandleType type, bool clampToEdge);
   static TextureObject* registerTexture(const char *textureName, const GBitmap *data, bool clampToEdge);
   static TextureObject* registerTexture(const char *textureName, GBitmap *data, TextureHandleType type, bool clampToEdge);
   static void           freeTexture(TextureObject *to);
   static bool           createGLName(GBitmap *pb, bool clampToEdge, U32 firstMip, TextureHandleType type, TextureObject* obj);
   static void           refresh(TextureObject *to);
   static void           refresh(TextureObject *to, GBitmap*);
   static GBitmap*       createMipBitmap(const GBitmap* pBitmap);
   static GBitmap*       createPaddedBitmap(GBitmap* pBitmap);


  public:
   static void create();
   static void preDestroy();
   static void destroy();

   static void makeZombie();  // This pair of functions is a flush() equivalent.  To flush
   static void resurrect();   //  the cache, call:
                              //   makeZombie(); /* blah blah blah */ resurrect();
                              //  Note that NO drawing must take place until resurrect is
                              //  called.  The manager is a stinking corpse at this point.
                              //  The split is necessary to support changing the OpenGL
                              //  device in the "right way".  This way glDeleteTexture is
                              //  called on the original device rather than on the new
                              //  device, as a flush() call would necessitate.
   static void flush();       // Added for convenience when you don't need to worry about
                              //  the above problems.
   static bool smIsZombie;

#ifdef GATHER_METRICS
   static void dumpStats();
#endif
   
   enum EventCodes {
      BeginZombification = 0,
      CacheResurrected   = 1
   };
   static U32  registerEventCallback(TextureEventCallback, const U32 userData);
   static void unregisterEventCallback(const U32 callbackKey);

  private:
   static void postTextureEvent(const U32);
   static bool smUseSmallTextures;

  public:
   static const char * csmTexturePrefix;

   static void setSmallTexturesActive(const bool t) { smUseSmallTextures = t;    }
   static bool areSmallTexturesActive()             { return smUseSmallTextures; }
   
#ifdef GATHER_METRICS
   static U32 smTextureSpaceLoaded;
   static U32 smTextureCacheMisses;

   static F32 getResidentFraction();
#endif
};

//------------------------------------------------------------------
//
// TextureHandle - this is how you access a bitmap, etc.
//
// Texture handles can be allocated in 2 ways - by name to be loaded
// from disk, or by name to a dynamically generated texture
// 
// If you create a GBitmap and register it, the Texture manager
// owns the pointer - so if you re-register a texture with the same
// name, the texture manager will delete the second copy.
//
//------------------------------------------------------------------

class TextureHandle
{
   TextureObject *object;
   void lock();
   void unlock();
  public:
   TextureHandle() { object = NULL; }
   TextureHandle(const TextureHandle &th) {
      object = th.object;
      lock();
   }

   TextureHandle(const char*       textureName,
                 TextureHandleType type=BitmapTexture,
                 bool              clampToEdge = false) {
      object = TextureManager::loadTexture(textureName, type, clampToEdge);
      lock();
   }

   TextureHandle(const char*    textureName,
                 const GBitmap* bmp,
                 bool           clampToEdge = false) {
      object = TextureManager::registerTexture(textureName, bmp, clampToEdge);
      lock();
   }

   TextureHandle(const char*       textureName,
                 GBitmap*          bmp,
                 TextureHandleType type,
                 bool              clampToEdge = false) {
      object = TextureManager::registerTexture(textureName, bmp, type, clampToEdge);
      lock();
   }

   ~TextureHandle() { unlock(); }
   
   TextureHandle& operator=(const TextureHandle &t) {
      unlock();
      object = t.object;
      lock();
      return *this;
   }
   void set(const char *textureName,
            TextureHandleType type=BitmapTexture,
            bool clampToEdge = false) {
      TextureObject* newObject = TextureManager::loadTexture(textureName, type, clampToEdge);;
      if (newObject != object)
      {
         unlock();
         object = newObject;
         lock();
      }
   }
   void set(const char *textureName,
            const GBitmap *data,
            bool clampToEdge = false) {
      TextureObject* newObject = TextureManager::registerTexture(textureName, data, clampToEdge);
      if (newObject != object)
      {
         unlock();
         object = newObject;
         lock();
      }
   }
   void set(const char *textureName,
            GBitmap *bmp,
            TextureHandleType type,
            bool clampToEdge = false) {
      TextureObject* newObject = TextureManager::registerTexture(textureName, bmp, type, clampToEdge);
      if (newObject != object)
      {
         unlock();
         object = newObject;
         lock();
      }
   }

   bool operator==(const TextureHandle &t) const { return t.object == object; }
   bool operator!=(const TextureHandle &t) const { return t.object != object; }

   void setClamp(const bool);
   
   void refresh();
   void refresh(GBitmap*);
   operator TextureObject*()        { return object; }
   const char* getName() const      { return (object ? object->texFileName      : NULL); }
   U32 getWidth() const             { return (object ? object->bitmapWidth      : 0UL);    }
   U32 getHeight() const            { return (object ? object->bitmapHeight     : 0UL);    }
   U32 getDownloadedWidth() const   { return (object ? object->downloadedWidth  : 0UL);    }
   U32 getDownloadedHeight() const  { return (object ? object->downloadedHeight : 0UL);    }
   GBitmap* getBitmap()             { return (object ? object->bitmap           : NULL); }
   U32 getGLName() const;
};

#if defined(GATHER_METRICS) && GATHER_METRICS > 1
#ifndef _PLATFORMGL_H_
#include "engine/platformWIN32/platformGL.h"
#endif

inline U32 TextureHandle::getGLName() const
{
   if (!object)
      return 0;

   U32 useName = tex->texGLName;
   if (TextureManager::areSmallTexturesActive() && object->smallTexGLName != 0)
      useName = object->smallTexGLName;

   if (useName != 0) {
      GLboolean res;
      glAreTexturesResident(1, &useName, &res);
      if (res == GL_FALSE)
         TextureManager::smTextureCacheMisses++;
   }

   return useName;
}

#else

inline U32 TextureHandle::getGLName() const
{
   if (!object)
      return 0;

   U32 useName = object->texGLName;
   if (TextureManager::areSmallTexturesActive() && object->smallTexGLName != 0)
      useName = object->smallTexGLName;

   return useName;
}

#endif

#endif // _GTEXMANAGER_H_
