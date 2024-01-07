//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _SHAPEBASE_H_
#define _SHAPEBASE_H_

#ifndef _GAMEBASE_H_
#include "game/gameBase.h"
#endif
#ifndef _MATERIALLIST_H_
#include "dgl/materialList.h"
#endif
#ifndef _PLATFORMAUDIO_H_
#include "platform/platformAudio.h"
#endif
#ifndef _MOVEMANAGER_H_
#include "game/moveManager.h"
#endif
#ifndef _COLOR_H_
#include "core/color.h"
#endif
#ifndef _CONVEX_H_
#include "collision/convex.h"
#endif
#ifndef _LIGHTMANAGER_H_
#include "scenegraph/lightManager.h"
#endif
#ifndef _SHIELDIMPACT_H_
#include "game/shieldImpact.h"
#endif

class TSShapeInstance;
class Shadow;
class SceneState;
class SceneRenderImage;
class TSShape;
class TSThread;
class GameConnection;
struct CameraScopeQuery;
//class AudioProfile;
class ParticleEmitter;
class ParticleEmitterData;
class ProjectileData;
class ExplosionData;
struct DebrisData;
class CommanderIconData;

typedef void* Light;


//--------------------------------------------------------------------------

extern void collisionFilter(SceneObject* object,S32 key);
extern void defaultFilter(SceneObject* object,S32 key);



class ShapeImageRenderImage : public SceneRenderImage
{
  public:
   ShapeBase* mSBase;
   U32        mIndex;
};



//--------------------------------------------------------------------------
class ShapeBaseConvex : public Convex
{
   typedef Convex Parent;
   friend class ShapeBase;
   friend class Vehicle;

  protected:
   ShapeBase* pShapeBase;
  public:
   U32       hullId;
   Box3F     box;

  public:
   ShapeBaseConvex() { mType = ShapeBaseConvexType; }
   ShapeBaseConvex(const ShapeBaseConvex& cv) {
      mObject    = cv.mObject;
      pShapeBase = cv.pShapeBase;
      hullId     = cv.hullId;
      box        = box;
   }

   Box3F getBoundingBox() const;
   Box3F getBoundingBox(const MatrixF& mat, const Point3F& scale) const;
   Point3F      support(const VectorF& v) const;
   void         getFeatures(const MatrixF& mat,const VectorF& n, ConvexFeature* cf);
   void         getPolyList(AbstractPolyList* list);
};

//--------------------------------------------------------------------------

struct ShapeBaseImageData: public GameBaseData {
  private:
   typedef GameBaseData Parent;

  public:
   enum Constants {
      MaxStates    = 31,            // Get one less that state bits because of
      NumStateBits = 5,             // the way data is packed.
      NumPotentialTargets = 32
   };
   enum LightType {
      NoLight = 0,
      ConstantLight,
      PulsingLight,
      WeaponFireLight,
      NumLightTypes
   };
   struct StateData {
      StateData();
      const char* name;             // State name

      // Transition states
      struct Transition {
         S32 loaded[2];             // NotLoaded/Loaded
         S32 ammo[2];               // Noammo/ammo
         S32 target[2];             // target/noTarget 
         S32 trigger[2];            // Trigger up/down
         S32 wet[2];                // wet/notWet
         S32 timeout;               // Transition after delay
      } transition;
      bool ignoreLoadedForReady;

      // State attributes
      bool fire;                    // Can only have one fire state
      bool ejectShell;              // Shoot shell casing out
      bool allowImageChange;
      bool scaleAnimation;          // Scale animation fit state timeout value
      bool direction;               // Animation direction
      bool waitForTimeout;          // Don't allow transitions if there is a timeout
      F32 timeoutValue;             // delay until next sequence
      F32 energyDrain;              // Drain energy during this state
      enum LoadedState {
         IgnoreLoaded,
         Loaded,
         NotLoaded,
         NumLoadedBits = 3
      } loaded;                     // Is the image considered loaded
      enum SpinState {
         IgnoreSpin,
         NoSpin,
         SpinUp,
         SpinDown,
         FullSpin,
         NumSpinBits = 3
      } spin;                       // Spin thread control
      enum RecoilState {
         NoRecoil,
         LightRecoil,
         MediumRecoil,
         HeavyRecoil,
         NumRecoilBits = 3
      } recoil;
      bool flashSequence;
      S32 sequence;                 // Main thread sequence
      S32 sequenceVis;                 // Vis thread sequence
      const char* script;
      ParticleEmitterData* emitter;
      AudioProfile* sound;
      F32 emitterTime;
      S32 emitterNode;
   };

