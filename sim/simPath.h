//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _SIMPATH_H_
#define _SIMPATH_H_

#ifndef _SCENEOBJECT_H_
#include "Sim/sceneObject.h"
#endif

//--------------------------------------------------------------------------
class Path : public SimGroup
{
   typedef SimGroup Parent;

  public:
   enum {
      NoPathIndex = 0xFFFFFFFF
   };


  private:
   U32 mPathIndex;

  protected:
   bool onAdd();
   void onRemove();

  public:
   Path();
   ~Path();

   void addObject(SimObject*);
   void removeObject(SimObject*);
   
   void finishPath();
   U32 getPathIndex() const;

   DECLARE_CONOBJECT(Path);
   static void initPersistFields();
   static void consoleInit();
};


//--------------------------------------------------------------------------
class Marker : public SceneObject
{
   typedef SceneObject Parent;
   friend class Path;

  public:
   U32   mSeqNum;
   U32   mMSToNext;

   // Rendering
  protected:
   bool prepRenderImage(SceneState*, const U32, const U32, const bool);
   void renderObject(SceneState*, SceneRenderImage*);

  protected:
   bool onAdd();
   void onRemove();

  public:
   Marker();
   ~Marker();

   DECLARE_CONOBJECT(Marker);
   static void initPersistFields();
   static void consoleInit();

   U32  packUpdate(NetConnection*, U32 mask, BitStream* stream);
   void unpackUpdate(NetConnection*, BitStream* stream);
};

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
inline U32 Path::getPathIndex() const
{
   return mPathIndex;
}


#endif // _H_PATH

