//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _TSSHAPE_H_
#define _TSSHAPE_H_

#ifndef _TSMESH_H_
#include "ts/tsMesh.h"
#endif
#ifndef _TSDECAL_H_
#include "ts/tsDecal.h"
#endif
#ifndef _TSINTEGERSET_H_
#include "ts/tsIntegerSet.h"
#endif
#ifndef _TSTRANSFORM_H_
#include "ts/tsTransform.h"
#endif
#ifndef _TSSHAPEALLOC_H_
#include "ts/tsShapeAlloc.h"
#endif
#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif
#ifndef _RESMANAGER_H_
#include "Core/resManager.h"
#endif
#ifndef _MMATH_H_
#include "Math/mMath.h"
#endif
#ifndef _STREAM_H_
#include "Core/stream.h"
#endif

#define DTS_EXPORTER_CURRENT_VERSION 123

class TSMaterialList;
class TSLastDetail;

class TSShape : public ResourceInstance
{
  public:
      enum { UniformScale=0x01, AlignedScale=0x02, ArbitraryScale=0x04,
             Blend=0x08, Cyclic=0x10, MakePath=0x20,
             IflInit=0x40, HasTranslucency=0x80,
             AnyScale=UniformScale|AlignedScale|ArbitraryScale };

   // Nodes hold the transforms in the shape tree...
   struct Node
   {
      S32 nameIndex;
      S32 parentIndex;

      // computed at runtime
      S32 firstObject;
      S32 firstChild;
      S32 nextSibling;
   };
   
   // Objects hold renderable items (in particular meshes)...
   // Each object has a number of meshes associated with it.
   // Each mesh corresponds to a different detail level.
   // "meshIndicesIndex" points to numMeshes consecutive indices
   // into the meshList and meshType vectors.  It indexes the
   // meshIndexList vector (meshIndexList is merely a clearinghouse
   // for the object's mesh lists).  Some indices may correspond to
   // no mesh -- which means no mesh will be drawn for the part for
   // the given detail level.  See comments on the meshIndexList
   // for how null meshes are coded.
   // Note:  stored this way so that there is no address specific information.
   struct Object
   {
      S32 nameIndex;
      S32 numMeshes;
      S32 startMeshIndex; // index into meshes array...
      S32 nodeIndex;
      
      // computed at load
      S32 nextSibling;
      S32 firstDecal;
   };

   // Decals hang off objects like objects hang off nodes.  A decal is rendered on
   // top of an object (normally will be translucent).  Note:  they hang off objects
   // conceptually...however, in the shapeInstance they are in their own list and
   // that list is rendered after all the objects are.
   struct Decal
   {
      S32 nameIndex;
      S32 numMeshes;
      S32 startMeshIndex; // index into meshes array...
      S32 objectIndex;
      
      // computed at load
      S32 nextSibling;
   };
   
   // IFL materials are used to animate material lists -- i.e., run through a series
   // of frames of a material... they work by replacing a material in the material
   // list so that it is transparent to the rest of the code.
   // Offset time of each frame is stored in iflFrameOffsets vector, starting at index position
   // firstFrameOffsetIndex..
   struct IflMaterial
   {
      S32 nameIndex; // file name w/ extension
      S32 materialSlot;
      S32 firstFrame;
      S32 firstFrameOffTimeIndex;
      S32 numFrames;
   };

   // A Sequence holds all the information necessary to perform a particular animation (sequence).
   // Sequences index a range of keyframes...keyframes are assumed to be equally spaced in time.
   // Each node and object is either a member of the sequence or not.  If not, they are set to
   // default values when we switch to the sequence unless they are members of some other active sequence.
   // Blended sequences "add" a transform to the current transform of a node.  Any object animation of
   // a blended sequence over-rides any existing object state.  Blended sequences are always
   // applied after non-blended sequences.
   struct Sequence
   {
      S32 nameIndex;
      S32 numKeyframes;
      F32 duration;
      S32 baseRotation;
      S32 baseTranslation;
      S32 baseScale;
      S32 baseObjectState;
      S32 baseDecalState;
      S32 firstGroundFrame;
      S32 numGroundFrames;
      S32 firstTrigger;
      S32 numTriggers;
      F32 toolBegin;

