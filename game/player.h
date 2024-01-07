//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _PLAYER_H_
#define _PLAYER_H_

#ifndef _SHAPEBASE_H_
#include "game/shapeBase.h"
#endif
#ifndef _BOXCONVEX_H_
#include "collision/boxConvex.h"
#endif

class ParticleEmitter;
class ParticleEmitterData;
class DecalData;
class SplashData;

//----------------------------------------------------------------------------

struct PlayerData: public ShapeBaseData {
   typedef ShapeBaseData Parent;
   enum Constants {
      RecoverDelayBits = 7,
      JumpDelayBits = 7,
      NumSpineNodes = 6,
      ImpactBits = 3,
      NUM_SPLASH_EMITTERS = 3,
      BUBBLE_EMITTER = 2,
   };
   F32 pickupRadius;          // Radius around player for items (on server)
   F32 maxTimeScale;          // Max timeScale for action animations

   F32 minLookAngle;
   F32 maxLookAngle;
   F32 maxFreelookAngle;

   F32 runForce;              // Force used to accelerate player 
   F32 runEnergyDrain;        // Energy drain/tick
   F32 minRunEnergy;
   F32 maxForwardSpeed;
   F32 maxBackwardSpeed;
   F32 maxSideSpeed;
   F32 maxUnderwaterForwardSpeed;
   F32 maxUnderwaterBackwardSpeed;
   F32 maxUnderwaterSideSpeed;

   F32 maxStepHeight;
   F32 runSurfaceAngle;       // Angle from vertical in degrees

   F32 horizMaxSpeed;
   F32 horizResistSpeed;
   F32 horizResistFactor;

   F32 upMaxSpeed;
   F32 upResistSpeed;
   F32 upResistFactor;

   S32 recoverDelay;          // # tick
   F32 recoverRunForceScale;  // RunForce multiplier in recover state

   F32 jumpForce;
   F32 jumpEnergyDrain;       // Energy per jump
   F32 minJumpEnergy;
   F32 minJumpSpeed;
   F32 maxJumpSpeed;
   F32 jumpSurfaceAngle;      // Angle from vertical in degrees
   S32 jumpDelay;             // Delay time in ticks
   
   F32 boxHeadPercentage;
   F32 boxTorsoPercentage;
   
   S32 boxHeadLeftPercentage;
   S32 boxHeadRightPercentage;
   S32 boxHeadBackPercentage;
   S32 boxHeadFrontPercentage;

   F32 minImpactSpeed;

   F32 decalOffset;

   F32               groundImpactMinSpeed;
   VectorF           groundImpactShakeFreq;
   VectorF           groundImpactShakeAmp;
   F32               groundImpactShakeDuration;
   F32               groundImpactShakeFalloff;

   enum Sounds {
      LFootSoft,
      RFootSoft,
      LFootHard,
      RFootHard,
      LFootMetal,
      RFootMetal,
      LFootSnow,
      RFootSnow,
      LFootShallowSplash,
      RFootShallowSplash,
      LFootWading,
      RFootWading,
      LFootUnderWater,
      RFootUnderWater,
      LFootBubbles,
      RFootBubbles,
      MoveBubbles,
      WaterBreath,
      ImpactSoft,
      ImpactHard,
      ImpactMetal,
      ImpactSnow,
      ImpactWaterEasy,
      ImpactWaterMedium,
      ImpactWaterHard,
      ExitWater,
      MaxSounds
   };
   AudioProfile* sound[MaxSounds];

   Point3F boxSize;           // Width, depth, heigth

   // Animation and other data intialized in onAdd
   struct ActionAnimationDef {
      const char* name;       // Sequence name
   };
   struct ActionAnimation {
      const char* name;       // Sequence name
      S32      sequence;
      bool     death;
      VectorF  dir;           // Dir of animation ground transform
      F32      speed;         // Speed in m/s
   };
   enum {
      // *** WARNING ***
      // These enum values are used to index the ActionAnimationList
      // array instantiated in player.cc
      // The first five are selected in the move state based on velocity
      RootAnim,
      RunForwardAnim,
      BackBackwardAnim,
      SideLeftAnim,

