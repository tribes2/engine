//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "console/consoleTypes.h"
#include "console/console.h"
#include "dgl/gFont.h"
#include "dgl/dgl.h"
#include "GUI/guiTypes.h"
#include "dgl/gBitmap.h"
#include "dgl/gTexManager.h"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

GuiCursor::GuiCursor()
{
   mHotSpot.set(0,0);
}

GuiCursor::~GuiCursor()
{
}

void GuiCursor::initPersistFields()
{
   Parent::initPersistFields();
   addField("hotSpot", TypePoint2I, Offset(mHotSpot, GuiCursor));
   addField("bitmapName", TypeString, Offset(mBitmapName, GuiCursor));
}

bool GuiCursor::onAdd()
{
   if(!Parent::onAdd())
      return false;

   mTextureHandle = TextureHandle(mBitmapName);
   if(!mTextureHandle)
      return false;
      
   mExtent.set(mTextureHandle.getWidth(), mTextureHandle.getHeight());
   Sim::getGuiDataGroup()->addObject(this);
   
   return true;
}

void GuiCursor::onRemove()
{
   Parent::onRemove();
}

static EnumTable::Enums alignEnums[] = 
{
   { GuiControlProfile::LeftJustify,          "left"      },
   { GuiControlProfile::CenterJustify,        "center"    },
   { GuiControlProfile::RightJustify,         "right"     }
};
static EnumTable gAlignTable(3, &alignEnums[0]); 

GuiControlProfile::GuiControlProfile(void) :
   mFontColor(mFontColors[BaseColor]),
   mFontColorHL(mFontColors[ColorHL]),
   mFontColorNA(mFontColors[ColorNA]),
   mFontColorSEL(mFontColors[ColorSEL])
{
   mRefCount = 0;

   GuiControlProfile *def = dynamic_cast<GuiControlProfile*>(Sim::findObject("GuiDefaultProfile"));
   if (def)
   {
      mTabable       = def->mTabable;
      mCanKeyFocus   = def->mCanKeyFocus;

      mOpaque        = def->mOpaque;
      mFillColor     = def->mFillColor;
      mFillColorHL   = def->mFillColorHL;
      mFillColorNA   = def->mFillColorNA;
   
      mBorder        = def->mBorder;
      mBorderColor   = def->mBorderColor;
      mBorderColorHL = def->mBorderColorHL;
      mBorderColorNA = def->mBorderColorNA;
   
      // default font
      mFontType      = def->mFontType;
      mFontSize      = def->mFontSize;

      for(U32 i = 0; i < 10; i++)
         mFontColors[i] = def->mFontColors[i];

      // default bitmap
      mBitmapName    = def->mBitmapName;
      mBitmapBase    = def->mBitmapBase;
      mTextOffset    = def->mTextOffset;

      // default sound
      mSoundButtonDown = def->mSoundButtonDown;
      mSoundButtonOver = def->mSoundButtonOver;

      //used by GuiTextCtrl
      mModal         = def->mModal;
      mAlignment     = def->mAlignment;
      mAutoSizeWidth = def->mAutoSizeWidth;
      mAutoSizeHeight= def->mAutoSizeHeight;
      mReturnTab     = def->mReturnTab;
      mNumbersOnly   = def->mNumbersOnly;
      mCursorColor   = def->mCursorColor;
   }
}

GuiControlProfile::~GuiControlProfile()
{
}


