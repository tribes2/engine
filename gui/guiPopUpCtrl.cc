//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "dgl/dgl.h"
#include "GUI/guiCanvas.h"
#include "GUI/guiPopUpCtrl.h"
#include "console/consoleTypes.h"


//------------------------------------------------------------------------------
GuiPopUpTextListCtrl::GuiPopUpTextListCtrl()
{
   mPopUpCtrl = NULL;
   mMouseDown = AlreadyDown;
}


//------------------------------------------------------------------------------
GuiPopUpTextListCtrl::GuiPopUpTextListCtrl(GuiPopUpMenuCtrl *ctrl)
{
   mPopUpCtrl = ctrl;
   mMouseDown = AlreadyDown;
}

//------------------------------------------------------------------------------
GuiPopUpTextListCtrl::~GuiPopUpTextListCtrl()
{
}

//------------------------------------------------------------------------------
void GuiPopUpTextListCtrl::onSleep()
{
   mMouseDown = None;
   Parent::onSleep();
}

//------------------------------------------------------------------------------
void GuiPopUpTextListCtrl::onCellSelected( Point2I /*cell*/ )
{
   // Do nothing, the parent control will take care of everything...
}

//------------------------------------------------------------------------------
bool GuiPopUpTextListCtrl::onKeyDown(const GuiEvent &event)
{
   //if the control is a dead end, don't process the input:
   if ( !mVisible || !mActive || !mAwake )
      return false;
   
   //see if the key down is a <return> or not
   if ( event.modifier == 0 )
   {
      if ( event.keyCode == KEY_RETURN )
      {
         mPopUpCtrl->closePopUp();
         return true;
      }
      else if ( event.keyCode == KEY_ESCAPE )
      {
         mSelectedCell.set( -1, -1 );
         mPopUpCtrl->closePopUp();
         return true;
      }
   }
   
   //otherwise, pass the event to it's parent
   return Parent::onKeyDown(event);
}

//------------------------------------------------------------------------------
void GuiPopUpTextListCtrl::onMouseDown(const GuiEvent &event)
{
   mMouseDown = None;

   // See if it's within the scroll content ctrl:
   GuiControl *parent = getParent();
   if ( !parent ) 
      return;

   if ( parent->cursorInControl() )
   {
      //let the parent handle the event
      Parent::onMouseDown(event);
      mMouseDown = TextListArea;
      //we're done
      return;
   }
   
   //see if it's within the scroll control
   GuiControl *grandParent = parent->getParent();
   if ( !grandParent ) 
      return;

   if ( grandParent->cursorInControl() )
   {
      //let the scroll control deal with it
      grandParent->onMouseDown(event);
      mMouseDown = ScrollArea;
      //and we're done
      return;
   }
}

//------------------------------------------------------------------------------
void GuiPopUpTextListCtrl::onMouseDragged(const GuiEvent &event)
{
   //the only way to get here, is if the scroll control is dragging
   //else see if it's within the scroll control
   GuiControl *parent = getParent();
   if ( !parent ) 
      return;

   GuiControl *grandParent = parent->getParent();
   if ( !grandParent ) 
      return;

   //let the scroll control deal with it
   grandParent->onMouseDragged( event );
   //will highlight the text while dragging
   onMouseMove( event );

   if ( mMouseDown == AlreadyDown )
   {
      if ( parent->cursorInControl() )
         mMouseDown = TextListArea;
      return;
   }
   
   if ( mMouseDown != ScrollArea )
      mPopUpCtrl->setupAutoScroll( event );
}

