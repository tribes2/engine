//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "dgl/dgl.h"
#include "GUI/guiChatMenuTreeCtrl.h"
#include "console/consoleTypes.h"
#include "Platform/event.h"

IMPLEMENT_CONOBJECT(GuiChatMenuTreeCtrl);

//------------------------------------------------------------------------------
GuiChatMenuTreeCtrl::GuiChatMenuTreeCtrl() : GuiTreeViewCtrl()
{
	mTexRollover = NULL;
	mTexSelected = NULL;

   // Default colors:
   mAltFontColor.set( 6, 215, 245 );
   mAltFontColorHL.set( 6, 215, 245 );
   mAltFontColorSE.set( 25, 56, 68 );
}


//------------------------------------------------------------------------------
void GuiChatMenuTreeCtrl::initPersistFields()
{
   Parent::initPersistFields();
   addField( "altFontColor",     TypeColorI, Offset( mAltFontColor, GuiChatMenuTreeCtrl ) );
   addField( "altFontColorHL",   TypeColorI, Offset( mAltFontColorHL, GuiChatMenuTreeCtrl ) );
   addField( "altFontColorSE",   TypeColorI, Offset( mAltFontColorSE, GuiChatMenuTreeCtrl ) );
}


//------------------------------------------------------------------------------
void GuiChatMenuTreeCtrl::consoleInit()
{
}


//------------------------------------------------------------------------------
bool GuiChatMenuTreeCtrl::onWake()
{
	if ( !Parent::onWake() )
		return( false );
		
   // Set the item height to the height of the bitmaps so we know they
   // will connect up right:
   mItemHeight = mImageBounds[BmpDunno].extent.y;

	char buf[512];
	dSprintf( buf, sizeof( buf ), "%s_rol.png", mProfile->mBitmapBase );
	mTexRollover = TextureHandle( buf, BitmapTexture );
	dSprintf( buf, sizeof( buf ), "%s_act.png", mProfile->mBitmapBase );
	mTexSelected = TextureHandle( buf, BitmapTexture );

	return( true );
}


//------------------------------------------------------------------------------
void GuiChatMenuTreeCtrl::onSleep()
{
	mTexRollover = NULL;
	mTexSelected = NULL;
   Parent::onSleep();
}


//------------------------------------------------------------------------------
bool GuiChatMenuTreeCtrl::onKeyDown( const GuiEvent& event )
{
   if ( !mVisible || !mActive || !mAwake )
      return true;

   if ( !mSelectedItem )
      return true;

   Item* item = getItem( mSelectedItem );
   if ( !item )
      return true;

   if ( event.modifier == 0 )
   {
      if ( event.keyCode == KEY_RETURN )
      {
         if ( mAltConsoleCommand[0] )
            Con::evaluate( mAltConsoleCommand );
         return true;
      }
      
      if ( event.keyCode == KEY_DELETE )
      {
         // Don't delete the root!
         if ( item->mParent )
         {
            removeItem( item->mId );
            return true;
         }
      }
   }

   // The Alt key lets you move items around:
   if ( event.modifier & SI_ALT )
   {
      switch ( event.keyCode )
      {
         case KEY_UP:
            if ( item->mPrevious )
            {
               moveItemUp( item->mId );
               buildVisibleTree();
            }
            return true;

         case KEY_DOWN:
            if ( item->mNext )
            {
               moveItemUp( item->mNext->mId );
               buildVisibleTree();
            }
            return true;

         case KEY_LEFT:
            if ( item->mParent )
            {
               if ( item->mParent->mParent )
               {
                  // We're movin'...
                  if ( item->mPrevious )
                     item->mPrevious->mNext = item->mNext;
                  else
                     item->mParent->mChild = item->mNext;

                  if ( item->mNext )
                     item->mNext->mPrevious = item->mPrevious;

                  // Make the item be its former parent's next sibling:
                  item->mPrevious = item->mParent;
                  item->mNext = item->mParent->mNext;

                  if ( item->mNext )
                     item->mNext->mPrevious = item;
                  item->mParent->mNext = item;

                  item->mParent = item->mParent->mParent;
                  buildVisibleTree();
               }
            }
            return true;

         case KEY_RIGHT:
            if ( item->mPrevious )
            {
               // Make the item the last child of its previous sibling:
               if ( dStrcmp( item->mPrevious->mValue, "0" ) == 0 )
               {
                  item->mPrevious->mNext = item->mNext;

                  if ( item->mNext )
                     item->mNext->mPrevious = item->mPrevious;

                  item->mParent = item->mPrevious;
                  item->mNext = NULL;

                  if ( item->mParent->mChild )
                  {
                     Item* temp = item->mParent->mChild;
                     while ( temp->mNext )
                        temp = temp->mNext;

                     temp->mNext = item;
                     item->mPrevious = temp;
                  }
                  else
                  {
                     // only child...<sniff>
                     item->mParent->mChild = item;
                     item->mPrevious = NULL;
                  }

                  // Make sure the new parent is expanded:
                  if ( !item->mParent->mState.test( Item::Expanded ) )
                     expandItem( item->mParent->mId, true );
                  buildVisibleTree();
               }
            }
            return true;
      }
   }

   // Not handled, so pass to parent:
   return Parent::onKeyDown( event );
}


