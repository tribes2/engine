//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _WORLDEDITOR_H_
#define _WORLDEDITOR_H_

#ifndef _EDITTSCTRL_H_
#include "Editor/editTSCtrl.h"
#endif
#ifndef _CONSOLETYPES_H_
#include "console/consoleTypes.h"
#endif
#ifndef _GTEXMANAGER_H_
#include "dgl/gTexManager.h"
#endif


// declare the console functions and then make them friends of the class - 
// hopefully this will reduce the number of functions needed in this class
static void cIgnoreObjClass(SimObject *, S32, const char **);
static void cClearIgnoreList(SimObject *, S32, const char**);
static void cUndoAction(SimObject *, S32, const char **);
static void cRedoAction(SimObject *, S32, const char **);
static void cClearSelection(SimObject *, S32, const char **);
static void cSelectObject(SimObject *, S32, const char **);
static void cUnselectObject(SimObject *, S32, const char **);
static S32 cGetSelectionSize(SimObject *, S32, const char **);
static S32 cGetSelectedObject(SimObject *, S32, const char **);
static const char * cGetSelectionCentroid(SimObject *, S32, const char **);
static void cDropSelection(SimObject *, S32, const char **);
static void cDeleteSelection(SimObject *, S32, const char **);
static void cCopySelection(SimObject *, S32, const char **);
static void cPasteSelection(SimObject *, S32, const char **);
static bool cCanPasteSelection(SimObject *, S32, const char **);
static void cHideSelection(SimObject *, S32, const char **);
static void cLockSelection(SimObject *, S32, const char **);
static const char * cGetMode(SimObject *, S32, const char **);
static void cSetMode(SimObject *, S32, const char **);
static void cAddUndoState(SimObject *, S32, const char **);
static void cRedirectConsole(SimObject * obj, S32, const char ** argv);

class SceneObject;
class WorldEditor : public EditTSCtrl
{
   //...
   friend void cIgnoreObjClass(SimObject *, S32, const char **);
   friend void cClearIgnoreList(SimObject *, S32, const char **);
   friend void cUndoAction(SimObject *, S32, const char **);
   friend void cRedoAction(SimObject *, S32, const char **);
   friend void cClearSelection(SimObject *, S32, const char **);
   friend void cSelectObject(SimObject *, S32, const char **);
   friend void cUnselectObject(SimObject *, S32, const char **);
   friend S32 cGetSelectionSize(SimObject *, S32, const char **);
   friend S32 cGetSelectedObject(SimObject *, S32, const char **);
   friend const char * cGetSelectionCentroid(SimObject *, S32, const char **);
   friend void cDropSelection(SimObject *, S32, const char **);
   friend void cDeleteSelection(SimObject *, S32, const char **);
   friend void cCopySelection(SimObject *, S32, const char **);
   friend void cPasteSelection(SimObject *, S32, const char **);
   friend bool cCanPasteSelection(SimObject *, S32, const char **);
   friend void cHideSelection(SimObject *, S32, const char **);
   friend void cLockSelection(SimObject *, S32, const char **);
   friend const char * cGetMode(SimObject *, S32, const char **);
   friend void cSetMode(SimObject *, S32, const char **);
   friend void cAddUndoState(SimObject *, S32, const char **);
   friend void cRedirectConsole(SimObject * obj, S32, const char ** argv);
   
   public:
   
      struct CollisionInfo
      {
         SceneObject *     obj;
         Point3F           pos;
         VectorF           normal;
      };
      
      class Selection : public SimObject
      {
         typedef SimObject    Parent;

         private:

            Point3F        mCentroid;
            Point3F        mBoxCentroid;
            bool           mCentroidValid;
            SimObjectList  mObjectList;
            bool           mAutoSelect;

            void           updateCentroid();

         public:
   
            Selection();
            ~Selection();

            //
            U32 size()  { return(mObjectList.size()); }
            SceneObject * operator[] (S32 index) { return((SceneObject*)mObjectList[index]); }