   // Individual state data used to initialize struct array
   const char* stateName[MaxStates];
   const char* stateTransitionLoaded[MaxStates];
   const char* stateTransitionNotLoaded[MaxStates];
   const char* stateTransitionAmmo[MaxStates];
   const char* stateTransitionNoAmmo[MaxStates];
   const char* stateTransitionTarget[MaxStates];
   const char* stateTransitionNoTarget[MaxStates];
   const char* stateTransitionWet[MaxStates];
   const char* stateTransitionNotWet[MaxStates];
   const char* stateTransitionTriggerUp[MaxStates];
   const char* stateTransitionTriggerDown[MaxStates];
   const char* stateTransitionTimeout[MaxStates];
   F32 stateTimeoutValue[MaxStates];
   bool stateWaitForTimeout[MaxStates];
   bool stateFire[MaxStates];
   bool stateEjectShell[MaxStates];
   F32 stateEnergyDrain[MaxStates];
   bool stateAllowImageChange[MaxStates];
   bool stateScaleAnimation[MaxStates];
   bool stateDirection[MaxStates];
   StateData::LoadedState stateLoaded[MaxStates];
   StateData::SpinState stateSpin[MaxStates];
   StateData::RecoilState stateRecoil[MaxStates];
   const char* stateSequence[MaxStates];
   bool        stateSequenceRandomFlash[MaxStates];
   bool        stateIgnoreLoadedForReady[MaxStates];

   AudioProfile* stateSound[MaxStates];
   const char* stateScript[MaxStates];
   const char* fireStateName;
   ParticleEmitterData* stateEmitter[MaxStates];
   F32 stateEmitterTime[MaxStates];
   const char* stateEmitterNode[MaxStates];

   //
   bool emap;
   bool firstPersonOnly;                

   StringTableEntry shapeName;      // Max shape to render
   U32 mountPoint;
   MatrixF offsetTransform;
   bool firstPerson;                // Render the image when first person

   ProjectileData* projectile;      // Projectile block fired by this image,

   bool  isSeeker;
   bool  useTargetAudio;
   F32   seekRadius;
   F32   maxSeekAngle;
   F32   seekTime;
   F32   minSeekHeat;
   F32   targetingDist;

   F32 mass;
   bool usesEnergy;                 // Uses energy instead off ammo
   F32 minEnergy;                   // Min amount for ammo state to be true
   bool accuFire;                   // Converge with crosshair
   bool cloakable;                  // is this image cloakable when mounted

   // Lighting
   S32         lightType;
   ColorF      lightColor;
   S32         lightTime;
   F32         lightRadius;
   LightInfo   mLight;

   // Data initialized onAdd
   Resource<TSShape> shape;         // Shape handle
   U32 mCRC;
   bool computeCRC;
   MatrixF mountTransform;          // MountPoint node * offsetTransform
   S32 retractNode;
   S32 muzzleNode;
   S32 emitterNode;
   S32 spinSequence;
   S32 ambientSequence;
   bool isAnimated;                 // Contains at least one animated states
   bool hasFlash;                   // Contains at least one flash animation state
   S32 fireState;                   // The fire state set on the client

   // shell casing data
   DebrisData *   casing;
   S32            casingID;
   Point3F        shellExitDir;
   F32            shellExitVariance;   // angle in degrees that the exit dir deviates
   F32            shellVelocity;
   

   // State array is initialized onAdd from the individual state
   // struct array elements.
   StateData state[MaxStates];
   bool      statesLoaded;

   //
   DECLARE_CONOBJECT(ShapeBaseImageData);
   ShapeBaseImageData();
   ~ShapeBaseImageData();
   bool onAdd();
   bool preload(bool server, char errorBuffer[256]);
   S32 lookupState(const char* name);
   static void consoleInit();
   static void initPersistFields();
   virtual void packData(BitStream* stream);
   virtual void unpackData(BitStream* stream);
   void registerImageLights(LightManager * lightManager, bool lightingScene, const Point3F &objectPosition, U32 startTime );
};


