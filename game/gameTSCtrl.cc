//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "game/gameTSCtrl.h"
#include "console/consoleTypes.h"
#include "game/projectile.h"
#include "game/gameBase.h"
#include "game/gameConnection.h"
#include "game/shapeBase.h"
#include "game/player.h"

//---------------------------------------------------------------------------
// Debug stuff:
Point3F lineTestStart = Point3F(0, 0, 0);
Point3F lineTestEnd =   Point3F(0, 1000, 0);
Point3F lineTestIntersect =  Point3F(0, 0, 0);
bool gSnapLine = false;

//----------------------------------------------------------------------------
// Class: GameTSCtrl
//----------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(GameTSCtrl);

GameTSCtrl::GameTSCtrl()
{
   // field data
   mBeaconBaseTextureName = StringTable->insert("gui/beacon_base");
   mBeaconTargetTextureName = StringTable->insert("gui/crosshairs");
   mBeaconTargetPeriod = 4000;
   mBeaconsVisible = true;
   mBeaconLineWidth = 2.5f;
   mBeaconLineBeginColor = ColorF(0.f, 1.f, 0.f, 0.2f);
   mBeaconLineEndColor = ColorF(0.f, 1.f, 0.f, 0.8f);
   mVehicleBeaconBeginColor = ColorF(1.f, 0.f, 0.f, 0.2f);
   mVehicleBeaconEndColor = ColorF(1.f, 0.f, 0.f, 0.8f);
   mFriendBeaconBeginColor = ColorF(1.f, 1.f, 0.f, 0.2f);
   mFriendBeaconEndColor = ColorF(1.f, 1.f, 0.f, 0.8f);
   mBeaconTextYOffset = 14;
   mShowAlternateTarget = false;
}

bool GameTSCtrl::onWake()
{
   if(!Parent::onWake())
      return(false);

   mBeaconTargetTexture = TextureHandle(mBeaconTargetTextureName);
   mBeaconBaseTexture = TextureHandle(mBeaconBaseTextureName);
   return(true);
}

void GameTSCtrl::onSleep()
{
   Parent::onSleep();
   mBeaconTargetTexture = 0;
   mBeaconBaseTexture = 0;
}

//---------------------------------------------------------------------------
bool GameTSCtrl::processCameraQuery(CameraQuery *camq)
{
   GameUpdateCameraFov();
   return GameProcessCameraQuery(camq);
}

//---------------------------------------------------------------------------
void GameTSCtrl::renderBeacon(const GameBase * beaconObj, const Point3F & eyePos,
                              const Point3F & sourcePos, const VectorF & sourceVel, 
                              const ProjectileData * projectile, 
                              const bool pilot, const F32 pilotHeight)
{
   Point3F pos;
   beaconObj->getTransform().getColumn(3, &pos);
   ShapeBase * shBeacon = static_cast<ShapeBase *>(const_cast<GameBase *>(beaconObj));
   mBeaconTextPos.push_back(Point3F(pos.x, pos.y, (pilot) ? pilotHeight : pos.z));

   Point3F dir = Point3F(pos - eyePos);
   if( dir.isZero() ) return;
   dir.normalize();

   F32 width = F32(mBeaconBaseTexture.getWidth()) * (100.f * mTan(mDegToRad(GameGetCameraFov()/2.f))) / F32(getExtent().x);

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, mBeaconBaseTexture.getGLName());
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

   ColorF beginColor(mBeaconLineBeginColor);
   ColorF endColor(mBeaconLineEndColor);      
   S32 beaconType = shBeacon->getBeaconType();
   if(beaconType == GameBase::vehicleBeacon)
   {
      beginColor = mVehicleBeaconBeginColor;
      endColor = mVehicleBeaconEndColor;      
   }
   else if(beaconType == GameBase::friendBeacon)
   {
      beginColor = mFriendBeaconBeginColor;
      endColor = mFriendBeaconEndColor;      
   }
       
   glColor4f(beginColor.red, beginColor.green, 
      beginColor.blue, beginColor.alpha);
   
   dglDrawBillboard(eyePos + dir * 100.f, width, 0.f);

   // targeted image:
   if(!projectile && !pilot || (beaconType == GameBase::friendBeacon))
      return;

   VectorF vector[2];
   F32 time[2];
   Point3F hit;
   
   if(!pilot)
   {
      // solveQuartic has problems if both velocities are |0|
      Point3F vel = beaconObj->getVelocity();
      Point3F srcVel = sourceVel;
      if(vel.isZero() && srcVel.isZero())
      {
         vel.set(1.f,0.f,0.f);
         srcVel.set(1.f,0.f,0.f);
      }

      // can reach it?
      if(!(const_cast<ProjectileData*>(projectile))->calculateAim(pos, vel, sourcePos, srcVel,
         &vector[0], &time[0], &vector[1], &time[1]))
         return;

      if(mShowAlternateTarget)
      {
         vector[0] = vector[1];
         time[0] = time[1];
      }

      // just show the min time target:
      dir = Point3F(pos - sourcePos);
      if( dir.isZero() ) return;
      dir.normalize();

      PlaneF plane(sourcePos + dir * 100.f, -vector[0]);
      F32 dist = plane.distToPlane(sourcePos);

      hit = sourcePos + (eyePos - sourcePos) + vector[0] * dist;

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, mBeaconTargetTexture.getGLName());
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

      F32 rot = 0.f;
      if(mBeaconTargetPeriod > 0)
         rot = (F32(Sim::getCurrentTime() % mBeaconTargetPeriod) * 2.f * M_PI) / F32(mBeaconTargetPeriod);

      width = F32(mBeaconTargetTexture.getWidth()) * (dist * mTan(mDegToRad(GameGetCameraFov()/2.f))) / F32(getExtent().x);
      glColor4f(beginColor.red, beginColor.green, 
                beginColor.blue, beginColor.alpha);

      dglDrawBillboard(hit, width, rot);
   }
   else
   {
      width = 5;
      dglDrawBillboard(Point3F(pos.x, pos.y, pilotHeight), width, 0.f);
      hit.set(pos.x, pos.y, pilotHeight - (width * 0.31f));
   }

   // render a line:
   glDisable(GL_TEXTURE_2D);
   glLineWidth(mBeaconLineWidth);

   glBegin(GL_LINES);
      glColor4f(beginColor.red, beginColor.green, 
                beginColor.blue, beginColor.alpha);
      glVertex3f(pos.x, pos.y, pos.z);

      glColor4f(beginColor.red, beginColor.green, 
                beginColor.blue, beginColor.alpha);
      glVertex3f(hit.x, hit.y, hit.z);
   glEnd();

   glLineWidth(1.f);
}

