//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "PlatformMacCarb/platformGL.h"
#include "PlatformMacCarb/platformMacCarb.h"
#include "console/console.h"

GLState gGLState;

bool gOpenGLDisablePT         = false;
bool gOpenGLDisableCVA         = false;
bool gOpenGLDisableTEC         = false;
bool gOpenGLDisableARBMT      = false;
bool gOpenGLDisableFC         = true; //false;
bool gOpenGLDisableTCompress   = false;
bool gOpenGLNoEnvColor         = false;
float gOpenGLGammaCorrection   = 0.5;
bool gOpenGLNoDrawArraysAlpha   = false;


#if 1 // until we decide how to actually implement these...

GLboolean mglAvailVB() {   return(false); }
GLint mglAllocVB(GLsizei size, GLint format, GLboolean preserve) {   return(0); }
void* mglLockVB(GLint handle, GLsizei size) {   return(NULL); }
void mglUnlockVB(GLint handle) {}
void mglSetVB(GLint handle) {}
void mglOffsetVB(GLint handle, GLuint offset) {}
void mglFillVB(GLint handle, GLint first, GLsizei count) {}
void mglFreeVB(GLint handle) {}
void mglFogCP(GLenum type, GLsizei stride, const GLvoid *pointer) {}
void mglColorTable(GLenum target, GLenum internalFormat, GLsizei width, GLenum format, GLenum type, const GLvoid *table) {}

GLboolean glAvailableVertexBufferEXT() {   return(false); }
GLint glAllocateVertexBufferEXT(GLsizei size, GLint format, GLboolean preserve) {   return(0); }
void* glLockVertexBufferEXT(GLint handle, GLsizei size) {   return(NULL); }
void glUnlockVertexBufferEXT(GLint handle) {}
void glSetVertexBufferEXT(GLint handle) {}
void glOffsetVertexBufferEXT(GLint handle, GLuint offset) {}
void glFillVertexBufferEXT(GLint handle, GLint first, GLsizei count) {}
void glFreeVertexBufferEXT(GLint handle) {}
void glFogCoordPointerEXT(GLenum type, GLsizei stride, const GLvoid *pointer) {}
void glColorTableEXT(GLenum target, GLenum internalFormat, GLsizei width, GLenum format, GLenum type, const GLvoid *table) {}

#endif



bool QGL_EXT_Init( )
{
   // Load extensions...
   //
   const char* pExtString = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));

   // OpenGL Exists ========================================
   if (glBegin == (void *)kUnresolvedCFragSymbolAddress)
   { // !!!!!!! TBD
      Con::printf("OpenGL Init: Failed to find OpenGL system library.");
      return(false);
   }

   // EXT_paletted_texture ========================================
//   glColorTableEXT = NULL;
   if (pExtString && dStrstr(pExtString, (const char*)"GL_EXT_paletted_texture") != NULL)
      gGLState.suppPalettedTexture = true;
   else
      gGLState.suppPalettedTexture = false;
   
   // EXT_compiled_vertex_array ========================================
/*
   glLockArraysEXT   = NULL;
   glUnlockArraysEXT = NULL;
*/
#if FOR_ATI_TRUFORM_DEMO //!!!!!TBD!!!!!!!! HACK HACK HACK
   if (pExtString && dStrstr(pExtString, (const char*)"GL_EXT_compiled_vertex_array") != NULL)
      gGLState.suppLockedArrays = true;
   else
#endif
      gGLState.suppLockedArrays = false;

   // ARB_multitexture ========================================
/*
      glActiveTextureARB       = NULL;
      glClientActiveTextureARB = NULL;
      glMultiTexCoord2fARB     = NULL;
      glMultiTexCoord2fvARB    = NULL;
*/
   if (pExtString && dStrstr(pExtString, (const char*)"GL_ARB_multitexture") != NULL)
      gGLState.suppARBMultitexture = true;
   else
      gGLState.suppARBMultitexture = false;

   // NV_vertex_array_range ========================================
/*
      glVertexArrayRangeNV      = dllVertexArrayRangeNV      = NULL;
      glFlushVertexArrayRangeNV = dllFlushVertexArrayRangeNV = NULL;
*/
/*
   wglAllocateMemoryNV       = NULL;
   wglFreeMemoryNV           = NULL;
*/
   if (pExtString && dStrstr(pExtString, (const char*)"GL_NV_vertex_array_range") != NULL)
      gGLState.suppVertexArrayRange = true;
   else
      gGLState.suppVertexArrayRange = false;
   

   // EXT_fog_coord ========================================
