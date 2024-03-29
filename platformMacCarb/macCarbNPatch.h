// macCarbNPatch.h
//
// mac specific implementation(s) of NPatch functionality
// since each platform might use slightly diff methods to control.

// current Mac NPatches is ATI TRUFORM implementation, accessed on OS9 via a
// back door method.  OSX tests for the ATIX extension.

#if !__APPLE__
#define AGLSETINT_NPATCH_FLAG    ((unsigned long)500)
#define AGLSETINT_NPATCH_LOD     ((unsigned long)501)
#endif

// for the moment, this seems to be the best roundup of
// the npatch extensions on the PC.

#ifndef GL_ATIX_pn_triangles
#define GL_ATIX_pn_triangles 1
#define GL_PN_TRIANGLES_ATIX                            0x6090
#define GL_MAX_PN_TRIANGLES_TESSELATION_LEVEL_ATIX      0x6091
#define GL_PN_TRIANGLES_POINT_MODE_ATIX                 0x6092
#define GL_PN_TRIANGLES_NORMAL_MODE_ATIX                0x6093
#define GL_PN_TRIANGLES_TESSELATION_LEVEL_ATIX          0x6094
#define GL_PN_TRIANGLES_POINT_MODE_LINEAR_ATIX          0x6095
#define GL_PN_TRIANGLES_POINT_MODE_CUBIC_ATIX           0x6096
#define GL_PN_TRIANGLES_NORMAL_MODE_LINEAR_ATIX         0x6097
#define GL_PN_TRIANGLES_NORMAL_MODE_QUADRATIC_ATIX      0x6098

#if __APPLE__ // for the moment...
extern void glPNTrianglesiATIX(GLenum pname, GLint param);
extern void glPNTrianglesfATIX(GLenum pname, GLfloat param);
#endif
#endif

typedef void (*PFNGLPNTRIANGLESIATIPROC)(GLenum pname, GLint param);
//typedef void (APIENTRY *PFNGLPNTRIANGLESFATIPROC)(GLenum pname, GLfloat param);

#define GL_NPATCH_EXT_STRING        "GL_ATIX_pn_triangles"
#define GL_NPATCH_SETINT_STRING     "glPNTrianglesiATIX"
typedef PFNGLPNTRIANGLESIATIPROC    PFNNPatchSetInt;

#define GETINT_NPATCH_MAX_LEVEL     GL_MAX_PN_TRIANGLES_TESSELATION_LEVEL_ATIX

#define GL_NPATCH_FLAG              GL_PN_TRIANGLES_ATIX

#define SETINT_NPATCH_LOD           GL_PN_TRIANGLES_TESSELATION_LEVEL_ATIX

#define SETINT_NPATCH_POINTINTERP   GL_PN_TRIANGLES_POINT_MODE_ATIX
#define SETINT_NPATCH_NORMALINTERP  GL_PN_TRIANGLES_NORMAL_MODE_ATIX

#define NPATCH_POINTINTERP_MIN      GL_PN_TRIANGLES_POINT_MODE_LINEAR_ATIX
#define NPATCH_POINTINTERP_MAX      GL_PN_TRIANGLES_POINT_MODE_CUBIC_ATIX

#define NPATCH_NORMALINTERP_MIN     GL_PN_TRIANGLES_NORMAL_MODE_LINEAR_ATIX
#define NPATCH_NORMALINTERP_MAX     GL_PN_TRIANGLES_NORMAL_MODE_QUADRATIC_ATIX
