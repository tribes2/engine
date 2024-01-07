//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUIBUBBLETEXTCTRL_H_
#define _GUIBUBBLETEXTCTRL_H_

#ifndef _GUITEXTCTRL_H_
#include "GUI/guiTextCtrl.h"
#endif
#ifndef _GUIMLTEXTCTRL_H_
#include "GUI/guiMLTextCtrl.h"
#endif

class GuiBubbleTextCtrl : public GuiTextCtrl
{
  private:
   typedef GuiTextCtrl Parent;

  protected:
  	bool mInAction;
	GuiControl *mDlg;
	GuiControl *mPopup;
	GuiMLTextCtrl *mMLText;

	void popBubble();

  public:
   DECLARE_CONOBJECT(GuiBubbleTextCtrl);

	GuiBubbleTextCtrl() { mInAction = false; }

	virtual void onMouseDown(const GuiEvent &event);
};

#endif /* _GUI_BUBBLE_TEXT_CONTROL_H_ */