      // These bitsets code whether this sequence cares certain aspects of animation
      // e.g., the rotation, translation, or scale of node transforms,
      // or the visibility, frame or material frame of objects.
      TSIntegerSet rotationMatters;     // set of nodes
      TSIntegerSet translationMatters;  // set of nodes
      TSIntegerSet scaleMatters;        // set of nodes
      TSIntegerSet visMatters;          // set of objects
      TSIntegerSet frameMatters;        // set of objects
      TSIntegerSet matFrameMatters;     // set of objects
      TSIntegerSet decalMatters;        // set of decals
      TSIntegerSet iflMatters;          // set of ifls

      S32 priority;
      U32 flags;
      U32 dirtyFlags; // determined at load time

      bool testFlags(U32 comp) const      { return (flags&comp)!=0; }
      bool animatesScale() const          { return testFlags(AnyScale); }
      bool animatesUniformScale() const   { return testFlags(UniformScale); }
      bool animatesAlignedScale() const   { return testFlags(AlignedScale); }
      bool animatesArbitraryScale() const { return testFlags(ArbitraryScale); }
      bool isBlend() const                { return testFlags(Blend); }
      bool isCyclic() const               { return testFlags(Cyclic); }
      bool makePath() const               { return testFlags(MakePath); }

      void read(Stream *, bool readNameIndex = true);
      void write(Stream *, bool writeNameIndex = true);
   };

   // Describes state of an individual object.  Includes everything in an object that can be
   // controlled by animation.
   struct ObjectState
   {
      F32 vis;
      S32 frameIndex;
      S32 matFrameIndex;
   };
   
   // Describes state of a decal.
   struct DecalState
   {
      S32 frameIndex;
   };
   
   // When time on a sequence advances past a certain point, a trigger takes effect and changes
   // one of the state variables to on or off...(state variables found on shape instance mTriggerStates)
   struct Trigger
   {
      enum { StateOn = 1 << 31, InvertOnReverse = 1 << 30, StateMask = 0x1f };

      U32 state; // see above enum
      F32 pos;
   };

   // Details are used for render detail selection.  As the projected size of the shape changes,
   // a different node structure can be used (subShape) and a different objectDetail can be selected
   // for each object drawn.   Either of these two parameters can also stay constant, but presumably
   // not both.  If size is negative then the detail level will never be selected by the standard
   // detail selection process.  It will have to be selected by name.  Such details are "utility
   // details" because they exist to hold data (node positions or collision information) but not
   // normally to be drawn.  By default there will always be a "Ground" utility detail.
   struct Detail
   {
      S32 nameIndex;
      S32 subShapeNum;
      S32 objectDetailNum;
      F32 size;
      F32 averageError;
      F32 maxError;
      S32 polyCount;
   };

   // For speeding up buildpolylist and support calls.
   struct ConvexHullAccelerator {
      U32      numVerts;
      Point3F* vertexList;
      Point3F* normalList;
      U8**     emitStrings;
   };
   ConvexHullAccelerator* getAccelerator(S32 dl);
   
   // shape vector data...non-resizable in game
   ToolVector<Node> nodes;
   ToolVector<Object> objects;
   ToolVector<Decal> decals;
   ToolVector<IflMaterial> iflMaterials;
   ToolVector<ObjectState> objectStates;
   ToolVector<DecalState> decalStates;
   ToolVector<S32> subShapeFirstNode;
   ToolVector<S32> subShapeFirstObject;
   ToolVector<S32> subShapeFirstDecal;
   ToolVector<S32> detailFirstSkin;
   ToolVector<S32> subShapeNumNodes;
   ToolVector<S32> subShapeNumObjects;
   ToolVector<S32> subShapeNumDecals;
   ToolVector<Detail> details;
   ToolVector<Quat16> defaultRotations;
   ToolVector<Point3F> defaultTranslations;

