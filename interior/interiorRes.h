//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _INTERIORRES_H_
#define _INTERIORRES_H_

#ifndef _RESMANAGER_H_
#include "Core/resManager.h"
#endif

class Stream;
class Interior;
class GBitmap;
class InteriorResTrigger;
class InteriorPath;
class InteriorPathFollower;
class ForceField;
class AISpecialNode;

class InteriorResource : public ResourceInstance
{
   typedef ResourceInstance Parent;
   static const U32 smFileVersion;

  protected:
   Vector<Interior*>             mDetailLevels;
   Vector<Interior*>             mSubObjects;
   Vector<InteriorResTrigger*>   mTriggers;
   Vector<InteriorPath*>         mPaths;
   Vector<InteriorPathFollower*> mInteriorPathFollowers;
   Vector<ForceField*>           mForceFields;
   Vector<AISpecialNode*>        mAISpecialNodes;

   GBitmap* mPreviewBitmap;

  public:
   InteriorResource();
   ~InteriorResource();

   bool            read(Stream& stream);
   bool            write(Stream& stream) const;
   static GBitmap* extractPreview(Stream&);

   S32       getNumDetailLevels() const;
   S32       getNumSubObjects() const;
   S32       getNumTriggers() const;
   S32       getNumPaths() const;
   S32       getNumInteriorPathFollowers() const;
   S32       getNumForceFields() const;
   S32       getNumSpecialNodes() const;

   Interior*             getDetailLevel(const U32);
   Interior*             getSubObject(const U32);
   InteriorResTrigger*   getTrigger(const U32);
   InteriorPath*         getPath(const U32);
   InteriorPathFollower* getInteriorPathFollower(const U32);
   ForceField*           getForceField(const U32);
   AISpecialNode*        getSpecialNode(const U32);
};
extern ResourceInstance* constructInteriorDIF(Stream& stream);

//--------------------------------------------------------------------------
inline S32 InteriorResource::getNumDetailLevels() const
{
   return mDetailLevels.size();
}

inline S32 InteriorResource::getNumSubObjects() const
{
   return mSubObjects.size();
}

inline S32 InteriorResource::getNumTriggers() const
{
   return mTriggers.size();
}

inline S32 InteriorResource::getNumPaths() const
{
   return mPaths.size();
}

inline S32 InteriorResource::getNumSpecialNodes() const
{
   return mAISpecialNodes.size();
}

inline S32 InteriorResource::getNumInteriorPathFollowers() const
{
   return mInteriorPathFollowers.size();
}

inline S32 InteriorResource::getNumForceFields() const
{
   return mForceFields.size();
}

inline Interior* InteriorResource::getDetailLevel(const U32 idx)
{
   AssertFatal(idx < getNumDetailLevels(), "Error, out of bounds detail level!");

   return mDetailLevels[idx];
}

inline Interior* InteriorResource::getSubObject(const U32 idx)
{
   AssertFatal(idx < getNumSubObjects(), "Error, out of bounds subObject!");

   return mSubObjects[idx];
}

inline InteriorResTrigger* InteriorResource::getTrigger(const U32 idx)
{
   AssertFatal(idx < getNumTriggers(), "Error, out of bounds trigger!");

   return mTriggers[idx];
}

inline InteriorPath* InteriorResource::getPath(const U32 idx)
{
   AssertFatal(idx < getNumPaths(), "Error, out of bounds path!");

   return mPaths[idx];
}

inline InteriorPathFollower* InteriorResource::getInteriorPathFollower(const U32 idx)
{
   AssertFatal(idx < getNumInteriorPathFollowers(), "Error, out of bounds path follower!");

   return mInteriorPathFollowers[idx];
}

inline ForceField* InteriorResource::getForceField(const U32 idx)
{
   AssertFatal(idx < getNumForceFields(), "Error, out of bounds force field!");

   return mForceFields[idx];
}

inline AISpecialNode* InteriorResource::getSpecialNode(const U32 idx)
{
   AssertFatal(idx < getNumSpecialNodes(), "Error, out of bounds Special Nodes!");

   return mAISpecialNodes[idx];
}

#endif  // _H_INTERIORRES_

