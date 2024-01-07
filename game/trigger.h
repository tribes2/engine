//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _H_TRIGGER
#define _H_TRIGGER

#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif
#ifndef _GAMEBASE_H_
#include "game/gameBase.h"
#endif
#ifndef _MBOX_H_
#include "Math/mBox.h"
#endif
#ifndef _EARLYOUTPOLYLIST_H_
#include "Collision/earlyOutPolyList.h"
#endif

class Convex;

struct TriggerData: public GameBaseData {
   typedef GameBaseData Parent;

  public:
   S32  tickPeriodMS;

   TriggerData();
   DECLARE_CONOBJECT(TriggerData);
   bool onAdd();
   static void consoleInit();
   static void initPersistFields();
   virtual void packData(BitStream* stream);
   virtual void unpackData(BitStream* stream);
};

class Trigger : public GameBase
{
   typedef GameBase Parent;

   Polyhedron        mTriggerPolyhedron;
   EarlyOutPolyList  mClippedList;
   Vector<GameBase*> mObjects;

   TriggerData*      mDataBlock;

   U32               mLastThink;
   U32               mCurrTick;

  protected:
   bool onAdd();
   void onRemove();
   void onDeleteNotify(SimObject*);
   bool onNewDataBlock(GameBaseData* dptr);

   bool testObject(GameBase* enter);
   void processTick(const Move*);

   Convex* mConvexList;
   void buildConvex(const Box3F& box, Convex* convex);

   // Rendering
  protected:
   bool prepRenderImage(SceneState*, const U32, const U32, const bool);
   void renderObject(SceneState*, SceneRenderImage*);

  public:
   void setTransform(const MatrixF&);

  public:
   Trigger();
   ~Trigger();

   void setTriggerPolyhedron(const Polyhedron&);

   void      potentialEnterObject(GameBase*);
   U32       getNumTriggeringObjects() const;
   GameBase* getObject(const U32);

   DECLARE_CONOBJECT(Trigger);
   static void initPersistFields();
   static void consoleInit();

   U32  packUpdate(NetConnection*, U32 mask, BitStream* stream);
   void unpackUpdate(NetConnection*, BitStream* stream);
};

inline U32 Trigger::getNumTriggeringObjects() const
{
   return mObjects.size();
}

inline GameBase* Trigger::getObject(const U32 index)
{
   AssertFatal(index < getNumTriggeringObjects(), "Error, out of range object index");

   return mObjects[index];
}

#endif // _H_TRIGGER