   // set up at load time...but memory allocated along with loaded data
   ToolVector<S32> subShapeFirstTranslucentObject;
   ToolVector<TSMesh*> meshes;
   ToolVector<F32> alphaIn;          // these vectors describe how to transition between detail
   ToolVector<F32> alphaOut;         // levels using alpha..."alpha-in" next detail as intraDL goes
                                     // from alphaIn+alphaOut to alphaOut..."alpha-out" current 
                                     // detail level as intraDL goes from alphaOut to 0.
                                     // NOTE:
                                     //   intraDL is at 1 when if shape were any closer to us we'd be at dl-1,
                                     //   intraDL is at 0 when if shape were any farther away we'd be at dl+1

   // re-sizeable vectors...
   Vector<Sequence> sequences;
   Vector<Quat16> nodeRotations;
   Vector<Point3F> nodeTranslations;
   Vector<F32> nodeUniformScales;
   Vector<Point3F> nodeAlignedScales;
   Vector<Quat16> nodeArbitraryScaleRots;
   Vector<Point3F> nodeArbitraryScaleFactors;
   Vector<Quat16> groundRotations;
   Vector<Point3F> groundTranslations;
   Vector<Trigger> triggers;
   Vector<F32> iflFrameOffTimes;
   Vector<TSLastDetail*> billboardDetails;
   Vector<ConvexHullAccelerator*> detailCollisionAccelerators;
   Vector<const char *> names;

   // most vectors are stored in a single memory block
   // except when compiled using MAX_UTIL defined
   // in that case, ToolVector becomes Vector<> and the
   // vectors are resizeable
   S8 * mMemoryBlock;

   TSMaterialList * materialList;

   // bounding information
   F32 radius;
   F32 tubeRadius;
   Point3F center;
   Box3F bounds;

   // various...
   U32 mExporterVersion;
   F32 mSmallestVisibleSize; // computed at load time from details vector
   S32 mSmallestVisibleDL;   // ditto
   S32 mReadVersion; // file version that this shape was read from
   U32 mFlags; // hasTranslucancy, iflInit
   U32 data; // let the user do whatever with this

   bool mSequencesConstructed;
   S32 mVertexBuffer;
   U32 mCallbackKey;
	bool mExportMerge;
	bool mMorphable;
	Vector<S32> mPreviousMerge;
   S32 mMergeBufferSize;
   
   // shape class has few methods --
   // just constructor/destructor, io, and lookup methods

   // constructor/destructor
   TSShape();
   ~TSShape();
   void init();
   void initMaterialList(); // you can swap in a new material list, but call this if you do
   void clearDynamicData();
   void setupBillboardDetails();

   bool getSequencesConstructed() const { return mSequencesConstructed; }
   void setSequencesConstructed(const bool c) { mSequencesConstructed = c; }
   
   // look up animation info -- indexed by keyframe number and offset (which objecct/node/decal
   // of the animated objects/nodes/decals you want information for).
   QuatF & getRotation(const Sequence & seq, S32 keyframeNum, S32 rotNum, QuatF *) const;
   const Point3F & getTranslation(const Sequence & seq, S32 keyframeNum, S32 tranNum) const;
   F32 getUniformScale(const Sequence & seq, S32 keyframeNum, S32 scaleNum) const;
   const Point3F & getAlignedScale(const Sequence & seq, S32 keyframeNum, S32 scaleNum) const;
   TSScale & getArbitraryScale(const Sequence & seq, S32 keyframeNum, S32 scaleNum, TSScale *) const;
   const ObjectState & getObjectState(const Sequence & seq, S32 keyframeNum, S32 objectNum) const;
   const DecalState & getDecalState(const Sequence & seq, S32 keyframeNum, S32 decalNum) const;
   
   // build LOS collision detail
   void computeAccelerator(S32 dl);
   bool buildConvexHull(S32 dl) const;
   void computeBounds(S32 dl, Box3F & bounds) const; // uses default transforms to compute bounding box around a detail level
                                                     // see like named method on shapeInstance if you want to use animated transforms

   // lookup methods
   S32 findName(const char *) const;
   const char * getName(S32) const;

   S32 findNode(S32 nameIndex) const;
   S32 findNode(const char * name) const { return findNode(findName(name)); }

   S32 findObject(S32 nameIndex) const;
   S32 findObject(const char * name) const { return findObject(findName(name)); }
   
