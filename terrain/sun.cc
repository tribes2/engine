//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "terrain/Sun.h"
#include "console/objectTypes.h"
#include "sceneGraph/sceneGraph.h"
#include "Core/bitStream.h"
#include "console/consoleTypes.h"
#include "terrain/terrData.h"
#include "dgl/gBitmap.h"
#include "Math/mathIO.h"

IMPLEMENT_CO_NETOBJECT_V1(Sun);

//-----------------------------------------------------------------------------

Sun::Sun()
{
   mNetFlags.set(Ghostable | ScopeAlways);
   mTypeMask = EnvironmentObjectType;

   mLight.mType = LightInfo::Vector;
   mLight.mDirection.set(0.f, 0.707f, -0.707f);
   mLight.mColor.set(0.7f, 0.7f, 0.7f);
   mLight.mAmbient.set(0.3f, 0.3f, 0.3f);
}

//-----------------------------------------------------------------------------

void Sun::conformLight()
{
   mLight.mDirection.normalize();
   mLight.mColor.clamp();
   mLight.mAmbient.clamp();
}

//-----------------------------------------------------------------------------

bool Sun::onAdd()
{
   if(!Parent::onAdd())
      return(false);

   if(isClientObject())
      Sim::getLightSet()->addObject(this);
   else
      conformLight();

   return(true);
}

void Sun::registerLights(LightManager * lightManager, bool)
{
   lightManager->addLight(&mLight);
}

//-----------------------------------------------------------------------------

void Sun::inspectPostApply()
{
   conformLight();
   setMaskBits(UpdateMask);
}

void Sun::unpackUpdate(NetConnection *, BitStream * stream)
{
   if(stream->readFlag())
   {
      // direction -> color -> ambient
      mathRead(*stream, &mLight.mDirection);

      stream->read(&mLight.mColor.red);
      stream->read(&mLight.mColor.green);
      stream->read(&mLight.mColor.blue);
      stream->read(&mLight.mColor.alpha);

      stream->read(&mLight.mAmbient.red);
      stream->read(&mLight.mAmbient.green);
      stream->read(&mLight.mAmbient.blue);
      stream->read(&mLight.mAmbient.alpha);
   }
}

U32 Sun::packUpdate(NetConnection *, U32 mask, BitStream * stream)
{
   if(stream->writeFlag(mask & UpdateMask))
   {
      // direction -> color -> ambient
      mathWrite(*stream, mLight.mDirection);

      stream->write(mLight.mColor.red);
      stream->write(mLight.mColor.green);
      stream->write(mLight.mColor.blue);
      stream->write(mLight.mColor.alpha);

      stream->write(mLight.mAmbient.red);
      stream->write(mLight.mAmbient.green);
      stream->write(mLight.mAmbient.blue);
      stream->write(mLight.mAmbient.alpha);
   }
   return(0);
}

//-----------------------------------------------------------------------------

void Sun::initPersistFields()
{
   Parent::initPersistFields();
   addField("direction",   TypePoint3F,   Offset(mLight.mDirection, Sun));
   addField("color",       TypeColorF,    Offset(mLight.mColor, Sun));
   addField("ambient",     TypeColorF,    Offset(mLight.mAmbient, Sun));
}
