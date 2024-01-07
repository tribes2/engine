//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "GUI/guiTreeViewCtrl.h"
#include "GUI/guiScrollCtrl.h"
#include "console/consoleTypes.h"
#include "console/console.h"
#include "dgl/dgl.h"
#include "GUI/guiTypes.h"
#include "Platform/event.h"

IMPLEMENT_CONOBJECT(GuiTreeViewCtrl);

GuiTreeViewCtrl::Item::Item()
{
   mText = NULL;
   mValue = NULL;
}

GuiTreeViewCtrl::Item::~Item()
{
   if ( mText )
   {
      delete [] mText;
      mText = NULL;
   }

   if ( mValue )
   {
      delete [] mValue;
      mValue = NULL;
   }
}


//------------------------------------------------------------------------------

GuiTreeViewCtrl::GuiTreeViewCtrl()
{
   VECTOR_SET_ASSOCIATION(mItems);
   VECTOR_SET_ASSOCIATION(mVisibleItems);
   VECTOR_SET_ASSOCIATION(mImageBounds);

   mItemFreeList = 0;
   mRoot = 0;
   mItemCount = 0;
   mSelectedItem = 0;

   // persist info..
   mTabSize = 16;
   mImagesBitmap = StringTable->insert("");
   mNumImages = 0;
   mTextOffset = 2;
   mFullRowSelect = false;
   mItemHeight = 20;

   //
   setSize(Point2I(1, 0));
}

GuiTreeViewCtrl::~GuiTreeViewCtrl()
{
   destroyTree();
}

//------------------------------------------------------------------------------

GuiTreeViewCtrl::Item * GuiTreeViewCtrl::getItem(S32 itemId)
{
   if((itemId > 0) && (itemId <= mItems.size()))
      return(mItems[itemId-1]);
   return(0);
}

//------------------------------------------------------------------------------

GuiTreeViewCtrl::Item * GuiTreeViewCtrl::createItem()
{
   Item * item;

   // grab from the free list?
   if(mItemFreeList)
   {
      item = mItemFreeList;
      mItemFreeList = item->mNext;

      // re-add to vector
      mItems[item->mId-1] = item;
   }
   else
   {
      item = new Item;
      mItems.push_back(item);

      // set the id
      item->mId = mItems.size();
   }

   // reset
   item->mNext = item->mPrevious = item->mParent = item->mChild = 0;
   item->mText = 0;
   item->mValue = 0;
   item->mState = 0;
   item->mTabLevel = 0;

   mItemCount++;
   return(item);
}

//------------------------------------------------------------------------------

void GuiTreeViewCtrl::destroyItem(Item * item)
{
   if(!item)
      return;

   // free memory:
   if ( item->mText )
   {
      delete [] item->mText;
      item->mText = NULL;
   }
   if ( item->mValue )
   {
      delete [] item->mValue;
      item->mValue = NULL;
   }

   // unlink
   if(item->mPrevious)
      item->mPrevious->mNext = item->mNext;
   if(item->mNext)
      item->mNext->mPrevious = item->mPrevious;
   if(item->mParent && (item->mParent->mChild == item))
      item->mParent->mChild = item->mNext;

   // destroy all the children
   while(item->mChild)
      destroyItem(item->mChild);

   // remove from vector
   mItems[item->mId-1] = 0;

   // set as root free item
   item->mNext = mItemFreeList;
   mItemFreeList = item;

   //
   if(item->mState.test(Item::Expanded))
      mFlags.set(RebuildVisible);

   mItemCount--;
}

//------------------------------------------------------------------------------

void GuiTreeViewCtrl::destroyTree()
{
   // clear the item list
   for(U32 i = 0; i < mItems.size(); i++)
      delete mItems[i];
   mItems.clear();

   // clear the free list
   while(mItemFreeList)
   {
      Item * next = mItemFreeList->mNext;
      delete mItemFreeList;
      mItemFreeList = next;
   }

   mVisibleItems.clear();
   
   //
   mItemFreeList = 0;
   mRoot = 0;
   mItemCount = 0;
   mSelectedItem = 0;
}

//------------------------------------------------------------------------------

void GuiTreeViewCtrl::buildItem( Item* item, U32 tabLevel )
{
   if ( !item )
      return;

   item->mTabLevel = tabLevel;
   mVisibleItems.push_back( item );

   if ( bool( mProfile->mFont ) && item->mText )
   {
      S32 width = ( tabLevel + 1 ) * mTabSize + mProfile->mFont->getStrWidth(item->mText);
      if ( mNumImages > 0 )
         width += mImageBounds[0].extent.x;

      if ( width > mMaxWidth )
         mMaxWidth = width;
   }

   // if expanded, then add all the children items as well
   if ( item->mState.test( Item::Expanded ) )
   {
      Item * child = item->mChild;
      while ( child )
      {
         buildItem( child, tabLevel + 1 );
         child = child->mNext;
      }
   }
}

//------------------------------------------------------------------------------

void GuiTreeViewCtrl::buildVisibleTree()
{
   mMaxWidth = 0;
   mVisibleItems.clear();

   // build the root items
   Item * traverse = mRoot;
   while(traverse)
   {
      buildItem(traverse, 0);
      traverse = traverse->mNext;
   }

   // adjust the GuiArrayCtrl
   mCellSize.set(mMaxWidth+1, mItemHeight);
   setSize(Point2I(1, mVisibleItems.size()));

   // Now, if in a scroll control, make sure we are positioned correctly:
   GuiControl *pappy = getParent();
   if ( !pappy || !dynamic_cast<GuiScrollContentCtrl*>( pappy ) )
      return;

   Point2I diff( pappy->mBounds.extent.x - mBounds.extent.x,
      pappy->mBounds.extent.y - mBounds.extent.y );
   Point2I newPos( ( ( diff.x >= 0 ) ? 0 : ( mBounds.point.x < diff.x ) ? diff.x : mBounds.point.x ),
      ( ( diff.y >= 0 ) ? 0 : ( mBounds.point.y < diff.y ) ? diff.y : mBounds.point.y ) );

   if ( newPos != mBounds.point )
      resize( newPos, mBounds.extent );
}

