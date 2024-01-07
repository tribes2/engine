//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "dgl/dgl.h"
#include "platform/platform.h"
#include "core/dnet.h"
#include "audio/audio.h"
#include "game/gameConnection.h"
#include "game/moveManager.h"
#include "console/consoleTypes.h"
#include "core/bitStream.h"
#include "ts/tsPartInstance.h"
#include "ts/tsShapeInstance.h"
#include "scenegraph/sceneGraph.h"
#include "scenegraph/sceneState.h"
#include "game/shadow.h"
#include "game/explosion.h"
#include "game/shapeBase.h"
#include "game/targetManager.h"
#include "terrain/waterBlock.h"
#include "game/Debris.h"
#include "terrain/Sky.h"
#include "game/physicalZone.h"
#include "game/shieldImpact.h"
#include "scenegraph/detailManager.h"
#include "math/mathUtils.h"
#include "math/mMatrix.h"
#include "math/mRandom.h"
#include "game/commanderMapIcon.h"
#include "game/player.h"
#include "platform/profiler.h"
#include "game/targetManager.h"
#include "game/projSeeker.h"

IMPLEMENT_CO_DATABLOCK_V1(ShapeBaseData);


//----------------------------------------------------------------------------
// Timeout for non-looping sounds on a channel
static SimTime sAudioTimeout = 500;
bool ShapeBase::gRenderEnvMaps = true;
F32  ShapeBase::sWhiteoutDec = 0.007;
F32  ShapeBase::sDamageFlashDec = 0.007;
bool ShapeBase::sUsePrefSkins = false;
U32  ShapeBase::sLastRenderFrame = 0;

static const char *sDamageStateName[] =
{
   // Index by enum ShapeBase::DamageState
   "Enabled",
   "Disabled",
   "Destroyed"
};


//----------------------------------------------------------------------------

ShapeBaseData::ShapeBaseData()
{
   shapeName = "";
   mass = 1;
   drag = 0;
   density = 1;
   maxEnergy = 0;
   maxDamage = 1.0;
   disabledLevel = 1.0;
   destroyedLevel = 1.0;
   repairRate = 0.0033;
   eyeNode = -1;
   shadowNode = -1;
   cameraNode = -1;
   damageSequence = -1;
   hulkSequence = -1;
   cameraMaxDist = 0;
   cameraMinDist = 0.2;
   cameraDefaultFov = 90.f;
   cameraMinFov = 5.f;
   cameraMaxFov = 120.f;
   emap = false;
   aiAvoidThis = false;
   isInvincible = false;
   renderWhenDestroyed = true;
   debris = NULL;
   debrisID = 0;
   debrisShapeName = NULL;
   explosion = NULL;
   explosionID = 0;
   underwaterExplosion = NULL;
   underwaterExplosionID = 0;
   firstPersonOnly = false;
   useEyePoint = false;
   sensorRadius = 0.f;
   sensorColor.set(255,0,0,200);
   heat = 1.0;
   shieldEffectLifetimeMS = 300;
   
   cmdCategory = 0;
   cmdIcon = 0;
   cmdIconId = 0;
   cmdMiniIconName = 0;
   canControl = false;
   canObserve = false;
   observeThroughObject = false;
   computeCRC = false;
   
   for (U32 i = 0; i < MaxCollisionShapes; i++) {
      collisionDetails[i]      = -1;
      LOSDetails[i]            = -1;
   }

   // no shadows by default
   genericShadowLevel = 2.0f;
   noShadowLevel = 2.0f;

   inheritEnergyFromMount = false;

   for(U32 j = 0; j < NumHudRenderImages; j++)
   {
      hudImageNameFriendly[j] = 0;
      hudImageNameEnemy[j] = 0;
      hudRenderCenter[j] = false;
      hudRenderModulated[j] = false;
      hudRenderAlways[j] = false;
      hudRenderDistance[j] = false;
      hudRenderName[j] = false;
   }
}

static ShapeBaseData gShapeBaseDataProto;

ShapeBaseData::~ShapeBaseData()
{

}

bool ShapeBaseData::preload(bool server, char errorBuffer[256])
{
   if (!Parent::preload(server, errorBuffer))
      return false;

   // Resolve objects transmitted from server
   if (!server) {

      if( !explosion && explosionID != 0 )
      {
         if( Sim::findObject( explosionID, explosion ) == false)
         {
            Con::errorf( ConsoleLogEntry::General, "ShapeBaseData::preload: Invalid packet, bad datablockId(explosion): 0x%x", explosionID );
         }
         AssertFatal(!(explosion && ((explosionID < DataBlockObjectIdFirst) || (explosionID > DataBlockObjectIdLast))),
            "ShapeBaseData::preload: invalid explosion data");
      }

      if( !underwaterExplosion && underwaterExplosionID != 0 )
      {
         if( Sim::findObject( underwaterExplosionID, underwaterExplosion ) == false)
         {
            Con::errorf( ConsoleLogEntry::General, "ShapeBaseData::preload: Invalid packet, bad datablockId(underwaterExplosion): 0x%x", underwaterExplosionID );
         }
         AssertFatal(!(underwaterExplosion && ((underwaterExplosionID < DataBlockObjectIdFirst) || (underwaterExplosionID > DataBlockObjectIdLast))),
            "ShapeBaseData::preload: invalid underwaterExplosion data");
      }

      if( !debris && debrisID != 0 )
      {
         Sim::findObject( debrisID, debris );
         AssertFatal(!(debris && ((debrisID < DataBlockObjectIdFirst) || (debrisID > DataBlockObjectIdLast))),
            "ShapeBaseData::preload: invalid debris data");
      }


      if( debrisShapeName && debrisShapeName[0] != '\0' && !bool(debrisShape) )
      {
         char fullName[256];
         dSprintf(fullName, sizeof(fullName), "shapes/%s", debrisShapeName);

         debrisShape = ResourceManager->load(fullName);
         if( bool(debrisShape) == false )
         {
            dSprintf(errorBuffer, 256, "ShapeBaseData::load: Couldn't load shape \"%s\"", debrisShapeName);
            return false;
         }
         else
         {
            TSShapeInstance* pDummy = new TSShapeInstance(debrisShape, !server);
            delete pDummy;
         }
      }
   }
   
   //
   if (shapeName && shapeName[0]) {
      S32 i;

      // Resolve shapename
      char fullName[256];
      dSprintf(fullName,sizeof(fullName),"shapes/%s",shapeName);
      shape = ResourceManager->load(fullName, computeCRC);
      if (!bool(shape)) {
         dSprintf(errorBuffer, 256, "ShapeBaseData: Couldn't load shape \"%s\"",shapeName);
         return false;
      }
      if(computeCRC)
      {
         Con::printf("Validation required for shape: %s", shapeName);
         if(server)
            mCRC = shape.getCRC();
         else if(mCRC != shape.getCRC())
         {
            dSprintf(errorBuffer, 256, "Shape \"%s\" does not match version on server.",shapeName);
            return false;
         }
      }
      // Resolve details and camera node indexes.
      for (i = 0; i < MaxCollisionShapes; i++) {
         char buff[128];
         dSprintf(buff, sizeof(buff), "Collision-%d", i + 1);
         collisionDetails[i] = shape->findDetail(buff);
         if (collisionDetails[i] != -1) {
            shape->computeBounds(collisionDetails[i], collisionBounds[i]);
            shape->getAccelerator(collisionDetails[i]);

            if (!shape->bounds.isContained(collisionBounds[i]))
            {
               Con::warnf("Warning: shape %s collision detail %d (Collision-%d) bounds exceed that of shape.", shapeName, i, collisionDetails[i]);
               collisionBounds[i] = shape->bounds;
            }
            else if (collisionBounds[i].isValidBox() == false)
            {
               Con::errorf("Error: shape %s-collision detail %d (Collision-%d) bounds box invalid!", shapeName, i, collisionDetails[i]);
               collisionBounds[i] = shape->bounds;
            }
         }

         dSprintf(buff, sizeof(buff), "LOS-%d", i + 1 + MaxCollisionShapes);
         if ((LOSDetails[i] = shape->findDetail(buff)) == -1)
            LOSDetails[i] = collisionDetails[i];
      }

      debrisDetail = shape->findDetail("Debris-17");
      eyeNode = shape->findNode("eye");
      cameraNode = shape->findNode("cam");
      if (cameraNode == -1)
         cameraNode = eyeNode;

      // Resolve mount point node indexes
      for (i = 0; i < NumMountPoints; i++) {
         dSprintf(fullName,sizeof(fullName),"mount%d",i);
         mountPointNode[i] = shape->findNode(fullName);
      }

		// find the AIRepairNode - hardcoded to be the last node in the array...
      mountPointNode[AIRepairNode] = shape->findNode("AIRepairNode");

      //
      hulkSequence = shape->findSequence("Visibility");
      damageSequence = shape->findSequence("Damage");

      //
      F32 w = shape->bounds.len_y() / 2;
      if (cameraMaxDist < w)
         cameraMaxDist = w;
   }
   
   if(!server)
   {   
      if(!cmdIcon && (cmdIconId != 0))
         cmdIcon = dynamic_cast<CommanderIconData*>(Sim::findObject(cmdIconId));

      if(cmdMiniIconName && cmdMiniIconName[0])
         cmdMiniIcon = TextureHandle(cmdMiniIconName, BitmapTexture);

      // grab all the hud images
      for(U32 i = 0; i < NumHudRenderImages; i++)
      {
         if(hudImageNameFriendly[i] && hudImageNameFriendly[i][0])
            hudImageFriendly[i] = TextureHandle(hudImageNameFriendly[i], BitmapTexture);

         if(hudImageNameEnemy[i] && hudImageNameEnemy[i][0])
            hudImageEnemy[i] = TextureHandle(hudImageNameEnemy[i], BitmapTexture);
      }
   }

   return true;
}


void ShapeBaseData::initPersistFields()
{
   Parent::initPersistFields();
   addField("shapeFile",      TypeCaseString, Offset(shapeName,      ShapeBaseData));
   addField("explosion",      TypeExplosionDataPtr, Offset(explosion, ShapeBaseData));
   addField("underwaterExplosion", TypeExplosionDataPtr, Offset(underwaterExplosion, ShapeBaseData));
   addField("debris",         TypeDebrisDataPtr,    Offset(debris,   ShapeBaseData));
   addField("mass",           TypeF32,        Offset(mass,           ShapeBaseData));
   addField("drag",           TypeF32,        Offset(drag,           ShapeBaseData));
   addField("density",        TypeF32,        Offset(density,        ShapeBaseData));
   addField("maxEnergy",      TypeF32,        Offset(maxEnergy,      ShapeBaseData));
   addField("maxDamage",      TypeF32,        Offset(maxDamage,      ShapeBaseData));
   addField("disabledLevel",  TypeF32,        Offset(disabledLevel,  ShapeBaseData));
   addField("destroyedLevel", TypeF32,        Offset(destroyedLevel, ShapeBaseData));
   addField("repairRate",     TypeF32,        Offset(repairRate,     ShapeBaseData));
   addField("cameraMaxDist",  TypeF32,        Offset(cameraMaxDist,  ShapeBaseData));
   addField("cameraMinDist",  TypeF32,        Offset(cameraMinDist,  ShapeBaseData));
   addField("cameraDefaultFov", TypeF32,      Offset(cameraDefaultFov, ShapeBaseData));
   addField("cameraMinFov",   TypeF32,        Offset(cameraMinFov,   ShapeBaseData));
   addField("cameraMaxFov",   TypeF32,        Offset(cameraMaxFov,   ShapeBaseData));      
   addField("emap",           TypeBool,       Offset(emap,           ShapeBaseData));
   addField("aiAvoidThis",  	TypeBool, 		 Offset(aiAvoidThis,    ShapeBaseData));
   addField("isInvincible",   TypeBool,       Offset(isInvincible,   ShapeBaseData));
   addField("inheritEnergyFromMount", TypeBool, Offset(inheritEnergyFromMount, ShapeBaseData));
   addField("renderWhenDestroyed",   TypeBool,  Offset(renderWhenDestroyed,   ShapeBaseData));
   addField("debrisShapeName", TypeString,    Offset(debrisShapeName, ShapeBaseData));
   addField("firstPersonOnly", TypeBool,      Offset(firstPersonOnly, ShapeBaseData));
   addField("useEyePoint",     TypeBool,      Offset(useEyePoint,     ShapeBaseData));
   addField("sensorRadius",   TypeF32,        Offset(sensorRadius,   ShapeBaseData));
   addField("sensorColor",    TypeColorI,     Offset(sensorColor,    ShapeBaseData));
   addField("cmdCategory",    TypeString,     Offset(cmdCategory,    ShapeBaseData));
   addField("cmdIcon",        TypeCommanderIconDataPtr, Offset(cmdIcon, ShapeBaseData));
   addField("cmdMiniIconName",TypeString,     Offset(cmdMiniIconName,ShapeBaseData));
   addField("canControl",     TypeBool,       Offset(canControl,     ShapeBaseData));
   addField("canObserve",     TypeBool,       Offset(canObserve,     ShapeBaseData));
   addField("observeThroughObject", TypeBool, Offset(observeThroughObject, ShapeBaseData));
   addField("computeCRC",     TypeBool,       Offset(computeCRC,     ShapeBaseData));
   addField("heatSignature",  TypeF32,        Offset(heat,          ShapeBaseData));
   addField("shieldEffectLifetimeMS",  TypeS32,  Offset(shieldEffectLifetimeMS,  ShapeBaseData));

   AssertFatal(NumHudRenderImages == TargetInfo::NumHudRenderImages, "ShapeBaseData: num hud render images mismatch");
   addField("hudImageName",         TypeString,    Offset(hudImageNameFriendly, ShapeBaseData), NumHudRenderImages);
   addField("hudImageNameFriendly", TypeString,    Offset(hudImageNameFriendly, ShapeBaseData), NumHudRenderImages);
   addField("hudImageNameEnemy",    TypeString,    Offset(hudImageNameEnemy, ShapeBaseData), NumHudRenderImages);
   addField("hudRenderCenter",      TypeBool,      Offset(hudRenderCenter, ShapeBaseData), NumHudRenderImages);
   addField("hudRenderModulated",   TypeBool,      Offset(hudRenderModulated, ShapeBaseData), NumHudRenderImages);
   addField("hudRenderAlways",      TypeBool,      Offset(hudRenderAlways, ShapeBaseData), NumHudRenderImages);
   addField("hudRenderDistance",    TypeBool,      Offset(hudRenderDistance, ShapeBaseData), NumHudRenderImages);
   addField("hudRenderName",        TypeBool,      Offset(hudRenderName, ShapeBaseData), NumHudRenderImages);
}


static bool cCheckDeployPos(SimObject* obj, S32, const char** argv)
{
   ShapeBaseData* pData = static_cast<ShapeBaseData*>(obj);
   if (bool(pData->shape) == false)
      return false;
   
   Point3F pos(0, 0, 0);
   AngAxisF aa(Point3F(0, 0, 1), 0);
   dSscanf(argv[2],"%f %f %f %f %f %f %f",
           &pos.x,&pos.y,&pos.z,&aa.axis.x,&aa.axis.y,&aa.axis.z,&aa.angle);
   MatrixF mat;
   aa.setMatrix(&mat);
   mat.setColumn(3,pos);

   Box3F objBox = pData->shape->bounds;
   Point3F boxCenter = (objBox.min + objBox.max) * 0.5;
   objBox.min = boxCenter + (objBox.min - boxCenter) * 0.9;
   objBox.max = boxCenter + (objBox.max - boxCenter) * 0.9;

   Box3F wBox = objBox;
   mat.mul(wBox);
   
   EarlyOutPolyList polyList;
   polyList.mNormal.set(0,0,0);
   polyList.mPlaneList.clear();
   polyList.mPlaneList.setSize(6);
   polyList.mPlaneList[0].set(objBox.min,VectorF(-1,0,0));
   polyList.mPlaneList[1].set(objBox.max,VectorF(0,1,0));
   polyList.mPlaneList[2].set(objBox.max,VectorF(1,0,0));
   polyList.mPlaneList[3].set(objBox.min,VectorF(0,-1,0));
   polyList.mPlaneList[4].set(objBox.min,VectorF(0,0,-1));
   polyList.mPlaneList[5].set(objBox.max,VectorF(0,0,1));

   for (U32 i = 0; i < 6; i++)
   {
      PlaneF temp;
      mTransformPlane(mat, Point3F(1, 1, 1), polyList.mPlaneList[i], &temp);
      polyList.mPlaneList[i] = temp;
   }
   
   if (gServerContainer.buildPolyList(wBox, InteriorObjectType | StaticShapeObjectType, &polyList))
      return false;
   return true;
}


static const char* cGetDeployTransform(SimObject*, S32, const char** argv)
{
   Point3F normal;
   Point3F position;
   dSscanf(argv[2], "%f %f %f", &position.x, &position.y, &position.z);
   dSscanf(argv[3], "%f %f %f", &normal.x, &normal.y, &normal.z);
   normal.normalize();
   
   VectorF xAxis;
   if( mFabs(normal.z) > mFabs(normal.x) && mFabs(normal.z) > mFabs(normal.y))
      mCross( VectorF( 0, 1, 0 ), normal, &xAxis );
   else
      mCross( VectorF( 0, 0, 1 ), normal, &xAxis );
   
   VectorF yAxis;
   mCross( normal, xAxis, &yAxis );

   MatrixF testMat(true);
   testMat.setColumn( 0, xAxis );
   testMat.setColumn( 1, yAxis );
   testMat.setColumn( 2, normal );
   testMat.setPosition( position );

   char *returnBuffer = Con::getReturnBuffer(256);
   Point3F pos;
   testMat.getColumn(3,&pos);
   AngAxisF aa(testMat);
   dSprintf(returnBuffer,256,"%g %g %g %g %g %g %g",
            pos.x,pos.y,pos.z,aa.axis.x,aa.axis.y,aa.axis.z,aa.angle);
   return returnBuffer;
}


void ShapeBaseData::consoleInit()
{
   Con::addCommand("ShapeBaseData", "checkDeployPos", cCheckDeployPos, "obj.checkDeployPos(xform)", 3, 3);
   Con::addCommand("ShapeBaseData", "getDeployTransform", cGetDeployTransform, "obj.getDeployTransform(pos, normal)", 4, 4);
}


void ShapeBaseData::packData(BitStream* stream)
{
   Parent::packData(stream);

   if(stream->writeFlag(computeCRC))
      stream->write(mCRC);

   stream->writeString(shapeName);
   if(stream->writeFlag(mass != gShapeBaseDataProto.mass))
      stream->write(mass);
   if(stream->writeFlag(drag != gShapeBaseDataProto.drag))
      stream->write(drag);
   if(stream->writeFlag(density != gShapeBaseDataProto.density))
      stream->write(density);
   if(stream->writeFlag(maxEnergy != gShapeBaseDataProto.maxEnergy))
      stream->write(maxEnergy);
   if(stream->writeFlag(cameraMaxDist != gShapeBaseDataProto.cameraMaxDist))
      stream->write(cameraMaxDist);
   if(stream->writeFlag(cameraMinDist != gShapeBaseDataProto.cameraMinDist))
      stream->write(cameraMinDist);
   cameraDefaultFov = mClampF(cameraDefaultFov, cameraMinFov, cameraMaxFov);
   if(stream->writeFlag(cameraDefaultFov != gShapeBaseDataProto.cameraDefaultFov))
      stream->write(cameraDefaultFov);
   if(stream->writeFlag(cameraMinFov != gShapeBaseDataProto.cameraMinFov))
      stream->write(cameraMinFov);
   if(stream->writeFlag(cameraMaxFov != gShapeBaseDataProto.cameraMaxFov))
      stream->write(cameraMaxFov);
   stream->writeString( debrisShapeName );

   if(stream->writeFlag(sensorRadius != 0.f))
   {
      stream->writeInt(sensorRadius, 10);
      stream->write(sensorColor.red);
      stream->write(sensorColor.green);
      stream->write(sensorColor.blue);
      stream->write(sensorColor.alpha);
   }
   if(stream->writeFlag(heat != gShapeBaseDataProto.heat))
      stream->write(heat);
         
   stream->writeString(cmdCategory);
   if(stream->writeFlag(cmdIcon))
      stream->writeRangedU32(cmdIcon->getId(), DataBlockObjectIdFirst, DataBlockObjectIdLast);
   stream->writeString(cmdMiniIconName);
   stream->writeFlag(canControl);
   stream->writeFlag(canObserve);
   stream->writeFlag(observeThroughObject);

   if( stream->writeFlag( debris != NULL ) )
   {
      stream->writeRangedU32(packed? SimObjectId(debris):
                             debris->getId(),DataBlockObjectIdFirst,DataBlockObjectIdLast);
   }

   stream->writeFlag(emap);
   stream->writeFlag(isInvincible);
   stream->writeFlag(renderWhenDestroyed);

   if( stream->writeFlag( explosion != NULL ) )
   {
      stream->writeRangedU32( explosion->getId(), DataBlockObjectIdFirst,  DataBlockObjectIdLast );
   }

   if( stream->writeFlag( underwaterExplosion != NULL ) )
   {
      stream->writeRangedU32( underwaterExplosion->getId(), DataBlockObjectIdFirst,  DataBlockObjectIdLast );
   }

   stream->writeFlag(inheritEnergyFromMount);
   stream->writeFlag(firstPersonOnly);
   stream->writeFlag(useEyePoint);

   stream->write( shieldEffectLifetimeMS );

   // hud images...
   for(U32 i = 0; i < TargetInfo::NumHudRenderImages; i++)
   {
      // must at least have a friendly image.. (default if no enemy)
      if(stream->writeFlag(hudImageNameFriendly[i] && hudImageNameFriendly[i][0]))  
      {
         stream->writeString(hudImageNameFriendly[i]);

         if(stream->writeFlag(hudImageNameEnemy[i] && hudImageNameEnemy[i][0]))
            stream->writeString(hudImageNameEnemy[i]);

         stream->writeFlag(hudRenderCenter[i]);
         stream->writeFlag(hudRenderModulated[i]);
         stream->writeFlag(hudRenderAlways[i]);
         stream->writeFlag(hudRenderDistance[i]);
         stream->writeFlag(hudRenderName[i]);
      }
   }
}

