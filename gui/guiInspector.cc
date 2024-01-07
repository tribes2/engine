//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "console/consoleTypes.h"
#include "console/console.h"
#include "dgl/dgl.h"
#include "gui/guiTypes.h"
#include "gui/guiTextCtrl.h"
#include "gui/guiTextEditCtrl.h"
#include "gui/guiInspector.h"
#include "gui/guiCheckBoxCtrl.h"
#include "gui/guiPopUpCtrl.h"
#include "platform/event.h"

// datablocks
#include "game/gameBase.h"
#include "game/explosion.h"
#include "game/particleEngine.h"
#include "game/projectile.h"
#include "sim/cannedChatDataBlock.h"
#include "game/Debris.h"
#include "game/commanderMapIcon.h"
#include "game/shockwave.h"
#include "game/splash.h"
#include "game/shieldImpact.h"
#include "game/projEnergy.h"
#include "game/projBomb.h"

#define NULL_STRING     "<NULL>"

GuiInspector::GuiInspector()
{
   mEditControlOffset = 5;
   mEntryHeight = 16;
   mTextExtent = 80;
   mEntrySpacing = 2;
   mMaxMenuExtent = 80;
}

void GuiInspector::onRemove()
{
   mTarget = 0;
   while(size())
      first()->deleteObject();
   Parent::onRemove();
}

//------------------------------------------------------------------------------
static S32 QSORT_CALLBACK stringCompare(const void *a,const void *b)
{
   StringTableEntry sa = *(StringTableEntry*)a;
   StringTableEntry sb = *(StringTableEntry*)b;
   return(dStricmp(sa, sb));
} 

