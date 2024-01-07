//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _DECALMANAGER_H_
#define _DECALMANAGER_H_

#ifndef _SCENEOBJECT_H_
#include "Sim/sceneObject.h"
#endif
#ifndef _GTEXMANAGER_H_
#include "dgl/gTexManager.h"
#endif

class DecalData : public SimDataBlock
{
   typedef SimDataBlock Parent;

   //-------------------------------------- Console set variables
  public:
   F32               sizeX;
   F32               sizeY;
   StringTableEntry  textureName;

   //-------------------------------------- load set variables
  public:
   TextureHandle textureHandle;

  public:
   DecalData();
   ~DecalData();

   void packData(BitStream*);
   void unpackData(BitStream*);
   bool preload(bool server, char errorBuffer[256]);

   DECLARE_CONOBJECT(DecalData);
   static void initPersistFields();
};

struct DecalInstance
{
   DecalData* decalData;
   Point3F    point[4];

   U32            allocTime;
   F32            fade;
   DecalInstance* next;
};

class DecalManager : public SceneObject
{
   typedef SceneObject Parent;

   Vector<DecalInstance*> mDecalQueue;
   bool                   mQueueDirty;

   static U32             smMaxNumDecals;
   static U32             smDecalTimeout;
   
   static const U32          csmFreePoolBlockSize;
   Vector<DecalInstance*>    mFreePoolBlocks;
   DecalInstance*            mFreePool;
   
  protected:
   void renderObject(SceneState* state, SceneRenderImage*);
   bool prepRenderImage(SceneState*, const U32, const U32, const bool);

   DecalInstance* allocateDecalInstance();
   void freeDecalInstance(DecalInstance*);
   
  public:      					
   DecalManager();
   ~DecalManager();

   static void consoleInit();
   
   void addDecal(Point3F pos, Point3F rot, Point3F normal, DecalData*);
   void addDecal(Point3F pos, Point3F normal, DecalData*);
   void dataDeleted(DecalData *data);

   void renderDecal();
   DECLARE_CONOBJECT(DecalManager);

   static bool smDecalsOn;
};

extern DecalManager* gDecalManager;

#endif // _H_DecalManager