void ShapeBaseData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);
   computeCRC = stream->readFlag();
   if(computeCRC)
      stream->read(&mCRC);

   shapeName = stream->readSTString();
   if(stream->readFlag())
      stream->read(&mass);
   else
      mass = gShapeBaseDataProto.mass;

   if(stream->readFlag())
      stream->read(&drag);
   else
      drag = gShapeBaseDataProto.drag;

   if(stream->readFlag())
      stream->read(&density);
   else
      density = gShapeBaseDataProto.density;

   if(stream->readFlag())
      stream->read(&maxEnergy);
   else
      maxEnergy = gShapeBaseDataProto.maxEnergy;

   if(stream->readFlag())
      stream->read(&cameraMaxDist);
   else
      cameraMaxDist = gShapeBaseDataProto.cameraMaxDist;

   if(stream->readFlag())
      stream->read(&cameraMinDist);
   else
      cameraMinDist = gShapeBaseDataProto.cameraMinDist;

   if(stream->readFlag())
      stream->read(&cameraDefaultFov);
   else
      cameraDefaultFov = gShapeBaseDataProto.cameraDefaultFov;

   if(stream->readFlag())
      stream->read(&cameraMinFov);
   else
      cameraMinFov = gShapeBaseDataProto.cameraMinFov;

   if(stream->readFlag())
      stream->read(&cameraMaxFov);
   else
      cameraMaxFov = gShapeBaseDataProto.cameraMaxFov;

   debrisShapeName = stream->readSTString();

   if(stream->readFlag())
   {  
      sensorRadius = stream->readInt(10);
      stream->read(&sensorColor.red);
      stream->read(&sensorColor.green);
      stream->read(&sensorColor.blue);
      stream->read(&sensorColor.alpha);
   }
   else
   {
      sensorRadius = 0.f;
   }
   if(stream->readFlag())
      stream->read(&heat);
   
   cmdCategory = stream->readSTString();
   if(stream->readFlag()) 
      cmdIconId = stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);
   cmdMiniIconName = stream->readSTString();
   canControl = stream->readFlag();
   canObserve = stream->readFlag();
   observeThroughObject = stream->readFlag();

   if( stream->readFlag() )
   {
      debrisID = stream->readRangedU32( DataBlockObjectIdFirst, DataBlockObjectIdLast );
   }

   emap = stream->readFlag();
   isInvincible = stream->readFlag();
   renderWhenDestroyed = stream->readFlag();
   
   if( stream->readFlag() )
   {
      explosionID = stream->readRangedU32( DataBlockObjectIdFirst, DataBlockObjectIdLast );
   }

   if( stream->readFlag() )
   {
      underwaterExplosionID = stream->readRangedU32( DataBlockObjectIdFirst, DataBlockObjectIdLast );
   }

   inheritEnergyFromMount = stream->readFlag();
   firstPersonOnly = stream->readFlag();
   useEyePoint = stream->readFlag();

   stream->read( &shieldEffectLifetimeMS );

   // hud images...
   for(U32 i = 0; i < TargetInfo::NumHudRenderImages; i++)
   {
      if(stream->readFlag())
      {
         hudImageNameFriendly[i] = stream->readSTString();

         if(stream->readFlag())
            hudImageNameEnemy[i] = stream->readSTString();

         hudRenderCenter[i] = stream->readFlag();
         hudRenderModulated[i] = stream->readFlag();
         hudRenderAlways[i] = stream->readFlag();
         hudRenderDistance[i] = stream->readFlag();
         hudRenderName[i] = stream->readFlag();
      }
   }
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

Chunker<ShapeBase::CollisionTimeout> sTimeoutChunker;
ShapeBase::CollisionTimeout*         sFreeTimeoutList = 0;


//----------------------------------------------------------------------------

IMPLEMENT_CO_NETOBJECT_V1(ShapeBase);

ShapeBase::ShapeBase()
{
   mTypeMask |= ShapeBaseObjectType;

   mDrag = 0;
   mBuoyancy = 0;
   mWaterCoverage = 0;
   mLiquidType = 0;
   mLiquidHeight = 0.0f;
   mControllingClient = 0;
   mControllingObject = 0;

   mGravityMod = 1.0;
   mAppliedForce.set(0, 0, 0);

   mTimeoutList = 0;
   mDataBlock = NULL;
   mShapeInstance = 0;
   mShadow = 0;
   mGenerateShadow = false;
   mEnergy = 0;
   mRechargeRate = 0;
   mDamage = 0;
   mRepairRate = 0;
   mRepairReserve = 0;
   mDamageState = Enabled;
   mDamageThread = 0;
   mHulkThread = 0;
   mLastRenderFrame = 0;
   mLastRenderDistance = 0;

   mCloaked    = false;
   mCloakLevel = 0.0;

   mPassiveJammed = false;

   mMount.object = 0;
   mMount.link = 0;
   mMount.list = 0;

   mActiveImage = 0;
   mHidden = false;

   for (int a = 0; a < MaxSoundThreads; a++) {
      mSoundThread[a].play = false;
      mSoundThread[a].profile = 0;
      mSoundThread[a].sound = 0;
   }

   S32 i;
   for (i = 0; i < MaxScriptThreads; i++) {
      mScriptThread[i].sequence = -1;
      mScriptThread[i].thread = 0;
      mScriptThread[i].sound = 0;
      mScriptThread[i].state = Thread::Stop;
      mScriptThread[i].atEnd = false;
      mScriptThread[i].forward = true;
   }

   for (i = 0; i < MaxTriggerKeys; i++)
      mTrigger[i] = false;

   mSkinTag = 0;
   mSkinPrefTag = 0;

   mDamageFlash = 0.0;
   mWhiteOut    = 0.0;

   mInvincibleEffect = 0.0f;
   mInvincibleDelta  = 0.0f;
   mInvincibleCount  = 0.0f;   
   mInvincibleSpeed  = 0.0f;   
   mInvincibleTime   = 0.0f;   
   mInvincibleFade   = 0.1;
   mInvincibleOn     = false;
 
   mTracking = false;
   mLockedOn = NotLocked;
   
   mPotentialTargets = NULL;
   mIsControlled = false;

   mConvexList = new Convex;
   mCameraFov = 90.f;
   mShieldNormal.set(0, 0, 1);

   mFadeOut = true;
   mFading = false;
   mFadeVal = 1.0;
   mFadeTime = 1.0;
   mFadeElapsedTime = 0.0;
   mFadeDelay = 0.0;
   mFlipFadeVal = false;
   mLightTime = 0;
   blowApart = false;
   damageDir.set(0, 0, 1);
}


ShapeBase::~ShapeBase()
{
   delete mConvexList;
   mConvexList = NULL;

   AssertFatal(mMount.link == 0,"ShapeBase::~ShapeBase: An object is still mounted");
   if( mShapeInstance && (mShapeInstance->getDebrisRefCount() == 0) )
   {
      delete mShapeInstance;
   }
   delete mShadow;

   CollisionTimeout* ptr = mTimeoutList;
   while (ptr) {
      CollisionTimeout* cur = ptr;
      ptr = ptr->next;
      cur->next = sFreeTimeoutList;
      sFreeTimeoutList = cur;
   }
}


//----------------------------------------------------------------------------

bool ShapeBase::onAdd()
{
   if(!Parent::onAdd())
      return false;

   // Resolve sounds that arrived in the initial update
   S32 i;
   for (i = 0; i < MaxSoundThreads; i++)
      updateAudioState(mSoundThread[i]);

   for (i = 0; i < MaxScriptThreads; i++)
   {
      Thread& st = mScriptThread[i];
      if(st.thread)
         updateThread(st);
   }
      
   if (isClientObject())
   {
      mCloakTexture = TextureHandle("special/cloakTexture", MeshTexture, false);
      mShieldEffect.init();

      //one of the mounted images must have a light source...
      for (S32 i = 0; i < MaxMountedImages; i++)
      {
         ShapeBaseImageData* imageData = getMountedImage(i);
         if (imageData != NULL && imageData->lightType != ShapeBaseImageData::NoLight)
         {
            Sim::getLightSet()->addObject(this);
            break;
         }
      }
   }

   setHeat(mDataBlock->heat);

   return true;
}

void ShapeBase::onRemove()
{
   mConvexList->nukeList();

   unmount();
   Parent::onRemove();

   // Stop any running sounds on the client
   if (isGhost())
      for (S32 i = 0; i < MaxSoundThreads; i++)
         stopAudio(i);
}


void ShapeBase::onSceneRemove()
{
   mConvexList->nukeList();
   Parent::onSceneRemove();
}



static void reSkin(TSShapeInstance *instance, const char *newBase, const char *prefBase)
{
   if (instance->ownMaterialList() == false)
      instance->cloneMaterialList();

   TSMaterialList* pMatList = instance->getMaterialList();

   for (U32 j = 0; j < pMatList->mMaterialNames.size(); j++) {
      const char* pName = pMatList->mMaterialNames[j];
      if (pName == NULL)
         continue;
      const U32 len     = dStrlen(pName);
      if (len < 6)
         continue;

      const char* pReplace = dStrstr(pName, (const char*)"base.");
      if (pReplace == NULL)
         continue;

      char newName[256];
      AssertFatal(len < 200, "ShapeBase::checkSkin: Error, len exceeds allowed name length");
      TextureHandle test;

      if(ShapeBase::sUsePrefSkins && prefBase[0])
      {
         dStrncpy(newName, pName, pReplace - pName);
         newName[pReplace - pName] = '\0';
         dStrcat(newName, prefBase);
         dStrcat(newName, ".");
         dStrcat(newName, pName + 5 + (pReplace - pName));
         test = TextureHandle(newName, MeshTexture, false);
      }
      if(test.getGLName() == 0)
      {
         dStrncpy(newName, pName, pReplace - pName);
         newName[pReplace - pName] = '\0';
         dStrcat(newName, newBase);
         dStrcat(newName, ".");
         dStrcat(newName, pName + 5 + (pReplace - pName));

         test = TextureHandle(newName, MeshTexture, false);
      }
      if (test.getGLName() != 0) {
         pMatList->mMaterials[j] = test;
      } else {
         pMatList->mMaterials[j] = TextureHandle(pName, MeshTexture, false);
      }
   }
}

bool ShapeBase::onNewDataBlock(GameBaseData* dptr)
{
   if (Parent::onNewDataBlock(dptr) == false)
      return false;

   mDataBlock = dynamic_cast<ShapeBaseData*>(dptr);
   if (!mDataBlock)
      return false;

   setMaskBits(DamageMask);
   mDamageThread = 0;
   mHulkThread = 0;

   // Even if loadShape succeeds, there may not actually be
   // a shape assigned to this object.
   if (bool(mDataBlock->shape)) {
      delete mShapeInstance;
      mShapeInstance = new TSShapeInstance(mDataBlock->shape, isClientObject());
      if (isClientObject())
         mShapeInstance->cloneMaterialList();

      mObjBox = mDataBlock->shape->bounds;
      resetWorldBox();

      // Initialize the threads
      for (U32 i = 0; i < MaxScriptThreads; i++) {
         Thread& st = mScriptThread[i];
         if (st.sequence != -1) {
            // TG: Need to see about supressing non-cyclic sounds
            // if the sequences were actived before the object was
            // ghosted.
            // TG: Cyclic animations need to have a random pos if
            // they were started before the object was ghosted.

            // If there was something running on the old shape, the thread
            // needs to be reset. Otherwise we assume that it's been
            // initialized either by the constructor or from the server.
            bool reset = st.thread != 0;
            st.thread = 0;
            setThreadSequence(i,st.sequence,reset);
         }
      }

      // get rid of current shadow...we'll generate new one when needed
      delete mShadow;
      mShadow = NULL;

      if (mDataBlock->damageSequence != -1) {
         mDamageThread = mShapeInstance->addThread();
         mShapeInstance->setSequence(mDamageThread,
                                     mDataBlock->damageSequence,0);
      }
      if (mDataBlock->hulkSequence != -1) {
         mHulkThread = mShapeInstance->addThread();
         mShapeInstance->setSequence(mHulkThread,
                                     mDataBlock->hulkSequence,0);
      }
   }
   if(isGhost() && mSkinTag)
   {
      reSkin(mShapeInstance, gNetStringTable->lookupString(mSkinTag), 
             mSkinPrefTag ? gNetStringTable->lookupString(mSkinPrefTag) : "");
      mSkinHash = (_StringTable::hashString(gNetStringTable->lookupString(mSkinTag)) ^
                   _StringTable::hashString(mSkinPrefTag ? gNetStringTable->lookupString(mSkinPrefTag) : ""));
   }

   // 
   mEnergy = 0;
   mDamage = 0;
   mDamageState = Enabled;
   mRepairReserve = 0;
   updateMass();
   updateDamageLevel();
   updateDamageState();

   mCameraFov = mDataBlock->cameraDefaultFov;
   return true;
}

void ShapeBase::onDeleteNotify(SimObject* obj)
{
   if (obj == getProcessAfter())
      clearProcessAfter();
   Parent::onDeleteNotify(obj);
   if (obj == mMount.object)
      unmount();
}

void ShapeBase::onImpact(SceneObject* obj, VectorF vec)
{
   if (!isGhost()) {
      char buff1[256];
      char buff2[32];
      
      dSprintf(buff1,sizeof(buff1),"%f %f %f",vec.x, vec.y, vec.z);
      dSprintf(buff2,sizeof(buff2),"%f",vec.len());
      Con::executef(mDataBlock,5,"onImpact",scriptThis(), obj->getIdString(), buff1, buff2);
   }
}

void ShapeBase::onImpact(VectorF vec)
{
   if (!isGhost()) {
      char buff1[256];
      char buff2[32];
      
      dSprintf(buff1,sizeof(buff1),"%f %f %f",vec.x, vec.y, vec.z);
      dSprintf(buff2,sizeof(buff2),"%f",vec.len());
      Con::executef(mDataBlock,5,"onImpact",scriptThis(), "0", buff1, buff2);
   }
}


//----------------------------------------------------------------------------

void ShapeBase::processTick(const Move* move)
{
   // Energy management
   if (mDamageState == Enabled && mDataBlock->inheritEnergyFromMount == false) {
      mEnergy += mRechargeRate;
      if (mEnergy > mDataBlock->maxEnergy)
         mEnergy = mDataBlock->maxEnergy;
      else if (mEnergy < 0)
         mEnergy = 0;

      // set update mask on vehicles...
      if(isServerObject() && getMountList())
      {
         bool found = false;
         for (ShapeBase* ptr = getMountList(); ptr; ptr = ptr->getMountLink())
         { 
            if(!dynamic_cast<Player*>(ptr))
               continue;

            GameConnection * controllingClient = ptr->getControllingClient();
            if(!controllingClient || (controllingClient->getControlObject() == this))
               continue;

            found = true;
            break;
         }

         if(found)
            setEnergyLevel(mEnergy);
      }
   }

   // Repair management
   if (mDataBlock->isInvincible == false)
   {
      F32 store = mDamage;
      mDamage -= mRepairRate;
      mDamage = mClampF(mDamage, 0.f, mDataBlock->maxDamage);

      if (mRepairReserve > mDamage)
         mRepairReserve = mDamage;
      if (mRepairReserve > 0.0)
      {
         F32 rate = getMin(mDataBlock->repairRate, mRepairReserve);
         mDamage -= rate;
         mRepairReserve -= rate;
      }

      if (store != mDamage)
      {
         updateDamageLevel();
         if (isServerObject()) {
            setMaskBits(DamageMask);
            Con::executef(mDataBlock,2,"onDamage",scriptThis());
         }
      }
   }

   if (isServerObject()) {
      // Server only...
      advanceThreads(TickSec);
      updateServerAudio();
      
      // update wet state
      setImageWetState(0, mWaterCoverage > 0.4); // more than 40 percent covered
      
      if (mWaterCoverage < 0.4 && getContainer() && getMountedImage(0) != NULL && getMountedImage(0)->isSeeker == true && move != NULL) {
         thinkAboutLocking();
         
         if (mLockedOn == NotLocked && mPotentialTargets != NULL)
            mTracking = true;
         else
            mTracking = false;
            
         setImageWetState(0, 0);
            
         // update the targeting for seeker images
         setImageTargetState(0, mTracking || ((mLockedOn == LockObject) || (mLockedOn == LockPosition)));
      } else {
         mTracking = false;
         if (mPotentialTargets != NULL) {
            PotentialLock* walk = mPotentialTargets;
            while (walk) {
               PotentialLock* pGarbage = walk;
               walk = walk->next;
               delete pGarbage;
            }
            mPotentialTargets = NULL;
         }
         setLockedTarget(NULL);
      }
      
      if(mFading)
      {
         F32 dt = TickMs / 1000.0;
         F32 newFadeET = mFadeElapsedTime + dt;
         if(mFadeElapsedTime < mFadeDelay && newFadeET >= mFadeDelay)
            setMaskBits(CloakMask);
         mFadeElapsedTime = newFadeET;
         if(mFadeElapsedTime > mFadeTime + mFadeDelay)
         {
            mFadeVal = F32(!mFadeOut);
            mFading = false;
         }
      }
   }

   // Advance images
   for (int i = 0; i < MaxMountedImages; i++)
   {
      if (mMountedImageList[i].dataBlock != NULL)
         updateImageState(i, TickSec);
   }

   // Call script on trigger state changes
   if (move && mDataBlock && isServerObject()) {
      for (S32 i = 0; i < MaxTriggerKeys; i++) {
         if (move->trigger[i] != mTrigger[i]) {
            mTrigger[i] = move->trigger[i];
            char buf1[20],buf2[20];
            dSprintf(buf1,sizeof(buf1),"%d",i);
            dSprintf(buf2,sizeof(buf2),"%d",(move->trigger[i]?1:0));
            Con::executef(mDataBlock,4,"onTrigger",scriptThis(),buf1,buf2);
         }
      }
   }
   
   // Update the damage flash and the whiteout
   //
   if (mDamageFlash > 0.0)
   {
      mDamageFlash -= sDamageFlashDec;
      if (mDamageFlash <= 0.0)
         mDamageFlash = 0.0;
   }
   if (mWhiteOut > 0.0)
   {
      mWhiteOut -= sWhiteoutDec;
      if (mWhiteOut <= 0.0)
         mWhiteOut = 0.0;
   }
}

void ShapeBase::advanceTime(F32 dt)
{
   // On the client, the shape threads and images are
   // advanced at framerate.
   advanceThreads(dt);
   updateAudioPos();
   for (int i = 0; i < MaxMountedImages; i++)
      if (mMountedImageList[i].dataBlock)
         updateImageAnimation(i,dt);

   // Cloaking takes 0.5 seconds
   if (mCloaked && mCloakLevel != 1.0) {
      mCloakLevel += dt * 2;
      if (mCloakLevel >= 1.0)
         mCloakLevel = 1.0;
   } else if (!mCloaked && mCloakLevel != 0.0) {
      mCloakLevel -= dt * 2;
      if (mCloakLevel <= 0.0)
         mCloakLevel = 0.0;
   }
   if(mInvincibleOn)
      updateInvincibleEffect(dt);
      
   if(mFading)
   {
      mFadeElapsedTime += dt;
      if(mFadeElapsedTime > mFadeTime)
      {
         mFadeVal = F32(!mFadeOut);
         mFading = false;
      }
      else
      {
         mFadeVal = mFadeElapsedTime / mFadeTime;
         if(mFadeOut)
            mFadeVal = 1 - mFadeVal;
      }
   }
   mShieldEffect.update( dt );
}


