//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _H_TURRET
#define _H_TURRET

#ifndef _STATICSHAPE_H_
#include "game/staticShape.h"
#endif

class ShockwaveData;

// -------------------------------------------------------------------------
class TurretData : public StaticShapeData
{
   typedef StaticShapeData Parent;

  protected:
   bool preload(bool server, char errorBuffer[256]);

   //-------------------------------------- Console set variables
  public:
   enum PrimaryAxis {
      YAxis,
      RevYAxis,
      ZAxis,
      RevZAxis,

      __FirstValidAxis = YAxis,
      __LastValidAxis  = RevZAxis
   };
   
   F32      thetaMin;
   F32      thetaMax;
   F32      thetaNull;

   PrimaryAxis primaryAxis;
   
   bool     neverUpdateControl;

   //-------------------------------------- load set variables
  public:
   S32      activateSeq;
   S32      elevateSeq;
   S32      turnSeq;

  public:
   TurretData();
   ~TurretData();

   void packData(BitStream*);
   void unpackData(BitStream*);

   DECLARE_CONOBJECT(TurretData);
   static void initPersistFields();
};

class TurretImageData : public ShapeBaseImageData
{
   typedef ShapeBaseImageData Parent;

  protected:
   bool onAdd();

   //-------------------------------------- Console set variables
  public:
   S32   activationMS;
   S32   deactivateDelayMS;
   S32   thinkTimeMS;

   F32   degPerSecTheta;
   F32   degPerSecPhi;

   F32   attackRadius;

   bool  dontFireInsideDamageRadius;
   F32   damageRadius;
   
   ShockwaveData *   muzzleFlash;
   S32               muzzleFlashID;
   
   //-------------------------------------- load set variables
  public:

  public:
   TurretImageData();
   ~TurretImageData();

   void packData(BitStream*);
   void unpackData(BitStream*);

   DECLARE_CONOBJECT(TurretImageData);
   static void initPersistFields();
};



// -------------------------------------------------------------------------
class TSThread;
class Turret : public StaticShape
{
   typedef StaticShape Parent;

  private:
   TurretData* mDataBlock;

  protected:
   enum States {
      Dormant              = 0,
      Activating           = 1,
      Deactivating         = 2,
      Active               = 3,
      DeactivateForReplace = 4,

      TurretFirstState     = Dormant,
      TurretLastState      = DeactivateForReplace
   };
   enum Constants {
      TurretWarpTicks = 10,

      PhiBits         = 10,
      ThetaBits       = 10,
      ActivationBits  = 8
   };
   enum TurretNetMasks {
      MountedUpdateMask = Parent::NextFreeMask,
      BogoMask     = MountedUpdateMask << 1,
      NextFreeMask = BogoMask << 2
   };

   static const U32 csmActiveScanMask;

   static const U32 csmDefaultDeactivateDelay;
   static const U32 csmDefaultThinkTime;
   static const F32 csmDefaultAttackRadius;
   static const F32 csmFullyDeactivated;
   static const F32 csmFullyActivated;
   static const F32 csmPhiNull;
   static const F32 csmThetaNull;

   // Used to deactivate if there is no image mounted
   static const F32 csmDefaultActivationSpeed;
   static const F32 csmDefaultPhiSpeed;
   static const F32 csmDefaultThetaSpeed;

   // Server data, replicated to client through writePacketData
  protected:
   States   mCurrState;
   F32      mActivationLevel;       // 0->completelyDeactivated, 1->completelyActivated
   F32      mCurrPhi;               // Both in degrees.  Phi   NULL == 0
   F32      mCurrTheta;             //                   Theta NULL == 90
   F32      mSkillLevel;

   StringTableEntry mCurrBarrel;

   // Server data/functions used by ai to track target
  protected:
   SimObjectPtr<ShapeBase> mCurrTarget;    // NULL == no target
   U32                     mTargetlessTime;
   U32                     mLastThink;
   bool			   mAutoFire;

  public:
   void setSkill(F32 skill);
   bool setTarget(ShapeBase*);
   S32  getTargetId() const;
   bool currTargetValid();
   bool isValidTarget(ShapeBase*);
   bool initiateReplace(ShapeBase*);
   void setAutoFire(bool status);
   void advanceTime(F32 dt);
  
   // Replacing object...
  protected:
   SimObjectPtr<ShapeBase> mCurrEngineer;

   // Animation thread data
  protected:
   TSThread* mActivateThread;
   TSThread* mElevateThread;
   TSThread* mTurnThread;

   TSThread* mBarrelDamageThread;
   TSThread* mDeployThread;
   
   // Client side interpolation data
  protected:
   F32      mPhiBase;
   F32      mThetaBase;
   F32      mActiveBase;
   F32      mPhiDelta;
   F32      mThetaDelta;
   F32      mActiveDelta;

   // Convenience functions...
  protected:
   void setOrientationThreads(F32 theta, F32 phi);
   bool isFrozen() const;

   F32  getPhiSpeed();
   F32  getThetaSpeed();
   F32  getActivationSpeed();
   U32  getDeactivateDelay();
   U32  getThinkTime();
   F32  getAttackRadius();

   F32  getThetaNull() const;

   F32  getPhiSpeedPerTick();
   F32  getThetaSpeedPerTick();
   F32  getActivationSpeedPerTick();

   void performActivateRamp();
   void performDeactivateRamp();

   void selectTarget();
   void checkReplace();

   bool mountImage(ShapeBaseImageData* image,U32 imageSlot,bool loaded,S32 team);
   bool unmountImage(U32 imageSlot);
   virtual void setImage(U32 imageSlot, ShapeBaseImageData* imageData, U32 teamTag,
                         bool loaded, bool ammo, bool triggerDown,
                         bool target);

   // Control functions
  protected:
   void updateState(bool isPlayerControlled);
   void updateWarp(const F32, const F32, const F32);

   // AI functions
  protected:
   void aiUpdateActive(Move* move);
   void aiThink();
   Point3F dopeAim(const Point3F &startLocation, const Point3F &aimLocation);

  protected:
   bool onAdd();
   void onRemove();
   bool onNewDataBlock(GameBaseData*);
   virtual void updateDamageLevel();

  public:
   Turret();
   ~Turret();

   DECLARE_CONOBJECT(Turret);
   static void initPersistFields();
   static void consoleInit();

   void processTick(const Move*);
   void interpolateTick(F32 delta);

   void getMuzzleVector(U32 imageSlot,VectorF* vec);

   bool writePacketData(GameConnection *, BitStream*);
   void readPacketData(GameConnection *, BitStream*);
   U32  packUpdate(NetConnection*, U32 mask, BitStream* stream);
   void unpackUpdate(NetConnection*, BitStream* stream);
};

#endif // _H_TURRET

