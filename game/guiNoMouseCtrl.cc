//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "GUI/guiControl.h"

//------------------------------------------------------------------------------
class GuiNoMouseCtrl : public GuiControl
{
   public:

      // GuiControl
      bool pointInControl(const Point2I &)   { return(false); }
      DECLARE_CONOBJECT(GuiNoMouseCtrl);
};
IMPLEMENT_CONOBJECT(GuiNoMouseCtrl);