//------------------------------------------------------------------------------

S32 GuiTreeViewCtrl::insertItem(S32 parentId, const char * text, const char * value, S16 normalImage, S16 expandedImage)
{
   if((parentId < 0) || (parentId > mItems.size()))
   {
      Con::errorf(ConsoleLogEntry::General, "GuiTreeViewCtrl::insertItem: invalid parent id!");
      return(0);
   }

   if((parentId != 0) && (mItems[parentId-1] == 0))
   {
      Con::errorf(ConsoleLogEntry::General, "GuiTreeViewCtrl::insertItem: parent item invalid!");
      return(0);
   }

   // create an item (assigns id)
   Item * item = createItem();

   // fill the data
   item->mText = new char[dStrlen( text ) + 1];
   dStrcpy( item->mText, text );
   item->mValue = new char[dStrlen( value ) + 1];
   dStrcpy( item->mValue, value );
   item->mNormalImage = normalImage;
   item->mExpandedImage = expandedImage;
      
   // root level?
   if(parentId == 0)
   {
      // insert back
      if(mRoot)
      {
         Item * traverse = mRoot;
         while(traverse->mNext)
            traverse = traverse->mNext;

         traverse->mNext = item;
         item->mPrevious = traverse;
      }
      else
         mRoot = item;
      mFlags.set(RebuildVisible);
   }
   else
   {
      Item * parent = mItems[parentId-1];

      // insert back
      if(parent->mChild)
      {
         Item * traverse = parent->mChild;
         while(traverse->mNext)
            traverse = traverse->mNext;

         traverse->mNext = item;
         item->mPrevious = traverse;
      }
      else
         parent->mChild = item;

      item->mParent = parent;

      if(parent->mState.test(Item::Expanded))
         mFlags.set(RebuildVisible);
   }

   //
   if(mFlags.test(RebuildVisible))
      buildVisibleTree();

   return(item->mId);
}

//------------------------------------------------------------------------------

bool GuiTreeViewCtrl::removeItem(S32 itemId)
{
   // tree?
   if(itemId == 0)
   {
      destroyTree();
      return(true);
   }

   Item * item = getItem(itemId);
   if(!item)
   {
      Con::errorf(ConsoleLogEntry::General, "GuiTreeViewCtrl::removeItem: invalid item id!");
      return(false);
   }

   // root?
   if(item == mRoot)
      mRoot = item->mNext;

   if(itemId == mSelectedItem)
   {
      // Select the previous visible item:
      if ( item->mPrevious )
      {
         Item* temp = item->mPrevious;
         while ( temp->mChild && temp->mState.test( Item::Expanded ) )
         {
            temp = temp->mChild;
            while ( temp->mNext )
               temp = temp->mNext;
         }
         selectItem( temp->mId, true );
         buildVisibleTree();
      }
      // or select parent:
      else if ( item->mParent )
      {
         selectItem( item->mParent->mId, true );
         buildVisibleTree();
      }
   }

   destroyItem(item);

   if(mFlags.test(RebuildVisible))
      buildVisibleTree();

   return(true);
}


//------------------------------------------------------------------------------

S32 GuiTreeViewCtrl::getFirstRootItem()
{
   if(!mRoot)
      return(0);

   return(mRoot->mId);
}

//------------------------------------------------------------------------------

S32 GuiTreeViewCtrl::getChildItem(S32 itemId)
{
   Item * item = getItem(itemId);
   if(!item)
   {
      Con::errorf(ConsoleLogEntry::General, "GuiTreeViewCtrl::getChild: invalid item id!");
      return(0);
   }

   return(item->mChild ? item->mChild->mId : 0);
}

S32 GuiTreeViewCtrl::getParentItem(S32 itemId)
{
   Item * item = getItem(itemId);
   if(!item)
   {
      Con::errorf(ConsoleLogEntry::General, "GuiTreeViewCtrl::getParent: invalid item id!");
      return(0);
   }

   return(item->mParent ? item->mParent->mId : 0);
}

S32 GuiTreeViewCtrl::getNextSiblingItem(S32 itemId)
{
   Item * item = getItem(itemId);
   if(!item)
   {
      Con::errorf(ConsoleLogEntry::General, "GuiTreeViewCtrl::getNextSibling: invalid item id!");
      return(0);
   }

   return(item->mNext ? item->mNext->mId : 0);
}

S32 GuiTreeViewCtrl::getPrevSiblingItem(S32 itemId)
{
   Item * item = getItem(itemId);
   if(!item)
   {
      Con::errorf(ConsoleLogEntry::General, "GuiTreeViewCtrl::getPrevSibling: invalid item id!");
      return(0);
   }

   return(item->mPrevious ? item->mPrevious->mId : 0);
}

//------------------------------------------------------------------------------

S32 GuiTreeViewCtrl::getItemCount()
{
   return(mItemCount);
}

S32 GuiTreeViewCtrl::getSelectedItem()
{
   return mSelectedItem;
}

//------------------------------------------------------------------------------

