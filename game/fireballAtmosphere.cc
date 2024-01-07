//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "console/consoleTypes.h"
#include "dgl/dgl.h"
#include "audio/audioDataBlock.h"
#include "sceneGraph/sceneGraph.h"
#include "sceneGraph/sceneState.h"
#include "game/gameConnection.h"
#include "math/mathIO.h"
#include "game/player.h"
#include "game/fireballAtmosphere.h"
#include "game/Debris.h"
#include "math/mathUtils.h"

#define DML_DIR "textures/"
#define COLOR_OFFSET 0.25


IMPLEMENT_CO_NETOBJECT_V1(FireballAtmosphere);
IMPLEMENT_CO_DATABLOCK_V1(FireballAtmosphereData);


//**************************************************************************
// Fireball Atmosphere Data
//**************************************************************************
FireballAtmosphereData::FireballAtmosphereData()
{

   fireball       = NULL;
   fireballID     = 0;
}

IMPLEMENT_GETDATATYPE(FireballAtmosphereData)
IMPLEMENT_SETDATATYPE(FireballAtmosphereData)

//--------------------------------------------------------------------------
// Init persist fields
//--------------------------------------------------------------------------
void FireballAtmosphereData::initPersistFields()
{
   Parent::initPersistFields();

   Con::registerType(TypeGameBaseDataPtr, sizeof(FireballAtmosphereData*),
                     REF_GETDATATYPE(FireballAtmosphereData),
                     REF_SETDATATYPE(FireballAtmosphereData));
   
   addField("fireball",       TypeDebrisDataPtr,   Offset(fireball,        FireballAtmosphereData));

}

//--------------------------------------------------------------------------
// onAdd
//--------------------------------------------------------------------------
bool FireballAtmosphereData::onAdd()
{
   if (Parent::onAdd() == false)
      return false;


   if( !fireball && fireballID != 0 )
   {
      if( !Sim::findObject( SimObjectId( fireballID ), fireball ) )
      {
         Con::errorf( ConsoleLogEntry::General, "FireballAtmosphereData::preload: Invalid packet, bad datablockId(fireball): 0x%x", fireballID );
      }
   }

   return true;
}

//--------------------------------------------------------------------------
// Pack Data
//--------------------------------------------------------------------------
void FireballAtmosphereData::packData(BitStream* stream)
{
   Parent::packData(stream);

   if( stream->writeFlag( fireball ) )
   {
      stream->writeRangedU32(packed? SimObjectId(fireball):
         fireball->getId(),DataBlockObjectIdFirst,DataBlockObjectIdLast);
   }
  
}

//--------------------------------------------------------------------------
// Unpack Data
//--------------------------------------------------------------------------
void FireballAtmosphereData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   if(stream->readFlag())
   {
      fireballID = stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);
   }
}

//**************************************************************************
// Fireball Atmosphere
//**************************************************************************
FireballAtmosphere::FireballAtmosphere()
{
   mTimeSinceLastDrop = 0.0;

   mDropRadius     = 600.0;
   mDropsPerMinute = 3.0;
   mMinDropAngle   = 0.0;
   mMaxDropAngle   = 30.0;
   mStartVelocity  = 20.0;
   mDropHeight     = 500.0;
   mDropDir.set( 0.5, 0.5, -0.5 );
}

//--------------------------------------------------------------------------
// initPersistFields
//--------------------------------------------------------------------------
void FireballAtmosphere::initPersistFields()
{
   Parent::initPersistFields();

   addField("dropRadius",     TypeF32,             Offset(mDropRadius,        FireballAtmosphere));
   addField("dropsPerMinute", TypeF32,             Offset(mDropsPerMinute,    FireballAtmosphere));
   addField("minDropAngle",   TypeF32,             Offset(mMinDropAngle,      FireballAtmosphere));
   addField("maxDropAngle",   TypeF32,             Offset(mMaxDropAngle,      FireballAtmosphere));
   addField("startVelocity",  TypeF32,             Offset(mStartVelocity,     FireballAtmosphere));
   addField("dropHeight",     TypeF32,             Offset(mDropHeight,        FireballAtmosphere));
   addField("dropDir",        TypePoint3F,         Offset(mDropDir,           FireballAtmosphere));
}

//--------------------------------------------------------------------------
// onAdd
//--------------------------------------------------------------------------
bool FireballAtmosphere::onAdd()
{
   if(!Parent::onAdd())
      return false;

       
   if (isClientObject())
   {
      int test = 1;
   }
   else
   {
      int test = 1;
   }
   
   if( mDataBlock && mDataBlock->fireball )
   {
      mDataBlock->fireball->terminalVelocity = mStartVelocity;
   }

   mDropDir.normalize();

   mObjBox.min.set(-1e6, -1e6, -1e6);
   mObjBox.max.set( 1e6,  1e6,  1e6);

   resetWorldBox();

   addToScene();
  
   return true;
}

