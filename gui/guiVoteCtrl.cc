//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "console/console.h"
#include "console/consoleTypes.h"
#include "dgl/dgl.h"

#include "GUI/guiVoteCtrl.h"

GuiVoteCtrl::GuiVoteCtrl()
{
   mYesProgress = 0.0f;
   mNoProgress = 0.0f;
   mQuorum = 0.5f;
   mPassHash = 0.25f;
}

const char* GuiVoteCtrl::getScriptValue()
{
   return NULL;
}

void GuiVoteCtrl::setScriptValue(const char *)
{
}

void GuiVoteCtrl::setQuorumValue(F32 value)
{
   if(!value)
      mQuorum = 0.0f;
   else
      mQuorum = value;
   
   //validate the value
   mQuorum = mClampF(mQuorum, 0.f, 1.f);
}

void GuiVoteCtrl::setPassValue(F32 value)
{
   if(!value)
      mPassHash = 0.0f;
   else
      mPassHash = value;
   
   //validate the value
   mPassHash = mClampF(mPassHash, 0.f, 1.f);
}

void GuiVoteCtrl::setYesValue(F32 value)
{
   if(!value)
      mYesProgress = 0.0f;
   else
      mYesProgress = value;
   
   //validate the value
   mYesProgress = mClampF(mYesProgress, 0.f, 1.f);
}

void GuiVoteCtrl::setNoValue(F32 value)
{
   if(!value)
      mNoProgress = 0.0f;
   else
      mNoProgress = value;
   
   //validate the value
   mNoProgress = mClampF(mNoProgress, 0.f, 1.f);
}

void GuiVoteCtrl::onPreRender()
{
}

void GuiVoteCtrl::onRender(Point2I offset, const RectI & /*updateRect*/, GuiControl * /*firstResponder*/)
{
   RectI ctrlRect(offset, mBounds.extent);
   RectI yesProgressRect = ctrlRect;
   RectI noProgressRect = ctrlRect;
   S32 yesWidth;
   S32 noWidth;
   
   ColorI fill;
   
   //draw the "Yes" progress
   if(mYesProgress < 1)
      yesWidth = (S32)((F32)mBounds.extent.x * (1 - mYesProgress));
   else
      yesWidth = ctrlRect.extent.x;
   
   if(yesWidth > 0)
   {
      yesProgressRect.point.y += 10;
      yesProgressRect.point.x = ctrlRect.point.x;
      yesProgressRect.extent.y -= 20;
      
      if(mYesProgress < 1)
         yesProgressRect.extent.x = ctrlRect.extent.x - yesWidth;
      else
         yesProgressRect.extent.x = ctrlRect.extent.x;
         
      fill.set(0, 255, 0, 255);
      
      if(mYesProgress >= mPassHash)
      {
         S32 time = (S32)Platform::getVirtualMilliseconds();
         F32 alpha = F32(time % 500) / 250.0f;
   
         if(alpha > 1)
            alpha = 1.f - (alpha - 1.f);

         fill.alpha *= alpha;
      }
      
      dglDrawRectFill(yesProgressRect, fill);
   }
   
   // draw the "No" progress
   noWidth = (S32)((F32)mBounds.extent.x * mNoProgress);
   
   fill.set(255, 0, 0, 255);
   
   if(noWidth > 0)
   {
      noProgressRect.point.y += 10;
      noProgressRect.point.x = ctrlRect.point.x + (S32)((F32)mBounds.extent.x * mYesProgress);
      noProgressRect.extent.y -= 20;
      noProgressRect.extent.x = noWidth;
      
      if(mNoProgress >= ( 1 - mPassHash ))
      {
         S32 time = (S32)Platform::getVirtualMilliseconds();
         F32 alpha = F32(time % 500) / 250.0f;
   
         if(alpha > 1)
            alpha = 1.f - (alpha - 1.f);

         fill.alpha *= alpha;
      }
      
      dglDrawRectFill(noProgressRect, fill);
   }
}

// console functions
////////////////////////////////////////////////////////////////
static const char* cGetQuorumValue(SimObject *obj, S32, const char **)
{
   GuiVoteCtrl *ctrl = static_cast<GuiVoteCtrl*>(obj);
   
   F32 quorum = ctrl->getQuorumValue();
   char * ret = Con::getReturnBuffer(64);
   dSprintf(ret, 64, "%f", quorum);
   return ret;
}

static const char* cGetPassValue(SimObject *obj, S32, const char **)
{
   GuiVoteCtrl *ctrl = static_cast<GuiVoteCtrl*>(obj);
   
   F32 hash = ctrl->getPassValue();
   char * ret = Con::getReturnBuffer(64);
   dSprintf(ret, 64, "%f", hash);
   return ret;
}

static const char* cGetYesValue(SimObject *obj, S32, const char **)
{
   GuiVoteCtrl *ctrl = static_cast<GuiVoteCtrl*>(obj);
   
   F32 yesValue = ctrl->getYesValue();
   char * ret = Con::getReturnBuffer(64);
   dSprintf(ret, 64, "%f", yesValue);
   return ret;
}

static const char* cGetNoValue(SimObject *obj, S32, const char **)
{
   GuiVoteCtrl *ctrl = static_cast<GuiVoteCtrl*>(obj);
   
   F32 noValue = ctrl->getNoValue();
   char * ret = Con::getReturnBuffer(64);
   dSprintf(ret, 64, "%f", noValue);
   return ret;
}

static void cSetQuorumValue(SimObject *obj, S32, const char **argv)
{
   GuiVoteCtrl *ctrl = static_cast<GuiVoteCtrl*>(obj);
   
   ctrl->setQuorumValue(dAtof(argv[2]));
}

static void cSetPassValue(SimObject *obj, S32, const char **argv)
{
   GuiVoteCtrl *ctrl = static_cast<GuiVoteCtrl*>(obj);
   
   ctrl->setPassValue(dAtof(argv[2]));
}

static void cSetYesValue(SimObject *obj, S32, const char **argv)
{
   GuiVoteCtrl *ctrl = static_cast<GuiVoteCtrl*>(obj);
   
   ctrl->setYesValue(dAtof(argv[2]));
}

static void cSetNoValue(SimObject *obj, S32, const char **argv)
{
   GuiVoteCtrl *ctrl = static_cast<GuiVoteCtrl*>(obj);
   
   ctrl->setNoValue(dAtof(argv[2]));
}

void GuiVoteCtrl::consoleInit()
{
   Parent::consoleInit();
   
   Con::addCommand("GuiVoteCtrl", "setQuorumValue",   cSetQuorumValue,  "ctrl.setQuorumValue(value)",  3, 3);
   Con::addCommand("GuiVoteCtrl", "setPassValue",     cSetPassValue,    "ctrl.setPassValue(value)",    3, 3);
   Con::addCommand("GuiVoteCtrl", "setYesValue",      cSetYesValue,     "ctrl.setYesValue(value)",     3, 3);
   Con::addCommand("GuiVoteCtrl", "setNoValue",       cSetNoValue,      "ctrl.setNoValue(value)",      3, 3);
   Con::addCommand("GuiVoteCtrl", "getQuorumValue",   cGetQuorumValue,  "ctrl.getQuorumValue()",       2, 2);
   Con::addCommand("GuiVoteCtrl", "getPassValue",     cGetPassValue,    "ctrl.getPassValue()",         2, 2);
   Con::addCommand("GuiVoteCtrl", "getYesValue",      cGetYesValue,     "ctrl.getYesValue()",          2, 2);
   Con::addCommand("GuiVoteCtrl", "getNoValue",       cGetNoValue,      "ctrl.getNoValue()",           2, 2);
}