void GuiTreeViewCtrl::moveItemUp( S32 itemId )
{
   Item* item = getItem( itemId );
   if ( !item )
   {
      Con::errorf( ConsoleLogEntry::General, "GuiTreeViewCtrl::moveItemUp: invalid item id!");
      return;
   }

   Item* prevItem = item->mPrevious;
   if ( !prevItem )
   {
      Con::errorf( ConsoleLogEntry::General, "GuiTreeViewCtrl::moveItemUp: no previous sibling - how'd this get called?");
      return;
   }

   if ( prevItem->mPrevious )
      prevItem->mPrevious->mNext = item;
   else if ( item->mParent )
      item->mParent->mChild = item;

   if ( item->mNext )
      item->mNext->mPrevious = prevItem;

   item->mPrevious = prevItem->mPrevious;
   prevItem->mNext = item->mNext;
   item->mNext = prevItem;
   prevItem->mPrevious = item;

   buildVisibleTree();
}

//------------------------------------------------------------------------------

bool GuiTreeViewCtrl::onWake()
{
   if(!Parent::onWake())
      return(false);

   mImagesHandle = TextureHandle(mImagesBitmap, BitmapKeepTexture);
   if(!bool(mImagesHandle))
      return(false);

   mImageBounds.setSize(mNumImages);

   if(!mImagesHandle.getBitmap())
      return(false);

   // ugh
   if(!createBitmapArray(mImagesHandle.getBitmap(), &mImageBounds[0], 1, mNumImages))
      return(false);

   return(true);   
}

void GuiTreeViewCtrl::onSleep()
{
   mImagesHandle = 0;
   mImageBounds.clear();
   Parent::onSleep();
}

//------------------------------------------------------------------------------

void GuiTreeViewCtrl::onPreRender()
{
   Parent::onPreRender();
}

//------------------------------------------------------------------------------

bool GuiTreeViewCtrl::hitTest(const Point2I & pnt, Item* & item, BitSet32 & flags)
{
   Point2I pos = globalToLocalCoord(pnt);

   //
   flags.clear();
   item = 0;

   // get the hit cell
   Point2I cell((pos.x < 0 ? -1 : pos.x / mCellSize.x),
                (pos.y < 0 ? -1 : pos.y / mCellSize.y));

   // valid?
   if((cell.x < 0 || cell.x >= mSize.x) ||
      (cell.y < 0 || cell.y >= mSize.y))
      return(false);

   flags.set(OnRow);

   // grab it
   AssertFatal(cell.y < mVisibleItems.size(), "GuiTreeViewCtrl::onMouseDown: invalid cell");
   item = mVisibleItems[cell.y];

   S32 min = mTabSize * item->mTabLevel;
   
   // left of icon/text?   
   if(pos.x < min)
      return(true);
   
   // check image
   S32 image = item->mState.test(Item::Expanded) ? item->mExpandedImage : item->mNormalImage;
   if((image >= 0) && (image < mNumImages))
      min += mImageBounds[image].extent.x;

   // image?
   if(pos.x < min)
   {
      flags.set(OnImage);
      return(true);
   }

   // offset
   min += mTextOffset;

   // check text
   min += mProfile->mFont->getStrWidth(item->mText);
   if(pos.x < min)
      flags.set(OnText);

   return(true);
}

bool GuiTreeViewCtrl::selectItem(S32 itemId, bool select)
{
   Item * item = getItem(itemId);
   if(!item)
   {
      Con::errorf(ConsoleLogEntry::General, "GuiTreeViewCtrl::selectItem: invalid item id!");
      return(false);
   }
   
   if(mSelectedItem)
   {
      Item * selected = getItem(mSelectedItem);
      selected->mState.set(Item::Selected, false);
   
      char buf[16];
      dSprintf(buf, 16, "%d", mSelectedItem);
      Con::executef(this, 2, "onUnSelect", buf);
   
      mSelectedItem = 0;
   }

   if(select)
   {
      item->mState.set(Item::Selected, true);
      
      char buf[16];
      dSprintf(buf, 16, "%d", item->mId);
      Con::executef(this, 2, "onSelect", buf);

      mSelectedItem = item->mId;
   }

	setUpdate();

   return(true);
}

bool GuiTreeViewCtrl::expandItem(S32 itemId, bool expand)
{
   Item * item = getItem(itemId);
   if(!item)
   {
      Con::errorf(ConsoleLogEntry::General, "GuiTreeViewCtrl::expandItem: invalid item id!");
      return(false);
   }

   if(item->mState.test(Item::Expanded) == expand)
      return(true);
         
   // expand parents
   if(expand)
   {
      while(item)
      {
         item->mState.set(Item::Expanded, true);
         item = item->mParent;
      }      
   }
   else
      item->mState.set(Item::Expanded, false);

   // rebuild
   buildVisibleTree();
   return(true);
}

const char * GuiTreeViewCtrl::getItemText(S32 itemId)
{
   Item * item = getItem(itemId);
   if(!item)
   {
      Con::errorf(ConsoleLogEntry::General, "GuiTreeViewCtrl::getItemText: invalid item id!");
      return("");
   }

   return(item->mText ? item->mText : "");
}

const char * GuiTreeViewCtrl::getItemValue(S32 itemId)
{
   Item * item = getItem(itemId);
   if(!item)
   {
      Con::errorf(ConsoleLogEntry::General, "GuiTreeViewCtrl::getItemValue: invalid item id!");
      return("");
   }

   return(item->mValue ? item->mValue : "");
}

