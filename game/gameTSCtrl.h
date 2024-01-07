//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GAMETSCTRL_H_
#define _GAMETSCTRL_H_

#ifndef _DGL_H_
#include "dgl/dgl.h"
#endif
#ifndef _GAME_H_
#include "game/game.h"
#endif
#ifndef _GUITSCONTROL_H_
#include "gui/guiTSControl.h"
#endif

class ProjectileData;
class GameBase;

//----------------------------------------------------------------------------
class GameTSCtrl : public GuiTSCtrl
{
   typedef GuiTSCtrl Parent;

   TextureHandle     mBeaconBaseTexture;
   TextureHandle     mBeaconTargetTexture;

public:
   StringTableEntry  mBeaconBaseTextureName;
   StringTableEntry  mBeaconTargetTextureName;
   S32               mBeaconTargetPeriod;
   bool              mBeaconsVisible;
   ColorF            mBeaconLineBeginColor;
   ColorF            mBeaconLineEndColor;   
   ColorF            mVehicleBeaconBeginColor;
   ColorF            mVehicleBeaconEndColor;   
   ColorF            mFriendBeaconBeginColor;
   ColorF            mFriendBeaconEndColor;   
   F32               mBeaconLineWidth;
   S32               mBeaconTextYOffset;
   bool              mShowAlternateTarget;

private:
   Vector<Point3F>   mBeaconTextPos;

   void renderBeacon(const GameBase * beaconObj, const Point3F & eyePos,
                     const Point3F & sourcePos, const VectorF & sourceVel, 
                     const ProjectileData * projectile, const bool pilot,
                     const F32 pilotHeight);

   void renderBeaconsText();
   void renderBeacons();

public:
   GameTSCtrl();

   bool processCameraQuery(CameraQuery *query);
   void renderWorld(const RectI &updateRect);

   bool onWake();
   void onSleep();

   void onMouseMove(const GuiEvent &evt);
   void onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder);

   static void initPersistFields();
   static void consoleInit();
 
   DECLARE_CONOBJECT(GameTSCtrl);
};

#endif
