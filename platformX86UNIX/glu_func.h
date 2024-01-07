//-----------------------------------------------------------------------------
// Torque Game Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

//------------------------------------------------------------------------------
// GLU functions
//------------------------------------------------------------------------------

GL_FUNCTION(int, gluProject, (GLdouble objx, GLdouble objy, GLdouble objz,  const GLdouble modelMatrix[16], const GLdouble projMatrix[16],  const GLint viewport[4], GLdouble *winx, GLdouble *winy, GLdouble *winz), return 0; )
GL_FUNCTION(int, gluUnProject, (GLdouble winx, GLdouble winy, GLdouble winz, const GLdouble modelMatrix[16], const GLdouble projMatrix[16],  const GLint viewport[4], GLdouble *objx, GLdouble *objy, GLdouble *objz), return 0; )