            bool objInSet(SceneObject *);
               
            bool addObject(SceneObject *);
            bool removeObject(SceneObject *);
            void clear();

            void onDeleteNotify(SimObject *);
            
            const Point3F & getCentroid();
            const Point3F & getBoxCentroid();

            void enableCollision();
            void disableCollision();

            //
            void autoSelect(bool b) { mAutoSelect = b; }
            void invalidateCentroid() { mCentroidValid = false; }

            //
            void offset(const Point3F &);
            void rotate(const EulerF &, const Point3F &);
            void scale(const VectorF &);
      };   

      //
      static SceneObject * getClientObj(SceneObject *);
      static void setClientObjInfo(SceneObject *, const MatrixF &, const VectorF &);
      static void updateClientTransforms(Selection &);

   // VERY basic undo stuff - only concerned with transform/scale/...
   private:

      struct SelectionState
      {
         struct Entry
         {
            MatrixF     mMatrix;
            VectorF     mScale;

            // validation 
            U32         mObjId;   
            U32         mObjNumber;
         };
         
         Vector<Entry>  mEntries;

         SelectionState() {
            VECTOR_SET_ASSOCIATION(mEntries);
         }
      };
      
      SelectionState * createUndo(Selection &);
      void addUndo(Vector<SelectionState *> & list, SelectionState * sel);
      bool processUndo(Vector<SelectionState *> & src, Vector<SelectionState *> & dest);
      void clearUndo(Vector<SelectionState *> & list);
      
      Vector<SelectionState*>       mUndoList;
      Vector<SelectionState*>       mRedoList;

      // someday get around to creating a growing memory stream...
      Vector<U8[2048]>              mStreamBufs;
            
      bool deleteSelection(Selection & sel);
      bool copySelection(Selection & sel);
      bool pasteSelection();
      void dropSelection(Selection & sel);

      // work off of mSelected
      void hideSelection(bool hide);
      void lockSelection(bool lock);

   private:
      typedef EditTSCtrl Parent;
      
      SceneObject * getControlObject();
      bool collide(const Gui3DMouseEvent & event, CollisionInfo & info);
      
      // gfx stuff
      void renderObjectBox(SceneObject * obj, const ColorI & col);
      void renderObjectFace(SceneObject * obj, const VectorF & normal, const ColorI & col);
      void renderSelectionWorldBox(Selection & sel);

      void renderPlane(const Point3F & origin);
      void renderMousePopupInfo();
      void renderScreenObj(SceneObject * obj, Point2I sPos);

      // axis gizmo stuff...
      void calcAxisInfo();
      bool collideAxisGizmo(const Gui3DMouseEvent & event);
      void renderAxisGizmo();
      void renderAxisGizmoText();

      // axis gizmo stuff...
      Point3F     mAxisGizmoCenter;
      VectorF     mAxisGizmoVector[3];
      F32         mAxisGizmoProjLen;
      S32         mAxisGizmoSelAxis;
      bool        mUsingAxisGizmo;

      //
      Point3F snapPoint(const Point3F & pnt);
      
      //
      bool                       mMouseDown;
      Selection                  mSelected;
      bool                       mUseVertMove;

      Selection                  mDragSelected;
      bool                       mDragSelect;
      RectI                      mDragRect;
      Point2I                    mDragStart;
      
      // modes for when dragging a selection
      enum {
         Move = 0,
         Rotate,
         Scale
      };
      
      //
      U32                        mCurrentMode;
      U32                        mDefaultMode;

      S32                        mRedirectID;

      CollisionInfo              mHitInfo;
      Point3F                    mHitOffset;
      SimObjectPtr<SceneObject>  mHitObject;
      Point2I                    mHitMousePos;
      Point3F                    mHitCentroid;
      EulerF                     mHitRotation;
      bool                       mMouseDragged;
      Gui3DMouseEvent            mLastMouseEvent;
      F32                        mLastRotation;

      // 
      class ClassInfo
      {  
         public:
            ~ClassInfo();
            
