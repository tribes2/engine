//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUICHANNELVECTORCTRL_H_
#define _GUICHANNELVECTORCTRL_H_

#ifndef _CHANNELVECTOR_H_
#include "GUI/channelVector.h"
#endif

class GuiChannelVectorCtrl : public GuiMessageVectorCtrl
{
   typedef GuiMessageVectorCtrl Parent;

  protected:
	enum {
      	NickColor = 1,
			TribeColor = 2,
			ServerColor = 3,
	};

	virtual void onMouseUp(const GuiEvent &event);

   virtual void onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder);
   virtual void lineInserted(const U32);

  public:
   DECLARE_CONOBJECT(GuiChannelVectorCtrl);
};

#endif  // _H_GUICHANNELVECTORCTRL_
