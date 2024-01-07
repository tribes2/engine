//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "GUI/guiPopUpCtrl.h"

class GuiControlListPopUp : public GuiPopUpMenuCtrl
{
   typedef GuiPopUpMenuCtrl Parent;
public:
   bool onAdd();
   static void consoleInit();
   
   DECLARE_CONOBJECT(GuiControlListPopUp);
};

IMPLEMENT_CONOBJECT(GuiControlListPopUp);

void GuiControlListPopUp::consoleInit()
{
}

bool GuiControlListPopUp::onAdd()
{
   if(!Parent::onAdd())
      return false;
   clear();
   
   for(AbstractClassRep *rep = AbstractClassRep::getClassList(); rep; rep = rep->getNextClass())
   {
      ConsoleObject *obj = rep->create();
      if(obj && dynamic_cast<GuiControl *>(obj))
         addEntry(rep->getClassName(), 0);
      delete obj;
   }
   return true;
}