//--------------------------------------------------------------------------

struct ShapeBaseData : public GameBaseData {
  private:
   typedef GameBaseData Parent;

  public:
   enum Constants {
      NumMountPoints = 32,
      NumMountPointBits = 5,
      MaxCollisionShapes = 8,
		AIRepairNode = 31
   };

   StringTableEntry  shapeName;
   DebrisData *      debris;
   S32               debrisID;
   StringTableEntry  debrisShapeName;
   Resource<TSShape> debrisShape;
   F32               heat;


   ExplosionData*    explosion;
   S32               explosionID;

   ExplosionData*    underwaterExplosion;
   S32               underwaterExplosionID;

   F32 mass;
   F32 drag;
   F32 density;
   F32 maxEnergy;
   F32 maxDamage;
   F32 repairRate;                  // Rate per tick.

   F32 disabledLevel;
   F32 destroyedLevel;

   S32 shieldEffectLifetimeMS;

   // First/Third person camera
   F32 cameraMaxDist;               // Distance from eye
   F32 cameraMinDist;               // Distance from eye

   // FOV
   F32 cameraDefaultFov;            // default fov (in degrees)
   F32 cameraMinFov;                // min fov allowed (in degrees)
   F32 cameraMaxFov;                // max fov for object (in degrees)

   // Data initialized on preload
   Resource<TSShape> shape;         // Shape handle
   U32 mCRC;
   bool computeCRC;

   S32 eyeNode;                     // Shape's eye node index
   S32 cameraNode;                  // Shape's camera node index
   S32 shadowNode;                  // Move shadow center as this node moves
   S32 mountPointNode[NumMountPoints]; // Node index of mountPoint
   S32 debrisDetail;                // Detail level used to generate debris
   S32 damageSequence;              // Damage level decals
   S32 hulkSequence;                // Destroyed hulk

   CommanderIconData * cmdIcon;              // commander icon
   SimObjectId       cmdIconId;              // id of icon
   F32               sensorRadius;           // sensor radius (0 = not a sensor)
   ColorI            sensorColor;            // color of sensor
   StringTableEntry  cmdCategory;            // category this belongs in
   StringTableEntry  cmdMiniIconName;        // list icon name
   bool              canControl;             // can this object be controlled?
   bool              canObserve;             // may look at object in commander map?
   bool              observeThroughObject;   // observe this object through its camera transform and default fov

   TextureHandle  cmdMiniIcon;         // icon in commander list

   // hud images...
   enum {
      NumHudRenderImages      = 8,
   };

   StringTableEntry  hudImageNameFriendly[NumHudRenderImages];
   StringTableEntry  hudImageNameEnemy[NumHudRenderImages];
   TextureHandle     hudImageFriendly[NumHudRenderImages];
   TextureHandle     hudImageEnemy[NumHudRenderImages];

   bool              hudRenderCenter[NumHudRenderImages];
   bool              hudRenderModulated[NumHudRenderImages];
   bool              hudRenderAlways[NumHudRenderImages];
   bool              hudRenderDistance[NumHudRenderImages];
   bool              hudRenderName[NumHudRenderImages];

   S32   collisionDetails[8];       // Detail level used to collide with
   Box3F collisionBounds[8];        // Detail level bounding boxes
   
   S32 LOSDetails[8];               // Detail level used to collide with

   // these are set by derived classes, not by script file
   // they control when shadows are rendered (and when generic shadows are substituted)
   F32 genericShadowLevel;
   F32 noShadowLevel;

   bool hackDisallowDamage;
   bool emap;
   bool firstPersonOnly;                
   bool useEyePoint;                
   bool aiAvoidThis;						//if set, the AI's will try to walk around this object...
   bool isInvincible;               // if set, object cannot take damage (wont show up with damage bar either)
   bool renderWhenDestroyed;        // if set, will not render this obj when destroyed
   
   bool inheritEnergyFromMount;

   bool preload(bool server, char errorBuffer[256]);
   void computeAccelerator(U32 i);
   S32  findMountPoint(U32 n);

