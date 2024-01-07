//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _MATHUTILS_H_
#define _MATHUTILS_H_

class Point3F;
class MatrixF;

namespace MathUtils
{

   MatrixF createOrientFromDir( Point3F &direction );
   Point3F randomDir( Point3F &axis, F32 thetaAngleMin, F32 thetaAngleMax, F32 phiAngleMin = 0.0, F32 phiAngleMax = 360.0 );
   void    getAnglesFromVector( VectorF &vec, F32 &yawAng, F32 &pitchAng );



}

#endif // _MATHUTILS_H_
