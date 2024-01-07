//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "game/turret.h"
#include "core/bitStream.h"
#include "game/gameConnection.h"
#include "ts/tsShapeInstance.h"
#include "console/consoleTypes.h"
#include "game/projectile.h"
#include "ai/graphMath.h"
#include "game/shockwave.h"

IMPLEMENT_CO_DATABLOCK_V1(TurretData);
IMPLEMENT_CO_DATABLOCK_V1(TurretImageData);
IMPLEMENT_CO_NETOBJECT_V1(Turret);

const U32 Turret::csmActiveScanMask   = (TerrainObjectType  |
                                         InteriorObjectType |
                                         VehicleObjectType);

const F32 Turret::csmFullyDeactivated = 0.0;
const F32 Turret::csmFullyActivated   = 1.0;
const F32 Turret::csmPhiNull          = 0.0;
const F32 Turret::csmThetaNull        = 90.0;

const U32 Turret::csmDefaultDeactivateDelay = 1000;
const U32 Turret::csmDefaultThinkTime       = 200;
const F32 Turret::csmDefaultActivationSpeed = 1.0f;
const F32 Turret::csmDefaultPhiSpeed        = 180.0f;
const F32 Turret::csmDefaultThetaSpeed      = 45.0f;
const F32 Turret::csmDefaultAttackRadius    = 40.0f;

const F32 sgBigClunkyMultiplicationFactor = 40.0f;

namespace {

void cTurretSetSkill(SimObject* obj, S32, const char** argv)
{
   AssertFatal(dynamic_cast<Turret*>(obj) != NULL, "How did a non-turret get here?");
   Turret* pTurret = static_cast<Turret*>(obj);

   F32 skillLevel = dAtof(argv[2]);
   pTurret->setSkill(skillLevel);
}

bool cTurretSetTarget(SimObject* obj, S32, const char** argv)
{
   AssertFatal(dynamic_cast<Turret*>(obj) != NULL, "How did a non-turret get here?");
   Turret* pTurret = static_cast<Turret*>(obj);

   S32 targetId = dAtoi(argv[2]);
   ShapeBase* pTarget = NULL;
   Sim::findObject(targetId, pTarget);

   return pTurret->setTarget(pTarget);
}

void cTurretClearTarget(SimObject* obj, S32, const char**)
{
   AssertFatal(dynamic_cast<Turret*>(obj) != NULL, "How did a non-turret get here?");
   Turret* pTurret = static_cast<Turret*>(obj);

   pTurret->setTarget(NULL);
}


S32 cTurretGetTarget(SimObject* obj, S32, const char**)
{
   AssertFatal(dynamic_cast<Turret*>(obj) != NULL, "How did a non-turret get here?");
   Turret* pTurret = static_cast<Turret*>(obj);

   return pTurret->getTargetId();
}

bool cTurretIsValidTarget(SimObject* obj, S32, const char** argv)
{
   AssertFatal(dynamic_cast<Turret*>(obj) != NULL, "How did a non-turret get here?");
   Turret* pTurret = static_cast<Turret*>(obj);

   S32 targetId = dAtoi(argv[2]);
   ShapeBase* pTarget = NULL;
   Sim::findObject(targetId, pTarget);

   if (pTarget)
      return pTurret->isValidTarget(pTarget);
   else
      return false;
}


bool cTurretBarrelReplace(SimObject* obj, S32, const char** argv)
{
   AssertFatal(dynamic_cast<Turret*>(obj) != NULL, "How did a non-turret get here?");
   Turret* pTurret = static_cast<Turret*>(obj);

   S32 targetId = dAtoi(argv[2]);
   ShapeBase* pTarget = NULL;
   Sim::findObject(targetId, pTarget);

   if (pTarget)
      return pTurret->initiateReplace(pTarget);
   else
      return false;
}

static void cTurretSetAutoFire(SimObject* obj, S32, const char** argv)
{
   AssertFatal(dynamic_cast<Turret*>(obj) != NULL, "How did a non-turret get here?");
   Turret* pTurret = static_cast<Turret*>(obj);

   bool status = (!dStricmp(argv[2], "true")) || (dAtoi(argv[2]) > 0);
   pTurret->setAutoFire(status);
}

} // namespace {}


//--------------------------------------------------------------------------
//--------------------------------------
//
TurretData::TurretData()
{
   thetaMin             = 45;
   thetaMax             = 135;
   thetaNull            = -1;

   neverUpdateControl   = false;

   primaryAxis = ZAxis;
}

TurretData::~TurretData()
{

}


//--------------------------------------------------------------------------
static EnumTable::Enums sgPrimaryAxisEnums[] = 
{
   { TurretData::YAxis,    "yaxis"    },
   { TurretData::RevYAxis, "revyaxis" },
   { TurretData::ZAxis,    "zaxis"    },
   { TurretData::RevZAxis, "revzaxis" }
};
static EnumTable sgPrimaryAxisTable(4, &sgPrimaryAxisEnums[0]); 

void TurretData::initPersistFields()
{
   Parent::initPersistFields();

   addField("thetaMin",  TypeF32, Offset(thetaMin,  TurretData));
   addField("thetaMax",  TypeF32, Offset(thetaMax,  TurretData));
   addField("thetaNull", TypeF32, Offset(thetaNull, TurretData));
   addField("neverUpdateControl", TypeBool, Offset(neverUpdateControl, TurretData));
   addField("primaryAxis", TypeEnum, Offset(primaryAxis, TurretData), 1, &sgPrimaryAxisTable);
}

bool TurretData::preload(bool server, char errorBuffer[256])
{
   if (Parent::preload(server, errorBuffer) == false)
      return false;

   if (bool(shape)) {
      activateSeq = shape->findSequence("activate");
      elevateSeq  = shape->findSequence("elevate");
      turnSeq     = shape->findSequence("turn");

      if (activateSeq == -1 || elevateSeq == -1 || turnSeq == -1) {
         Con::errorf(ConsoleLogEntry::General,
                     "TurretData(%s)::onAdd: shape %s does not have all sequences necessary (turn, elevate, activate)",
                     getName(), shapeName);
         return false;
      } else {
         if (thetaMin < 0.0f || thetaMin > 90.0f) {
            Con::warnf(ConsoleLogEntry::General,
                       "TurretData(%s)::onAdd: thetaMin must be in the range [0, 90]", getName());
            thetaMin = thetaMin < 0.0 ? 0.0 : 90.0f;
         }
         if (thetaMax < 90.0f || thetaMax > 180.0f) {
            Con::warnf(ConsoleLogEntry::General,
                       "TurretData(%s)::onAdd: thetaMax must be in the range [90, 180]", getName());
            thetaMax = thetaMax < 90.0 ? 90.0 : 180.0f;
         }

         return true;
      }
   }

   Con::errorf(ConsoleLogEntry::General, "TurretData(%s)::onAdd: must have a shape: %s not found", getName(), shapeName);
   return false;
}


