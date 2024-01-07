//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "console/console.h"
#include "dgl/dgl.h"
#include "console/consoleTypes.h"
#include "Platform/platformAudio.h"
#include "audio/audioDataBlock.h"
#include "GUI/guiCanvas.h"
#include "GUI/guiButtonCtrl.h"

GuiButtonCtrl::GuiButtonCtrl()
{
   mActive = true;
   mBounds.extent.set(140, 30);
   mButtonText = 0;
   mButtonState    = Normal;
   mMouseInControl = false;
}

void GuiButtonCtrl::consoleInit()
{
}

void GuiButtonCtrl::initPersistFields()
{
   Parent::initPersistFields();
   addField("text", TypeCaseString, Offset(mButtonText, GuiButtonCtrl));
}

const char * GuiButtonCtrl::getScriptValue()
{
   return(mButtonText);
}

void GuiButtonCtrl::setScriptValue(const char * value)
{
   mButtonText = StringTable->insert(value);
}

//--------------------------------------------------------------------------
void GuiButtonCtrl::AcceleratorKeyPress(void)
{
   if ((! mVisible) || (! mActive) || (! mAwake))
      return;

   onAction();
   setFirstResponder();
}


//--------------------------------------------------------------------------
bool GuiButtonCtrl::onKeyDown(const GuiEvent &event)
{
   //if the control is a dead end, kill the event
   if ((! mVisible) || (! mActive) || (! mAwake))
      return true;
   
   //see if the key down is a return or space or not
   if ((event.keyCode == KEY_RETURN || event.keyCode == KEY_SPACE)
       && event.modifier == 0) 
   {
	   if ( mProfile->mSoundButtonDown )
	   {
	      F32 pan = ( F32( event.mousePoint.x ) / F32( Canvas->mBounds.extent.x ) * 2.0f - 1.0f ) * 0.8f;
	      AUDIOHANDLE handle = alxCreateSource( mProfile->mSoundButtonDown );
	      alxSourcef( handle, AL_PAN, pan );
	      alxPlay( handle );
	   }

      onAction();
      return true;
   }
   
   //otherwise, pass the event to it's parent
   return Parent::onKeyDown(event);
}


//--------------------------------------------------------------------------
void GuiButtonCtrl::onMouseDown(const GuiEvent &event)
{
   // Are we answering the phone?
   if (!mVisible || !mActive || !mAwake)
      return;

   // Ok, this is a bona-fide real click.  Check our preconditions
   AssertFatal(mButtonState == Normal, "Error, we should never reach this state!");

   setFirstResponder();
   mButtonState = MouseDown;
   getRoot()->mouseLock(this);

   if (mProfile->mSoundButtonDown)
   {
      F32 pan = (F32(event.mousePoint.x)/F32(Canvas->mBounds.extent.x)*2.0f-1.0f)*0.8f;
      AUDIOHANDLE handle = alxCreateSource(mProfile->mSoundButtonDown);
      alxSourcef(handle, AL_PAN, pan);
      alxPlay(handle);
      //alxPlay(mProfile->mSoundButtonDown, NULL, NULL);
      //Audio::play2D(mProfile->mSoundButtonDown->getName(), mProfile->mSoundButtonDown->getDescription(), pan);
   }
   setUpdate();
}


//--------------------------------------------------------------------------
void GuiButtonCtrl::onMouseUp(const GuiEvent&)
{
   GuiCanvas* root = getRoot();
   if (mButtonState == Normal) {
      AssertFatal(root->getMouseLockedControl() != this, "Error, we should not have the mouse locked here!");
      return;
   }

   mButtonState = Normal;
   setUpdate();

   root->mouseUnlock(this);
   if (cursorInControl())
      onAction();
}


