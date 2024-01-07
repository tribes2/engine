//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _SCENEOBJECT_H_
#define _SCENEOBJECT_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif
#ifndef _NETOBJECT_H_
#include "sim/netObject.h"
#endif
#ifndef _COLLISION_H_
#include "collision/collision.h"
#endif
#ifndef _POLYHEDRON_H_
#include "collision/polyhedron.h"
#endif
#ifndef _ABSTRACTPOLYLIST_H_
#include "collision/abstractPolyList.h"
#endif
#ifndef _OBJECTTYPES_H_
#include "game/objectTypes.h"
#endif
#ifndef _COLOR_H_
#include "core/color.h"
#endif
#ifndef _LIGHTMANAGER_H_
#include "scenegraph/lightManager.h"
#endif

//-------------------------------------- Forward declarations...
class SceneObject;
class SceneGraph;
class SceneState;
class SceneRenderImage;
class Box3F;
class Point3F;
class LightManager;
class Convex;

//----------------------------------------------------------------------------
struct RayInfo: public Collision {
   // The collision struct has object, point, normal & material.
   F32 t;
};

//--------------------------------------------------------------------------
// There are two indiscretions here.  First is the name, which refers rather
//  blatantly to the container bin system.  A hygine issue.  Next is the
//  user defined U32, which is added solely for the zoning system.  This should
//  properly be split up into two structures, for the disparate purposes, especially
//  since it's not nice to force the container bin to use 20 bytes structures when
//  it could get away with a 16 byte version.
class SceneObjectRef
{
  public:
   SceneObject*    object;
   SceneObjectRef* nextInBin;
   SceneObjectRef* prevInBin;
   SceneObjectRef* nextInObj;

   U32             zone;
};

// A scope frustum describes a pyramid to clip new portals against.  It is
//  rooted at the root position of the scoping query, which is not stored
//  here.
class ScopeFrustum
{
  public:
   enum Constants {
      TopLeft     = 0,
      TopRight    = 1,
      BottomLeft  = 2,
      BottomRight = 3
   };

   Point3F frustumPoints[4];
};


//----------------------------------------------------------------------------
class Container
{
  public:
   struct Link
   {
      Link* next;
      Link* prev;
      Link();
      void unlink();
      void linkAfter(Link* ptr);
   };

   struct CallbackInfo {
      AbstractPolyList* polyList;
      Box3F boundingBox;
      SphereF boundingSphere;
      S32 key;
   };

   static const U32 csmNumBins;
   static const F32 csmBinSize;
   static const F32 csmTotalBinSize;
   static const U32 csmRefPoolBlockSize;
   static U32       smCurrSeqKey;

  private:
   Link mStart,mEnd;

   SceneObjectRef*         mFreeRefPool;
   Vector<SceneObjectRef*> mRefPoolBlocks;

   SceneObjectRef* mBinArray;
   SceneObjectRef  mOverflowBin;

  public:
   Container();
   ~Container();

   // Basic database operations
   typedef void (*FindCallback)(SceneObject*,S32 key);
   void findObjects(U32 mask, FindCallback, S32 key = 0);
   void findObjects(const Box3F& box, U32 mask, FindCallback, S32 key = 0);
   void polyhedronFindObjects(const Polyhedron& polyhedron, U32 mask,
                              FindCallback, S32 key = 0);

   // Line intersection
   bool castRay(const Point3F &start, const Point3F &end, U32 mask, RayInfo* info);
   bool collideBox(const Point3F &start, const Point3F &end, U32 mask, RayInfo* info);

   // Poly list
   bool buildPolyList(const Box3F& box, U32 mask, AbstractPolyList*,FindCallback=0,S32 key = 0);
   bool buildCollisionList(const Box3F& box, const Point3F& start, const Point3F& end, const VectorF& velocity,
      U32 mask,CollisionList* collisionList,FindCallback = 0,S32 key = 0,const Box3F *queryExpansion = 0);
   bool buildCollisionList(const Polyhedron& polyhedron,
                           const Point3F& start, const Point3F& end,
                           const VectorF& velocity,
                           U32 mask, CollisionList* collisionList,
                           FindCallback callback = 0, S32 key = 0);

   //
   bool addObject(SceneObject*);
   bool removeObject(SceneObject*);
   
   void addRefPoolBlock();
   SceneObjectRef* allocateObjectRef();
   void freeObjectRef(SceneObjectRef*);
   void insertIntoBins(SceneObject*);
   void removeFromBins(SceneObject*);

   // Checkbins makes sure that we're not just sticking the object right back
   //  where it came from.  The overloaded insertInto is so we don't calculate
   //  the ranges twice.
   void checkBins(SceneObject*);
   void insertIntoBins(SceneObject*, U32, U32, U32, U32);
   
   // Object searches to support console querying of the database.  ONLY WORKS ON SERVER
  private:
   Vector<SimObjectPtr<SceneObject>*>  mSearchList;
   S32                                 mCurrSearchPos;
   Point3F                             mSearchReferencePoint;
   void cleanupSearchVectors();