//--------------------------------------------------------------------------
void TurretData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->write(thetaMin);
   stream->write(thetaMax);
   stream->write(thetaNull);
   stream->writeFlag(neverUpdateControl);
   stream->writeRangedU32(primaryAxis, __FirstValidAxis, __LastValidAxis);
}

void TurretData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);
   
   stream->read(&thetaMin);
   stream->read(&thetaMax);
   stream->read(&thetaNull);
   neverUpdateControl = stream->readFlag();
   primaryAxis = (PrimaryAxis)stream->readRangedU32(__FirstValidAxis, __LastValidAxis);
}


//--------------------------------------------------------------------------
//--------------------------------------
//
TurretImageData::TurretImageData()
{
   activationMS      = 1000;
   deactivateDelayMS = 1000;
   thinkTimeMS       = 100;
   attackRadius      = 40.0;
   degPerSecTheta    = 45.0;
   degPerSecPhi      = 180.0;
   dontFireInsideDamageRadius = false;
   damageRadius = 0.0f;
   muzzleFlash = NULL;
   muzzleFlashID = 0;
}

TurretImageData::~TurretImageData()
{

}


//--------------------------------------------------------------------------
void TurretImageData::initPersistFields()
{
   Parent::initPersistFields();

   addField("activationMS",      TypeS32, Offset(activationMS,      TurretImageData));
   addField("deactivateDelayMS", TypeS32, Offset(deactivateDelayMS, TurretImageData));
   addField("thinkTimeMS",       TypeS32, Offset(thinkTimeMS,       TurretImageData));
   addField("degPerSecTheta",    TypeF32, Offset(degPerSecTheta,    TurretImageData));
   addField("degPerSecPhi",      TypeF32, Offset(degPerSecPhi,      TurretImageData));
   addField("attackRadius",      TypeF32, Offset(attackRadius,      TurretImageData));
   addField("damageRadius",      TypeF32, Offset(damageRadius,      TurretImageData));
   addField("dontFireInsideDamageRadius", TypeBool, Offset(damageRadius, TurretImageData));
   addField("muzzleFlash",       TypeShockwaveDataPtr, Offset(muzzleFlash,   TurretImageData));
}


//--------------------------------------------------------------------------
bool TurretImageData::onAdd()
{
   if(!Parent::onAdd())
      return false;

   if (activationMS < TickMs || activationMS > 5000) {
      Con::warnf(ConsoleLogEntry::General, "TurretImageData(%s)::onAdd: activationMS must be in the range [%d, 5000]", getName(), TickMs);
      activationMS = activationMS < TickMs ? TickMs : 5000;
   }
   activationMS = (activationMS >> 5) << 5;

   if (deactivateDelayMS < TickMs || deactivateDelayMS > 5000) {
      Con::warnf(ConsoleLogEntry::General, "TurretImageData(%s)::onAdd: deactivateDelayMS must be in the range [%d, 5000]", getName(), TickMs);
      deactivateDelayMS = deactivateDelayMS < TickMs ? TickMs : 5000;
   }
   deactivateDelayMS = (deactivateDelayMS >> 5) << 5;

   if (thinkTimeMS < TickMs || thinkTimeMS > 5000) {
      Con::warnf(ConsoleLogEntry::General, "TurretImageData(%s)::onAdd: thinkTimeMS must be in the range [%d, 5000]", getName(), TickMs);
      thinkTimeMS = thinkTimeMS < TickMs ? TickMs : 5000;
   }
   if (degPerSecTheta < 1.0f || degPerSecTheta > 1080.0f) {
      Con::warnf(ConsoleLogEntry::General, "TurretImageData(%s)::onAdd: degPerSecTheta must be in the range [1, 1080]", getName());
      degPerSecTheta = degPerSecTheta < 1.0 ? 1.0 : 1080.0f;
   }
   degPerSecTheta = U32(degPerSecTheta);

   if (degPerSecPhi < 1.0f || degPerSecPhi > 1080.0f) {
      Con::warnf(ConsoleLogEntry::General, "TurretImageData(%s)::onAdd: degPerSecPhi must be in the range [1, 1080]", getName());
      degPerSecPhi = degPerSecPhi < 1.0 ? 1.0 : 1080.0f;
   }
   degPerSecPhi = U32(degPerSecPhi);
   if (attackRadius < 10.0f || attackRadius > 1000) {
      Con::warnf(ConsoleLogEntry::General, "TurretImageData(%s)::onAdd: attackRadius must be in the range [10, 1000]", getName());
      attackRadius = attackRadius < 10.0 ? 10.0 : 1000.0f;
   }

   // Make activation time a multiple of a tick
   activationMS = (activationMS + TickMs - 1) & ~(TickMs - 1);

   // Make ThinkTime a multiple of a tick
   thinkTimeMS = (thinkTimeMS + TickMs - 1) & ~(TickMs - 1);

if( muzzleFlash )
{
   int test = 1;
}

   if (!muzzleFlash && muzzleFlashID) {
      if (!Sim::findObject( muzzleFlashID, muzzleFlash )) {
         Con::errorf( ConsoleLogEntry::General, "TurretImageData::onAdd: Invalid packet, bad datablockId(shockwave): 0x%x", muzzleFlashID );
      }
   }

   return true;
}

//--------------------------------------------------------------------------
void TurretImageData::packData(BitStream* stream)
{
   Parent::packData(stream);

   stream->writeInt(activationMS >> 5, 8);
   stream->writeInt(deactivateDelayMS >> 5, 8);
   stream->writeRangedU32(degPerSecTheta,0, 1080);
   stream->writeRangedU32(degPerSecPhi,0, 1080);
   stream->writeFlag(dontFireInsideDamageRadius);
   stream->write(damageRadius);

   if( stream->writeFlag( muzzleFlash ) )
   {
      stream->writeRangedU32( muzzleFlash->getId(), DataBlockObjectIdFirst,  DataBlockObjectIdLast );
   }
}

void TurretImageData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   activationMS = stream->readInt(8) << 5;
   deactivateDelayMS = stream->readInt(8) << 5;
   degPerSecTheta = stream->readRangedU32(0, 1080);
   degPerSecPhi = stream->readRangedU32(0, 1080);
   dontFireInsideDamageRadius = stream->readFlag();
   stream->read(&damageRadius);

   if( stream->readFlag() )
   {
      muzzleFlashID = (S32) stream->readRangedU32( DataBlockObjectIdFirst, DataBlockObjectIdLast );
   }
}