void GameTSCtrl::renderBeacons()
{
   if(!mBeaconsVisible)
      return;

   mBeaconTextPos.clear();

   if(!bool(mBeaconBaseTexture) || !bool(mBeaconTargetTexture))
      return;

   GameConnection * gc = GameConnection::getServerConnection();
   if(!gc)
      return;

   U32 sensorGroup = gc->getSensorGroup();

   // make sure firstperson...
   if(!gc->isFirstPerson())
      return;

   // make sure its a player?
   Player * player = static_cast<Player *>(gc->getControlObject());
   if(!player)
      return;

   F32 pilotHeight = 0.0f;
   if(player->isPilot())
   {
      Point3F pilotPos;
      player->getTransform().getColumn(3, &pilotPos);
      pilotHeight = pilotPos.z;  
   }   

   // grab the control object
   ShapeBase * controlObj = player->getControlObject();
   if(!controlObj)
      controlObj = player;

   // grab the weapon image:
   ShapeBaseImageData * weaponImage = controlObj->getMountedImage(controlObj->getActiveImage());

   ProjectileData * projectile = 0;
   if(weaponImage)
      projectile = weaponImage->projectile;

   // get the source point:
   Point3F muzzlePos;
   controlObj->getMuzzlePoint(0, &muzzlePos);

   // get the eye point;
   MatrixF mat;
   controlObj->getEyeTransform(&mat);

   Point3F eyePos;
   mat.getColumn(3, &eyePos);

   SimSet * targetSet = Sim::getClientTargetSet();
   for(SimSet::iterator itr = targetSet->begin(); itr != targetSet->end(); ++itr)
   {
      GameBase * target = dynamic_cast<GameBase*>(*itr);
      if(!target || !target->isBeacon())
         continue;
         
      S32 targId = target->getTarget();
      if((targId != -1) && (gTargetManager->getTargetSensorGroup(targId) == sensorGroup))
         renderBeacon(target, eyePos, muzzlePos, controlObj->getVelocity(), projectile, player->isPilot(), pilotHeight);
   }
}

