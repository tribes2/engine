//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include <time.h>
#include <math.h>

#include <SDL/SDL.h>

#include "PlatformWin32/platformGL.h"
#include "platformLinux/platformLinux.h"
#include "console/console.h"
#include "console/consoleTypes.h"

//
// Externally referencable GL renderer state.
//
GLState gGLState;

bool gOpenGLDisablePT = false;
bool gOpenGLDisableCVA = false;
bool gOpenGLDisableTEC = false;
bool gOpenGLDisableARBMT = false;
bool gOpenGLDisableFC = false;
bool gOpenGLDisableTCompress = false;
bool gOpenGLNoEnvColor = false;
float gOpenGLGammaCorrection = 0.5;
bool gOpenGLNoDrawArraysAlpha = false;

typedef const GLubyte* (*gluErrorString_t) (GLenum errCode);
gluErrorString_t gluErrorString;
typedef const GLubyte* (*gluGetString_t) (GLenum name);
gluGetString_t gluGetString;
typedef void (*gluOrtho2D_t) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top);
gluOrtho2D_t gluOrtho2D;
typedef void (*gluPerspective_t) (GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar);
gluPerspective_t gluPerspective;
typedef void (*gluPickMatrix_t) (GLdouble x, GLdouble y, GLdouble width, GLdouble height, GLint viewport[4]);
gluPickMatrix_t gluPickMatrix;
typedef void (*gluLookAt_t) (GLdouble eyex, GLdouble eyey, GLdouble eyez, GLdouble centerx, GLdouble centery, GLdouble centerz, GLdouble upx, GLdouble upy, GLdouble upz);
gluLookAt_t gluLookAt;
typedef int (*gluProject_t) (GLdouble objx, GLdouble objy, GLdouble objz, const GLdouble modelMatrix[16], const GLdouble projMatrix[16], const GLint viewport[4], GLdouble *winx, GLdouble *winy, GLdouble *winz);
gluProject_t gluProject;
typedef int (*gluUnProject_t) (GLdouble winx, GLdouble winy, GLdouble winz, const GLdouble modelMatrix[16], const GLdouble projMatrix[16], const GLint viewport[4], GLdouble *objx, GLdouble *objy, GLdouble *objz);
gluUnProject_t gluUnProject;
typedef int (*gluScaleImage_t) (GLenum format, GLint widthin, GLint heightin, GLenum typein, const void *datain, GLint widthout, GLint heightout, GLenum typeout, void *dataout);
gluScaleImage_t gluScaleImage;
typedef int (*gluBuild1DMipmaps_t) (GLenum target, GLint components, GLint width, GLenum format, GLenum type, const void *data);
gluBuild1DMipmaps_t gluBuild1DMipmaps;
typedef int (*gluBuild2DMipmaps_t) (GLenum target, GLint components, GLint width, GLint height, GLenum format, GLenum type, const void *data);
gluBuild2DMipmaps_t gluBuild2DMipmaps;