//--------------------------------------------------------------------------
//--------------------------------------
//
Turret::Turret()
{
   mNetFlags.set(Ghostable);
   mTypeMask |= TurretObjectType;

   mCurrState       = Dormant;
   mActivationLevel = csmFullyDeactivated;
   mCurrPhi         = csmPhiNull;
   mCurrTheta       = csmThetaNull;

   mActiveBase    = mActivationLevel;
   mPhiBase       = mCurrPhi;
   mThetaBase     = mCurrTheta;
   mActiveDelta   =
      mThetaDelta =
      mPhiDelta   = 0.0;

   mActivateThread = NULL;
   mElevateThread  = NULL;
   mTurnThread     = NULL;

   mCurrBarrel     = "";
   mSkillLevel     = 1.0f;
   mAutoFire       = true;

   mBarrelDamageThread = NULL;
   mDeployThread = NULL;
}

Turret::~Turret()
{
   //
}

//--------------------------------------------------------------------------
void Turret::initPersistFields()
{
   Parent::initPersistFields();

   addField("initialBarrel", TypeString, Offset(mCurrBarrel, Turret));
}


void Turret::consoleInit()
{
   Con::addCommand("Turret", "setSkill",           cTurretSetSkill,      "[Turret].setSkill(skill< 0 - 1 >)",        3, 3);
   Con::addCommand("Turret", "setTargetObject",    cTurretSetTarget,     "[Turret].setTargetObject(target id)",      3, 3);
   Con::addCommand("Turret", "clearTarget",        cTurretClearTarget,   "[Turret].clearTarget()",                   2, 2);
   Con::addCommand("Turret", "getTargetObject",    cTurretGetTarget,     "[Turret].getTargetObject()",               2, 2);
   Con::addCommand("Turret", "isValidTarget",      cTurretIsValidTarget, "[Turret].isValidTarget(target id)",        3, 3);
   Con::addCommand("Turret", "initiateBarrelSwap", cTurretBarrelReplace, "[Turret].initiateBarrelSwap(engineer id)", 3, 3);
   Con::addCommand("Turret", "setAutoFire",        cTurretSetAutoFire,   "[Turret].setAutoFire(bool)",               3, 3);
}

//--------------------------------------------------------------------------
bool Turret::onAdd()
{
   if(!Parent::onAdd())
      return false;

   if (isServerObject()) {
      // If there's a barrel, load it
      if (mCurrBarrel && mCurrBarrel[0] != '\0') {
         ShapeBaseImageData* pBarrel;
         if (Sim::findObject(mCurrBarrel, pBarrel) == true)
            mountImage(pBarrel, 0, false, 0);
      }
   }

   AssertFatal(mShapeInstance != NULL, "Turret::onAdd: must have a shape");
   AssertFatal((mDataBlock->activateSeq != -1 &&
                mDataBlock->elevateSeq != -1  &&
                mDataBlock->turnSeq != -1), "Turret::onAdd: must have a valid sequence list");

   mLastThink = 0;

   return true;
}


void Turret::onRemove()
{
   if (mActivateThread) {
      mShapeInstance->destroyThread(mActivateThread);
      mActivateThread = NULL;
   }
   if (mElevateThread) {
      mShapeInstance->destroyThread(mElevateThread);
      mElevateThread = NULL;
   }
   if (mTurnThread) {
      mShapeInstance->destroyThread(mTurnThread);
      mTurnThread = NULL;
   }

   Parent::onRemove();
}


bool Turret::onNewDataBlock(GameBaseData* dptr)
{
   mDataBlock = dynamic_cast<TurretData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr))
      return false;

   scriptOnNewDataBlock();
   return true;
}


//--------------------------------------------------------------------------
bool Turret::mountImage(ShapeBaseImageData* image,U32 imageSlot,bool loaded,S32 team)
{
   bool ret = Parent::mountImage(image, imageSlot, loaded, team);

   if (ret == true)
      mCurrBarrel = image->getName();

   return true;
}

bool Turret::unmountImage(U32 imageSlot)
{
   bool ret = Parent::unmountImage(imageSlot);

   if (ret == true)
      mCurrBarrel = "";
   return true;
}

void Turret::setImage(U32                 imageSlot,
                      ShapeBaseImageData* imageData,
                      U32  skinTag,
                      bool loaded,
                      bool ammo,
                      bool triggerDown,
                      bool target)
{
   if (isClientObject() && getMountedImage(imageSlot) != NULL && mBarrelDamageThread != NULL)
   {
      mMountedImageList[imageSlot].shapeInstance->destroyThread(mBarrelDamageThread);
      mBarrelDamageThread = NULL;
   }

   if (isClientObject() && getMountedImage(imageSlot) != NULL && mDeployThread != NULL)
   {
      mMountedImageList[imageSlot].shapeInstance->destroyThread(mDeployThread);
      mDeployThread = NULL;
   }

   Parent::setImage(imageSlot, imageData, skinTag, loaded, ammo, triggerDown, target);
   
   if (isClientObject() && mMountedImageList[imageSlot].shapeInstance != NULL)
   {
      S32 seq = mMountedImageList[imageSlot].shapeInstance->getShape()->findSequence("damage");
      if (seq != -1)
      {
         mBarrelDamageThread = mMountedImageList[imageSlot].shapeInstance->addThread();
         mMountedImageList[imageSlot].shapeInstance->setSequence(mBarrelDamageThread, seq, 0);
         updateDamageLevel();
      }
      seq = mMountedImageList[imageSlot].shapeInstance->getShape()->findSequence("deploy");
      if (seq != -1)
      {
         mDeployThread = mMountedImageList[imageSlot].shapeInstance->addThread();
         mMountedImageList[imageSlot].shapeInstance->setSequence(mDeployThread, seq, 0);
      }      
   }
}

//--------------------------------------------------------------------------
void Turret::updateDamageLevel()
{
   Parent::updateDamageLevel();
   if (mBarrelDamageThread) {
      // mDamage is already 0-1 on the client
      F32 level = mDamage / mDataBlock->destroyedLevel;
      if (level >= 1.0)
         level = 1.0;
      mMountedImageList[0].shapeInstance->setPos(mBarrelDamageThread,level);
   }
}

