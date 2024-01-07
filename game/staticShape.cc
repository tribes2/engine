//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "Platform/platform.h"
#include "dgl/dgl.h"
#include "Core/dnet.h"
#include "Core/bitStream.h"
#include "game/game.h"
#include "Math/mMath.h"
#include "console/simBase.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "game/moveManager.h"
#include "ts/tsShapeInstance.h"
#include "Core/resManager.h"
#include "game/staticShape.h"
#include "Math/mathIO.h"
#include "game/shadow.h"

extern void wireCube(F32 size,Point3F pos);

static const U32 sgAllowedDynamicTypes = (StationObjectType   |
                                          GeneratorObjectType |
                                          SensorObjectType);

//----------------------------------------------------------------------------

IMPLEMENT_CO_DATABLOCK_V1(StaticShapeData);

StaticShapeData::StaticShapeData()
{
   dynamicTypeField     = 0;

   genericShadowLevel = StaticShape_GenericShadowLevel;
   noShadowLevel = StaticShape_NoShadowLevel;
   noIndividualDamage = false;
}

void StaticShapeData::initPersistFields()
{
   Parent::initPersistFields();

   addField("noIndividualDamage",   TypeBool, Offset(noIndividualDamage,   StaticShapeData));
   addField("dynamicType",          TypeS32,  Offset(dynamicTypeField,     StaticShapeData));
}

void StaticShapeData::packData(BitStream* stream)
{
   Parent::packData(stream);
   stream->writeFlag(noIndividualDamage);
   stream->write(dynamicTypeField);
}

void StaticShapeData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);
   noIndividualDamage = stream->readFlag();
   stream->read(&dynamicTypeField);
}


//----------------------------------------------------------------------------

IMPLEMENT_CO_NETOBJECT_V1(StaticShape);

StaticShape::StaticShape()
{
   mTypeMask |= StaticShapeObjectType | StaticObjectType;
   mDataBlock = 0;
}

StaticShape::~StaticShape()
{
}


//----------------------------------------------------------------------------

bool StaticShape::onAdd()
{
   if(!Parent::onAdd() || !mDataBlock)
      return false;

   // We need to modify our type mask based on what our datablock says...
   mTypeMask |= (mDataBlock->dynamicTypeField & sgAllowedDynamicTypes);

   addToScene();

   if (isServerObject())
      scriptOnAdd();
   return true;
}

bool StaticShape::onNewDataBlock(GameBaseData* dptr)
{
   mDataBlock = dynamic_cast<StaticShapeData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr))
      return false;

   scriptOnNewDataBlock();
   return true;
}

void StaticShape::onRemove()
{
   scriptOnRemove();
   removeFromScene();
   Parent::onRemove();
}


//----------------------------------------------------------------------------

void StaticShape::processTick(const Move* move)
{
   Parent::processTick(move);

   // Image Triggers
   if (move && mDamageState == Enabled) {
      setImageTriggerState(0,move->trigger[0]);
      setImageTriggerState(1,move->trigger[1]);
   }

   if (isMounted()) {
      MatrixF mat;
      mMount.object->getMountTransform(mMount.node,&mat);
      Parent::setTransform(mat);
      Parent::setRenderTransform(mat);
   }
}

void StaticShape::interpolateTick(F32)
{
   if (isMounted()) {
      MatrixF mat;
      mMount.object->getRenderMountTransform(mMount.node,&mat);
      Parent::setRenderTransform(mat);
   }

   if (mShadow)
   {
      mShadow->setMoving(false);
      if (mDataBlock->shape && mDataBlock->shape->sequences.empty())
         // no sequences, can't animate...
         mShadow->setAnimating(false);
   }
}

void StaticShape::setTransform(const MatrixF& mat)
{
   Parent::setTransform(mat);
   setMaskBits(PositionMask);
}

void StaticShape::onUnmount(ShapeBase*,S32)
{
   // Make sure the client get's the final server pos.
   setMaskBits(PositionMask);
}


//----------------------------------------------------------------------------

U32 StaticShape::packUpdate(NetConnection *connection, U32 mask, BitStream *bstream)
{
   U32 retMask = Parent::packUpdate(connection,mask,bstream);
   if (bstream->writeFlag(mask & PositionMask)) {
      bstream->writeAffineTransform(mObjToWorld);
      mathWrite(*bstream, mObjScale);
   }

   // powered?
   bstream->writeFlag(mPowered);

   return retMask;
}

void StaticShape::unpackUpdate(NetConnection *connection, BitStream *bstream)
{
   Parent::unpackUpdate(connection,bstream);
   if (bstream->readFlag()) {
      MatrixF mat;
      bstream->readAffineTransform(&mat);
      Parent::setTransform(mat);
      Parent::setRenderTransform(mat);

      VectorF scale;
      mathRead(*bstream, &scale);
      setScale(scale);
   }

   // powered?
   mPowered = bstream->readFlag();
}


//----------------------------------------------------------------------------
static void cSetPowered(SimObject * obj, S32, const char ** argv)
{
   StaticShape * sObj = static_cast<StaticShape *>(obj);
   if(!sObj->isServerObject())
      return;
   sObj->setPowered(dAtob(argv[2]));
}

static bool cIsPowered(SimObject * obj, S32, const char **)
{
   StaticShape * sObj = static_cast<StaticShape *>(obj);
   if(!sObj->isServerObject())
      return(false);
   return(sObj->isPowered());
}

void StaticShape::consoleInit()
{
   Con::addCommand("StaticShape", "setPoweredState", cSetPowered, "obj.setPoweredState(bool)", 3, 3);
   Con::addCommand("StaticShape", "getPoweredState", cIsPowered, "obj.getPoweredState(bool)", 2, 2);
}