//--------------------------------------------------------------------------
void GuiButtonCtrl::onMouseEnter(const GuiEvent& event)
{
   mMouseInControl = true;

   if ( mActive && mProfile->mSoundButtonOver )
   {
      F32 pan = (F32(event.mousePoint.x)/F32(Canvas->mBounds.extent.x)*2.0f-1.0f)*0.8f;
      AUDIOHANDLE handle = alxCreateSource(mProfile->mSoundButtonOver);
      alxSourcef(handle, AL_PAN, pan);
      alxPlay(handle);

      //alxPlay(mProfile->mSoundButtonOver, NULL, NULL);
      //Audio::play2D(mProfile->mSoundButtonOver->getName(), mProfile->mSoundButtonOver->getDescription(), pan);
   }

   Parent::onMouseEnter( event );
   setUpdate();
}


//--------------------------------------------------------------------------
void GuiButtonCtrl::onMouseLeave(const GuiEvent& event)
{
   mMouseInControl = false;
   Parent::onMouseLeave(event);
   setUpdate();
}


//--------------------------------------------------------------------------
void GuiButtonCtrl::onSleep()
{
   mMouseInControl = false;
   mButtonState = Normal;
   Parent::onSleep();
}

//--------------------------------------------------------------------------
void GuiButtonCtrl::onRender(Point2I      offset,
                             const RectI& updateRect,
                             GuiControl*  firstResponder)
{
   bool highlight = false;
   bool depressed = false;

   if ( mActive )
   {
      if ( mButtonState == MouseDown ) 
      {
         highlight = true;
         depressed = cursorInControl();
      } 
      else 
         highlight = mMouseInControl;
   }

   ColorI fontColor   = mActive ? (highlight ? mProfile->mFontColorHL : mProfile->mFontColor) : mProfile->mFontColorNA;
   ColorI backColor   = mActive ? mProfile->mFillColor : mProfile->mFillColorNA; 
   ColorI borderColor = mActive ? mProfile->mBorderColor : mProfile->mBorderColorNA;

#if 1
   // first draw the background
   Point2I extent( offset.x+mBounds.extent.x-1, offset.y+mBounds.extent.y-1);
   if ( mProfile->mOpaque )
   {
      if (depressed)
         renderLoweredBox(offset, extent, backColor);
      else
         renderRaisedBox(offset, extent, backColor);
   }
   else
      if ( mProfile->mBorder )
         dglDrawRect( offset, Point2I(extent.x+1, extent.y+1), backColor );

#else
   // first draw the background
   RectI r( offset, mBounds.extent );
   if ( mProfile->mOpaque )
      dglDrawRectFill( r, backColor );

   if ( mProfile->mBorder )
   {
      dglDrawRect( r, borderColor );

      if ( mActive && firstResponder == this ) 
      {
         r.point  += Point2I(1, 1);
         r.extent -= Point2I(2, 2);
         dglDrawRect( r, mProfile->mBorderColorHL );
      }
   }
#endif

   // finally draw the text
   if ( mButtonText && mButtonText[0] != NULL )
	{
   	S32 txt_w = mProfile->mFont->getStrWidth( mButtonText );
   	Point2I localStart;
   
   	// align the horizontal
   	switch (mProfile->mAlignment)
   	{
   	   case GuiControlProfile::RightJustify:
   	      localStart.x = mBounds.extent.x - txt_w;  
   	      break;
   	   case GuiControlProfile::CenterJustify:
   	      localStart.x = ( mBounds.extent.x - txt_w ) / 2;
   	      break;
   	   default:
   	      // GuiControlProfile::LeftJustify
   	      localStart.x = 0;
   	      break;
   	}

   	// center the vertical
   	localStart.y = ( mBounds.extent.y - ( mProfile->mFont->getHeight() - 2 ) ) / 2;

   	localStart.y -= 2;
   	if ( depressed )
   	{
   	   localStart.x += 1;
   	   localStart.y += 1;
   	}
      Point2I globalStart = localToGlobalCoord( localStart );

   	dglSetBitmapModulation( fontColor );
   	dglDrawText(mProfile->mFont, globalStart, mButtonText, mProfile->mFontColors);
   
   	//render the children
   	renderChildControls( offset, updateRect, firstResponder );
	}
}