//--------------------------------------------------------------------------
void Turret::updateState(bool playerControlled)
{
   if (playerControlled) {
      mCurrTarget = NULL;

      if (mCurrState == Dormant) {
         mCurrState = Activating;
      } else if (mCurrState == Activating) {
         // Do nothing
      } else if (mCurrState == Deactivating) {
         if (mActivationLevel == csmFullyActivated) {
            if (!mElevateThread || !mTurnThread) {
               mElevateThread  = mShapeInstance->addThread();
               mTurnThread     = mShapeInstance->addThread();
               mShapeInstance->setSequence(mElevateThread,  mDataBlock->elevateSeq,  0);
               mShapeInstance->setSequence(mTurnThread,     mDataBlock->turnSeq,     0);
            }

            mCurrState = Active;
         } else {
            mCurrState = Activating;
         }
      } else if (mCurrState == DeactivateForReplace) {
         // Do nothing
      } 
   } else {
      if (getMountedImage(0) != NULL) {
         // Go to active state if we have a target, otherwise, wait for deactive
         //  delay to expire, and deactivate...
         //
         if (mCurrState == Active) {
            if (bool(mCurrTarget) == false && mTargetlessTime > getDeactivateDelay()) {
               // Deactivate
               mCurrState = Deactivating;
            }
         }
         else if (mCurrState == Activating) {
            // Do nothing, just wait
         }
         else if (mCurrState == Deactivating) {
            if (bool(mCurrTarget)) {
               if (mActivationLevel == csmFullyActivated) {
                  if (!mElevateThread || !mTurnThread) {
                     mElevateThread  = mShapeInstance->addThread();
                     mTurnThread     = mShapeInstance->addThread();
                     mShapeInstance->setSequence(mElevateThread,  mDataBlock->elevateSeq,  0);
                     mShapeInstance->setSequence(mTurnThread,     mDataBlock->turnSeq,     0);
                  }

                  mCurrState = Active;
               } else {
                  mCurrState = Activating;
               }
            }
         }
         else if (mCurrState == Dormant) {
            if (mElevateThread) {
               mShapeInstance->destroyThread(mElevateThread);
               mElevateThread = NULL;
            }
            if (mTurnThread) {
               mShapeInstance->destroyThread(mTurnThread);
               mTurnThread = NULL;
            }
            if (bool(mCurrTarget))
               mCurrState = Activating;
         } else {
            // Do nothing
         }
      } else {
         // Go to inactive state
         mCurrTarget = NULL;

         if (mCurrState == Dormant || mCurrState == Deactivating) {
            // Do nothing, just wait
            if (mElevateThread) {
               mShapeInstance->destroyThread(mElevateThread);
               mElevateThread = NULL;
            }
            if (mTurnThread) {
               mShapeInstance->destroyThread(mTurnThread);
               mTurnThread = NULL;
            }
         } else if (mCurrState == Activating) {
            mCurrState = Deactivating;
         } else {
            mCurrState = Deactivating;
         }
      }
   }
}
                                     
void Turret::processTick(const Move* move)
{
   Parent::processTick(move);

   if (isHidden() || mDataBlock->neverUpdateControl)
      return;

   bool playerControlled = (move != NULL);

   Move nullMove = { 0, 0, 0, 0, 0, 0 };
   if (move == NULL)
      move = &nullMove;

   if (isServerObject() || playerControlled) {
      if(mMount.object)
         setMaskBits(MountedUpdateMask);
      // Controlled path: Either a serverObject or a playerControled client object
      //                   if we're here.
      //
      if (isFrozen() == false) {
         updateState(playerControlled);

         // We might have to give the ai a chance to think before we allow it
         //  to update...
         if (!playerControlled && mAutoFire && getMountedImage(0) != NULL) {
            mLastThink += TickMs;
            if (mLastThink >= getThinkTime())
               aiThink();
         }

         if (mCurrState == Dormant) {
            // Just make sure...
            mActivationLevel = mActiveBase = 0.0;
            mCurrPhi         = mPhiBase    = csmPhiNull;
            mCurrTheta       = mThetaBase  = getThetaNull();
            mPhiDelta = mThetaDelta = mActiveDelta = 0.0;
         } 
         else if (mCurrState == Activating) {
            performActivateRamp();
            setMaskBits(BogoMask);
         } 
         else if (mCurrState == Deactivating) {
            performDeactivateRamp();
            setMaskBits(BogoMask);
         } 
         else if (mCurrState == Active) {
            // If we're not player controlled, the ai needs to think about
            //  this move...
            //
            F32 phiMove, thetaMove;
            if (playerControlled == false) {
               if (isServerObject())
                  aiUpdateActive(&nullMove);

               phiMove   = mFabs(move->yaw);
               thetaMove = mFabs(move->pitch);

               if (bool(mCurrTarget) == false) {
                  mTargetlessTime += TickMs;
               }
            } else {
               phiMove   = mFabs(move->yaw   * sgBigClunkyMultiplicationFactor);
               thetaMove = mFabs(move->pitch * sgBigClunkyMultiplicationFactor);
            }

            if (phiMove > getPhiSpeedPerTick())
               phiMove = getPhiSpeedPerTick();
            if (move->yaw < 0.0)
               phiMove *= -1.0f;
            if (thetaMove > getThetaSpeedPerTick())
               thetaMove = getThetaSpeedPerTick();
            if (move->pitch < 0.0)
               thetaMove *= -1.0f;

            // Setup the deltas...
            mCurrPhi    += phiMove;
            mCurrTheta  += thetaMove;

            mPhiBase     = mCurrPhi;
            mPhiDelta    = -phiMove;
            mThetaBase   = mCurrTheta;
            mThetaDelta  = -thetaMove;
            mActiveBase  = 1.0;
            mActiveDelta = 0.0;


            // Clamp phi to [0, 360)
            while (mCurrPhi >= 360.0f) mCurrPhi -= 360.0f;
            while (mCurrPhi < 0.0f)    mCurrPhi += 360.0f;

            // Clamp theta
            if (mCurrTheta > mDataBlock->thetaMax)
               mCurrTheta = mDataBlock->thetaMax;
            else if (mCurrTheta < mDataBlock->thetaMin)
               mCurrTheta = mDataBlock->thetaMin;

            setMaskBits(BogoMask);
         } 
         else if (mCurrState == DeactivateForReplace) {
            if (bool(mCurrEngineer)) {
               performDeactivateRamp();
               if (mCurrState == Dormant) {
                  // We just went dormant.  Let's see if we can replace the
                  //  barrel here...
                  checkReplace();
               }
            } else {
               mCurrState = Deactivating;
            }
         }

         if (mCurrState == Active) {
            setImageTriggerState(0, move->trigger[0]);
            setImageTriggerState(1, move->trigger[1]);
         } else {
            setImageTriggerState(0, false);
            setImageTriggerState(1, false);
         }
      } else {
         // If we're frozen, we do nothing but clear our current target (if any)
         mCurrTarget   = NULL;
         mCurrEngineer = NULL;
      }
      
      updateContainer();
      
      if( mWaterCoverage > 0 )
         setHeat(0);
      else
         setHeat(mDataBlock->heat);
   
   } else {
      // Non-player controlled client object if we're here...
      //
      mActiveBase = mActivationLevel;
      mPhiBase    = mCurrPhi;
      mThetaBase  = mCurrTheta;
      mPhiDelta   = mThetaDelta = mActiveDelta = 0.0;

      if (mActivationLevel == 1.0) {
         if (!mElevateThread || !mTurnThread) {
            mElevateThread  = mShapeInstance->addThread();
            mTurnThread     = mShapeInstance->addThread();
            mShapeInstance->setSequence(mElevateThread,  mDataBlock->elevateSeq,  0);
            mShapeInstance->setSequence(mTurnThread,     mDataBlock->turnSeq,     0);
         }
      } else {
         if (mElevateThread || mTurnThread) {
            mShapeInstance->destroyThread(mElevateThread);
            mShapeInstance->destroyThread(mTurnThread);
            mElevateThread  = NULL;
            mTurnThread     = NULL;
         }
      }
   }
   
   // Set current positional state...
   if (mActivationLevel != 0.0)
   {
      if (mActivateThread == NULL)
      {
         mActivateThread = mShapeInstance->addThread();
         mShapeInstance->setSequence(mActivateThread, mDataBlock->activateSeq, 0);
      }
      mShapeInstance->setPos(mActivateThread, mActivationLevel);
   }
   else
   {
      if (mActivateThread != NULL)
      {
         if (mActivateThread != NULL) {
            mShapeInstance->destroyThread(mActivateThread);
            mActivateThread = NULL;
         }
      }
   }

   setOrientationThreads(mCurrTheta, mCurrPhi);

   if (isServerObject() || playerControlled != NULL)
      mShapeInstance->animate();
}