  public:
   void initRadiusSearch(const Point3F& searchPoint,
                         const F32      searchRadius,
                         const U32      searchMask);
   U32  containerSearchNext();
   F32  containerSearchCurrDist();
   F32  containerSearchCurrRadDamageDist();
};


//----------------------------------------------------------------------------
// For simple queries.  Simply creates a vector of the objects
//
class SimpleQueryList
{
  public:
   Vector<SceneObject*> mList;

  public:
   SimpleQueryList() {
      VECTOR_SET_ASSOCIATION(mList);
   }

   void insertObject(SceneObject* obj) { mList.push_back(obj); }
   static void insertionCallback(SceneObject* obj, S32 key);
};


//----------------------------------------------------------------------------
class SceneObject : public NetObject, public Container::Link
{
   typedef NetObject Parent;
   friend class Container;
   friend class SceneGraph;
   friend class SceneState;

   //-------------------------------------- Public constants
  public:
   enum {
      MaxObjectZones = 128
   };
   enum TraverseColor {
      White = 0,
      Grey  = 1,
      Black = 2
   };

   //-------------------------------------- Public interfaces
   // C'tors and D'tors
  private:
   SceneObject(const SceneObject&); // disallowed
  public:
   SceneObject();
   virtual ~SceneObject();

   // Scripting interface
   const char* scriptThis();

   // Collision and transform related interface
  public:
   virtual void disableCollision();
   virtual void enableCollision();
   bool         isCollisionEnabled() const { return mCollisionCount == 0; }

   virtual bool    isDisplacable() const;
   virtual Point3F getMomentum() const;
   virtual void    setMomentum(const Point3F&);
   virtual F32     getMass() const;
   virtual bool    displaceObject(const Point3F& displaceVector);

   const MatrixF& getTransform() const      { return mObjToWorld; }
   const MatrixF& getWorldTransform() const { return mWorldToObj; }
   const VectorF& getScale() const          { return mObjScale;   }

   const Box3F&   getObjBox() const        { return mObjBox;      }
   const Box3F&   getWorldBox() const      { return mWorldBox;    }
   const SphereF& getWorldSphere() const   { return mWorldSphere; }
   Point3F        getBoxCenter() const     { return (mWorldBox.min + mWorldBox.max) * 0.5f; }
   
   virtual void setTransform(const MatrixF&);
   virtual void setScale(const VectorF & scale);

   virtual void   setRenderTransform(const MatrixF&);
   const MatrixF& getRenderTransform() const      { return mRenderObjToWorld; }
   const MatrixF& getRenderWorldTransform() const { return mRenderWorldToObj; }
   const Box3F&   getRenderWorldBox()  const      { return mRenderWorldBox;   }

   virtual void     buildConvex(const Box3F& box,Convex* convex);
   virtual bool     buildPolyList(AbstractPolyList* polyList, const Box3F &box, const SphereF &sphere);
   virtual BSPNode* buildCollisionBSP(BSPTree *tree, const Box3F &box, const SphereF &sphere);

   virtual bool castRay(const Point3F &start, const Point3F &end, RayInfo* info);
   virtual bool collideBox(const Point3F &start, const Point3F &end, RayInfo* info);

   Point3F  getPosition() const;
   Point3F  getRenderPosition() const;
   void     setPosition(const Point3F &pos);

   // Rendering and zone interface
  public:
   bool isManagingZones() const;
   U32  getZoneRangeStart() const                         { return mZoneRangeStart;   }
   U32  getNumCurrZones() const                           { return mNumCurrZones;     }
   U32  getCurrZone(const U32 index) const;

   virtual bool getOverlappingZones(SceneObject* obj, U32* zones, U32* numZones);
   virtual U32  getPointZone(const Point3F& p);

   virtual void renderObject(SceneState*, SceneRenderImage*);
   virtual bool prepRenderImage(SceneState*, const U32 stateKey, const U32 startZone,
                                const bool modifyBaseZoneState = false);
   virtual bool scopeObject(const Point3F&        rootPosition,
                            const F32             rootDistance,
                            bool*                 zoneScopeState);

   void addToScene();
   void removeFromScene();

   //-------------------------------------- Derived class interface
   // Overrides
  protected:
   bool onAdd();
   void onRemove();
   void inspectPostApply();
   
   // Overrideables
  protected:
   virtual bool onSceneAdd(SceneGraph*);
   virtual void onSceneRemove();

   // Portal functions.
   virtual void transformModelview(const U32 portalIndex, const MatrixF& oldMV, MatrixF* newMV);
   virtual void transformPosition(const U32 portalIndex, Point3F& point);
   virtual bool computeNewFrustum(const U32      portalIndex,
                                  const F64*     oldFrustum,
                                  const F64      nearPlane,
                                  const F64      farPlane,
                                  const RectI&   oldViewport,
                                  F64*           newFrustum,
                                  RectI&         newViewport,
                                  const bool     flippedMatrix);
   virtual void openPortal(const U32   portalIndex,
                           SceneState* pCurrState,
                           SceneState* pParentState);
   virtual void closePortal(const U32   portalIndex,
                            SceneState* pCurrState,
                            SceneState* pParentState);
  public:
   virtual void getWSPortalPlane(const U32 portalIndex, PlaneF*);

