//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUITREEVIEWCTRL_H_
#define _GUITREEVIEWCTRL_H_

#ifndef _MRECT_H_
#include "Math/mRect.h"
#endif
#ifndef _GFONT_H_
#include "dgl/gFont.h"
#endif
#ifndef _GUICONTROL_H_
#include "GUI/guiControl.h"
#endif
#ifndef _GUIARRAYCTRL_H_
#include "GUI/guiArrayCtrl.h"
#endif

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _BITSET_H_
#include "Core/bitSet.h"
#endif
class GuiTreeViewCtrl : public GuiArrayCtrl
{
   private:
      typedef GuiArrayCtrl Parent;

   protected:
      
      struct Item
      {
         enum ItemState {
            Selected       = BIT(0),
            Expanded       = BIT(1),
            Focus          = BIT(2),
            MouseOverBmp   = BIT(3),
            MouseOverText  = BIT(4),
         };

         BitSet32             mState;
         char*                mText;
         char*                mValue;
         S16                  mNormalImage;
         S16                  mExpandedImage;

         S32                  mId;
         U32                  mTabLevel;
         Item *               mParent;
         Item *               mChild;
         Item *               mNext;
         Item *               mPrevious;

         Item();
         ~Item();
      };

      Vector<Item*>           mItems;
      Vector<Item*>           mVisibleItems;

      S32                     mItemCount;
      Item *                  mItemFreeList;
      Item *                  mRoot;
      S32                     mMaxWidth;
      S32                     mSelectedItem;

      enum TreeState {
         RebuildVisible    = BIT(0)
      };
      BitSet32                mFlags;

      TextureHandle           mImagesHandle;
      Vector<RectI>           mImageBounds;

      // persist field info...
      S32                     mTabSize;
      StringTableEntry        mImagesBitmap;
      S32                     mNumImages;
      S32                     mTextOffset;
      bool                    mFullRowSelect;
      S32                     mItemHeight;

      Item * getItem(S32 itemId);
      Item * createItem();
      void destroyItem(Item * item);
      void destroyTree();

      // 
      void buildItem(Item * item, U32 tabLevel);
      void buildVisibleTree();

      //
      enum HitFlags {
         OnIndent       = BIT(0),
         OnImage        = BIT(1),
         OnText         = BIT(2),
         OnRow          = BIT(3),
      };
      bool hitTest(const Point2I & pnt, Item* & item, BitSet32 & flags);

   public:
      
      GuiTreeViewCtrl();
      virtual ~GuiTreeViewCtrl();

      //
      bool selectItem(S32 itemId, bool select);
      bool expandItem(S32 itemId, bool expand);
      const char * getItemText(S32 itemId);
      const char * getItemValue(S32 itemId);

      bool editItem( S32 itemId, const char* newText, const char* newValue );

      // insertion/removal
      S32 insertItem(S32 parentId, const char * text, const char * value, S16 normalImage, S16 expandedImage);
      bool removeItem(S32 itemId);

      // tree items
      S32 getFirstRootItem();
      S32 getChildItem(S32 itemId);
      S32 getParentItem(S32 itemId);
      S32 getNextSiblingItem(S32 itemId);
      S32 getPrevSiblingItem(S32 itemId);
      S32 getItemCount();
      S32 getSelectedItem();
      void moveItemUp( S32 itemId );

//      // visible items
//      S32 getFirstVisible();
//      S32 getLastVisible();
//      S32 getNextVisible(S32 itemId);
//      S32 getPrevVisible(S32 itemId);
//      S32 getVisibleCount();

//      // misc.
      //bool scrollVisible( S32 itemId );

      // GuiControl
      bool onWake();
      void onSleep();
      void onPreRender();
      bool onKeyDown( const GuiEvent &event );
		void onMouseDown(const GuiEvent &event);
      void onMouseMove(const GuiEvent &event);
      void onMouseEnter(const GuiEvent &event);
      void onMouseLeave(const GuiEvent &event);
      void onRightMouseDown(const GuiEvent &event);

      // GuiArrayCtrl
      void onRenderCell(Point2I offset, Point2I cell, bool, bool);

      //
      static void consoleInit();
      static void initPersistFields();

      DECLARE_CONOBJECT(GuiTreeViewCtrl);
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

class GuiTreeView : public GuiArrayCtrl
{
	private:
		typedef GuiArrayCtrl Parent;

	   struct ObjNode
	   {
	      S32 level;
	      SimObject *object;
	      bool lastInGroup;
	      S32 parentIndex;
	      ObjNode(S32 in_level, SimObject *in_object, bool in_last, S32 in_pi)
         {
         	level = in_level;
         	object = in_object;
         	lastInGroup = in_last;
         	parentIndex = in_pi;
         }
	   };

	   Vector<ObjNode> mObjectList;

		enum BitmapIndices
		{
			BmpNone = -1,
			BmpChildAbove,         
			BmpChildBelow,         
			BmpChildBetween,
			       
			BmpParentOpen,
			BmpParentOpenAbove,    
			BmpParentOpenBelow,    
			BmpParentOpenBetween,  
			         
			BmpParentClosed,
			BmpParentClosedAbove,  
			BmpParentClosedBelow,  
			BmpParentClosedBetween,
			       
			BmpParentContinue,
			       
			BmpCount
		};
		RectI mBitmapBounds[BmpCount];  //bmp is [3*n], bmpHL is [3*n + 1], bmpNA is [3*n + 2]
		TextureHandle mTextureHandle;

		//font
	   Resource<GFont> mFont;

		SimSet *mRootObject;

      // persist data
      bool mAllowMultipleSelections;
      bool mRecurseSets;
      
      ObjNode * getHitNode(const GuiEvent & event);   
      
	public:
	   DECLARE_CONOBJECT(GuiTreeView);
      GuiTreeView();
		static void consoleInit();
      static void initPersistFields();
      
		bool onWake();
		void onSleep();

		void setTreeRoot(SimSet *srcObj);
		void buildTree(SimSet *srcObj, S32 srcLevel, S32 srcParentIndex);

      void onPreRender();
		void onMouseDown(const GuiEvent &event);
      void onRightMouseDown(const GuiEvent & event);
      void onRightMouseUp(const GuiEvent & event);

      void setInstantGroup(SimObject * obj);
      void inspectObject(SimObject * obj);
      void selectObject(SimObject * obj);
      void unselectObject(SimObject * obj);
      void toggleSelected(SimObject * obj);
      void clearSelected();
		void setSelected(SimObject *selObj);

		void onRenderCell(Point2I offset, Point2I cell, bool, bool);
};

#endif
