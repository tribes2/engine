//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "PlatformWin32/platformGL.h"
#include "PlatformPPC/platformPPC.h"
#include <time.h>
#include "console/console.h"

GLState gGLState;

bool gOpenGLDisableCVA   = false;
bool gOpenGLDisableTEC   = false;
bool gOpenGLDisableARBMT = false;
bool gOpenGLDisableFC    = true;


// #define GL_EXT_abgr                       1
// #define GL_EXT_blend_color                1
// #define GL_EXT_blend_minmax               1
// #define GL_EXT_blend_subtract             1
// #define GL_EXT_compiled_vertex_array      1
// #define GL_ARB_multitexture               1
// #define GL_APPLE_specular_vector          1
// #define GL_APPLE_transform_hint           1




bool QGL_EXT_Init( )
{
   // Load extensions...
   //
   const char* pExtString = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));

   // EXT_compiled_vertex_array
   if (GL_EXT_compiled_vertex_array) //pExtString && dStrstr(pExtString, (const char*)"GL_EXT_compiled_vertex_array") != NULL) 
   {
      //glLockArraysEXT   = dllLockArraysEXT   = (glLockArrays_t)   qwglGetProcAddress("glLockArraysEXT");
      //glUnlockArraysEXT = dllUnlockArraysEXT = (glUnlockArrays_t) qwglGetProcAddress("glUnlockArraysEXT");
      gGLState.suppLockedArrays = true;
   } else {
      //glLockArraysEXT   = dllLockArraysEXT   = NULL;
      //glUnlockArraysEXT = dllUnlockArraysEXT = NULL;
      gGLState.suppLockedArrays = false;
   }

   // ARB_multitexture
   if (GL_ARB_multitexture) //pExtString && dStrstr(pExtString, (const char*)"GL_ARB_multitexture") != NULL) 
   {
      //glActiveTextureARB       = dllActiveTextureARB       = (glActiveTextureARB_t)       qwglGetProcAddress("glActiveTextureARB");
      //glClientActiveTextureARB = dllClientActiveTextureARB = (glClientActiveTextureARB_t) qwglGetProcAddress("glClientActiveTextureARB");
      //glMultiTexCoord2fARB     = dllMultiTexCoord2fARB     = (glMultiTexCoord2fARB_t)     qwglGetProcAddress("glMultiTexCoord2fARB");
      //glMultiTexCoord2fvARB    = dllMultiTexCoord2fvARB    = (glMultiTexCoord2fvARB_t)    qwglGetProcAddress("glMultiTexCoord2fvARB");
      gGLState.suppARBMultitexture = true;
   } else {
      //glActiveTextureARB       = dllActiveTextureARB       = NULL;
      //glClientActiveTextureARB = dllClientActiveTextureARB = NULL;
      //glMultiTexCoord2fARB     = dllMultiTexCoord2fARB     = NULL;
      //glMultiTexCoord2fvARB    = dllMultiTexCoord2fvARB    = NULL;
      gGLState.suppARBMultitexture = false;
   }

   // NV_vertex_array_range
   if (false) //pExtString && dStrstr(pExtString, (const char*)"GL_NV_vertex_array_range") != NULL) 
   {
      //glVertexArrayRangeNV      = dllVertexArrayRangeNV      = (glVertexArrayRange_t)      qwglGetProcAddress("glVertexArrayRangeNV");
      //glFlushVertexArrayRangeNV = dllFlushVertexArrayRangeNV = (glFlushVertexArrayRange_t) qwglGetProcAddress("glFlushVertexArrayRangeNV");
      gGLState.suppVertexArrayRange = true;
   } else {
      //glVertexArrayRangeNV      = dllVertexArrayRangeNV      = NULL;
      //glFlushVertexArrayRangeNV = dllFlushVertexArrayRangeNV = NULL;
      gGLState.suppVertexArrayRange = false;
   }

   // EXT_fog_coord
   if (false) //pExtString && dStrstr(pExtString, (const char*)"GL_EXT_fog_coord") != NULL) 
   {
      //glFogCoordfEXT       = dllFogCoordfEXT       = (glFogCoordf_t)       qwglGetProcAddress("glFogCoordfEXT");
      //glFogCoordPointerEXT = dllFogCoordPointerEXT = (glFogCoordPointer_t) qwglGetProcAddress("glFogCoordPointerEXT");
      gGLState.suppFogCoord = true;
   } else {
      //glFogCoordfEXT       = dllFogCoordfEXT       = NULL;
      //glFogCoordPointerEXT = dllFogCoordPointerEXT = NULL;
      gGLState.suppFogCoord = false;
   }

   // Binary states, i.e., no supporting functions
   // EXT_packed_pixels
   // EXT_texture_env_combine
   //
   gGLState.suppPackedPixels      = false; //pExtString? (dStrstr(pExtString, (const char*)"GL_EXT_packed_pixels") != NULL) : false;
   gGLState.suppTextureEnvCombine = false; //pExtString? (dStrstr(pExtString, (const char*)"GL_EXT_texture_env_combine") != NULL) : false;

   Con::printf("OpenGL Init: Enabled Extensions");
   if (gGLState.suppARBMultitexture)   Con::printf("  ARB_multitexture");
   if (gGLState.suppLockedArrays)      Con::printf("  EXT_compiled_vertex_array");
   if (gGLState.suppVertexArrayRange)  Con::printf("  NV_vertex_array_range");
   if (gGLState.suppTextureEnvCombine) Con::printf("  EXT_texture_env_combine");
   if (gGLState.suppPackedPixels)      Con::printf("  EXT_packed_pixels");
   if (gGLState.suppFogCoord)          Con::printf("  EXT_fog_coord");

   Con::warnf(ConsoleLogEntry::General, "OpenGL Init: Disabled Extensions");
   if (!gGLState.suppARBMultitexture)   Con::warnf(ConsoleLogEntry::General, "  ARB_multitexture");
   if (!gGLState.suppLockedArrays)      Con::warnf(ConsoleLogEntry::General, "  EXT_compiled_vertex_array");
   if (!gGLState.suppVertexArrayRange)  Con::warnf(ConsoleLogEntry::General, "  NV_vertex_array_range");
   if (!gGLState.suppTextureEnvCombine) Con::warnf(ConsoleLogEntry::General, "  EXT_texture_env_combine");
   if (!gGLState.suppPackedPixels)      Con::warnf(ConsoleLogEntry::General, "  EXT_packed_pixels");
   if (!gGLState.suppFogCoord)          Con::warnf(ConsoleLogEntry::General, "  EXT_fog_coord");
   Con::printf("");
   
   return true;
}