bool GuiTreeViewCtrl::editItem( S32 itemId, const char* newText, const char* newValue )
{
   Item* item = getItem( itemId );
   if ( !item )
   {
      Con::errorf(ConsoleLogEntry::General, "GuiTreeViewCtrl::editItem: invalid item id!");
      return false;
   }

   delete [] item->mText;
   item->mText = new char[dStrlen( newText ) + 1];
   dStrcpy( item->mText, newText );

   delete [] item->mValue;
   item->mValue = new char[dStrlen( newValue ) + 1];
   dStrcpy( item->mValue, newValue );

   // Update the widths and such:
   buildVisibleTree();

   return true;
}


//------------------------------------------------------------------------------
bool GuiTreeViewCtrl::onKeyDown( const GuiEvent& event )
{
   if ( !mVisible || !mActive || !mAwake )
      return true;

   // All the keyboard functionality requires a selected item, so if none exists...
   if ( !mSelectedItem )
      return true;

   Item* item = getItem( mSelectedItem );
   if ( !item )
      return true;

   // Explorer-esque Navigation...
   switch( event.keyCode )
   {
      case KEY_UP:
         // Select previous visible item:
         if ( item->mPrevious )
         {
            item = item->mPrevious;
            while ( item->mChild && item->mState.test( Item::Expanded ) )
            {
               item = item->mChild;
               while ( item->mNext )
                  item = item->mNext;
            }
            selectItem( item->mId, true );
            buildVisibleTree();
            return true;
         }
         // or select parent:
         if ( item->mParent )
         {
            selectItem( item->mParent->mId, true );
            buildVisibleTree();
            return true;
         }
         break;

      case KEY_DOWN:
         // Selected child if it is visible:
         if ( item->mChild && item->mState.test( Item::Expanded ) )
         {
            selectItem( item->mChild->mId, true );
            buildVisibleTree();
            return true;
         }
         // or select next sibling (recursively):
         do
         {
            if ( item->mNext )
            {
               selectItem( item->mNext->mId, true );
               buildVisibleTree();
               return true;
            }

            item = item->mParent;
         } while ( item );
         break;

      case KEY_LEFT:
         // Contract current menu:
         if ( item->mState.test( Item::Expanded ) )
         {
            expandItem( item->mId, false );
            buildVisibleTree();
            return true;
         }
         // or select parent:
         if ( item->mParent )
         {
            selectItem( item->mParent->mId, true );
            buildVisibleTree();
            return true;
         }
         break;

      case KEY_RIGHT:
         // Expand selected item:
         if ( item->mChild )
         {
            if ( !item->mState.test( Item::Expanded ) )
            {
               expandItem( item->mId, true );
               buildVisibleTree();
               return true;
            }
            // or select child:
            selectItem( item->mChild->mId, true );
            buildVisibleTree();
            return true;
         }
         break;
   }

   // Not processed, so pass the event on:
   return Parent::onKeyDown( event );
}


//------------------------------------------------------------------------------
void GuiTreeViewCtrl::onMouseDown(const GuiEvent & event)
{
   if( !mActive || !mAwake || !mVisible )
   {
      Parent::onMouseDown(event);
      return;
   }

   if ( mProfile->mCanKeyFocus )
      setFirstResponder();

   Item * item = 0;
   BitSet32 hitFlags;

   S32 prevSelected = mSelectedItem;

   //
   if(!hitTest(event.mousePoint, item, hitFlags))
      return;

   //
   if(event.modifier & SI_CTRL)
      selectItem(item->mId, !item->mState.test(Item::Selected));
   else
      selectItem(item->mId, true);

   if ( hitFlags.test( OnText ) && ( event.mouseClickCount > 1 ) && ( prevSelected == mSelectedItem ) && mAltConsoleCommand[0] )
      Con::evaluate( mAltConsoleCommand );

   if(!item->mChild)
      return;

   //
   if ( mFullRowSelect || hitFlags.test( OnImage ) )
   {
      item->mState.toggle( Item::Expanded );
      buildVisibleTree();
   }
}


//------------------------------------------------------------------------------
void GuiTreeViewCtrl::onMouseMove( const GuiEvent &event )
{
   if ( mMouseOverCell.y >= 0 )
      mVisibleItems[mMouseOverCell.y]->mState.clear( Item::MouseOverBmp | Item::MouseOverText );

   Parent::onMouseMove( event );

   if ( mMouseOverCell.y >= 0 )
   {
      Item* item = NULL;
      BitSet32 hitFlags = 0;
      if ( !hitTest( event.mousePoint, item, hitFlags ) )
         return;

      if ( hitFlags.test( OnImage ) )
         item->mState.set( Item::MouseOverBmp );
      else
         item->mState.set( Item::MouseOverText );

      // Always redraw the entire mouse over item, since we are distinguishing
      // between the bitmap and the text:
      setUpdateRegion( Point2I( mMouseOverCell.x * mCellSize.x, mMouseOverCell.y * mCellSize.y ), mCellSize );
   }
}


//------------------------------------------------------------------------------
void GuiTreeViewCtrl::onMouseEnter( const GuiEvent &event )
{
   Parent::onMouseEnter( event );
   onMouseMove( event );
}


//------------------------------------------------------------------------------
void GuiTreeViewCtrl::onMouseLeave( const GuiEvent &event )
{
   if ( mMouseOverCell.y >= 0 )
      mVisibleItems[mMouseOverCell.y]->mState.clear( Item::MouseOverBmp | Item::MouseOverText );

   Parent::onMouseLeave( event );
}