void GuiControlProfile::initPersistFields()
{
   Parent::initPersistFields();
   
   addField("tab",           TypeBool,       Offset(mTabable, GuiControlProfile));
   addField("canKeyFocus",   TypeBool,       Offset(mCanKeyFocus, GuiControlProfile));
   
   addField("modal",         TypeBool,       Offset(mModal, GuiControlProfile));
   addField("opaque",        TypeBool,       Offset(mOpaque, GuiControlProfile));
   addField("fillColor",     TypeColorI,     Offset(mFillColor, GuiControlProfile));
   addField("fillColorHL",   TypeColorI,     Offset(mFillColorHL, GuiControlProfile));
   addField("fillColorNA",   TypeColorI,     Offset(mFillColorNA, GuiControlProfile));
   addField("border",        TypeBool,       Offset(mBorder, GuiControlProfile));
   addField("borderColor",   TypeColorI,     Offset(mBorderColor, GuiControlProfile));
   addField("borderColorHL", TypeColorI,     Offset(mBorderColorHL, GuiControlProfile));
   addField("borderColorNA", TypeColorI,     Offset(mBorderColorNA, GuiControlProfile));
   
   addField("fontType",      TypeString,     Offset(mFontType, GuiControlProfile));
   addField("fontSize",      TypeS32,        Offset(mFontSize, GuiControlProfile));
   addField("fontColors",    TypeColorI,     Offset(mFontColors, GuiControlProfile), 10);
   addField("fontColor",     TypeColorI,     Offset(mFontColors[BaseColor], GuiControlProfile));
   addField("fontColorHL",   TypeColorI,     Offset(mFontColors[ColorHL], GuiControlProfile));
   addField("fontColorNA",   TypeColorI,     Offset(mFontColors[ColorNA], GuiControlProfile));
   addField("fontColorSEL",  TypeColorI,     Offset(mFontColors[ColorSEL], GuiControlProfile));
   
   addField("justify",       TypeEnum,       Offset(mAlignment, GuiControlProfile), 1, &gAlignTable);
   addField("textOffset",    TypePoint2I,    Offset(mTextOffset, GuiControlProfile));
   addField("bitmapBase",    TypeString,     Offset(mBitmapBase, GuiControlProfile));
   addField("autoSizeWidth", TypeBool,       Offset(mAutoSizeWidth, GuiControlProfile));
   addField("autoSizeHeight",TypeBool,       Offset(mAutoSizeHeight, GuiControlProfile));
   addField("returnTab",     TypeBool,       Offset(mReturnTab, GuiControlProfile));
   addField("numbersOnly",   TypeBool,       Offset(mNumbersOnly, GuiControlProfile));
   addField("cursorColor",   TypeColorI,     Offset(mCursorColor, GuiControlProfile));
   
   addField("bitmap",        TypeString,     Offset(mBitmapName, GuiControlProfile));

   addField("soundButtonDown", TypeAudioProfilePtr,  Offset(mSoundButtonDown, GuiControlProfile));
   addField("soundButtonOver", TypeAudioProfilePtr,  Offset(mSoundButtonOver, GuiControlProfile));
}

bool GuiControlProfile::onAdd()
{
   if(!Parent::onAdd())
      return false;

   Sim::getGuiDataGroup()->addObject(this);
      
   return true;
}

void GuiControlProfile::incRefCount()
{
   if(!mRefCount++)
   {
      //verify the font
      mFont = GFont::create(mFontType, mFontSize);
      AssertFatal(bool(mFont), "Failed to create the font.");
      
      //verify the bitmap
      mTextureHandle = TextureHandle(mBitmapName, BitmapKeepTexture);
      AssertFatal(bool(mTextureHandle), "Profile has no bitmap");
   }

   AssertFatal(bool(mFont), "GuiControlProfile::incRefCount: invalid font");
   AssertFatal(bool(mTextureHandle), "GuiControlProfile::incRefCount: invalid bitmap");
}

void GuiControlProfile::decRefCount()
{
   AssertFatal(mRefCount, "GuiControlProfile::decRefCount: zero ref count");
   if(!mRefCount)
      return;

   if(!--mRefCount)
   {
      mFont = NULL;
      mTextureHandle = NULL;
   }
}

static void setDataTypeGuiProfile(void *dptr, S32 argc, const char **argv, EnumTable *, BitSet32)
{
   GuiControlProfile *profile = NULL;
   if(argc == 1)
      Sim::findObject(argv[0], profile);
   
   AssertWarn(profile != NULL, avar("GuiControlProfile: requested gui profile (%s) does not exist.", argv[0]));
   if(!profile)
      profile = dynamic_cast<GuiControlProfile*>(Sim::findObject("GuiDefaultProfile"));

   AssertFatal(profile != NULL, avar("GuiControlProfile: unable to find specified profile (%s) and GuiDefaultProfile does not exist!", argv[0]));
   
   GuiControlProfile **obj = (GuiControlProfile **)dptr;
   if((*obj) == profile)
      return;
      
   if(*obj)
      (*obj)->decRefCount();

   *obj = profile;
   (*obj)->incRefCount();
}

static const char *getDataTypeGuiProfile(void *dptr, EnumTable *, BitSet32)
{
   static char returnBuffer[256];
   
   GuiControlProfile **obj = (GuiControlProfile**)dptr;
   dSprintf(returnBuffer, sizeof(returnBuffer), "%s", *obj ? (*obj)->getName() : "");
   return returnBuffer;
}

void RegisterGuiTypes(void)
{
   Con::registerType(TypeGuiProfile, sizeof(GuiControlProfile *), getDataTypeGuiProfile,  setDataTypeGuiProfile);
}