void ShapeBase::thinkAboutLocking()
{
   AssertFatal(isServerObject(), "Error, must not call this on the client!");
   AssertFatal(getMountedImage(0) && getMountedImage(0)->isSeeker, "Error, no image, or a non-seeker image!");

   // For right now, we're just going to check every tick.  This should
   //  be slowed down to every third or fourth tick.

   Point3F muzzleVector;
   Point3F muzzlePoint;
   getMuzzleVector(0, &muzzleVector);
   getMuzzlePoint(0, &muzzlePoint);

   Point3F coord0, coord1;
   if (mFabs(muzzleVector.z) < 0.9) {
      mCross(muzzleVector, Point3F(0, 0, 1), &coord0);
   } else {
      mCross(muzzleVector, Point3F(0, 1, 0), &coord0);
   }
   coord0.normalize();
   mCross(muzzleVector, coord0, &coord1);
   coord1.normalize();

   F32 seekAngle = getMountedImage(0)->maxSeekAngle / 2.0;
   F32 sinSeekAngle = mSin(mDegToRad(seekAngle));
   F32 cosSeekAngle = mCos(mDegToRad(seekAngle));

   Point3F end = muzzlePoint + muzzleVector * getMountedImage(0)->seekRadius;
   coord0     *= getMountedImage(0)->seekRadius * sinSeekAngle;
   coord1     *= getMountedImage(0)->seekRadius * sinSeekAngle;

   Box3F queryBox;
   queryBox.min = muzzlePoint;
   queryBox.max = muzzlePoint;
   queryBox.min.setMin(end + coord0 + coord1);
   queryBox.min.setMin(end + coord0 - coord1);
   queryBox.min.setMin(end - coord0 + coord1);
   queryBox.min.setMin(end - coord0 - coord1);
   queryBox.max.setMax(end + coord0 + coord1);
   queryBox.max.setMax(end + coord0 - coord1);
   queryBox.max.setMax(end - coord0 + coord1);
   queryBox.max.setMax(end - coord0 - coord1);

   disableCollision();
   SimpleQueryList sql;
   U32 mask = SensorObjectType | TurretObjectType | PlayerObjectType | VehicleObjectType;
   getContainer()->findObjects(queryBox, mask, SimpleQueryList::insertionCallback, U32(&sql));

   static U32 sTag = 0;
   sTag++;
   for (U32 i = 0; i < sql.mList.size(); i++) {
      AssertFatal(dynamic_cast<ShapeBase*>(sql.mList[i]) != NULL, "Error, should only encounter shapebases in the locker!");
      ShapeBase* pSBase = static_cast<ShapeBase*>(sql.mList[i]);
      
      F32 heatSig = pSBase->getHeat();
      F32 minHeat = getMountedImage(0)->minSeekHeat;
      if(( heatSig < minHeat) || pSBase->isDestroyed() )
         continue;
      
      // require that beacon objects be processed through targetSet   
      if( pSBase->isBeacon() )
         continue;

      // players do not use the visible state of the target (heat only)
      if((!gTargetManager->isTargetVisible(pSBase->getTarget(), getSensorGroup()) && !dynamic_cast<Player*>(this)) ||
         gTargetManager->isTargetFriendly(getTarget(), pSBase->getSensorGroup()))
         continue;

      Point3F centerBox;
      sql.mList[i]->getWorldBox().getCenter(&centerBox);
      
      // must be within the targeting dist
      F32 distance = Point3F(centerBox - muzzlePoint).len();
      if(distance <= getMountedImage(0)->targetingDist)
         continue;
      
      // range = lifetime * maxVelocity
      SeekerProjectileData * seekerData = dynamic_cast<SeekerProjectileData*>(getMountedImage(0)->projectile);
      if(seekerData && (distance > ((F32(seekerData->lifetimeMS) / 1000.f) * seekerData->maxVelocity)))
         continue;
      
      // make sure there is an los to this object...
      pSBase->disableCollision();

      RayInfo rInfo;
      if (getContainer()->castRay(muzzlePoint, centerBox,
                                  (TerrainObjectType | InteriorObjectType |
                                   PlayerObjectType  | VehicleObjectType), &rInfo)) 
      {
         pSBase->enableCollision();
         continue;
      }
      pSBase->enableCollision();
      
      Point3F dif = centerBox - muzzlePoint;
      dif.normalize();

      F32 dot = mDot(dif, muzzleVector);
      if (dot < cosSeekAngle || dynamic_cast<ShapeBase*>(sql.mList[i]) == NULL)
         continue;

      // This one goes on the list if it's not there already...
      bool found = false;
      PotentialLock* walk = mPotentialTargets;
      while (walk && !found) {
         if (bool(walk->potentialTarget)) {
            if (sql.mList[i]->getId() == walk->potentialTarget->getId()) {
               walk->tag = sTag;
               walk->numTicks++;
               found = true;
            }
         }
         walk = walk->next;
      }
      if (!found) {
         PotentialLock* newLock = new PotentialLock;
         newLock->potentialTarget = pSBase;
         newLock->isTarget        = false;
         newLock->tag             = sTag;
         newLock->numTicks        = 1;
         newLock->next            = mPotentialTargets;
         mPotentialTargets        = newLock;
      }
   }

   // grab all the beacon'd objects (probably just 'beacons' and target beams)
   SimSet* pTargetSet = Sim::getServerTargetSet();
   for (SimSet::iterator itr = pTargetSet->begin(); itr != pTargetSet->end(); itr++) {
      AssertFatal(dynamic_cast<GameBase*>(*itr) != NULL, "Error, should only encounter shapebases in the locker!");
      GameBase* pGBase = static_cast<GameBase*>(*itr);

      // only players allowed to fire on beacons/targets
      if(!dynamic_cast<Player*>(this))
         continue;
         
      // dont allow freind beacons to be locked (these are the marker ones)
      if( pGBase->getBeaconType() == GameBase::friendBeacon )
         continue;
         
      // needs to be friendly
      if(!gTargetManager->isTargetVisible(pGBase->getTarget(), getSensorGroup()) ||
         !gTargetManager->isTargetFriendly(getTarget(), pGBase->getSensorGroup()))
         continue;
      
      Point3F target;
      U32 teamId;
      if( pGBase->getTarget( &target, &teamId ) == false )
         continue;

      // must be within the targeting dist
      F32 distance = Point3F(target - muzzlePoint).len();
      if(distance <= getMountedImage(0)->targetingDist)
         continue;

      SeekerProjectileData * seekerData = dynamic_cast<SeekerProjectileData*>(getMountedImage(0)->projectile);
      if(seekerData && (distance > ((F32(seekerData->lifetimeMS) / 1000.f) * seekerData->maxVelocity)))
         continue;

      Point3F dif = target - muzzlePoint;
      dif.normalize();

      F32 dot = mDot(dif, muzzleVector);
      if (dot < cosSeekAngle)
         continue;

      // This one goes on the list if it's not there already...
      bool found = false;
      PotentialLock* walk = mPotentialTargets;
      while (walk && !found) {
         if (bool(walk->potentialTarget)) {
            if ((*itr)->getId() == walk->potentialTarget->getId()) {
               walk->tag = sTag;
               walk->numTicks++;
               found = true;
            }
         }
         walk = walk->next;
      }
      if (!found) {
         PotentialLock* newLock = new PotentialLock;
         newLock->potentialTarget = pGBase;
         newLock->isTarget        = true;
         newLock->tag             = sTag;
         newLock->numTicks        = 1;
         newLock->next            = mPotentialTargets;
         mPotentialTargets        = newLock;
      }
   }

   F32 maxDp       = -2.0;
   ShapeBase* pMin = NULL;
   F32 maxDpGB     = -2.0;
   GameBase* pMinGameBase = NULL;
   Point3F minPosition;

   // Choose best from mPotentialTargets...
   PotentialLock** pWalk = &mPotentialTargets;
   while (*pWalk != NULL) {
      if ((*pWalk)->isTarget == false) {
         F32 heatTime;
         ShapeBase* pSBase = static_cast<ShapeBase*>((GameBase*)(*pWalk)->potentialTarget);

         if ((*pWalk)->tag != sTag || pSBase->getHeat() < getMountedImage(0)->minSeekHeat) 
         {
            // Need to be removed
            PotentialLock* pGarbage = *pWalk;
            *pWalk = pGarbage->next;
            delete pGarbage;
            continue;
         } else if ((*pWalk)->numTicks * TickSec >= (heatTime = (2.0 - pSBase->getHeat()) * getMountedImage(0)->seekTime)) {
            // Consider
            PotentialLock* pConsider = *pWalk;

            Point3F centerBox;
            pSBase->getWorldBox().getCenter(&centerBox);

            pSBase->disableCollision();
            RayInfo rInfo;
            if (getContainer()->castRay(muzzlePoint, centerBox,
                                        (TerrainObjectType | InteriorObjectType |
                                         PlayerObjectType  | VehicleObjectType),
                                        &rInfo)) {
               // Remove pConsider, it's occluded...
               pSBase->enableCollision();
               *pWalk = pConsider->next;
               delete pConsider;
               continue;
            }
            
            pSBase->enableCollision();

            Point3F dif = centerBox - muzzlePoint;
            dif.normalize();

            F32 dot = mDot(dif, muzzleVector);
            if (dot > maxDp) {
               maxDp = dot;
               pMin  = pSBase;
            }
         }
      } else {
         GameBase* pGBase = (*pWalk)->potentialTarget;
         Point3F centerBox;
         U32 teamId;
         if ((*pWalk)->tag != sTag || pGBase->getTarget(&centerBox, &teamId) == false) {
            // Need to be removed
            PotentialLock* pGarbage = *pWalk;
            *pWalk = pGarbage->next;
            delete pGarbage;
            continue;
         } else if ((*pWalk)->numTicks > 8) {
            Point3F dif = centerBox - muzzlePoint;
            dif.normalize();
            F32 dot = mDot(dif, muzzleVector);
            if (dot > maxDp) {
               maxDpGB = dot;
               pMinGameBase  = pGBase;
               minPosition = centerBox;
            }
         }
      }

      pWalk = &((*pWalk)->next);
   }

   enableCollision();

   if (pMin != NULL) 
   {
      if (getLockedTargetId() != pMin->getId())
      {   
         setLockedTarget(pMin);
      }
   }
   else if (pMinGameBase != NULL) 
   {
      setLockedTargetPosition(minPosition);
   }
   else 
   {
      setLockedTarget(NULL);
   }
}

//----------------------------------------------------------------------------

void ShapeBase::setControllingClient(GameConnection* client)
{
   mControllingClient = client;

   // piggybacks on the cloak update
   setMaskBits(CloakMask);
}

void ShapeBase::setControllingObject(ShapeBase* obj)
{
   if (obj) {
      setProcessTick(false);
      // Even though we don't processTick, we still need to
      // process after the controller in case anyone is mounted
      // on this object.
      processAfter(obj);
   }
   else {
      setProcessTick(true);
      clearProcessAfter();
      // Catch the case of the controlling object actually
      // mounted on this object.
      if (mControllingObject->mMount.object == this)
         mControllingObject->processAfter(this);
   }
   mControllingObject = obj;
}

ShapeBase* ShapeBase::getControlObject()
{
   return 0;
}

void ShapeBase::setControlObject(ShapeBase*)
{
}

bool ShapeBase::isFirstPerson()
{
   // Always first person as far as the server is concerned.
   if (!isGhost())
      return true;

   if (GameConnection* con = getControllingClient())
      return con->getControlObject() == this && con->isFirstPerson();
   return false;
}

// Camera: (in degrees) ------------------------------------------------------
F32 ShapeBase::getCameraFov()
{
   return(mCameraFov);
}

F32 ShapeBase::getDefaultCameraFov()
{
   return(mDataBlock->cameraDefaultFov);
}

bool ShapeBase::isValidCameraFov(F32 fov)
{
   return((fov >= mDataBlock->cameraMinFov) && (fov <= mDataBlock->cameraMaxFov));
}

void ShapeBase::setCameraFov(F32 fov)
{
   mCameraFov = mClampF(fov, mDataBlock->cameraMinFov, mDataBlock->cameraMaxFov);
}

//----------------------------------------------------------------------------
static void scopeCallback(SceneObject* obj, S32 conPtr)
{
   NetConnection * ptr = reinterpret_cast<NetConnection*>(conPtr);
   if (obj->isScopeable())
      ptr->objectInScope(obj);
}

void ShapeBase::onCameraScopeQuery(NetConnection *cr, CameraScopeQuery * query)
{
   // update the camera query
   query->camera = this;
   // bool grabEye = true;
   if(GameConnection * con = dynamic_cast<GameConnection*>(cr))
   {
      // get the fov from the connection (in deg)
      F32 fov;
      if (con->getControlCameraFov(&fov))
      {
         query->fov = mDegToRad(fov/2);
         query->sinFov = mSin(query->fov);
         query->cosFov = mCos(query->fov);
      }
   }

   // failed to query the camera info?
   // if(grabEye)    LH - always use eye as good enough, avoid camera animate
   {
      MatrixF eyeTransform;
      getEyeTransform(&eyeTransform);
      eyeTransform.getColumn(3, &query->pos);
      eyeTransform.getColumn(1, &query->orientation);
   }

   // grab the visible distance from the sky
   Sky * sky = gClientSceneGraph->getCurrentSky();
   if(sky)
      query->visibleDistance = sky->getVisibleDistance();
   else
      query->visibleDistance = 1000.f;

   // First, we are certainly in scope, and whatever we're riding is too...
   cr->objectInScope(this);
   if (isMounted())
      cr->objectInScope(mMount.object);

   if (mSceneManager == NULL) {
      // Scope everything...
      gServerContainer.findObjects(-1,scopeCallback,S32(cr));
      return;
   }

   // update the scenemanager
   mSceneManager->scopeScene(query->pos, query->visibleDistance, cr);

   // let the (game)connection do some scoping of its own (commandermap...)
   cr->doneScopingScene();
}


//----------------------------------------------------------------------------
F32 ShapeBase::getEnergyLevel()
{
   if (mDataBlock->inheritEnergyFromMount == false)
      return mEnergy;
   else if (isMounted()) {
      return getObjectMount()->getEnergyLevel();
   } else {
      return 0.0f;
   }
}

F32 ShapeBase::getEnergyValue()
{
   if (mDataBlock->inheritEnergyFromMount == false) {
      F32 maxEnergy = mDataBlock->maxEnergy;
      if ( maxEnergy > 0.f )
         return (mEnergy / mDataBlock->maxEnergy);
   } else if (isMounted()) {
      F32 maxEnergy = getObjectMount()->mDataBlock->maxEnergy;
      if ( maxEnergy > 0.f )
         return (getObjectMount()->getEnergyLevel() / maxEnergy);
   }
   return 0.0f;
}

void ShapeBase::setEnergyLevel(F32 energy)
{
   if (mDataBlock->inheritEnergyFromMount == false) {
      if (mDamageState == Enabled) {
         mEnergy = (energy > mDataBlock->maxEnergy)?
            mDataBlock->maxEnergy: (energy < 0)? 0: energy;
      }
   } else {
      // Pass the set onto whatever we're mounted to...
      if (isMounted())
         getObjectMount()->setEnergyLevel(energy);
   }
}

void ShapeBase::setDamageLevel(F32 damage)
{
   if (!mDataBlock->isInvincible && (damage != mDamage)) 
   {
      mDamage = mClampF(damage, 0.f, mDataBlock->maxDamage);

      updateDamageLevel();
      if (!isGhost()) {
         setMaskBits(DamageMask);
         Con::executef(mDataBlock,2,"onDamage",scriptThis());
      }
   }
}

//----------------------------------------------------------------------------

static F32 sWaterDensity   = 1;
static F32 sWaterViscosity = 15;
static F32 sWaterCoverage  = 0;
static U32 sWaterType      = 0;
static F32 sWaterHeight    = 0.0f;

static void waterFind(SceneObject* obj,S32 key)
{
   ShapeBase* shape = reinterpret_cast<ShapeBase*>(key);
   WaterBlock* wb   = dynamic_cast<WaterBlock*>(obj);
   AssertFatal(wb != NULL, "Error, not a water block!");
   if (wb == NULL) {
      sWaterCoverage = 0;
      return;
   }

   const Box3F& wbox = obj->getWorldBox();
   const Box3F& sbox = shape->getWorldBox();
   sWaterType = 0;
   if (wbox.isOverlapped(sbox)) {
      sWaterType = wb->getLiquidType();
      if (wbox.max.z < sbox.max.z)
         sWaterCoverage = (wbox.max.z - sbox.min.z) / (sbox.max.z - sbox.min.z);
      else
         sWaterCoverage = 1;

      sWaterViscosity = wb->getViscosity();
      sWaterDensity   = wb->getDensity();
      sWaterHeight    = wb->getSurfaceHeight();
   }
}

void physicalZoneFind(SceneObject* obj, S32 key)
{
   ShapeBase* shape = reinterpret_cast<ShapeBase*>(key);
   PhysicalZone* pz = dynamic_cast<PhysicalZone*>(obj);
   AssertFatal(pz != NULL, "Error, not a water block!");
   if (pz == NULL || pz->testObject(shape) == false) {
      return;
   }

   if (pz->isActive()) {
      shape->mGravityMod   *= pz->getGravityMod();
      shape->mAppliedForce += pz->getForce();
   }
}

void findRouter(SceneObject* obj, S32 key)
{
   if (obj->getTypeMask() & WaterObjectType)
      waterFind(obj, key);
   else if (obj->getTypeMask() & PhysicalZoneObjectType)
      physicalZoneFind(obj, key);
   else {
      AssertFatal(false, "Error, must be either water or physical zone here!");
   }
}

void ShapeBase::updateContainer()
{
   // Update container drag and buoyancy properties
   mDrag = 0;
   mBuoyancy = 0;
   sWaterCoverage = 0;
   mGravityMod = 1.0;
   mAppliedForce.set(0, 0, 0);
   mContainer->findObjects(getWorldBox(), WaterObjectType|PhysicalZoneObjectType,findRouter,S32(this));
   sWaterCoverage = mClampF(sWaterCoverage,0,1);
   mWaterCoverage = sWaterCoverage;
   mLiquidType    = sWaterType;
   mLiquidHeight  = sWaterHeight;
   if (mWaterCoverage >= 0.1f) {
      mDrag = mDataBlock->drag * sWaterViscosity * sWaterCoverage;
      mBuoyancy = (sWaterDensity / mDataBlock->density) * sWaterCoverage;
   }
}


//----------------------------------------------------------------------------

void ShapeBase::applyRepair(F32 amount)
{
   // Repair increases the repair reserve
   if (amount > 0 && ((mRepairReserve += amount) > mDamage))
      mRepairReserve = mDamage;
}

void ShapeBase::applyDamage(F32 amount)
{
   if (amount > 0)
      setDamageLevel(mDamage + amount);
}

F32 ShapeBase::getDamageValue()
{
   // Return a 0-1 damage value.
   return mDamage / mDataBlock->maxDamage;
}

void ShapeBase::updateDamageLevel()
{
   if (mDamageThread) {
      // mDamage is already 0-1 on the client
      if (mDamage >= mDataBlock->destroyedLevel) {
         if (getDamageState() == Destroyed)
            mShapeInstance->setPos(mDamageThread, 0);
         else
            mShapeInstance->setPos(mDamageThread, 1);
      } else {
         mShapeInstance->setPos(mDamageThread, mDamage / mDataBlock->destroyedLevel);
      }
   }
}


//----------------------------------------------------------------------------

void ShapeBase::setDamageState(DamageState state)
{
   if (mDamageState == state)
      return;

   const char* script = 0;
   const char* lastState = 0;

   if (!isGhost()) {
      if (state != getDamageState())
         setMaskBits(DamageMask);

      lastState = getDamageStateName();
      switch (state) {
         case Destroyed: {
            if (mDamageState == Enabled)
               setDamageState(Disabled);
            script = "onDestroyed";
            break;
         }
         case Disabled:
            if (mDamageState == Enabled)
               script = "onDisabled";
            break;
         case Enabled:
            script = "onEnabled";
            break;
      }
   }

   mDamageState = state;
   if (mDamageState != Enabled) {
      mRepairReserve = 0;
      mEnergy = 0;
   }
   if (script) {
      // Like to call the scripts after the state has been intialize.
      // This should only end up being called on the server.
      Con::executef(mDataBlock,3,script,scriptThis(),lastState);
   }
   updateDamageState();
   updateDamageLevel();
}