/*
      glFogCoordfEXT       = NULL;
      glFogCoordPointerEXT = NULL;
*/
   if (pExtString && dStrstr(pExtString, (const char*)"GL_EXT_fog_coord") != NULL)
      gGLState.suppFogCoord = true;
   else
      gGLState.suppFogCoord = false;
   
   // ARB_texture_compression ========================================
/*
      glCompressedTexImage3DARB    = NULL;
      glCompressedTexImage2DARB    = NULL;
      glCompressedTexImage1DARB    = NULL;
      glCompressedTexSubImage3DARB = NULL;
      glCompressedTexSubImage2DARB = NULL;
      glCompressedTexSubImage1DARB = NULL;
      glGetCompressedTexImageARB   = NULL;
*/
   if (pExtString && dStrstr(pExtString, (const char*)"GL_ARB_texture_compression") != NULL)
      gGLState.suppTextureCompression = true;
   else
      gGLState.suppTextureCompression = false;
   

   // 3DFX_texture_compression_FXT1 ========================================
   if (pExtString && dStrstr(pExtString, (const char*)"GL_3DFX_texture_compression_FXT1") != NULL)
      gGLState.suppFXT1 = true;
   else
      gGLState.suppFXT1 = false;


   // EXT_texture_compression_S3TC ========================================
   if (pExtString && dStrstr(pExtString, (const char*)"GL_EXT_texture_compression_s3tc") != NULL)
      gGLState.suppS3TC = true;
   else
      gGLState.suppS3TC = false;


   // WGL_3DFX_gamma_control ========================================
/*
      qwglGetDeviceGammaRamp3DFX = NULL;
      qwglSetDeviceGammaRamp3DFX = NULL;
*/
   if (pExtString && dStrstr(pExtString, (const char*)"WGL_3DFX_gamma_control" ) != NULL)
   { 
//      qwglGetDeviceGammaRamp3DFX = (qwglGetDeviceGammaRamp3DFX_t) qwglGetProcAddress( "wglGetDeviceGammaRamp3DFX" ); 
//     qwglSetDeviceGammaRamp3DFX = (qwglSetDeviceGammaRamp3DFX_t) qwglGetProcAddress( "wglSetDeviceGammaRamp3DFX" );
   }
   else
   {
   }


   // EXT_vertex_buffer ========================================
/*
      glAvailableVertexBufferEXT   = NULL;
      glAllocateVertexBufferEXT   = NULL;
      glLockVertexBufferEXT      = NULL;
      glUnlockVertexBufferEXT      = NULL;
      glSetVertexBufferEXT         = NULL;
      glOffsetVertexBufferEXT      = NULL;
      glFillVertexBufferEXT      = NULL;
      glFreeVertexBufferEXT      = NULL;
*/
   if (pExtString && dStrstr(pExtString, (const char*)"GL_EXT_vertex_buffer") != NULL)
   {
      gGLState.suppVertexBuffer = true;
      AssertWarn(gGLState.suppVertexBuffer == false, "We're assuming no vertex bufffer support on Mac for now!");
   }
   else
      gGLState.suppVertexBuffer = false;

   // Binary states, i.e., no supporting functions  ========================================
   // EXT_packed_pixels
   // EXT_texture_env_combine ...
   gGLState.suppPackedPixels      = pExtString? (dStrstr(pExtString, (const char*)"GL_EXT_packed_pixels") != NULL) : false;
   gGLState.suppTextureEnvCombine = pExtString? (dStrstr(pExtString, (const char*)"GL_EXT_texture_env_combine") != NULL) : false;
   gGLState.suppEdgeClamp         = pExtString? (dStrstr(pExtString, (const char*)"GL_EXT_texture_edge_clamp") != NULL) : false;
   gGLState.suppTexEnvAdd         = pExtString? (dStrstr(pExtString, (const char*)"GL_ARB_texture_env_add") != NULL) : false;
   gGLState.suppTexEnvAdd        |= pExtString? (dStrstr(pExtString, (const char*)"GL_EXT_texture_env_add") != NULL) : false;


   // Anisotropic filtering ========================================
   gGLState.suppTexAnisotropic    = pExtString? (dStrstr(pExtString, (const char*)"GL_EXT_texture_filter_anisotropic") != NULL) : false;
   if (gGLState.suppTexAnisotropic)
      glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &gGLState.maxAnisotropy);


   // Texture combine units  ========================================
   if (gGLState.suppARBMultitexture)
      glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &gGLState.maxTextureUnits);
   else
      gGLState.maxTextureUnits = 1;


   // Swap interval ========================================
