//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUITYPES_H_
#define _GUITYPES_H_

#ifndef _GFONT_H_
#include "dgl/gFont.h"
#endif
#ifndef _COLOR_H_
#include "Core/color.h"
#endif
#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif
#ifndef _GTEXMANAGER_H_
#include "dgl/gTexManager.h"
#endif
#ifndef _PLATFORMAUDIO_H_
#include "Platform/platformAudio.h"
#endif
#ifndef _AUDIODATABLOCK_H_
#include "audio/audioDataBlock.h"
#endif

class GBitmap;

struct GuiEvent
{
   U16		ascii;       		// ascii character code 'a', 'A', 'b', '*', etc (if device==keyboard) - possibly a uchar or something
   U8			modifier;     		// SI_LSHIFT, etc
	U8			keyCode;				// for unprintables, 'tab', 'return', ...
	Point2I	mousePoint;			// for mouse events
	U8			mouseClickCount;	// to determine double clicks, etc...
};   

class GuiCursor : public SimObject
{
private:
	typedef SimObject Parent;
   StringTableEntry mBitmapName;
   
   Point2I mHotSpot;
   Point2I mExtent;
   TextureHandle mTextureHandle;
   
public:
   TextureHandle getTextureHandle() { return mTextureHandle; }
   Point2I getHotSpot() { return mHotSpot; }
   Point2I getExtent() { return mExtent; }
      
	DECLARE_CONOBJECT(GuiCursor);
	GuiCursor(void);
	~GuiCursor(void);
	static void initPersistFields();

	bool onAdd(void);
   void onRemove();
};


class GuiControlProfile : public SimObject
{
private:
	typedef SimObject Parent;

public:
   S32  mRefCount;
	bool mTabable;
   bool mCanKeyFocus;
   bool mModal;

	bool mOpaque;
	ColorI mFillColor;
	ColorI mFillColorHL;
	ColorI mFillColorNA;

   bool mBorder;
	ColorI mBorderColor;
	ColorI mBorderColorHL;
	ColorI mBorderColorNA;

	// font members
	StringTableEntry	mFontType;
	S32					mFontSize;
   enum {
      BaseColor = 0,
      ColorHL,
      ColorNA,
      ColorSEL,
      ColorUser0,
      ColorUser1,
      ColorUser2,
      ColorUser3,
      ColorUser4,
      ColorUser5,
   };
   ColorI            mFontColors[10];
	ColorI &				mFontColor;
	ColorI &				mFontColorHL;
	ColorI &				mFontColorNA;
   ColorI &          mFontColorSEL;

	Resource<GFont>   mFont;

	enum AlignmentType
	{
	   LeftJustify,
	   RightJustify,
	   CenterJustify
	};

	AlignmentType mAlignment;
	bool mAutoSizeWidth;
	bool mAutoSizeHeight;
	bool mReturnTab;
	bool mNumbersOnly;
	ColorI mCursorColor;

   StringTableEntry mBitmapBase;
   Point2I mTextOffset;
   
	// bitmap members
   StringTableEntry mBitmapName;
	TextureHandle mTextureHandle;

   // sound members
   AudioProfile *mSoundButtonDown;
   AudioProfile *mSoundButtonOver;

public:
	DECLARE_CONOBJECT(GuiControlProfile);
	GuiControlProfile();
	~GuiControlProfile();
	static void initPersistFields();
   bool onAdd();

	void incRefCount();
   void decRefCount();
};

void RegisterGuiTypes(void);

#endif //_GUITYPES_H