   // The derived class should provide the following:
   DECLARE_CONOBJECT(ShapeBaseData);
   ShapeBaseData();
   ~ShapeBaseData();
   static void consoleInit();
   static void initPersistFields();
   virtual void packData(BitStream* stream);
   virtual void unpackData(BitStream* stream);
};

//----------------------------------------------------------------------------
class ShapeBase : public GameBase
{
   typedef GameBase Parent;
   friend class ShapeBaseConvex;
   friend void physicalZoneFind(SceneObject*, S32);

   void fade( F32 dt );

   ShieldImpact mShieldEffect;
   bool         mFlipFadeVal;
   U32          mLightTime;

  public:
   F32 getUpdatePriority(CameraScopeQuery *focusObject, U32 updateMask, S32 updateSkips);

   enum PublicConstants {
      ThreadSequenceBits = 5,
      MaxSequenceIndex = (1 << ThreadSequenceBits) - 1,
      EnergyLevelBits = 5,
      DamageLevelBits = 6,
      DamageStateBits = 2,
      MaxSoundThreads =  4,            // Should be a power of 2
      MaxScriptThreads = 4,            // Should be a power of 2
      MaxMountedImages = 8,            // Should be a power of 2
      MaxImageEmitters = 3,
      NumImageBits = 3,
      ShieldNormalBits = 8,
      NumHeatBits  = 5,
      CollisionTimeoutValue = 250      // Timeout in Ms.
   };

   enum DamageState {
      // These enums index into the sDamageStateName array
      Enabled,
      Disabled,
      Destroyed,
      NumDamageStates,
      NumDamageStateBits = 2,
   };

   static bool sUsePrefSkins;
  protected:
   ShapeBaseData*  mDataBlock;
   GameConnection* mControllingClient; // Controlling client
   ShapeBase* mControllingObject;
   bool mTrigger[MaxTriggerKeys];

   // Scripted Sound
   struct Sound {
      bool play;
      SimTime timeout;
      AudioProfile* profile;        // Profile on server
      AUDIOHANDLE sound;               // Handle on client
   };
   Sound mSoundThread[MaxSoundThreads];

   // Scripted Animation Threads
   struct Thread {
      enum State {
         Play, Stop, Pause
      };
      TSThread* thread;
      U32 state;
      S32 sequence;
      U32 sound;
      bool atEnd;
      bool forward;
   };
   Thread mScriptThread[MaxScriptThreads];

public:
   // Mounted Images.
   struct MountedImage {
      ShapeBaseImageData* dataBlock;
      ShapeBaseImageData::StateData *state;
      ShapeBaseImageData* nextImage;
      U32 skinTag;
      U32 desiredTag;
      bool loaded;                  // Loaded with ammo
      U32 nextTeam;
      bool nextLoaded;
      F32 delayTime;                // Current state countdown
      U32 fireCount;                // Fire skip count
      bool triggerDown;             // Current trigger state
      bool ammo;                    // Set internally if using energy
      bool target;
      bool wet;

      TSShapeInstance* shapeInstance;
      TSThread *ambientThread;
      TSThread *visThread;
      TSThread *animThread;
      TSThread *flashThread;
      TSThread *spinThread;

      //TSLight light;
      SimTime lightStart;
      bool animLoopingSound;
      AUDIOHANDLE animSound;

      struct ImageEmitter {
         S32 node;
         F32 time;
         SimObjectPtr<ParticleEmitter> emitter;
      };
      ImageEmitter emitter[MaxImageEmitters];

      //
      MountedImage();
      ~MountedImage();
   };

protected:
   MountedImage mMountedImageList[MaxMountedImages];
   U32 mActiveImage;

   // Collision Notification
  public:
   struct CollisionTimeout {
      CollisionTimeout* next;
      ShapeBase* object;
      U32 objectNumber;
      SimTime expireTime;
      bool useData;
      F32 data;
   };
   enum LockMode {
      NotLocked    = 0,
      LockObject   = 1,
      LockPosition = 2
   };
   F32 mLiquidHeight;
   F32 mWaterCoverage;
   
   bool     blowApart;
   VectorF  damageDir;

  protected:
   CollisionTimeout* mTimeoutList;

