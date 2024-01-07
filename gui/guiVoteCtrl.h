//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUIVOTECTRL_H_
#define _GUIVOTECTRL_H_

#ifndef _GUICONTROL_H_
#include "GUI/guiControl.h"
#endif

class GuiVoteCtrl : public GuiControl
{
private:
	typedef GuiControl Parent;

	F32 mNoProgress;
   F32 mYesProgress;
   F32 mQuorum;
   F32 mPassHash;

public:
	//creation methods
	DECLARE_CONOBJECT(GuiVoteCtrl);
	GuiVoteCtrl();

	//console related methods
	static void consoleInit();
   virtual const char *getScriptValue();
   virtual void setScriptValue(const char *value);

   void onPreRender();
	void onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder);
   
   public:
      void setPassValue(F32 value);
      void setQuorumValue(F32 value);
      void setYesValue(F32 value);
      void setNoValue(F32 value);
      
      F32 getPassValue();
      F32 getQuorumValue();
      F32 getYesValue();
      F32 getNoValue();
   
};

inline F32 GuiVoteCtrl::getQuorumValue()
{
   return mQuorum;
}

inline F32 GuiVoteCtrl::getPassValue()
{
   return mPassHash;
}

inline F32 GuiVoteCtrl::getYesValue()
{
   return mYesProgress;
}

inline F32 GuiVoteCtrl::getNoValue()
{
   return mNoProgress;
}


#endif