//--------------------------------------------------------------------------
void Turret::advanceTime(F32 dt)
{
   Parent::advanceTime(dt);
   if(mDeployThread)
      mMountedImageList[0].shapeInstance->advanceTime((dt / 2.0f), mDeployThread);
}

//--------------------------------------------------------------------------
void Turret::interpolateTick(F32 delta)
{
   Parent::interpolateTick(delta);

   if (isHidden())
      return;

   F32 interpActive = mActiveBase + delta * mActiveDelta;
   F32 interpPhi    = mPhiBase    + delta * mPhiDelta;
   F32 interpTheta  = mThetaBase  + delta * mThetaDelta;

   while (interpPhi < 0.0)
      interpPhi += 360.0f;
   while (interpPhi >= 360.0f)
      interpPhi -= 360.0f;

   if (interpTheta < mDataBlock->thetaMin || interpTheta > mDataBlock->thetaMax)
      interpTheta = interpTheta < mDataBlock->thetaMin ? mDataBlock->thetaMin : mDataBlock->thetaMax;

   if (mActivateThread != NULL)
      mShapeInstance->setPos(mActivateThread, interpActive);
   setOrientationThreads(interpTheta, interpPhi);
   mShapeInstance->animate();
}


//--------------------------------------------------------------------------
//--------------------------------------
bool Turret::writePacketData(GameConnection *connection, BitStream* stream)
{
   bool ret = Parent::writePacketData(connection, stream);

   // Dump entire state
   stream->writeRangedU32(mCurrState, TurretFirstState, TurretLastState);
   stream->write(mActivationLevel);
   stream->write(mCurrPhi);
   stream->write(mCurrTheta);
   return ret;
}


void Turret::readPacketData(GameConnection *connection, BitStream* stream)
{
   Parent::readPacketData(connection, stream);

   mCurrState = (States)stream->readRangedU32(TurretFirstState, TurretLastState);
   stream->read(&mActivationLevel);
   stream->read(&mCurrPhi);
   stream->read(&mCurrTheta);
   mActiveBase    = mActivationLevel;
   mPhiBase       = mCurrPhi;
   mThetaBase     = mCurrTheta;
   mActiveDelta   =
      mThetaDelta =
      mPhiDelta   = 0.0;
}


//--------------------------------------------------------------------------
//--------------------------------------
U32 Turret::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   // The rest of the data is part of the control object packet update.
   // If we're controlled by this client, we don't need to send it.
   if(stream->writeFlag(getControllingClient() == con && !(mask & InitialUpdateMask)))
      return retMask;

   if(stream->writeFlag(mask & BogoMask))
   {
      // Otherwise, write our phi, theta, and activation levels
      F32 compressedPhi = mCurrPhi / 360.0f;
      F32 compressedTheta = (mCurrTheta - mDataBlock->thetaMin) /
         (mDataBlock->thetaMax - mDataBlock->thetaMin);

      stream->writeFloat(compressedPhi,    PhiBits);
      stream->writeFloat(compressedTheta,  ThetaBits);
      stream->writeFloat(mActivationLevel, ActivationBits);
   }
   return retMask;
}

void Turret::unpackUpdate(NetConnection* con, BitStream* stream)
{
   Parent::unpackUpdate(con, stream);

   // If we're not the controlling object, then we just return...
   if(stream->readFlag())
      return;

   if(stream->readFlag())
   {
      F32 newPhi        = stream->readFloat(PhiBits);
      F32 newTheta      = stream->readFloat(ThetaBits);
      F32 newActivation = stream->readFloat(ActivationBits);
      newPhi *= 360.0f;
      newTheta *= (mDataBlock->thetaMax - mDataBlock->thetaMin);
      newTheta += mDataBlock->thetaMin;

      updateWarp(newPhi, newTheta, newActivation);
   }
}


//--------------------------------------------------------------------------
void Turret::setOrientationThreads(F32 theta, F32 phi)
{
   AssertFatal(theta >= mDataBlock->thetaMin && theta <= mDataBlock->thetaMax, "Turret::setOrientation theta out of range");

   if (theta < mDataBlock->thetaMin || theta > mDataBlock->thetaMax)
      theta = theta < mDataBlock->thetaMin ? mDataBlock->thetaMin : mDataBlock->thetaMax;

   while (phi >= 360.0f)
      phi -= 360.0f;
   while (phi < 0.0f)
      phi += 360.0f;

   F32 thetaPos = theta / 180.0f;
   F32 phiPos   = phi   / 360.0f;

   if (mTurnThread)
      mShapeInstance->setPos(mTurnThread, phiPos);
   if (mElevateThread)
      mShapeInstance->setPos(mElevateThread, thetaPos);
}

F32 Turret::getPhiSpeed()
{
   ShapeBaseImageData* pSBID = getMountedImage(0);
   if (pSBID == NULL)
      return csmDefaultPhiSpeed;
   AssertFatal(dynamic_cast<TurretImageData*>(pSBID) != NULL, "Error, non-turretimage mounted on a turret base!");

   return static_cast<TurretImageData*>(pSBID)->degPerSecPhi;
}

F32 Turret::getThetaSpeed()
{
   ShapeBaseImageData* pSBID = getMountedImage(0);
   if (pSBID == NULL)
      return csmDefaultThetaSpeed;
   AssertFatal(dynamic_cast<TurretImageData*>(pSBID) != NULL, "Error, non-turretimage mounted on a turret base!");

   return static_cast<TurretImageData*>(pSBID)->degPerSecTheta;
}