bool ShapeBase::setDamageState(const char* state)
{
   for (S32 i = 0; i < NumDamageStates; i++)
      if (!dStricmp(state,sDamageStateName[i])) {
         setDamageState(DamageState(i));
         return true;
      }
   return false;
}

const char* ShapeBase::getDamageStateName()
{
   return sDamageStateName[mDamageState];
}

void ShapeBase::updateDamageState()
{
   if (mHulkThread) {
      F32 pos = (mDamageState == Destroyed)? 1: 0;
      if (mShapeInstance->getPos(mHulkThread) != pos) {
         mShapeInstance->setPos(mHulkThread,pos);

         if (isClientObject())
            mShapeInstance->animate();
      }
   }
}


//----------------------------------------------------------------------------

void ShapeBase::blowUp()
{
   Point3F center;
   mObjBox.getCenter(&center);
   center += getPosition();
   MatrixF trans = getTransform();
   trans.setPosition( center );
   
   // explode
   Explosion* pExplosion = NULL;

   if( pointInWater( (Point3F &)center ) && mDataBlock->underwaterExplosion )
   {
      pExplosion = new Explosion;
      pExplosion->onNewDataBlock(mDataBlock->underwaterExplosion);
   }
   else
   {
      if (mDataBlock->explosion)
      {
         pExplosion = new Explosion;
         pExplosion->onNewDataBlock(mDataBlock->explosion);
      }
   }

   if( pExplosion )
   {
      pExplosion->setTransform(trans);
      pExplosion->setInitialState(center, damageDir);
      if (pExplosion->registerObject() == false)
      {
         Con::errorf(ConsoleLogEntry::General, "ShapeBase(%s)::explode: couldn't register explosion",
                     mDataBlock->getName() );
         delete pExplosion;
         pExplosion = NULL;
      }
   }

   TSShapeInstance *debShape = NULL;

   if( !mDataBlock->debrisShape )
   {
      return;
   }
   else
   {
      debShape = new TSShapeInstance( mDataBlock->debrisShape, true);
   }


   Vector< TSPartInstance * > partList;
   TSPartInstance::breakShape( debShape, 0, partList, NULL, NULL, 0 );

   if( !mDataBlock->debris )
   {
      mDataBlock->debris = new DebrisData;
   }

   // cycle through partlist and create debris pieces
   for( U32 i=0; i<partList.size(); i++ )
   {
      //Point3F axis( 0.0, 0.0, 1.0 );
      Point3F randomDir = MathUtils::randomDir( damageDir, 0, 50 );

      Debris *debris = new Debris;
      debris->setPartInstance( partList[i] );
      debris->init( center, randomDir );
      debris->onNewDataBlock( mDataBlock->debris );
      debris->setTransform( trans );
      
      if( !debris->registerObject() )
      {
         Con::warnf( ConsoleLogEntry::General, "Could not register debris for class: %s", mDataBlock->getName() );
         delete debris;
         debris = NULL;
      }
      else
      {
         debShape->incDebrisRefCount();
      }
   }
   
   damageDir.set(0, 0, 1);
}   


//----------------------------------------------------------------------------
void ShapeBase::mountObject(ShapeBase* obj,U32 node)
{
//   if (obj->mMount.object == this)
//      return;
   if (obj->mMount.object)
      obj->unmount();

   // Since the object is mounting to us, nothing should be colliding with it for a while
   obj->mConvexList->nukeList();

   obj->mMount.object = this;
   obj->mMount.node = (node >= 0 && node < ShapeBaseData::NumMountPoints)? node: 0;
   obj->mMount.link = mMount.list;
   mMount.list = obj;
   if (obj != getControllingObject())
      obj->processAfter(this);
   obj->deleteNotify(this);
   obj->setMaskBits(MountedMask);
   obj->onMount(this,node);
}


void ShapeBase::unmountObject(ShapeBase* obj)
{
   if (obj->mMount.object == this) {

      // Find and unlink the object
      for(ShapeBase **ptr = & mMount.list; (*ptr); ptr = &((*ptr)->mMount.link) )
      {
         if(*ptr == obj)
         {
            *ptr = obj->mMount.link;
            break;
         }
      }
      if (obj != getControllingObject())
         obj->clearProcessAfter();
      obj->clearNotify(this);
      obj->mMount.object = 0;
      obj->mMount.link = 0;
      obj->setMaskBits(MountedMask);
      obj->onUnmount(this,obj->mMount.node);
   }
}

void ShapeBase::unmount()
{
   if (mMount.object)
      mMount.object->unmountObject(this);
}

void ShapeBase::onMount(ShapeBase* obj,S32 node)
{
   if (!isGhost()) {
      char buff1[32];
      dSprintf(buff1,sizeof(buff1),"%d",node);
      Con::executef(mDataBlock,4,"onMount",scriptThis(),obj->scriptThis(),buff1);
   }
}   

void ShapeBase::onUnmount(ShapeBase* obj,S32 node)
{
   if (!isGhost()) {
      char buff1[32];
      dSprintf(buff1,sizeof(buff1),"%d",node);
      Con::executef(mDataBlock,4,"onUnmount",scriptThis(),obj->scriptThis(),buff1);
   }
}

S32 ShapeBase::getMountedObjectCount()
{
   S32 count = 0;
   for (ShapeBase* itr = mMount.list; itr; itr = itr->mMount.link)
      count++;
   return count;
}

ShapeBase* ShapeBase::getMountedObject(S32 idx)
{
   if (idx >= 0) {
      S32 count = 0;
      for (ShapeBase* itr = mMount.list; itr; itr = itr->mMount.link)
         if (count++ == idx)
            return itr;
   }
   return 0;
}

S32 ShapeBase::getMountedObjectNode(S32 idx)
{
   if (idx >= 0) {
      S32 count = 0;
      for (ShapeBase* itr = mMount.list; itr; itr = itr->mMount.link)
         if (count++ == idx)
            return itr->mMount.node;
   }
   return -1;
}

ShapeBase* ShapeBase::getMountNodeObject(S32 node)
{
   for (ShapeBase* itr = mMount.list; itr; itr = itr->mMount.link)
      if (itr->mMount.node == node)
         return itr;
   return 0;
}

Point3F ShapeBase::getAIRepairPoint()
{
   if (mDataBlock->mountPointNode[ShapeBaseData::AIRepairNode] < 0)
		return Point3F(0, 0, 0);
   MatrixF xf(true);
   getMountTransform(ShapeBaseData::AIRepairNode,&xf);
   Point3F pos(0, 0, 0);
   xf.getColumn(3,&pos);
   return pos;
}

//----------------------------------------------------------------------------

void ShapeBase::getEyeTransform(MatrixF* mat)
{  
   // Returns eye to world space transform
   S32 eyeNode = mDataBlock->eyeNode;
   if (eyeNode != -1)
      mat->mul(getTransform(), mShapeInstance->mNodeTransforms[eyeNode]);
   else
      *mat = getTransform();
}

void ShapeBase::getRenderEyeTransform(MatrixF* mat)
{  
   // Returns eye to world space transform
   S32 eyeNode = mDataBlock->eyeNode;
   if (eyeNode != -1)
      mat->mul(getRenderTransform(), mShapeInstance->mNodeTransforms[eyeNode]);
   else
      *mat = getRenderTransform();
}

void ShapeBase::getCameraTransform(F32* pos,MatrixF* mat)
{
   // Returns camera to world space transform
   // Handles first person / third person camera position

   if (isServerObject() && mShapeInstance)
      mShapeInstance->animateNodeSubtrees(true);

   if (*pos != 0)
   {
      F32 min,max;
      Point3F offset;
      MatrixF eye,rot;
      getCameraParameters(&min,&max,&offset,&rot);
      getRenderEyeTransform(&eye);
      mat->mul(eye,rot);

      // Use the eye transform to orient the camera
      VectorF vp,vec;
      vp.x = vp.z = 0;
      vp.y = -(max - min) * *pos;
      eye.mulV(vp,&vec);

      // Use the camera node's pos.
      Point3F osp,sp;
      if (mDataBlock->cameraNode != -1) {
         mShapeInstance->mNodeTransforms[mDataBlock->cameraNode].getColumn(3,&osp);
         getRenderTransform().mulP(osp,&sp);
      }
      else
         getRenderTransform().getColumn(3,&sp);

      // Make sure we don't extend the camera into anything solid
      Point3F ep = sp + vec + offset;
      disableCollision();
      if (isMounted())
         getObjectMount()->disableCollision();
      RayInfo collision;
      if (mContainer->castRay(sp, ep,
                              (0xFFFFFFFF & ~(WaterObjectType      |
                                              ForceFieldObjectType |
                                              GameBaseObjectType   |
                                              DefaultObjectType)),
                              &collision) == true) {
         F32 veclen = vec.len();
         F32 adj = (-mDot(vec, collision.normal) / veclen) * 0.1;
         F32 newPos = getMax(0.0f, collision.t - adj);
         if (newPos == 0.0f)
            eye.getColumn(3,&ep);
         else
            ep = sp + offset + (vec * newPos);
      }
      mat->setColumn(3,ep);
      if (isMounted())
         getObjectMount()->enableCollision();
      enableCollision();
   }
   else
   {
      getRenderEyeTransform(mat);
   }
}

// void ShapeBase::getCameraTransform(F32* pos,MatrixF* mat)
// {
//    // Returns camera to world space transform
//    // Handles first person / third person camera position

//    if (isServerObject() && mShapeInstance)
//       mShapeInstance->animateNodeSubtrees(true);

//    if (*pos != 0) {
//       F32 min,max;
//       Point3F offset;
//       MatrixF eye,rot;
//       getCameraParameters(&min,&max,&offset,&rot);
//       getRenderEyeTransform(&eye);
//       mat->mul(eye,rot);

//       // Use the eye transform to orient the camera
//       VectorF vp,vec;
//       vp.x = vp.z = 0;
//       vp.y = -(max - min) * *pos;
//       eye.mulV(vp,&vec);

//       // Use the camera node's pos.
//       Point3F osp,sp;
//       if (mDataBlock->cameraNode != -1) {
//          mShapeInstance->mNodeTransforms[mDataBlock->cameraNode].getColumn(3,&osp);
//          getRenderTransform().mulP(osp,&sp);
//       }
//       else
//          getRenderTransform().getColumn(3,&sp);

//       // Make sure we don't extend the camera into anything solid
//       Point3F ep = sp + vec;
//       ep += offset;
//       disableCollision();
//       if (isMounted())
//          getObjectMount()->disableCollision();
//       RayInfo collision;
//       if (mContainer->castRay(sp,ep,(0xFFFFFFFF & ~(WaterObjectType|ForceFieldObjectType|GameBaseObjectType|DefaultObjectType)),&collision)) {
//          *pos = collision.t *= 0.9;
//          if (*pos == 0)
//             eye.getColumn(3,&ep);
//          else
//             ep = sp + vec * *pos;
//       }
//       mat->setColumn(3,ep);
//       if (isMounted())
//          getObjectMount()->enableCollision();
//       enableCollision();
//    }
//    else
//    {
//       getRenderEyeTransform(mat);
//    }
// }


// void ShapeBase::getRenderCameraTransform(F32* pos,MatrixF* mat)
// {
//    // Returns camera to world space transform
//    // Handles first person / third person camera position

//    if (isServerObject() && mShapeInstance)
//       mShapeInstance->animateNodeSubtrees(true);

//    if (*pos != 0) {
//       F32 min,max;
//       Point3F offset;
//       MatrixF eye,rot;
//       getCameraParameters(&min,&max,&offset,&rot);
//       getRenderEyeTransform(&eye);
//       mat->mul(eye,rot);

//       // Use the eye transform to orient the camera
//       VectorF vp,vec;
//       vp.x = vp.z = 0;
//       vp.y = -(max - min) * *pos;
//       eye.mulV(vp,&vec);

//       // Use the camera node's pos.
//       Point3F osp,sp;
//       if (mDataBlock->cameraNode != -1) {
//          mShapeInstance->mNodeTransforms[mDataBlock->cameraNode].getColumn(3,&osp);
//          getRenderTransform().mulP(osp,&sp);
//       }
//       else
//          getRenderTransform().getColumn(3,&sp);

//       // Make sure we don't extend the camera into anything solid
//       Point3F ep = sp + vec;
//       ep += offset;
//       disableCollision();
//       if (isMounted())
//          getObjectMount()->disableCollision();
//       RayInfo collision;
//       if (mContainer->castRay(sp,ep,(0xFFFFFFFF & ~(WaterObjectType|ForceFieldObjectType|GameBaseObjectType|DefaultObjectType)),&collision)) {
//          *pos = collision.t *= 0.9;
//          if (*pos == 0)
//             eye.getColumn(3,&ep);
//          else
//             ep = sp + vec * *pos;
//       }
//       mat->setColumn(3,ep);
//       if (isMounted())
//          getObjectMount()->enableCollision();
//       enableCollision();
//    }
//    else
//    {
//       getRenderEyeTransform(mat);
//    }
// }

void ShapeBase::getCameraParameters(F32 *min,F32* max,Point3F* off,MatrixF* rot)
{
   *min = mDataBlock->cameraMinDist;
   *max = mDataBlock->cameraMaxDist;
   off->set(0,0,0);
   rot->identity();
}


//----------------------------------------------------------------------------
F32 ShapeBase::getDamageFlash() const
{
   return mDamageFlash;
}

void ShapeBase::setDamageFlash(const F32 flash)
{
   mDamageFlash = flash;
   if (mDamageFlash < 0.0)
      mDamageFlash = 0;
   else if (mDamageFlash > 1.0)
      mDamageFlash = 1.0;
}


//----------------------------------------------------------------------------
F32 ShapeBase::getWhiteOut() const
{
   return mWhiteOut;
}

void ShapeBase::setWhiteOut(const F32 flash)
{
   mWhiteOut = flash;
   if (mWhiteOut < 0.0)
      mWhiteOut = 0;
   else if (mWhiteOut > 1.5)
      mWhiteOut = 1.5;
}


//----------------------------------------------------------------------------
void ShapeBase::playShieldEffect(const Point3F& normal, F32 strength )
{
   mShieldNormal = normal;
   if (isServerObject())
   {
      setMaskBits(ShieldMask);
   }
   else
   {
      mShieldEffect.setLifetime( mDataBlock->shieldEffectLifetimeMS / 1000.0 );
      mShieldEffect.setShieldStrength( strength );
      mShieldEffect.setShieldedObj( this );
      mShieldEffect.start();
   }
}


//----------------------------------------------------------------------------
F32 ShapeBase::getHeat() const
{
   return mHeat;
}

bool ShapeBase::onlyFirstPerson() const
{
   return mDataBlock->firstPersonOnly;   
}

bool ShapeBase::useObjsEyePoint() const
{
   return mDataBlock->useEyePoint;   
}

void ShapeBase::setHeat(const F32 heat)
{
   mHeat = heat;
   if (mHeat < 0.0)
     mHeat = 0;
   else if (mHeat > 1.0)
      mHeat = 1.0;
}


//----------------------------------------------------------------------------
F32 ShapeBase::getInvincibleEffect() const
{
   return mInvincibleEffect;
}

void ShapeBase::setupInvincibleEffect(F32 time, F32 speed)
{
   if(isClientObject())
   {
      mInvincibleCount = mInvincibleTime = time;
      mInvincibleSpeed = mInvincibleDelta = speed;
      mInvincibleEffect = 0.0f;
      mInvincibleOn = true;
      mInvincibleFade = 1.0f;
   }
   else
   { 
      mInvincibleTime  = time;   
      mInvincibleSpeed = speed; 
      setMaskBits(InvincibleMask);
   }
}

void ShapeBase::updateInvincibleEffect(F32 dt)
{
   if(mInvincibleCount > 0.0f )
   {
      if(mInvincibleEffect >= ((0.3 * mInvincibleFade) + 0.05f) && mInvincibleDelta > 0.0f)
         mInvincibleDelta = -mInvincibleSpeed;         
      else if(mInvincibleEffect <= 0.05f && mInvincibleDelta < 0.0f)
      {
         mInvincibleDelta = mInvincibleSpeed;         
         mInvincibleFade = mInvincibleCount / mInvincibleTime;
      }
      mInvincibleEffect += mInvincibleDelta;
      mInvincibleCount -= dt;
   }
   else  
   {
      mInvincibleEffect = 0.0f;
      mInvincibleOn = false;
   }
}

//----------------------------------------------------------------------------
void ShapeBase::setVelocity(const VectorF&)
{
}

void ShapeBase::applyImpulse(const Point3F&,const VectorF&)
{
}


//----------------------------------------------------------------------------

void ShapeBase::playAudio(U32 slot,AudioProfile* profile)
{
   AssertFatal(slot < MaxSoundThreads,"ShapeBase::playSound: Incorrect argument");
   Sound& st = mSoundThread[slot];
   if (profile && (!st.play || st.profile != profile)) {
      setMaskBits(SoundMaskN << slot);
      st.play = true;
      st.profile = profile;
      updateAudioState(st);
   }
}

void ShapeBase::stopAudio(U32 slot)
{
   AssertFatal(slot < MaxSoundThreads,"ShapeBase::stopSound: Incorrect argument");
   Sound& st = mSoundThread[slot];
   if (st.play) {
      st.play = false;
      setMaskBits(SoundMaskN << slot);
      updateAudioState(st);
   }
}

void ShapeBase::updateServerAudio()
{
   // Timeout non-looping sounds
   for (int i = 0; i < MaxSoundThreads; i++) {
      Sound& st = mSoundThread[i];
      if (st.play && st.timeout && st.timeout < Sim::getCurrentTime()) {
         clearMaskBits(SoundMaskN << i);
         st.play = false;
      }
   }
}

void ShapeBase::updateAudioState(Sound& st)
{
   if (st.sound) {
      alxStop(st.sound);
      st.sound = 0;
   }
   if (st.play && st.profile) {
      if (isGhost()) {
         if (Sim::findObject(SimObjectId(st.profile), st.profile)) 
            st.sound = alxPlay(st.profile, &getTransform());
         else
            st.play = false;
      }
      else {
         // Non-looping sounds timeout on the server
         st.timeout = st.profile->mDescriptionObject->mDescription.mIsLooping? 0:
            Sim::getCurrentTime() + sAudioTimeout;
      }
   }
   else
      st.play = false;
}

void ShapeBase::updateAudioPos()
{
   for (int i = 0; i < MaxSoundThreads; i++)
      if (AUDIOHANDLE sh = mSoundThread[i].sound) 
         alxSourceMatrixF(sh, &getTransform());
}

//----------------------------------------------------------------------------

bool ShapeBase::setThreadSequence(U32 slot,S32 seq,bool reset)
{
   Thread& st = mScriptThread[slot];
   if (st.thread && st.sequence == seq && st.state == Thread::Play)
      return true;

   if (seq < MaxSequenceIndex) {
      setMaskBits(ThreadMaskN << slot);
      st.sequence = seq;
      if (reset) {
         st.state = Thread::Play;
         st.atEnd = false;
         st.forward = true;
      }
      if (mShapeInstance) {
         if (!st.thread)
            st.thread = mShapeInstance->addThread();
         mShapeInstance->setSequence(st.thread,seq,0);
         stopThreadSound(st);
         updateThread(st);
      }
      return true;
   }
   return false;
}

void ShapeBase::updateThread(Thread& st)
{
   switch (st.state) {
      case Thread::Stop:
         mShapeInstance->setTimeScale(st.thread,1);
         mShapeInstance->setPos(st.thread,0);
         // Drop through to pause state
      case Thread::Pause:
         mShapeInstance->setTimeScale(st.thread,0);
         stopThreadSound(st);
         break;
      case Thread::Play:
         if (st.atEnd) {
            mShapeInstance->setTimeScale(st.thread,1);
            mShapeInstance->setPos(st.thread,st.forward? 1: 0);
            mShapeInstance->setTimeScale(st.thread,0);
            stopThreadSound(st);
         }
         else {
            mShapeInstance->setTimeScale(st.thread,st.forward? 1: -1);
            if (!st.sound)
               startSequenceSound(st);
         }
         break;
   }
}  

bool ShapeBase::stopThread(U32 slot)
{
   Thread& st = mScriptThread[slot];
   if (st.sequence != -1 && st.state != Thread::Stop) {
      setMaskBits(ThreadMaskN << slot);
      st.state = Thread::Stop;
      updateThread(st);
      return true;
   }
   return false;
}