//------------------------------------------------------------------------------
void GuiTreeViewCtrl::onRightMouseDown(const GuiEvent & event)
{
   if(!mActive)
   {
      Parent::onRightMouseDown(event);
      return;
   }

   Item * item = 0;
   BitSet32 hitFlags;

   //
   if(!hitTest(event.mousePoint, item, hitFlags))
      return;

   selectItem(item->mId, true);

   //
   char bufs[2][32];
   dSprintf(bufs[0], 32, "%d", item->mId);
   dSprintf(bufs[1], 32, "%d %d", event.mousePoint.x, event.mousePoint.y);

   Con::executef(this, 3, "onRightMouseDown", bufs[0], bufs[1]);
}

//------------------------------------------------------------------------------

void GuiTreeViewCtrl::onRenderCell(Point2I offset, Point2I cell, bool, bool )
{
   AssertFatal(cell.y < mVisibleItems.size(), "GuiTreeViewCtrl::onRenderCell: invalid cell");
   Item * item = mVisibleItems[cell.y];

   // draw the bitmap   
   S32 image = item->mState.test(Item::Expanded) ? item->mExpandedImage : item->mNormalImage;
   U32 textOffset = mTextOffset;
   if((image >= 0) && (image < mNumImages))
   {
      dglClearBitmapModulation();
      dglDrawBitmapSR(mImagesHandle, Point2I(offset.x + mTabSize * item->mTabLevel, offset.y), mImageBounds[image]);
      textOffset += mImageBounds[image].extent.x;
   }

   if ( item->mState.test( Item::Selected ) )
   {
      dglDrawRectFill( RectI( offset.x + ( mTabSize * item->mTabLevel ), offset.y, mCellSize.x, mCellSize.y ), mProfile->mFillColorHL );
      dglSetBitmapModulation( mProfile->mFontColorHL );
   }
   else
      dglSetBitmapModulation( item->mState.test( Item::MouseOverText ) ? mProfile->mFontColorHL : mProfile->mFontColor );

   dglDrawText( mProfile->mFont, Point2I( textOffset + offset.x + ( mTabSize * item->mTabLevel ), offset.y ), item->mText, mProfile->mFontColors );
}

//------------------------------------------------------------------------------

void GuiTreeViewCtrl::initPersistFields()
{
   Parent::initPersistFields();
   addField("tabSize", TypeS32, Offset(mTabSize, GuiTreeViewCtrl));
   addField("imagesBitmap", TypeString, Offset(mImagesBitmap, GuiTreeViewCtrl));
   addField("numImages", TypeS32, Offset(mNumImages, GuiTreeViewCtrl));   
   addField("textOffset", TypeS32, Offset(mTextOffset, GuiTreeViewCtrl));
   addField("fullRowSelect", TypeBool, Offset(mFullRowSelect, GuiTreeViewCtrl));
   addField("itemHeight", TypeS32, Offset(mItemHeight, GuiTreeViewCtrl));
}

//------------------------------------------------------------------------------

static S32 cInsertItem(SimObject * obj, S32 argc, const char ** argv)
{
   argc;

   GuiTreeViewCtrl * tree = static_cast<GuiTreeViewCtrl*>(obj);
   return(tree->insertItem(dAtoi(argv[2]), argv[3], argv[4], 0, 1));
}

static const char * cGetItemText(SimObject * obj, S32, const char ** argv)
{
   GuiTreeViewCtrl * tree = static_cast<GuiTreeViewCtrl*>(obj);
   return(tree->getItemText(dAtoi(argv[2])));
}

static const char * cGetItemValue(SimObject * obj, S32, const char ** argv)
{
   GuiTreeViewCtrl * tree = static_cast<GuiTreeViewCtrl*>(obj);
   return(tree->getItemValue(dAtoi(argv[2])));
}

static bool cEditItem(SimObject * obj, S32, const char ** argv)
{
   GuiTreeViewCtrl* tree = static_cast<GuiTreeViewCtrl*>(obj);
   return(tree->editItem(dAtoi(argv[2]), argv[3], argv[4]));
}

static bool cRemoveItem(SimObject * obj, S32, const char ** argv)
{
   GuiTreeViewCtrl * tree = static_cast<GuiTreeViewCtrl*>(obj);
   return(tree->removeItem(dAtoi(argv[2])));
}

static void cClear(SimObject * obj, S32, const char **)
{
   GuiTreeViewCtrl * tree = static_cast<GuiTreeViewCtrl*>(obj);
   tree->removeItem(0);
}

static bool cSelectItem(SimObject * obj, S32 argc, const char ** argv)
{
   GuiTreeViewCtrl * tree = static_cast<GuiTreeViewCtrl*>(obj);

   S32 id = dAtoi(argv[2]);
   bool select = true;
   if(argc == 4)
      select = dAtob(argv[3]);
   
   return(tree->selectItem(id, select));
}

static bool cExpandItem(SimObject * obj, S32 argc, const char ** argv)
{
   GuiTreeViewCtrl * tree = static_cast<GuiTreeViewCtrl*>(obj);

   S32 id = dAtoi(argv[2]);
   bool expand = true;
   if(argc == 4)
      expand = dAtob(argv[3]);
   
   return(tree->expandItem(id, expand));
}

static S32 cGetFirstRootItem(SimObject * obj, S32, const char **)
{
   GuiTreeViewCtrl * tree = static_cast<GuiTreeViewCtrl*>(obj);
   return(tree->getFirstRootItem());
}

static S32 cGetChild(SimObject * obj, S32, const char ** argv)
{
   GuiTreeViewCtrl * tree = static_cast<GuiTreeViewCtrl*>(obj);
   return(tree->getChildItem(dAtoi(argv[2])));
}