      // These are set explicitly based on player actions
      FallAnim,
      JumpAnim,
      LandAnim,

      //
      NumMoveActionAnims = SideLeftAnim + 1,
      NumTableActionAnims = LandAnim + 1,
      NumExtraActionAnims = 60,
      NumActionAnims = NumTableActionAnims + NumExtraActionAnims,
      ActionAnimBits = 8,
      NullAnimation = (1 << ActionAnimBits) - 1
   };

   static ActionAnimationDef ActionAnimationList[NumTableActionAnims];
   ActionAnimation actionList[NumActionAnims];
   U32 actionCount;
   U32 lookAction;
   U32 skiAction;
   U32 standJumpAction;
   S32 spineNode[NumSpineNodes];
   S32 pickupDelta;           // Base off of pcikupRadius
   F32 runSurfaceCos;         // Angle from vertical in cos(runSurfaceAngle)
   F32 jumpSurfaceCos;        // Angle from vertical in cos(jumpSurfaceAngle)

   enum Impacts {
      ImpactNone, 
      ImpactNormal, 
   };

   enum Recoil {
      LightRecoil,
      MediumRecoil,
      HeavyRecoil,
      NumRecoilSequences
   };
   S32 recoilSequence[NumRecoilSequences];

   ParticleEmitterData * footPuffEmitter;
   S32                   footPuffID;
   S32                   footPuffNumParts;
   F32                   footPuffRadius;

   DecalData* decalData;
   S32        decalID;
   
   ParticleEmitterData * dustEmitter;
   S32                   dustID;

   SplashData*          splash;
   S32                  splashId;
   F32                  splashVelocity;
   F32                  splashAngle;
   F32                  splashFreqMod;
   F32                  splashVelEpsilon;
   F32                  bubbleEmitTime;

   F32                  medSplashSoundVel;
   F32                  hardSplashSoundVel;
   F32                  exitSplashSoundVel;
   F32                  footSplashHeight;
   
   ParticleEmitterData*    splashEmitterList[NUM_SPLASH_EMITTERS];
   S32                     splashEmitterIDList[NUM_SPLASH_EMITTERS];

   //
   DECLARE_CONOBJECT(PlayerData);
   PlayerData();
   bool preload(bool server, char errorBuffer[256]);
   void getGroundInfo(TSShapeInstance*,TSThread*,ActionAnimation*);
   bool isTableSequence(S32 seq);
   bool isJumpAction(U32 action);

   static void consoleInit();
   static void initPersistFields();
   virtual void packData(BitStream* stream);
   virtual void unpackData(BitStream* stream);
};


//----------------------------------------------------------------------------

class Player: public ShapeBase
{
   typedef ShapeBase Parent;

   //-------------------------------------- NOTE! DO NOT ADD ANY MORE MASK BITS TO THIS
   //--------------------------------------  CLASS WITHOUT CHECKING WITH DMOORE OR MARKF
   //--------------------------------------  We're totally out on the player.
   enum MaskBits {
      ActionMask   = Parent::NextFreeMask << 0,
      MoveMask     = Parent::NextFreeMask << 1,
      ImpactMask   = Parent::NextFreeMask << 2,
      NextFreeMask = Parent::NextFreeMask << 3
   };

   struct Range {
      Range(F32 _min,F32 _max) {
         min = _min;
         max = _max;
         delta = _max - _min;
      };
      F32 min,max;
      F32 delta;
   };

   ParticleEmitter *mSplashEmitter[PlayerData::NUM_SPLASH_EMITTERS];
   F32 mBubbleEmitterTime;