bool ShapeBase::pauseThread(U32 slot)
{
   Thread& st = mScriptThread[slot];
   if (st.sequence != -1 && st.state != Thread::Pause) {
      setMaskBits(ThreadMaskN << slot);
      st.state = Thread::Pause;
      updateThread(st);
      return true;
   }
   return false;
}

bool ShapeBase::playThread(U32 slot)
{
   Thread& st = mScriptThread[slot];
   if (st.sequence != -1 && st.state != Thread::Play) {
      setMaskBits(ThreadMaskN << slot);
      st.state = Thread::Play;
      updateThread(st);
      return true;
   }
   return false;
}

bool ShapeBase::setThreadDir(U32 slot,bool forward)
{
   Thread& st = mScriptThread[slot];
   if (st.sequence != -1) {
      if (st.forward != forward) {
         setMaskBits(ThreadMaskN << slot);
         st.forward = forward;
         st.atEnd = false;
         updateThread(st);
      }
      return true;
   }
   return false;
}  

void ShapeBase::stopThreadSound(Thread& thread)
{
   if (thread.sound) {
   }
}

void ShapeBase::startSequenceSound(Thread& thread)
{
   if (!isGhost() || !thread.thread)
      return;
   stopThreadSound(thread);
}

void ShapeBase::advanceThreads(F32 dt)
{
   for (U32 i = 0; i < MaxScriptThreads; i++) {
      Thread& st = mScriptThread[i];
      if (st.thread) {
         if (!mShapeInstance->getShape()->sequences[st.sequence].isCyclic() && !st.atEnd &&
             (st.forward? mShapeInstance->getPos(st.thread) >= 1.0:
              mShapeInstance->getPos(st.thread) <= 0)) {
            st.atEnd = true;
            updateThread(st);
            if (!isGhost()) {
               char slot[16];
               dSprintf(slot,sizeof(slot),"%d",i);
               Con::executef(mDataBlock,3,"onEndSequence",scriptThis(),slot);
            }
         }
         mShapeInstance->advanceTime(dt,st.thread);
      }
   }
}


//----------------------------------------------------------------------------

TSShape const* ShapeBase::getShape()
{
   return mShapeInstance? mShapeInstance->getShape(): 0;
}


void ShapeBase::calcClassRenderData()
{
   // This is truly lame, but I didn't want to duplicate the whole preprender logic
   //  in the player as well as the renderImage logic.  DMM
}


bool ShapeBase::prepRenderImage(SceneState* state, const U32 stateKey,
                                const U32 startZone, const bool modifyBaseState)
{
   AssertFatal(modifyBaseState == false, "Error, should never be called with this parameter set");
   AssertFatal(startZone == 0xFFFFFFFF, "Error, startZone should indicate -1");

   if (isLastState(state, stateKey))
      return false;
   setLastState(state, stateKey);
   
   if(blowApart)
      return false;

   if( ( getDamageState() == Destroyed ) && ( !mDataBlock->renderWhenDestroyed ) )
      return false;
   
   // Select detail levels on mounted items
   // but... always draw the control object's mounted images
   // in high detail (I can't believe I'm commenting this hack :)
   F32 saveError = TSShapeInstance::smScreenError;
   GameConnection *con = GameConnection::getServerConnection();
   bool fogExemption = false;
   ShapeBase *co = NULL;
   if(con && ( (co = con->getControlObject()) != NULL) )
   {
      if(co == this || co->getObjectMount() == this) 
      {
         TSShapeInstance::smScreenError = 0.001;
         fogExemption = true;
      }
   }

   if (state->isObjectRendered(this))
   {
      mLastRenderFrame = sLastRenderFrame;
      // get shape detail and fog information...we might not even need to be drawn
      Point3F cameraOffset;
      getRenderTransform().getColumn(3,&cameraOffset);
      cameraOffset -= state->getCameraPosition();
      F32 dist = cameraOffset.len();
      if (dist < 0.01)
         dist = 0.01;
      F32 fogAmount = state->getHazeAndFog(dist,cameraOffset.z);
      F32 invScale = (1.0f/getMax(getMax(mObjScale.x,mObjScale.y),mObjScale.z));
      if (mShapeInstance)
         DetailManager::selectPotentialDetails(mShapeInstance,dist,invScale);
      
      if ((fogAmount>0.99f && fogExemption == false) ||
          (mShapeInstance && mShapeInstance->getCurrentDetail()<0) ||
          (!mShapeInstance && !gShowBoundingBox)) {
         // no, don't draw anything
         return false;
      }


      for (U32 i = 0; i < MaxMountedImages; i++)
      {
         MountedImage& image = mMountedImageList[i];
         if (image.dataBlock && image.shapeInstance)
         {
            DetailManager::selectPotentialDetails(image.shapeInstance,dist,invScale);

            if (mCloakLevel == 0.0f && image.shapeInstance->hasSolid() && mFadeVal == 1.0f)
            {
               ShapeImageRenderImage* rimage = new ShapeImageRenderImage;
               rimage->obj = this;
               rimage->mSBase = this;
               rimage->mIndex = i;
               rimage->isTranslucent = false;
               rimage->textureSortKey = U32(image.dataBlock);
               state->insertRenderImage(rimage);
            }
      
            if ((mCloakLevel != 0.0f || mFadeVal != 1.0f || mShapeInstance->hasTranslucency()) ||
                (mMount.object == NULL && mGenerateShadow == true))
            {
               ShapeImageRenderImage* rimage = new ShapeImageRenderImage;
               rimage->obj = this;
               rimage->mSBase = this;
               rimage->mIndex = i;
               rimage->isTranslucent = true;
               rimage->sortType = SceneRenderImage::Point;
               rimage->textureSortKey = U32(image.dataBlock);
               state->setImageRefPoint(this, rimage);
               state->insertRenderImage(rimage);
            }
         }
      }
      TSShapeInstance::smScreenError = saveError;

      if (mCloakLevel == 0.0f && mShapeInstance->hasSolid() && mFadeVal == 1.0f)
      {
         SceneRenderImage* image = new SceneRenderImage;
         image->obj = this;
         image->isTranslucent = false;
         image->textureSortKey = mSkinHash ^ U32(mDataBlock);
         state->insertRenderImage(image);
      }
      
      if ((mCloakLevel != 0.0f || mFadeVal != 1.0f || mShapeInstance->hasTranslucency()) ||
          (mMount.object == NULL && mGenerateShadow == true))
      {
         SceneRenderImage* image = new SceneRenderImage;
         image->obj = this;
         image->isTranslucent = true;
         image->sortType = SceneRenderImage::Point;
         image->textureSortKey = mSkinHash ^ U32(mDataBlock);
         state->setImageRefPoint(this, image);
         state->insertRenderImage(image);
      }

      if( mShieldEffect.isActive() )
      {
         ShieldImpactRenderImage* image = new ShieldImpactRenderImage;
         image->setup(this);
         state->setImageRefPoint(this, image);
         state->insertRenderImage(image);
      }

      calcClassRenderData();
   }

   return false;
}


void ShapeBase::renderObject(SceneState* state, SceneRenderImage* image)
{
   PROFILE_START(ShapeBaseRenderObject);
   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on entry");

   RectI viewport;
   dglGetViewport(&viewport);

   installLights();

   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   state->setupObjectProjection(this);

   // This is something of a hack, but since the 3space objects don't have a
   //  clear conception of texels/meter like the interiors do, we're sorta
   //  stuck.  I can't even claim this is anything more scientific than eyeball
   //  work.  DMM
   F32 axis = (getObjBox().len_x() + getObjBox().len_y() + getObjBox().len_z()) / 3.0;
   F32 dist = (getRenderWorldBox().getClosestPoint(state->getCameraPosition()) - state->getCameraPosition()).len();
   if (dist != 0)
   {
      F32 projected = dglProjectRadius(dist, axis) / 350;
      if (projected < (1.0 / 16.0))
      {
         TextureManager::setSmallTexturesActive(true);
      }
   }
   
   // render shield effect
   ShieldImpactRenderImage *siri = dynamic_cast< ShieldImpactRenderImage * > ( image );
   if( siri )
   {
      mShieldEffect.renderObject( state, image );
   }
   else
   {   
      if (mCloakLevel == 0.0f && mFadeVal == 1.0f)
      {
         if (image->isTranslucent == true)
         {
            TSShapeInstance::smNoRenderNonTranslucent = true;
            TSShapeInstance::smNoRenderTranslucent    = false;
         }
         else
         {
            TSShapeInstance::smNoRenderNonTranslucent = false;
            TSShapeInstance::smNoRenderTranslucent    = true;
         }
      }
      else
      {
         TSShapeInstance::smNoRenderNonTranslucent = false;
         TSShapeInstance::smNoRenderTranslucent    = false;
      }

      TSMesh::setOverrideFade( mFadeVal );

      ShapeImageRenderImage* shiri = dynamic_cast<ShapeImageRenderImage*>(image);
      if (shiri != NULL)
      {
         renderMountedImage(state, shiri);
      }
      else
      {
         renderImage(state, image);
      }

      TSMesh::setOverrideFade( 1.0 );


      TSShapeInstance::smNoRenderNonTranslucent = false;
      TSShapeInstance::smNoRenderTranslucent    = false;
   }
   
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   dglSetViewport(viewport);

   uninstallLights();

   // Debugging Bounding Box
   if (!mShapeInstance || gShowBoundingBox) {
      glDisable(GL_DEPTH_TEST);
      Point3F box;
      glPushMatrix();
      dglMultMatrix(&getRenderTransform());
      box = (mObjBox.min + mObjBox.max) * 0.5;
      glTranslatef(box.x,box.y,box.z);
      box = (mObjBox.max - mObjBox.min) * 0.5;
      glScalef(box.x,box.y,box.z);
      glColor3f(1, 0, 1);
      wireCube(Point3F(1,1,1),Point3F(0,0,0));
      glPopMatrix();

      glPushMatrix();
      box = (mWorldBox.min + mWorldBox.max) * 0.5;
      glTranslatef(box.x,box.y,box.z);
      box = (mWorldBox.max - mWorldBox.min) * 0.5;                               
      glScalef(box.x,box.y,box.z);
      glColor3f(0, 1, 1);
      wireCube(Point3F(1,1,1),Point3F(0,0,0));
      glPopMatrix();

      for (U32 i = 0; i < MaxMountedImages; i++) {
         MountedImage& image = mMountedImageList[i];
         if (image.dataBlock && image.shapeInstance) {
            MatrixF mat;
            glPushMatrix();
            getRenderImageTransform(i,&mat);
            dglMultMatrix(&mat);
            glColor3f(1, 0, 1);
            wireCube(Point3F(0.05,0.05,0.05),Point3F(0,0,0));
            glPopMatrix();

            glPushMatrix();
            getRenderMountTransform(i,&mat);
            dglMultMatrix(&mat);
            glColor3f(1, 0, 1);
            wireCube(Point3F(0.05,0.05,0.05),Point3F(0,0,0));
            glPopMatrix();

            glPushMatrix();
            getRenderMuzzleTransform(i,&mat);
            dglMultMatrix(&mat);
            glColor3f(1, 0, 1);
            wireCube(Point3F(0.05,0.05,0.05),Point3F(0,0,0));
            glPopMatrix();
         }
      }
      glEnable(GL_DEPTH_TEST);
   }

   dglSetCanonicalState();
   TextureManager::setSmallTexturesActive(false);
   
   AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on exit");
   PROFILE_END();
}


void ShapeBase::renderMountedImage(SceneState* state, ShapeImageRenderImage* rimage)
{
   AssertFatal(rimage->mSBase == this, "Error, wrong image");

   Point3F cameraOffset;
   getRenderTransform().getColumn(3,&cameraOffset);
   cameraOffset -= state->getCameraPosition();
   F32 dist = cameraOffset.len();
   F32 fogAmount = state->getHazeAndFog(dist,cameraOffset.z);

   // Mounted items
   PROFILE_START(ShapeBaseRenderMounted);
   MountedImage& image = mMountedImageList[rimage->mIndex];
   if (image.dataBlock && image.shapeInstance && DetailManager::selectCurrentDetail(image.shapeInstance)) {
      MatrixF mat;
      getRenderImageTransform(rimage->mIndex, &mat);
      glPushMatrix();
      dglMultMatrix(&mat);

      if (image.dataBlock->cloakable && mCloakLevel != 0.0)
         image.shapeInstance->setAlphaAlways(0.15 + (1 - mCloakLevel) * 0.85);
      else
         image.shapeInstance->setAlphaAlways(1.0);

      if (mCloakLevel == 0.0 && (image.dataBlock->emap && gRenderEnvMaps) && state->getEnvironmentMap().getGLName() != 0) {
         image.shapeInstance->setEnvironmentMap(state->getEnvironmentMap());
         image.shapeInstance->setEnvironmentMapOn(true, 1.0);
      } else {
         image.shapeInstance->setEnvironmentMapOn(false, 1.0);
      }

      image.shapeInstance->setupFog(fogAmount,state->getFogColor());
      image.shapeInstance->animate();
      image.shapeInstance->render();

      // easiest just to shut it off here.  If we're cloaked on the next frame,
      //  we don't want envmaps...
      image.shapeInstance->setEnvironmentMapOn(false, 1.0);

      glPopMatrix();
   }
   PROFILE_END();
}


void ShapeBase::renderImage(SceneState* state, SceneRenderImage* image)
{
   glMatrixMode(GL_MODELVIEW);

   // Base shape
   F32 fogAmount = 0.0f;
   F32 dist = 0.0f;

   PROFILE_START(ShapeBaseRenderPrimary);
   if (mShapeInstance && DetailManager::selectCurrentDetail(mShapeInstance)) {
      glPushMatrix();
      dglMultMatrix(&getRenderTransform());
      glScalef(mObjScale.x,mObjScale.y,mObjScale.z);

      if (mCloakLevel != 0.0) {
         glMatrixMode(GL_TEXTURE);
         glPushMatrix();

         static U32 shiftX = 0;
         static U32 shiftY = 0;

         shiftX = (shiftX + 1) % 128;
         shiftY = (shiftY + 1) % 127;
         glTranslatef(F32(shiftX) / 127.0, F32(shiftY)/126.0, 0);
         glMatrixMode(GL_MODELVIEW);

         mShapeInstance->setAlphaAlways(0.125 + (1 - mCloakLevel) * 0.875);
         mShapeInstance->setOverrideTexture(mCloakTexture);
      }
      else {
         mShapeInstance->setAlphaAlways(1.0);
      }

      if (mCloakLevel == 0.0 && (mDataBlock->emap && gRenderEnvMaps) && state->getEnvironmentMap().getGLName() != 0) {
         mShapeInstance->setEnvironmentMap(state->getEnvironmentMap());
         mShapeInstance->setEnvironmentMapOn(true, 1.0);
      } else {
         mShapeInstance->setEnvironmentMapOn(false, 1.0);
      }

      Point3F cameraOffset;
      getRenderTransform().getColumn(3,&cameraOffset);
      cameraOffset -= state->getCameraPosition();
      dist = cameraOffset.len();
      fogAmount = state->getHazeAndFog(dist,cameraOffset.z);

      mShapeInstance->setupFog(fogAmount,state->getFogColor());
      mShapeInstance->animate();
      mShapeInstance->render();

      mShapeInstance->setEnvironmentMapOn(false, 1.0);

      if (mCloakLevel != 0.0) {
         glMatrixMode(GL_TEXTURE);
         glPopMatrix();

         mShapeInstance->clearOverrideTexture();
      }

      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
   }
   PROFILE_END();

   PROFILE_START(ShapeBaseRenderShadow);
   // Shadow...
   if (mShapeInstance && mCloakLevel == 0.0 &&
       mMount.object == NULL && mGenerateShadow == true &&
       image->isTranslucent == true)
   {
      // we are shadow enabled...
      renderShadow(dist,fogAmount);
   }
   PROFILE_END();
}

void ShapeBase::renderShadow(F32 dist, F32 fogAmount)
{
   if (Shadow::getGlobalShadowDetailLevel()<mDataBlock->noShadowLevel)
      return;
   if (mShapeInstance->getShape()->subShapeFirstTranslucentObject.empty() ||
       mShapeInstance->getShape()->subShapeFirstTranslucentObject[0] == 0)
      return;
   
   if (!mShadow)
      mShadow = new Shadow();
   
   if (Shadow::getGlobalShadowDetailLevel() < mDataBlock->genericShadowLevel)
      mShadow->setGeneric(true);
   else
      mShadow->setGeneric(false);
   
   Point3F lightDir(0.57f,0.57f,-0.57f);
   F32 shadowLen = 10.0f * mShapeInstance->getShape()->radius;
   Point3F pos = mShapeInstance->getShape()->center;
   // this is a bit of a hack...move generic shadows towards feet/base of shape
   if (Shadow::getGlobalShadowDetailLevel() < mDataBlock->genericShadowLevel)
      pos *= 0.5f;
   S32 shadowNode = mDataBlock->shadowNode;
   if (shadowNode>=0)
   {
      // adjust for movement of shape outside of bounding box by tracking this node
      Point3F offset;
      mShapeInstance->mNodeTransforms[shadowNode].getColumn(3,&offset);
      offset -= mShapeInstance->getShape()->defaultTranslations[shadowNode];
      offset.z = 0.0f;
      pos += offset;
   }
   pos.convolve(mObjScale);
   getRenderTransform().mulP(pos);

   // pos is where shadow will be centered (in world space)
   mShadow->setRadius(mShapeInstance, mObjScale);
   if (!mShadow->prepare(pos, lightDir, shadowLen, mObjScale, dist, fogAmount, mShapeInstance))
      return;

   F32 maxScale = getMax(mObjScale.x,getMax(mObjScale.y,mObjScale.z));

   if (mShadow->needBitmap())
   {
      mShadow->beginRenderToBitmap();
      mShadow->selectShapeDetail(mShapeInstance,dist,maxScale);
      mShadow->renderToBitmap(mShapeInstance, getRenderTransform(), pos, mObjScale);

      // render mounted items to shadow bitmap
      for (U32 i = 0; i < MaxMountedImages; i++)
      {
         MountedImage& image = mMountedImageList[i];
         if (image.dataBlock && image.shapeInstance)
         {
            MatrixF mat;
            getRenderImageTransform(i,&mat);
            mShadow->selectShapeDetail(image.shapeInstance,dist,maxScale);
            mShadow->renderToBitmap(image.shapeInstance,mat,pos,Point3F(1,1,1));
         }
      }

      // We only render the first mounted object for now...
      if (mMount.link && mMount.link->mShapeInstance)
      {
         Point3F linkScale = mMount.link->getScale();
         maxScale = getMax(linkScale.x,getMax(linkScale.y,linkScale.z));
         mShadow->selectShapeDetail(mMount.link->mShapeInstance,dist,maxScale);
         mShadow->renderToBitmap(mMount.link->mShapeInstance,
                                 mMount.link->getRenderTransform(),
                                 pos,
                                 linkScale);
      }

      mShadow->endRenderToBitmap();
   }
   
   mShadow->render();
}

//----------------------------------------------------------------------------

static ColorF cubeColors[8] = {
   ColorF(0, 0, 0), ColorF(1, 0, 0), ColorF(0, 1, 0), ColorF(0, 0, 1),
   ColorF(1, 1, 0), ColorF(1, 0, 1), ColorF(0, 1, 1), ColorF(1, 1, 1)
};

static Point3F cubePoints[8] = {
   Point3F(-1, -1, -1), Point3F(-1, -1,  1), Point3F(-1,  1, -1), Point3F(-1,  1,  1),
   Point3F( 1, -1, -1), Point3F( 1, -1,  1), Point3F( 1,  1, -1), Point3F( 1,  1,  1)
};

static U32 cubeFaces[6][4] = {
   { 0, 2, 6, 4 }, { 0, 2, 3, 1 }, { 0, 1, 5, 4 },
   { 3, 2, 6, 7 }, { 7, 6, 4, 5 }, { 3, 7, 5, 1 }
};

void ShapeBase::wireCube(const Point3F& size, const Point3F& pos)
{
   glDisable(GL_CULL_FACE);

   for(int i = 0; i < 6; i++) {
      glBegin(GL_LINE_LOOP);
      for(int vert = 0; vert < 4; vert++) {
         int idx = cubeFaces[i][vert];
         glVertex3f(cubePoints[idx].x * size.x + pos.x, cubePoints[idx].y * size.y + pos.y, cubePoints[idx].z * size.z + pos.z);
      }
      glEnd();
   }
}


//----------------------------------------------------------------------------