void GuiInspector::inspect(SimObject * obj)
{
   mTarget = obj;

   while(size())
      first()->deleteObject();

   S32 curYOffset = mEntrySpacing;
      
   if(!bool(mTarget))
   {
      resize(mBounds.point, Point2I(mBounds.extent.x, curYOffset));
      return;
   }

   // add in the static fields
   AbstractClassRep::FieldList fieldList = mTarget->getFieldList();
   AbstractClassRep::FieldList::iterator itr;
   for(itr = fieldList.begin(); itr != fieldList.end(); itr++)
   {
      if(itr->type == AbstractClassRep::DepricatedFieldType)
         continue;

      char fdata[1024];
      const char * dstr = Con::getData(itr->type, (void *)(S32(obj) + itr->offset), 0, itr->table, itr->flag);
      if(!dstr)
         dstr = "";
      expandEscape(fdata, dstr);
      
      GuiTextCtrl * textCtrl = new GuiTextCtrl();
      textCtrl->setField("profile", "GuiTextProfile");
      textCtrl->setField("text", itr->pFieldname);
      textCtrl->registerObject();
      addObject(textCtrl);

   	S32 textWidth = textCtrl->mProfile->mFont->getStrWidth(itr->pFieldname);
      S32 xStartPoint = (textWidth < (mTextExtent + mEntrySpacing + mEditControlOffset)) ? 
         (mEntrySpacing + mTextExtent) : textWidth + mEditControlOffset;

      textCtrl->mBounds.point = Point2I(mEntrySpacing, curYOffset);
      textCtrl->mBounds.extent = Point2I(textWidth, mEntryHeight);

      S32 maxWidth = mBounds.extent.x - xStartPoint - mEntrySpacing;
             
      //now add the field
      GuiControl * editControl = NULL;
      
      switch(itr->type)
      {
         // text control
         default:
         {
            GuiTextEditCtrl * edit = new GuiTextEditCtrl();
            edit->setField("profile", "GuiInspectorTextEditProfile");
            edit->setField("text", fdata);
            editControl = edit;

            edit->mBounds.point = Point2I(xStartPoint, curYOffset);
            edit->mBounds.extent = Point2I(maxWidth, mEntryHeight);
            edit->setSizing(GuiControl::horizResizeWidth, GuiControl::vertResizeBottom);
            break;
         }

         // checkbox
         case TypeBool:
         case TypeFlag:
         {
            GuiCheckBoxCtrl * checkBox = new GuiCheckBoxCtrl();
            checkBox->setField("profile", "GuiCheckBoxProfile");
            checkBox->mBounds.point = Point2I(xStartPoint, curYOffset);
            checkBox->mBounds.extent = Point2I(mEntryHeight, mEntryHeight);
            checkBox->setScriptValue(fdata);

            editControl = checkBox;
            break;
         }

         // dropdown list
         case TypeEnum:
         {
            AssertFatal(itr->table, "TypeEnum declared with NULL table");
            GuiPopUpMenuCtrl * menu = new GuiPopUpMenuCtrl();
            menu->setField("profile", "GuiPopUpMenuProfile");
		      menu->setField("text", fdata);

            menu->mBounds.point = Point2I(xStartPoint, curYOffset);
            menu->mBounds.extent = Point2I(getMin(maxWidth, mMaxMenuExtent), mEntryHeight);
            
            //now add the entries
            for(S32 i = 0; i < itr->table->size; i++)
               menu->addEntry(itr->table->table[i].label, itr->table->table[i].index);

            editControl = menu;
            break;
         }

         // guiprofiles
         case TypeGuiProfile:
         {
            GuiPopUpMenuCtrl * menu = new GuiPopUpMenuCtrl();
            menu->setField("profile", "GuiPopUpMenuProfile");
		      menu->setField("text", *fdata ? fdata : NULL_STRING);
            menu->mBounds.point = Point2I(xStartPoint, curYOffset);
            menu->mBounds.extent = Point2I(getMin(maxWidth, mMaxMenuExtent), mEntryHeight);

            // add 'NULL'
            menu->addEntry(NULL_STRING, -1);
      
            // add entries to list so they can be sorted prior to adding to menu (want null on top)
            Vector<StringTableEntry> entries;
            
            SimGroup * grp = Sim::getGuiDataGroup();
            for(SimGroup::iterator i = grp->begin(); i != grp->end(); i++)
            {
               GuiControlProfile * profile = dynamic_cast<GuiControlProfile *>(*i);
               if(profile)
                  entries.push_back(profile->getName());
            }
            
            // sort the entries
            dQsort(entries.address(), entries.size(), sizeof(StringTableEntry), stringCompare);
            for(U32 j = 0; j < entries.size(); j++)
               menu->addEntry(entries[j], 0);

            editControl = menu;
            break;
         }

         // datablock types
         case TypeGameBaseDataPtr:
         case TypeExplosionDataPtr:
         case TypeShockwaveDataPtr:
         case TypeSplashDataPtr:
         case TypeEnergyProjectileDataPtr:
         case TypeBombProjectileDataPtr:
         case TypeParticleEmitterDataPtr:
         case TypeAudioDescriptionPtr:
         case TypeAudioProfilePtr:
         case TypeProjectileDataPtr:
         case TypeCannedChatItemPtr:
         case TypeDebrisDataPtr:
         case TypeCommanderIconDataPtr:
         {
            GuiPopUpMenuCtrl * menu = new GuiPopUpMenuCtrl();
            menu->setField("profile", "GuiPopUpMenuProfile");
		      menu->setField("text", *fdata ? fdata : NULL_STRING);
            menu->mBounds.point = Point2I(xStartPoint, curYOffset);
            menu->mBounds.extent = Point2I(getMin(maxWidth, mMaxMenuExtent), mEntryHeight);
            
            // add the 'NULL' entry on top
            menu->addEntry(NULL_STRING, -1);

            // add to a list so they can be sorted
            Vector<StringTableEntry> entries;

            SimGroup * grp = Sim::getDataBlockGroup();
            for(SimGroup::iterator i = grp->begin(); i != grp->end(); i++)
            {
               SimObject * obj = 0;
               switch(itr->type)
               {
                  case TypeGameBaseDataPtr:
                     obj = dynamic_cast<GameBaseData*>(*i); 
                     break;
                  case TypeExplosionDataPtr:
                     obj = dynamic_cast<ExplosionData*>(*i); 
                     break;
                  case TypeShockwaveDataPtr:
                     obj = dynamic_cast<ShockwaveData*>(*i); 
                     break;
                  case TypeSplashDataPtr:
                     obj = dynamic_cast<SplashData*>(*i); 
                     break;
                  case TypeEnergyProjectileDataPtr:
                     obj = dynamic_cast<EnergyProjectileData*>(*i); 
                     break;
                  case TypeBombProjectileDataPtr:
                     obj = dynamic_cast<BombProjectileData*>(*i); 
                     break;
                  case TypeParticleEmitterDataPtr:
                     obj = dynamic_cast<ParticleEmitterData*>(*i); 
                     break;
                  case TypeAudioDescriptionPtr:
                     obj = dynamic_cast<AudioDescription*>(*i); 
                     break;
                  case TypeAudioProfilePtr:
                     obj = dynamic_cast<AudioProfile*>(*i); 
                     break;
                  case TypeProjectileDataPtr:
                     obj = dynamic_cast<ProjectileData*>(*i);
                     break;
                  case TypeCannedChatItemPtr:
                     obj = dynamic_cast<CannedChatItem*>(*i);
                     break;
                  case TypeDebrisDataPtr:
                     obj = dynamic_cast<DebrisData*>(*i);
                     break;
                  case TypeCommanderIconDataPtr:
                     obj = dynamic_cast<CommanderIconData*>(*i);
                     break;
               }

               if(obj)
                  entries.push_back(obj->getName());
            }

            // sort the entries
            dQsort(entries.address(), entries.size(), sizeof(StringTableEntry), stringCompare);
            for(U32 j = 0; j < entries.size(); j++)
               menu->addEntry(entries[j], 0);

            editControl = menu;
            break;
         }
      }

		if(editControl)
      {
  	      char buf[256];
  	      dSprintf(buf, sizeof(buf), "InspectStatic%s", itr->pFieldname);
  	      editControl->registerObject(buf);
  	      addObject(editControl);
      }

      curYOffset += (mEntryHeight + mEntrySpacing);
   }

   // dynamic field seperator: text
   GuiTextCtrl * textCtrl = new GuiTextCtrl();
   textCtrl->setField("profile", "GuiTextProfile");
   textCtrl->setField("text", "  Dynamic Fields");
   textCtrl->registerObject();
   textCtrl->mBounds.point = Point2I(mEntrySpacing, curYOffset);
   textCtrl->mBounds.extent = Point2I(mTextExtent, mEntryHeight);
   addObject(textCtrl);

   // button
   GuiButtonCtrl * button = new GuiButtonCtrl();
   button->setField("profile", "GuiButtonProfile");
	button->setField("text", "Add");

   Con::setIntVariable("InspectingObject", mTarget->getId());
   Con::setIntVariable("Inspector", getId());

   S32 textWidth = textCtrl->mProfile->mFont->getStrWidth(textCtrl->getScriptValue());
   S32 xStartPoint = (textWidth < (mTextExtent + mEntrySpacing + mEditControlOffset)) ? 
      (mEntrySpacing + mTextExtent) : textWidth + mEditControlOffset;
   S32 maxWidth = mBounds.extent.x - xStartPoint - mEntrySpacing;

   button->mBounds.point = Point2I(xStartPoint, curYOffset);
   button->mBounds.extent = Point2I(getMin(maxWidth, mMaxMenuExtent), mEntryHeight);
  	button->registerObject();

   char buf[1024];
   dSprintf(buf, sizeof(buf), "%d.addDynamicField(%d);", getId(), mTarget->getId());
   button->setField("command", buf);
  	addObject(button);
   
   // offset   
   curYOffset += (mEntryHeight + mEntrySpacing);

   // add the dynamic fields
   SimFieldDictionary * fieldDictionary = mTarget->getFieldDictionary();
   for(SimFieldDictionaryIterator ditr(fieldDictionary); *ditr; ++ditr)
   {
      SimFieldDictionary::Entry * entry = (*ditr);

      // create the name ctrl
      GuiTextCtrl * nameCtrl = new GuiTextCtrl();

      nameCtrl->setField("profile", "GuiTextProfile");
      nameCtrl->setField("text", entry->slotName);
      nameCtrl->registerObject();
      addObject(nameCtrl);

      nameCtrl->mBounds.point = Point2I(mEntrySpacing, curYOffset);
      nameCtrl->mBounds.extent = Point2I(mTextExtent, mEntryHeight);

      // add a 'remove' button
      GuiButtonCtrl * button = new GuiButtonCtrl();

      button->setField("profile", "GuiButtonProfile");
      button->setField("text", "x");
      button->registerObject();
      addObject(button);

      char buf[1024];
      dSprintf(buf, sizeof(buf), "%d.%s = \"\";%d.inspect(%d);", mTarget->getId(), entry->slotName, getId(), mTarget->getId());
      button->setField("command", buf);
         
   	S32 textWidth = mProfile->mFont->getStrWidth(entry->slotName);

      // 10x10 with 2x2 frame   
      button->mBounds.point.set(textWidth + 4, curYOffset + 2);
      button->mBounds.extent.set(10, 10);

      textWidth += 14;
      S32 xStartPoint = (textWidth < (mTextExtent + mEntrySpacing + mEditControlOffset)) ? 
         (mEntrySpacing + mTextExtent) : textWidth + mEditControlOffset;
      S32 maxWidth = mBounds.extent.x - xStartPoint - mEntrySpacing;

      expandEscape(buf, entry->value ? entry->value : "");

      // create the edit ctrl
      GuiTextEditCtrl * editCtrl = new GuiTextEditCtrl();
      editCtrl->setField("profile", "GuiInspectorTextEditProfile");
      editCtrl->setField("text", buf);
      editCtrl->setSizing(GuiControl::horizResizeWidth, GuiControl::vertResizeBottom);

  	   dSprintf(buf, sizeof(buf), "InspectDynamic%s", entry->slotName);
  	   editCtrl->registerObject(buf);
      addObject(editCtrl);
      
      editCtrl->mBounds.point = Point2I(xStartPoint, curYOffset);
      editCtrl->mBounds.extent = Point2I(maxWidth, mEntryHeight);

      curYOffset += (mEntryHeight + mEntrySpacing);
   }

   resize(mBounds.point, Point2I(mBounds.extent.x, curYOffset));
}