            struct Entry
            {
               StringTableEntry  mName;
               bool              mIgnoreCollision;
               TextureHandle     mDefaultHandle;
               TextureHandle     mSelectHandle;
               TextureHandle     mLockedHandle;
            };
         
            Vector<Entry*>       mEntries;
      };

   
      ClassInfo            mClassInfo;
      ClassInfo::Entry     mDefaultClassEntry;

      bool objClassIgnored(const SceneObject * obj);
      ClassInfo::Entry * getClassEntry(StringTableEntry name);
      ClassInfo::Entry * getClassEntry(const SceneObject * obj);
      bool addClassEntry(ClassInfo::Entry * entry);

   // persist field data
   public:

      enum {
         DropAtOrigin = 0,
         DropAtCamera,
         DropAtCameraWithRot,
         DropBelowCamera,
         DropAtScreenCenter,
         DropAtCentroid,
         DropToGround
      };

      bool              mPlanarMovement;
      S32               mUndoLimit;
      S32               mDropType;
      F32               mProjectDistance;
      bool              mBoundingBoxCollision;
      bool              mRenderPlane;
      bool              mRenderPlaneHashes;
      ColorI            mGridColor;
      F32               mPlaneDim;
      Point3F           mGridSize;
      bool              mRenderPopupBackground;
      ColorI            mPopupBackgroundColor;
      ColorI            mPopupTextColor;
      StringTableEntry  mSelectHandle;
      StringTableEntry  mDefaultHandle;
      StringTableEntry  mLockedHandle;
      ColorI            mObjectTextColor;
      bool				   mObjectsUseBoxCenter;
      S32               mAxisGizmoMaxScreenLen;
      bool              mAxisGizmoActive;
      F32               mMouseMoveScale;
      F32               mMouseRotateScale;
      F32               mMouseScaleScale;
      F32               mMinScaleFactor;
      F32               mMaxScaleFactor;
      ColorI            mObjSelectColor;
      ColorI            mObjMouseOverSelectColor;
      ColorI            mObjMouseOverColor;
      bool              mShowMousePopupInfo;
      ColorI            mDragRectColor;
      bool              mRenderObjText;
      bool              mRenderObjHandle;
      StringTableEntry  mObjTextFormat;
      ColorI            mFaceSelectColor;
      bool              mRenderSelectionBox;
      ColorI            mSelectionBoxColor;
      bool              mSelectionLocked;
      bool              mSnapToGrid;
      bool              mSnapRotations;
      F32               mRotationSnap;
      bool              mToggleIgnoreList;
      bool              mRenderNav;

   private:
      // cursor stuff
      enum {
         HandCursor = 0,
         RotateCursor,
         ScaleCursor,
         MoveCursor,
         ArrowCursor,
         DefaultCursor,

         //
         NumCursors
      };
         
      GuiCursor *                mCursors[NumCursors];
      GuiCursor *                mCurrentCursor;
      bool grabCursors();
      void setCursor(U32 cursor);
      GuiCursor * getCursor();
      
   public:
   
      WorldEditor();
      ~WorldEditor();
      
      // SimObject
      bool onAdd();
      void onEditorEnable();
      
      // EditTSCtrl
      void on3DMouseMove(const Gui3DMouseEvent & event);
      void on3DMouseDown(const Gui3DMouseEvent & event);
      void on3DMouseUp(const Gui3DMouseEvent & event);
      void on3DMouseDragged(const Gui3DMouseEvent & event);
      void on3DMouseEnter(const Gui3DMouseEvent & event);
      void on3DMouseLeave(const Gui3DMouseEvent & event);
      void on3DRightMouseDown(const Gui3DMouseEvent & event);
      void on3DRightMouseUp(const Gui3DMouseEvent & event);

      void updateGuiInfo();

      //
      void renderScene(const RectI & updateRect);
      
      static void consoleInit();
      static void initPersistFields();

      DECLARE_CONOBJECT(WorldEditor);
};

#endif



