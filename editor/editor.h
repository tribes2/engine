//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _EDITOR_H_
#define _EDITOR_H_

#ifndef _MMATRIX_H_
#include "Math/mMatrix.h"
#endif
#ifndef _GUICONTROL_H_
#include "GUI/guiControl.h"
#endif

class GameBase;

//------------------------------------------------------------------------------

class EditManager : public GuiControl
{
   private:
      typedef GuiControl Parent;
      
   public:
      EditManager();
      ~EditManager();
      
      bool onWake();
      void onSleep();
      
      // SimObject
      bool onAdd();
      static void consoleInit();

      MatrixF mBookmarks[10];
      DECLARE_CONOBJECT(EditManager);
};

extern bool gEditingMission;

//------------------------------------------------------------------------------

#endif