bool ShapeBase::castRay(const Point3F &start, const Point3F &end, RayInfo* info)
{
   if (mShapeInstance) {
      RayInfo shortest;
      shortest.t = 1e8;

      info->object = NULL;
      for (U32 i = 0; i < ShapeBaseData::MaxCollisionShapes; i++) {
         if (mDataBlock->LOSDetails[i] != -1) {
            mShapeInstance->animate(mDataBlock->LOSDetails[i]);
            if (mShapeInstance->castRay(start, end, info, mDataBlock->LOSDetails[i])) {
               info->object = this;
               if (info->t < shortest.t) {
                  shortest = *info;
               }
            }
         }
      }

      if (info->object == this) {
         // Copy out the shortest time...
         *info = shortest;
         return true;
      }
   }

   return false;
}


//----------------------------------------------------------------------------

bool ShapeBase::buildPolyList(AbstractPolyList* polyList, const Box3F &, const SphereF &)
{
   if (mShapeInstance) {
      bool ret = false;

      polyList->setTransform(&mObjToWorld, mObjScale);
      polyList->setObject(this);
      for (U32 i = 0; i < ShapeBaseData::MaxCollisionShapes; i++) {
         if (mDataBlock->collisionDetails[i] != -1) {
            mShapeInstance->buildPolyList(polyList,mDataBlock->collisionDetails[i]);
            ret = true;
         }
      }

      return ret;
   }

   return false;
}


void ShapeBase::buildConvex(const Box3F& box, Convex* convex)
{
   if (mShapeInstance == NULL)
      return;

   // These should really come out of a pool
   mConvexList->collectGarbage();

   Box3F realBox = box;
   mWorldToObj.mul(realBox);
   realBox.min.convolveInverse(mObjScale);
   realBox.max.convolveInverse(mObjScale);

   if (realBox.isOverlapped(getObjBox()) == false)
      return;

   for (U32 i = 0; i < ShapeBaseData::MaxCollisionShapes; i++) {
      if (mDataBlock->collisionDetails[i] != -1) {
         Box3F newbox = mDataBlock->collisionBounds[i];
         newbox.min.convolve(mObjScale);
         newbox.max.convolve(mObjScale);
         mObjToWorld.mul(newbox);
         if (box.isOverlapped(newbox) == false)
            continue;

         // See if this hull exists in the working set already...
         Convex* cc = 0;
         CollisionWorkingList& wl = convex->getWorkingList();
         for (CollisionWorkingList* itr = wl.wLink.mNext; itr != &wl; itr = itr->wLink.mNext) {
            if (itr->mConvex->getType() == ShapeBaseConvexType &&
                (static_cast<ShapeBaseConvex*>(itr->mConvex)->pShapeBase == this &&
                 static_cast<ShapeBaseConvex*>(itr->mConvex)->hullId     == i)) {
               cc = itr->mConvex;
               break;
            }
         }
         if (cc)
            continue;

         // Create a new convex.
         ShapeBaseConvex* cp = new ShapeBaseConvex;
         mConvexList->registerObject(cp);
         convex->addToWorkingList(cp);
         cp->mObject    = this;
         cp->pShapeBase = this;
         cp->hullId     = i;
         cp->box        = mDataBlock->collisionBounds[i];
      }
   }
}


//----------------------------------------------------------------------------

void ShapeBase::queueCollision(ShapeBase* obj, const F32 inData)
{
   // Add object to list of collisions.
   SimTime time = Sim::getCurrentTime();
   S32 num = obj->getId();

   CollisionTimeout** adr = &mTimeoutList;
   CollisionTimeout* ptr = mTimeoutList;
   while (ptr) {
      if (ptr->objectNumber == num) {
         if (ptr->expireTime < time) {
            ptr->expireTime = time + CollisionTimeoutValue;
            ptr->object = obj;
         }
         return;
      }
      // Recover expired entries
      if (ptr->expireTime < time) {
         CollisionTimeout* cur = ptr;
         *adr = ptr->next;
         ptr = ptr->next;
         cur->next = sFreeTimeoutList;
         sFreeTimeoutList = cur;
      }
      else {
         adr = &ptr->next;
         ptr = ptr->next;
      }
   }

   // New entry for the object
   if (sFreeTimeoutList != NULL)
   {
      ptr = sFreeTimeoutList;
      sFreeTimeoutList = ptr->next;
      ptr->next = NULL;
   }
   else
   {
      ptr = sTimeoutChunker.alloc();
   }
   
   ptr->object = obj;
   ptr->objectNumber = obj->getId();
   ptr->expireTime = time + CollisionTimeoutValue;
   ptr->next = mTimeoutList;
   if (inData != -1e9)
   {
      ptr->useData = true;
      ptr->data = inData;
   }
   else
   {
      ptr->useData = false;
   }
   
   mTimeoutList = ptr;
}

void ShapeBase::notifyCollision()
{
   // Notify all the objects that were just stamped during the queueing
   // process.
   SimTime expireTime = Sim::getCurrentTime() + CollisionTimeoutValue;
   for (CollisionTimeout* ptr = mTimeoutList; ptr; ptr = ptr->next)
   {
      if (ptr->expireTime == expireTime && ptr->object) 
      {
         SimObjectPtr<ShapeBase> safePtr(ptr->object);
         SimObjectPtr<ShapeBase> safeThis(this);
         onCollision(ptr->object);
         ptr->object = 0;

         if(!bool(safeThis))
            return;

         if(bool(safePtr))
            safePtr->onCollision(this);

         if(!bool(safeThis))
            return;
      }
   }
}

void ShapeBase::onCollision(ShapeBase* object)
{
   if (!isGhost())
      Con::executef(mDataBlock,3,"onCollision",scriptThis(),object->scriptThis());
}

//--------------------------------------------------------------------------
bool ShapeBase::pointInWater( Point3F &point )
{
   SimpleQueryList sql;
   if (isServerObject())
      gServerSceneGraph->getWaterObjectList(sql);
   else
      gClientSceneGraph->getWaterObjectList(sql);

   for (U32 i = 0; i < sql.mList.size(); i++)
   {
      WaterBlock* pBlock = dynamic_cast<WaterBlock*>(sql.mList[i]);
      if (pBlock && pBlock->isPointSubmergedSimple( point ))
         return true;
   }

   return false;
}

//----------------------------------------------------------------------------

bool ShapeBase::writePacketData(GameConnection *connection, BitStream *stream)
{
   bool ret = Parent::writePacketData(connection, stream);

   stream->write(getEnergyLevel());
   stream->write(mRechargeRate);
   return ret;
}

void ShapeBase::readPacketData(GameConnection *connection, BitStream *stream)
{
   Parent::readPacketData(connection, stream);
   
   F32 energy;
   stream->read(&energy);
   setEnergyLevel(energy);

   stream->read(&mRechargeRate);
}

F32 ShapeBase::getUpdatePriority(CameraScopeQuery *camInfo, U32 updateMask, S32 updateSkips)
{
   // If it's the scope object, must be high priority
   if (camInfo->camera == this) {
      // Most priorities are between 0 and 1, so this
      // should be something larger.
      return 10.0f;
   }
   if (camInfo->camera && (camInfo->camera->getType() & ShapeBaseObjectType))
   {
      // see if the camera is mounted to this...
      // if it is, this should have a high priority
      if(((ShapeBase *) camInfo->camera)->getObjectMount() == this)
         return 10.0f;
   }
   return Parent::getUpdatePriority(camInfo, updateMask, updateSkips);
}

U32 ShapeBase::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if (mask & InitialUpdateMask) {
      // mask off sounds that aren't playing
      S32 i;
      for (i = 0; i < MaxSoundThreads; i++)
         if (!mSoundThread[i].play)
            mask &= ~(SoundMaskN << i);

      // mask off threads that aren't running
      for (i = 0; i < MaxScriptThreads; i++)
         if (mScriptThread[i].sequence == -1)
            mask &= ~(ThreadMaskN << i);

      // mask off images that aren't updated
      for(i = 0; i < MaxMountedImages; i++)
         if(!mMountedImageList[i].dataBlock) 
            mask &= ~(ImageMaskN << i);
   }
   
   if(!stream->writeFlag(mask & (DamageMask | SoundMask | ThreadMask | ImageMask | CloakMask | MountedMask | InvincibleMask | ShieldMask)))
      return retMask;

   if (stream->writeFlag(mask & DamageMask)) {
      stream->writeFloat(mClampF(mDamage / mDataBlock->maxDamage, 0.f, 1.f), DamageLevelBits);
      stream->writeInt(mDamageState,NumDamageStateBits);
      stream->writeFlag(blowApart);
      stream->writeNormalVector( damageDir, 8 );
   }

   if (stream->writeFlag(mask & ThreadMask)) {
      for (int i = 0; i < MaxScriptThreads; i++) {
         Thread& st = mScriptThread[i];
         if (stream->writeFlag(st.sequence != -1 && (mask & (ThreadMaskN << i)))) {
            stream->writeInt(st.sequence,ThreadSequenceBits);
            stream->writeInt(st.state,2);
            stream->writeFlag(st.forward);
            stream->writeFlag(st.atEnd);
         }
      }
   }

   if (stream->writeFlag(mask & SoundMask)) {
      for (int i = 0; i < MaxSoundThreads; i++) {
         Sound& st = mSoundThread[i];
         if (stream->writeFlag(mask & (SoundMaskN << i)))
            if (stream->writeFlag(st.play))
               stream->writeRangedU32(st.profile->getId(),DataBlockObjectIdFirst,
                                      DataBlockObjectIdLast);
      }
   }

   if (stream->writeFlag(mask & ImageMask)) {
      for (int i = 0; i < MaxMountedImages; i++)
         if (stream->writeFlag(mask & (ImageMaskN << i))) {
            MountedImage& image = mMountedImageList[i];
            if (stream->writeFlag(image.dataBlock))
               stream->writeInt(image.dataBlock->getId() - DataBlockObjectIdFirst,
                                DataBlockObjectIdBitSize);
            if (stream->writeFlag(image.skinTag))
            {
               stream->writeInt(image.skinTag,NetStringTable::StringIdBitSize);
               con->checkString(image.skinTag);
            }
            stream->writeFlag(image.wet);
            stream->writeFlag(image.ammo);
            stream->writeFlag(image.loaded);
            stream->writeFlag(image.target);
            stream->writeFlag(image.triggerDown);
            stream->writeInt(image.fireCount,3);
            if (mask & InitialUpdateMask)
               stream->writeFlag(isImageFiring(i));
         }
   }

   if (stream->writeFlag(mask & (ShieldMask | CloakMask | InvincibleMask)))
   {
      if(stream->writeFlag(mask & CloakMask))
      {
         // cloaking
         stream->writeFlag( mCloaked );

         // piggyback control update
         stream->writeFlag(bool(getControllingClient()));
         
         // fading
         if(stream->writeFlag(mFading && mFadeElapsedTime >= mFadeDelay))
         {
            stream->writeFlag(mFadeOut);
            stream->write(mFadeTime);
         }
         else
            stream->writeFlag(mFadeVal == 1.0f);
      }
      if(stream->writeFlag(mask & ShieldMask))
      {
         stream->writeNormalVector(mShieldNormal, ShieldNormalBits);
         stream->writeFloat( getEnergyValue(), EnergyLevelBits );
      }
      if (stream->writeFlag(mask & InvincibleMask))
      {
         stream->write(mInvincibleTime);
         stream->write(mInvincibleSpeed);
      }
   }

   if (mask & MountedMask) {
      if (mMount.object) {
         S32 gIndex = con->getGhostIndex(mMount.object);
         if (stream->writeFlag(gIndex != -1)) {
            stream->writeFlag(true);
            stream->writeInt(gIndex,10);
            stream->writeInt(mMount.node,ShapeBaseData::NumMountPointBits);
         }
         else
            // Will have to try again later
            retMask |= MountedMask;
      }
      else
         // Unmount if this isn't the initial packet
         if (stream->writeFlag(!(mask & InitialUpdateMask)))
            stream->writeFlag(false);
   }
   else
      stream->writeFlag(false);

   return retMask;
}

void ShapeBase::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con, stream);
   mLastRenderFrame = sLastRenderFrame; // make sure we get a process after the event...

   if(!stream->readFlag())
      return;

   if (stream->readFlag()) {
      mDamage = mClampF(stream->readFloat(DamageLevelBits) * mDataBlock->maxDamage, 0.f, mDataBlock->maxDamage);
      DamageState prevState = mDamageState;
      mDamageState = DamageState(stream->readInt(NumDamageStateBits));
      blowApart = stream->readFlag();
      stream->readNormalVector( &damageDir, 8 );
      if ((prevState != Destroyed && mDamageState == Destroyed || ( blowApart )) && isProperlyAdded())
         blowUp();
      updateDamageLevel();
      updateDamageState();
   }

   if (stream->readFlag()) {
      for (S32 i = 0; i < MaxScriptThreads; i++) {
         if (stream->readFlag()) {
            Thread& st = mScriptThread[i];
            U32 seq = stream->readInt(ThreadSequenceBits);
            st.state = stream->readInt(2);
            st.forward = stream->readFlag();
            st.atEnd = stream->readFlag();
            if (st.sequence != seq)
               setThreadSequence(i,seq,false);
            else
               updateThread(st);
         }
      }
   }

   if (stream->readFlag()) {
      for (S32 i = 0; i < MaxSoundThreads; i++) {
         if (stream->readFlag()) {
            Sound& st = mSoundThread[i];
            if ((st.play = stream->readFlag()) == true) {
               st.profile = (AudioProfile*) stream->readRangedU32(DataBlockObjectIdFirst,
                                                                  DataBlockObjectIdLast);
            }
            if (isProperlyAdded())
               updateAudioState(st);
         }
      }
   }

   bool checkSkinNeeded = false;
   
   if (stream->readFlag()) {
      for (int i = 0; i < MaxMountedImages; i++) {
         if (stream->readFlag()) {
            MountedImage& image = mMountedImageList[i];
            ShapeBaseImageData* imageData = 0;
            if (stream->readFlag()) {
               SimObjectId id = stream->readInt(DataBlockObjectIdBitSize) +
                  DataBlockObjectIdFirst;
               if (!Sim::findObject(id,imageData)) {
                  con->setLastError("Invalid packet (mounted images).");
                  return;
               }
            }
            U32 skinTag = 0;
            if (stream->readFlag())
            {
               skinTag = stream->readInt(NetStringTable::StringIdBitSize);
               checkSkinNeeded = true;
            }
            image.wet = stream->readFlag();
            image.ammo = stream->readFlag();
            image.loaded = stream->readFlag();
            image.target = stream->readFlag();
            image.triggerDown = stream->readFlag();
            int count = stream->readInt(3);
            if (image.dataBlock != imageData || image.desiredTag != skinTag)
               setImage(i,imageData,skinTag,image.loaded,image.ammo,image.triggerDown);

            if (isProperlyAdded()) {
               // Normal processing
               if (count != image.fireCount)
               {
                  image.fireCount = count;
                  setImageState(i,getImageFireState(i),true);

                  if( imageData && imageData->lightType == ShapeBaseImageData::WeaponFireLight )
                  {
                     mLightTime = Sim::getCurrentTime();
                  }
               }
               updateImageState(i,0);
            }
            else
            {
               bool firing = stream->readFlag();
               if(imageData) 
               {
                  // Initial state
                  image.fireCount = count;
                  if (firing)
                     setImageState(i,getImageFireState(i),true);
               }
            }
         }
      }
   }

   if (stream->readFlag())
   {
      if(stream->readFlag()) // cloaked and control
      {
         
         setCloakedState(stream->readFlag());
         mIsControlled = stream->readFlag();

         if(( mFading = stream->readFlag()) == true)
         {
            mFadeOut = stream->readFlag();
            if(mFadeOut)
               mFadeVal = 1.0f;
            else
               mFadeVal = 0;
            stream->read(&mFadeTime);
            mFadeDelay = 0;
            mFadeElapsedTime = 0;
         }
         else
         {
            mFadeVal = F32(stream->readFlag());
         }
      }
      if(stream->readFlag()) // shielded
      {
         // Cloaking, Shield, and invul masking
         Point3F shieldNormal;
         stream->readNormalVector(&shieldNormal, ShieldNormalBits);
         F32 energyPercent = stream->readFloat(EnergyLevelBits);
         if (isProperlyAdded())
            playShieldEffect(mShieldNormal,energyPercent);
      }
      if (stream->readFlag()) {
         F32 time, speed; 
         stream->read(&time);
         stream->read(&speed);
         setupInvincibleEffect(time, speed);
      }
   }

   if(checkSkinNeeded)
      checkSkin();

   if (stream->readFlag()) {
      if (stream->readFlag()) {
         S32 gIndex = stream->readInt(10);
         ShapeBase* obj = dynamic_cast<ShapeBase*>(con->resolveGhost(gIndex));
         S32 node = stream->readInt(ShapeBaseData::NumMountPointBits);
         if(!obj)
         {
            con->setLastError("Invalid packet from server.");
            return;
         }
         obj->mountObject(this,node);
      }
      else
         unmount();
   }
}

class CheckSkinEvent : public SimEvent
{
  public:
   void process(SimObject *object)
   {
      ((ShapeBase *) object)->checkSkin();
   }
};

void ShapeBase::targetInfoChanged(TargetInfo *info)
{
   Parent::targetInfoChanged(info);
   
   if(isServerObject() || !info)
      return;

   U32 desiredPrefSkin = info->skinPrefTag;
   U32 desiredSkin = info->skinTag;

   if((sUsePrefSkins && ((desiredPrefSkin != mSkinPrefTag) || !mSkinPrefTag))
      || (!sUsePrefSkins && (mSkinTag != desiredSkin)))
   {
      const char *newBase;
      const char *newPref = "";
      mSkinTag = desiredSkin;
      mSkinPrefTag = desiredPrefSkin;

      if(!mSkinTag)
         newBase = "base";
      else
         newBase = gNetStringTable->lookupString(mSkinTag);
      if(mSkinPrefTag)
         newPref = gNetStringTable->lookupString(mSkinPrefTag);
      reSkin(mShapeInstance, newBase, newPref);
      mSkinHash = _StringTable::hashString(newBase) ^ _StringTable::hashString(newPref);
   }
}

void ShapeBase::checkSkin()
{
   NetConnection *tosv = NetConnection::getServerConnection();
   
   // now check the images
   for(int i = 0; i < MaxMountedImages; i++)
   {
      MountedImage& image = mMountedImageList[i];
      if(image.dataBlock && image.skinTag != image.desiredTag)
      {
         const char *newBase = NULL;
         if(image.desiredTag)
         {
            if(!tosv)
               return;
            U32 localString = tosv->translateRemoteStringId(image.desiredTag);
            if(!localString)
            {
               Sim::postEvent( this, new CheckSkinEvent, Sim::getCurrentTime() + 1000 );
               return;
            }
            newBase = gNetStringTable->lookupString(localString);
         }
         else
            newBase = "base";
         image.skinTag = image.desiredTag;
         reSkin(image.shapeInstance, newBase, "");
      }
   }
}


//--------------------------------------------------------------------------
void ShapeBase::forceUncloak(const char * reason)
{
   AssertFatal(isServerObject(), "ShapeBase::forceUncloak: server only call");
   if(!mCloaked)
      return;

   Con::executef(mDataBlock, 3, "onForceUncloak", scriptThis(), reason ? reason : "");
}

void ShapeBase::setCloakedState(bool cloaked)
{
   if (cloaked == mCloaked)
      return;

   if (isServerObject())
      setMaskBits(CloakMask);

   // Have to do this for the client, if we are ghosted over in the initial
   //  packet as cloaked, we set the state immediately to the extreme
   if (isProperlyAdded() == false) {
      mCloaked = cloaked;
      if (mCloaked)
         mCloakLevel = 1.0;
      else
         mCloakLevel = 0.0;
   } else {
      mCloaked = cloaked;
   }
}


//--------------------------------------------------------------------------

void ShapeBase::setHidden(bool hidden)
{
   if (hidden != mHidden) {
      // need to set a mask bit to make the ghost manager delete copies of this object
      // hacky, but oh well.
      setMaskBits(CloakMask);
      if (mHidden)
         addToScene();
      else
         removeFromScene();
      
      mHidden = hidden;
   }
}

//--------------------------------------------------------------------------
void ShapeBase::setLockedTarget(ShapeBase* pSB)
{
   AssertFatal(isServerObject(), "Shouldn't be called on the client!");

   if (pSB != NULL) {
      if( mCurrLockTarget.isNull() == false && useTargetAudio() )
      {   
         mCurrLockTarget->decLockCount();
      }
      
      mCurrLockTarget = pSB;
      mLockedOn = LockObject;
      
      if(useTargetAudio())
         pSB->incLockCount();
   } else {
      
      if( mCurrLockTarget && useTargetAudio() )
         mCurrLockTarget->decLockCount();
         
      mCurrLockTarget = NULL;
      mLockedOn = NotLocked;
   }
}