F32 Turret::getActivationSpeed()
{
   ShapeBaseImageData* pSBID = getMountedImage(0);
   if (pSBID == NULL)
      return csmDefaultActivationSpeed;
   AssertFatal(dynamic_cast<TurretImageData*>(pSBID) != NULL, "Error, non-turretimage mounted on a turret base!");

   return 1000.0f / (static_cast<TurretImageData*>(pSBID)->activationMS);
}

U32 Turret::getDeactivateDelay()
{
   ShapeBaseImageData* pSBID = getMountedImage(0);
   if (pSBID == NULL)
      return csmDefaultDeactivateDelay;
   AssertFatal(dynamic_cast<TurretImageData*>(pSBID) != NULL, "Error, non-turretimage mounted on a turret base!");

   return static_cast<TurretImageData*>(pSBID)->deactivateDelayMS;
}

U32 Turret::getThinkTime()
{
   ShapeBaseImageData* pSBID = getMountedImage(0);
   if (pSBID == NULL)
      return csmDefaultThinkTime;
   AssertFatal(dynamic_cast<TurretImageData*>(pSBID) != NULL, "Error, non-turretimage mounted on a turret base!");

   return static_cast<TurretImageData*>(pSBID)->thinkTimeMS;
}

F32 Turret::getAttackRadius()
{
   ShapeBaseImageData* pSBID = getMountedImage(0);
   if (pSBID == NULL)
      return csmDefaultAttackRadius;
   AssertFatal(dynamic_cast<TurretImageData*>(pSBID) != NULL, "Error, non-turretimage mounted on a turret base!");

   return static_cast<TurretImageData*>(pSBID)->attackRadius;
}


F32 Turret::getPhiSpeedPerTick()
{
   return getPhiSpeed() * (TickMs / 1000.0f);
}

F32 Turret::getThetaSpeedPerTick()
{
   return getThetaSpeed() * (TickMs / 1000.0f);
}

F32 Turret::getActivationSpeedPerTick()
{
   return getActivationSpeed() * (TickMs / 1000.0f);
}

bool Turret::isFrozen() const
{
   return mDamageState != Enabled;
}

F32 Turret::getThetaNull() const
{

   if (mDataBlock && mDataBlock->thetaNull != -1)
      return mDataBlock->thetaNull;
   return
      csmThetaNull;
}

//--------------------------------------------------------------------------
void Turret::setSkill(F32 skill)
{
   (skill <= 1 && skill > 0) ? (mSkillLevel = skill) : (mSkillLevel = 1.0f); 
}

bool Turret::setTarget(ShapeBase* newTarget)
{
   AssertFatal(isServerObject(), "Error, clients may not have targets right now");

   mCurrTarget     = newTarget;
   mTargetlessTime = 0;
   return bool(mCurrTarget);
}

S32 Turret::getTargetId() const
{
   if (bool(mCurrTarget) == false)
      return 0;
   return mCurrTarget->getId();
}

bool Turret::currTargetValid()
{
   if (bool(mCurrTarget) == false)
      return false;

   return isValidTarget(mCurrTarget);
}


bool Turret::isValidTarget(ShapeBase* target)
{
   if (target == NULL)
      return false;

   // First, let's see if this target is dead...
   if (target->getDamageValue() >= 1.0)
      return false;

   // this and target need to have an associated 'target'
   if(!target->getTargetInfo() || !getTargetInfo())
      return false;

   // If we are loaded with seeking projectiles, then we need
   //  to check to see if the target is hot enough to kill
   if (getMountedImage(0) && getMountedImage(0)->isSeeker == true)
   {
      if (target->getHeat() < getMountedImage(0)->minSeekHeat)
         return false;
   }
   
   // information we'll need for a bunch of checks
   Point3F centerBox;
   F32 distToTarget;
   target->getWorldBox().getCenter(&centerBox);
   Point3F myPos;
   getTransform().getColumn(3, &myPos);
   distToTarget = (myPos - centerBox).len();

   // make sure we have a fire condition before continuing
   if( getMountedImage(0) )
   {
      TurretImageData *tData = static_cast<TurretImageData*>(getMountedImage(0));      
      
      if( tData )
      {
         // see if we have to be at least the damage radius from the target
         if( tData->dontFireInsideDamageRadius )
            if( distToTarget <= tData->damageRadius )
               return false;
      
         // see if we need to be a certain distance from the target
         ShapeBaseImageData *sData = static_cast<ShapeBaseImageData*>(tData);
         if( sData->targetingDist )
         {
            if( distToTarget <= sData->targetingDist)
               return false;
         }      
      }      
   }
   
   // Query sensor network
   // - target needs to be visible to turret
   // - turret cannot be friendly to targets sensor group
   if(!gTargetManager->isTargetVisible(target->getTarget(), getSensorGroup()) ||
      gTargetManager->isTargetFriendly(getTarget(), target->getSensorGroup()))
      return false;

   // Let's see if the target is outside of our attack range...
   MatrixF mountTransform;
   getMuzzleTransform(0, &mountTransform);
   Point3F mountPoint;
   mountTransform.getColumn(3, &mountPoint);
   Point3F targetPoint;
   target->getWorldBox().getCenter(&targetPoint);

   if ((targetPoint - mountPoint).len() > getAttackRadius())
      return false;
      
   target->disableCollision();
   disableCollision();
   RayInfo rinfo;
   
   if (gServerContainer.castRay(mountPoint, targetPoint, csmActiveScanMask, &rinfo) == true) {
      // Can't see the center of our target.  we're going to deactivate...
      target->enableCollision();
      enableCollision();
      return false;
   }
   
   target->enableCollision();
   enableCollision();
   return true;
}


void Turret::selectTarget()
{
   // Call out to script to set the target, if possible...
   //
   Con::executef(mDataBlock, 2, "selectTarget", scriptThis());
}

void Turret::setAutoFire(bool status)
{
   mAutoFire = status;
   if (! mAutoFire)
      setTarget(NULL);
}


void Turret::checkReplace()
{
   AssertFatal(bool(mCurrEngineer), "Shouldn't be here if our engineer is dead...");

   Con::executef(mDataBlock, 3, "replaceCallback", scriptThis(), mCurrEngineer->scriptThis());
   mCurrEngineer = NULL;
}

bool Turret::initiateReplace(ShapeBase* engineer)
{
   if (engineer == NULL)
      return false;

   mCurrEngineer = engineer;
   if (getMountedImage(0) == false || mCurrState == Dormant) {
      checkReplace();
      Con::errorf(ConsoleLogEntry::General, "instant replace");
   } else {
      mCurrState = DeactivateForReplace;
      Con::errorf(ConsoleLogEntry::General, "active replace");
   }

   return true;
}

