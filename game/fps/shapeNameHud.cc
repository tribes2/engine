//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
//-----------------------------------------------------------------------------

#include "game/fps/shapeNameHud.h"

#include "dgl/dgl.h"
#include "gui/guiTSControl.h"
#include "console/consoleTypes.h"
#include "terrain/sky.h"
#include "game/shapeBase.h"
#include "game/gameConnection.h"

IMPLEMENT_CONOBJECT(ShapeNameHud);

static const F32 cDefaultVisibleDistance = 500.0f;

ShapeNameHud::ShapeNameHud()
{
   mFillColor.set( 0.25, 0.25, 0.25, 0.25 );
   mFrameColor.set( 0, 1, 0, 1 );
   mTextColor.set( 0, 1, 0, 1 );
   mDamageFillColor.set( 0, 1, 0, 1 );
   mDamageFrameColor.set( 1, 0.6, 0, 1 );
   mDamageRectSize.set(50, 4);
   mOpacity = 0.5f;
   mShowFrame = mShowFill = true;
   mVerticalOffset = 0.5;
   mDistanceFade = 0.1;
}

void ShapeNameHud::initPersistFields()
{
   Parent::initPersistFields();
   addField( "fillColor", TypeColorF, Offset( mFillColor, ShapeNameHud ) );
   addField( "frameColor", TypeColorF, Offset( mFrameColor, ShapeNameHud ) );
   addField( "textColor", TypeColorF, Offset( mTextColor, ShapeNameHud ) );
   addField( "damageFillColor", TypeColorF, Offset( mDamageFillColor, ShapeNameHud ) );
   addField( "damageFrameColor", TypeColorF, Offset( mDamageFrameColor, ShapeNameHud ) );
   addField( "damageRect", TypePoint2I, Offset( mDamageRectSize, ShapeNameHud ) );

   addField( "opacity", TypeF32, Offset( mOpacity, ShapeNameHud ) );
   addField( "showFill", TypeBool, Offset( mShowFill, ShapeNameHud ) );
   addField( "showFrame", TypeBool, Offset( mShowFrame, ShapeNameHud ) );
   addField( "verticalOffset", TypeF32, Offset( mVerticalOffset, ShapeNameHud ) );
   addField( "distanceFade", TypeF32, Offset( mDistanceFade, ShapeNameHud ) );
}

void ShapeNameHud::onRender( Point2I, const RectI &updateRect,GuiControl *)
{
   // Border and background stuff.
   if (mShowFill) {
      mFillColor.alpha = mOpacity;
      dglDrawRectFill(updateRect, mFillColor);
   }
   if (mShowFrame)
      dglDrawRect(updateRect, mFrameColor);

   // Must be in a TS Control
   GuiTSCtrl *parent = dynamic_cast<GuiTSCtrl*>(getParent());
   if (!parent) return;

   // Must have a connection and control object
   GameConnection* conn = GameConnection::getServerConnection();
   if (!conn) return;
   ShapeBase* control = conn->getControlObject();
   if (!control) return;

   // Get control camera info
   MatrixF cam;
   Point3F camPos;
   VectorF camDir;
   conn->getControlCameraTransform(0,&cam);
   cam.getColumn(3, &camPos);
   cam.getColumn(1, &camDir);

   F32 camFov;
   conn->getControlCameraFov(&camFov);
   camFov = mDegToRad(camFov) / 2;

   // Visible distance info & name fading
   Sky* sky = gClientSceneGraph->getCurrentSky();
   F32 visDistance = sky ? sky->getVisibleDistance() : cDefaultVisibleDistance;
   F32 visDistanceSqr = visDistance * visDistance;
   F32 fadeDistance = visDistance * mDistanceFade;

   // Collision info. We're going to be running LOS tests and we
   // don't want to collide with the control object.
   static U32 losMask = TerrainObjectType | InteriorObjectType | ShapeBaseObjectType;
   control->disableCollision();

   // All ghosted objects are added to the server connection group,
   // so we can find all the shape base objects by iterating through
   // our current connection.
   for (SimSetIterator itr(conn); *itr; ++itr) {
      if ((*itr)->getType() & ShapeBaseObjectType) {
         ShapeBase* shape = static_cast<ShapeBase*>(*itr);
         if (shape != control && shape->getShapeName()) {

            // Target pos to test, if it's a player run the LOS to his eye
            // point, otherwise we'll grab the generic box center.
            Point3F shapePos;
            if (shape->getType() & PlayerObjectType) {
               MatrixF eye;
               shape->getEyeTransform(&eye);
               eye.getColumn(3, &shapePos);
            }
            else
               shapePos = shape->getBoxCenter();
            VectorF shapeDir = shapePos - camPos;

            // Test to see if it's in range
            F32 shapeDist = shapeDir.lenSquared();
            if (shapeDist == 0 || shapeDist > visDistanceSqr)
               continue;
            shapeDist = mSqrt(shapeDist);

            // Test to see if it's within our viewcone, this test doesn't
            // actually match the viewport very well, should consider
            // projection and box test.
            shapeDir.normalize();
            F32 dot = mDot(shapeDir, camDir);
            if (dot < camFov)
               continue;

            // Test to see if it's behind something, and we want to
            // ignore anything it's mounted on when we run the LOS.
            RayInfo info;
            shape->disableCollision();
            ShapeBase *mount = shape->getObjectMount();
            if (mount)
               mount->disableCollision();
            bool los = !gClientContainer.castRay(camPos, shapePos,losMask, &info);
            shape->enableCollision();
            if (mount)
               mount->enableCollision();
            if (!los)
               continue;
   
            // Project the shape pos into screen space and calculate
            // the distance opacity used to fade the labels into the
            // distance.
            Point3F projPnt;
            shapePos.z += mVerticalOffset;
            if (!parent->project(shapePos, &projPnt))
               continue;
            F32 opacity = (shapeDist < fadeDistance)? 1.0:
               1.0 - (shapeDist - fadeDistance) / (visDistance - fadeDistance);

            // Render the shape's name
            drawName(Point2I(projPnt.x, projPnt.y),shape->getShapeName(),opacity);

            // Render the shape's damage bar
            F32 damage = mClampF(1 - shape->getDamageValue(), 0, 1);
            drawDamage(Point2I(projPnt.x, projPnt.y),damage,opacity);
         }
      }
   }

   // Restore control object collision
   control->enableCollision();
}

void ShapeNameHud::drawName(Point2I offset, const char *name, F32 opacity)
{
   // Center the name
   offset.x -= mProfile->mFont->getStrWidth(name) / 2;
   offset.y -= mProfile->mFont->getHeight();

   //
   mTextColor.alpha = opacity;
   dglSetBitmapModulation(mTextColor);
   dglDrawText(mProfile->mFont, offset, name);
   dglClearBitmapModulation();
}

void ShapeNameHud::drawDamage(Point2I offset, F32 damage, F32 opacity)
{
   mDamageFillColor.alpha = mDamageFrameColor.alpha = opacity;

   // Center the bar
   RectI rect(offset, mDamageRectSize);
   rect.point.x -= mDamageRectSize.x / 2;

   // Draw the border
   dglDrawRect(rect, mDamageFrameColor);

   // Draw the damage % fill
   rect.point += Point2I(1, 1);
   rect.extent -= Point2I(1, 1);
   rect.extent.x = (S32)(rect.extent.x * damage);
   if (rect.extent.x == 1)
      rect.extent.x = 2;
   if (rect.extent.x > 0)
      dglDrawRectFill(rect, mDamageFillColor);
}   
