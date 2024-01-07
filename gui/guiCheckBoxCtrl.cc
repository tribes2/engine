//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "console/console.h"
#include "dgl/dgl.h"
#include "GUI/guiCanvas.h"
#include "GUI/guiCheckBoxCtrl.h"
#include "console/consoleTypes.h"

//--------------------------------------------------------------------------- 
GuiCheckBoxCtrl::GuiCheckBoxCtrl()
{
   mActive = true;
   mBounds.extent.set(140, 30);
   mKeyPressed = false;
	mStateOn = false;
}

//--------------------------------------------------------------------------- 
void GuiCheckBoxCtrl::consoleInit()
{
}

//--------------------------------------------------------------------------- 
void GuiCheckBoxCtrl::setScriptValue(const char *value)
{
	mStateOn = dAtob(value);

	// Update the console variable:
	if ( mConsoleVariable[0] )
		Con::setBoolVariable( mConsoleVariable, mStateOn );

   setUpdate();
}

//--------------------------------------------------------------------------- 
const char *GuiCheckBoxCtrl::getScriptValue()
{
	return mStateOn ? "1" : "0";
}

//--------------------------------------------------------------------------- 
void GuiCheckBoxCtrl::AcceleratorKeyPress(void)
{
   if (! mActive) return;
   
   //set the bool
   mKeyPressed = true;
   
   if (mProfile->mTabable)
   {
      setFirstResponder();
   }
   
}

//--------------------------------------------------------------------------- 
void GuiCheckBoxCtrl::AcceleratorKeyRelease(void)
{
   if (! mActive) return;
   
   //set the bool
   mKeyPressed = false;
   //perform the action
   onAction();
   
   //update
   setUpdate();
}

//--------------------------------------------------------------------------- 
bool GuiCheckBoxCtrl::onKeyDown(const GuiEvent &event)
{
   //if the control is a dead end, kill the event
   if ((! mVisible) || (! mActive) || (! mAwake))
      return true;
   
   //see if the key down is a <return> or not
   if (event.keyCode == KEY_RETURN && event.modifier == 0)
   {
      onAction();
      return true;
   }
   
   //otherwise, pass the event to it's parent
   return Parent::onKeyDown(event);
}

//--------------------------------------------------------------------------- 
void GuiCheckBoxCtrl::drawBorder(const RectI &r, const ColorI &color)
{
   Point2I p1, p2;
   p1 = r.point;
   p2 = r.point;
   p2.x += r.extent.x - 1;
   p2.y += r.extent.y - 1;
   
   glColor4ub(color.red, color.green, color.blue, color.alpha);
   glBegin(GL_LINE_LOOP);
   	glVertex2i(p1.x + 2, p1.y);
   	glVertex2i(p2.x - 2, p1.y);
   	glVertex2i(p2.x, p1.y + 2);
   	glVertex2i(p2.x, p2.y - 2);
   	glVertex2i(p2.x - 2, p2.y);
   	glVertex2i(p1.x + 2, p2.y);
   	glVertex2i(p1.x, p2.y - 2);
   	glVertex2i(p1.x, p1.y + 2);
   glEnd();
}   

//---------------------------------------------------------------------------
void GuiCheckBoxCtrl::onAction()
{
	mStateOn = mStateOn ? false : true;

	// Update the console variable:
	if ( mConsoleVariable[0] )
		Con::setBoolVariable( mConsoleVariable, mStateOn );
			
   Parent::onAction();
}

//--------------------------------------------------------------------------- 
void GuiCheckBoxCtrl::onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder)
{
   ColorI backColor = mActive ? mProfile->mFillColor : mProfile->mFillColorNA; 
   ColorI fontColor = (cursorInControl()) ? mProfile->mFontColorHL : mProfile->mFontColor;
	ColorI insideBorderColor = (firstResponder == this) ? mProfile->mBorderColorHL : mProfile->mBorderColor; 

	if(mText[0] != NULL)
	{
  		ColorI outsideBorderColor = mProfile->mBorderColor;

   	S32 txt_w = mFont->getStrWidth(mText);

   	Point2I localStart;
   
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

   	// center the vertical
   	localStart.y = (mBounds.extent.y - (mFont->getHeight() - 2)) / 2;
	
   	// first draw the background
   	RectI r(offset, mBounds.extent);
   	dglDrawRectFill(r, backColor);
   	r.point.x +=2;
   	r.point.y +=2;
   	r.extent.x -=4;
   	r.extent.y -=4;
   	dglDrawRectFill(r, backColor);

		S32 incVal = mBounds.extent.y - 8;
		//draw the check box
		glColor3i(insideBorderColor.red, insideBorderColor.green, insideBorderColor.blue);
   	glBegin(GL_LINE_LOOP);
   		glVertex2i(r.point.x+3,      r.point.y+3);
   		glVertex2i(r.point.x+incVal, r.point.y+3);
   		glVertex2i(r.point.x+incVal, r.point.y+incVal);
   		glVertex2i(r.point.x+3,      r.point.y+incVal);
   	glEnd();	
   	 
   	if (mStateOn)
		{
			glColor3i(fontColor.red, fontColor.green, fontColor.blue);
   		glBegin(GL_LINES);
   			glVertex2i(r.point.x+3,      r.point.y+3);
   			glVertex2i(r.point.x+incVal, r.point.y+incVal);
   			glVertex2i(r.point.x+incVal, r.point.y+3);
   			glVertex2i(r.point.x+3,      r.point.y+incVal);
   		glEnd();
   	}
   	// draw the oustside border
   	r.point.x -=1;
   	r.point.y -=1;
   	r.extent.x +=2;
   	r.extent.y +=2;
   	drawBorder(r, outsideBorderColor);

   	// finally draw the text
   	localStart.y -= 2;
		localStart.x += 4;
   	if (mStateOn)
   	{
   	   localStart.x += 1;
   	   localStart.y += 1;
   	}
   	Point2I globalStart = localToGlobalCoord(localStart);

   	dglSetBitmapModulation(fontColor);
   	dglDrawText(mFont, globalStart, mText, mProfile->mFontColors);
  	}
   else
	{
   	RectI r(Point2I(offset.x+3, offset.y+3), Point2I(mBounds.extent.x-3, mBounds.extent.y-3));
   	dglDrawRectFill(r, backColor);

		dglDrawRect(r,insideBorderColor);

   	if (mStateOn)
		{
			glColor3i(fontColor.red, fontColor.green, fontColor.blue);
   		glBegin(GL_LINES);
   			glVertex2i(r.point.x-1,              r.point.y);
   			glVertex2i(r.point.x-1 + r.extent.x, r.point.y + r.extent.y);
   			glVertex2i(r.point.x-1 + r.extent.x, r.point.y);
   			glVertex2i(r.point.x-1,              r.point.y + r.extent.y);
   		glEnd();
   	}
	}
   //render the children
   renderChildControls(offset, updateRect, firstResponder);
}
//--------------------------------------------------------------------------- 
// EOF //