   // Client interpolation/warp data
   struct StateDelta {
      Move move;                    // Last move from server
      F32 dt;                       // Last interpolation time
      // Interpolation data
      Point3F pos;
      Point3F rot;
      Point3F head;
      VectorF posVec;
      VectorF rotVec;
      VectorF headVec;
      // Warp data
      S32 warpTicks;
      Point3F warpOffset;
      Point3F rotOffset;
   };
   StateDelta delta;
   S32 mPredictionCount;            // Number of ticks to predict
   // Current pos, vel etc.
   Point3F mHead;                   // Head rotation, uses only x & z
   Point3F mRot;                    // Body rotation, uses only z
   VectorF mVelocity;
   Point3F mAnchorPoint;            // Pos compression anchor
   static F32 mGravity;
   S32 mImpactSound;
   S32 mMountPending;
   
   // Main player state
   enum ActionState {
      NullState,
      MoveState,
      RecoverState,
      NumStateBits = 3
   };
   ActionState mState;
   bool mFalling;                   // Falling in mid-air
   S32  mJumpDelay;                 // Delay till next jump
   S32  mContactTimer;              // Ticks since last contact

   Point3F mJumpSurfaceNormal;
   U32 mJumpSurfaceLastContact;
   F32  mWeaponBackFraction;        // Amount to slide the weapon back

   AUDIOHANDLE mMoveBubbleHandle;
   AUDIOHANDLE mWaterBreathHandle;

   SimObjectPtr<ShapeBase> mControlObject;

   // Animation threads & data
   struct ActionAnimation {
      U32 action;
      TSThread* thread;
      S32 delayTicks;               // before picking another.
      bool forward;
      bool firstPerson;
      enum Advance {
         Normal,
         Scale,
         Move
      } time;
      bool waitForEnd;
      bool holdAtEnd;
      bool animateOnServer;
      bool atEnd;
   } mActionAnimation;

   struct ArmAnimation {
      U32 action;
      TSThread* thread;
   } mArmAnimation;
   TSThread* mArmThread;

   TSThread* mHeadVThread;
   TSThread* mHeadHThread;
   TSThread* mRecoilThread;
   static Range mArmRange;
   static Range mHeadVRange;
   static Range mHeadHRange;

   bool mDisableMove;
   bool mPilot;
   bool mInMissionArea;
   //
   U32 mRecoverTicks;
   U32 mReversePending;
   U32 mVoiceTag;
   U32 mDesiredVoiceTag;

   bool inLiquid;
   //
   PlayerData* mDataBlock;

   Point3F mLastPos;
   Point3F mLastWaterPos;
   
   struct ContactInfo {
      bool     contacted, jump, run;
      VectorF  contactNormal;
      void clear()   {contacted=jump=run=false; contactNormal.set(1,1,1);}
      ContactInfo()  {clear();}
   } mContactInfo;
   
   struct Death {
      F32      lastPos;
      Point3F  posAdd;
      VectorF  rotate;
      VectorF  curNormal;
      F32      curSink;
      void     clear()           {dMemset(this, 0, sizeof(*this)); initFall();}
      VectorF  getPosAdd()       {VectorF ret(posAdd); posAdd.set(0,0,0); return ret;}
      bool     haveVelocity()    {return posAdd.x != 0 || posAdd.y != 0;}
      void     initFall()        {curNormal.set(0,0,1); curSink = 0;}
      Death()                    {clear();}
      MatrixF* fallToGround(F32 adjust, const Point3F& pos, F32 zrot, F32 boxRad);
   } mDeath;

   // New collision
  public:
   OrthoBoxConvex mConvex;
   Box3F          mWorkingQueryBox;

  protected:
   void setState(ActionState state, U32 ticks=0);
   void updateState();

   void updateMove(const Move* move);
   bool updatePos(const F32 travelTime = TickSec);
   void updateLookAnimation();
   void updateAnimation(F32 dt);
   void updateAnimationTree(bool firstPerson);
   bool step(Point3F *pos,F32 *maxStep,F32 time);

   bool setArmThread(U32 action);
   void setActionThread(U32 action,bool forward,bool hold = false,bool wait = false,bool fsp = false, bool forceSet = false);
   void updateActionThread();
   void pickActionAnimation();
   void onUnmount(ShapeBase* obj,S32 node);