//------------------------------------------------------------------------------
void GuiPopUpTextListCtrl::onMouseUp(const GuiEvent &event)
{
   mPopUpCtrl->mScrollDir = GuiScrollCtrl::None;

   //see if it's within the scroll content ctrl
   GuiControl *parent = getParent();
   if ( !parent ) 
      return;

   // Initial case--this control locks the mouse upon creation.
   // If mouse is released outside of the menu, just move to next stage
   // else if dragged over menu and released, select the appropriate entry
   if ( mMouseDown == AlreadyDown )
   {
      if ( !parent->cursorInControl() )
      {
         mMouseDown = None;
         return;
      }
   }

   if ( parent->cursorInControl() )
   {
      // Select the item the mouse is over:
      Point2I localPt = globalToLocalCoord( event.mousePoint );
      Point2I cell( 0, ( localPt.y < 0 ? -1 : localPt.y / mCellSize.y ) );
      if ( cell.y >= 0 && cell.y < mSize.y )
         cellSelected( Point2I( 0, cell.y ) );
      
      //now let the pop up know a cell has been selected...
      mPopUpCtrl->closePopUp();
      //we're done
      return;
   }

   //see if it's within the scroll control
   GuiControl *grandParent = parent->getParent();
   if ( !grandParent ) 
      return;

   if ( grandParent->cursorInControl() )
   {
      //let the scroll control deal with it
      grandParent->onMouseUp( event );
      //and we're done
      return;
   }
   
   // otherwise, we clicked outside the popup, and we're done
   // let the pop up know a cell has been selected...
   if ( mMouseDown == None )
      mPopUpCtrl->closePopUp();
}