   void scriptCallback(U32 imageSlot,const char* function);
   void updateMass();
   virtual void setImage(U32 imageSlot, ShapeBaseImageData* imageData, U32 teamTag = 0,
                         bool loaded = true, bool ammo = false, bool triggerDown = false,
                         bool target = false);
   void resetImageSlot(U32 imageSlot);
   U32  getImageFireState(U32 imageSlot);
   void setImageState(U32 imageSlot, U32 state, bool force = false);
   void updateImageAnimation(U32 imageSlot, F32 dt);
   void updateImageState(U32 imageSlot,F32 dt);
   void startImageEmitter(MountedImage&,ShapeBaseImageData::StateData&);
   Light* getImageLight(U32 imageSlot);
   void updateServerAudio();
   void updateAudioState(Sound& st);
   void updateAudioPos();
  protected:
   F32 mEnergy;
   F32 mRechargeRate;               // Energy/tick recharge rate
   bool mChargeEnergy;

   F32 mMass;
   F32 mOneOverMass;
   F32 mDrag;                       // Container drag
   F32 mBuoyancy;                   // Container buoyancy factor
   U32 mLiquidType;

   Point3F mAppliedForce;
   F32     mGravityMod;

   F32 mDamageFlash;
   F32 mWhiteOut;
   F32 mHeat;
   
  protected:   
   // Last shield direction
   Point3F mShieldNormal;

   // Invincible
   F32 mInvincibleCount;
   F32 mInvincibleTime;
   F32 mInvincibleSpeed;
   F32 mInvincibleDelta;
   F32 mInvincibleEffect;
   F32 mInvincibleFade;
   bool mInvincibleOn;

   // Mounted objects
   struct MountInfo {
      ShapeBase* list;              // Objects mounted on this object
      ShapeBase* object;            // Object this object is mounted on.
      ShapeBase* link;              // Object link of next object mounted to this object's mount
      U32 node;                     // Node point.
   } mMount;

   // Damage
   F32  mDamage;
   F32  mRepairRate;
   F32  mRepairReserve;
   bool mRepairDamage;
   DamageState mDamageState;
   TSThread *mDamageThread;
   TSThread *mHulkThread;

   bool mHidden; // in/out of world

   // Cloaking
   bool mCloaked;
   F32  mCloakLevel;
   TextureHandle mCloakTexture;

   // Fading
   bool  mFadeOut;
   bool  mFading;
   F32   mFadeVal;
   F32   mFadeElapsedTime;
   F32   mFadeTime;
   F32   mFadeDelay;

   // Passive Jammed (active when cloaking pack is equiped)
   bool mPassiveJammed;

   // Camera (in degrees)
   F32  mCameraFov;

   // being controlled?
   bool mIsControlled;
   
   // Locking and target data...
   struct PotentialLock
   {
      SimObjectPtr<GameBase> potentialTarget;
      bool                   isTarget;
      U32                    tag;
      U32                    numTicks;
      PotentialLock*         next;
   };

   bool                    mTracking;
   SimObjectPtr<ShapeBase> mCurrLockTarget;
   Point3F                 mCurrLockPosition;

   LockMode                mLockedOn;

   PotentialLock*          mPotentialTargets;

  public:
   static U32 sLastRenderFrame;
   U32 mLastRenderFrame;
   F32 mLastRenderDistance;

   bool didRenderLastRender() { return mLastRenderFrame == sLastRenderFrame; }

   LockMode getLockMode() { return mLockedOn; }
   ShapeBase *getLockedTarget() { return mCurrLockTarget; }

   void setLockedTarget(ShapeBase*);
   void setLockedTargetPosition(const Point3F&);
   bool isLocked() const;
   bool isTracking() const;
   S32  getLockedTargetId() const;
   const Point3F& getLockedPosition() const;
   void thinkAboutLocking();
   bool useTargetAudio();

   virtual void setHidden(bool);
   bool isHidden()   { return mHidden; }
   bool isControlled() { return(mIsControlled); }
   bool isInvincible();
   
   void startFade( F32 fadeTime, F32 fadeDelay = 0.0, bool fadeOut = true );

   void registerLights(LightManager * lightManager, bool lightingScene);

   //
  protected:
   TSShapeInstance* mShapeInstance;
   Shadow * mShadow;
   bool mGenerateShadow;
   U32 mSkinTag;
   U32 mSkinPrefTag;

   S32 getNodeIndex(U32 imageSlot,StringTableEntry nodeName);

