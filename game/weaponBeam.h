//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _WEAPONBEAM_H_
#define _WEAPONBEAM_H_

#define BEAM_EDGE_TOLERANCE -0.99980  //   1.1459 degrees
#define BEAM_EDGE_ADJUST 0.022
#define BEAM_EDGE_EPSILON 0.0001

class Point3F;

//**************************************************************************
// Beam Data
//**************************************************************************

struct BeamData
{
   Point3F  direction;
   Point3F  startPos;
   Point3F  endPos;           // actual end position
   Point3F  endRenderPos;     // modified if beam is on-edge
   Point3F  axis;             // axis for front facing beam
   F32      angToCamPos;      // angle from line btwn cam pos and inital pos and the beam dir
   F32      angToCamDir;      // angle btwn cam look dir and beam dir
   bool     onEdge;           // true if beam perfectly lines up with camera
   F32      length;
   Point3F  dirFromCam;       // dir from camPos to beam initialPos


   BeamData()
   {
      dMemset( this, 0, sizeof( BeamData ) );
   }
};


//**************************************************************************
// Weapon Beam
//**************************************************************************
class WeaponBeam
{
private:
   
public:
  

   BeamData mData;


public:

   WeaponBeam();

   void adjustEdge( BeamData &beamData );
   void render( F32 width, F32 UVOffset = 0.0, F32 numRepeat = 1.0 );




};


#endif
