//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "console/consoleTypes.h"
#include "console/console.h"
#include "dgl/dgl.h"
#include "GUI/guiCanvas.h"
#include "GUI/guiConsoleEditCtrl.h"

IMPLEMENT_CONOBJECT(GuiConsoleEditCtrl);

GuiConsoleEditCtrl::GuiConsoleEditCtrl()
{
   tabBuffer[0] = 0;
   baseStart = 0;
   baseLen = 0;
}

void GuiConsoleEditCtrl::handleTab(bool forwardTab)
{
   char buf[GuiTextCtrl::MAX_STRING_LENGTH + 1];
   if(dStrcmp(tabBuffer, mText))
   {
      dStrcpy(tabBuffer, mText);
      // scan back from mCursorPos
      S32 p = mCursorPos;
      while(p > 0 && mText[p-1] != ' ' && mText[p-1] != '.' && mText[p-1] != '(')
         p--;
      baseStart = p;
      baseLen = mCursorPos-p;
      if(mText[p-1] == '.')
      {
         if(p <= 1)
            return;
         S32 objLast = --p;
         while(p > 0 && mText[p-1] != ' ')
            p--;
         if(objLast == p)
            return;
         dStrncpy(buf, mText + p, objLast - p);
         buf[objLast - p] = 0;
         tabObject = Sim::findObject(buf);
         if(!bool(tabObject))
            return;
      }
      else
         tabObject = 0;
   }
   const char *newText;
   mText[mCursorPos] = 0;
   
   if(bool(tabObject))
      newText = tabObject->tabComplete(mText + baseStart, baseLen, forwardTab);
   else
      newText = Con::tabComplete(mText + baseStart, baseLen, forwardTab);
   if(newText)
   {
      S32 len = dStrlen(newText);
      if(len + baseStart > GuiTextCtrl::MAX_STRING_LENGTH)
         len = GuiTextCtrl::MAX_STRING_LENGTH - baseStart;
      dStrncpy(mText + baseStart, newText, len);
      mText[baseStart + len] = 0;
      mCursorPos = baseStart + len;
   }
   else
   {
      mText[baseStart + baseLen] = 0;
      mCursorPos = baseStart + baseLen;
   }
   dStrcpy(tabBuffer, mText);
}

bool GuiConsoleEditCtrl::onKeyDown(const GuiEvent &event)
{
   S32 stringLen = dStrlen(mText);
   setUpdate();

   if (event.keyCode == KEY_TAB)   
   {
      if (event.modifier & SI_SHIFT)
      {
         handleTab(false);
         return true;
      }
      else
      {
         handleTab(true);
         return true;
      }
   }
   return Parent::onKeyDown(event);
}

