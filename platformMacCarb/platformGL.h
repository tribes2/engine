//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _PLATFORMGL_H_
#define _PLATFORMGL_H_


// ON MAC, WE ARE USING THE STD APPLE OPENGL HDRS.
#include <gl.h>
#include <glu.h>
#include <glext.h>


// FROM HERE ON IS >NOT< IN APPLE OPENGL 1.2 SDK
#if defined(GL_GLEXT_VERSION) && (GL_GLEXT_VERSION<=6) && defined(GL_VERSION_1_2)

#ifndef GL_EXT_vertex_buffer
#define GL_EXT_vertex_buffer 1

// these are the V12-custom extensions.  really only useful in external 3d driver.
#define GL_V12MTVFMT_EXT                     0x8702
#define GL_V12MTNVFMT_EXT                     0x8703
#define GL_V12FTVFMT_EXT                     0x8704
#define GL_V12FMTVFMT_EXT                     0x8705

#ifdef GL_GLEXT_PROTOTYPES
extern GLboolean APIENTRY glAvailableVertexBufferEXT();
extern GLint APIENTRY glAllocateVertexBufferEXT(GLsizei size, GLint format, GLboolean preserve);
extern void* APIENTRY glLockVertexBufferEXT(GLint handle, GLsizei size);
extern void APIENTRY glUnlockVertexBufferEXT(GLint handle);
extern void APIENTRY glSetVertexBufferEXT(GLint handle);
extern void APIENTRY glOffsetVertexBufferEXT(GLint handle, GLuint offset);
extern void APIENTRY glFillVertexBufferEXT(GLint handle, GLint first, GLsizei count);
extern void APIENTRY glFreeVertexBufferEXT(GLint handle);
#endif /* GL_GLEXT_PROTOTYPES */
typedef GLboolean (APIENTRY * PFNGLAVAILABLEVERTEXBUFFEREXTPROC) ();
typedef GLint (APIENTRY * PFNGLALLOCATEVERTEXBUFFEREXTPROC) (GLsizei size, GLint format, GLboolean preserve);
typedef void* (APIENTRY * PFNGLLOCKVERTEXBUFFEREXTPROC) (GLint handle, GLsizei size);
typedef void (APIENTRY * PFNGLUNLOCKVERTEXBUFFEREXTPROC) (GLint handle);
typedef void (APIENTRY * PFNGLSETVERTEXBUFFEREXTPROC) (GLint handle);
typedef void (APIENTRY * PFNGLOFFSETVERTEXBUFFEREXTPROC) (GLint handle, GLuint offset);
typedef void (APIENTRY * PFNGLFILLVERTEXBUFFEREXTPROC) (GLint handle, GLint first, GLsizei count);
typedef void (APIENTRY * PFNGLFREEVERTEXBUFFEREXTPROC) (GLint handle);
#endif

/* A forgotten token. */
#define GL_CLAMP_TO_EDGE_EXT                     0x812F

#define UNSIGNED_SHORT_5_6_5                     0x8363
#define UNSIGNED_SHORT_5_6_5_REV                  0x8364

#endif // glexts < v6, gl v1.2 stuff needed additionally.


/*
 * GL state information.
 */
struct GLState
{
   bool suppARBMultitexture;
   bool suppPackedPixels;
   bool suppTexEnvAdd;
   bool suppLockedArrays;
   bool suppTextureEnvCombine;
   bool suppVertexArrayRange;
   bool suppFogCoord;
   bool suppEdgeClamp;
   bool suppTextureCompression;
   bool suppS3TC;
   bool suppFXT1;
   bool suppTexAnisotropic;
   bool suppPalettedTexture;
   bool suppVertexBuffer;
   bool suppSwapInterval;
   unsigned int triCount[4];
   unsigned int primCount[4];
   unsigned int primMode; // 0-3

   GLfloat maxAnisotropy;
   GLint   maxTextureUnits;
};

extern GLState gGLState;

extern bool gOpenGLDisablePT;
extern bool gOpenGLDisableCVA;
extern bool gOpenGLDisableTEC;
extern bool gOpenGLDisableARBMT;
extern bool gOpenGLDisableFC;
extern bool gOpenGLDisableTCompress;
extern bool gOpenGLNoEnvColor;
extern float gOpenGLGammaCorrection;
extern bool gOpenGLNoDrawArraysAlpha;

/* 
 * Inline state helpers.
 */
inline void dglSetRenderPrimType(unsigned int type)
{
   gGLState.primMode = type;
}

inline void dglClearPrimMetrics()
{
   for(int i = 0; i < 4; i++)
      gGLState.triCount[i] = gGLState.primCount[i] = 0;
}

inline bool dglDoesSupportPalettedTexture()
{
   return gGLState.suppPalettedTexture && (gOpenGLDisablePT == false);
}

inline bool dglDoesSupportCompiledVertexArray()
{
   return gGLState.suppLockedArrays && (gOpenGLDisableCVA == false);
}

inline bool dglDoesSupportTextureEnvCombine()
{
   return gGLState.suppTextureEnvCombine && (gOpenGLDisableTEC == false);
}

inline bool dglDoesSupportARBMultitexture()
{
   return gGLState.suppARBMultitexture && (gOpenGLDisableARBMT == false);
}

inline bool dglDoesSupportVertexArrayRange()
{
   return gGLState.suppVertexArrayRange;
}

inline bool dglDoesSupportFogCoord()
{
   return gGLState.suppFogCoord && (gOpenGLDisableFC == false);
}

inline bool dglDoesSupportEdgeClamp()
{
   return gGLState.suppEdgeClamp;
}

inline bool dglDoesSupportTextureCompression()
{
   return gGLState.suppTextureCompression && (gOpenGLDisableTCompress == false);
}

inline bool dglDoesSupportS3TC()
{
   return gGLState.suppS3TC;
}

inline bool dglDoesSupportFXT1()
{
   return gGLState.suppFXT1;
}

inline bool dglDoesSupportTexEnvAdd()
{
   return gGLState.suppTexEnvAdd;
}

inline bool dglDoesSupportTexAnisotropy()
{
   return gGLState.suppTexAnisotropic;
}

inline bool dglDoesSupportVertexBuffer()
{
   return false;
}

inline GLfloat dglGetMaxAnisotropy()
{
   return gGLState.maxAnisotropy;
}

inline GLint dglGetMaxTextureUnits()
{
   if (dglDoesSupportARBMultitexture())
      return gGLState.maxTextureUnits;
   else
      return 1; 
}

#endif
