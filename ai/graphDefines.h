//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GRAPHDEFINES_H_
#define _GRAPHDEFINES_H_

enum GridOffsets {
   GridBottomLeft,
   GridBottom, GridLeft,
   GridBottomRight, GridTopLeft,
   GridRight, GridTop,
   GridTopRight,
   NumGridOffsets
};

enum NavGraphDefinitions {
   GraphNodeClear = 0,              // Navigable flags on the terrain grid information.
   GraphNodeShadowed,               // We're using 4, allow up to 8 types.  
   GraphNodeObstructed, 
   GraphNodeSubmerged, 
   GroundNodeSteep = (1 << 3),      // Signals next to unwalkable slope. 
   
   GraphThreatLimit = 64, 
   GraphMaxTeams = 32, 
   AbsMaxBotCount = 64, 
};

// 2.5 minutes- 
#define  GraphMaxNodeAvoidMS  150000

// Pull-in amount used to avoid rounding errors in indoor node volume obstuction checks
#define  NodeVolumeShrink        0.014f
#define  UncappedNodeVolumeHt    1000.0f

// Bridge builder assures jet connections at least a certain amount.  Jet code uses a little
// smaller threshold (for not attempting) since bots might not be right on point- 
#define  GraphJetBridgeXY     1.7f
#define  GraphJetFailXY       1.3f

#define  MaxGraphNodeVolRad   25.0f

#define  LiberalBonkXY        21.0f


// Repository for all graph dimension numbers.  
struct NavGraphGlobals
{
   // Terrain:
   F32      mSquareWidth;
   F32      mInverseWidth;
   F32      mSquareRadius;
   S32      mSquareShift;
   Point3F  mHalfSquare;
   
   void setTerrNumbers(S32 shift)
   {
      mSquareShift = shift;
      mSquareWidth = F32(1 << shift);
      mInverseWidth = 1.0 / mSquareWidth;
      mSquareRadius = F32(1 << shift-1);
      mHalfSquare.set(mSquareRadius - 0.01, mSquareRadius - 0.01, 0.0);
   }
   
   // Walk / jet / jump configuration numbers
   F32   mWalkableDot;
   F32   mWalkableAngle;
   F32   mJumpAdd;
   F32   mTallestPlayer;
   F32   mStepHeight;
   F32   mPrettySteepAngle;
   F32   mPrettySteepDot;
   
   void setWalkData(F32 angle, F32 stepHeight)
   {
      mWalkableAngle = mDegToRad(angle);
      mWalkableDot = mCos(mWalkableAngle);
      mJumpAdd = 2.0;
      mTallestPlayer = 2.3;
      mStepHeight = stepHeight;

      // Angle slightly less than max used to decide to make a transient connection 
      // jettable between two points outdoors.  cf. graphLocate.cc
      mPrettySteepAngle = mWalkableAngle * (6.0 / 7.0);
      mPrettySteepDot = mCos(mPrettySteepAngle);
   }
   
   // Build a default version - 
   NavGraphGlobals()
   {
      setTerrNumbers(3);            // Default shift value
      setWalkData(70, 0.75);        // Walk angle (min of all maxes), step height
   }
};

// Defined in NavigationGraph.cc- 
extern   NavGraphGlobals   gNavGlobs;

#endif