/*
   qwglSwapIntervalEXT = NULL;
*/
   if (pExtString && dStrstr(pExtString, (const char*)"WGL_EXT_swap_control") != NULL)
      gGLState.suppSwapInterval = true; //( qwglSwapIntervalEXT != NULL );
   else
      gGLState.suppSwapInterval = false;


   // console out all the extensions...
   Con::printf("OpenGL Init: Enabled Extensions");
   if (gGLState.suppARBMultitexture)    Con::printf("  ARB_multitexture (Max Texture Units: %d)", gGLState.maxTextureUnits);
   if (gGLState.suppPalettedTexture)    Con::printf("  EXT_paletted_texture");
   if (gGLState.suppLockedArrays)       Con::printf("  EXT_compiled_vertex_array");
   if (gGLState.suppVertexArrayRange)   Con::printf("  NV_vertex_array_range");
   if (gGLState.suppTextureEnvCombine)  Con::printf("  EXT_texture_env_combine");
   if (gGLState.suppPackedPixels)       Con::printf("  EXT_packed_pixels");
   if (gGLState.suppFogCoord)           Con::printf("  EXT_fog_coord");
   if (gGLState.suppTextureCompression) Con::printf("  ARB_texture_compression");
   if (gGLState.suppS3TC)               Con::printf("  EXT_texture_compression_s3tc");
   if (gGLState.suppFXT1)               Con::printf("  3DFX_texture_compression_FXT1");
   if (gGLState.suppTexEnvAdd)          Con::printf("  (ARB|EXT)_texture_env_add");
   if (gGLState.suppTexAnisotropic)     Con::printf("  EXT_texture_filter_anisotropic (Max anisotropy: %f)", gGLState.maxAnisotropy);
   if (gGLState.suppSwapInterval)       Con::printf("  WGL_EXT_swap_control");

   Con::warnf("OpenGL Init: Disabled Extensions");
   if (!gGLState.suppARBMultitexture)    Con::warnf("  ARB_multitexture");
   if (!gGLState.suppPalettedTexture)    Con::warnf("  EXT_paletted_texture");
   if (!gGLState.suppLockedArrays)       Con::warnf("  EXT_compiled_vertex_array");
   if (!gGLState.suppVertexArrayRange)   Con::warnf("  NV_vertex_array_range");
   if (!gGLState.suppTextureEnvCombine)  Con::warnf("  EXT_texture_env_combine");
   if (!gGLState.suppPackedPixels)       Con::warnf("  EXT_packed_pixels");
   if (!gGLState.suppFogCoord)           Con::warnf("  EXT_fog_coord");
   if (!gGLState.suppTextureCompression) Con::warnf("  ARB_texture_compression");
   if (!gGLState.suppS3TC)               Con::warnf("  EXT_texture_compression_s3tc");
   if (!gGLState.suppFXT1)               Con::warnf("  3DFX_texture_compression_FXT1");
   if (!gGLState.suppTexEnvAdd)          Con::warnf("  (ARB|EXT)_texture_env_add");
   if (!gGLState.suppTexAnisotropic)     Con::warnf("  EXT_texture_filter_anisotropic");
   if (!gGLState.suppSwapInterval)       Con::warnf("  WGL_EXT_swap_control");
   Con::printf("");

   // Set some console variables:
   Con::setBoolVariable( "$FogCoordSupported", gGLState.suppFogCoord );
   Con::setBoolVariable( "$TextureCompressionSupported", gGLState.suppTextureCompression );
   Con::setBoolVariable( "$AnisotropySupported", gGLState.suppTexAnisotropic );
   Con::setBoolVariable( "$PalettedTextureSupported", gGLState.suppPalettedTexture );
   Con::setBoolVariable( "$SwapIntervalSupported", gGLState.suppSwapInterval );

   if (!gGLState.suppPalettedTexture && Con::getBoolVariable("$pref::OpenGL::forcePalettedTexture",false))
   {
      Con::setBoolVariable("$pref::OpenGL::forcePalettedTexture", false);
      Con::setBoolVariable("$pref::OpenGL::force16BitTexture", true);
   }

   return true;
}
