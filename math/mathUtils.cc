//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "Math/mMath.h"
#include "Math/mathUtils.h"
#include "Math/mRandom.h"

namespace MathUtils
{

MRandomLCG sgRandom(0xdeadbeef);

//------------------------------------------------------------------------------
// Creates orientation matrix from a direction vector.  Assumes ( 0 0 1 ) is up.
//------------------------------------------------------------------------------
MatrixF createOrientFromDir( Point3F &direction )
{
	Point3F j = direction;
	Point3F k(0.0, 0.0, 1.0);
	Point3F i;
	
	mCross( j, k, &i );

	if( i.magnitudeSafe() == 0.0 )
	{
		i = Point3F( 0.0, -1.0, 0.0 );
	}

	i.normalizeSafe();
	mCross( i, j, &k );

   MatrixF mat( true );
   mat.setColumn( 0, i );
   mat.setColumn( 1, j );
   mat.setColumn( 2, k );

	return mat;
}


//------------------------------------------------------------------------------
// Creates random direction given angle parameters similar to the particle system.
// The angles are relative to the specified axis.
//------------------------------------------------------------------------------
Point3F randomDir( Point3F &axis, F32 thetaAngleMin, F32 thetaAngleMax, 
                                 F32 phiAngleMin, F32 phiAngleMax )
{
   MatrixF orient = createOrientFromDir( axis );
   Point3F axisx;
   orient.getColumn( 0, &axisx );

   F32 theta = (thetaAngleMax - thetaAngleMin) * sgRandom.randF() + thetaAngleMin;
   F32 phi = (phiAngleMax - phiAngleMin) * sgRandom.randF() + phiAngleMin;

   // Both phi and theta are in degs.  Create axis angles out of them, and create the
   //  appropriate rotation matrix...
   AngAxisF thetaRot(axisx, theta * (M_PI / 180.0));
   AngAxisF phiRot(axis,    phi   * (M_PI / 180.0));

   Point3F ejectionAxis = axis;

   MatrixF temp(true);
   thetaRot.setMatrix(&temp);
   temp.mulP(ejectionAxis);
   phiRot.setMatrix(&temp);
   temp.mulP(ejectionAxis);

   return ejectionAxis;
}


//------------------------------------------------------------------------------
// Returns yaw and pitch angles from a given vector.  Angles are in RADIANS.
// Assumes north is (0.0, 1.0, 0.0), the degrees move upwards clockwise.
// The range of yaw is 0 - 2PI.  The range of pitch is -PI/2 - PI/2.
//------------------------------------------------------------------------------
void getAnglesFromVector( VectorF &vec, F32 &yawAng, F32 &pitchAng )
{
   yawAng = mAtan( vec.x, vec.y );

   if( yawAng < 0.0 )
   {
      yawAng += M_2PI;
   }

   pitchAng = mAtan( fabs(vec.z), fabs(vec.y) );

   if( vec.z < 0.0 )
   {
      pitchAng = -pitchAng;
   }

}





} // end namespace MathUtils