//------------------------------------------------------------------------------
void GuiPopUpTextListCtrl::onRenderCell(Point2I offset, Point2I cell, bool selected, bool mouseOver)
{
   ColorI fontColor;
   mPopUpCtrl->getFontColor( fontColor, mList[cell.y].id, selected, mouseOver );

   dglSetBitmapModulation( fontColor );
   dglDrawText( mFont, Point2I( offset.x + 4, offset.y ), mList[cell.y].text );
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
GuiPopUpMenuCtrl::GuiPopUpMenuCtrl(void)
{
   VECTOR_SET_ASSOCIATION(mEntries);
   VECTOR_SET_ASSOCIATION(mSchemes);

   mSelIndex = -1;
   mActive = true;
   mMaxPopupHeight = 200;
   mScrollDir = GuiScrollCtrl::None;
   mScrollCount = 0;
   mLastYvalue = 0;
   mIncValue = 0;
   mRevNum = 0;
   mInAction = false;
}

//------------------------------------------------------------------------------
GuiPopUpMenuCtrl::~GuiPopUpMenuCtrl()
{

}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrl::initPersistFields(void)
{
   Parent::initPersistFields();
   
   addField("maxPopupHeight",           TypeS32,          Offset(mMaxPopupHeight, GuiPopUpMenuCtrl));
}

//------------------------------------------------------------------------------
static void cGuiPopUpMenuAdd(SimObject *obj, S32 argc, const char **argv)
{
   GuiPopUpMenuCtrl *ctrl = static_cast<GuiPopUpMenuCtrl*>(obj);
   if ( argc > 4 )
      ctrl->addEntry(argv[2],dAtoi(argv[3]),dAtoi(argv[4]));
   else
      ctrl->addEntry(argv[2],dAtoi(argv[3]),0);
}

static void cGuiPopUpMenuAddScheme(SimObject *obj, S32, const char **argv)
{
   GuiPopUpMenuCtrl *ctrl = static_cast<GuiPopUpMenuCtrl*>(obj);
   ColorI fontColor, fontColorHL, fontColorSEL;
   U32 r, g, b;
   char buf[64];

   dStrcpy( buf, argv[3] );
   char* temp = dStrtok( buf, " \0" );
   r = temp ? dAtoi( temp ) : 0;
   temp = dStrtok( NULL, " \0" );
   g = temp ? dAtoi( temp ) : 0;
   temp = dStrtok( NULL, " \0" );
   b = temp ? dAtoi( temp ) : 0;
   fontColor.set( r, g, b );

   dStrcpy( buf, argv[4] );
   temp = dStrtok( buf, " \0" );
   r = temp ? dAtoi( temp ) : 0;
   temp = dStrtok( NULL, " \0" );
   g = temp ? dAtoi( temp ) : 0;
   temp = dStrtok( NULL, " \0" );
   b = temp ? dAtoi( temp ) : 0;
   fontColorHL.set( r, g, b );

   dStrcpy( buf, argv[5] );
   temp = dStrtok( buf, " \0" );
   r = temp ? dAtoi( temp ) : 0;
   temp = dStrtok( NULL, " \0" );
   g = temp ? dAtoi( temp ) : 0;
   temp = dStrtok( NULL, " \0" );
   b = temp ? dAtoi( temp ) : 0;
   fontColorSEL.set( r, g, b );

   ctrl->addScheme( dAtoi( argv[2] ), fontColor, fontColorHL, fontColorSEL );
}

static void cGuiPopUpMenuSetText(SimObject *obj, S32, const char **argv)
{
   GuiPopUpMenuCtrl *ctrl = static_cast<GuiPopUpMenuCtrl*>(obj);
   ctrl->setText(argv[2]);
}

static const char * cGuiPopUpMenuGetText(SimObject *obj, S32, const char **)
{
   GuiPopUpMenuCtrl *ctrl = static_cast<GuiPopUpMenuCtrl*>(obj);
   return ctrl->getText();
}

static void cGuiPopUpMenuClear(SimObject *obj, S32, const char **)
{
   GuiPopUpMenuCtrl *ctrl = static_cast<GuiPopUpMenuCtrl*>(obj);
   ctrl->clear();
}

static void cGuiPopUpMenuSort(SimObject *obj, S32, const char **)
{
   GuiPopUpMenuCtrl *ctrl = static_cast<GuiPopUpMenuCtrl*>(obj);
   ctrl->sort();
}

static void cGuiPopUpMenuOnAction(SimObject *obj, S32, const char **)
{
   GuiPopUpMenuCtrl *ctrl = static_cast<GuiPopUpMenuCtrl*>(obj);
   ctrl->onAction();
}

static void cGuiPopUpMenuClose(SimObject *obj, S32, const char **)
{
   GuiPopUpMenuCtrl *ctrl = static_cast<GuiPopUpMenuCtrl*>(obj);
   ctrl->closePopUp();
}

static S32 cGuiPopUpMenuGetSelected(SimObject * obj, S32, const char **)
{
   GuiPopUpMenuCtrl * ctrl = static_cast<GuiPopUpMenuCtrl*>(obj);
   return ctrl->getSelected();
}

static void cGuiPopUpMenuSetSelected(SimObject * obj, S32, const char ** argv)
{
   GuiPopUpMenuCtrl * ctrl = static_cast<GuiPopUpMenuCtrl*>(obj);
   ctrl->setSelected(dAtoi(argv[2]));
}

static const char * cGuiPopUpMenuGetTextById(SimObject * obj, S32, const char ** argv)
{
   GuiPopUpMenuCtrl * ctrl = static_cast<GuiPopUpMenuCtrl*>(obj);
   return(ctrl->getTextById(dAtoi(argv[2])));
}

//------------------------------------------------------------------------------
// this fills the popup with a classrep's field enumeration type info - more of a 
// helper function than anything.   If console access to the field list is added, 
// at least for the enumerated types, then this should go away..
//
// args:
//    argv[2] - class
//    argv[3] - field
static void cGuiPopUpMenuSetEnumContent(SimObject * obj, S32, const char ** argv)
{
   GuiPopUpMenuCtrl * menu = static_cast<GuiPopUpMenuCtrl*>(obj);
   AbstractClassRep * classRep = AbstractClassRep::getClassList();

   // walk the class list to get our class
   while(classRep)
   {
      if(!dStricmp(classRep->getClassName(), argv[2]))
         break;
      classRep = classRep->getNextClass();
   }

   // get it?
   if(!classRep)
   {
      Con::warnf(ConsoleLogEntry::General, "failed to locate class rep for '%s'", argv[2]);
      return;
   }

   // walk the fields to check for this one (findField checks StringTableEntry ptrs...)
   U32 i;
   for(i = 0; i < classRep->mFieldList.size(); i++)
      if(!dStricmp(classRep->mFieldList[i].pFieldname, argv[3]))
         break;
   
   // found it?   
   if(i == classRep->mFieldList.size())
   {   
      Con::warnf(ConsoleLogEntry::General, "failed to locate field '%s' for class '%s'", argv[3], argv[2]);
      return;
   }
   
   const AbstractClassRep::Field & field = classRep->mFieldList[i];

   // check the type
   if(field.type != TypeEnum)
   {
      Con::warnf(ConsoleLogEntry::General, "field '%s' is not an enumeration for class '%s'", argv[3], argv[2]);
      return;
   }

   AssertFatal(field.table, avar("enumeration '%s' for class '%s' with NULL ", argv[3], argv[2]));

   // fill it
   for(i = 0; i < field.table->size; i++)
      menu->addEntry(field.table->table[i].label, field.table->table[i].index);
}

//------------------------------------------------------------------------------
static S32 cGuiPopUpMenuFindText( SimObject* obj, S32, const char** argv )
{
   AssertFatal( dynamic_cast<GuiPopUpMenuCtrl*>( obj ), "Object passed to cGuiPopUpMenuFindText is not a GuiPopUpMenuCtrl!" );
   GuiPopUpMenuCtrl* ctrl = static_cast<GuiPopUpMenuCtrl*>( obj );
   return( ctrl->findText( argv[2] ) );   
}

//------------------------------------------------------------------------------
static S32 cGuiPopUpSize( SimObject* obj, S32, const char** )
{
   AssertFatal( dynamic_cast<GuiPopUpMenuCtrl*>( obj ), "Object passed to cGuiPopUpSize is not a GuiPopUpMenuCtrl!" );
   GuiPopUpMenuCtrl* ctrl = static_cast<GuiPopUpMenuCtrl*>( obj );
   return( ctrl->getNumEntries() ); 
}

//------------------------------------------------------------------------------
void cGuiReplaceText( SimObject* obj, S32, const char** argv)
{
   GuiPopUpMenuCtrl* ctrl = static_cast<GuiPopUpMenuCtrl*>( obj );
   ctrl->replaceText(dAtoi(argv[2]));  
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrl::consoleInit()
{
   Con::addCommand("GuiPopUpMenuCtrl", "sort", cGuiPopUpMenuSort, "menu.sort()", 2, 2);
   Con::addCommand("GuiPopUpMenuCtrl", "add", cGuiPopUpMenuAdd, "menu.add(name,idNum{,scheme})", 4, 5);
   Con::addCommand("GuiPopUpMenuCtrl", "addScheme", cGuiPopUpMenuAddScheme, "menu.addScheme(id, fontColor, fontColorHL, fontColorSEL)", 6, 6);
   Con::addCommand("GuiPopUpMenuCtrl", "getText", cGuiPopUpMenuGetText, "menu.getText()", 2, 2);
   Con::addCommand("GuiPopUpMenuCtrl", "setText", cGuiPopUpMenuSetText, "menu.setText(text)", 3, 3);
   Con::addCommand("GuiPopUpMenuCtrl", "getValue", cGuiPopUpMenuGetText, "menu.getValue()", 2, 2);
   Con::addCommand("GuiPopUpMenuCtrl", "setValue", cGuiPopUpMenuSetText, "menu.setValue(text)", 3, 3);
   Con::addCommand("GuiPopUpMenuCtrl", "clear", cGuiPopUpMenuClear, "menu.clear()", 2, 2);
   Con::addCommand("GuiPopUpMenuCtrl", "forceOnAction", cGuiPopUpMenuOnAction, "menu.forceOnAction()", 2, 2);
   Con::addCommand("GuiPopUpMenuCtrl", "forceClose", cGuiPopUpMenuClose, "menu.forceClose()", 2, 2);
   Con::addCommand("GuiPopUpMenuCtrl", "getSelected", cGuiPopUpMenuGetSelected, "menu.getSelected()", 2, 2);
   Con::addCommand("GuiPopUpMenuCtrl", "setSelected", cGuiPopUpMenuSetSelected, "menu.setSelected(id)", 3, 3);
   Con::addCommand("GuiPopUpMenuCtrl", "getTextById", cGuiPopUpMenuGetTextById, "menu.getTextById(id)", 3, 3);
   Con::addCommand("GuiPopUpMenuCtrl", "setEnumContent", cGuiPopUpMenuSetEnumContent, "menu.setEnumContent(class, enum)", 4, 4);
   Con::addCommand("GuiPopUpMenuCtrl", "findText", cGuiPopUpMenuFindText, "menu.findText(text)", 3, 3);
   Con::addCommand("GuiPopUpMenuCtrl", "size", cGuiPopUpSize, "menu.size()", 2, 2);
   Con::addCommand("GuiPopUpMenuCtrl", "replaceText", cGuiReplaceText, "menu.replaceText(bool)", 3, 3);
}

//------------------------------------------------------------------------------
bool GuiPopUpMenuCtrl::onAdd()
{
   if (!Parent::onAdd())
      return false;
   mSelIndex = -1;
   mReplaceText = true;
   return true;
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrl::onSleep()
{
   Parent::onSleep();
   closePopUp();  // Tests in function.
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrl::clear()
{
   mEntries.setSize(0);
   setText("");
   mSelIndex = -1;
   mRevNum = 0;
}

//------------------------------------------------------------------------------
static S32 QSORT_CALLBACK textCompare(const void *a,const void *b)
{
   GuiPopUpMenuCtrl::Entry *ea = (GuiPopUpMenuCtrl::Entry *) (a);
   GuiPopUpMenuCtrl::Entry *eb = (GuiPopUpMenuCtrl::Entry *) (b);
   return (dStricmp(eb->buf, ea->buf));
} 

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrl::sort()
{
   dQsort((void *)&(mEntries[0]), mEntries.size(), sizeof(Entry), textCompare);
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrl::addEntry(const char *buf, S32 id, U32 scheme)
{
   Entry e;
   dStrcpy(e.buf, buf);
   e.id = id;
   e.scheme = scheme;
   
   // see if there is a shortcut key
   char * cp = dStrchr(e.buf, '~');
   e.ascii = cp ? cp[1] : 0;

   mEntries.push_back(e);

   if ( mInAction && mTl )
   {
      // Add the new entry:
      mTl->addEntry( e.id, e.buf );
      repositionPopup();
   }
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrl::addScheme( U32 id, ColorI fontColor, ColorI fontColorHL, ColorI fontColorSEL )
{
   if ( !id )
      return;

   Scheme newScheme;
   newScheme.id = id;
   newScheme.fontColor = fontColor;
   newScheme.fontColorHL = fontColorHL;
   newScheme.fontColorSEL = fontColorSEL;

   mSchemes.push_back( newScheme );
}

//------------------------------------------------------------------------------
S32 GuiPopUpMenuCtrl::getSelected()
{
   if (mSelIndex == -1)
      return 0;
   return mEntries[mSelIndex].id;
}

//------------------------------------------------------------------------------
const char* GuiPopUpMenuCtrl::getTextById(S32 id)
{
   for ( U32 i = 0; i < mEntries.size(); i++ )
   {
      if ( mEntries[i].id == id )
         return( mEntries[i].buf );
   }

   return( "" );
}

//------------------------------------------------------------------------------
S32 GuiPopUpMenuCtrl::findText( const char* text )
{
   for ( U32 i = 0; i < mEntries.size(); i++ )
   {
      if ( dStrcmp( text, mEntries[i].buf ) == 0 )
         return( mEntries[i].id );        
   }
   return( -1 );
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrl::setSelected(S32 id)
{
   S32 i;
   for (i = 0; U32(i) < mEntries.size(); i++)
      if (id == mEntries[i].id)
      {
         i = (mRevNum > i) ? mRevNum - i : i;
         mSelIndex = i;
         setText(mEntries[i].buf);
         return;
      }

   setText("");
   mSelIndex = -1;
}

//------------------------------------------------------------------------------
const char *GuiPopUpMenuCtrl::getScriptValue()
{
   return getText();
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrl::onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder)
{
   updateRect;
   firstResponder;
   Point2I localStart;
   
   if(mScrollDir != GuiScrollCtrl::None)
      autoScroll();
   RectI r(offset, mBounds.extent);
   dglDrawRectFill(r, mProfile->mBorderColor);
   r.point.x +=2;
   r.point.y +=2;
   r.extent.x -=4;
   r.extent.y -=4;
   dglDrawRectFill(r, mProfile->mFillColor);

   S32 txt_w = mFont->getStrWidth(mText);
   localStart.x = 0;
   localStart.y = (mBounds.extent.y - (mFont->getHeight())) / 2;

   // align the horizontal
   switch (mProfile->mAlignment)
   {
      case GuiControlProfile::RightJustify:
         localStart.x = mBounds.extent.x - txt_w;  
         break;
      case GuiControlProfile::CenterJustify:
         localStart.x = (mBounds.extent.x - txt_w) / 2;
         break;
      default:
         // GuiControlProfile::LeftJustify
         localStart.x = 0;
         break;
   }
   Point2I globalStart = localToGlobalCoord(localStart);
   dglSetBitmapModulation(mProfile->mFontColor);
   dglDrawText(mFont, globalStart, mText, mProfile->mFontColors);
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrl::closePopUp()
{
   if ( !mInAction )
      return;

   // Get the selection from the text list:
   mSelIndex = mTl->getSelectedCell().y;
   mSelIndex = (mRevNum >= mSelIndex && mSelIndex != -1) ? mRevNum - mSelIndex : mSelIndex;
   if ( mSelIndex != -1 )
   {
      if(mReplaceText)
         setText( mEntries[mSelIndex].buf );
      setIntVariable( mEntries[mSelIndex].id );
   }

   // Release the mouse:
   mInAction = false;
   mTl->mouseUnlock();

   // Pop the background:
   getRoot()->popDialogControl(mBackground);
   
   // Kill the popup:
   mBackground->removeObject( mSc );
   mTl->deleteObject();
   mSc->deleteObject();
   mBackground->deleteObject();
   
   // Set this as the first responder:
   setFirstResponder();
   
   // Now perform the popup action:
   if ( mSelIndex != -1 )
   {
      char idval[24];
      dSprintf( idval, sizeof(idval), "%d", mEntries[mSelIndex].id );
      Con::executef( this, 3, "onSelect", idval, mEntries[mSelIndex].buf );
   }
   else
      Con::executef( this, 1, "onCancel" );
  
   // Execute the popup console command:
   if ( mConsoleCommand[0] )
      Con::evaluate( mConsoleCommand, false );
}

//------------------------------------------------------------------------------
bool GuiPopUpMenuCtrl::onKeyDown(const GuiEvent &event)
{
   //if the control is a dead end, don't process the input:
   if ( !mVisible || !mActive || !mAwake )
      return false;
   
   //see if the key down is a <return> or not
   if ( event.keyCode == KEY_RETURN && event.modifier == 0 )
   {
      onAction();
      return true;
   }
   
   //otherwise, pass the event to its parent
   return Parent::onKeyDown( event );
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrl::onAction()
{
   GuiControl *canCtrl = getParent();
   
   addChildren();

   GuiCanvas *root = getRoot();
   Point2I windowExt = root->mBounds.extent;

   mBackground->mBounds.point.set(0,0);
   mBackground->mBounds.extent = root->mBounds.extent;

   S32 textWidth = 0, width = mBounds.extent.x;
   const S32 menuSpace = 5;
   const S32 textSpace = 2;
   bool setScroll = false;
      
   for(U32 i=0; i<mEntries.size(); ++i)
      if(S32(mFont->getStrWidth(mEntries[i].buf)) > textWidth)
         textWidth = mFont->getStrWidth(mEntries[i].buf);

   if(textWidth > mBounds.extent.x)
   {
      textWidth +=10;
      width = textWidth;
   }

   mTl->setCellSize(Point2I(width, mFont->getHeight()+3));

   for(U32 j=0; j<mEntries.size(); ++j)
      mTl->addEntry(mEntries[j].id, mEntries[j].buf);

   Point2I pointInGC = canCtrl->localToGlobalCoord(mBounds.point);
   Point2I scrollPoint(pointInGC.x, pointInGC.y + mBounds.extent.y); 

   //Calc max Y distance, so Scroll Ctrl will fit on window 
   S32 maxYdis = windowExt.y - pointInGC.y - mBounds.extent.y - menuSpace; 
                     
   //If scroll bars need to be added
   if(maxYdis < mTl->mBounds.extent.y + textSpace)
   {
      //Should we pop menu list above the button
      if(maxYdis < pointInGC.y - menuSpace)
      {
         reverseTextList();
         maxYdis = pointInGC.y - menuSpace;
         //Does the menu need a scroll bar 
         if(maxYdis < mTl->mBounds.extent.y + textSpace)
         {
            //Calc for the width of the scroll bar 
            if(textWidth >=  width)
               width += 20;
            mTl->setCellSize(Point2I(width,mFont->getHeight() + textSpace));
            //Pop menu list above the button
            scrollPoint.set(pointInGC.x, menuSpace - 1);
            setScroll = true;
         }
         //No scroll bar needed
         else
         {
            maxYdis = mTl->mBounds.extent.y + textSpace;
            scrollPoint.set(pointInGC.x, pointInGC.y - maxYdis -1);
         }
      } 
      //Scroll bar needed but Don't pop above button
      else
      {
         mRevNum = 0;
         //Calc for the width of the scroll bar 
         if(textWidth >=  width)
            width += 20;
         mTl->setCellSize(Point2I(width,mFont->getHeight() + textSpace));
         setScroll = true;
      }
   }
   //No scroll bar needed
   else
      maxYdis = mTl->mBounds.extent.y + textSpace;
         
   //offset it from the background so it lines up properly
   mSc->mBounds.point = mBackground->globalToLocalCoord(scrollPoint);

   if(mSc->mBounds.point.x + width > mBackground->mBounds.extent.x)
      if(width - mBounds.extent.x > 0)
         mSc->mBounds.point.x -= width - mBounds.extent.x;

   mSc->mBounds.extent.set(width-1, maxYdis);

   mSc->registerObject();
   mTl->registerObject();
   mBackground->registerObject();

   mSc->addObject( mTl );
   mBackground->addObject( mSc );

   root->pushDialogControl(mBackground,canCtrl->mLayer);

   if ( setScroll )
   {
      if ( mSelIndex )
         mTl->scrollSelectionTop();
      else
         mTl->scrollCellVisible( Point2I( 0, 0 ) );
   }

   mTl->setFirstResponder();
   mTl->mouseLock();

   mInAction = true;
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrl::addChildren()
{
   mTl = new GuiPopUpTextListCtrl(this);
   AssertFatal(mTl, "Failed to create the GuiPopUpTextListCtrl for the PopUpMenu");
   mTl->mProfile = mProfile;
   mTl->setField("noDuplicates", "false");

   mSc = new GuiScrollCtrl;
   AssertFatal(mSc, "Failed to create the GuiScrollCtrl for the PopUpMenu");
   mSc->mProfile = mProfile;
   mSc->setField("hScrollBar","AlwaysOff");
   mSc->setField("vScrollBar","dynamic");

   mBackground = new GuiBackgroundCtrl;
   AssertFatal(mBackground, "Failed to create the GuiBackgroundCtrl for the PopUpMenu");
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrl::repositionPopup()
{
   if ( !mInAction || !mSc || !mTl )
      return;

   // I'm not concerned with this right now...
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrl::reverseTextList()
{
   mTl->clear();
   for(S32 i=mEntries.size()-1; i >= 0; --i)
      mTl->addEntry(mEntries[i].id, mEntries[i].buf);

   // Don't lose the selected cell:
   if ( mSelIndex >= 0 )
      mTl->setSelectedCell( Point2I( 0, mEntries.size() - mSelIndex - 1 ) ); 

   mRevNum = mEntries.size() - 1;
}

//------------------------------------------------------------------------------
bool GuiPopUpMenuCtrl::getFontColor( ColorI &fontColor, S32 id, bool selected, bool mouseOver )
{
   U32 i;
   Entry* entry = NULL;
   for ( i = 0; i < mEntries.size(); i++ )
   {
      if ( mEntries[i].id == id )
      {
         entry = &mEntries[i];
         break;
      }
   }

   if ( !entry )
      return( false );

   if ( entry->scheme != 0 )
   {
      // Find the entry's color scheme:
      for ( i = 0; i < mSchemes.size(); i++ )
      {
         if ( mSchemes[i].id == entry->scheme )
         {
            fontColor = selected ? mSchemes[i].fontColorSEL : mouseOver ? mSchemes[i].fontColorHL : mSchemes[i].fontColor;
            return( true );
         }
      }
   }

   // Default color scheme...
   fontColor = selected ? mProfile->mFontColorSEL : mouseOver ? mProfile->mFontColorHL : mProfile->mFontColor;

   return( true );
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrl::onMouseDown(const GuiEvent &event)
{
   event;
   onAction();
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrl::onMouseUp(const GuiEvent &event)
{
   event;
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrl::setupAutoScroll(const GuiEvent &event)
{
   GuiControl *parent = getParent();
   if (! parent) return;
 
   Point2I mousePt = mSc->globalToLocalCoord(event.mousePoint);
      
   mEventSave = event;      
         
   if(mLastYvalue != mousePt.y)
   {
      mScrollDir = GuiScrollCtrl::None;
      if(mousePt.y > mSc->mBounds.extent.y || mousePt.y < 0)
      {
         S32 topOrBottom = (mousePt.y > mSc->mBounds.extent.y) ? 1 : 0;
         mSc->scrollTo(0, topOrBottom);
         return;
      }   

      F32 percent = (F32)mousePt.y / (F32)mSc->mBounds.extent.y;
      if(percent > 0.7f && mousePt.y > mLastYvalue)
      {
         mIncValue = percent - 0.5f;
         mScrollDir = GuiScrollCtrl::DownArrow;
      }
      else if(percent < 0.3f && mousePt.y < mLastYvalue)
      {
         mIncValue = 0.5f - percent;         
         mScrollDir = GuiScrollCtrl::UpArrow;
      }
      mLastYvalue = mousePt.y;
   }
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrl::autoScroll()
{
   mScrollCount += mIncValue;

   while(mScrollCount > 1)
   {
      mSc->autoScroll(mScrollDir);
      mScrollCount -= 1;
   }
   mTl->onMouseMove(mEventSave);
}

//------------------------------------------------------------------------------
void GuiPopUpMenuCtrl::replaceText(S32 boolVal)
{
   mReplaceText = boolVal;
}
// EOF //
