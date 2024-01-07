//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUIPOPUPCTRL_H_
#define _GUIPOPUPCTRL_H_

#ifndef _GUITEXTCTRL_H_
#include "GUI/guiTextCtrl.h"
#endif
#ifndef _GUITEXTLISTCTRL_H_
#include "GUI/guiTextListCtrl.h"
#endif
#ifndef _GUIBUTTONCTRL_H_
#include "GUI/guiButtonCtrl.h"
#endif
#ifndef _GUIBACKGROUNDCTRL_H_
#include "GUI/guiBackgroundCtrl.h"
#endif
#ifndef _GUISCROLLCTRL_H_
#include "GUI/guiScrollCtrl.h"
#endif
class GuiPopUpMenuCtrl;

class GuiPopUpTextListCtrl : public GuiTextListCtrl
{
   private:
      typedef GuiTextListCtrl Parent;

   public:
      enum ButtonDown
      {
         None = 0,      
         AlreadyDown = 1,
         ScrollArea = 2,
         TextListArea = 3
      };

   protected:
      GuiPopUpMenuCtrl *mPopUpCtrl;
      ButtonDown mMouseDown;

   public:
      GuiPopUpTextListCtrl(); // for inheritance
      GuiPopUpTextListCtrl(GuiPopUpMenuCtrl *ctrl);
      ~GuiPopUpTextListCtrl();

      void onSleep();

      // GuiArrayCtrl overload:
      void onCellSelected(Point2I cell);

      // GuiControl overloads:
      void onMouseDown(const GuiEvent &event);
      void onMouseDragged(const GuiEvent &event);
      void onMouseUp(const GuiEvent &event);
      bool onKeyDown(const GuiEvent &event);
      void onRenderCell(Point2I offset, Point2I cell, bool selected, bool mouseOver);
};

class GuiPopUpMenuCtrl : public GuiTextCtrl
{
   typedef GuiTextCtrl Parent;

  public:
   struct Entry
   {
      char buf[256];
      S32 id;
      U16 ascii;
      U16 scheme;
   };

   struct Scheme
   {
      U32      id;
      ColorI   fontColor;
      ColorI   fontColorHL;
      ColorI   fontColorSEL;
   };

  protected:
   GuiPopUpTextListCtrl *mTl;
   GuiScrollContentCtrl *mSContentc;
   GuiScrollCtrl *mSc;
   GuiBackgroundCtrl *mBackground;
   Vector<Entry> mEntries;
   Vector<Scheme> mSchemes;
   S32 mSelIndex;
   S32 mMaxPopupHeight;
   F32 mIncValue;
   F32 mScrollCount;
   S32 mLastYvalue;
   GuiEvent mEventSave;
   S32 mRevNum;
   bool mInAction;
   bool mReplaceText;

   virtual void addChildren();
   virtual void repositionPopup();

  public:
   GuiPopUpMenuCtrl(void);
   ~GuiPopUpMenuCtrl();	
   GuiScrollCtrl::Region mScrollDir;
   bool onAdd();
   void onSleep();
   static void consoleInit();
   void sort();
   void addEntry(const char *buf, S32 id, U32 scheme = 0);
   void addScheme(U32 id, ColorI fontColor, ColorI fontColorHL, ColorI fontColorSEL);
   void onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder);
   void onAction();
   virtual void closePopUp();
   void clear();
   void onMouseDown(const GuiEvent &event);
   void onMouseUp(const GuiEvent &event);
   void setupAutoScroll(const GuiEvent &event);
   void autoScroll();
   bool onKeyDown(const GuiEvent &event);
   void reverseTextList();
   bool getFontColor(ColorI &fontColor, S32 id, bool selected, bool mouseOver);

   S32 getSelected();
   void setSelected(S32 id);
   const char *getScriptValue();
   const char *getTextById(S32 id);
   S32 findText( const char* text );
   S32 getNumEntries()	{ return( mEntries.size() ); }
   void replaceText(S32);
   
   DECLARE_CONOBJECT(GuiPopUpMenuCtrl);
   static void initPersistFields(void);
};

#endif //_GUI_POPUPMENU_CTRL_H
