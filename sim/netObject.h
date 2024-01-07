//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _NETOBJECT_H_
#define _NETOBJECT_H_

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif
#ifndef _MMATH_H_
#include "Math/mMath.h"
#endif


//-----------------------------------------------------------------------------

class NetConnection;
class NetObject;


//-----------------------------------------------------------------------------

struct CameraScopeQuery
{
   NetObject *camera;
   Point3F pos;				// Pos in world space
   Point3F orientation;		// Vector in world space
   F32 fov; 					// viwing angle/2
   F32 sinFov;           // sin(fov/2);
   F32 cosFov;           // cos(fov/2);
	F32 visibleDistance;  
};

struct GhostInfo;


//-----------------------------------------------------------------------------

class NetObject: public SimObject
{
	// The Ghost Manager needs read/write access
	friend class  NetConnection;
   friend struct GhostInfo;
   friend class  ProcessList;

   // Not the best way to do this, but the event needs access to mNetFlags
   friend class GhostAlwaysObjectEvent;

private:
   typedef SimObject Parent;
   NetObject *mPrevDirtyList;
   NetObject *mNextDirtyList;
   U32 mDirtyMaskBits;

   static NetObject *mDirtyList;
protected:
   SimObjectPtr<NetObject> mServerObject;

	enum NetFlag
	{ 
		IsGhost = 				BIT(1),	// This is a ghost
      ScopeAlways =        BIT(6),  // if set, object always ghosts to clientReps
      ScopeLocal =         BIT(7),  // Ghost only to local client 2
      Ghostable =          BIT(8),  // new flag -- set if this object CAN ghost
		MaxNetFlagBit =		15
	};
	BitSet32 mNetFlags;
   U32 mNetIndex;                   // the index of this ghost in the GhostManager on the server
   GhostInfo *mFirstObjectRef;
public:
	NetObject();
	~NetObject();

	// Base class provides IO for it's members but is
	// not declared as persitent.

   DECLARE_CONOBJECT(NetObject);
   static void initPersistFields();
   static void consoleInit();
   static void collapseDirtyList();
   bool onAdd();
   void onRemove();

   void setMaskBits(U32 orMask);
   void clearMaskBits(U32 orMask);

   void setScopeAlways();

   virtual F32 getUpdatePriority(CameraScopeQuery *focusObject, U32 updateMask, S32 updateSkips);
   virtual U32  packUpdate(NetConnection *, U32 mask, BitStream *stream);
   virtual void unpackUpdate(NetConnection *, BitStream *stream);
   virtual void onCameraScopeQuery(NetConnection *cr, CameraScopeQuery *camInfo);
   
   U32 getNetIndex() { return mNetIndex; }

	bool isServerObject() const;
   bool isClientObject() const;
   
   bool isGhost() const;
	bool isScopeLocal() const;
   bool isScopeable() const;
   bool isGhostAlways() const;
};

//-----------------------------------------------------------------------------

inline bool NetObject::isGhost() const
{
	return mNetFlags.test(IsGhost);
}

inline bool NetObject::isClientObject() const
{
	return mNetFlags.test(IsGhost);
}

inline bool NetObject::isServerObject() const
{
	return !mNetFlags.test(IsGhost);
}

inline bool NetObject::isScopeLocal() const
{
	return mNetFlags.test(ScopeLocal);
}

inline bool NetObject::isScopeable() const
{
	return mNetFlags.test(Ghostable) && !mNetFlags.test(ScopeAlways);
}

inline bool NetObject::isGhostAlways() const
{
   AssertFatal(mNetFlags.test(Ghostable) || mNetFlags.test(ScopeAlways) == false,
               "That's strange, a ScopeAlways non-ghostable object?  Something wrong here");
	return mNetFlags.test(Ghostable) && mNetFlags.test(ScopeAlways);
}

#endif