   void notifyCollision();
   void updateContainer();
   virtual void onDeleteNotify(SimObject*);
   virtual void onImageRecoil(U32 imageSlot,ShapeBaseImageData::StateData::RecoilState);
   virtual void ejectShellCasing( U32 imageSlot );
   virtual void updateDamageLevel();
   virtual void updateDamageState();
   virtual void blowUp();
   virtual void onMount(ShapeBase* obj,S32 node);
   virtual void onUnmount(ShapeBase* obj,S32 node);
   virtual void onImpact(SceneObject* obj, VectorF vec);
   virtual void onImpact(VectorF vec);
  public:
   ShapeBase();
   ~ShapeBase();

   TSShapeInstance* getShapeInstance() { return mShapeInstance; }

   //-------------------------------------- NOTE! DO NOT ADD ANY MORE MASK BITS TO THIS
   //--------------------------------------  CLASS WITHOUT CHECKING WITH DMOORE OR MARKF
   //--------------------------------------  We're totally out on the player.
   enum ShapeBaseMasks {
      DamageMask      = Parent::NextFreeMask,
      NoWarpMask      = Parent::NextFreeMask << 1,
      MountedMask     = Parent::NextFreeMask << 2,
      CloakMask       = Parent::NextFreeMask << 3,
      ShieldMask      = Parent::NextFreeMask << 4,
      InvincibleMask  = Parent::NextFreeMask << 5,
      SoundMaskN      = Parent::NextFreeMask << 6,       // Extends + MaxSoundThreads bits
      ThreadMaskN     = SoundMaskN  << MaxSoundThreads,  // Extends + MaxScriptThreads bits
      ImageMaskN      = ThreadMaskN << MaxScriptThreads, // Extends + MaxMountedImage bits
      NextFreeMask    = ImageMaskN  << MaxMountedImages
   };

   enum BaseMaskConstants {
      SoundMask      = (SoundMaskN << MaxSoundThreads) - SoundMaskN,
      ThreadMask     = (ThreadMaskN << MaxScriptThreads) - ThreadMaskN,
      ImageMask      = (ImageMaskN << MaxMountedImages) - ImageMaskN
   };

   static bool gRenderEnvMaps;
   static F32  sWhiteoutDec;
   static F32  sDamageFlashDec;

   // Init
   bool onAdd();
   void onRemove();
   void onSceneRemove();
   static void consoleInit();
   static void initPersistFields();

   bool onNewDataBlock(GameBaseData* dptr);

   // Name & Skin tags
   U32 mSkinHash;
   void targetInfoChanged(TargetInfo *);
   void checkSkin();

   // Basic attributes
   void setControlDirty();

   void setDamageLevel(F32 damage);
   void setDamageState(DamageState state);
   bool setDamageState(const char*);
   const char* getDamageStateName();
   DamageState getDamageState() { return mDamageState; }
   bool isDestroyed() { return mDamageState == Destroyed; }
   void setRepairRate(F32 rate) { mRepairRate = rate; }
   F32  getDamageLevel()  { return mDamage; }
   F32  getDamageValue();
   F32  getRepairRate() { return mRepairRate; }
   void applyDamage(F32 amount);
   void applyRepair(F32 amount);

   virtual void setEnergyLevel(F32 energy);
   void setRechargeRate(F32 rate) { mRechargeRate = rate; }
   F32  getEnergyLevel();
   F32  getEnergyValue();
   F32  getRechargeRate() { return mRechargeRate; }

   // Script sounds
   void playAudio(U32 slot,AudioProfile* profile);
   void stopAudio(U32 slot);

   // Script animation
   bool setThreadSequence(U32 slot,S32 seq,bool reset = true);
   void updateThread(Thread& st);
   bool stopThread(U32 slot);
   bool pauseThread(U32 slot);
   bool playThread(U32 slot);
   bool setThreadDir(U32 slot,bool forward);
   void startSequenceSound(Thread& thread);
   void stopThreadSound(Thread& thread);
   void advanceThreads(F32 dt);

   // Cloaking
   void forceUncloak(const char *);
   void setCloakedState(bool);
   bool getCloakedState();
   F32 getCloakLevel();

   // passive jamming
   void setPassiveJamState(bool val) {mPassiveJammed = val;}
   bool getPassiveJamState() {return(mPassiveJammed);}