void Turret::aiThink()
{
   mLastThink = 0;

   // First, if we have a target, let's see if the target is still valid
   if (bool(mCurrTarget)) {
      if (currTargetValid() == false) {
         // Call out to select a new target, if possible...
         mCurrTarget = NULL;
         selectTarget();
      } else {
         // Ok, we can still kill this guy, no need to worry
      }
   } else {
      // No target.  Select one...
      selectTarget();
   }

   // Let's see if we still have a target...
   //
   if (bool(mCurrTarget)) {
      // Ok, kill!  Let's activate ourselves if necessary.
      if (mCurrState == Dormant) {
         mCurrState = Activating;
      }
      else if (mCurrState == Activating) {
         // Just be patient.
      }
      else if (mCurrState == Active) {
         // Right where we want to be.
      }
      else if (mCurrState == Deactivating) {
         if (mActivationLevel == csmFullyActivated) {
            mCurrState = Active;
         } else {
            mCurrState = Activating;
         }
      }
      else if (mCurrState == DeactivateForReplace) {
         // Do nothing.  We're swapping out...
      }
   } else {
      // No target.  Since we had a chance to select one above, we're all done here.
      //
   }
}

Point3F Turret::dopeAim(const Point3F &startLocation, const Point3F &aimLocation)
{
   static MRandomLCG  rand(Sim::getCurrentTime());
   
   //find the "horizontal" orthogonal vector
   Point3F horzOrth;
   if (! findCrossVector(startLocation - aimLocation, Point3F(0, 0, 1), &horzOrth))
      return aimLocation;
      
   //now find the "vertical" orthogonal vector
   Point3F vertOrth;
   if (! findCrossVector(startLocation - aimLocation, horzOrth, &vertOrth))
      return aimLocation;
      
   //now we have the horizonal and vertical components of the plane which is perpendicular to
   //the direction we are firing...

   //determine the radius factor
   F32 radiusFactor;
   if (mSkillLevel == 1.0f)
      radiusFactor = 0.0f;
   else
      radiusFactor = 0.04 + (1.0f - mSkillLevel) * 0.2;
      
   //calculate the radius error
   F32 radiusError = (startLocation - aimLocation).len() * radiusFactor * 0.1;
   F32 horzError = rand.randF() * radiusError * (rand.randF() < 0.5f ? -1.0f : 1.0f);
   F32 vertError = rand.randF() * radiusError * (rand.randF() < 0.5f ? -1.0f : 1.0f);
   
   Point3F dopedAimLocation = aimLocation + (horzOrth * horzError) + (vertOrth * vertError);
   return dopedAimLocation;
}


Point3F gbogo;
void Turret::aiUpdateActive(Move* move)
{
   if (bool(mCurrTarget)) {
      // Let's find the vector we need to follow to hit this target...
      //
      MatrixF mountTransform;
      getMuzzleTransform(0, &mountTransform);
      Point3F mountPoint;
      mountTransform.getColumn(3, &mountPoint);
      Point3F targetPoint;
      mCurrTarget->getWorldBox().getCenter(&targetPoint);
      gbogo = targetPoint;

      // Can we see the target?
      disableCollision();
      mCurrTarget->disableCollision();
      RayInfo rinfo;
      
      // only used by bots - when a player mounts, skill should be set to 1.0 (from script)
      if (mSkillLevel < 1.0f)
         targetPoint = dopeAim(mountPoint, targetPoint);
      
      if (gServerContainer.castRay(mountPoint, targetPoint, csmActiveScanMask, &rinfo) == true) {
         // Can't see the center of our target.  we're going to deactivate...
         mCurrTarget->enableCollision();
         enableCollision();

         mTargetlessTime = 0;
         mCurrTarget     = NULL;
         return;
      }
      mCurrTarget->enableCollision();
      enableCollision();

      ShapeBaseImageData* pSBID = getMountedImage(0);
      AssertFatal(pSBID && dynamic_cast<TurretImageData*>(pSBID), "This shouldn't happen to an AI controlled turret...");
      TurretImageData* pTID = static_cast<TurretImageData*>(pSBID);
      AssertFatal(pTID->projectile != NULL, "Error, must have a projectile!");

      Point3F dirMin, dirMax;
      F32 timeMin, timeMax;
      bool allowFire;
      if (pTID->projectile->calculateAim(targetPoint,
                                         mCurrTarget->getVelocity(),
                                         mountPoint, Point3F(0, 0, 0), &dirMin, &timeMin, &dirMax, &timeMax)) {
         // Yay.
         allowFire = true;
      } else {
         // Can't hit it.  Point somewhere near it...
         dirMin = targetPoint - mountPoint;
         allowFire = false;
      }

      // Let's translate this into our own coordinate system...
//      mWorldToObj.mulV(dirMin);
      MatrixF mat = getTransform();
      mat.setColumn(3, mountPoint);
      mat.affineInverse();
      mat.mulV(dirMin);
      dirMin.normalize();

      // Ok, now we need to derive a phi/theta angle to point this way.
      F32 thetaLen;
      F32 newPhi;
      F32 newTheta;
      if (mDataBlock->primaryAxis == TurretData::ZAxis)
      {
         thetaLen = mSqrt(dirMin.x*dirMin.x + dirMin.y*dirMin.y);
         newPhi   = mAtan(dirMin.x, dirMin.y) * (360 / (2 * M_PI));
         newTheta = 180 - (mAtan(dirMin.z, thetaLen) * (90/(M_PI/2.0)) + 90.0);
      }
      else if (mDataBlock->primaryAxis == TurretData::YAxis)
      {
         thetaLen = mSqrt(dirMin.x*dirMin.x + dirMin.z*dirMin.z);
         newPhi   = mAtan(-dirMin.x, dirMin.z) * (360 / (2 * M_PI));
         newTheta = 180 - (mAtan(dirMin.y, thetaLen) * (90/(M_PI/2.0)) + 90.0);
      }
      else if (mDataBlock->primaryAxis == TurretData::RevZAxis)
      {
         thetaLen = mSqrt(dirMin.x*dirMin.x + dirMin.y*dirMin.y);
         newPhi   = 360 - (mAtan(dirMin.x, dirMin.y) * (360 / (2 * M_PI)));
         newTheta = mAtan(dirMin.z, thetaLen) * (90/(M_PI/2.0)) + 90.0;
      }
      else if (mDataBlock->primaryAxis == TurretData::RevYAxis)
      {
         thetaLen = mSqrt(dirMin.x*dirMin.x + dirMin.z*dirMin.z);
         newPhi   = mAtan(-dirMin.x, dirMin.z) * (360 / (2 * M_PI));
         newTheta = 180 - (mAtan(dirMin.y, thetaLen) * (90/(M_PI/2.0)) + 90.0);
      }
      else
      {
         AssertFatal(false, "Invalid turret primary axis!");
      }

      // Lets twiddle newPhi
      if (newPhi < 0.0f)
         newPhi += 360.0f;
      if (mFabs((newPhi - 360) - mCurrPhi) < mFabs(newPhi - mCurrPhi)) {
         newPhi -= 360.0f;
      } else if (mFabs((newPhi + 360) - mCurrPhi) < mFabs(newPhi - mCurrPhi)) {
         newPhi += 360.0f;
      }

      move->yaw   = newPhi - mCurrPhi;
      move->pitch = newTheta - mCurrTheta;

      // Determine whether or not we fire...
      bool fire = false;
      if (allowFire && mFabs(move->yaw) < 0.5 && mFabs(move->pitch) < 0.5) {
         fire = true;
      }
      move->trigger[0] = fire;
      move->trigger[1] = fire;
   } else {
      // Do nothing for right now.  Possibly think out to script later
      //  Wait for deactivate
      move->trigger[0] = false;
      move->trigger[1] = false;
   }
}