void GuiInspector::apply(const char * newName)
{
   if(!bool(mTarget))
   {
      while(size())
         first()->deleteObject();
      return;
   }
   
   mTarget->assignName(newName);
   mTarget->inspectPreApply();
   
   SimObject * obj = static_cast<SimObject*>(mTarget);
      
   //now add in the fields
   AbstractClassRep::FieldList fieldList = mTarget->getFieldList();
   AbstractClassRep::FieldList::iterator itr;
   for(itr = fieldList.begin(); itr != fieldList.end(); itr++)
   {
      if(itr->type == AbstractClassRep::DepricatedFieldType)
         continue;

      char fdata[1024];
      dSprintf(fdata, sizeof(fdata), "InspectStatic%s", itr->pFieldname);
      GuiControl * editCtrl = NULL;
      SimObject * inspectObj = Sim::findObject(fdata);
      if(inspectObj)
         editCtrl = dynamic_cast<GuiControl*>(inspectObj);
      if(!editCtrl)
         continue;

      const char * newValue = 0;
      
      // check for null on profiles (-1 popup id)
      GuiPopUpMenuCtrl * menu = dynamic_cast<GuiPopUpMenuCtrl*>(editCtrl);
      if(!(menu && (menu->getSelected() == -1)))
         newValue = editCtrl->getScriptValue();
         
      if(!newValue)
         newValue = "";

      dStrcpy(fdata, newValue);
      collapseEscape(fdata);
      
      //now set the field
      const char *argv[1];
      argv[0] = &fdata[0];
      Con::setData(itr->type, (void *)(S32(obj) + itr->offset), 0, 1, argv, itr->table, itr->flag);
   }

   // get the dynamic field data
   SimFieldDictionary * fieldDictionary = mTarget->getFieldDictionary();
   for(SimFieldDictionaryIterator ditr(fieldDictionary); *ditr; ++ditr)
   {
      SimFieldDictionary::Entry * entry = (*ditr);

      char buf[1024];
      dSprintf(buf, sizeof(buf), "InspectDynamic%s", entry->slotName);
      
      GuiControl * editCtrl = static_cast<GuiControl*>(Sim::findObject(buf));
      if(!editCtrl)
         continue;

      const char * newValue = editCtrl->getScriptValue();
      dStrcpy(buf, newValue ? newValue : "");
      collapseEscape(buf);

      fieldDictionary->setFieldValue(entry->slotName, buf);
   }

   mTarget->inspectPostApply();
   
   //now re-inspect the object
   inspect(mTarget);
}