   // Mounted objects
   virtual void mountObject(ShapeBase* obj,U32 node);
   void unmountObject(ShapeBase *obj);
   void unmount();
   ShapeBase* getObjectMount()  { return mMount.object; }
   ShapeBase* getMountLink()  { return mMount.link; }
   ShapeBase* getMountList()  { return mMount.list; }
   U32 getMountNode()  { return mMount.node; }
   bool isMounted() { return mMount.object != 0; }
   S32 getMountedObjectCount();
   ShapeBase* getMountedObject(S32 idx);
   S32 getMountedObjectNode(S32 idx);
   ShapeBase* getMountNodeObject(S32 node);
	Point3F getAIRepairPoint();

   // Mounted images
   virtual bool mountImage(ShapeBaseImageData* image,U32 imageSlot,bool loaded,S32 team);
   virtual bool unmountImage(U32 imageSlot);
   ShapeBaseImageData* getMountedImage(U32 imageSlot);
   MountedImage* retrieveMountedImage(U32 imageSlot);
   ShapeBaseImageData* getPendingImage(U32 imageSlot);
   bool isImageFiring(U32 imageSlot);
   bool isImageReady(U32 imageSlot,U32 ns = (U32)-1,U32 depth = 0);
   bool isImageMounted(ShapeBaseImageData* image);
   S32 getMountSlot(ShapeBaseImageData* image);
   U32 getImageSkinTag(U32 imageSlot);
   const char* getImageState(U32 imageSlot);
   void setImageTriggerState(U32 imageSlot,bool trigger);
   bool getImageTriggerState(U32 imageSlot);
   void setImageAmmoState(U32 imageSlot,bool ammo);
   bool getImageAmmoState(U32 imageSlot);
   void setImageTargetState(U32 imageSlot,bool target);
   bool getImageTargetState(U32 imageSlot);
   void setImageWetState(U32 imageSlot,bool wet);
   bool getImageWetState(U32 imageSlot);
   void setImageLoadedState(U32 imageSlot,bool loaded);
   bool getImageLoadedState(U32 imageSlot);
   bool getCorrectedAim(const MatrixF& muzMat, VectorF* result);
   virtual void getMuzzleVector(U32 imageSlot,VectorF* vec);
   void getMuzzlePoint(U32 imageSlot,Point3F* pos);

   // Transforms in world space
   virtual void getCameraParameters(F32 *min,F32* max,Point3F* offset,MatrixF* rot);
   virtual void getCameraTransform(F32* pos,MatrixF* mat);
   virtual void getEyeTransform(MatrixF* mat);
   virtual void getRetractionTransform(U32 index,MatrixF* mat);
   virtual void getMountTransform(U32 index,MatrixF* mat);
   virtual void getMuzzleTransform(U32 index,MatrixF* mat);
   virtual void getImageTransform(U32 imageSlot,MatrixF* mat);
   virtual void getImageTransform(U32 index,S32 node, MatrixF* mat);
   virtual void getImageTransform(U32 index, StringTableEntry nodeName, MatrixF* mat);

   virtual void getRenderRetractionTransform(U32 index,MatrixF* mat);
   virtual void getRenderMountTransform(U32 index,MatrixF* mat);
   virtual void getRenderMuzzleTransform(U32 index,MatrixF* mat);
   virtual void getRenderImageTransform(U32 imageSlot,MatrixF* mat);
   virtual void getRenderImageTransform(U32 index,S32 node, MatrixF* mat);
   virtual void getRenderImageTransform(U32 index, StringTableEntry nodeName, MatrixF* mat);
   virtual void getRenderMuzzleVector(U32 imageSlot,VectorF* vec);
   virtual void getRenderMuzzlePoint(U32 imageSlot,Point3F* pos);
   virtual void getRenderEyeTransform(MatrixF* mat);

   MatrixF getNodeTransform( StringTableEntry nodeName );

   virtual F32  getDamageFlash() const;
   virtual void setDamageFlash(const F32);
   virtual F32  getWhiteOut() const;
   virtual void setWhiteOut(const F32);

   virtual void playShieldEffect(const Point3F& normal, F32 strength = 1.0 );

   virtual void setHeat(const F32);
   virtual F32  getHeat() const;