   S32 findDecal(S32 nameIndex) const;
   S32 findDecal(const char * name) const { return findDecal(findName(name)); }
   
   S32 findIflMaterial(S32 nameIndex) const;
   S32 findIflMaterial(const char * name) const { return findIflMaterial(findName(name)); }

   S32 findDetail(S32 nameIndex) const;
   S32 findDetail(const char * name) const { return findDetail(findName(name)); }

   S32 findSequence(S32 nameIndex) const;
   S32 findSequence(const char * name) const { return findSequence(findName(name)); }

   bool hasTranslucency() const { return mFlags & HasTranslucency; }

   // these control default values for alpha transitions between detail levels
   static F32 smAlphaOutLastDetail;
   static F32 smAlphaInBillboard;
   static F32 smAlphaOutBillboard;
   static F32 smAlphaInDefault;
   static F32 smAlphaOutDefault;

   // don't load this many of the highest detail levels (although we always
   // load one renderable detail if there is one)
   static S32 smNumSkipLoadDetails;

   // by default we initialize shape when we read...
   static bool smInitOnRead;

   // version info
   // -- smVersion is most recent version...the one we write
   // -- smReadVersion is version currently being read
   //    smReadVersion is only valid during a read
   static S32 smVersion;
   static S32 smReadVersion;
   static const U32 smMostRecentExporterVersion;

   // persist methods
   void write(Stream *);
   bool read(Stream *);
   void readOldShape(Stream * s, S32 * &, S16 * &, S8 * &, S32 &, S32 &, S32 &);
   void writeName(Stream *, S32 nameIndex);
   S32  readName(Stream *, bool addName);

   void exportSequences(Stream *);
   bool importSequences(Stream *);

   void readIflMaterials();

   // persist helper functions
   static TSShapeAlloc alloc;
   void fixEndian(S32 *, S16 *, S8 *, S32, S32, S32);

   // memory buffer transfer methods (uses TSShape::Alloc structure)
   void assembleShape();
   void disassembleShape();

   // mem buffer transfer helper (indicate when we don't want to include a particular mesh/decal)
   bool checkSkip(S32 meshNum, S32 & curObject, S32 & curDecal, S32 skipDL);

   // used when reading old shapes/sequences
   void rearrangeKeyframeData(Sequence &, S32 keyframeStart, U8 * pns32 = NULL, U8 * pns16 = NULL, U8 * pos = NULL, U8 * pds = NULL, S32 szNS32=-1, S32 szNS16=-1, S32 szOS32=-1, S32 szDS32=-1);
   void rearrangeStates(S32 start, S32 rows, S32 cols, U8 * data, S32 size);

   void fixupOldSkins(S32 numMeshes, S32 numSkins, S32 numDetails, S32 * detailFirstSkin, S32 * detailNumSkins);
};

class TSMaterialList : public MaterialList
{
   typedef MaterialList Parent;
   
   Vector<U32> mFlags;
   Vector<U32> mReflectanceMaps;
   Vector<U32> mBumpMaps;
   Vector<U32> mDetailMaps;
   Vector<F32> mDetailScales;
   Vector<F32> mReflectionAmounts;
   
   bool mNamesTransformed;

   void allocate(U32 sz);

  public:
   static const char* csmTSTexturePrefix;     // currently "skins/", set in tsShape.cc
   static const char* csmOldTSTexturePrefix;  // currently "", set in tsShape.cc
   static const char* csmIFLTexturePrefix;

  public:
   
   enum
   {
      S_Wrap             = 1 << 0,
      T_Wrap             = 1 << 1,
      Translucent        = 1 << 2,
      Additive           = 1 << 3,
      Subtractive        = 1 << 4,
      SelfIlluminating   = 1 << 5,
      NeverEnvMap        = 1 << 6,
      NoMipMap           = 1 << 7,
      MipMap_ZeroBorder  = 1 << 8,
      IflMaterial        = 1 << 27,
      IflFrame           = 1 << 28,
      DetailMapOnly      = 1 << 29,
      BumpMapOnly        = 1 << 30,
      ReflectanceMapOnly = 1 << 31,
      AuxiliaryMap       = DetailMapOnly | BumpMapOnly | ReflectanceMapOnly
   };