void Turret::getMuzzleVector(U32 imageSlot,VectorF* vec)
{
   
   if (GameConnection * gc = getControllingClient())
   {
      if (gc->isAIControlled() == false)
      {
         if (gc->isFirstPerson() == true)
         {
            MatrixF mat;
            getMuzzleTransform(imageSlot,&mat);
            if (getCorrectedAim(mat, vec))
            {
               return;
            }
         }
      }
   }
            
   Point3F vector;
   if (mDataBlock->primaryAxis == TurretData::ZAxis)
   {
      F32 len = mSin(mDegToRad(mCurrTheta));

      vector.x = mSin(mDegToRad(mCurrPhi)) * len;
      vector.y = mCos(mDegToRad(mCurrPhi)) * len;
      vector.z = mCos(mDegToRad(mCurrTheta));

      *vec = vector;
   }
   else if (mDataBlock->primaryAxis == TurretData::YAxis)
   {
      F32 len = mSin(mDegToRad(mCurrTheta));

      vector.x = -mSin(mDegToRad(mCurrPhi)) * len;
      vector.y = mCos(mDegToRad(mCurrTheta));
      vector.z = mCos(mDegToRad(mCurrPhi)) * len;
      *vec = vector;
   }
   else if (mDataBlock->primaryAxis == TurretData::RevZAxis)
   {
      F32 len = mSin(mDegToRad(mCurrTheta));

      vector.x = mSin(mDegToRad(360 - mCurrPhi)) * len;
      vector.y = mCos(mDegToRad(360 - mCurrPhi)) * len;
      vector.z = mCos(mDegToRad(180 - mCurrTheta));
      *vec = vector;
   }
   else if (mDataBlock->primaryAxis == TurretData::RevYAxis)
   {
      F32 len = mSin(mDegToRad(mCurrTheta));

      vector.x = mSin(mDegToRad(mCurrPhi)) * len;
      vector.y = -mCos(mDegToRad(mCurrTheta));
      vector.z = -mCos(mDegToRad(mCurrPhi)) * len;
      *vec = vector;
   }
   
   getTransform().mulV(*vec);
}


//--------------------------------------------------------------------------
void Turret::performActivateRamp()
{
   setImageLoadedState(0, true);
   mActivationLevel += getActivationSpeedPerTick();
   if (mActivationLevel >= csmFullyActivated) {
      if (!mElevateThread || !mTurnThread) {
         mElevateThread  = mShapeInstance->addThread();
         mTurnThread     = mShapeInstance->addThread();
         mShapeInstance->setSequence(mElevateThread,  mDataBlock->elevateSeq,  0);
         mShapeInstance->setSequence(mTurnThread,     mDataBlock->turnSeq,     0);
      }

      mActivationLevel = csmFullyActivated;
      mCurrState       = Active;
   }

   // Setup the deltas
   mPhiBase     = csmPhiNull;
   mPhiDelta    = 0.0;
   mThetaBase   = getThetaNull();
   mThetaDelta  = 0.0;
   mActiveBase  = mActivationLevel;
   mActiveDelta = 0.0;
}

void Turret::performDeactivateRamp()
{
   bool rampDownActive = true;

   // Setup the deltas...
   // Check the phi angle.  This rotates, which is annoying, since we want the most
   //  efficient spin down.
   AssertFatal(csmPhiNull == 0.0f, "Error, must rewrite this if phiNull != 0");
   if (mCurrPhi != 0.0f) {
      rampDownActive = false;
      F32 speed = getPhiSpeed() * (TickMs / 1000.0f);

      if (mCurrPhi > 180.0f) {
         mCurrPhi += speed;
         mPhiDelta = -speed;
      }
      else {
         mCurrPhi -= speed;
         mPhiDelta = speed;
      }
      mPhiBase = mCurrPhi;
   
      if (mCurrPhi >= 360.0f ||
          mCurrPhi <= 0.0f) {
         mCurrPhi = 0.0f;
      }
   } else {
      mPhiBase  = mCurrPhi;
      mPhiDelta = 0.0;
   }

   // Check the theta angle.  This is much easier
   if (mCurrTheta != csmThetaNull) {
      rampDownActive = false;
      F32 speed = getThetaSpeed() * (TickMs / 1000.0f);

      F32 dir = (csmThetaNull - mCurrTheta) / mFabs(csmThetaNull - mCurrTheta);

      mCurrTheta += dir * speed;
      mThetaDelta = -dir*speed;
      mThetaBase  = mCurrTheta;

      if ((dir < 0.0 && mCurrTheta <= csmThetaNull) ||
          (dir > 0.0 && mCurrTheta >= csmThetaNull))
         mCurrTheta = csmThetaNull;
   } else {
      mThetaDelta = 0.0;
   }

   if (rampDownActive == true) {
      if (mElevateThread || mTurnThread) {
         mShapeInstance->destroyThread(mElevateThread);
         mShapeInstance->destroyThread(mTurnThread);
         mElevateThread = NULL;
         mTurnThread    = NULL;
      }

      // TODO: Activate time from datablock
      setImageLoadedState(0, false);
   
      mActivationLevel -= getActivationSpeedPerTick();

      mActiveDelta = getActivationSpeedPerTick();
      mActiveBase = mActivationLevel;

      if (mActivationLevel <= csmFullyDeactivated) {
         mActivationLevel = csmFullyDeactivated;
         mCurrState = Dormant;
      }
   } else {
      mActiveBase = mActivationLevel;
      mActiveDelta = 0.0;
   }
}

//--------------------------------------------------------------------------
void Turret::updateWarp(const F32 phi, const F32 theta, const F32 active)
{
   // First, we have to derive the exact current position
   F32 currPhi    = mPhiBase    + mLastDelta * mPhiDelta;
   F32 currTheta  = mThetaBase  + mLastDelta * mThetaDelta;
   F32 currActive = mActiveBase + mLastDelta * mActiveDelta;

   mCurrPhi         = phi;
   mCurrTheta       = theta;
   mActivationLevel = active;
}

