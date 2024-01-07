//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "console/console.h"
#include "dgl/dgl.h"
#include "GUI/guiCanvas.h"
#include "GUI/guiRadioCtrl.h"
#include "console/consoleTypes.h"

//--------------------------------------------------------------------------- 
GuiRadioCtrl::GuiRadioCtrl()
{
   mActive = true;
   mBounds.extent.set(140, 30);
   mKeyPressed = false;
	mStateOn = false;
   mGroupNum = -1;
}

//--------------------------------------------------------------------------- 
void GuiRadioCtrl::initPersistFields()
{
   Parent::initPersistFields();
   addField("groupNum", TypeS32, Offset(mGroupNum, GuiRadioCtrl));
}

//--------------------------------------------------------------------------- 
void GuiRadioCtrl::consoleInit()
{
}

//--------------------------------------------------------------------------- 
void GuiRadioCtrl::AcceleratorKeyPress(void)
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
void GuiRadioCtrl::AcceleratorKeyRelease(void)
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
bool GuiRadioCtrl::onKeyDown(const GuiEvent &event)
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
void GuiRadioCtrl::drawBorder(const RectI &r, const ColorI &color)
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
void GuiRadioCtrl::onAction()
{
	mStateOn = true;	
   if (mGroupNum >= 0)
      messageSiblings(mGroupNum);
      
   setUpdate();
   Parent::onAction();
}

//--------------------------------------------------------------------------- 
void GuiRadioCtrl::onMessage(GuiControl *sender, S32 msg)
{
   Parent::onMessage(sender, msg);
   if (mGroupNum == msg)
   {
      setUpdate();
      mStateOn = (sender == this);
   }
}

//--------------------------------------------------------------------------- 
void GuiRadioCtrl::onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder)
{
   bool stateOver = cursorInControl();
   bool stateDepressed = mGroupNum >= 0 && mStateOn;
   ColorI fontColor = (stateOver ? mProfile->mFontColorHL : mProfile->mFontColor);
   ColorI backColor = mActive ? mProfile->mFillColor : mProfile->mFillColorNA; 
   ColorI insideBorderColor = (firstResponder == this) ? mProfile->mBorderColorHL : mProfile->mBorderColor; 
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

	//draw the radio box
   glColor3f(0.4f,0.4f,0.4f);
   glBegin(GL_LINE_LOOP);
   	glVertex2i(r.point.x+3,  r.point.y+3);
   	glVertex2i(r.point.x+12, r.point.y+3);
   	glVertex2i(r.point.x+12, r.point.y+12);
   	glVertex2i(r.point.x+3,  r.point.y+12);
   glEnd();	
    
   if (stateDepressed)
	{
		glColor3f(0.0f,0.0f,0.0f);
   	glBegin(GL_QUADS);
   		glVertex2i(r.point.x+6,  r.point.y+6);
   		glVertex2i(r.point.x+10, r.point.y+6);
   		glVertex2i(r.point.x+10, r.point.y+10);
   		glVertex2i(r.point.x+6,  r.point.y+10);
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
   if (stateDepressed)
   {
      localStart.x += 1;
      localStart.y += 1;
   }
   Point2I globalStart = localToGlobalCoord(localStart);

   dglSetBitmapModulation(fontColor);
   dglDrawText(mFont, globalStart, mText, mProfile->mFontColors);
   
   //render the children
   renderChildControls(offset, updateRect, firstResponder);
}

//--------------------------------------------------------------------------- 
void GuiRadioCtrl::setScriptValue(const char *newState)
{
	if(dAtob(newState))
		onAction();
	else
		mStateOn = false;
   setUpdate();
}

//--------------------------------------------------------------------------- 
const char *GuiRadioCtrl::getScriptValue()
{
   return mStateOn ? "1" : "0";
}

// EOF //