   TSMaterialList(U32 materialCount, const char **materialNames, const U32 * materialFlags,
                  const U32 * reflectanceMaps, const U32 * bumpMaps, const U32 * detailMaps,
                  const F32 * detailScales, const F32 * reflectionAmounts);
   TSMaterialList();
   TSMaterialList(const TSMaterialList*);
   ~TSMaterialList();
   void free();

   void tsmlTransform();
   
   void load(U32 index);
   bool load(TextureHandleType type, bool clampToEdge = false) { return Parent::load(type,clampToEdge); }

   TextureHandle * getReflectionMap(U32 index) { return mReflectanceMaps[index] == 0xFFFFFFFF ? NULL : &getMaterial(mReflectanceMaps[index]); }
   F32 getReflectionAmount(U32 index) { return mReflectionAmounts[index]; }
   TextureHandle * getBumpMap(U32 index) { return mBumpMaps[index] == 0xFFFFFFFF ? NULL : &getMaterial(mBumpMaps[index]); }
   TextureHandle * getDetailMap(U32 index) { return mDetailMaps[index] == 0xFFFFFFFF ? NULL : &getMaterial(mDetailMaps[index]); }
   F32 getDetailMapScale(U32 index) { return mDetailScales[index]; }
   bool reflectionInAlpha(U32 index) { return mReflectanceMaps[index] == index; }

   U32 getFlags(U32 index);
   void setFlags(U32 index, U32 value);
   
   void remap(U32 toIndex, U32 fromIndex); // support for ifl sequences

   // pre-load only ... support for ifl sequences   
   void push_back(const char * name, U32 flags,
                  U32 a=0xFFFFFFFF, U32 b=0xFFFFFFFF, U32 c=0xFFFFFFFF,
                  F32 dm=1.0f, F32 em=1.0f); 

   bool write(Stream &);
   bool read(Stream &);
};

extern ResourceInstance *constructTSShape(Stream &stream);

#define TSNode TSShape::Node
#define TSObject TSShape::Object
#define TSDecal TSShape::Decal
#define TSSequence TSShape::Sequence
#define TSDetail TSShape::Detail

inline QuatF & TSShape::getRotation(const Sequence & seq, S32 keyframeNum, S32 rotNum, QuatF * quat) const
{
   return nodeRotations[seq.baseRotation + rotNum*seq.numKeyframes + keyframeNum].getQuatF(quat);
}

inline const Point3F & TSShape::getTranslation(const Sequence & seq, S32 keyframeNum, S32 tranNum) const
{
   return nodeTranslations[seq.baseTranslation + tranNum*seq.numKeyframes + keyframeNum];
}

inline F32 TSShape::getUniformScale(const Sequence & seq, S32 keyframeNum, S32 scaleNum) const
{
   return nodeUniformScales[seq.baseScale + scaleNum*seq.numKeyframes + keyframeNum];
}

inline const Point3F & TSShape::getAlignedScale(const Sequence & seq, S32 keyframeNum, S32 scaleNum) const
{
   return nodeAlignedScales[seq.baseScale + scaleNum*seq.numKeyframes + keyframeNum];
}

inline TSScale & TSShape::getArbitraryScale(const Sequence & seq, S32 keyframeNum, S32 scaleNum, TSScale * scale) const
{
   nodeArbitraryScaleRots[seq.baseScale + scaleNum*seq.numKeyframes + keyframeNum].getQuatF(&scale->mRotate);
   scale->mScale = nodeArbitraryScaleFactors[seq.baseScale + scaleNum*seq.numKeyframes + keyframeNum];
   return *scale;
}

inline const TSShape::ObjectState & TSShape::getObjectState(const TSSequence & seq, S32 keyframeNum, S32 objectNum) const
{
   return objectStates[seq.baseObjectState + objectNum*seq.numKeyframes + keyframeNum];
}

inline const TSShape::DecalState & TSShape::getDecalState(const TSSequence & seq, S32 keyframeNum, S32 decalNum) const
{
   return decalStates[seq.baseDecalState + decalNum*seq.numKeyframes + keyframeNum];
}

#endif