void GameTSCtrl::renderBeaconsText()
{
   if(!mBeaconsVisible || !mBeaconTextPos.size())
      return;

   GameConnection * gc = GameConnection::getServerConnection();
   if(!gc)
      return;

   // grab the control object
   ShapeBase * controlObj = gc->getControlObject();
   if(!controlObj)
      return;

   // get the eye point;
   MatrixF mat;
   controlObj->getEyeTransform(&mat);

   Point3F eyePos;
   mat.getColumn(3, &eyePos);
      
   U32 height = mProfile->mFont->getHeight();

   Point3F sPos;
   for(U32 i = 0; i < mBeaconTextPos.size(); i++)
   {
      if(project(mBeaconTextPos[i], &sPos))
      {
         F32 dist = Point3F(mBeaconTextPos[i] - eyePos).len();
         dist = mClampF(dist, 0.f, 999.9f);

         char buf[40];
         dSprintf(buf, sizeof(buf), "%3.1f", dist);

         U32 width = mProfile->mFont->getStrWidth(buf);
         
         dglClearBitmapModulation();

         Point2I pos(S32(sPos.x - F32(width / 2)), S32(sPos.y - F32(height / 2)) + mBeaconTextYOffset);
         dglDrawText(mProfile->mFont, pos, buf);
      }  
   }
}

//---------------------------------------------------------------------------
static bool gHudHackCalled = false;
void GameTSCtrl::renderWorld(const RectI &updateRect)
{
//#pragma message("E3 hack here to reshow the HUD elements after loading is complete.");
   if (! gHudHackCalled)
   {
      gHudHackCalled = true;
      Con::executef(2, "HideHudHACK", "true");
   }

   GameRenderWorld();

   renderBeacons();
   dglSetClipRect(updateRect);
   renderBeaconsText();
}

//---------------------------------------------------------------------------
void GameTSCtrl::onMouseMove(const GuiEvent &evt)
{
   if(gSnapLine)
      return;

   MatrixF mat;
   Point3F vel;
   if ( GameGetCameraTransform(&mat, &vel) ) 
   {
      Point3F pos;
      mat.getColumn(3,&pos);
      Point3F screenPoint(evt.mousePoint.x, evt.mousePoint.y, -1);
      Point3F worldPoint;
      if (unproject(screenPoint, &worldPoint)) {
         Point3F vec = worldPoint - pos;
         lineTestStart = pos;
         vec.normalizeSafe();
         lineTestEnd = pos + vec * 1000;
      }
   }
}

void GameTSCtrl::onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder)
{
   // check if should bother with a render
   GameConnection * con = GameConnection::getServerConnection();
   bool skipRender = !con || (con->getWhiteOut() >= 1.f) || (con->getDamageFlash() >= 1.f) || (con->getBlackOut() >= 1.f);

   if(!skipRender)
   {
      Parent::onRender(offset, updateRect, firstResponder);
      renderChildControls(offset, updateRect, firstResponder);
   }

   dglSetViewport(updateRect);
   CameraQuery camq;
   if(GameProcessCameraQuery(&camq))  
      GameRenderFilters(camq);
}

//---------------------------------------------------------------------------
void GameTSCtrl::initPersistFields()
{
   Parent::initPersistFields();
   addField("beaconBaseTextureName", TypeString, Offset(mBeaconBaseTextureName, GameTSCtrl));
   addField("beaconTargetTextureName", TypeString, Offset(mBeaconTargetTextureName, GameTSCtrl));
   addField("beaconTargetPeriod", TypeS32, Offset(mBeaconTargetPeriod, GameTSCtrl));
   addField("beaconsVisible", TypeBool, Offset(mBeaconsVisible, GameTSCtrl));
   addField("enemyBeaconLineBeginColor", TypeColorF, Offset(mBeaconLineBeginColor, GameTSCtrl));
   addField("enemyBeaconLineEndColor", TypeColorF, Offset(mBeaconLineEndColor, GameTSCtrl));
   addField("vehicleBeaconLineBeginColor", TypeColorF, Offset(mVehicleBeaconBeginColor, GameTSCtrl));
   addField("vehicleBeaconLineEndColor", TypeColorF, Offset(mVehicleBeaconEndColor, GameTSCtrl));
   addField("friendBeaconLineBeginColor", TypeColorF, Offset(mFriendBeaconBeginColor, GameTSCtrl));
   addField("friendBeaconLineEndColor", TypeColorF, Offset(mFriendBeaconEndColor, GameTSCtrl));
   addField("beaconLineWidth", TypeF32, Offset(mBeaconLineWidth, GameTSCtrl));
   addField("beaconTextYOffset", TypeS32, Offset(mBeaconTextYOffset, GameTSCtrl));
   addField("showAlternateTarget", TypeBool, Offset(mShowAlternateTarget, GameTSCtrl));
}

//--------------------------------------------------------------------------
void cSnapToggle(SimObject *, S32, const char **)
{
   gSnapLine = !gSnapLine;
}

void GameTSCtrl::consoleInit()
{
   Con::addCommand("snapToggle", cSnapToggle, "snapToggle();", 1, 1);
}