  protected:
   void          setLastState(SceneState*, U32);
   bool          isLastState(SceneState*, U32) const;
   void          setTraverseColor(TraverseColor);
   TraverseColor getTraverseColor() const;

   // lighting info:
   struct LightingInfo 
   {
      LightingInfo();

      bool                       mUseInfo;
      bool                       mDirty;   
      ColorF                     mDefaultColor;
      ColorF                     mAlarmColor;

      SimObjectPtr<SceneObject>  mInterior;

      bool                       mHasLastColor;
      ColorF                     mLastColor;
      U32                        mLastTime;

      static LightInfo           smAmbientLight;

      enum {
         Interior = 0,
         Terrain,
      };
      U32                        mLightSource;
   };

   virtual void installLights();
   virtual void uninstallLights();
   virtual bool getLightingAmbientColor(ColorF * col);
   LightingInfo      mLightingInfo;

   // Transform and Collision related members
  protected:
   Container* mContainer;

   MatrixF mObjToWorld;   // object transform
   MatrixF mWorldToObj;   // inverse transform
   Point3F mObjScale;     // object scale

   Box3F   mObjBox;
   Box3F   mWorldBox;
   SphereF mWorldSphere;

   MatrixF mRenderObjToWorld;
   MatrixF mRenderWorldToObj;
   Box3F   mRenderWorldBox;
   SphereF mRenderWorldSphere;
   
   void    resetWorldBox();
   void    resetRenderWorldBox();

   SceneObjectRef* mZoneRefHead;
   SceneObjectRef* mBinRefHead;

   U32 mBinMinX;
   U32 mBinMaxX;
   U32 mBinMinY;
   U32 mBinMaxY;

   U32  mContainerSeqKey;
   U32  getContainerSeqKey() const        { return mContainerSeqKey; }
   void setContainerSeqKey(const U32 key) { mContainerSeqKey = key;  }

public:
   Container* getContainer()         { return mContainer;       }

  protected:
   S32     mCollisionCount;

  public:   
   U32 getTypeMask() { return(mTypeMask); }

   // Rendering related members
  protected:
   SceneGraph* mSceneManager;
   U32         mZoneRangeStart;    // 0xFFFFFFFF == no zones

//   U32          mCurrZones[MaxObjectZones];
//   SceneObject* mCurrZoneOwners[MaxObjectZones];
   U32          mNumCurrZones;

  private:
   TraverseColor mTraverseColor;
   SceneState*   mLastState;
   U32           mLastStateKey;


   // Persist and console
  public:
   static void initPersistFields();
   static void consoleInit();

   DECLARE_CONOBJECT(SceneObject);
};


//--------------------------------------------------------------------------
extern Container gServerContainer;
extern Container gClientContainer;

//--------------------------------------------------------------------------
//-------------------------------------- Inlines
//
inline bool SceneObject::isManagingZones() const
{
   return mZoneRangeStart != 0xFFFFFFFF;
}

inline void SceneObject::setLastState(SceneState* state, U32 key)
{
   mLastState    = state;
   mLastStateKey = key;
}

inline bool SceneObject::isLastState(SceneState* state, U32 key) const
{
   return (mLastState == state && mLastStateKey == key);
}

inline void  SceneObject::setTraverseColor(TraverseColor c)
{
   mTraverseColor = c;
}

inline SceneObject::TraverseColor SceneObject::getTraverseColor() const
{
   return mTraverseColor;
}

inline U32 SceneObject::getCurrZone(const U32 index) const
{
   // Not the most efficient way to do this, walking the list,
   //  but it's an uncommon call...
   SceneObjectRef* walk = mZoneRefHead;
   for (U32 i = 0; i < index; i++) {
      walk = walk->nextInObj;
      AssertFatal(walk, "Error, too few object refs!");
   }
   AssertFatal(walk, "Error, too few object refs!");

   return walk->zone;
}


//--------------------------------------------------------------------------
inline SceneObjectRef* Container::allocateObjectRef()
{
   if (mFreeRefPool == NULL) {
      addRefPoolBlock();
   }
   AssertFatal(mFreeRefPool, "Error, should always have a free reference here!");

   SceneObjectRef* ret = mFreeRefPool;
   mFreeRefPool = mFreeRefPool->nextInObj;

   ret->nextInObj = NULL;
   return ret;
}

inline void Container::freeObjectRef(SceneObjectRef* trash)
{
   trash->nextInBin = NULL;
   trash->prevInBin = NULL;
   trash->nextInObj = mFreeRefPool;
   mFreeRefPool     = trash;
}

inline void Container::findObjects(U32 mask, FindCallback callback, S32 key)
{
   for (Link* itr = mStart.next; itr != &mEnd; itr = itr->next) {
      SceneObject* ptr = static_cast<SceneObject*>(itr);
      if ((ptr->getType() & mask) != 0 && !ptr->mCollisionCount)
         (*callback)(ptr,key);
   }
}

#endif  // _H_SCENEOBJECT_

