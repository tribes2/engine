//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _TERRAINEDITOR_H_
#define _TERRAINEDITOR_H_

#ifndef _EDITTSCTRL_H_
#include "Editor/editTSCtrl.h"
#endif
#ifndef _TERRDATA_H_
#include "terrain/terrData.h"
#endif

//------------------------------------------------------------------------------

class GridInfo
{
   public:
      Point2I                    mGridPos;
      TerrainBlock::Material     mMaterial;
      F32                        mHeight;
      U8                         mMaterialGroup;
      F32                        mWeight;

      bool                       mPrimarySelect;
   
      // hash table
      S32                        mNext;
      S32                        mPrev;
};

//------------------------------------------------------------------------------

class Selection : public Vector<GridInfo>
{
   private:

      StringTableEntry     mName;
      BitSet32             mUndoFlags;
      
      // hash table
      S32 lookup(const Point2I & pos);
      void insert(GridInfo & info);
      U32 getHashIndex(const Point2I & pos);

      Vector<S32>          mHashLists;
      U32                  mHashListSize;

   public:
      
      Selection();
      
      void reset();
      bool add(GridInfo & info);
      bool getInfo(Point2I pos, GridInfo & info);
      bool setInfo(GridInfo & info);
      bool remove(const GridInfo & info);
      void setName(StringTableEntry name);
      StringTableEntry getName(){return(mName);}
      F32 getAvgHeight();
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

class TerrainEditor;
class Brush : public Selection
{
   protected:
      TerrainEditor *   mTerrainEditor;
      Point2I           mSize;
      Point2I           mGridPos;
      
   public:

      enum {
         MaxBrushDim    =  40
      };
      
      Brush(TerrainEditor * editor);
      virtual ~Brush(){};
      
      //
      void setPosition(const Point3F & pos);
      void setPosition(const Point2I & pos);
      const Point2I & getPosition();

      void update();
      virtual void rebuild() = 0;
      
      Point2I getSize(){return(mSize);}
      virtual void setSize(const Point2I & size){mSize = size;}
};

class BoxBrush : public Brush
{
   public:
      BoxBrush(TerrainEditor * editor) : Brush(editor){}
      void rebuild();
};

class EllipseBrush : public Brush
{
   public:
      EllipseBrush(TerrainEditor * editor) : Brush(editor){}
      void rebuild();
};

class SelectionBrush : public Brush
{
   public:
      SelectionBrush(TerrainEditor * editor);
      void rebuild();
      void setSize(const Point2I &){}
};
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

// must declare the console functions before making them a friend of TerrainEditor
static void cAttachTerrain(SimObject *, S32, const char **);
static void cSetBrushType(SimObject *, S32, const char **);
static void cSetBrushSize(SimObject *, S32, const char **);
static const char * cGetBrushPos(SimObject *, S32, const char **);
static void cSetBrushPos(SimObject *, S32, const char **);
static void cSetAction(SimObject *, S32, const char **);
static S32 cGetNumActions(SimObject *, S32, const char **);
static const char * cGetActionName(SimObject *, S32, const char **);
static const char * cGetCurrentAction(SimObject *, S32, const char **);
static void cResetSelWeights(SimObject *, S32, const char **);
static void cUndoAction(SimObject *, S32, const char **);
static void cRedoAction(SimObject *, S32, const char **);
static void cClearSelection(SimObject *, S32, const char **);
static void cProcessAction(SimObject *, S32, const char **);
static void cBuildMaterialMap(SimObject * obj, S32, const char **);
static S32 cGetNumTextures(SimObject * obj, S32, const char **);
static const char * cGetTextureName(SimObject * obj, S32, const char ** argv);
static void cMarkEmptySquares(SimObject * obj, S32, const char **);
static void cClearModifiedFlags(SimObject * obj, S32, const char **);
static void cMirrorTerrain(SimObject * obj, S32, const char **);
static void cPushBaseMaterialInfo(SimObject * obj, S32, const char **);
static void cPopBaseMaterialInfo(SimObject * obj, S32, const char **);
static void cSetLoneBaseMaterial(SimObject * obj, S32, const char ** argv);

struct BaseMaterialInfo {
   StringTableEntry     mMaterialNames[TerrainBlock::MaterialGroups];
   U8                   mBaseMaterials[TerrainBlock::BlockSize * TerrainBlock::BlockSize];
};

class TerrainAction;
class TerrainEditor : public EditTSCtrl
{
   private:
   
      // give console functions full access
      friend void cAttachTerrain(SimObject *, S32, const char **);
      friend void cSetBrushType(SimObject *, S32, const char **);
      friend void cSetBrushSize(SimObject *, S32, const char **);
      friend const char * cGetBrushPos(SimObject *, S32, const char **);
      friend void cSetBrushPos(SimObject *, S32, const char **);
      friend void cSetAction(SimObject *, S32, const char **);
      friend S32 cGetNumActions(SimObject *, S32, const char **);
      friend const char * cGetActionName(SimObject *, S32, const char **);
      friend const char * cGetCurrentAction(SimObject *, S32, const char **);
      friend void cResetSelWeights(SimObject *, S32, const char **);   
      friend void cUndoAction(SimObject *, S32, const char **);
      friend void cRedoAction(SimObject *, S32, const char **);
      friend void cClearSelection(SimObject *, S32, const char**);
      friend void cProcessAction(SimObject *, S32, const char **);
      friend void cBuildMaterialMap(SimObject * obj, S32, const char **);
      friend S32 cGetNumTextures(SimObject * obj, S32, const char **);
      friend const char * cGetTextureName(SimObject * obj, S32, const char ** argv);
      friend void cMarkEmptySquares(SimObject * obj, S32, const char **);
      friend void cClearModifiedFlags(SimObject * obj, S32, const char **);
      friend void cMirrorTerrain(SimObject * obj, S32, const char **);
      friend void cPushBaseMaterialInfo(SimObject * obj, S32, const char **);
      friend void cPopBaseMaterialInfo(SimObject * obj, S32, const char **);
      friend void cSetLoneBaseMaterial(SimObject * obj, S32, const char ** argv);