//--------------------------------------------------------------------------
void ShapeBase::setLockedTargetPosition(const Point3F& pos)
{
   mCurrLockTarget   = NULL;
   mCurrLockPosition = pos;
   mLockedOn         = LockPosition;
}

//--------------------------------------------------------------------------
bool ShapeBase::isLocked() const
{
   return mLockedOn != NotLocked;
}

S32 ShapeBase::getLockedTargetId() const
{
   if (bool(mCurrLockTarget))
      return mCurrLockTarget->getId();
   else
      return 0;
}

const Point3F& ShapeBase::getLockedPosition() const
{
   return mCurrLockPosition;
}

//--------------------------------------------------------------------------
void ShapeBase::setControlDirty()
{
   if(mControllingClient)
      mControllingClient->setControlObjectDirty();
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
Box3F ShapeBaseConvex::getBoundingBox() const
{
   return getBoundingBox(mObject->getTransform(), mObject->getScale());
}

Box3F ShapeBaseConvex::getBoundingBox(const MatrixF& mat, const Point3F& scale) const
{
   Box3F newBox = box;
   newBox.min.convolve(scale);
   newBox.max.convolve(scale);
   mat.mul(newBox);
   return newBox;
}

Point3F ShapeBaseConvex::support(const VectorF& v) const
{
   TSShape::ConvexHullAccelerator* pAccel =
      pShapeBase->mShapeInstance->getShape()->getAccelerator(pShapeBase->mDataBlock->collisionDetails[hullId]);
   AssertFatal(pAccel != NULL, "Error, no accel!");

   F32 currMaxDP = mDot(pAccel->vertexList[0], v);
   U32 index = 0;
   for (U32 i = 1; i < pAccel->numVerts; i++) {
      F32 dp = mDot(pAccel->vertexList[i], v);
      if (dp > currMaxDP) {
         currMaxDP = dp;
         index = i;
      }
   }

   return pAccel->vertexList[index];
}


void ShapeBaseConvex::getFeatures(const MatrixF& mat, const VectorF& n, ConvexFeature* cf)
{
   cf->material = 0;
   cf->object = mObject;

   TSShape::ConvexHullAccelerator* pAccel =
      pShapeBase->mShapeInstance->getShape()->getAccelerator(pShapeBase->mDataBlock->collisionDetails[hullId]);
   AssertFatal(pAccel != NULL, "Error, no accel!");

   F32 currMaxDP = mDot(pAccel->vertexList[0], n);
   U32 index = 0;
   U32 i;
   for (i = 1; i < pAccel->numVerts; i++) {
      F32 dp = mDot(pAccel->vertexList[i], n);
      if (dp > currMaxDP) {
         currMaxDP = dp;
         index = i;
      }
   }

   const U8* emitString = pAccel->emitStrings[index];
   U32 currPos = 0;
   U32 numVerts = emitString[currPos++];
   for (i = 0; i < numVerts; i++) {
      cf->mVertexList.increment();
      U32 index = emitString[currPos++];
      mat.mulP(pAccel->vertexList[index], &cf->mVertexList.last());
   }

   U32 numEdges = emitString[currPos++];
   for (i = 0; i < numEdges; i++) {
      U32 ev0 = emitString[currPos++];
      U32 ev1 = emitString[currPos++];
      cf->mEdgeList.increment();
      cf->mEdgeList.last().vertex[0] = ev0;
      cf->mEdgeList.last().vertex[1] = ev1;
   }

   U32 numFaces = emitString[currPos++];
   for (i = 0; i < numFaces; i++) {
      cf->mFaceList.increment();
      U32 plane = emitString[currPos++];
      mat.mulV(pAccel->normalList[plane], &cf->mFaceList.last().normal);
      for (U32 j = 0; j < 3; j++)
         cf->mFaceList.last().vertex[j] = emitString[currPos++];
   }
}


void ShapeBaseConvex::getPolyList(AbstractPolyList* list)
{
   list->setTransform(&pShapeBase->getTransform(), pShapeBase->getScale());
   list->setObject(pShapeBase);

   pShapeBase->mShapeInstance->animate(pShapeBase->mDataBlock->collisionDetails[hullId]);
   pShapeBase->mShapeInstance->buildPolyList(list,pShapeBase->mDataBlock->collisionDetails[hullId]);
}


//--------------------------------------------------------------------------
//----------------------------------------------------------------------------

static void cSetHidden(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   obj->setHidden(dAtob(argv[2]));
}

static bool cIsHidden(SimObject *ptr, S32, const char **)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   return obj->isHidden();
}

//----------------------------------------------------------------------------

static bool cPlayAudio(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   U32 slot = dAtoi(argv[2]);
   if (slot >= 0 && slot < ShapeBase::MaxScriptThreads) {
      AudioProfile* profile;
      if (Sim::findObject(argv[3],profile)) {
         obj->playAudio(slot,profile);
         return true;
      }
   } 
   return false;
}

static bool cStopAudio(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   U32 slot = dAtoi(argv[2]);
   if (slot >= 0 && slot < ShapeBase::MaxScriptThreads) {
      obj->stopAudio(slot);
      return true;
   }
   return false;
}


//----------------------------------------------------------------------------

static bool cPlayThread(SimObject *ptr, S32 argc, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   U32 slot = dAtoi(argv[2]);
   if (slot >= 0 && slot < ShapeBase::MaxScriptThreads) {
      if (argc == 4) {
         if (obj->getShape()) {
            S32 seq = obj->getShape()->findSequence(argv[3]);
            if (seq != -1 && obj->setThreadSequence(slot,seq))
               return true;
         }
      }
      else
         if (obj->playThread(slot))
            return true;
   }
   return false;
}

static bool cSetThreadDir(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   int slot = dAtoi(argv[2]);
   if (slot >= 0 && slot < ShapeBase::MaxScriptThreads) {
      if (obj->setThreadDir(slot,dAtob(argv[3])))
         return true;
   }
   return false;
}

static bool cStopThread(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   int slot = dAtoi(argv[2]);
   if (slot >= 0 && slot < ShapeBase::MaxScriptThreads) {
      if (obj->stopThread(slot))
         return true;
   }
   return false;
}

static bool cPauseThread(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   int slot = dAtoi(argv[2]);
   if (slot >= 0 && slot < ShapeBase::MaxScriptThreads) {
      if (obj->pauseThread(slot))
         return true;
   }
   return false;
}


//----------------------------------------------------------------------------

static bool cMountObject(SimObject *ptr, S32, const char **argv)
{
   ShapeBase *target,*obj = static_cast<ShapeBase*>(ptr);
   if (Sim::findObject(argv[2],target)) {
      S32 node = -1;
      dSscanf(argv[3],"%d",&node);
      if (node >= 0 && node < ShapeBaseData::NumMountPoints)
         obj->mountObject(target,node);
      return true;
   }
   return false;
}

static bool cUnmountObject(SimObject *ptr, S32, const char **argv)
{
   ShapeBase *target,*obj = static_cast<ShapeBase*>(ptr);
   if (Sim::findObject(argv[2],target)) {
      obj->unmountObject(target);
      return true;
   }
   return false;
}

static void cUnmount(SimObject *ptr, S32, const char **)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   obj->unmount();
}

static bool cIsMounted(SimObject *ptr, S32, const char **)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   return obj->isMounted();
}

static S32 cGetObjectMount(SimObject *ptr, S32, const char **)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   return obj->isMounted()? obj->getObjectMount()->getId(): 0;
}

static S32 cGetMountedObjectCount(SimObject *ptr, S32, const char **)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   return obj->getMountedObjectCount();
}

static S32 cGetMountedObject(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   ShapeBase* mobj = obj->getMountedObject(dAtoi(argv[2]));
   return mobj? mobj->getId(): 0;
}

static S32 cGetMountedObjectNode(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   return obj->getMountedObjectNode(dAtoi(argv[2]));
}

static S32 cGetMountNodeObject(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   ShapeBase* mobj = obj->getMountNodeObject(dAtoi(argv[2]));
   return mobj? mobj->getId(): 0;
}


//----------------------------------------------------------------------------

static bool cMountImage(SimObject *ptr, S32 argc, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   ShapeBaseImageData* imageData;
   if (Sim::findObject(argv[2],imageData)) {
      U32 slot = dAtoi(argv[3]);
      bool loaded = (argc == 5)? dAtob(argv[4]): true;
      U32 team = 0;
      if(argc == 6)
      {
         if(argv[5][0] == StringTagPrefixByte)
            team = dAtoi(argv[5]+1);
      }
      if (slot >= 0 && slot < ShapeBase::MaxMountedImages)
         obj->mountImage(imageData,slot,loaded,team);
   }
   return false;
}

static bool cUnmountImage(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   int slot = dAtoi(argv[2]);
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages)
      return obj->unmountImage(slot);
   return false;
}

static S32 cGetMountedImage(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   int slot = dAtoi(argv[2]);
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages)
      if (ShapeBaseImageData* data = obj->getMountedImage(slot))
         return data->getId();
   return 0;
}

static S32 cGetPendingImage(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   int slot = dAtoi(argv[2]);
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages)
      if (ShapeBaseImageData* data = obj->getPendingImage(slot))
         return data->getId();
   return 0;
}

static bool cIsImageFiring(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   int slot = dAtoi(argv[2]);
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages)
      return obj->isImageFiring(slot);
   return false;
}

static bool cIsImageMounted(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   ShapeBaseImageData* imageData;
   if (Sim::findObject(argv[2],imageData))
      return obj->isImageMounted(imageData);
   return false;
}

static S32 cGetMountSlot(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   ShapeBaseImageData* imageData;
   if (Sim::findObject(argv[2],imageData))
      return obj->getMountSlot(imageData);
   return -1;
}

static S32 cGetImageSkinTag(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   int slot = dAtoi(argv[2]);
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages)
      return obj->getImageSkinTag(slot);
   return -1;
}

static const char* cGetImageState(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   int slot = dAtoi(argv[2]);
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages)
      return obj->getImageState(slot);
   return "Error";
}

static bool cGetImageTrigger(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   int slot = dAtoi(argv[2]);
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages)
      return obj->getImageTriggerState(slot);
   return false;
}

static bool cSetImageTrigger(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   int slot = dAtoi(argv[2]);
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages) {
      obj->setImageTriggerState(slot,dAtob(argv[3]));
      return obj->getImageTriggerState(slot);
   }
   return false;
}

static bool cGetImageAmmo(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   int slot = dAtoi(argv[2]);
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages)
      return obj->getImageAmmoState(slot);
   return false;
}

static bool cSetImageAmmo(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   int slot = dAtoi(argv[2]);
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages) {
      bool ammo = dAtob(argv[3]);
      obj->setImageAmmoState(slot,dAtob(argv[3]));
      return ammo;
   }
   return false;
}

static bool cGetImageTarget(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   int slot = dAtoi(argv[2]);
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages)
      return obj->getImageTargetState(slot);
   return false;
}

static bool cSetImageTarget(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   int slot = dAtoi(argv[2]);
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages) {
      bool target = dAtob(argv[3]);
      obj->setImageTargetState(slot,dAtob(argv[3]));
      return target;
   }
   return false;
}

static bool cGetImageLoaded(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   int slot = dAtoi(argv[2]);
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages)
      return obj->getImageLoadedState(slot);
   return false;
}

static bool cSetImageLoaded(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   int slot = dAtoi(argv[2]);
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages) {
      bool loaded = dAtob(argv[3]);
      obj->setImageLoadedState(slot, dAtob(argv[3]));
      return loaded;
   }
   return false;
}

static const char* cGetMuzzleVector(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   int slot = dAtoi(argv[2]);
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages) {
      VectorF v;
      obj->getMuzzleVector(slot,&v);
      char* buff = Con::getReturnBuffer(100);
      dSprintf(buff,100,"%g %g %g",v.x,v.y,v.z);
      return buff;
   }
   return "0 1 0";
}

static const char* cGetMuzzlePoint(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   int slot = dAtoi(argv[2]);
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages) {
      Point3F p;
      obj->getMuzzlePoint(slot,&p);
      char* buff = Con::getReturnBuffer(100);
      dSprintf(buff,100,"%g %g %g",p.x,p.y,p.z);
      return buff;
   }
   return "0 0 0";
}

static const char* cGetSlotTransform(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   int slot = dAtoi(argv[2]);
   MatrixF xf(true);
   if (slot >= 0 && slot < ShapeBase::MaxMountedImages)
      obj->getMountTransform(slot,&xf);

   Point3F pos;
   xf.getColumn(3,&pos);
   AngAxisF aa(xf);
   char* buff = Con::getReturnBuffer(200);
   dSprintf(buff,200,"%g %g %g %g %g %g %g",
            pos.x,pos.y,pos.z,aa.axis.x,aa.axis.y,aa.axis.z,aa.angle);
   return buff;
}

static const char* cGetAIRepairPoint(SimObject *ptr, S32, const char **)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
	Point3F pos = obj->getAIRepairPoint();
   char* buff = Con::getReturnBuffer(200);
   dSprintf(buff,200,"%g %g %g", pos.x,pos.y,pos.z);
   return buff;
}

static const char* cGetVelocity(SimObject *ptr, S32, const char **)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   const VectorF& vel = obj->getVelocity();
   char* buff = Con::getReturnBuffer(100);
   dSprintf(buff,100,"%g %g %g",vel.x,vel.y,vel.z);
   return buff;
}

static bool cSetVelocity(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   VectorF vel(0,0,0);
   dSscanf(argv[2],"%f %f %f",&vel.x,&vel.y,&vel.z);
   obj->setVelocity(vel);
   obj->setControlDirty();
   return true;
}

static bool cApplyImpulse(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   Point3F pos(0,0,0);
   VectorF vel(0,0,0);
   dSscanf(argv[2],"%f %f %f",&pos.x,&pos.y,&pos.z);
   dSscanf(argv[3],"%f %f %f",&vel.x,&vel.y,&vel.z);
   obj->applyImpulse(pos,vel);
   obj->setControlDirty();
   return true;
}

static const char* cGetEyeVector(SimObject *ptr, S32, const char **)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   MatrixF mat;
   obj->getEyeTransform(&mat);
   VectorF v1(0,1,0),v2;
   mat.mulV(v1,&v2);
   char* buff = Con::getReturnBuffer(100);
   dSprintf(buff, 100,"%g %g %g",v2.x,v2.y,v2.z);
   return buff;
}

static const char* cGetEyeTransform(SimObject *ptr, S32, const char **)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   MatrixF mat;
   obj->getEyeTransform(&mat);

   Point3F pos;
   mat.getColumn(3,&pos);
   AngAxisF aa(mat);
   char* buff = Con::getReturnBuffer(100);
   dSprintf(buff,100,"%g %g %g %g %g %g %g",
            pos.x,pos.y,pos.z,aa.axis.x,aa.axis.y,aa.axis.z,aa.angle);
   return buff;
}

static void cSetEnergyLevel(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   obj->setEnergyLevel(dAtof(argv[2]));
   obj->setControlDirty();
}

static F32 cGetEnergyLevel(SimObject *ptr, S32, const char **)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   return obj->getEnergyLevel();
}

static F32 cGetEnergyPercent(SimObject *ptr, S32, const char **)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   return obj->getEnergyValue();
}

static void cSetDamageLevel(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   obj->setDamageLevel(dAtof(argv[2]));
}

static F32 cGetDamageLevel(SimObject *ptr, S32, const char **)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   return obj->getDamageLevel();
}

static F32 cGetDamagePercent(SimObject *ptr, S32, const char **)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   return obj->getDamageValue();
}

static bool cSetDamageState(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   return obj->setDamageState(argv[2]);
}

static const char* cGetDamageState(SimObject *ptr, S32, const char **)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   return obj->getDamageStateName();
}

static bool cIsDestroyed(SimObject *ptr, S32, const char **)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   return obj->isDestroyed();
}

static bool cIsDisabled(SimObject *ptr, S32, const char **)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   return obj->getDamageState() != ShapeBase::Enabled;
}

static bool cIsEnabled(SimObject *ptr, S32, const char **)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   return obj->getDamageState() == ShapeBase::Enabled;
}

static void cApplyDamage(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   obj->applyDamage(dAtof(argv[2]));
}

static void cApplyRepair(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   obj->applyRepair(dAtof(argv[2]));
}

static void cSetRepairRate(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   F32 rate = dAtof(argv[2]);
   if(rate < 0)
      rate = 0;
   obj->setRepairRate(rate);
}

static F32 cGetRepairRate(SimObject *ptr, S32, const char **)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   return obj->getRepairRate();
}

static void cSetRechargeRate(SimObject *ptr, S32, const char **argv)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   obj->setRechargeRate(dAtof(argv[2]));
   obj->setControlDirty();
}

static F32 cGetRechargeRate(SimObject *ptr, S32, const char **)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   return obj->getRechargeRate();
}

static S32 cGetControllingClient(SimObject *ptr, S32, const char **)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   if (GameConnection* con = obj->getControllingClient())
      return con->getId();
   return 0;
}

static S32 cGetControllingObject(SimObject *ptr, S32, const char **)
{
   ShapeBase* obj = static_cast<ShapeBase*>(ptr);
   if (ShapeBase* con = obj->getControllingObject())
      return con->getId();
   return 0;
}

// return true if can cloak, otherwise the reason why object cannot cloak
static const char * cCanCloak(SimObject * obj, S32, const char **)
{
   ShapeBase * shape = static_cast<ShapeBase*>(obj);

   // target?
   TargetInfo * targInfo = shape->getTargetInfo();
   if(!targInfo)
      return("true");
  
   // target is sensorjammed?
   if(targInfo->sensorFlags & TargetInfo::EnemySensorJammed)
      return("jammed");

   return("true");
}

static void cSetCloaked(SimObject* obj, S32, const char** argv)
{
   AssertFatal(dynamic_cast<ShapeBase*>(obj) != NULL, "How did a non-shapebase get here?");
   ShapeBase* shape = static_cast<ShapeBase*>(obj);

   bool cloaked = dAtob(argv[2]);
   if (shape->isServerObject())
      shape->setCloakedState(cloaked);
}

static bool cIsCloaked(SimObject* obj, S32, const char**)
{
   AssertFatal(dynamic_cast<ShapeBase*>(obj) != NULL, "How did a non-shapebase get here?");
   ShapeBase* shape = static_cast<ShapeBase*>(obj);

   return shape->getCloakedState();
}

static void cSetPassiveJammed(SimObject * obj, S32, const char ** argv)
{
   ShapeBase * shape = static_cast<ShapeBase*>(obj);
   if(shape->isServerObject())
      shape->setPassiveJamState(dAtob(argv[2]));
}

static bool cIsPassiveJammed(SimObject * obj, S32, const char **)
{
   ShapeBase * shape = static_cast<ShapeBase*>(obj);
   return(shape->getPassiveJamState());
}

static void cSetDamageFlash(SimObject* obj, S32, const char** argv)
{
   AssertFatal(dynamic_cast<ShapeBase*>(obj) != NULL, "How did a non-shapebase get here?");
   ShapeBase* shape = static_cast<ShapeBase*>(obj);

   F32 flash = dAtof(argv[2]);
   if (shape->isServerObject())
      shape->setDamageFlash(flash);
}

static F32 cGetDamageFlash(SimObject* obj, S32, const char**)
{
   AssertFatal(dynamic_cast<ShapeBase*>(obj) != NULL, "How did a non-shapebase get here?");
   ShapeBase* shape = static_cast<ShapeBase*>(obj);

   return shape->getDamageFlash();
}

static void cSetWhiteOut(SimObject* obj, S32, const char** argv)
{
   AssertFatal(dynamic_cast<ShapeBase*>(obj) != NULL, "How did a non-shapebase get here?");
   ShapeBase* shape = static_cast<ShapeBase*>(obj);

   F32 flash = dAtof(argv[2]);
   if (shape->isServerObject())
      shape->setWhiteOut(flash);
}

static F32 cGetWhiteOut(SimObject* obj, S32, const char**)
{
   AssertFatal(dynamic_cast<ShapeBase*>(obj) != NULL, "How did a non-shapebase get here?");
   ShapeBase* shape = static_cast<ShapeBase*>(obj);

   return shape->getWhiteOut();
}

static void cSetHeat(SimObject* obj, S32, const char** argv)
{
   AssertFatal(dynamic_cast<ShapeBase*>(obj) != NULL, "How did a non-shapebase get here?");
   ShapeBase* shape = static_cast<ShapeBase*>(obj);

   F32 heat = dAtof(argv[2]);
   if (heat < 0.0f || heat > 1.0f)
      heat = heat < 0.0f ? 0.0 : 1.0;

   if (shape->isServerObject())
      shape->setHeat(heat);
}

