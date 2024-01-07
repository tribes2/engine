//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "Platform/platform.h"
#include "console/simBase.h"
#include "console/consoleTypes.h"

class ScriptObject : public SimObject
{
   typedef SimObject Parent;
   StringTableEntry mClassName;
   StringTableEntry mSuperClassName;
public:
   ScriptObject();
   bool onAdd();
   void onRemove();
   
   DECLARE_CONOBJECT(ScriptObject);
   
   static void initPersistFields();
};

IMPLEMENT_CONOBJECT(ScriptObject);

void ScriptObject::initPersistFields()
{
   addField("class", TypeString, Offset(mClassName, ScriptObject));
   addField("superClass", TypeString, Offset(mSuperClassName, ScriptObject));
}

ScriptObject::ScriptObject()
{
   mClassName = "";
   mSuperClassName = "";
}

bool ScriptObject::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   if(mClassName[0])
   {
      if(mSuperClassName[0])
      {
         Con::linkNamespaces(mSuperClassName, mClassName);
         Con::linkNamespaces("ScriptObject", mSuperClassName);
      }
      else
         Con::linkNamespaces("ScriptObject", mClassName);
         
      mNameSpace = Con::lookupNamespace(mClassName);
   }
   return true;
}

void ScriptObject::onRemove()
{
   Parent::onRemove();
}