      typedef EditTSCtrl Parent;
      TerrainBlock * mTerrainBlock;

      Point3F                    mMousePos;
      Brush *                    mMouseBrush;
      bool                       mRenderBrush;
      Point2I                    mBrushSize;
      Vector<TerrainAction *>    mActions;
      TerrainAction *            mCurrentAction;
      bool                       mInAction;
      Selection                  mDefaultSel;
      bool                       mSelectionLocked;
      GuiCursor *                mDefaultCursor;
      Selection *                mCurrentSel;

      //
      bool                       mRebuildEmpty;
      bool                       mRebuildTextures;
      void rebuild();

      void addUndo(Vector<Selection *> & list, Selection * sel);
      bool processUndo(Vector<Selection *> & src, Vector<Selection *> & dest);
      void clearUndo(Vector<Selection *> & list);

      U32                        mUndoLimit;
      Selection *                mUndoSel;
      
      Vector<Selection*>         mUndoList;
      Vector<Selection*>         mRedoList;

      Vector<BaseMaterialInfo*>  mBaseMaterialInfos;
   public:

      TerrainEditor();
      ~TerrainEditor();

      // conversion functions 
      bool gridToWorld(const Point2I & gPos, Point3F & wPos);
      bool worldToGrid(const Point3F & wPos, Point2I & gPos);
      bool gridToCenter(const Point2I & gPos, Point2I & cPos);

      bool getGridInfo(const Point3F & wPos, GridInfo & info);
      bool getGridInfo(const Point2I & gPos, GridInfo & info);
      void setGridInfo(const GridInfo & info);

      bool collide(const Gui3DMouseEvent & event, Point3F & pos);
      void lockSelection(bool lock) { mSelectionLocked = lock; };

      Selection * getUndoSel(){return(mUndoSel);}
      Selection * getCurrentSel(){return(mCurrentSel);}
      void setCurrentSel(Selection * sel) { mCurrentSel = sel; }
      void resetCurrentSel() {mCurrentSel = &mDefaultSel; }

      Point2I getBrushSize() { return(mBrushSize); }      
      TerrainBlock * getTerrainBlock() { AssertFatal(mTerrainBlock, "No terrain block"); return(mTerrainBlock); }
      bool terrainBlockValid() { return(mTerrainBlock ? true : false); }
      void setCursor(GuiCursor * cursor);

      TerrainAction * lookupAction(const char * name);

   private:
   
      TerrainBlock * getClientTerrain();

      // terrain interface functions
      F32 getGridHeight(const Point2I & gPos);
      void setGridHeight(const Point2I & gPos, const F32 height);

      TerrainBlock::Material getGridMaterial(const Point2I & gPos);
      void setGridMaterial(const Point2I & gPos, const TerrainBlock::Material & material);

      U8 getGridMaterialGroup(const Point2I & gPos);
      void setGridMaterialGroup(const Point2I & gPos, U8 group);

      //
      void updateBrush(Brush & brush, const Point2I & gPos);

      //
      Point3F getMousePos(){return(mMousePos);};

      // 
      void renderSelection(const Selection & sel, const ColorF & inColorFull, const ColorF & inColorNone, const ColorF & outColorFull, const ColorF & outColorNone, bool renderFill, bool renderFrame);
      void renderBorder();

   public:
   
      // persist field data - these are dynamic
      bool                 mRenderBorder;
      F32                  mBorderHeight;
      ColorI               mBorderFillColor;
      ColorI               mBorderFrameColor;   
      bool                 mBorderLineMode;
      bool                 mSelectionHidden;
      bool                 mEnableSoftBrushes;
      bool                 mRenderVertexSelection;
      bool                 mProcessUsesBrush;

      //   
      F32                  mAdjustHeightVal;
      F32                  mSetHeightVal;
      F32                  mScaleVal;
      F32                  mSmoothFactor;
      S32                  mMaterialGroup;
      F32                  mSoftSelectRadius;
      StringTableEntry     mSoftSelectFilter;
      StringTableEntry     mSoftSelectDefaultFilter;
      F32                  mAdjustHeightMouseScale;

   public:
   
      // SimObject
      bool onAdd();
      void onDeleteNotify(SimObject * object);

      static void consoleInit();
      static void initPersistFields();

      // EditTSCtrl
      void on3DMouseUp(const Gui3DMouseEvent & event);
      void on3DMouseDown(const Gui3DMouseEvent & event);
      void on3DMouseMove(const Gui3DMouseEvent & event);
      void on3DMouseDragged(const Gui3DMouseEvent & event);
      void on3DMouseEnter(const Gui3DMouseEvent & event);
      void on3DMouseLeave(const Gui3DMouseEvent & event);
      void updateGuiInfo();
      void renderScene(const RectI & updateRect);

      DECLARE_CONOBJECT(TerrainEditor);
};

#endif
