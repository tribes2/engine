//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "editor/editor.h"
#include "console/console.h"
#include "console/consoleInternal.h"
#include "gui/guiTextListCtrl.h"
#include "platform/event.h"
#include "game/shapeBase.h"
#include "game/gameConnection.h"

bool gEditingMission = false;

//------------------------------------------------------------------------------
// Class EditManager
//------------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(EditManager);

EditManager::EditManager()
{
   for(U32 i = 0; i < 10; i++)
      mBookmarks[i] = MatrixF(true);
}

EditManager::~EditManager()
{
}

//------------------------------------------------------------------------------

bool EditManager::onWake()
{
   if(!Parent::onWake())
      return(false);
      
   for(SimSetIterator itr(Sim::getRootGroup());  *itr; ++itr)
      (*itr)->onEditorEnable();

   gEditingMission = true;
            
   return(true);
}

void EditManager::onSleep()
{
   for(SimSetIterator itr(Sim::getRootGroup());  *itr; ++itr)
      (*itr)->onEditorDisable();

   gEditingMission = false;
   Parent::onSleep();
}

//------------------------------------------------------------------------------

bool EditManager::onAdd()
{
   if(!Parent::onAdd())
      return(false);

   // hook the namespace
   const char * name = getName();
   if(name && name[0] && getClassRep())
   {
      Namespace * parent = getClassRep()->getNameSpace();
      Con::linkNamespaces(parent->mName, name);
      mNameSpace = Con::lookupNamespace(name);
   }

   return(true);
}

//------------------------------------------------------------------------------

static GameBase * getControlObj()
{
   GameConnection * connection = GameConnection::getLocalClientConnection();
   ShapeBase* control = 0;
   if(connection)
      control = connection->getControlObject();
   return(control);   
}

static void cSetBookmark(SimObject * obj, S32, const char ** argv)
{
   S32 val = dAtoi(argv[2]);
   if(val < 0 || val > 9)
      return;

   EditManager * editor = static_cast<EditManager*>(obj);
   GameBase * control = getControlObj();
   if(control)
      editor->mBookmarks[val] = control->getTransform();
}

static void cGotoBookmark(SimObject * obj, S32, const char ** argv)
{
   S32 val = dAtoi(argv[2]);
   if(val < 0 || val > 9)
      return;

   EditManager * editor = static_cast<EditManager*>(obj);
   GameBase * control = getControlObj();
   if(control)
      control->setTransform(editor->mBookmarks[val]);
}

void EditManager::consoleInit()
{
   Con::addCommand("EditManager", "setBookmark", cSetBookmark, "editor.setBookmark(<1-0>);", 3, 3);
   Con::addCommand("EditManager", "gotoBookmark", cGotoBookmark, "editor.gotoBookmark(<1-0>);", 3, 3);
}