//------------------------------------------------------------------------------
// GL Functions
typedef void (*glAccum_t )(GLenum op, GLfloat value);
glAccum_t glAccum;
typedef void (*glAlphaFunc_t )(GLenum func, GLclampf ref);
glAlphaFunc_t glAlphaFunc;
typedef GLboolean (*glAreTexturesResident_t )(GLsizei n, const GLuint *textures, GLboolean *residences);
glAreTexturesResident_t glAreTexturesResident;
typedef void (*glArrayElement_t )(GLint i);
glArrayElement_t glArrayElement;
typedef void (*glBegin_t )(GLenum mode);
glBegin_t glBegin;
typedef void (*glBindTexture_t )(GLenum target, GLuint texture);
glBindTexture_t glBindTexture;
typedef void (*glBitmap_t )(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
glBitmap_t glBitmap;
typedef void (*glBlendFunc_t )(GLenum sfactor, GLenum dfactor);
glBlendFunc_t glBlendFunc;
typedef void (*glCallList_t )(GLuint list);
glCallList_t glCallList;
typedef void (*glCallLists_t )(GLsizei n, GLenum type, const GLvoid *lists);
glCallLists_t glCallLists;
typedef void (*glClear_t )(GLbitfield mask);
glClear_t glClear;
typedef void (*glClearAccum_t )(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
glClearAccum_t glClearAccum;
typedef void (*glClearColor_t )(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
glClearColor_t glClearColor;
typedef void (*glClearDepth_t )(GLclampd depth);
glClearDepth_t glClearDepth;
typedef void (*glClearIndex_t )(GLfloat c);
glClearIndex_t glClearIndex;
typedef void (*glClearStencil_t )(GLint s);
glClearStencil_t glClearStencil;
typedef void (*glClipPlane_t )(GLenum plane, const GLdouble *equation);
glClipPlane_t glClipPlane;
typedef void (*glColor3b_t )(GLbyte red, GLbyte green, GLbyte blue);
glColor3b_t glColor3b;
typedef void (*glColor3bv_t )(const GLbyte *v);
glColor3bv_t glColor3bv;
typedef void (*glColor3d_t )(GLdouble red, GLdouble green, GLdouble blue);
glColor3d_t glColor3d;
typedef void (*glColor3dv_t )(const GLdouble *v);
glColor3dv_t glColor3dv;
typedef void (*glColor3f_t )(GLfloat red, GLfloat green, GLfloat blue);
glColor3f_t glColor3f;
typedef void (*glColor3fv_t )(const GLfloat *v);
glColor3fv_t glColor3fv;
typedef void (*glColor3i_t )(GLint red, GLint green, GLint blue);
glColor3i_t glColor3i;
typedef void (*glColor3iv_t )(const GLint *v);
glColor3iv_t glColor3iv;
typedef void (*glColor3s_t )(GLshort red, GLshort green, GLshort blue);
glColor3s_t glColor3s;
typedef void (*glColor3sv_t )(const GLshort *v);
glColor3sv_t glColor3sv;
typedef void (*glColor3ub_t )(GLubyte red, GLubyte green, GLubyte blue);
glColor3ub_t glColor3ub;
typedef void (*glColor3ubv_t )(const GLubyte *v);
glColor3ubv_t glColor3ubv;
typedef void (*glColor3ui_t )(GLuint red, GLuint green, GLuint blue);
glColor3ui_t glColor3ui;
typedef void (*glColor3uiv_t )(const GLuint *v);
glColor3uiv_t glColor3uiv;
typedef void (*glColor3us_t )(GLushort red, GLushort green, GLushort blue);
glColor3us_t glColor3us;
typedef void (*glColor3usv_t )(const GLushort *v);
glColor3usv_t glColor3usv;
typedef void (*glColor4b_t )(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
glColor4b_t glColor4b;
typedef void (*glColor4bv_t )(const GLbyte *v);
glColor4bv_t glColor4bv;
typedef void (*glColor4d_t )(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
glColor4d_t glColor4d;
typedef void (*glColor4dv_t )(const GLdouble *v);
glColor4dv_t glColor4dv;
typedef void (*glColor4f_t )(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
glColor4f_t glColor4f;
typedef void (*glColor4fv_t )(const GLfloat *v);
glColor4fv_t glColor4fv;
typedef void (*glColor4i_t )(GLint red, GLint green, GLint blue, GLint alpha);
glColor4i_t glColor4i;
typedef void (*glColor4iv_t )(const GLint *v);
glColor4iv_t glColor4iv;
typedef void (*glColor4s_t )(GLshort red, GLshort green, GLshort blue, GLshort alpha);
glColor4s_t glColor4s;
typedef void (*glColor4sv_t )(const GLshort *v);
glColor4sv_t glColor4sv;
typedef void (*glColor4ub_t )(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
glColor4ub_t glColor4ub;
typedef void (*glColor4ubv_t )(const GLubyte *v);
glColor4ubv_t glColor4ubv;
typedef void (*glColor4ui_t )(GLuint red, GLuint green, GLuint blue, GLuint alpha);
glColor4ui_t glColor4ui;
typedef void (*glColor4uiv_t )(const GLuint *v);
glColor4uiv_t glColor4uiv;
typedef void (*glColor4us_t )(GLushort red, GLushort green, GLushort blue, GLushort alpha);
glColor4us_t glColor4us;
typedef void (*glColor4usv_t )(const GLushort *v);
glColor4usv_t glColor4usv;
typedef void (*glColorMask_t )(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
glColorMask_t glColorMask;
typedef void (*glColorMaterial_t )(GLenum face, GLenum mode);
glColorMaterial_t glColorMaterial;
typedef void (*glColorPointer_t )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
glColorPointer_t glColorPointer;
typedef void (*glCopyPixels_t )(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
glCopyPixels_t glCopyPixels;
typedef void (*glCopyTexImage1D_t )(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border);
glCopyTexImage1D_t glCopyTexImage1D;
typedef void (*glCopyTexImage2D_t )(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
glCopyTexImage2D_t glCopyTexImage2D;
typedef void (*glCopyTexSubImage1D_t )(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
glCopyTexSubImage1D_t glCopyTexSubImage1D;
typedef void (*glCopyTexSubImage2D_t )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
glCopyTexSubImage2D_t glCopyTexSubImage2D;
typedef void (*glCullFace_t )(GLenum mode);
glCullFace_t glCullFace;
typedef void (*glDeleteLists_t )(GLuint list, GLsizei range);
glDeleteLists_t glDeleteLists;
typedef void (*glDeleteTextures_t )(GLsizei n, const GLuint *textures);
glDeleteTextures_t glDeleteTextures;
typedef void (*glDepthFunc_t )(GLenum func);
glDepthFunc_t glDepthFunc;
typedef void (*glDepthMask_t )(GLboolean flag);
glDepthMask_t glDepthMask;
typedef void (*glDepthRange_t )(GLclampd zNear, GLclampd zFar);
glDepthRange_t glDepthRange;
typedef void (*glDisable_t )(GLenum cap);
glDisable_t glDisable;
typedef void (*glDisableClientState_t )(GLenum array);
glDisableClientState_t glDisableClientState;
typedef void (*glDrawArrays_t )(GLenum mode, GLint first, GLsizei count);
glDrawArrays_t glDrawArrays;
typedef void (*glDrawBuffer_t )(GLenum mode);
glDrawBuffer_t glDrawBuffer;
typedef void (*glDrawElements_t )(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
glDrawElements_t glDrawElements;
typedef void (*glDrawPixels_t )(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
glDrawPixels_t glDrawPixels;
typedef void (*glEdgeFlag_t )(GLboolean flag);
glEdgeFlag_t glEdgeFlag;
typedef void (*glEdgeFlagPointer_t )(GLsizei stride, const GLvoid *pointer);
glEdgeFlagPointer_t glEdgeFlagPointer;
typedef void (*glEdgeFlagv_t )(const GLboolean *flag);
glEdgeFlagv_t glEdgeFlagv;
typedef void (*glEnable_t )(GLenum cap);
glEnable_t glEnable;
typedef void (*glEnableClientState_t )(GLenum array);
glEnableClientState_t glEnableClientState;
typedef void (*glEnd_t )(void);
glEnd_t glEnd;
typedef void (*glEndList_t )(void);
glEndList_t glEndList;
typedef void (*glEvalCoord1d_t )(GLdouble u);
glEvalCoord1d_t glEvalCoord1d;
typedef void (*glEvalCoord1dv_t )(const GLdouble *u);
glEvalCoord1dv_t glEvalCoord1dv;
typedef void (*glEvalCoord1f_t )(GLfloat u);
glEvalCoord1f_t glEvalCoord1f;
typedef void (*glEvalCoord1fv_t )(const GLfloat *u);
glEvalCoord1fv_t glEvalCoord1fv;
typedef void (*glEvalCoord2d_t )(GLdouble u, GLdouble v);
glEvalCoord2d_t glEvalCoord2d;
typedef void (*glEvalCoord2dv_t )(const GLdouble *u);
glEvalCoord2dv_t glEvalCoord2dv;
typedef void (*glEvalCoord2f_t )(GLfloat u, GLfloat v);
glEvalCoord2f_t glEvalCoord2f;
typedef void (*glEvalCoord2fv_t )(const GLfloat *u);
glEvalCoord2fv_t glEvalCoord2fv;
typedef void (*glEvalMesh1_t )(GLenum mode, GLint i1, GLint i2);
glEvalMesh1_t glEvalMesh1;
typedef void (*glEvalMesh2_t )(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
glEvalMesh2_t glEvalMesh2;
typedef void (*glEvalPoint1_t )(GLint i);
glEvalPoint1_t glEvalPoint1;
typedef void (*glEvalPoint2_t )(GLint i, GLint j);
glEvalPoint2_t glEvalPoint2;
typedef void (*glFeedbackBuffer_t )(GLsizei size, GLenum type, GLfloat *buffer);
glFeedbackBuffer_t glFeedbackBuffer;
typedef void (*glFinish_t )(void);
glFinish_t glFinish;
typedef void (*glFlush_t )(void);
glFlush_t glFlush;
typedef void (*glFogf_t )(GLenum pname, GLfloat param);
glFogf_t glFogf;
typedef void (*glFogfv_t )(GLenum pname, const GLfloat *params);
glFogfv_t glFogfv;
typedef void (*glFogi_t )(GLenum pname, GLint param);
glFogi_t glFogi;
typedef void (*glFogiv_t )(GLenum pname, const GLint *params);
glFogiv_t glFogiv;
typedef void (*glFrontFace_t )(GLenum mode);
glFrontFace_t glFrontFace;
typedef void (*glFrustum_t )(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
glFrustum_t glFrustum;
typedef GLuint (*glGenLists_t )(GLsizei range);
glGenLists_t glGenLists;
typedef void (*glGenTextures_t )(GLsizei n, GLuint *textures);
glGenTextures_t glGenTextures;
typedef void (*glGetBooleanv_t )(GLenum pname, GLboolean *params);
glGetBooleanv_t glGetBooleanv;
typedef void (*glGetClipPlane_t )(GLenum plane, GLdouble *equation);
glGetClipPlane_t glGetClipPlane;
typedef void (*glGetDoublev_t )(GLenum pname, GLdouble *params);
glGetDoublev_t glGetDoublev;
typedef GLenum (*glGetError_t )(void);
glGetError_t glGetError;
typedef void (*glGetFloatv_t )(GLenum pname, GLfloat *params);
glGetFloatv_t glGetFloatv;
typedef void (*glGetIntegerv_t )(GLenum pname, GLint *params);
glGetIntegerv_t glGetIntegerv;
typedef void (*glGetLightfv_t )(GLenum light, GLenum pname, GLfloat *params);
glGetLightfv_t glGetLightfv;
typedef void (*glGetLightiv_t )(GLenum light, GLenum pname, GLint *params);
glGetLightiv_t glGetLightiv;
typedef void (*glGetMapdv_t )(GLenum target, GLenum query, GLdouble *v);
glGetMapdv_t glGetMapdv;
typedef void (*glGetMapfv_t )(GLenum target, GLenum query, GLfloat *v);
glGetMapfv_t glGetMapfv;
typedef void (*glGetMapiv_t )(GLenum target, GLenum query, GLint *v);
glGetMapiv_t glGetMapiv;
typedef void (*glGetMaterialfv_t )(GLenum face, GLenum pname, GLfloat *params);
glGetMaterialfv_t glGetMaterialfv;
typedef void (*glGetMaterialiv_t )(GLenum face, GLenum pname, GLint *params);
glGetMaterialiv_t glGetMaterialiv;
typedef void (*glGetPixelMapfv_t )(GLenum map, GLfloat *values);
glGetPixelMapfv_t glGetPixelMapfv;
typedef void (*glGetPixelMapuiv_t )(GLenum map, GLuint *values);
glGetPixelMapuiv_t glGetPixelMapuiv;
typedef void (*glGetPixelMapusv_t )(GLenum map, GLushort *values);
glGetPixelMapusv_t glGetPixelMapusv;
typedef void (*glGetPointerv_t )(GLenum pname, GLvoid* *params);
glGetPointerv_t glGetPointerv;
typedef void (*glGetPolygonStipple_t )(GLubyte *mask);
glGetPolygonStipple_t glGetPolygonStipple;
typedef const GLubyte * (*glGetString_t )(GLenum name);
glGetString_t glGetString;
typedef void (*glGetTexEnvfv_t )(GLenum target, GLenum pname, GLfloat *params);
glGetTexEnvfv_t glGetTexEnvfv;
typedef void (*glGetTexEnviv_t )(GLenum target, GLenum pname, GLint *params);
glGetTexEnviv_t glGetTexEnviv;
typedef void (*glGetTexGendv_t )(GLenum coord, GLenum pname, GLdouble *params);
glGetTexGendv_t glGetTexGendv;
typedef void (*glGetTexGenfv_t )(GLenum coord, GLenum pname, GLfloat *params);
glGetTexGenfv_t glGetTexGenfv;
typedef void (*glGetTexGeniv_t )(GLenum coord, GLenum pname, GLint *params);
glGetTexGeniv_t glGetTexGeniv;
typedef void (*glGetTexImage_t )(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
glGetTexImage_t glGetTexImage;
typedef void (*glGetTexLevelParameterfv_t )(GLenum target, GLint level, GLenum pname, GLfloat *params);
glGetTexLevelParameterfv_t glGetTexLevelParameterfv;
typedef void (*glGetTexLevelParameteriv_t )(GLenum target, GLint level, GLenum pname, GLint *params);
glGetTexLevelParameteriv_t glGetTexLevelParameteriv;
typedef void (*glGetTexParameterfv_t )(GLenum target, GLenum pname, GLfloat *params);
glGetTexParameterfv_t glGetTexParameterfv;
typedef void (*glGetTexParameteriv_t )(GLenum target, GLenum pname, GLint *params);
glGetTexParameteriv_t glGetTexParameteriv;
typedef void (*glHint_t )(GLenum target, GLenum mode);
glHint_t glHint;
typedef void (*glIndexMask_t )(GLuint mask);
glIndexMask_t glIndexMask;
typedef void (*glIndexPointer_t )(GLenum type, GLsizei stride, const GLvoid *pointer);
glIndexPointer_t glIndexPointer;
typedef void (*glIndexd_t )(GLdouble c);
glIndexd_t glIndexd;
typedef void (*glIndexdv_t )(const GLdouble *c);
glIndexdv_t glIndexdv;
typedef void (*glIndexf_t )(GLfloat c);
glIndexf_t glIndexf;
typedef void (*glIndexfv_t )(const GLfloat *c);
glIndexfv_t glIndexfv;
typedef void (*glIndexi_t )(GLint c);
glIndexi_t glIndexi;
typedef void (*glIndexiv_t )(const GLint *c);
glIndexiv_t glIndexiv;
typedef void (*glIndexs_t )(GLshort c);
glIndexs_t glIndexs;
typedef void (*glIndexsv_t )(const GLshort *c);
glIndexsv_t glIndexsv;
typedef void (*glIndexub_t )(GLubyte c);
glIndexub_t glIndexub;
typedef void (*glIndexubv_t )(const GLubyte *c);
glIndexubv_t glIndexubv;
typedef void (*glInitNames_t )(void);
glInitNames_t glInitNames;
typedef void (*glInterleavedArrays_t )(GLenum format, GLsizei stride, const GLvoid *pointer);
glInterleavedArrays_t glInterleavedArrays;
typedef GLboolean (*glIsEnabled_t )(GLenum cap);
glIsEnabled_t glIsEnabled;
typedef GLboolean (*glIsList_t )(GLuint list);
glIsList_t glIsList;
typedef GLboolean (*glIsTexture_t )(GLuint texture);
glIsTexture_t glIsTexture;
typedef void (*glLightModelf_t )(GLenum pname, GLfloat param);
glLightModelf_t glLightModelf;
typedef void (*glLightModelfv_t )(GLenum pname, const GLfloat *params);
glLightModelfv_t glLightModelfv;
typedef void (*glLightModeli_t )(GLenum pname, GLint param);
glLightModeli_t glLightModeli;
typedef void (*glLightModeliv_t )(GLenum pname, const GLint *params);
glLightModeliv_t glLightModeliv;
typedef void (*glLightf_t )(GLenum light, GLenum pname, GLfloat param);
glLightf_t glLightf;
typedef void (*glLightfv_t )(GLenum light, GLenum pname, const GLfloat *params);
glLightfv_t glLightfv;
typedef void (*glLighti_t )(GLenum light, GLenum pname, GLint param);
glLighti_t glLighti;
typedef void (*glLightiv_t )(GLenum light, GLenum pname, const GLint *params);
glLightiv_t glLightiv;
typedef void (*glLineStipple_t )(GLint factor, GLushort pattern);
glLineStipple_t glLineStipple;
typedef void (*glLineWidth_t )(GLfloat width);
glLineWidth_t glLineWidth;
typedef void (*glListBase_t )(GLuint base);
glListBase_t glListBase;
typedef void (*glLoadIdentity_t )(void);
glLoadIdentity_t glLoadIdentity;
typedef void (*glLoadMatrixd_t )(const GLdouble *m);
glLoadMatrixd_t glLoadMatrixd;
typedef void (*glLoadMatrixf_t )(const GLfloat *m);
glLoadMatrixf_t glLoadMatrixf;
typedef void (*glLoadName_t )(GLuint name);
glLoadName_t glLoadName;
typedef void (*glLogicOp_t )(GLenum opcode);
glLogicOp_t glLogicOp;
typedef void (*glMap1d_t )(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
glMap1d_t glMap1d;
typedef void (*glMap1f_t )(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
glMap1f_t glMap1f;
typedef void (*glMap2d_t )(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
glMap2d_t glMap2d;
typedef void (*glMap2f_t )(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
glMap2f_t glMap2f;
typedef void (*glMapGrid1d_t )(GLint un, GLdouble u1, GLdouble u2);
glMapGrid1d_t glMapGrid1d;
typedef void (*glMapGrid1f_t )(GLint un, GLfloat u1, GLfloat u2);
glMapGrid1f_t glMapGrid1f;
typedef void (*glMapGrid2d_t )(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
glMapGrid2d_t glMapGrid2d;
typedef void (*glMapGrid2f_t )(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
glMapGrid2f_t glMapGrid2f;
typedef void (*glMaterialf_t )(GLenum face, GLenum pname, GLfloat param);
glMaterialf_t glMaterialf;
typedef void (*glMaterialfv_t )(GLenum face, GLenum pname, const GLfloat *params);
glMaterialfv_t glMaterialfv;
typedef void (*glMateriali_t )(GLenum face, GLenum pname, GLint param);
glMateriali_t glMateriali;
typedef void (*glMaterialiv_t )(GLenum face, GLenum pname, const GLint *params);
glMaterialiv_t glMaterialiv;
typedef void (*glMatrixMode_t )(GLenum mode);
glMatrixMode_t glMatrixMode;
typedef void (*glMultMatrixd_t )(const GLdouble *m);
glMultMatrixd_t glMultMatrixd;
typedef void (*glMultMatrixf_t )(const GLfloat *m);
glMultMatrixf_t glMultMatrixf;
typedef void (*glNewList_t )(GLuint list, GLenum mode);
glNewList_t glNewList;
typedef void (*glNormal3b_t )(GLbyte nx, GLbyte ny, GLbyte nz);
glNormal3b_t glNormal3b;
typedef void (*glNormal3bv_t )(const GLbyte *v);
glNormal3bv_t glNormal3bv;
typedef void (*glNormal3d_t )(GLdouble nx, GLdouble ny, GLdouble nz);
glNormal3d_t glNormal3d;
typedef void (*glNormal3dv_t )(const GLdouble *v);
glNormal3dv_t glNormal3dv;
typedef void (*glNormal3f_t )(GLfloat nx, GLfloat ny, GLfloat nz);
glNormal3f_t glNormal3f;
typedef void (*glNormal3fv_t )(const GLfloat *v);
glNormal3fv_t glNormal3fv;
typedef void (*glNormal3i_t )(GLint nx, GLint ny, GLint nz);
glNormal3i_t glNormal3i;
typedef void (*glNormal3iv_t )(const GLint *v);
glNormal3iv_t glNormal3iv;
typedef void (*glNormal3s_t )(GLshort nx, GLshort ny, GLshort nz);
glNormal3s_t glNormal3s;
typedef void (*glNormal3sv_t )(const GLshort *v);
glNormal3sv_t glNormal3sv;
typedef void (*glNormalPointer_t )(GLenum type, GLsizei stride, const GLvoid *pointer);
glNormalPointer_t glNormalPointer;
typedef void (*glOrtho_t )(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
glOrtho_t glOrtho;
typedef void (*glPassThrough_t )(GLfloat token);
glPassThrough_t glPassThrough;
typedef void (*glPixelMapfv_t )(GLenum map, GLsizei mapsize, const GLfloat *values);
glPixelMapfv_t glPixelMapfv;
typedef void (*glPixelMapuiv_t )(GLenum map, GLsizei mapsize, const GLuint *values);
glPixelMapuiv_t glPixelMapuiv;
typedef void (*glPixelMapusv_t )(GLenum map, GLsizei mapsize, const GLushort *values);
glPixelMapusv_t glPixelMapusv;
typedef void (*glPixelStoref_t )(GLenum pname, GLfloat param);
glPixelStoref_t glPixelStoref;
typedef void (*glPixelStorei_t )(GLenum pname, GLint param);
glPixelStorei_t glPixelStorei;
typedef void (*glPixelTransferf_t )(GLenum pname, GLfloat param);
glPixelTransferf_t glPixelTransferf;
typedef void (*glPixelTransferi_t )(GLenum pname, GLint param);
glPixelTransferi_t glPixelTransferi;
typedef void (*glPixelZoom_t )(GLfloat xfactor, GLfloat yfactor);
glPixelZoom_t glPixelZoom;
typedef void (*glPointSize_t )(GLfloat size);
glPointSize_t glPointSize;
typedef void (*glPolygonMode_t )(GLenum face, GLenum mode);
glPolygonMode_t glPolygonMode;
typedef void (*glPolygonOffset_t )(GLfloat factor, GLfloat units);
glPolygonOffset_t glPolygonOffset;
typedef void (*glPolygonStipple_t )(const GLubyte *mask);
glPolygonStipple_t glPolygonStipple;
typedef void (*glPopAttrib_t )(void);
glPopAttrib_t glPopAttrib;
typedef void (*glPopClientAttrib_t )(void);
glPopClientAttrib_t glPopClientAttrib;
typedef void (*glPopMatrix_t )(void);
glPopMatrix_t glPopMatrix;
typedef void (*glPopName_t )(void);
glPopName_t glPopName;
typedef void (*glPrioritizeTextures_t )(GLsizei n, const GLuint *textures, const GLclampf *priorities);
glPrioritizeTextures_t glPrioritizeTextures;
typedef void (*glPushAttrib_t )(GLbitfield mask);
glPushAttrib_t glPushAttrib;
typedef void (*glPushClientAttrib_t )(GLbitfield mask);
glPushClientAttrib_t glPushClientAttrib;
typedef void (*glPushMatrix_t )(void);
glPushMatrix_t glPushMatrix;
typedef void (*glPushName_t )(GLuint name);
glPushName_t glPushName;
typedef void (*glRasterPos2d_t )(GLdouble x, GLdouble y);
glRasterPos2d_t glRasterPos2d;
typedef void (*glRasterPos2dv_t )(const GLdouble *v);
glRasterPos2dv_t glRasterPos2dv;
typedef void (*glRasterPos2f_t )(GLfloat x, GLfloat y);
glRasterPos2f_t glRasterPos2f;
typedef void (*glRasterPos2fv_t )(const GLfloat *v);
glRasterPos2fv_t glRasterPos2fv;
typedef void (*glRasterPos2i_t )(GLint x, GLint y);
glRasterPos2i_t glRasterPos2i;
typedef void (*glRasterPos2iv_t )(const GLint *v);
glRasterPos2iv_t glRasterPos2iv;
typedef void (*glRasterPos2s_t )(GLshort x, GLshort y);
glRasterPos2s_t glRasterPos2s;
typedef void (*glRasterPos2sv_t )(const GLshort *v);
glRasterPos2sv_t glRasterPos2sv;
typedef void (*glRasterPos3d_t )(GLdouble x, GLdouble y, GLdouble z);
glRasterPos3d_t glRasterPos3d;
typedef void (*glRasterPos3dv_t )(const GLdouble *v);
glRasterPos3dv_t glRasterPos3dv;
typedef void (*glRasterPos3f_t )(GLfloat x, GLfloat y, GLfloat z);
glRasterPos3f_t glRasterPos3f;
typedef void (*glRasterPos3fv_t )(const GLfloat *v);
glRasterPos3fv_t glRasterPos3fv;
typedef void (*glRasterPos3i_t )(GLint x, GLint y, GLint z);
glRasterPos3i_t glRasterPos3i;
typedef void (*glRasterPos3iv_t )(const GLint *v);
glRasterPos3iv_t glRasterPos3iv;
typedef void (*glRasterPos3s_t )(GLshort x, GLshort y, GLshort z);
glRasterPos3s_t glRasterPos3s;
typedef void (*glRasterPos3sv_t )(const GLshort *v);
glRasterPos3sv_t glRasterPos3sv;
typedef void (*glRasterPos4d_t )(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
glRasterPos4d_t glRasterPos4d;
typedef void (*glRasterPos4dv_t )(const GLdouble *v);
glRasterPos4dv_t glRasterPos4dv;
typedef void (*glRasterPos4f_t )(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
glRasterPos4f_t glRasterPos4f;
typedef void (*glRasterPos4fv_t )(const GLfloat *v);
glRasterPos4fv_t glRasterPos4fv;
typedef void (*glRasterPos4i_t )(GLint x, GLint y, GLint z, GLint w);
glRasterPos4i_t glRasterPos4i;
typedef void (*glRasterPos4iv_t )(const GLint *v);
glRasterPos4iv_t glRasterPos4iv;
typedef void (*glRasterPos4s_t )(GLshort x, GLshort y, GLshort z, GLshort w);
glRasterPos4s_t glRasterPos4s;
typedef void (*glRasterPos4sv_t )(const GLshort *v);
glRasterPos4sv_t glRasterPos4sv;
typedef void (*glReadBuffer_t )(GLenum mode);
glReadBuffer_t glReadBuffer;
typedef void (*glReadPixels_t )(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
glReadPixels_t glReadPixels;
typedef void (*glRectd_t )(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
glRectd_t glRectd;
typedef void (*glRectdv_t )(const GLdouble *v1, const GLdouble *v2);
glRectdv_t glRectdv;
typedef void (*glRectf_t )(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
glRectf_t glRectf;
typedef void (*glRectfv_t )(const GLfloat *v1, const GLfloat *v2);
glRectfv_t glRectfv;
typedef void (*glRecti_t )(GLint x1, GLint y1, GLint x2, GLint y2);
glRecti_t glRecti;
typedef void (*glRectiv_t )(const GLint *v1, const GLint *v2);
glRectiv_t glRectiv;
typedef void (*glRects_t )(GLshort x1, GLshort y1, GLshort x2, GLshort y2);
glRects_t glRects;
typedef void (*glRectsv_t )(const GLshort *v1, const GLshort *v2);
glRectsv_t glRectsv;
typedef GLint (*glRenderMode_t )(GLenum mode);
glRenderMode_t glRenderMode;
typedef void (*glRotated_t )(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
glRotated_t glRotated;
typedef void (*glRotatef_t )(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
glRotatef_t glRotatef;
typedef void (*glScaled_t )(GLdouble x, GLdouble y, GLdouble z);
glScaled_t glScaled;
typedef void (*glScalef_t )(GLfloat x, GLfloat y, GLfloat z);
glScalef_t glScalef;
typedef void (*glScissor_t )(GLint x, GLint y, GLsizei width, GLsizei height);
glScissor_t glScissor;
typedef void (*glSelectBuffer_t )(GLsizei size, GLuint *buffer);
glSelectBuffer_t glSelectBuffer;
typedef void (*glShadeModel_t )(GLenum mode);
glShadeModel_t glShadeModel;
typedef void (*glStencilFunc_t )(GLenum func, GLint ref, GLuint mask);
glStencilFunc_t glStencilFunc;
typedef void (*glStencilMask_t )(GLuint mask);
glStencilMask_t glStencilMask;
typedef void (*glStencilOp_t )(GLenum fail, GLenum zfail, GLenum zpass);
glStencilOp_t glStencilOp;
typedef void (*glTexCoord1d_t )(GLdouble s);
glTexCoord1d_t glTexCoord1d;
typedef void (*glTexCoord1dv_t )(const GLdouble *v);
glTexCoord1dv_t glTexCoord1dv;
typedef void (*glTexCoord1f_t )(GLfloat s);
glTexCoord1f_t glTexCoord1f;
typedef void (*glTexCoord1fv_t )(const GLfloat *v);
glTexCoord1fv_t glTexCoord1fv;
typedef void (*glTexCoord1i_t )(GLint s);
glTexCoord1i_t glTexCoord1i;
typedef void (*glTexCoord1iv_t )(const GLint *v);
glTexCoord1iv_t glTexCoord1iv;
typedef void (*glTexCoord1s_t )(GLshort s);
glTexCoord1s_t glTexCoord1s;
typedef void (*glTexCoord1sv_t )(const GLshort *v);
glTexCoord1sv_t glTexCoord1sv;
typedef void (*glTexCoord2d_t )(GLdouble s, GLdouble t);
glTexCoord2d_t glTexCoord2d;
typedef void (*glTexCoord2dv_t )(const GLdouble *v);
glTexCoord2dv_t glTexCoord2dv;
typedef void (*glTexCoord2f_t )(GLfloat s, GLfloat t);
glTexCoord2f_t glTexCoord2f;
typedef void (*glTexCoord2fv_t )(const GLfloat *v);
glTexCoord2fv_t glTexCoord2fv;
typedef void (*glTexCoord2i_t )(GLint s, GLint t);
glTexCoord2i_t glTexCoord2i;
typedef void (*glTexCoord2iv_t )(const GLint *v);
glTexCoord2iv_t glTexCoord2iv;
typedef void (*glTexCoord2s_t )(GLshort s, GLshort t);
glTexCoord2s_t glTexCoord2s;
typedef void (*glTexCoord2sv_t )(const GLshort *v);
glTexCoord2sv_t glTexCoord2sv;
typedef void (*glTexCoord3d_t )(GLdouble s, GLdouble t, GLdouble r);
glTexCoord3d_t glTexCoord3d;
typedef void (*glTexCoord3dv_t )(const GLdouble *v);
glTexCoord3dv_t glTexCoord3dv;
typedef void (*glTexCoord3f_t )(GLfloat s, GLfloat t, GLfloat r);
glTexCoord3f_t glTexCoord3f;
typedef void (*glTexCoord3fv_t )(const GLfloat *v);
glTexCoord3fv_t glTexCoord3fv;
typedef void (*glTexCoord3i_t )(GLint s, GLint t, GLint r);
glTexCoord3i_t glTexCoord3i;
typedef void (*glTexCoord3iv_t )(const GLint *v);
glTexCoord3iv_t glTexCoord3iv;
typedef void (*glTexCoord3s_t )(GLshort s, GLshort t, GLshort r);
glTexCoord3s_t glTexCoord3s;
typedef void (*glTexCoord3sv_t )(const GLshort *v);
glTexCoord3sv_t glTexCoord3sv;
typedef void (*glTexCoord4d_t )(GLdouble s, GLdouble t, GLdouble r, GLdouble q);
glTexCoord4d_t glTexCoord4d;
typedef void (*glTexCoord4dv_t )(const GLdouble *v);
glTexCoord4dv_t glTexCoord4dv;
typedef void (*glTexCoord4f_t )(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
glTexCoord4f_t glTexCoord4f;
typedef void (*glTexCoord4fv_t )(const GLfloat *v);
glTexCoord4fv_t glTexCoord4fv;
typedef void (*glTexCoord4i_t )(GLint s, GLint t, GLint r, GLint q);
glTexCoord4i_t glTexCoord4i;
typedef void (*glTexCoord4iv_t )(const GLint *v);
glTexCoord4iv_t glTexCoord4iv;
typedef void (*glTexCoord4s_t )(GLshort s, GLshort t, GLshort r, GLshort q);
glTexCoord4s_t glTexCoord4s;
typedef void (*glTexCoord4sv_t )(const GLshort *v);
glTexCoord4sv_t glTexCoord4sv;
typedef void (*glTexCoordPointer_t )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
glTexCoordPointer_t glTexCoordPointer;
typedef void (*glTexEnvf_t )(GLenum target, GLenum pname, GLfloat param);
glTexEnvf_t glTexEnvf;
typedef void (*glTexEnvfv_t )(GLenum target, GLenum pname, const GLfloat *params);
glTexEnvfv_t glTexEnvfv;
typedef void (*glTexEnvi_t )(GLenum target, GLenum pname, GLint param);
glTexEnvi_t glTexEnvi;
typedef void (*glTexEnviv_t )(GLenum target, GLenum pname, const GLint *params);
glTexEnviv_t glTexEnviv;
typedef void (*glTexGend_t )(GLenum coord, GLenum pname, GLdouble param);
glTexGend_t glTexGend;
typedef void (*glTexGendv_t )(GLenum coord, GLenum pname, const GLdouble *params);
glTexGendv_t glTexGendv;
typedef void (*glTexGenf_t )(GLenum coord, GLenum pname, GLfloat param);
glTexGenf_t glTexGenf;
typedef void (*glTexGenfv_t )(GLenum coord, GLenum pname, const GLfloat *params);
glTexGenfv_t glTexGenfv;
typedef void (*glTexGeni_t )(GLenum coord, GLenum pname, GLint param);
glTexGeni_t glTexGeni;
typedef void (*glTexGeniv_t )(GLenum coord, GLenum pname, const GLint *params);
glTexGeniv_t glTexGeniv;
typedef void (*glTexImage1D_t )(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
glTexImage1D_t glTexImage1D;
typedef void (*glTexImage2D_t )(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
glTexImage2D_t glTexImage2D;
typedef void (*glTexParameterf_t )(GLenum target, GLenum pname, GLfloat param);
glTexParameterf_t glTexParameterf;
typedef void (*glTexParameterfv_t )(GLenum target, GLenum pname, const GLfloat *params);
glTexParameterfv_t glTexParameterfv;
typedef void (*glTexParameteri_t )(GLenum target, GLenum pname, GLint param);
glTexParameteri_t glTexParameteri;
typedef void (*glTexParameteriv_t )(GLenum target, GLenum pname, const GLint *params);
glTexParameteriv_t glTexParameteriv;
typedef void (*glTexSubImage1D_t )(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
glTexSubImage1D_t glTexSubImage1D;
typedef void (*glTexSubImage2D_t )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
glTexSubImage2D_t glTexSubImage2D;
typedef void (*glTranslated_t )(GLdouble x, GLdouble y, GLdouble z);
glTranslated_t glTranslated;
typedef void (*glTranslatef_t )(GLfloat x, GLfloat y, GLfloat z);
glTranslatef_t glTranslatef;
typedef void (*glVertex2d_t )(GLdouble x, GLdouble y);
glVertex2d_t glVertex2d;
typedef void (*glVertex2dv_t )(const GLdouble *v);
glVertex2dv_t glVertex2dv;
typedef void (*glVertex2f_t )(GLfloat x, GLfloat y);
glVertex2f_t glVertex2f;
typedef void (*glVertex2fv_t )(const GLfloat *v);
glVertex2fv_t glVertex2fv;
typedef void (*glVertex2i_t )(GLint x, GLint y);
glVertex2i_t glVertex2i;
typedef void (*glVertex2iv_t )(const GLint *v);
glVertex2iv_t glVertex2iv;
typedef void (*glVertex2s_t )(GLshort x, GLshort y);
glVertex2s_t glVertex2s;
typedef void (*glVertex2sv_t )(const GLshort *v);
glVertex2sv_t glVertex2sv;
typedef void (*glVertex3d_t )(GLdouble x, GLdouble y, GLdouble z);
glVertex3d_t glVertex3d;
typedef void (*glVertex3dv_t )(const GLdouble *v);
glVertex3dv_t glVertex3dv;
typedef void (*glVertex3f_t )(GLfloat x, GLfloat y, GLfloat z);
glVertex3f_t glVertex3f;
typedef void (*glVertex3fv_t )(const GLfloat *v);
glVertex3fv_t glVertex3fv;
typedef void (*glVertex3i_t )(GLint x, GLint y, GLint z);
glVertex3i_t glVertex3i;
typedef void (*glVertex3iv_t )(const GLint *v);
glVertex3iv_t glVertex3iv;
typedef void (*glVertex3s_t )(GLshort x, GLshort y, GLshort z);
glVertex3s_t glVertex3s;
typedef void (*glVertex3sv_t )(const GLshort *v);
glVertex3sv_t glVertex3sv;
typedef void (*glVertex4d_t )(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
glVertex4d_t glVertex4d;
typedef void (*glVertex4dv_t )(const GLdouble *v);
glVertex4dv_t glVertex4dv;
typedef void (*glVertex4f_t )(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
glVertex4f_t glVertex4f;
typedef void (*glVertex4fv_t )(const GLfloat *v);
glVertex4fv_t glVertex4fv;
typedef void (*glVertex4i_t )(GLint x, GLint y, GLint z, GLint w);
glVertex4i_t glVertex4i;
typedef void (*glVertex4iv_t )(const GLint *v);
glVertex4iv_t glVertex4iv;
typedef void (*glVertex4s_t )(GLshort x, GLshort y, GLshort z, GLshort w);
glVertex4s_t glVertex4s;
typedef void (*glVertex4sv_t )(const GLshort *v);
glVertex4sv_t glVertex4sv;
typedef void (*glVertexPointer_t )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
glVertexPointer_t glVertexPointer;
typedef void (*glViewport_t )(GLint x, GLint y, GLsizei width, GLsizei height);
glViewport_t glViewport;


/* EXT_paletted_texture */
typedef void (*glColorTable_t)(GLenum target, GLenum internalFormat, GLsizei width, GLenum format, GLenum type, const void* data);
glColorTable_t glColorTableEXT;

/* EXT_compiled_vertex_array */
typedef void (*glLockArrays_t)(GLint first, GLsizei count);
typedef void (*glUnlockArrays_t)();
glLockArrays_t glLockArraysEXT;
glUnlockArrays_t glUnlockArraysEXT;

/* ARB_multitexture */
typedef void (*glActiveTextureARB_t)(GLenum target);
typedef void (*glClientActiveTextureARB_t)(GLenum target);
typedef void (*glMultiTexCoord2fARB_t)(GLenum texture, GLfloat, GLfloat);
typedef void (*glMultiTexCoord2fvARB_t)(GLenum texture, const GLfloat*);
glActiveTextureARB_t glActiveTextureARB;
glClientActiveTextureARB_t glClientActiveTextureARB;
glMultiTexCoord2fARB_t glMultiTexCoord2fARB;
glMultiTexCoord2fvARB_t glMultiTexCoord2fvARB;

/* NV_vertex_array_range */
typedef void (*glVertexArrayRange_t)(GLsizei length, const GLvoid* pointer);
typedef void (*glFlushVertexArrayRange_t)();
typedef void* (*glXAllocateMemory_t)(GLsizei, GLfloat, GLfloat, GLfloat);
typedef void (*glXFreeMemory_t)(void*);
glVertexArrayRange_t glVertexArrayRangeNV;
glFlushVertexArrayRange_t glFlushVertexArrayRangeNV;
glXAllocateMemory_t glXAllocateMemoryNV;
glXFreeMemory_t glXFreeMemoryNV;

/* EXT_fog_coord */
typedef void (*glFogCoordf_t)(GLfloat);
typedef void (*glFogCoordPointer_t)(GLenum, GLsizei, const GLvoid*);
glFogCoordf_t glFogCoordfEXT;
glFogCoordPointer_t glFogCoordPointerEXT;

/* ARB_texture_compression */
typedef void (*glCompressedTexImage3DARB_t)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid*);
typedef void (*glCompressedTexImage2DARB_t)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid*);
typedef void (*glCompressedTexImage1DARB_t)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid*);
typedef void (*glCompressedTexSubImage3DARB_t)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid*);
typedef void (*glCompressedTexSubImage2DARB_t)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid*);
typedef void (*glCompressedTexSubImage1DARB_t)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid*);
typedef void (*glGetCompressedTexImageARB_t)(GLenum target, GLint lod, GLvoid* img);
glCompressedTexImage3DARB_t glCompressedTexImage3DARB;
glCompressedTexImage2DARB_t glCompressedTexImage2DARB;
glCompressedTexImage1DARB_t glCompressedTexImage1DARB;
glCompressedTexSubImage3DARB_t glCompressedTexSubImage3DARB;
glCompressedTexSubImage2DARB_t glCompressedTexSubImage2DARB;
glCompressedTexSubImage1DARB_t glCompressedTexSubImage1DARB;
glGetCompressedTexImageARB_t glGetCompressedTexImageARB;

/* EXT_vertex_buffer */
typedef GLboolean (*glAvailableVertexBufferEXT_t)();
typedef GLint (*glAllocateVertexBufferEXT_t)(GLsizei size, GLint format, GLboolean preserve);
typedef void* (*glLockVertexBufferEXT_t)(GLint handle, GLsizei size);
typedef void (*glUnlockVertexBufferEXT_t)(GLint handle);
typedef void (*glSetVertexBufferEXT_t)(GLint handle);
typedef void (*glOffsetVertexBufferEXT_t)(GLint handle, GLuint offset);
typedef void (*glFillVertexBufferEXT_t)(GLint handle, GLint first, GLsizei count);
typedef void (*glFreeVertexBufferEXT_t)(GLint handle);
glAvailableVertexBufferEXT_t glAvailableVertexBufferEXT;
glAllocateVertexBufferEXT_t glAllocateVertexBufferEXT;
glLockVertexBufferEXT_t glLockVertexBufferEXT;
glUnlockVertexBufferEXT_t glUnlockVertexBufferEXT;
glSetVertexBufferEXT_t glSetVertexBufferEXT;
glOffsetVertexBufferEXT_t glOffsetVertexBufferEXT;
glFillVertexBufferEXT_t glFillVertexBufferEXT;
glFreeVertexBufferEXT_t glFreeVertexBufferEXT;

// From the Mesa implementation of GLU. Used with permission from Brian Paul.

/*
 * Transform a point (column vector) by a 4x4 matrix.  I.e.  out = m * in
 * Input:  m - the 4x4 matrix
 *         in - the 4x1 vector
 * Output:  out - the resulting 4x1 vector.
 */
static void
transform_point(GLdouble out[4], const GLdouble m[16], const GLdouble in[4])
{
#define M(row,col)  m[col*4+row]
   out[0] =
      M(0, 0) * in[0] + M(0, 1) * in[1] + M(0, 2) * in[2] + M(0, 3) * in[3];
   out[1] =
      M(1, 0) * in[0] + M(1, 1) * in[1] + M(1, 2) * in[2] + M(1, 3) * in[3];
   out[2] =
      M(2, 0) * in[0] + M(2, 1) * in[1] + M(2, 2) * in[2] + M(2, 3) * in[3];
   out[3] =
      M(3, 0) * in[0] + M(3, 1) * in[1] + M(3, 2) * in[2] + M(3, 3) * in[3];
#undef M
}

/*
 * Perform a 4x4 matrix multiplication  (product = a x b).
 * Input:  a, b - matrices to multiply
 * Output:  product - product of a and b
 */
static void
matmul(GLdouble * product, const GLdouble * a, const GLdouble * b)
{
   /* This matmul was contributed by Thomas Malik */
   GLdouble temp[16];
   GLint i;

#define A(row,col)  a[(col<<2)+row]
#define B(row,col)  b[(col<<2)+row]
#define T(row,col)  temp[(col<<2)+row]

   /* i-te Zeile */
   for (i = 0; i < 4; i++) {
      T(i, 0) =
	 A(i, 0) * B(0, 0) + A(i, 1) * B(1, 0) + A(i, 2) * B(2, 0) + A(i,
								       3) *
	 B(3, 0);
      T(i, 1) =
	 A(i, 0) * B(0, 1) + A(i, 1) * B(1, 1) + A(i, 2) * B(2, 1) + A(i,
								       3) *
	 B(3, 1);
      T(i, 2) =
	 A(i, 0) * B(0, 2) + A(i, 1) * B(1, 2) + A(i, 2) * B(2, 2) + A(i,
								       3) *
	 B(3, 2);
      T(i, 3) =
	 A(i, 0) * B(0, 3) + A(i, 1) * B(1, 3) + A(i, 2) * B(2, 3) + A(i,
								       3) *
	 B(3, 3);
   }

#undef A
#undef B
#undef T
   memcpy(product, temp, 16 * sizeof(GLdouble));
}

/*
 * Compute inverse of 4x4 transformation matrix.
 * Code contributed by Jacques Leroy jle@star.be
 * Return GL_TRUE for success, GL_FALSE for failure (singular matrix)
 */
static GLboolean
invert_matrix(const GLdouble * m, GLdouble * out)
{
/* NB. OpenGL Matrices are COLUMN major. */
#define SWAP_ROWS(a, b) { GLdouble *_tmp = a; (a)=(b); (b)=_tmp; }
#define MAT(m,r,c) (m)[(c)*4+(r)]

   GLdouble wtmp[4][8];
   GLdouble m0, m1, m2, m3, s;
   GLdouble *r0, *r1, *r2, *r3;

   r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];

   r0[0] = MAT(m, 0, 0), r0[1] = MAT(m, 0, 1),
      r0[2] = MAT(m, 0, 2), r0[3] = MAT(m, 0, 3),
      r0[4] = 1.0, r0[5] = r0[6] = r0[7] = 0.0,
      r1[0] = MAT(m, 1, 0), r1[1] = MAT(m, 1, 1),
      r1[2] = MAT(m, 1, 2), r1[3] = MAT(m, 1, 3),
      r1[5] = 1.0, r1[4] = r1[6] = r1[7] = 0.0,
      r2[0] = MAT(m, 2, 0), r2[1] = MAT(m, 2, 1),
      r2[2] = MAT(m, 2, 2), r2[3] = MAT(m, 2, 3),
      r2[6] = 1.0, r2[4] = r2[5] = r2[7] = 0.0,
      r3[0] = MAT(m, 3, 0), r3[1] = MAT(m, 3, 1),
      r3[2] = MAT(m, 3, 2), r3[3] = MAT(m, 3, 3),
      r3[7] = 1.0, r3[4] = r3[5] = r3[6] = 0.0;

   /* choose pivot - or die */
   if (fabs(r3[0]) > fabs(r2[0]))
      SWAP_ROWS(r3, r2);
   if (fabs(r2[0]) > fabs(r1[0]))
      SWAP_ROWS(r2, r1);
   if (fabs(r1[0]) > fabs(r0[0]))
      SWAP_ROWS(r1, r0);
   if (0.0 == r0[0])
      return GL_FALSE;

   /* eliminate first variable     */
   m1 = r1[0] / r0[0];
   m2 = r2[0] / r0[0];
   m3 = r3[0] / r0[0];
   s = r0[1];
   r1[1] -= m1 * s;
   r2[1] -= m2 * s;
   r3[1] -= m3 * s;
   s = r0[2];
   r1[2] -= m1 * s;
   r2[2] -= m2 * s;
   r3[2] -= m3 * s;
   s = r0[3];
   r1[3] -= m1 * s;
   r2[3] -= m2 * s;
   r3[3] -= m3 * s;
   s = r0[4];
   if (s != 0.0) {
      r1[4] -= m1 * s;
      r2[4] -= m2 * s;
      r3[4] -= m3 * s;
   }
   s = r0[5];
   if (s != 0.0) {
      r1[5] -= m1 * s;
      r2[5] -= m2 * s;
      r3[5] -= m3 * s;
   }
   s = r0[6];
   if (s != 0.0) {
      r1[6] -= m1 * s;
      r2[6] -= m2 * s;
      r3[6] -= m3 * s;
   }
   s = r0[7];
   if (s != 0.0) {
      r1[7] -= m1 * s;
      r2[7] -= m2 * s;
      r3[7] -= m3 * s;
   }

   /* choose pivot - or die */
   if (fabs(r3[1]) > fabs(r2[1]))
      SWAP_ROWS(r3, r2);
   if (fabs(r2[1]) > fabs(r1[1]))
      SWAP_ROWS(r2, r1);
   if (0.0 == r1[1])
      return GL_FALSE;

   /* eliminate second variable */
   m2 = r2[1] / r1[1];
   m3 = r3[1] / r1[1];
   r2[2] -= m2 * r1[2];
   r3[2] -= m3 * r1[2];
   r2[3] -= m2 * r1[3];
   r3[3] -= m3 * r1[3];
   s = r1[4];
   if (0.0 != s) {
      r2[4] -= m2 * s;
      r3[4] -= m3 * s;
   }
   s = r1[5];
   if (0.0 != s) {
      r2[5] -= m2 * s;
      r3[5] -= m3 * s;
   }
   s = r1[6];
   if (0.0 != s) {
      r2[6] -= m2 * s;
      r3[6] -= m3 * s;
   }
   s = r1[7];
   if (0.0 != s) {
      r2[7] -= m2 * s;
      r3[7] -= m3 * s;
   }

   /* choose pivot - or die */
   if (fabs(r3[2]) > fabs(r2[2]))
      SWAP_ROWS(r3, r2);
   if (0.0 == r2[2])
      return GL_FALSE;

   /* eliminate third variable */
   m3 = r3[2] / r2[2];
   r3[3] -= m3 * r2[3], r3[4] -= m3 * r2[4],
      r3[5] -= m3 * r2[5], r3[6] -= m3 * r2[6], r3[7] -= m3 * r2[7];

   /* last check */
   if (0.0 == r3[3])
      return GL_FALSE;

   s = 1.0 / r3[3];		/* now back substitute row 3 */
   r3[4] *= s;
   r3[5] *= s;
   r3[6] *= s;
   r3[7] *= s;

   m2 = r2[3];			/* now back substitute row 2 */
   s = 1.0 / r2[2];
   r2[4] = s * (r2[4] - r3[4] * m2), r2[5] = s * (r2[5] - r3[5] * m2),
      r2[6] = s * (r2[6] - r3[6] * m2), r2[7] = s * (r2[7] - r3[7] * m2);
   m1 = r1[3];
   r1[4] -= r3[4] * m1, r1[5] -= r3[5] * m1,
      r1[6] -= r3[6] * m1, r1[7] -= r3[7] * m1;
   m0 = r0[3];
   r0[4] -= r3[4] * m0, r0[5] -= r3[5] * m0,
      r0[6] -= r3[6] * m0, r0[7] -= r3[7] * m0;

   m1 = r1[2];			/* now back substitute row 1 */
   s = 1.0 / r1[1];
   r1[4] = s * (r1[4] - r2[4] * m1), r1[5] = s * (r1[5] - r2[5] * m1),
      r1[6] = s * (r1[6] - r2[6] * m1), r1[7] = s * (r1[7] - r2[7] * m1);
   m0 = r0[2];
   r0[4] -= r2[4] * m0, r0[5] -= r2[5] * m0,
      r0[6] -= r2[6] * m0, r0[7] -= r2[7] * m0;

   m0 = r0[1];			/* now back substitute row 0 */
   s = 1.0 / r0[0];
   r0[4] = s * (r0[4] - r1[4] * m0), r0[5] = s * (r0[5] - r1[5] * m0),
      r0[6] = s * (r0[6] - r1[6] * m0), r0[7] = s * (r0[7] - r1[7] * m0);

   MAT(out, 0, 0) = r0[4];
   MAT(out, 0, 1) = r0[5], MAT(out, 0, 2) = r0[6];
   MAT(out, 0, 3) = r0[7], MAT(out, 1, 0) = r1[4];
   MAT(out, 1, 1) = r1[5], MAT(out, 1, 2) = r1[6];
   MAT(out, 1, 3) = r1[7], MAT(out, 2, 0) = r2[4];
   MAT(out, 2, 1) = r2[5], MAT(out, 2, 2) = r2[6];
   MAT(out, 2, 3) = r2[7], MAT(out, 3, 0) = r3[4];
   MAT(out, 3, 1) = r3[5], MAT(out, 3, 2) = r3[6];
   MAT(out, 3, 3) = r3[7];

   return GL_TRUE;

#undef MAT
#undef SWAP_ROWS
}

/* projection du point (objx,objy,obz) sur l'ecran (winx,winy,winz) */
GLint
mesa_gluProject(GLdouble objx, GLdouble objy, GLdouble objz,
		const GLdouble model[16], const GLdouble proj[16],
		const GLint viewport[4],
		GLdouble * winx, GLdouble * winy, GLdouble * winz)
{
   /* matrice de transformation */
   GLdouble in[4], out[4];

   /* initilise la matrice et le vecteur a transformer */
   in[0] = objx;
   in[1] = objy;
   in[2] = objz;
   in[3] = 1.0;
   transform_point(out, model, in);
   transform_point(in, proj, out);

   /* d'ou le resultat normalise entre -1 et 1 */
   if (in[3] == 0.0)
      return GL_FALSE;

   in[0] /= in[3];
   in[1] /= in[3];
   in[2] /= in[3];

   /* en coordonnees ecran */
   *winx = viewport[0] + (1 + in[0]) * viewport[2] / 2;
   *winy = viewport[1] + (1 + in[1]) * viewport[3] / 2;
   /* entre 0 et 1 suivant z */
   *winz = (1 + in[2]) / 2;
   return GL_TRUE;
}

/* transformation du point ecran (winx,winy,winz) en point objet */
GLint
mesa_gluUnProject(GLdouble winx, GLdouble winy, GLdouble winz,
		  const GLdouble model[16], const GLdouble proj[16],
		  const GLint viewport[4],
		  GLdouble * objx, GLdouble * objy, GLdouble * objz)
{
   /* matrice de transformation */
   GLdouble m[16], A[16];
   GLdouble in[4], out[4];

   /* transformation coordonnees normalisees entre -1 et 1 */
   in[0] = (winx - viewport[0]) * 2 / viewport[2] - 1.0;
   in[1] = (winy - viewport[1]) * 2 / viewport[3] - 1.0;
   in[2] = 2 * winz - 1.0;
   in[3] = 1.0;

   /* calcul transformation inverse */
   matmul(A, proj, model);
   invert_matrix(A, m);

   /* d'ou les coordonnees objets */
   transform_point(out, m, in);
   if (out[3] == 0.0)
      return GL_FALSE;
   *objx = out[0] / out[3];
   *objy = out[1] / out[3];
   *objz = out[2] / out[3];
   return GL_TRUE;
}

void QGL_Shutdown( void )
{
	// GLU Functions
	gluErrorString              = NULL;
	gluGetString                = NULL;
	gluOrtho2D                  = NULL;
	gluPerspective              = NULL;
	gluPickMatrix               = NULL;
	gluLookAt                   = NULL;
	gluProject                  = NULL;
	gluUnProject                = NULL;
	gluScaleImage               = NULL;
	gluBuild1DMipmaps           = NULL;
	gluBuild2DMipmaps           = NULL;

	// GL Functions
	glAccum                     = NULL;
	glAlphaFunc                 = NULL;
	glAreTexturesResident       = NULL;
	glArrayElement              = NULL;
	glBegin                     = NULL;
	glBindTexture               = NULL;
	glBitmap                    = NULL;
	glBlendFunc                 = NULL;
	glCallList                  = NULL;
	glCallLists                 = NULL;
	glClear                     = NULL;
	glClearAccum                = NULL;
	glClearColor                = NULL;
	glClearDepth                = NULL;
	glClearIndex                = NULL;
	glClearStencil              = NULL;
	glClipPlane                 = NULL;
	glColor3b                   = NULL;
	glColor3bv                  = NULL;
	glColor3d                   = NULL;
	glColor3dv                  = NULL;
	glColor3f                   = NULL;
	glColor3fv                  = NULL;
	glColor3i                   = NULL;
	glColor3iv                  = NULL;
	glColor3s                   = NULL;
	glColor3sv                  = NULL;
	glColor3ub                  = NULL;
	glColor3ubv                 = NULL;
	glColor3ui                  = NULL;
	glColor3uiv                 = NULL;
	glColor3us                  = NULL;
	glColor3usv                 = NULL;
	glColor4b                   = NULL;
	glColor4bv                  = NULL;
	glColor4d                   = NULL;
	glColor4dv                  = NULL;
	glColor4f                   = NULL;
	glColor4fv                  = NULL;
	glColor4i                   = NULL;
	glColor4iv                  = NULL;
	glColor4s                   = NULL;
	glColor4sv                  = NULL;
	glColor4ub                  = NULL;
	glColor4ubv                 = NULL;
	glColor4ui                  = NULL;
	glColor4uiv                 = NULL;
	glColor4us                  = NULL;
	glColor4usv                 = NULL;
	glColorMask                 = NULL;
	glColorMaterial             = NULL;
	glColorPointer              = NULL;
	glCopyPixels                = NULL;
	glCopyTexImage1D            = NULL;
	glCopyTexImage2D            = NULL;
	glCopyTexSubImage1D         = NULL;
	glCopyTexSubImage2D         = NULL;
	glCullFace                  = NULL;
	glDeleteLists               = NULL;
	glDeleteTextures            = NULL;
	glDepthFunc                 = NULL;
	glDepthMask                 = NULL;
	glDepthRange                = NULL;
	glDisable                   = NULL;
	glDisableClientState        = NULL;
	glDrawArrays                = NULL;
	glDrawBuffer                = NULL;
	glDrawElements              = NULL;
	glDrawPixels                = NULL;
	glEdgeFlag                  = NULL;
	glEdgeFlagPointer           = NULL;
	glEdgeFlagv                 = NULL;
	glEnable                    = NULL;
	glEnableClientState         = NULL;
	glEnd                       = NULL;
	glEndList                   = NULL;
	glEvalCoord1d               = NULL;
	glEvalCoord1dv              = NULL;
	glEvalCoord1f               = NULL;
	glEvalCoord1fv              = NULL;
	glEvalCoord2d               = NULL;
	glEvalCoord2dv              = NULL;
	glEvalCoord2f               = NULL;
	glEvalCoord2fv              = NULL;
	glEvalMesh1                 = NULL;
	glEvalMesh2                 = NULL;
	glEvalPoint1                = NULL;
	glEvalPoint2                = NULL;
	glFeedbackBuffer            = NULL;
	glFinish                    = NULL;
	glFlush                     = NULL;
	glFogf                      = NULL;
	glFogfv                     = NULL;
	glFogi                      = NULL;
	glFogiv                     = NULL;
	glFrontFace                 = NULL;
	glFrustum                   = NULL;
	glGenLists                  = NULL;
	glGenTextures               = NULL;
	glGetBooleanv               = NULL;
	glGetClipPlane              = NULL;
	glGetDoublev                = NULL;
	glGetError                  = NULL;
	glGetFloatv                 = NULL;
	glGetIntegerv               = NULL;
	glGetLightfv                = NULL;
	glGetLightiv                = NULL;
	glGetMapdv                  = NULL;
	glGetMapfv                  = NULL;
	glGetMapiv                  = NULL;
	glGetMaterialfv             = NULL;
	glGetMaterialiv             = NULL;
	glGetPixelMapfv             = NULL;
	glGetPixelMapuiv            = NULL;
	glGetPixelMapusv            = NULL;
	glGetPointerv               = NULL;
	glGetPolygonStipple         = NULL;
	glGetString                 = NULL;
	glGetTexEnvfv               = NULL;
	glGetTexEnviv               = NULL;
	glGetTexGendv               = NULL;
	glGetTexGenfv               = NULL;
	glGetTexGeniv               = NULL;
	glGetTexImage               = NULL;
	glGetTexLevelParameterfv    = NULL;
	glGetTexLevelParameteriv    = NULL;
	glGetTexParameterfv         = NULL;
	glGetTexParameteriv         = NULL;
	glHint                      = NULL;
	glIndexMask                 = NULL;
	glIndexPointer              = NULL;
	glIndexd                    = NULL;
	glIndexdv                   = NULL;
	glIndexf                    = NULL;
	glIndexfv                   = NULL;
	glIndexi                    = NULL;
	glIndexiv                   = NULL;
	glIndexs                    = NULL;
	glIndexsv                   = NULL;
	glIndexub                   = NULL;
	glIndexubv                  = NULL;
	glInitNames                 = NULL;
	glInterleavedArrays         = NULL;
	glIsEnabled                 = NULL;
	glIsList                    = NULL;
	glIsTexture                 = NULL;
	glLightModelf               = NULL;
	glLightModelfv              = NULL;
	glLightModeli               = NULL;
	glLightModeliv              = NULL;
	glLightf                    = NULL;
	glLightfv                   = NULL;
	glLighti                    = NULL;
	glLightiv                   = NULL;
	glLineStipple               = NULL;
	glLineWidth                 = NULL;
	glListBase                  = NULL;
	glLoadIdentity              = NULL;
	glLoadMatrixd               = NULL;
	glLoadMatrixf               = NULL;
	glLoadName                  = NULL;
	glLogicOp                   = NULL;
	glMap1d                     = NULL;
	glMap1f                     = NULL;
	glMap2d                     = NULL;
	glMap2f                     = NULL;
	glMapGrid1d                 = NULL;
	glMapGrid1f                 = NULL;
	glMapGrid2d                 = NULL;
	glMapGrid2f                 = NULL;
	glMaterialf                 = NULL;
	glMaterialfv                = NULL;
	glMateriali                 = NULL;
	glMaterialiv                = NULL;
	glMatrixMode                = NULL;
	glMultMatrixd               = NULL;
	glMultMatrixf               = NULL;
	glNewList                   = NULL;
	glNormal3b                  = NULL;
	glNormal3bv                 = NULL;
	glNormal3d                  = NULL;
	glNormal3dv                 = NULL;
	glNormal3f                  = NULL;
	glNormal3fv                 = NULL;
	glNormal3i                  = NULL;
	glNormal3iv                 = NULL;
	glNormal3s                  = NULL;
	glNormal3sv                 = NULL;
	glNormalPointer             = NULL;
	glOrtho                     = NULL;
	glPassThrough               = NULL;
	glPixelMapfv                = NULL;
	glPixelMapuiv               = NULL;
	glPixelMapusv               = NULL;
	glPixelStoref               = NULL;
	glPixelStorei               = NULL;
	glPixelTransferf            = NULL;
	glPixelTransferi            = NULL;
	glPixelZoom                 = NULL;
	glPointSize                 = NULL;
	glPolygonMode               = NULL;
	glPolygonOffset             = NULL;
	glPolygonStipple            = NULL;
	glPopAttrib                 = NULL;
	glPopClientAttrib           = NULL;
	glPopMatrix                 = NULL;
	glPopName                   = NULL;
	glPrioritizeTextures        = NULL;
	glPushAttrib                = NULL;
	glPushClientAttrib          = NULL;
	glPushMatrix                = NULL;
	glPushName                  = NULL;
	glRasterPos2d               = NULL;
	glRasterPos2dv              = NULL;
	glRasterPos2f               = NULL;
	glRasterPos2fv              = NULL;
	glRasterPos2i               = NULL;
	glRasterPos2iv              = NULL;
	glRasterPos2s               = NULL;
	glRasterPos2sv              = NULL;
	glRasterPos3d               = NULL;
	glRasterPos3dv              = NULL;
	glRasterPos3f               = NULL;
	glRasterPos3fv              = NULL;
	glRasterPos3i               = NULL;
	glRasterPos3iv              = NULL;
	glRasterPos3s               = NULL;
	glRasterPos3sv              = NULL;
	glRasterPos4d               = NULL;
	glRasterPos4dv              = NULL;
	glRasterPos4f               = NULL;
	glRasterPos4fv              = NULL;
	glRasterPos4i               = NULL;
	glRasterPos4iv              = NULL;
	glRasterPos4s               = NULL;
	glRasterPos4sv              = NULL;
	glReadBuffer                = NULL;
	glReadPixels                = NULL;
	glRectd                     = NULL;
	glRectdv                    = NULL;
	glRectf                     = NULL;
	glRectfv                    = NULL;
	glRecti                     = NULL;
	glRectiv                    = NULL;
	glRects                     = NULL;
	glRectsv                    = NULL;
	glRenderMode                = NULL;
	glRotated                   = NULL;
	glRotatef                   = NULL;
	glScaled                    = NULL;
	glScalef                    = NULL;
	glScissor                   = NULL;
	glSelectBuffer              = NULL;
	glShadeModel                = NULL;
	glStencilFunc               = NULL;
	glStencilMask               = NULL;
	glStencilOp                 = NULL;
	glTexCoord1d                = NULL;
	glTexCoord1dv               = NULL;
	glTexCoord1f                = NULL;
	glTexCoord1fv               = NULL;
	glTexCoord1i                = NULL;
	glTexCoord1iv               = NULL;
	glTexCoord1s                = NULL;
	glTexCoord1sv               = NULL;
	glTexCoord2d                = NULL;
	glTexCoord2dv               = NULL;
	glTexCoord2f                = NULL;
	glTexCoord2fv               = NULL;
	glTexCoord2i                = NULL;
	glTexCoord2iv               = NULL;
	glTexCoord2s                = NULL;
	glTexCoord2sv               = NULL;
	glTexCoord3d                = NULL;
	glTexCoord3dv               = NULL;
	glTexCoord3f                = NULL;
	glTexCoord3fv               = NULL;
	glTexCoord3i                = NULL;
	glTexCoord3iv               = NULL;
	glTexCoord3s                = NULL;
	glTexCoord3sv               = NULL;
	glTexCoord4d                = NULL;
	glTexCoord4dv               = NULL;
	glTexCoord4f                = NULL;
	glTexCoord4fv               = NULL;
	glTexCoord4i                = NULL;
	glTexCoord4iv               = NULL;
	glTexCoord4s                = NULL;
	glTexCoord4sv               = NULL;
	glTexCoordPointer           = NULL;
	glTexEnvf                   = NULL;
	glTexEnvfv                  = NULL;
	glTexEnvi                   = NULL;
	glTexEnviv                  = NULL;
	glTexGend                   = NULL;
	glTexGendv                  = NULL;
	glTexGenf                   = NULL;
	glTexGenfv                  = NULL;
	glTexGeni                   = NULL;
	glTexGeniv                  = NULL;
	glTexImage1D                = NULL;
	glTexImage2D                = NULL;
	glTexParameterf             = NULL;
	glTexParameterfv            = NULL;
	glTexParameteri             = NULL;
	glTexParameteriv            = NULL;
	glTexSubImage1D             = NULL;
	glTexSubImage2D             = NULL;
	glTranslated                = NULL;
	glTranslatef                = NULL;
	glVertex2d                  = NULL;
	glVertex2dv                 = NULL;
	glVertex2f                  = NULL;
	glVertex2fv                 = NULL;
	glVertex2i                  = NULL;
	glVertex2iv                 = NULL;
	glVertex2s                  = NULL;
	glVertex2sv                 = NULL;
	glVertex3d                  = NULL;
	glVertex3dv                 = NULL;
	glVertex3f                  = NULL;
	glVertex3fv                 = NULL;
	glVertex3i                  = NULL;
	glVertex3iv                 = NULL;
	glVertex3s                  = NULL;
	glVertex3sv                 = NULL;
	glVertex4d                  = NULL;
	glVertex4dv                 = NULL;
	glVertex4f                  = NULL;
	glVertex4fv                 = NULL;
	glVertex4i                  = NULL;
	glVertex4iv                 = NULL;
	glVertex4s                  = NULL;
	glVertex4sv                 = NULL;
	glVertexPointer             = NULL;
	glViewport                  = NULL;

	// EXT_compiled_vertex_array
	glLockArraysEXT             = NULL;
	glUnlockArraysEXT           = NULL;

	// ARB_multitexture
	glActiveTextureARB          = NULL;
	glClientActiveTextureARB    = NULL;
	glMultiTexCoord2fARB        = NULL;
	glMultiTexCoord2fvARB       = NULL;

	// NV_vertex_array_range
	glVertexArrayRangeNV        = NULL;
	glFlushVertexArrayRangeNV   = NULL;
	glXAllocateMemoryNV         = NULL;
	glXFreeMemoryNV             = NULL;

	// EXT_fog_coord
	glFogCoordfEXT              = NULL;
	glFogCoordPointerEXT        = NULL;

	// ARB_texture_compression
	glCompressedTexImage3DARB    = NULL;
	glCompressedTexImage2DARB    = NULL;
	glCompressedTexImage1DARB    = NULL;
	glCompressedTexSubImage3DARB = NULL;
	glCompressedTexSubImage2DARB = NULL;
	glCompressedTexSubImage1DARB = NULL;
	glGetCompressedTexImageARB   = NULL;

	// EXT_vertex_buffer
	glAvailableVertexBufferEXT	= NULL;
	glAllocateVertexBufferEXT	= NULL;
	glLockVertexBufferEXT		= NULL;
	glUnlockVertexBufferEXT		= NULL;
	glSetVertexBufferEXT			= NULL;
	glOffsetVertexBufferEXT		= NULL;
	glFillVertexBufferEXT		= NULL;
	glFreeVertexBufferEXT		= NULL;
}

// NOTE: helpers--anything you want to stub you can
// assign to this, anything you want to trigger a 
// breakpoint at run-time, assign to debug_stub
static void void_stub( void )
{
	// empty
}

static void debug_stub( void )
{
	__asm__( "int $03" );
}

bool QGL_Init( const char* dllname_gl, const char* dllname_glu )
{
#ifndef DEDICATED
	// NOTE: the video subsystem *MUST* be shutdown at this
	// point, or crap will break.

	if( SDL_GL_LoadLibrary( dllname_gl ) == -1 ) {
		return false;
	}

	gluProject = mesa_gluProject;
	gluUnProject = mesa_gluUnProject;

#define GPA_GL(a) SDL_GL_GetProcAddress( a )
	glAccum = (glAccum_t) GPA_GL( "glAccum" );
	glAlphaFunc = (glAlphaFunc_t) GPA_GL( "glAlphaFunc" );
	glAreTexturesResident = (glAreTexturesResident_t) GPA_GL( "glAreTexturesResident" );
	glArrayElement = (glArrayElement_t) GPA_GL( "glArrayElement" );
	glBegin = (glBegin_t) GPA_GL( "glBegin" );
	glBindTexture = (glBindTexture_t) GPA_GL( "glBindTexture" );
	glBitmap = (glBitmap_t) GPA_GL( "glBitmap" );
	glBlendFunc = (glBlendFunc_t) GPA_GL( "glBlendFunc" );
	glCallList = (glCallList_t) GPA_GL( "glCallList" );
	glCallLists = (glCallLists_t) GPA_GL( "glCallLists" );
	glClear = (glClear_t) GPA_GL( "glClear" );
	glClearAccum = (glClearAccum_t) GPA_GL( "glClearAccum" );
	glClearColor = (glClearColor_t) GPA_GL( "glClearColor" );
	glClearDepth = (glClearDepth_t) GPA_GL( "glClearDepth" );
	glClearIndex = (glClearIndex_t) GPA_GL( "glClearIndex" );
	glClearStencil = (glClearStencil_t) GPA_GL( "glClearStencil" );
	glClipPlane = (glClipPlane_t) GPA_GL( "glClipPlane" );
	glColor3b = (glColor3b_t) GPA_GL( "glColor3b" );
	glColor3bv = (glColor3bv_t) GPA_GL( "glColor3bv" );
	glColor3d = (glColor3d_t) GPA_GL( "glColor3d" );
	glColor3dv = (glColor3dv_t) GPA_GL( "glColor3dv" );
	glColor3f = (glColor3f_t) GPA_GL( "glColor3f" );
	glColor3fv = (glColor3fv_t) GPA_GL( "glColor3fv" );
	glColor3i = (glColor3i_t) GPA_GL( "glColor3i" );
	glColor3iv = (glColor3iv_t) GPA_GL( "glColor3iv" );
	glColor3s = (glColor3s_t) GPA_GL( "glColor3s" );
	glColor3sv = (glColor3sv_t) GPA_GL( "glColor3sv" );
	glColor3ub = (glColor3ub_t) GPA_GL( "glColor3ub" );
	glColor3ubv = (glColor3ubv_t) GPA_GL( "glColor3ubv" );
	glColor3ui = (glColor3ui_t) GPA_GL( "glColor3ui" );
	glColor3uiv = (glColor3uiv_t) GPA_GL( "glColor3uiv" );
	glColor3us = (glColor3us_t) GPA_GL( "glColor3us" );
	glColor3usv = (glColor3usv_t) GPA_GL( "glColor3usv" );
	glColor4b = (glColor4b_t) GPA_GL( "glColor4b" );
	glColor4bv = (glColor4bv_t) GPA_GL( "glColor4bv" );
	glColor4d = (glColor4d_t) GPA_GL( "glColor4d" );
	glColor4dv = (glColor4dv_t) GPA_GL( "glColor4dv" );
	glColor4f = (glColor4f_t) GPA_GL( "glColor4f" );
	glColor4fv = (glColor4fv_t) GPA_GL( "glColor4fv" );
	glColor4i = (glColor4i_t) GPA_GL( "glColor4i" );
	glColor4iv = (glColor4iv_t) GPA_GL( "glColor4iv" );
	glColor4s = (glColor4s_t) GPA_GL( "glColor4s" );
	glColor4sv = (glColor4sv_t) GPA_GL( "glColor4sv" );
	glColor4ub = (glColor4ub_t) GPA_GL( "glColor4ub" );
	glColor4ubv = (glColor4ubv_t) GPA_GL( "glColor4ubv" );
	glColor4ui = (glColor4ui_t) GPA_GL( "glColor4ui" );
	glColor4uiv = (glColor4uiv_t) GPA_GL( "glColor4uiv" );
	glColor4us = (glColor4us_t) GPA_GL( "glColor4us" );
	glColor4usv = (glColor4usv_t) GPA_GL( "glColor4usv" );
	glColorMask = (glColorMask_t) GPA_GL( "glColorMask" );
	glColorMaterial = (glColorMaterial_t) GPA_GL( "glColorMaterial" );
	glColorPointer = (glColorPointer_t) GPA_GL( "glColorPointer" );
	glCopyPixels = (glCopyPixels_t) GPA_GL( "glCopyPixels" );
	glCopyTexImage1D = (glCopyTexImage1D_t) GPA_GL( "glCopyTexImage1D" );
	glCopyTexImage2D = (glCopyTexImage2D_t) GPA_GL( "glCopyTexImage2D" );
	glCopyTexSubImage1D = (glCopyTexSubImage1D_t) GPA_GL( "glCopyTexSubImage1D" );
	glCopyTexSubImage2D = (glCopyTexSubImage2D_t) GPA_GL( "glCopyTexSubImage2D" );
	glCullFace = (glCullFace_t) GPA_GL( "glCullFace" );
	glDeleteLists = (glDeleteLists_t) GPA_GL( "glDeleteLists" );
	glDeleteTextures = (glDeleteTextures_t) GPA_GL( "glDeleteTextures" );
	glDepthFunc = (glDepthFunc_t) GPA_GL( "glDepthFunc" );
	glDepthMask = (glDepthMask_t) GPA_GL( "glDepthMask" );
	glDepthRange = (glDepthRange_t) GPA_GL( "glDepthRange" );
	glDisable = (glDisable_t) GPA_GL( "glDisable" );
	glDisableClientState = (glDisableClientState_t) GPA_GL( "glDisableClientState" );
	glDrawArrays = (glDrawArrays_t) GPA_GL( "glDrawArrays" );
	glDrawBuffer = (glDrawBuffer_t) GPA_GL( "glDrawBuffer" );
	glDrawElements = (glDrawElements_t) GPA_GL( "glDrawElements" );
	glDrawPixels = (glDrawPixels_t) GPA_GL( "glDrawPixels" );
	glEdgeFlag = (glEdgeFlag_t) GPA_GL( "glEdgeFlag" );
	glEdgeFlagPointer = (glEdgeFlagPointer_t) GPA_GL( "glEdgeFlagPointer" );
	glEdgeFlagv = (glEdgeFlagv_t) GPA_GL( "glEdgeFlagv" );
	glEnable = (glEnable_t) GPA_GL( "glEnable" );
	glEnableClientState = (glEnableClientState_t) GPA_GL( "glEnableClientState" );
	glEnd = (glEnd_t) GPA_GL( "glEnd" );
	glEndList = (glEndList_t) GPA_GL( "glEndList" );
	glEvalCoord1d = (glEvalCoord1d_t) GPA_GL( "glEvalCoord1d" );
	glEvalCoord1dv = (glEvalCoord1dv_t) GPA_GL( "glEvalCoord1dv" );
	glEvalCoord1f = (glEvalCoord1f_t) GPA_GL( "glEvalCoord1f" );
	glEvalCoord1fv = (glEvalCoord1fv_t) GPA_GL( "glEvalCoord1fv" );
	glEvalCoord2d = (glEvalCoord2d_t) GPA_GL( "glEvalCoord2d" );
	glEvalCoord2dv = (glEvalCoord2dv_t) GPA_GL( "glEvalCoord2dv" );
	glEvalCoord2f = (glEvalCoord2f_t) GPA_GL( "glEvalCoord2f" );
	glEvalCoord2fv = (glEvalCoord2fv_t) GPA_GL( "glEvalCoord2fv" );
	glEvalMesh1 = (glEvalMesh1_t) GPA_GL( "glEvalMesh1" );
	glEvalMesh2 = (glEvalMesh2_t) GPA_GL( "glEvalMesh2" );
	glEvalPoint1 = (glEvalPoint1_t) GPA_GL( "glEvalPoint1" );
	glEvalPoint2 = (glEvalPoint2_t) GPA_GL( "glEvalPoint2" );
	glFeedbackBuffer = (glFeedbackBuffer_t) GPA_GL( "glFeedbackBuffer" );

	if( getenv( "TRIBES2_USE_FLUSH" ) ) {
		glFinish = (glFinish_t) GPA_GL( "glFinish" );
		glFlush = (glFlush_t) GPA_GL( "glFlush" );
	} else {
		glFinish = (glFinish_t) void_stub;
		glFlush = (glFlush_t) void_stub;
	}

	glFogf = (glFogf_t) GPA_GL( "glFogf" );
	glFogfv = (glFogfv_t) GPA_GL( "glFogfv" );
	glFogi = (glFogi_t) GPA_GL( "glFogi" );
	glFogiv = (glFogiv_t) GPA_GL( "glFogiv" );
	glFrontFace = (glFrontFace_t) GPA_GL( "glFrontFace" );
	glFrustum = (glFrustum_t) GPA_GL( "glFrustum" );
	glGenLists = (glGenLists_t) GPA_GL( "glGenLists" );
	glGenTextures = (glGenTextures_t) GPA_GL( "glGenTextures" );
	glGetBooleanv = (glGetBooleanv_t) GPA_GL( "glGetBooleanv" );
	glGetClipPlane = (glGetClipPlane_t) GPA_GL( "glGetClipPlane" );
	glGetDoublev = (glGetDoublev_t) GPA_GL( "glGetDoublev" );
	glGetError = (glGetError_t) GPA_GL( "glGetError" );
	glGetFloatv = (glGetFloatv_t) GPA_GL( "glGetFloatv" );
	glGetIntegerv = (glGetIntegerv_t) GPA_GL( "glGetIntegerv" );
	glGetLightfv = (glGetLightfv_t) GPA_GL( "glGetLightfv" );
	glGetLightiv = (glGetLightiv_t) GPA_GL( "glGetLightiv" );
	glGetMapdv = (glGetMapdv_t) GPA_GL( "glGetMapdv" );
	glGetMapfv = (glGetMapfv_t) GPA_GL( "glGetMapfv" );
	glGetMapiv = (glGetMapiv_t) GPA_GL( "glGetMapiv" );
	glGetMaterialfv = (glGetMaterialfv_t) GPA_GL( "glGetMaterialfv" );
	glGetMaterialiv = (glGetMaterialiv_t) GPA_GL( "glGetMaterialiv" );
	glGetPixelMapfv = (glGetPixelMapfv_t) GPA_GL( "glGetPixelMapfv" );
	glGetPixelMapuiv = (glGetPixelMapuiv_t) GPA_GL( "glGetPixelMapuiv" );
	glGetPixelMapusv = (glGetPixelMapusv_t) GPA_GL( "glGetPixelMapusv" );
	glGetPointerv = (glGetPointerv_t) GPA_GL( "glGetPointerv" );
	glGetPolygonStipple = (glGetPolygonStipple_t) GPA_GL( "glGetPolygonStipple" );
	glGetString = (glGetString_t) GPA_GL( "glGetString" );
	glGetTexEnvfv = (glGetTexEnvfv_t) GPA_GL( "glGetTexEnvfv" );
	glGetTexEnviv = (glGetTexEnviv_t) GPA_GL( "glGetTexEnviv" );
	glGetTexGendv = (glGetTexGendv_t) GPA_GL( "glGetTexGendv" );
	glGetTexGenfv = (glGetTexGenfv_t) GPA_GL( "glGetTexGenfv" );
	glGetTexGeniv = (glGetTexGeniv_t) GPA_GL( "glGetTexGeniv" );
	glGetTexImage = (glGetTexImage_t) GPA_GL( "glGetTexImage" );
	glGetTexLevelParameterfv = (glGetTexLevelParameterfv_t) GPA_GL( "glGetLevelParameterfv" );
	glGetTexLevelParameteriv = (glGetTexLevelParameteriv_t) GPA_GL( "glGetLevelParameteriv" );
	glGetTexParameterfv = (glGetTexParameterfv_t) GPA_GL( "glGetTexParameterfv" );
	glGetTexParameteriv = (glGetTexParameteriv_t) GPA_GL( "glGetTexParameteriv" );
	glHint = (glHint_t) GPA_GL( "glHint" );
	glIndexMask = (glIndexMask_t) GPA_GL( "glIndexMask" );
	glIndexPointer = (glIndexPointer_t) GPA_GL( "glIndexPointer" );
	glIndexd = (glIndexd_t) GPA_GL( "glIndexd" );
	glIndexdv = (glIndexdv_t) GPA_GL( "glIndexdv" );
	glIndexf = (glIndexf_t) GPA_GL( "glIndexf" );
	glIndexfv = (glIndexfv_t) GPA_GL( "glIndexfv" );
	glIndexi = (glIndexi_t) GPA_GL( "glIndexi" );
	glIndexiv = (glIndexiv_t) GPA_GL( "glIndexiv" );
	glIndexs = (glIndexs_t) GPA_GL( "glIndexs" );
	glIndexsv = (glIndexsv_t) GPA_GL( "glIndexsv" );
	glIndexub = (glIndexub_t) GPA_GL( "glIndexub" );
	glIndexubv = (glIndexubv_t) GPA_GL( "glIndexubv" );
	glInitNames = (glInitNames_t) GPA_GL( "glInitNames" );
	glInterleavedArrays = (glInterleavedArrays_t) GPA_GL( "glInterleavedArrays" );
	glIsEnabled = (glIsEnabled_t) GPA_GL( "glIsEnabled" );
	glIsList = (glIsList_t) GPA_GL( "glIsList" );
	glIsTexture = (glIsTexture_t) GPA_GL( "glIsTexture" );
	glLightModelf = (glLightModelf_t) GPA_GL( "glLightModelf" );
	glLightModelfv = (glLightModelfv_t) GPA_GL( "glLightModelfv" );
	glLightModeli = (glLightModeli_t) GPA_GL( "glLightModeli" );
	glLightModeliv = (glLightModeliv_t) GPA_GL( "glLightModeliv" );
	glLightf = (glLightf_t) GPA_GL( "glLightf" );
	glLightfv = (glLightfv_t) GPA_GL( "glLightfv" );
	glLighti = (glLighti_t) GPA_GL( "glLighti" );
	glLightiv = (glLightiv_t) GPA_GL( "glLightiv" );
	glLineStipple = (glLineStipple_t) GPA_GL( "glLineStipple" );
	glLineWidth = (glLineWidth_t) GPA_GL( "glLineWidth" );
	glListBase = (glListBase_t) GPA_GL( "glListBase" );
	glLoadIdentity = (glLoadIdentity_t) GPA_GL( "glLoadIdentity" );
	glLoadMatrixd = (glLoadMatrixd_t) GPA_GL( "glLoadMatrixd" );
	glLoadMatrixf = (glLoadMatrixf_t) GPA_GL( "glLoadMatrixf" );
	glLoadName = (glLoadName_t) GPA_GL( "glLoadName" );
	glLogicOp = (glLogicOp_t) GPA_GL( "glLogicOp" );
	glMap1d = (glMap1d_t) GPA_GL( "glMap1d" );
	glMap1f = (glMap1f_t) GPA_GL( "glMap1f" );
	glMap2d = (glMap2d_t) GPA_GL( "glMap2d" );
	glMap2f = (glMap2f_t) GPA_GL( "glMap2f" );
	glMapGrid1d = (glMapGrid1d_t) GPA_GL( "glMapGrid1d" );
	glMapGrid1f = (glMapGrid1f_t) GPA_GL( "glMapGrid1f" );
	glMapGrid2d = (glMapGrid2d_t) GPA_GL( "glMapGrid2d" );
	glMapGrid2f = (glMapGrid2f_t) GPA_GL( "glMapGrid2f" );
	glMaterialf = (glMaterialf_t) GPA_GL( "glMaterialf" );
	glMaterialfv = (glMaterialfv_t) GPA_GL( "glMaterialfv" );
	glMateriali = (glMateriali_t) GPA_GL( "glMateriali" );
	glMaterialiv = (glMaterialiv_t) GPA_GL( "glMaterialiv" );
	glMatrixMode = (glMatrixMode_t) GPA_GL( "glMatrixMode" );
	glMultMatrixd = (glMultMatrixd_t) GPA_GL( "glMultMatrixd" );
	glMultMatrixf = (glMultMatrixf_t) GPA_GL( "glMultMatrixf" );
	glNewList = (glNewList_t) GPA_GL( "glNewList" );
	glNormal3b = (glNormal3b_t) GPA_GL( "glNormal3b" );
	glNormal3bv = (glNormal3bv_t) GPA_GL( "glNormal3bv" );
	glNormal3d = (glNormal3d_t) GPA_GL( "glNormal3d" );
	glNormal3dv = (glNormal3dv_t) GPA_GL( "glNormal3dv" );
	glNormal3f = (glNormal3f_t) GPA_GL( "glNormal3f" );
	glNormal3fv = (glNormal3fv_t) GPA_GL( "glNormal3fv" );
	glNormal3i = (glNormal3i_t) GPA_GL( "glNormal3i" );
	glNormal3iv = (glNormal3iv_t) GPA_GL( "glNormal3iv" );
	glNormal3s = (glNormal3s_t) GPA_GL( "glNormal3s" );
	glNormal3sv = (glNormal3sv_t) GPA_GL( "glNormal3sv" );
	glNormalPointer = (glNormalPointer_t) GPA_GL( "glNormalPointer" );
	glOrtho = (glOrtho_t) GPA_GL( "glOrtho" );
	glPassThrough = (glPassThrough_t) GPA_GL( "glPassThrough" );
	glPixelMapfv = (glPixelMapfv_t) GPA_GL( "glPixelMapfv" );
	glPixelMapuiv = (glPixelMapuiv_t) GPA_GL( "glPixelMapuiv" );
	glPixelMapusv = (glPixelMapusv_t) GPA_GL( "glPixelMapusv" );
	glPixelStoref = (glPixelStoref_t) GPA_GL( "glPixelStoref" );
	glPixelStorei = (glPixelStorei_t) GPA_GL( "glPixelStorei" );
	glPixelTransferf = (glPixelTransferf_t) GPA_GL( "glPixelTransferf" );
	glPixelTransferi = (glPixelTransferi_t) GPA_GL( "glPixelTransferi" );
	glPixelZoom = (glPixelZoom_t) GPA_GL( "glPixelZoom" );
	glPointSize = (glPointSize_t) GPA_GL( "glPointSize" );
	glPolygonMode = (glPolygonMode_t) GPA_GL( "glPolygonMode" );
	glPolygonOffset = (glPolygonOffset_t) GPA_GL( "glPolygonOffset" );
	glPolygonStipple = (glPolygonStipple_t) GPA_GL( "glPolygonStipple" );
	glPopAttrib = (glPopAttrib_t) GPA_GL( "glPopAttrib" );
	glPopClientAttrib = (glPopClientAttrib_t) GPA_GL( "glPopClientAttrib" );
	glPopMatrix = (glPopMatrix_t) GPA_GL( "glPopMatrix" );
	glPopName = (glPopName_t) GPA_GL( "glPopName" );
	glPrioritizeTextures = (glPrioritizeTextures_t) GPA_GL( "glPrioritizeTextures" );
	glPushAttrib = (glPushAttrib_t) GPA_GL( "glPushAttrib" );
	glPushClientAttrib = (glPushClientAttrib_t) GPA_GL( "glPushClientAttrib" );
	glPushMatrix = (glPushMatrix_t) GPA_GL( "glPushMatrix" );
	glPushName = (glPushName_t) GPA_GL( "glPushName" );
	glRasterPos2d = (glRasterPos2d_t) GPA_GL( "glRasterPos2d" );
	glRasterPos2dv = (glRasterPos2dv_t) GPA_GL( "glRasterPos2dv" );
	glRasterPos2f = (glRasterPos2f_t) GPA_GL( "glRasterPos2f" );
	glRasterPos2fv = (glRasterPos2fv_t) GPA_GL( "glRasterPos2fv" );
	glRasterPos2i = (glRasterPos2i_t) GPA_GL( "glRasterPos2i" );
	glRasterPos2iv = (glRasterPos2iv_t) GPA_GL( "glRasterPos2iv" );
	glRasterPos2s = (glRasterPos2s_t) GPA_GL( "glRasterPos2s" );
	glRasterPos2sv = (glRasterPos2sv_t) GPA_GL( "glRasterPos2sv" );
	glRasterPos3d = (glRasterPos3d_t) GPA_GL( "glRasterPos3d" );
	glRasterPos3dv = (glRasterPos3dv_t) GPA_GL( "glRasterPos3dv" );
	glRasterPos3f = (glRasterPos3f_t) GPA_GL( "glRasterPos3f" );
	glRasterPos3fv = (glRasterPos3fv_t) GPA_GL( "glRasterPos3fv" );
	glRasterPos3i = (glRasterPos3i_t) GPA_GL( "glRasterPos3i" );
	glRasterPos3iv = (glRasterPos3iv_t) GPA_GL( "glRasterPos3iv" );
	glRasterPos3s = (glRasterPos3s_t) GPA_GL( "glRasterPos3s" );
	glRasterPos3sv = (glRasterPos3sv_t) GPA_GL( "glRasterPos3sv" );
	glRasterPos4d = (glRasterPos4d_t) GPA_GL( "glRasterPos4d" );
	glRasterPos4dv = (glRasterPos4dv_t) GPA_GL( "glRasterPos4dv" );
	glRasterPos4f = (glRasterPos4f_t) GPA_GL( "glRasterPos4f" );
	glRasterPos4fv = (glRasterPos4fv_t) GPA_GL( "glRasterPos4fv" );
	glRasterPos4i = (glRasterPos4i_t) GPA_GL( "glRasterPos4i" );
	glRasterPos4iv = (glRasterPos4iv_t) GPA_GL( "glRasterPos4iv" );
	glRasterPos4s = (glRasterPos4s_t) GPA_GL( "glRasterPos4s" );
	glRasterPos4sv = (glRasterPos4sv_t) GPA_GL( "glRasterPos4sv" );
	glReadBuffer = (glReadBuffer_t) GPA_GL( "glReadBuffer" );
	glReadPixels = (glReadPixels_t) GPA_GL( "glReadPixels" );
	glRectd = (glRectd_t) GPA_GL( "glRectd" );
	glRectdv = (glRectdv_t) GPA_GL( "glRectdv" );
	glRectf = (glRectf_t) GPA_GL( "glRectf" );
	glRectfv = (glRectfv_t) GPA_GL( "glRectfv" );
	glRecti = (glRecti_t) GPA_GL( "glRecti" );
	glRectiv = (glRectiv_t) GPA_GL( "glRectiv" );
	glRects = (glRects_t) GPA_GL( "glRects" );
	glRectsv = (glRectsv_t) GPA_GL( "glRectsv" );
	glRenderMode = (glRenderMode_t) GPA_GL( "glRenderMode" );
	glRotated = (glRotated_t) GPA_GL( "glRotated" );
	glRotatef = (glRotatef_t) GPA_GL( "glRotatef" );
	glScaled = (glScaled_t) GPA_GL( "glScaled" );
	glScalef = (glScalef_t) GPA_GL( "glScalef" );
	glScissor = (glScissor_t) GPA_GL( "glScissor" );
	glSelectBuffer = (glSelectBuffer_t) GPA_GL( "glSelectBuffer" );
	glShadeModel = (glShadeModel_t) GPA_GL( "glShadeModel" );
	glStencilFunc = (glStencilFunc_t) GPA_GL( "glStencilFunc" );
	glStencilMask = (glStencilMask_t) GPA_GL( "glStencilMask" );
	glStencilOp = (glStencilOp_t) GPA_GL( "glStencilOp" );
	glTexCoord1d = (glTexCoord1d_t) GPA_GL( "glTexCoord1d" );
	glTexCoord1dv = (glTexCoord1dv_t) GPA_GL( "glTexCoord1dv" );
	glTexCoord1f = (glTexCoord1f_t) GPA_GL( "glTexCoord1f" );
	glTexCoord1fv = (glTexCoord1fv_t) GPA_GL( "glTexCoord1fv" );
	glTexCoord1i = (glTexCoord1i_t) GPA_GL( "glTexCoord1i" );
	glTexCoord1iv = (glTexCoord1iv_t) GPA_GL( "glTexCoord1iv" );
	glTexCoord1s = (glTexCoord1s_t) GPA_GL( "glTexCoord1s" );
	glTexCoord1sv = (glTexCoord1sv_t) GPA_GL( "glTexCoord1sv" );
	glTexCoord2d = (glTexCoord2d_t) GPA_GL( "glTexCoord2d" );
	glTexCoord2dv = (glTexCoord2dv_t) GPA_GL( "glTexCoord2dv" );
	glTexCoord2f = (glTexCoord2f_t) GPA_GL( "glTexCoord2f" );
	glTexCoord2fv = (glTexCoord2fv_t) GPA_GL( "glTexCoord2fv" );
	glTexCoord2i = (glTexCoord2i_t) GPA_GL( "glTexCoord2i" );
	glTexCoord2iv = (glTexCoord2iv_t) GPA_GL( "glTexCoord2iv" );
	glTexCoord2s = (glTexCoord2s_t) GPA_GL( "glTexCoord2s" );
	glTexCoord2sv = (glTexCoord2sv_t) GPA_GL( "glTexCoord2sv" );
	glTexCoord3d = (glTexCoord3d_t) GPA_GL( "glTexCoord3d" );
	glTexCoord3dv = (glTexCoord3dv_t) GPA_GL( "glTexCoord3dv" );
	glTexCoord3f = (glTexCoord3f_t) GPA_GL( "glTexCoord3f" );
	glTexCoord3fv = (glTexCoord3fv_t) GPA_GL( "glTexCoord3fv" );
	glTexCoord3i = (glTexCoord3i_t) GPA_GL( "glTexCoord3i" );
	glTexCoord3iv = (glTexCoord3iv_t) GPA_GL( "glTexCoord3iv" );
	glTexCoord3s = (glTexCoord3s_t) GPA_GL( "glTexCoord3s" );
	glTexCoord3sv = (glTexCoord3sv_t) GPA_GL( "glTexCoord3sv" );
	glTexCoord4d = (glTexCoord4d_t) GPA_GL( "glTexCoord4d" );
	glTexCoord4dv = (glTexCoord4dv_t) GPA_GL( "glTexCoord4dv" );
	glTexCoord4f = (glTexCoord4f_t) GPA_GL( "glTexCoord4f" );
	glTexCoord4fv = (glTexCoord4fv_t) GPA_GL( "glTexCoord4fv" );
	glTexCoord4i = (glTexCoord4i_t) GPA_GL( "glTexCoord4i" );
	glTexCoord4iv = (glTexCoord4iv_t) GPA_GL( "glTexCoord4iv" );
	glTexCoord4s = (glTexCoord4s_t) GPA_GL( "glTexCoord4s" );
	glTexCoord4sv = (glTexCoord4sv_t) GPA_GL( "glTexCoord4sv" );
	glTexCoordPointer = (glTexCoordPointer_t) GPA_GL( "glTexCoordPointer" );
	glTexEnvf = (glTexEnvf_t) GPA_GL( "glTexEnvf" );
	glTexEnvfv = (glTexEnvfv_t) GPA_GL( "glTexEnvfv" );
	glTexEnvi = (glTexEnvi_t) GPA_GL( "glTexEnvi" );
	glTexEnviv = (glTexEnviv_t) GPA_GL( "glTexEnviv" );
	glTexGend = (glTexGend_t) GPA_GL( "glTexGend" );
	glTexGendv = (glTexGendv_t) GPA_GL( "glTexGendv" );
	glTexGenf = (glTexGenf_t) GPA_GL( "glTexGenf" );
	glTexGenfv = (glTexGenfv_t) GPA_GL( "glTexGenfv" );
	glTexGeni = (glTexGeni_t) GPA_GL( "glTexGeni" );
	glTexGeniv = (glTexGeniv_t) GPA_GL( "glTexGeniv" );
	glTexImage1D = (glTexImage1D_t) GPA_GL( "glTexImage1D" );
	glTexImage2D = (glTexImage2D_t) GPA_GL( "glTexImage2D" );
	glTexParameterf = (glTexParameterf_t) GPA_GL( "glTexParameterf" );
	glTexParameterfv = (glTexParameterfv_t) GPA_GL( "glTexParameterfv" );
	glTexParameteri = (glTexParameteri_t) GPA_GL( "glTexParameteri" );
	glTexParameteriv = (glTexParameteriv_t) GPA_GL( "glTexParameteriv" );
	glTexSubImage1D = (glTexSubImage1D_t) GPA_GL( "glTexSubImage1D" );
	glTexSubImage2D = (glTexSubImage2D_t) GPA_GL( "glTexSubImage2D" );
	glTranslated = (glTranslated_t) GPA_GL( "glTranslated" );
	glTranslatef = (glTranslatef_t) GPA_GL( "glTranslatef" );
	glVertex2d = (glVertex2d_t) GPA_GL( "glVertex2d" );
	glVertex2dv = (glVertex2dv_t) GPA_GL( "glVertex2dv" );
	glVertex2f = (glVertex2f_t) GPA_GL( "glVertex2f" );
	glVertex2fv = (glVertex2fv_t) GPA_GL( "glVertex2fv" );
	glVertex2i = (glVertex2i_t) GPA_GL( "glVertex2i" );
	glVertex2iv = (glVertex2iv_t) GPA_GL( "glVertex2iv" );

	glVertex2s = (glVertex2s_t) GPA_GL( "glVertex2s" );
	glVertex2sv = (glVertex2sv_t) GPA_GL( "glVertex2sv" );
	glVertex3d = (glVertex3d_t) GPA_GL( "glVertex3d" );
	glVertex3dv = (glVertex3dv_t) GPA_GL( "glVertex3dv" );
	glVertex3f = (glVertex3f_t) GPA_GL( "glVertex3f" );
	glVertex3fv = (glVertex3fv_t) GPA_GL( "glVertex3fv" );
	glVertex3i = (glVertex3i_t) GPA_GL( "glVertex3i" );
	glVertex3iv = (glVertex3iv_t) GPA_GL( "glVertex3iv" );
	glVertex3s = (glVertex3s_t) GPA_GL( "glVertex3s" );
	glVertex3sv = (glVertex3sv_t) GPA_GL( "glVertex3sv" );
	glVertex4d = (glVertex4d_t) GPA_GL( "glVertex4d" );
	glVertex4dv = (glVertex4dv_t) GPA_GL( "glVertex4dv" );
	glVertex4f = (glVertex4f_t) GPA_GL( "glVertex4f" );
	glVertex4fv = (glVertex4fv_t) GPA_GL( "glVertex4fv" );
	glVertex4i = (glVertex4i_t) GPA_GL( "glVertex4i" );
	glVertex4iv = (glVertex4iv_t) GPA_GL( "glVertex4iv" );
	glVertex4s = (glVertex4s_t) GPA_GL( "glVertex4s" );
	glVertex4sv = (glVertex4sv_t) GPA_GL( "glVertex4sv" );
	glVertexPointer = (glVertexPointer_t) GPA_GL( "glVertexPointer" );
	glViewport = (glViewport_t) GPA_GL( "glViewport" );
#endif

	return true;
}

bool QGL_EXT_Init( void )
{
#ifndef DEDICATED
	const char* pExtString = reinterpret_cast<const char*>( glGetString( GL_EXTENSIONS ) );
	gGLState.primMode = 0;

	// EXT_paletted_texture
	if (pExtString && dStrstr(pExtString, (const char*)"GL_EXT_paletted_texture") != NULL) {
		glColorTableEXT = (glColorTable_t) GPA_GL( "glColorTableEXT" );
		gGLState.suppPalettedTexture = true;
	} else {
		gGLState.suppPalettedTexture = false;
	}
   
	// EXT_compiled_vertex_array
	if( pExtString && dStrstr( pExtString, (const char*) "GL_EXT_compiled_vertex_array" ) != 0 ) {
		glLockArraysEXT = (glLockArrays_t) GPA_GL( "glLockArraysEXT" );
		glUnlockArraysEXT = (glUnlockArrays_t) GPA_GL( "glUnlockArraysEXT" );
		gGLState.suppLockedArrays = true;
	} else {
		glLockArraysEXT = 0;
		glUnlockArraysEXT = 0;
		gGLState.suppLockedArrays = false;
	}

	// ARB_multitexture
	if( pExtString && dStrstr( pExtString, (const char*) "GL_ARB_multitexture" ) != 0 ) {
		glActiveTextureARB = (glActiveTextureARB_t) GPA_GL( "glActiveTextureARB" );
		glClientActiveTextureARB = (glClientActiveTextureARB_t) GPA_GL( "glClientActiveTextureARB" );
		glMultiTexCoord2fARB = (glMultiTexCoord2fARB_t) GPA_GL( "glMultiTexCoord2fARB" );
		glMultiTexCoord2fvARB = (glMultiTexCoord2fvARB_t) GPA_GL( "glMultiTexCoord2fvARB" );
		gGLState.suppARBMultitexture = true;
	} else {
		glActiveTextureARB = 0;
		glClientActiveTextureARB = 0;
		glMultiTexCoord2fARB = 0;
		glMultiTexCoord2fvARB = 0;
		gGLState.suppARBMultitexture = false;
	}

	// NV_vertex_array_range
	if( pExtString && dStrstr( pExtString, (const char*) "GL_NV_vertex_array_range" ) != 0 ) {
		glVertexArrayRangeNV = (glVertexArrayRange_t) GPA_GL( "glVertexArrayRangeNV" );
		glFlushVertexArrayRangeNV = (glFlushVertexArrayRange_t) GPA_GL( "glFlushVertexArrayRangeNV" );
		glXAllocateMemoryNV = (glXAllocateMemory_t) GPA_GL( "glXAllocateMemoryNV" );
		glXFreeMemoryNV = (glXFreeMemory_t) GPA_GL( "glXFreeMemoryNV" );
		gGLState.suppVertexArrayRange = true;
	} else {
		glVertexArrayRangeNV = 0;
		glFlushVertexArrayRangeNV = 0;
		glXAllocateMemoryNV = 0;
		glXFreeMemoryNV = 0;
		gGLState.suppVertexArrayRange = false;
	}

	// EXT_fog_coord
	if( pExtString && dStrstr( pExtString, (const char*) "GL_EXT_fog_coord" ) != 0 ) {
		glFogCoordfEXT = (glFogCoordf_t) GPA_GL( "glFogCoordfEXT" );
		glFogCoordPointerEXT = (glFogCoordPointer_t) GPA_GL( "glFogCoordPointerEXT" );
		gGLState.suppFogCoord = true;
	} else {
		glFogCoordfEXT = 0;
		glFogCoordPointerEXT = 0;
		gGLState.suppFogCoord = false;
	}

	// ARB_texture_compression
	if( pExtString && dStrstr( pExtString, (const char*) "GL_ARB_texture_compression" ) != 0 ) {
		glCompressedTexImage3DARB = (glCompressedTexImage3DARB_t) GPA_GL( "glCompressedTexImage3DARB" );
		glCompressedTexImage2DARB = (glCompressedTexImage2DARB_t) GPA_GL( "glCompressedTexImage2DARB" );
		glCompressedTexImage1DARB = (glCompressedTexImage1DARB_t) GPA_GL( "glCompressedTexImage1DARB" );
		glCompressedTexSubImage3DARB = (glCompressedTexSubImage3DARB_t) GPA_GL( "glCompressedTexSubImage3DARB" );
		glCompressedTexSubImage2DARB = (glCompressedTexSubImage2DARB_t) GPA_GL( "glCompressedTexSubImage2DARB" );
		glCompressedTexSubImage1DARB = (glCompressedTexSubImage1DARB_t) GPA_GL( "glCompressedTexSubImage1DARB" );
		glGetCompressedTexImageARB = (glGetCompressedTexImageARB_t) GPA_GL( "glGetCompressedTexImageARB" );
		gGLState.suppTextureCompression = true;
	} else {
		glCompressedTexImage3DARB = 0;
		glCompressedTexImage2DARB = 0;
		glCompressedTexImage1DARB = 0;
		glCompressedTexSubImage3DARB = 0;
		glCompressedTexSubImage2DARB = 0;
		glCompressedTexSubImage1DARB = 0;
		glGetCompressedTexImageARB = 0;
		gGLState.suppTextureCompression = false;
	}

#undef GPA_GL

	// 3DFX_texture_compression_FXT1
	if( pExtString && dStrstr( pExtString, (const char*) "GL_3DFX_texture_compression_FXT1" ) != 0 ) {
		gGLState.suppFXT1 = true;
	} else {
		gGLState.suppFXT1 = false;
	}

	// EXT_texture_compression_S3TC
	if( pExtString && dStrstr( pExtString, (const char*) "GL_EXT_texture_compression_s3tc" ) != 0 ) {
		gGLState.suppS3TC = true;
	} else {
		gGLState.suppS3TC = false;
	}

	// Binary states, i.e., no supporting functions
	// EXT_packed_pixels
	// EXT_texture_env_combine
	gGLState.suppPackedPixels = pExtString ? ( dStrstr( pExtString, (const char*) "GL_EXT_packed_pixels" ) != 0 ) : false;
	gGLState.suppTextureEnvCombine = pExtString ? ( dStrstr( pExtString, (const char*) "GL_EXT_texture_env_combine" ) != 0 ) : false;
	gGLState.suppEdgeClamp = pExtString? ( dStrstr( pExtString, (const char*) "GL_EXT_texture_edge_clamp" ) != 0 ) : false;
	gGLState.suppTexEnvAdd = pExtString? ( dStrstr( pExtString, (const char*) "GL_ARB_texture_env_add" ) != 0 ) : false;
	gGLState.suppTexEnvAdd |= pExtString? ( dStrstr( pExtString, (const char*) "GL_EXT_texture_env_add" ) != 0 ) : false;

	// Anisotropic filtering
	gGLState.suppTexAnisotropic = pExtString? ( dStrstr( pExtString, (const char*) "GL_EXT_texture_filter_anisotropic" ) != 0 ) : false;

	if( gGLState.suppTexAnisotropic ) {
		glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &gGLState.maxAnisotropy );
	}
	if (gGLState.suppARBMultitexture)
		glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &gGLState.maxTextureUnits);
	else
		gGLState.maxTextureUnits = 1;

	Con::printf( "OpenGL Init: Enabled Extensions" );

	if( gGLState.suppARBMultitexture ) {
		Con::printf( "  ARB_multitexture (Max Texture Units: %d)", gGLState.maxTextureUnits);
	}

	if( gGLState.suppPalettedTexture ) {
		Con::printf( "  EXT_paletted_texture" );
	}

	if( gGLState.suppLockedArrays ) {
		Con::printf( "  EXT_compiled_vertex_array" );
	}

	if( gGLState.suppVertexArrayRange ) {
		Con::printf( "  NV_vertex_array_range" );
	}

	if( gGLState.suppTextureEnvCombine ) {
		Con::printf( "  EXT_texture_env_combine" );
	}

	if( gGLState.suppPackedPixels ) {
		Con::printf( "  EXT_packed_pixels" );
	}

	if( gGLState.suppFogCoord ) {
		Con::printf( "  EXT_fog_coord" );
	}

	if( gGLState.suppTextureCompression ) {
		Con::printf( "  ARB_texture_compression" );
	}

	if( gGLState.suppS3TC ) {
		Con::printf( "  EXT_texture_compression_s3tc" );
	}

	if( gGLState.suppFXT1 ) {
		Con::printf( "  3DFX_texture_compression_FXT1" );
	}

	if( gGLState.suppTexEnvAdd ) {
		Con::printf( "  (ARB|EXT)_texture_env_add" );
	}

	if( gGLState.suppTexAnisotropic ) {
		Con::printf( "  EXT_texture_filter_anisotropic (Max anisotropy: %f)", gGLState.maxAnisotropy );
	}

	Con::warnf( ConsoleLogEntry::General, "OpenGL Init: Disabled Extensions" );

	if( !gGLState.suppARBMultitexture ) {
		Con::warnf( ConsoleLogEntry::General, "  ARB_multitexture" );
	}

	if( !gGLState.suppPalettedTexture ) {
		Con::warnf( ConsoleLogEntry::General, "  EXT_paletted_texture" );
	}

	if( !gGLState.suppLockedArrays ) {
		Con::warnf( ConsoleLogEntry::General, "  EXT_compiled_vertex_array" );
	}

	if( !gGLState.suppVertexArrayRange ) {
		Con::warnf( ConsoleLogEntry::General, "  NV_vertex_array_range" );
	}

	if( !gGLState.suppTextureEnvCombine ) {
		Con::warnf( ConsoleLogEntry::General, "  EXT_texture_env_combine" );
	}

	if( !gGLState.suppPackedPixels ) {
		Con::warnf( ConsoleLogEntry::General, "  EXT_packed_pixels" );
	}

	if( !gGLState.suppFogCoord ) {
		Con::warnf( ConsoleLogEntry::General, "  EXT_fog_coord" );
	}

	if( !gGLState.suppTextureCompression ) {
		Con::warnf( ConsoleLogEntry::General, "  ARB_texture_compression" );
	}

	if( !gGLState.suppS3TC ) {
		Con::warnf( ConsoleLogEntry::General, "  EXT_texture_compression_s3tc" );
	}

	if( !gGLState.suppFXT1 ) {
		Con::warnf( ConsoleLogEntry::General, "  3DFX_texture_compression_FXT1" );
	}

	if( !gGLState.suppTexEnvAdd ) {
		Con::warnf( ConsoleLogEntry::General, "  (ARB|EXT)_texture_env_add" );
	}

	if( !gGLState.suppTexAnisotropic ) {
		Con::warnf( ConsoleLogEntry::General, "  EXT_texture_filter_anisotropic" );
	}

	// Set some console vars.
	Con::setBoolVariable( "$TextureCompressionSupported", gGLState.suppTextureCompression );
	Con::setBoolVariable( "$AnisotropySupported", gGLState.suppTexAnisotropic );
	Con::setBoolVariable( "$PalettedTextureSupported", gGLState.suppPalettedTexture );

	if (!gGLState.suppPalettedTexture && Con::getBoolVariable("$pref::OpenGL::forcePalettedTexture",false))
	{
		Con::setBoolVariable("$pref::OpenGL::forcePalettedTexture", false);
		Con::setBoolVariable("$pref::OpenGL::force16BitTexture", true);
	}

#endif

	return true;
}

static bool loggingEnabled = false;
static bool outlineEnabled = false;
static bool perfEnabled = false;

#if defined (DEBUG) || defined(INTERNAL_RELEASE)
ConsoleFunction(GLEnableLogging, void, 2, 2, "GLEnableLogging(bool);")
{
	// Ignore
}

static void outlineDrawArrays(GLenum mode, GLint first, GLsizei count)
{
   if(mode == GL_POLYGON)
      mode = GL_LINE_LOOP;

   if(mode == GL_POINTS || mode == GL_LINE_STRIP || mode == GL_LINE_LOOP || mode == GL_LINES)   
      glDrawArrays( mode, first, count );
   else
   {
      glBegin(GL_LINES);
      if(mode == GL_TRIANGLE_STRIP)
      {
         U32 i;
         for(i = 0; i < count - 1; i++) 
         {
            glArrayElement(first + i);
            glArrayElement(first + i + 1);
            if(i + 2 != count)
            {
               glArrayElement(first + i);
               glArrayElement(first + i + 2);
            }
         }
      }
      else if(mode == GL_TRIANGLE_FAN)
      {
         for(U32 i = 1; i < count; i ++)
         {
            glArrayElement(first);
            glArrayElement(first + i);
            if(i != count - 1)
            {
               glArrayElement(first + i);
               glArrayElement(first + i + 1);
            }
         }
      }
      else if(mode == GL_TRIANGLES)
      {
         for(U32 i = 3; i <= count; i += 3)
         {
            glArrayElement(first + i - 3);
            glArrayElement(first + i - 2);
            glArrayElement(first + i - 2);
            glArrayElement(first + i - 1);
            glArrayElement(first + i - 3);
            glArrayElement(first + i - 1);
         }
      }
      else if(mode == GL_QUADS)
      {
         for(U32 i = 4; i <= count; i += 4)
         {
            glArrayElement(first + i - 4);
            glArrayElement(first + i - 3);
            glArrayElement(first + i - 3);
            glArrayElement(first + i - 2);
            glArrayElement(first + i - 2);
            glArrayElement(first + i - 1);
            glArrayElement(first + i - 4);
            glArrayElement(first + i - 1);
         }
      }
      else if(mode == GL_QUAD_STRIP)
      {
         if(count < 4)
            return;
         glArrayElement(first + 0);
         glArrayElement(first + 1);
         for(U32 i = 4; i <= count; i += 2)
         {
            glArrayElement(first + i - 4);
            glArrayElement(first + i - 2);

            glArrayElement(first + i - 3);
            glArrayElement(first + i - 1);

            glArrayElement(first + i - 2);
            glArrayElement(first + i - 1);
         }
      }
      glEnd();
   }
}

static U32 getIndex(GLenum type, const void *indices, U32 i)
{
   if(type == GL_UNSIGNED_BYTE)
      return ((U8 *) indices)[i];
   else if(type == GL_UNSIGNED_SHORT)
      return ((U16 *) indices)[i];
   else
      return ((U32 *) indices)[i];
}

static void outlineDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices)
{
   if(mode == GL_POLYGON)
      mode = GL_LINE_LOOP;

   if(mode == GL_POINTS || mode == GL_LINE_STRIP || mode == GL_LINE_LOOP || mode == GL_LINES)   
      glDrawElements( mode, count, type, indices );
   else
   {
      glBegin(GL_LINES);
      if(mode == GL_TRIANGLE_STRIP)
      {
         U32 i;
         for(i = 0; i < count - 1; i++) 
         {
            glArrayElement(getIndex(type, indices, i));
            glArrayElement(getIndex(type, indices, i + 1));
            if(i + 2 != count)
            {
               glArrayElement(getIndex(type, indices, i));
               glArrayElement(getIndex(type, indices, i + 2));
            }
         }
      }
      else if(mode == GL_TRIANGLE_FAN)
      {
         for(U32 i = 1; i < count; i ++)
         {
            glArrayElement(getIndex(type, indices, 0));
            glArrayElement(getIndex(type, indices, i));
            if(i != count - 1)
            {
               glArrayElement(getIndex(type, indices, i));
               glArrayElement(getIndex(type, indices, i + 1));
            }
         }
      }
      else if(mode == GL_TRIANGLES)
      {
         for(U32 i = 3; i <= count; i += 3)
         {
            glArrayElement(getIndex(type, indices, i - 3));
            glArrayElement(getIndex(type, indices, i - 2));
            glArrayElement(getIndex(type, indices, i - 2));
            glArrayElement(getIndex(type, indices, i - 1));
            glArrayElement(getIndex(type, indices, i - 3));
            glArrayElement(getIndex(type, indices, i - 1));
         }
      }
      else if(mode == GL_QUADS)
      {
         for(U32 i = 4; i <= count; i += 4)
         {
            glArrayElement(getIndex(type, indices, i - 4));
            glArrayElement(getIndex(type, indices, i - 3));
            glArrayElement(getIndex(type, indices, i - 3));
            glArrayElement(getIndex(type, indices, i - 2));
            glArrayElement(getIndex(type, indices, i - 2));
            glArrayElement(getIndex(type, indices, i - 1));
            glArrayElement(getIndex(type, indices, i - 4));
            glArrayElement(getIndex(type, indices, i - 1));
         }
      }
      else if(mode == GL_QUAD_STRIP)
      {
         if(count < 4)
            return;
         glArrayElement(getIndex(type, indices, 0));
         glArrayElement(getIndex(type, indices, 1));
         for(U32 i = 4; i <= count; i += 2)
         {
            glArrayElement(getIndex(type, indices, i - 4));
            glArrayElement(getIndex(type, indices, i - 2));

            glArrayElement(getIndex(type, indices, i - 3));
            glArrayElement(getIndex(type, indices, i - 1));

            glArrayElement(getIndex(type, indices, i - 2));
            glArrayElement(getIndex(type, indices, i - 1));
         }
      }
      glEnd();
   }
}

ConsoleFunction(GLEnableOutline, void, 2, 2, "GLEnableOutline(bool);")
{
   argc;
   bool enable = dAtob(argv[1]);

#if 0  // Don't ever use outline mode, GL functions go recursive, not needed.
   if(outlineEnabled == enable)
      return;
   
   if(enable && (loggingEnabled || perfEnabled))
      return;
   
   outlineEnabled = enable;

   if ( enable )
   {
      glDrawElements = outlineDrawElements;
      glDrawArrays = outlineDrawArrays;
      // FIXME: (outline mode only used for debugging, not critical)
      // SwapBuffers should be replaced with:
      // SwapBuffers;
      // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   }
   else
   {
      glDrawElements = glDrawElements;
      glDrawArrays = glDrawArrays;
   }
#endif /* 0 */
}

static void perfDrawArrays(GLenum mode, GLint first, GLsizei count)
{
   gGLState.primCount[gGLState.primMode]++;
   U32 tc = 0;

   if(mode == GL_TRIANGLES)
      tc = count / 3;
   else if(mode == GL_TRIANGLE_FAN || mode == GL_TRIANGLE_STRIP)
      tc = count - 2;

   gGLState.triCount[gGLState.primMode] += tc;
   glDrawArrays( mode, first, count );
}

static void perfDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices)
{
   gGLState.primCount[gGLState.primMode]++;
   U32 tc = 0;

   if(mode == GL_TRIANGLES)
      tc = count / 3;
   else if(mode == GL_TRIANGLE_FAN || mode == GL_TRIANGLE_STRIP)
      tc = count - 2;

   gGLState.triCount[gGLState.primMode] += tc;
   glDrawElements( mode, count, type, indices );
}

ConsoleFunction(GLEnableMetrics, void, 2, 2, "GLEnableMetrics(bool);")
{
   argc;
   static bool varsAdded = false;
   
   if(!varsAdded)
   {
      Con::addVariable("OpenGL::triCount0", TypeS32, &gGLState.triCount[0]);
      Con::addVariable("OpenGL::triCount1", TypeS32, &gGLState.triCount[1]);
      Con::addVariable("OpenGL::triCount2", TypeS32, &gGLState.triCount[2]);
      Con::addVariable("OpenGL::triCount3", TypeS32, &gGLState.triCount[3]);

      Con::addVariable("OpenGL::primCount0", TypeS32, &gGLState.primCount[0]);
      Con::addVariable("OpenGL::primCount1", TypeS32, &gGLState.primCount[1]);
      Con::addVariable("OpenGL::primCount2", TypeS32, &gGLState.primCount[2]);
      Con::addVariable("OpenGL::primCount3", TypeS32, &gGLState.primCount[3]);
      varsAdded = true;
   }
   
   bool enable = dAtob(argv[1]);
#if 0  // Don't ever use perf mode, GL functions go recursive, not needed.
   if(perfEnabled == enable)
      return;
   
   if(enable && (loggingEnabled || outlineEnabled))
      return;
   
   perfEnabled = enable;

   if ( enable )
   {
      glDrawElements = perfDrawElements;
      glDrawArrays = perfDrawArrays;
   }
   else
   {
      glDrawElements = glDrawElements;
      glDrawArrays = glDrawArrays;
   }
#endif /* 0 */
}
#endif /* DEBUG || INTERNAL_RELEASE */
