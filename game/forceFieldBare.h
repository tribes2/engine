//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _FORCEFIELDBARE_H_
#define _FORCEFIELDBARE_H_

#ifndef _GAMEBASE_H_
#include "game/gameBase.h"
#endif
#ifndef _COLOR_H_
#include "core/color.h"
#endif

class Convex;

// -------------------------------------------------------------------------
class ForceFieldBareData : public GameBaseData
{
   typedef GameBaseData Parent;

   enum Constants
   {
      NUM_TEX = 5,
   };

  protected:
   bool onAdd();

   //-------------------------------------- Console set variables
  public:
   S32   fadeMS;
   F32   baseTranslucency;
   F32   powerOffTranslucency;

   StringTableEntry  textureName[NUM_TEX];
   TextureHandle     textureHandle[NUM_TEX];

   F32   umapping;
   F32   vmapping;
   F32   scrollSpeed;
   U32   framesPerSec;
   U32   numFrames;
   bool  teamPermiable;
   bool  otherPermiable;
   ColorF color;
   ColorF powerOffColor;

   //-------------------------------------- load set variables
  public:

  public:
   ForceFieldBareData();
   ~ForceFieldBareData();

   void packData(BitStream*);
   void unpackData(BitStream*);

   DECLARE_CONOBJECT(ForceFieldBareData);
   static void initPersistFields();
   bool preload(bool server, char errorBuffer[256]);
};


// -------------------------------------------------------------------------
class ForceFieldBare : public GameBase
{
   typedef GameBase Parent;

  private:
   ForceFieldBareData* mDataBlock;

  protected:
   // Most of this copied from ForceFieldInstance (Interior forcefields)
   enum State {
      Open    = 0,
      Opening = 1,
      Closing = 2,
      Closed  = 3,

      StateBitsRequired = 2
   };

   State    mCurrState;
   S32      mCurrPosition;

   S32      mClientLastPosition;    // Client interpolation _only_
   F32      mCurrBackDelta;
   F32      mCurrFade;
   F32      mElapsedTime;

   enum FFBStateChangeMasks {
      TransformMask   = Parent::NextFreeMask << 0,
      StateChangeMask = Parent::NextFreeMask << 1
   };

   Convex* mConvexList;
   void buildConvex(const Box3F& box, Convex* convex);

  protected:
   bool onAdd();
   void onRemove();
   bool onNewDataBlock(GameBaseData*);
   void advanceTime(F32 dt);

   // Rendering
  protected:
   bool prepRenderImage(SceneState*, const U32, const U32, const bool);
   void renderObject(SceneState*, SceneRenderImage*);
   void setImagePoly(SceneRenderImage*);
   
   // Collision
  public:
   bool buildPolyList(AbstractPolyList*, const Box3F&, const SphereF&);
   bool castRay(const Point3F&, const Point3F&, RayInfo*);

  protected:
   void setTransform(const MatrixF&);
   void setScale(const VectorF & scale);

   // Time/Move Management
  public:
   virtual void processClientTick();   // These functions got unwieldy as processTick.
   virtual void processServerTick();   //  derived classes should override these...

   void processTick(const Move*);
   void interpolateTick(F32);
   void setClientState(const State newState, const U32 newPosition);

  public:
   ForceFieldBare();
   ~ForceFieldBare();

   bool isOpen() { return mCurrState == Open; }
   void open();
   void close();

   bool isPermiableTo(GameBase*);
   bool isTeamControlled();

   DECLARE_CONOBJECT(ForceFieldBare);
   static void initPersistFields();
   static void consoleInit();

   U32  packUpdate(NetConnection*, U32 mask, BitStream* stream);
   void unpackUpdate(NetConnection*, BitStream* stream);
};

#endif // _H_FORCEFIELDBARE

