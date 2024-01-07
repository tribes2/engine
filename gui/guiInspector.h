//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUIINSPECTOR_H_
#define _GUIINSPECTOR_H_

#ifndef _GUICONTROL_H_
#include "GUI/guiControl.h"
#endif
#ifndef _GUIARRAYCTRL_H_
#include "GUI/guiArrayCtrl.h"
#endif

class GuiInspector : public GuiControl
{
	private:
		typedef GuiControl Parent;
		SimObjectPtr<SimObject>    mTarget;

	public:
		DECLARE_CONOBJECT(GuiInspector);

      GuiInspector();

      // field data
      S32                  mEditControlOffset;
      S32                  mEntryHeight;
      S32                  mTextExtent;
      S32                  mEntrySpacing;
      S32                  mMaxMenuExtent;

		static void consoleInit();
      static void initPersistFields();
      void onRemove();

		void inspect(SimObject *);
		void apply(const char *);
};

#endif