static F32 cGetCameraFov(SimObject* obj, S32, const char**)
{
   AssertFatal(dynamic_cast<ShapeBase*>(obj) != NULL, "How did a non-shapebase get here?");
   ShapeBase* shape = static_cast<ShapeBase*>(obj);

   if (shape->isServerObject())
      return shape->getCameraFov();
   return 0.0;
}

static void cSetCameraFov(SimObject* obj, S32, const char** argv)
{
   AssertFatal(dynamic_cast<ShapeBase*>(obj) != NULL, "How did a non-shapebase get here?");
   ShapeBase* shape = static_cast<ShapeBase*>(obj);

   if (shape->isServerObject())
      shape->setCameraFov(dAtof(argv[2]));
}

static F32 cGetHeat(SimObject* obj, S32, const char**)
{
   AssertFatal(dynamic_cast<ShapeBase*>(obj) != NULL, "How did a non-shapebase get here?");
   ShapeBase* shape = static_cast<ShapeBase*>(obj);

   if (shape->isServerObject())
      return shape->getHeat();
   return 0.0;
}

static void cSetupInvincibleEffect(SimObject* obj, S32, const char** argv)
{
   AssertFatal(dynamic_cast<ShapeBase*>(obj) != NULL, "How did a non-shapebase get here?");
   ShapeBase* shape = static_cast<ShapeBase*>(obj);

   shape->setupInvincibleEffect(dAtof(argv[2]), dAtof(argv[3]));
}

static bool cSetLockedTarget(SimObject* obj, S32, const char** argv)
{
   AssertFatal(dynamic_cast<ShapeBase*>(obj) != NULL, "How did a non-shapebase get here?");
   ShapeBase* shape = static_cast<ShapeBase*>(obj);
   if (shape->isServerObject() == false)
      return false;

   S32 id = dAtoi(argv[2]);
   if (id == 0) {
      shape->setLockedTarget(NULL);
      return true;
   } else {
      ShapeBase* pObject;
      if (Sim::findObject(id, pObject)) {
         shape->setLockedTarget(pObject);
         return true;
      } else {
         shape->setLockedTarget(NULL);
         return false;
      }
   }
}

static S32 cGetLockedTarget(SimObject* obj, S32, const char**)
{
   AssertFatal(dynamic_cast<ShapeBase*>(obj) != NULL, "How did a non-shapebase get here?");
   ShapeBase* shape = static_cast<ShapeBase*>(obj);
   if (shape->isServerObject() == false)
      return 0;

   return shape->getLockedTargetId();
}


static const char* cGetLockedPosition(SimObject* obj, S32, const char**)
{
   AssertFatal(dynamic_cast<ShapeBase*>(obj) != NULL, "How did a non-shapebase get here?");
   ShapeBase* shape = static_cast<ShapeBase*>(obj);
   if (shape->isServerObject() == false)
      return "0 0 0";

   char* returnBuffer = Con::getReturnBuffer(256);
   Point3F center = shape->getLockedPosition();
   char *buff = Con::getReturnBuffer(128);
   dSprintf(buff,128, "%f %f %f", center.x, center.y, center.z);
   return buff;
}

static bool cIsLocked(SimObject* obj, S32, const char**)
{
   AssertFatal(dynamic_cast<ShapeBase*>(obj) != NULL, "How did a non-shapebase get here?");
   ShapeBase* shape = static_cast<ShapeBase*>(obj);
   if (shape->isServerObject() == false)
      return false;

   return shape->isLocked();
}

static bool cIsTracking(SimObject* obj, S32, const char**)
{
   AssertFatal(dynamic_cast<ShapeBase*>(obj) != NULL, "How did a non-shapebase get here?");
   ShapeBase* shape = static_cast<ShapeBase*>(obj);
   if (shape->isServerObject() == false)
      return false;

   return shape->isTracking();
}


static void cPlayShieldEffect( SimObject* obj, S32, const char** argv )
{
   AssertFatal(dynamic_cast<ShapeBase*>(obj) != NULL, "How did a non-shapebase get here?");
   ShapeBase* shape = static_cast<ShapeBase*>(obj);
   if (shape->isServerObject() == false)
      return;

   Point3F normal;
   dSscanf(argv[2], "%f %f %f", &normal.x, &normal.y, &normal.z);
   normal.normalize();

   shape->playShieldEffect(normal);
}

static void cScopeWhenSensorVisible(SimObject * obj, S32, const char ** argv)
{
   ShapeBase * shapeBase = static_cast<ShapeBase*>(obj);

   if (dAtob(argv[2]))
      Sim::getScopeSensorVisibleSet()->addObject(obj);
   else
      Sim::getScopeSensorVisibleSet()->removeObject(obj);
}

ConsoleFunction(setShadowDetailLevel, void , 2, 2, "setShadowDetailLevel(val 0...1);")
{
   argc;
   F32 val = dAtof(argv[1]);
   if (val < 0.0f)
      val = 0.0f;
   else if (val > 1.0f)
      val = 1.0f;

   if (mFabs(Shadow::getGlobalShadowDetailLevel()-val)<0.001f)
      return;

   // shadow details determined in two places:
   // 1. setGlobalShadowDetailLevel
   // 2. static shape header has some #defines that determine
   //    at what level of shadow detail each type of
   //    object uses a generic shadow or no shadow at all
   Shadow::setGlobalShadowDetailLevel(val);
   Con::setFloatVariable("$pref::Shadows", val);
}

static void cSetDeployRotation(SimObject *obj, S32, const char** argv)
{
   ShapeBase *SB = static_cast<ShapeBase*>(obj);
   if(!SB)
      return;
      
   Point3F normal;
   Point3F position;
   dSscanf(argv[2], "%f %f %f", &position.x, &position.y, &position.z);
   dSscanf(argv[3], "%f %f %f", &normal.x, &normal.y, &normal.z);
   normal.normalize();
   
   VectorF xAxis;
   if( mFabs(normal.z) > mFabs(normal.x) && mFabs(normal.z) > mFabs(normal.y))
      mCross( VectorF( 0, 1, 0 ), normal, &xAxis );
   else
      mCross( VectorF( 0, 0, 1 ), normal, &xAxis );
   
   VectorF yAxis;
   mCross( normal, xAxis, &yAxis );

   MatrixF testMat(true);
   testMat.setColumn( 0, xAxis );
   testMat.setColumn( 1, yAxis );
   testMat.setColumn( 2, normal );
   testMat.setPosition( position );
   
   SB->setTransform( testMat );
}

static void cStartFade(SimObject *obj, S32, const char** argv)
{
   U32   fadeTime;
   U32   fadeDelay;
   bool  fadeOut;
   
   dSscanf(argv[2], "%d", &fadeTime );
   dSscanf(argv[3], "%d", &fadeDelay );
   fadeOut = dAtob(argv[4]);

   if( obj )
   {
      ShapeBase *sbo = static_cast< ShapeBase * >( obj );
      if( sbo )
      {
         sbo->startFade( fadeTime / 1000.0, fadeDelay / 1000.0, fadeOut );
      }
   }

}

static void cBlowup(SimObject * obj, S32, const char **)
{
   ShapeBase * shape = static_cast<ShapeBase*>(obj);
   
   if(shape)
   {   
      shape->blowApart = true;
   }
}

static void cSetMomVector(SimObject * obj, S32, const char **argv)
{
   ShapeBase * shape = static_cast<ShapeBase*>(obj);
   
   if(shape)
   {   
      VectorF normal;
      dSscanf(argv[2], "%f %f %f", &normal.x, &normal.y, &normal.z);
      normal.normalize();
      
      shape->damageDir.set( normal.x, normal.y, normal.z );
   }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void ShapeBase::initPersistFields()
{
   Parent::initPersistFields();
}

void ShapeBase::consoleInit()
{
   Con::addVariable("SB::DFDec", TypeF32, &sDamageFlashDec);
   Con::addVariable("SB::WODec", TypeF32, &sWhiteoutDec);
   Con::addVariable("pref::environmentMaps", TypeBool, &gRenderEnvMaps);
   Con::addVariable("pref::usePrefSkins", TypeBool, &sUsePrefSkins);

   //
   Con::addCommand("ShapeBase", "playAudio", cPlayAudio, "obj.playAudio(slot,AudioProfile)", 4, 4);
   Con::addCommand("ShapeBase", "stopAudio", cStopAudio, "obj.playAudio(slot)", 3, 3);

   Con::addCommand("ShapeBase", "playThread", cPlayThread, "obj.playThread(thread,<sequence>)", 3, 4);
   Con::addCommand("ShapeBase", "setThreadDir", cSetThreadDir, "obj.setThreadDir(thread,bool)", 4, 4);
   Con::addCommand("ShapeBase", "stopThread", cStopThread, "obj.stopThread(thread)", 3, 3);
   Con::addCommand("ShapeBase", "pauseThread", cPauseThread, "obj.pauseThread(thread)", 3, 3);

   Con::addCommand("ShapeBase", "mountObject", cMountObject, "obj.mountObject(object,node)", 4, 4);
   Con::addCommand("ShapeBase", "unmountObject", cUnmountObject, "obj.unmountObject(object)", 3, 3);
   Con::addCommand("ShapeBase", "unmount", cUnmount, "obj.unmount()", 2, 2);
   Con::addCommand("ShapeBase", "isMounted", cIsMounted, "obj.isMounted()", 2, 2);
   Con::addCommand("ShapeBase", "getObjectMount", cGetObjectMount, "obj.getObjectMount()", 2, 2);
   Con::addCommand("ShapeBase", "getMountedObjectCount", cGetMountedObjectCount, "obj.getMountedObjectCount()", 2, 2);
   Con::addCommand("ShapeBase", "getMountedObjectNode", cGetMountedObjectNode, "obj.getMountedObjectNode(index)", 3, 3);
   Con::addCommand("ShapeBase", "getMountedObject", cGetMountedObject, "obj.getMountedObject(index)", 3, 3);
   Con::addCommand("ShapeBase", "getMountNodeObject", cGetMountNodeObject, "obj.getMountNodeObject(node)", 3, 3);

   Con::addCommand("ShapeBase", "mountImage", cMountImage, "obj.mountImage(DataBlock,slot,[loaded=true],[skinTag])", 4, 6);
   Con::addCommand("ShapeBase", "unmountImage", cUnmountImage, "obj.unmountImage(slot)", 3, 3);
   Con::addCommand("ShapeBase", "getMountedImage", cGetMountedImage, "obj.getMountedImage(slot)", 3, 3);
   Con::addCommand("ShapeBase", "getPendingImage", cGetPendingImage, "obj.getPendingImage(slot)", 3, 3);
   Con::addCommand("ShapeBase", "isImageFiring", cIsImageFiring, "obj.isImageFiring(slot)", 3, 3);
   Con::addCommand("ShapeBase", "isImageMounted", cIsImageMounted, "obj.isImageMounted(DataBlock)", 3, 3);
   Con::addCommand("ShapeBase", "getMountSlot", cGetMountSlot, "obj.getMountSlot(DataBlock)", 3, 3);
   Con::addCommand("ShapeBase", "getImageSkinTag", cGetImageSkinTag, "obj.getImageSkinTag(slot)", 3, 3);
   Con::addCommand("ShapeBase", "getImageState", cGetImageState, "obj.getImageState(slot)", 3, 3);
   Con::addCommand("ShapeBase", "getImageTrigger", cGetImageTrigger, "obj.getImageTrigger(slot)", 3, 3);
   Con::addCommand("ShapeBase", "setImageTrigger", cSetImageTrigger, "obj.setImageTrigger(slot,bool)", 4, 4);
   Con::addCommand("ShapeBase", "getImageAmmo", cGetImageAmmo, "obj.getImageAmmo(slot)", 3, 3);
   Con::addCommand("ShapeBase", "setImageAmmo", cSetImageAmmo, "obj.setImageAmmo(slot,bool)", 4, 4);
   Con::addCommand("ShapeBase", "getImageTarget", cGetImageTarget, "obj.getImageTarget(slot)", 3, 3);
   Con::addCommand("ShapeBase", "setImageTarget", cSetImageTarget, "obj.setImageTarget(slot,bool)", 4, 4);
   Con::addCommand("ShapeBase", "getImageLoaded", cGetImageLoaded, "obj.getImageLoaded(slot)", 3, 3);
   Con::addCommand("ShapeBase", "setImageLoaded", cSetImageLoaded, "obj.setImageLoaded(slot,bool)", 4, 4);
   Con::addCommand("ShapeBase", "getMuzzleVector", cGetMuzzleVector, "obj.getMuzzleVector(slot)", 3, 3);
   Con::addCommand("ShapeBase", "getMuzzlePoint", cGetMuzzlePoint, "obj.getMuzzlePoint(slot)", 3, 3);
   Con::addCommand("ShapeBase", "getSlotTransform", cGetSlotTransform, "obj.getSlotTransform(slot)", 3, 3);
   Con::addCommand("ShapeBase", "getAIRepairPoint", cGetAIRepairPoint, "obj.getAIRepairPoint()", 2, 2);

   Con::addCommand("ShapeBase", "getVelocity", cGetVelocity, "obj.getVelocity()", 2, 2);
   Con::addCommand("ShapeBase", "setVelocity", cSetVelocity, "obj.setVelocity(Vector)", 3, 3);
   Con::addCommand("ShapeBase", "applyImpulse", cApplyImpulse, "obj.applyImpulse(Pos,Vector)", 4, 4);
   Con::addCommand("ShapeBase", "getEyeVector", cGetEyeVector, "obj.getEyeVector()", 2, 2);
   Con::addCommand("ShapeBase", "getEyeTransform", cGetEyeTransform, "obj.getEyeTransform()", 2, 2);

   Con::addCommand("ShapeBase", "setEnergyLevel", cSetEnergyLevel, "obj.setEnergyLevel(value)", 3, 3);
   Con::addCommand("ShapeBase", "getEnergyLevel", cGetEnergyLevel, "obj.getEnergyLevel()", 2, 2);
   Con::addCommand("ShapeBase", "getEnergyPercent", cGetEnergyPercent, "obj.getEnergyPercent()", 2, 2);
   Con::addCommand("ShapeBase", "setDamageLevel", cSetDamageLevel, "obj.setDamageLevel(value)", 3, 3);
   Con::addCommand("ShapeBase", "getDamageLevel", cGetDamageLevel, "obj.getDamageLevel()", 2, 2);
   Con::addCommand("ShapeBase", "getDamagePercent", cGetDamagePercent, "obj.getDamagePercent()", 2, 2);
   Con::addCommand("ShapeBase", "setDamageState", cSetDamageState, "obj.setDamageState(state)", 3, 3);
   Con::addCommand("ShapeBase", "getDamageState", cGetDamageState, "obj.getDamageState()", 2, 2);
   Con::addCommand("ShapeBase", "isDestroyed", cIsDestroyed, "obj.isDestroyed()", 2, 2);
   Con::addCommand("ShapeBase", "isDisabled", cIsDisabled, "obj.isDisabled()", 2, 2);
   Con::addCommand("ShapeBase", "isEnabled", cIsEnabled, "obj.isEnabled()", 2, 2);

   Con::addCommand("ShapeBase", "applyDamage", cApplyDamage, "obj.applyDamage(value)", 3, 3);
   Con::addCommand("ShapeBase", "applyRepair", cApplyRepair, "obj.applyRepair(value)", 3, 3);

   Con::addCommand("ShapeBase", "setRepairRate", cSetRepairRate, "obj.setRepairRate(value)", 3, 3);
   Con::addCommand("ShapeBase", "getRepairRate", cGetRepairRate, "obj.getRepairRate()", 2, 2);
   Con::addCommand("ShapeBase", "setRechargeRate", cSetRechargeRate, "obj.setRechargeRate(value)", 3, 3);
   Con::addCommand("ShapeBase", "getRechargeRate", cGetRechargeRate, "obj.getRechargeRate()", 2, 2);

   Con::addCommand("ShapeBase", "getControllingClient", cGetControllingClient, "obj.getControllingClient()", 2, 2);
   Con::addCommand("ShapeBase", "getControllingObject", cGetControllingObject, "obj.getControllingObject()", 2, 2);

   Con::addCommand("ShapeBase", "canCloak", cCanCloak, "obj.canCloak()", 2, 2);
   Con::addCommand("ShapeBase", "setCloaked", cSetCloaked, "obj.setCloaked(true|false)", 3, 3);
   Con::addCommand("ShapeBase", "isCloaked", cIsCloaked, "obj.isCloaked()", 2, 2);

   Con::addCommand("ShapeBase", "setPassiveJammed", cSetPassiveJammed, "obj.setPassiveJammed(true|false)", 3, 3);
   Con::addCommand("ShapeBase", "isPassiveJammed", cIsPassiveJammed, "obj.isPassiveJammed()", 2, 2);
   
   Con::addCommand("ShapeBase", "setDamageFlash", cSetDamageFlash, "obj.setDamageFlash(flash level)", 3, 3);
   Con::addCommand("ShapeBase", "getDamageFlash", cGetDamageFlash, "obj.getDamageFlash()", 2, 2);
   Con::addCommand("ShapeBase", "setWhiteOut", cSetWhiteOut, "obj.setWhiteOut(flash level)", 3, 3);
   Con::addCommand("ShapeBase", "getWhiteOut", cGetWhiteOut, "obj.getWhiteOut()", 2, 2);
   Con::addCommand("ShapeBase", "setInvincibleMode", cSetupInvincibleEffect, "obj.setInvincibleMode(time <sec>, speed)", 4, 4);

   Con::addCommand("ShapeBase", "getCameraFov", cGetCameraFov, "obj.getCameraFov()", 2, 2);
   Con::addCommand("ShapeBase", "setCameraFov", cSetCameraFov, "obj.setCameraFov(fov)", 3, 3);

   Con::addCommand("ShapeBase", "getHeat", cGetHeat, "obj.getHeat()", 2, 2);
   Con::addCommand("ShapeBase", "setHeat", cSetHeat, "obj.getHeat(heat [0..1])", 3, 3);
   
   Con::addCommand("ShapeBase", "setLockedTarget",   cSetLockedTarget,   "obj.setLockedTarget(id)", 3, 3);
   Con::addCommand("ShapeBase", "getLockedTarget",   cGetLockedTarget,   "obj.getLockedTarget()", 2, 2);
   Con::addCommand("ShapeBase", "getLockedPosition", cGetLockedPosition, "obj.getLockedPosition()", 2, 2);
   Con::addCommand("ShapeBase", "isLocked",          cIsLocked,          "obj.isLocked()", 2, 2);
   Con::addCommand("ShapeBase", "isTracking",        cIsTracking,        "obj.isTracking()", 2, 2);

   Con::addCommand("ShapeBase", "hide", cSetHidden, "obj.hide(bool)", 3, 3);
   Con::addCommand("ShapeBase", "isHidden", cIsHidden, "obj.isHidden()", 2, 2);

   Con::addCommand("ShapeBase", "playShieldEffect", cPlayShieldEffect, "obj.playShieldEffect( Vector )", 3, 3 );
   Con::addCommand("ShapeBase", "scopeWhenSensorVisible", cScopeWhenSensorVisible, "obj.scopeWhenSensorVisible(bool)", 3, 3);
   
   Con::addCommand("ShapeBase", "setDeployRotation", cSetDeployRotation, "setDeployRotation( normal )", 4, 4);
   Con::addCommand("ShapeBase", "startFade", cStartFade, "startFade( U32, U32, bool )", 5, 5);
   Con::addCommand("ShapeBase", "setMomentumVector", cSetMomVector, "obj.setMomentumVector()", 3, 3);
   Con::addCommand("ShapeBase", "blowup", cBlowup, "obj.blowup()", 2, 2);
}

bool ShapeBase::isInvincible()
{
   if( mDataBlock )
   {
      return mDataBlock->isInvincible;
   }
   return false;
}

void ShapeBase::startFade( F32 fadeTime, F32 fadeDelay, bool fadeOut )
{
   setMaskBits(CloakMask);
   mFadeElapsedTime = 0;
   mFading = true;
   if(fadeDelay < 0)
      fadeDelay = 0;
   if(fadeTime < 0)
      fadeTime = 0;
   mFadeTime = fadeTime;
   mFadeDelay = fadeDelay;
   mFadeOut = fadeOut;
   mFadeVal = F32(mFadeOut);
}


//--------------------------------------------------------------------------
//  Get node transform
//--------------------------------------------------------------------------
MatrixF ShapeBase::getNodeTransform( StringTableEntry nodeName )
{
   TSShape *shape = mShapeInstance->getShape();
   if( !shape )
   {
      return MatrixF(true);
   }
   
   S32 node = shape->findNode( nodeName );
   if( node < 0 )
   {
      return MatrixF(true);
   }
   
   return mShapeInstance->mNodeTransforms[node];
}