static S32 cGetParent(SimObject * obj, S32, const char ** argv)
{
   GuiTreeViewCtrl * tree = static_cast<GuiTreeViewCtrl*>(obj);
   return(tree->getParentItem(dAtoi(argv[2])));
}

static S32 cGetNextSibling(SimObject * obj, S32, const char ** argv)
{
   GuiTreeViewCtrl * tree = static_cast<GuiTreeViewCtrl*>(obj);
   return(tree->getNextSiblingItem(dAtoi(argv[2])));
}

static S32 cGetPrevSibling(SimObject * obj, S32, const char ** argv)
{
   GuiTreeViewCtrl * tree = static_cast<GuiTreeViewCtrl*>(obj);
   return(tree->getPrevSiblingItem(dAtoi(argv[2])));
}

static S32 cGetItemCount(SimObject * obj, S32, const char **)
{
   GuiTreeViewCtrl * tree = static_cast<GuiTreeViewCtrl*>(obj);
   return(tree->getItemCount());
}

static S32 cGetSelectedItem(SimObject * obj, S32, const char **)
{
   GuiTreeViewCtrl* tree = static_cast<GuiTreeViewCtrl*>(obj);
   return ( tree->getSelectedItem() );
}

static void cMoveItemUp(SimObject* obj, S32, const char** argv)
{
   GuiTreeViewCtrl* tree = static_cast<GuiTreeViewCtrl*>(obj);
   tree->moveItemUp( dAtoi( argv[2] ) );
}


