//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUIHELPCTRL_H_
#define _GUIHELPCTRL_H_

#ifndef _GUITEXTCTRL_H_
#include "GUI/guiTextCtrl.h"
#endif

class guiCanvas;

class GuiHelpCtrl : public guiTextCtrl
{

private:
   typedef guiTextCtrl Parent;

protected:
	S32 mHelpTag;
   const char *mHelpText;

public:
    GuiHelpCtrl();
   ~GuiHelpCtrl();

	bool onAdd(void);

   virtual void setHelpText(guiCanvas *root, const char *text, F32 timeElapsed, bool mouseClicked = FALSE);
   virtual void setHelpTag(guiCanvas *root, S32 helpTag, F32 timeElapsed, bool mouseClicked = FALSE);
   virtual void render(GFXSurface *sfc);

   DECLARE_PERSISTENT(GuiHelpCtrl);
};

#endif //_GUI_HELP_CTRL_H