//------------------------------------------------------------------------------
void GuiChatMenuTreeCtrl::onRenderCell( Point2I offset, Point2I cell, bool, bool )
{
   AssertFatal( cell.y < mVisibleItems.size(), "GuiChatMenuTreeCtrl::onRenderCell: invalid cell" );
   Item* item = mVisibleItems[cell.y];
	RectI drawRect( offset, mCellSize );
   U32 bitmap;
   dglClearBitmapModulation();

   // Draw inheritance lines:
   drawRect.point.x += ( mTabSize * item->mTabLevel );
   Item* parent = item->mParent;
   for ( S32 i = item->mTabLevel; ( parent && i > 0 ); i-- )
   {
      drawRect.point.x -= mTabSize;
      if ( parent->mNext )
         dglDrawBitmapSR( mImagesHandle, drawRect.point, mImageBounds[BmpLine] );

      parent = parent->mParent;
   }

   // Draw the bitmap:
	drawRect.point.x = offset.x + mTabSize * item->mTabLevel;

   if ( !item->mChild )
      bitmap = item->mNext ? BmpChild : BmpLastChild;
   else 
   {
      bitmap = item->mState.test( Item::Expanded ) ? BmpExp : BmpCon;

      if ( item->mParent || item->mPrevious )
         bitmap += ( item->mNext ? 3 : 2 );
      else
         bitmap += ( item->mNext ? 1 : 0 );
   }
      
   if ( ( bitmap >= 0 ) && ( bitmap < mNumImages ) )
   {
      dglDrawBitmapSR( mImagesHandle, drawRect.point, mImageBounds[bitmap] );

      // Draw the rollover glow if applicable:
      if ( bitmap > BmpChild && item->mState.test( Item::MouseOverBmp ) )
         dglDrawBitmapSR( mImagesHandle, drawRect.point, mImageBounds[BmpGlow] );

      drawRect.point.x += ( mImageBounds[bitmap].extent.x + 2 );
   }

	// Draw the rollover/selected bar if applicable:
   drawRect.extent.x = mProfile->mFont->getStrWidth( item->mText ) + ( 2 * mTextOffset );
   if ( item->mState.test( Item::Selected ) && mTexSelected )
		dglDrawBitmapStretch( mTexSelected, drawRect );
	else if ( item->mState.test( Item::MouseOverText ) && mTexRollover )
		dglDrawBitmapStretch( mTexRollover, drawRect );

	drawRect.point.x += mTextOffset;
   ColorI fontColor;
   if ( dStrcmp( item->mValue, "0" ) )
   	fontColor = item->mState.test( Item::Selected ) ? mProfile->mFontColorSEL : ( item->mState.test( Item::MouseOverText ) ? mProfile->mFontColorHL : mProfile->mFontColor );
   else
      fontColor = item->mState.test( Item::Selected ) ? mAltFontColorSE : ( item->mState.test( Item::MouseOverText ) ? mAltFontColorHL : mAltFontColor );
   dglSetBitmapModulation( fontColor );
   dglDrawText( mProfile->mFont, drawRect.point, item->mText, mProfile->mFontColors );
}
