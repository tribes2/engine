//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "Math/mMath.h"

#include "game/weaponBeam.h"
#include "PlatformWin32/platformGL.h"

//**************************************************************************
// Weapon Beam
//**************************************************************************
WeaponBeam::WeaponBeam()
{

};


//--------------------------------------------------------------------------
void WeaponBeam::render( F32 width, F32 UVOffset, F32 numRepeat )
{
   width *= 0.5;

   glBegin(GL_TRIANGLE_FAN);
      glTexCoord2f( UVOffset, 0.0 );
      glVertex3fv( mData.startPos + mData.axis * width );

      glTexCoord2f( UVOffset, 1.0 );
      glVertex3fv( mData.startPos - mData.axis * width );

      glTexCoord2f( numRepeat+UVOffset, 1.0 );
      glVertex3fv( mData.endRenderPos - mData.axis * width );

      glTexCoord2f( numRepeat+UVOffset, 0.0 );
      glVertex3fv( mData.endRenderPos + mData.axis * width );
   glEnd();


}

//--------------------------------------------------------------------------
void WeaponBeam::adjustEdge( BeamData &beamData )
{

   beamData.onEdge = false;
   beamData.endRenderPos = beamData.endPos;

   if( beamData.angToCamPos < BEAM_EDGE_TOLERANCE )
   {

      Point3F diff = beamData.direction - (-beamData.dirFromCam);
      F32 curOffset = diff.len();
      if( curOffset <= BEAM_EDGE_EPSILON )
      {
         // beam is perfectly on edge
         curOffset = 0.0;
         diff.set(0.0, 0.0, 0.0 );
         beamData.onEdge = true;
      }
      else
      {
         diff.normalize();
      }
      diff *= BEAM_EDGE_ADJUST - curOffset;

      Point3F newDir = beamData.direction + diff;
      newDir.normalizeSafe();

      beamData.endRenderPos = beamData.startPos + newDir * beamData.length;

   }

}