void GuiTreeViewCtrl::consoleInit()
{
   Con::addCommand("GuiTreeViewCtrl", "insertItem", cInsertItem, "tree.insertItem(parent, name, value, normalImage, expandedImage);", 4, 7);
   Con::addCommand("GuiTreeViewCtrl", "getItemText", cGetItemText, "tree.getItemText(item)", 3, 3);
   Con::addCommand("GuiTreeViewCtrl", "getItemValue", cGetItemValue, "tree.getItemValue(item)", 3, 3);
   Con::addCommand("GuiTreeViewCtrl", "editItem", cEditItem, "tree.editItem(item, \"text\", \"value\")", 5, 5);
   Con::addCommand("GuiTreeViewCtrl", "removeItem", cRemoveItem, "tree.removeItem(item);", 3, 3);
   Con::addCommand("GuiTreeViewCtrl", "clear", cClear, "tree.clear();", 2, 2);
   Con::addCommand("GuiTreeViewCtrl", "selectItem", cSelectItem, "tree.selectItem(item, <bool>);", 3, 4);
   Con::addCommand("GuiTreeViewCtrl", "expandItem", cExpandItem, "tree.expandItem(item, <bool>);", 3, 4);
   Con::addCommand("GuiTreeViewCtrl", "getFirstRootItem", cGetFirstRootItem, "tree.getFirstRootItem();", 2, 2);
   Con::addCommand("GuiTreeViewCtrl", "getChild", cGetChild, "tree.getChild(item);", 3, 3);
   Con::addCommand("GuiTreeViewCtrl", "getParent", cGetParent, "tree.getParent(item);", 3, 3);
   Con::addCommand("GuiTreeViewCtrl", "getNextSibling", cGetNextSibling, "tree.getNextSibling(item);", 3, 3);
   Con::addCommand("GuiTreeViewCtrl", "getPrevSibling", cGetPrevSibling, "tree.getPrevSibling(item);", 3, 3);
   Con::addCommand("GuiTreeViewCtrl", "getItemCount", cGetItemCount, "tree.getItemCount();", 2, 2);
   Con::addCommand("GuiTreeViewCtrl", "getSelectedItem", cGetSelectedItem, "tree.getSelectedItem();", 2, 2);
   Con::addCommand("GuiTreeViewCtrl", "moveItemUp", cMoveItemUp, "tree.moveItemUp(item);", 3, 3);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

static void cGuiTreeViewOpen(SimObject *obj, S32, const char **argv)
{
   GuiTreeView *tree = static_cast<GuiTreeView*>(obj);
   SimSet *treeRoot = NULL;
	SimObject* target = Sim::findObject(argv[2]);
   if (target)
      treeRoot = dynamic_cast<SimSet*>(target);
   if (! treeRoot)
      Sim::findObject(RootGroupId, treeRoot);
   tree->setTreeRoot(treeRoot);
}

void GuiTreeView::consoleInit()
{
   Con::addCommand("GuiTreeView", "open",  cGuiTreeViewOpen,   "treeView.open(obj)",  3, 3);
}

void GuiTreeView::initPersistFields()
{
	Parent::initPersistFields();
   addField("allowMultipleSelections", TypeBool, Offset(mAllowMultipleSelections, GuiTreeView));
   addField("recurseSets", TypeBool, Offset(mRecurseSets, GuiTreeView));
}

GuiTreeView::GuiTreeView()
{
   VECTOR_SET_ASSOCIATION(mObjectList);
   

   mAllowMultipleSelections = false;
   mRecurseSets = false;
   mSize = Point2I(1, 0);
}

bool GuiTreeView::onWake()
{
   if (! Parent::onWake())
      return false;
   
   //get the bitmap texture handle
   mTextureHandle = mProfile->mTextureHandle;
   
   //get the font
   mFont = mProfile->mFont;
   
   bool result = createBitmapArray(mTextureHandle.getBitmap(), mBitmapBounds, 1, BmpCount);
   AssertFatal(result, "Failed to create the bitmap array");

   //init the size
   mCellSize.set(640, mBitmapBounds[BmpParentContinue].extent.y);
   return(true);
}

void GuiTreeView::onSleep()
{
   Parent::onSleep();
   mTextureHandle = NULL;
   mFont = NULL;
}

void GuiTreeView::buildTree(SimSet *srcObj, S32 srcLevel, S32 srcParentIndex)
{
   //loop through the children of the src object
   SimSet::iterator i;
   for(i = srcObj->begin(); i != srcObj->end(); i++)
   {
      //push the child
      mObjectList.push_back(ObjNode(srcLevel, *i, i+1 == srcObj->end(), srcParentIndex));
      
      //if the child is expanded, build the tree under the child
      SimSet *g = dynamic_cast<SimSet *>(*i);
      if(g && g->isExpanded())
      {
         buildTree(g, srcLevel + 1, mObjectList.size() - 1);
      }
   }
}

void GuiTreeView::setTreeRoot(SimSet *srcObj)
{
   if (srcObj)
   {
      mObjectList.clear();
      
      //push the root
      mObjectList.push_back(ObjNode(0, srcObj, true, -1));
      
      if(srcObj->isExpanded())
      {
         buildTree(srcObj, 1, 0);
      }
      
      //set the size
      setSize(Point2I(1, mObjectList.size()));
      
      mRootObject = srcObj;
   }
}

void GuiTreeView::onMouseDown(const GuiEvent &event)
{
   if(! mActive)
   {
      Parent::onMouseDown(event);
      return;
   }

   Point2I pt = globalToLocalCoord(event.mousePoint);
   
   //find out which cell was hit
   Point2I cell((pt.x < 0 ? -1 : pt.x / mCellSize.x), (pt.y < 0 ? -1 : pt.y / mCellSize.y));
   if (cell.x >= 0 && cell.x < mSize.x && cell.y >= 0 && cell.y < mSize.y)
   {
      //see where in the cell the hit happened
      ObjNode *hit = &mObjectList[cell.y];
      S32 statusWidth = mBitmapBounds[BmpParentContinue].extent.x;
      S32 statusOffset = statusWidth * hit->level;
      
      //if we clicked on the expanded icon
      SimSet *grp = dynamic_cast<SimSet *>(hit->object);
      if (pt.x >= statusOffset && pt.x <= statusOffset + statusWidth && grp && grp->size())
      {
         grp->setExpanded(! grp->isExpanded());
         setTreeRoot(mRootObject);
      }

      //if we clicked on the object's name...
      else if (pt.x >= 2 + ((hit->level + 1) * statusWidth))
      {
      	if(!hit->object)
         	return;

			if(mAllowMultipleSelections)
         {
         	// calls 'onInspect' for the clicked object and 'onSelect/onUnselect' for
            // every object (for set recursion)
      		if(event.modifier & SI_CTRL)
         		toggleSelected(hit->object);
         	else if(event.modifier & SI_SHIFT)
         		selectObject(hit->object);
         	else if(event.modifier & SI_ALT)
               setInstantGroup(hit->object);
            else
         		setSelected(hit->object);

				//
			 	inspectObject(hit->object);
         }
         else     
         	// will not call 'onInspect'
         	setSelected(hit->object);
      }
   }
}

GuiTreeView::ObjNode * GuiTreeView::getHitNode(const GuiEvent & event)
{
   Point2I pt = globalToLocalCoord(event.mousePoint);
   
   //find out which cell was hit
   Point2I cell((pt.x < 0 ? -1 : pt.x / mCellSize.x), (pt.y < 0 ? -1 : pt.y / mCellSize.y));
   if (cell.x >= 0 && cell.x < mSize.x && cell.y >= 0 && cell.y < mSize.y)
   {
      //see where in the cell the hit happened
      ObjNode *hit = &mObjectList[cell.y];
      if(!hit->object)
         return(0);

      S32 statusWidth = mBitmapBounds[BmpParentContinue].extent.x;

      //if we clicked on the object's name...
      if (pt.x >= 2 + ((hit->level + 1) * statusWidth))
         return(hit);
   }
   return(0);
}

void GuiTreeView::onRightMouseDown(const GuiEvent & event)
{
   if(!mActive)
   {
      Parent::onMouseDown(event);
      return;
   }

   ObjNode * hit;
   if(!(hit = getHitNode(event)))
      return;

   if(!hit->object)
      return;

   setSelected(hit->object);
}

void GuiTreeView::onRightMouseUp(const GuiEvent & event)
{
   if(!mActive)
   {
      Parent::onMouseDown(event);
      return;
   }

   ObjNode * hit;
   if(!(hit = getHitNode(event)))
      return;

   if(!hit->object)
      return;

   //
   char buf1[32];
   dSprintf(buf1, sizeof(buf1), "%d %d", event.mousePoint.x, event.mousePoint.y);

   char buf2[16];
   dSprintf(buf2, sizeof(buf2), "%d", hit->object->getId());

   Con::executef(this, 3, "onContextMenu", buf1, buf2);
}

void GuiTreeView::setInstantGroup(SimObject * obj)
{
   // make sure a group
   SimGroup * grp = dynamic_cast<SimGroup*>(obj);
   if(!grp)
      return;
      
   char buf[16];
   dSprintf(buf, sizeof(buf), "%d", grp->getId());
   Con::setVariable("instantGroup", buf);
}

void GuiTreeView::inspectObject(SimObject * obj)
{
	char buf[16];
   dSprintf(buf, sizeof(buf), "%d", obj->getId());
   Con::executef(this, 2, "onInspect", buf);
}

void GuiTreeView::selectObject(SimObject * obj)
{
	if(mRecurseSets)
   {
   	SimSet * set = dynamic_cast<SimSet*>(obj);
      if(set)
      	for(SimSet::iterator itr = set->begin(); itr != set->end(); itr++)
         	selectObject(*itr);
   }
   
   //
   obj->setSelected(true);
   
   //
   char buf[16];
   dSprintf(buf, sizeof(buf), "%d", obj->getId());
   Con::executef(this, 2, "onSelect", buf);
}

void GuiTreeView::unselectObject(SimObject * obj)
{
	if(mRecurseSets)
   {
   	SimSet * set = dynamic_cast<SimSet*>(obj);
      if(set)
      	for(SimSet::iterator itr = set->begin(); itr != set->end(); itr++)
         	unselectObject(*itr);
   }

	//	
   obj->setSelected(false);
   
   //
   char buf[16];
   dSprintf(buf, sizeof(buf), "%d", obj->getId());
   Con::executef(this, 2, "onUnselect", buf);
}

void GuiTreeView::clearSelected()
{
	for(U32 i = 0; i < mObjectList.size(); i++)
   	if(mObjectList[i].object->isSelected())
         unselectObject(mObjectList[i].object);
}

void GuiTreeView::toggleSelected(SimObject * obj)
{
	if(obj->isSelected())
      unselectObject(obj);
   else
      selectObject(obj);
}

void GuiTreeView::setSelected(SimObject *selObj)
{
   clearSelected();
   selectObject(selObj);
}

void GuiTreeView::onPreRender()
{
   setTreeRoot(mRootObject);
}

void GuiTreeView::onRenderCell(Point2I offset, Point2I cell, bool, bool)
{
   Point2I cellOffset = offset;
   
   ObjNode *obj = &(mObjectList[cell.y]);
   ObjNode *prev = cell.y ? &(mObjectList[cell.y - 1]) : NULL;
   
   /*
   S32 bitmap = (mVBarEnabled ? ((curHitRegion == UpArrow && stateDepressed) ?
         BmpStates * BmpUp + BmpHilite : BmpStates * BmpUp) : BmpStates * BmpUp + BmpDisabled);

   dglClearBitmapModulation();
   dglDrawBitmapSR(mTextureHandle, pos, mBitmapBounds[bitmap]);
   */
   
   S32 statusWidth = mBitmapBounds[BmpParentContinue].extent.x;
   bool sel = obj->object->isSelected();
   
   //draw the background behind the selected cell
   if (sel)
   {
      RectI selRect(Point2I(cellOffset.x + 2, cellOffset.y), Point2I(mBounds.extent.x - 2, mBounds.extent.y));
      dglDrawRectFill(selRect, ColorI(255, 255, 255));
   }
   
   // check if instantGroup - should draw a bitmap or something else...
   const char * instantGroupName = Con::getVariable("instantGroup");
   SimGroup * instantGroup = dynamic_cast<SimGroup*>(Sim::findObject(instantGroupName));
   if(instantGroup && instantGroup->getId() == obj->object->getId())
   {
      RectI selRect(Point2I(cellOffset.x + 2, cellOffset.y), Point2I(mBounds.extent.x - 2, mBounds.extent.y));
      dglDrawRectFill(selRect, ColorI(200, 200, 200));
   }
   
   //find the status bmp index
   S32 bmpIndex = -1;
   SimSet *grp = dynamic_cast<SimSet *>(obj->object);
   if(grp && grp->size())
   {
      if(grp->isExpanded()) bmpIndex = BmpParentOpen;
      else bmpIndex = BmpParentClosed;
   }
   else bmpIndex = BmpNone;

   //add to the index based on the status
   if (! prev || obj->level == 0)
   {
      if (!obj->lastInGroup) bmpIndex += 1;
   }
   else if (obj->lastInGroup) bmpIndex += 2;
   else bmpIndex += 3;
   
   //now we have our bmpIndex, draw the status
   if (bmpIndex >= 0)
   {
      dglClearBitmapModulation();
      dglDrawBitmapSR(mTextureHandle, Point2I(cellOffset.x + (obj->level * statusWidth), cellOffset.y), mBitmapBounds[bmpIndex]);
   }
   
   //draw in all the required continuation lines
   S32 parent = obj->parentIndex;
   while(parent != -1)
   {
      ObjNode *p = &(mObjectList[parent]);
      if (!p->lastInGroup)
      {
         dglClearBitmapModulation();
         dglDrawBitmapSR(mTextureHandle, Point2I(cellOffset.x + (p->level * statusWidth), cellOffset.y), mBitmapBounds[BmpParentContinue]);
      }
      parent = p->parentIndex;
   }
   
   //draw the name - JFF temp hidden/locked
   char buf[256];
   if(obj->object->isHidden())
      dSprintf(buf, sizeof(buf), "(H) %d: %s - %s", obj->object->getId(), obj->object->getName() ? obj->object->getName() : "", obj->object->getClassName());
   else if(obj->object->isLocked())
      dSprintf(buf, sizeof(buf), "(L) %d: %s - %s", obj->object->getId(), obj->object->getName() ? obj->object->getName() : "", obj->object->getClassName());
   else
      dSprintf(buf, sizeof(buf), "%d: %s - %s", obj->object->getId(), obj->object->getName() ? obj->object->getName() : "", obj->object->getClassName());
   
   dglSetBitmapModulation(sel ? mProfile->mFontColorHL : mProfile->mFontColor);
   dglDrawText(mFont, Point2I(cellOffset.x + (obj->level + 1) * statusWidth, cellOffset.y), buf, mProfile->mFontColors);
}