   // Invincible functions
   virtual F32 getInvincibleEffect() const;
   virtual void setupInvincibleEffect(F32 time, F32 speed);
   virtual void updateInvincibleEffect(F32 dt);

   // Movement & velocity
   virtual void setVelocity(const VectorF& vel);
   virtual void applyImpulse(const Point3F& pos,const VectorF& vec);

   // User control
   GameConnection* getControllingClient() { return mControllingClient; }
   ShapeBase* getControllingObject()   { return mControllingObject; }
   virtual void setControllingClient(GameConnection* client);
   virtual void setControllingObject(ShapeBase* obj);
   virtual ShapeBase* getControlObject();
   virtual void setControlObject(ShapeBase*);
   bool isFirstPerson();
   bool useObjsEyePoint() const;
   bool onlyFirstPerson() const;

   void setActiveImage(U32 slot);
   U32 getActiveImage() const { return(mActiveImage); }

   // camera: in degrees
   virtual F32 getCameraFov();
   virtual F32 getDefaultCameraFov();
   virtual void setCameraFov(F32 fov);
   virtual bool isValidCameraFov(F32 fov);

   // Processing
   void processTick(const Move*);
   void advanceTime(F32 dt);

   // Rendering
   TSShape const* getShape();
   bool prepRenderImage(SceneState*, const U32 stateKey, const U32 startZone, const bool modifyBaseState);
   void renderObject(SceneState* , SceneRenderImage*);
   virtual void renderMountedImage(SceneState* state, ShapeImageRenderImage* image);
   virtual void renderImage(SceneState* state, SceneRenderImage*);
   void renderShadow(F32 dist, F32 fogAmount);
   static void wireCube(const Point3F& size, const Point3F& pos);
   virtual void calcClassRenderData();

   // Control object scoping
   void onCameraScopeQuery(NetConnection *cr, CameraScopeQuery *camInfo);

   // Collision
   bool castRay(const Point3F &start, const Point3F &end, RayInfo* info);
   bool buildPolyList(AbstractPolyList* polyList, const Box3F &box, const SphereF& sphere);
   void buildConvex(const Box3F& box, Convex* convex);
  protected:
   Convex* mConvexList;

  public:
   virtual void onCollision(ShapeBase* object);
   void queueCollision(ShapeBase* object, const F32 inData = -1e9);
   bool pointInWater( Point3F &point );

   // Network
   U32  packUpdate(NetConnection *, U32 mask, BitStream *stream);
   void unpackUpdate(NetConnection *, BitStream *stream);
   bool writePacketData(GameConnection *, BitStream *stream);
   void readPacketData(GameConnection *, BitStream *stream);

   DECLARE_CONOBJECT(ShapeBase);
};

//------------------------------------------------------------------------------
// inlines
//------------------------------------------------------------------------------

inline bool ShapeBase::useTargetAudio()
{
   return (getMountedImage(0) != 0 && getMountedImage(0)->useTargetAudio);
}

inline bool ShapeBase::getCloakedState()
{
   return(mCloaked);
}

inline F32 ShapeBase::getCloakLevel()
{
   return(mCloakLevel);
}

inline bool ShapeBase::isTracking() const
{
   return mTracking;
}

inline void ShapeBase::setActiveImage(U32 slot)
{
   AssertFatal(slot < MaxMountedImages, "ShapeBase::setActiveImage: slot out of range");
   mActiveImage = slot;
}

// shadow detail numbers...
// the generic shadow level is the shadow detail at which
// a generic shadow is drawn (a disk) rather than a generated
// shadow...the no shadow level is the shadow level at which
// no shadow is drawn. (shadow level goes from 0 to 1,
// higher numbers result in more detailed shadows).
#define Player_GenericShadowLevel 0.4f
#define Player_NoShadowLevel 0.01f
#define Vehicle_GenericShadowLevel 0.7f
#define Vehicle_NoShadowLevel 0.2f
#define Item_GenericShadowLevel 0.4f
#define Item_NoShadowLevel 0.01f
#define StaticShape_GenericShadowLevel 2.0f
#define StaticShape_NoShadowLevel 2.0f
#define Turret_GenericShadowLevel 2.0f
#define Turret_NoShadowLevel 2.0f

#endif  // _H_SHAPEBASE_