   void setPosition(const Point3F& pos,const Point3F& viewRot);
   void setRenderPosition(const Point3F& pos,const Point3F& viewRot,F32 dt=-1);
   void findContact(bool* run,bool* jump,VectorF* contactNormal);
   virtual void onImageRecoil(U32 imageSlot,ShapeBaseImageData::StateData::RecoilState);
   virtual void updateDamageLevel();
   virtual void updateDamageState();
   void setControllingClient(GameConnection* client);

   void calcClassRenderData();
   void renderMountedImage(SceneState* state, ShapeImageRenderImage* image);
   void renderImage(SceneState* state, SceneRenderImage*);
   void playFootstepSound(bool triggeredLeft, S32 sound);
   void playImpactSound();
   
   bool inDeathAnim();
   F32  deathDelta(Point3F &delta);
   void updateDeathOffsets();
   bool inSittingAnim();

   void updateSplash();
   void updateFroth( F32 dt );
   bool pointInWater( Point3F &point );
   void createSplash( Point3F &pos, F32 speed );
   bool collidingWithWater( Point3F &waterHeight );

public:
   DECLARE_CONOBJECT(Player);

   Player();
   ~Player();
   static void initPersistFields();
   static void consoleInit();
   
   // Transforms are all in object space
   void setTransform(const MatrixF&);
   void getEyeTransform(MatrixF* mat);
   void getRenderEyeTransform(MatrixF* mat);
   void getCameraParameters(F32 *min,F32* max,Point3F* off,MatrixF* rot);
   void getMuzzleTransform(U32 imageSlot,MatrixF* mat);
   void getRenderMuzzleTransform(U32 imageSlot,MatrixF* mat);
   Point3F getVelocity() const;
   void setVelocity(const VectorF& vel);
   void applyImpulse(const Point3F& pos,const VectorF& vec);
   const Point3F& getRotation() { return mRot; }
   const Point3F& getHeadRotation() { return mHead; }
   void getDamageLocation(const Point3F& in_rPos, const char *&out_rpVert, const char *&out_rpQuad);

   bool  canJump();
   F32   getJetAbility(F32& thrust, F32& duration, F32& jumpSpeed);
   bool  haveContact()     {return !mContactTimer;}
   void  getMuzzlePointAI(U32 imageSlot, Point3F* point);
   float getMaxForwardVelocity() { return (mDataBlock != NULL ? mDataBlock->maxForwardSpeed : 0); }
   
   virtual bool    isDisplacable() const;
   virtual Point3F getMomentum() const;
   virtual void    setMomentum(const Point3F&);
   virtual F32     getMass() const;
   virtual bool    displaceObject(const Point3F& displaceVector);

   bool checkDismountPosition(const MatrixF& oldPos, const MatrixF& newPos);
   
   //
   bool onAdd();
   void onRemove();
   bool onNewDataBlock(GameBaseData* dptr);

   // Animation
   const char* getStateName();
   bool setActionThread(const char* sequence,bool hold,bool wait,bool fsp = false);
   bool setArmThread(const char* sequence);

   // Object control
   void setControlObject(ShapeBase*);
   ShapeBase* getControlObject();

   //
   void updateWorkingCollisionSet();
   void disableMove(bool);
   void setPilot(bool);
   bool isPilot() const;
   void processTick(const Move*);
   void interpolateTick(F32 dt);
   void advanceTime(F32 dt);
   bool castRay(const Point3F &start, const Point3F &end, RayInfo* info);
   bool buildPolyList(AbstractPolyList* polyList, const Box3F &box, const SphereF &sphere);
   void buildConvex(const Box3F& box, Convex* convex);
   bool isControlObject();

   void onCameraScopeQuery(NetConnection *cr, CameraScopeQuery *);
   bool writePacketData(GameConnection *, BitStream *stream);
   void readPacketData(GameConnection *, BitStream *stream);
   U32  packUpdate(NetConnection *, U32 mask, BitStream *stream);
   void unpackUpdate(NetConnection *, BitStream *stream);
};


#endif