//--------------------------------------------------------------------------
// onRemove
//--------------------------------------------------------------------------
void FireballAtmosphere::onRemove()
{
   removeFromScene();

   Parent::onRemove();
}

//--------------------------------------------------------------------------
// onNewDataBlock
//--------------------------------------------------------------------------
bool FireballAtmosphere::onNewDataBlock(GameBaseData* dptr)
{
   mDataBlock = dynamic_cast<FireballAtmosphereData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr))
      return false;
   
   scriptOnNewDataBlock();
   return true;
}


//--------------------------------------------------------------------------
// prepRenderImage
//--------------------------------------------------------------------------
bool FireballAtmosphere::prepRenderImage(SceneState* state, const U32 stateKey,
                                    const U32 /*startZone*/, const bool /*modifyBaseState*/)
{

   if (isLastState(state, stateKey))
      return false;
   setLastState(state, stateKey);

   return false;
   
//    // This should be sufficient for most objects that don't manage zones, and
//    //  don't need to return a specialized RenderImage...
//    if (state->isObjectRendered(this)) {
//       SceneRenderImage* image = new SceneRenderImage;
//       image->obj = this;
//       image->isTranslucent = true;
//       image->sortKey = -1000.0;
//       state->insertRenderImage(image);
//    }

//    return false;
}

//--------------------------------------------------------------------------
// RENDER
//--------------------------------------------------------------------------
void FireballAtmosphere::renderObject(SceneState* , SceneRenderImage*)
{
   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on entry");


   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on exit");
}


//--------------------------------------------------------------------------
// advanceTime
//--------------------------------------------------------------------------
void FireballAtmosphere::advanceTime(F32 dt)
{
   mTimeSinceLastDrop += dt;
   
   F32 dropFrequency = 60.0 / mDropsPerMinute;

   if( mTimeSinceLastDrop > dropFrequency )
   {
      mTimeSinceLastDrop -= dropFrequency;
      dropNewFireball();
   }
}

//--------------------------------------------------------------------------
// dropNewFireball
//--------------------------------------------------------------------------
void FireballAtmosphere::dropNewFireball()
{
   MatrixF camTrans;
   GameConnection* connection = GameConnection::getServerConnection();
   connection->getControlCameraTransform( 0.0, &camTrans );

   
   Debris *fireball = new Debris;
   fireball->onNewDataBlock( mDataBlock->fireball );


   // set velocity
   VectorF launchVel = MathUtils::randomDir( mDropDir, mMinDropAngle, mMaxDropAngle );
   launchVel *= mStartVelocity;

   // set start point
   VectorF down( 0.0, 0.0, -1.0 );
   Point3F launchPoint = MathUtils::randomDir( down, 90.0, 90.0 );
   launchPoint *= mDropRadius * gRandGen.randF( 0.1, 1.0 );
   launchPoint += camTrans.getPosition();
   launchPoint.z = 0.0;

   F32 timeToHit = mDropHeight / launchVel.z;
   launchPoint += launchVel * timeToHit;
   
   
   if( !fireball->registerObject() )
   {
      delete fireball;
   }
   else
   {
      fireball->init( launchPoint, launchVel );
   }
   
}

//--------------------------------------------------------------------------
// packUpdate
//--------------------------------------------------------------------------
U32 FireballAtmosphere::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if( stream->writeFlag(mask & GameBase::InitialUpdateMask) )
   {
      stream->write(mDropRadius);
      stream->write(mDropsPerMinute);
      stream->write(mMaxDropAngle);
      stream->write(mMinDropAngle);
      stream->write(mStartVelocity);
      stream->write(mDropHeight);
      stream->write(mDropDir.x);
      stream->write(mDropDir.y);
      stream->write(mDropDir.z);
   }

   return retMask;
}

//--------------------------------------------------------------------------
// unpackUpdate
//--------------------------------------------------------------------------
void FireballAtmosphere::unpackUpdate(NetConnection* con, BitStream* stream)
{
   Parent::unpackUpdate(con, stream);

   if( stream->readFlag() )
   {
      stream->read(&mDropRadius);
      stream->read(&mDropsPerMinute);
      stream->read(&mMaxDropAngle);
      stream->read(&mMinDropAngle);
      stream->read(&mStartVelocity);
      stream->read(&mDropHeight);
      stream->read(&mDropDir.x);
      stream->read(&mDropDir.y);
      stream->read(&mDropDir.z);
  }
}