//------------------------------------------------------------------------------
static void cInspect(SimObject *obj, S32, const char **argv)
{
   GuiInspector * inspector = static_cast<GuiInspector*>(obj);
	SimObject * target = Sim::findObject(argv[2]);
   if(!target)
   {
      Con::printf("%s(): invalid object: %s", argv[0], argv[2]);
      return;
   }
   inspector->inspect(target);
}

static void cApply(SimObject *obj, S32, const char **argv)
{
   GuiInspector *inspector = static_cast<GuiInspector*>(obj);
   inspector->apply(argv[2]);
}

void GuiInspector::consoleInit()
{
   Con::addCommand("GuiInspector", "inspect", cInspect, "inspector.inspect(obj)", 3, 3);
   Con::addCommand("GuiInspector", "apply", cApply, "inspector.apply(newName)", 3, 3);
}

void GuiInspector::initPersistFields()
{
   Parent::initPersistFields();
   addField("editControlOffset", TypeS32, Offset(mEditControlOffset, GuiInspector));
   addField("entryHeight", TypeS32, Offset(mEntryHeight, GuiInspector));
   addField("textExtent", TypeS32, Offset(mTextExtent, GuiInspector));
   addField("entrySpacing", TypeS32, Offset(mEntrySpacing, GuiInspector));
   addField("maxMenuExtent", TypeS32, Offset(mMaxMenuExtent, GuiInspector));
}
