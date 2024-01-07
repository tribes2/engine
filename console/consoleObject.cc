//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "console/consoleObject.h"
#include "core/stringTable.h"
#include "console/console.h"

static AbstractClassRep::FieldList sg_tempFieldList;
AbstractClassRep *AbstractClassRep::classLinkList = NULL;
AbstractClassRep *AbstractClassRep::classTable[MaxClassId+1];
bool AbstractClassRep::initialized = false;

//--------------------------------------

const AbstractClassRep::Field *AbstractClassRep::findField(StringTableEntry name) const
{
   for(U32 i = 0; i < mFieldList.size(); i++)
      if(mFieldList[i].pFieldname == name)
         return &mFieldList[i];
   return NULL;
}

//--------------------------------------
void AbstractClassRep::registerClassRep(AbstractClassRep* in_pRep)
{
   AssertFatal(in_pRep != NULL, "Hmph, that's odd...");

#ifdef DEBUG  // assert if this class is already registered.
   for(AbstractClassRep *walk = classLinkList; walk; walk = walk->nextClass)
   {
      AssertFatal(dStrcmp(in_pRep->mClassName, walk->mClassName),
         "Duplicate Class name registered.");
   }
#endif
   in_pRep->nextClass = classLinkList;
   classLinkList = in_pRep;
}

//--------------------------------------

ConsoleObject* AbstractClassRep::create(const char* in_pClassName)
{
   AssertFatal(initialized, "creating an object before AbstractClassRep::initialize.");

   for (AbstractClassRep *walk = classLinkList; walk; walk = walk->nextClass)
      if (!dStrcmp(walk->getClassName(), in_pClassName))
         return walk->create();

   AssertWarn(0, avar("Couldn't find class rep for dynamic class: %s", in_pClassName));
   return NULL;
}

//--------------------------------------
ConsoleObject* AbstractClassRep::create(const S32 in_classId)
{
   AssertFatal(initialized, "creating an object before AbstractClassRep::initialize.");
   AssertFatal(in_classId >= 0 && in_classId <= MaxClassId, "Class id out of range.");
   AssertFatal(classTable[in_classId] != NULL, "No class with declared id type.");

   if(classTable[in_classId])
      return classTable[in_classId]->create();
   return NULL;
}

//--------------------------------------

static S32 QSORT_CALLBACK ACRCompare(const void *aptr, const void *bptr)
{
   const AbstractClassRep *a = *((const AbstractClassRep **) aptr);
   const AbstractClassRep *b = *((const AbstractClassRep **) bptr);

   if(a->mClassIdBase != b->mClassIdBase)
      return a->mClassIdBase - b->mClassIdBase;
   if(a->mClassVersion != b->mClassVersion)
      return a->mClassVersion - b->mClassVersion;
   return dStrcmp(a->getClassName(), b->getClassName());
}

void AbstractClassRep::initialize()
{
   AssertFatal(!initialized, "Duplicate call to AbstractClassRep::initialize()");
   U32 i;
   for(i = 0; i <= MaxClassId; i++)
      classTable[i] = NULL;

   Vector<AbstractClassRep *> dynamicTable(__FILE__, __LINE__);
      
   AbstractClassRep *walk;
   
   for (walk = classLinkList; walk; walk = walk->nextClass)
      walk->mNamespace = Con::lookupNamespace(StringTable->insert(walk->getClassName()));

   for (walk = classLinkList; walk; walk = walk->nextClass)
   {
      sg_tempFieldList.setSize(0);
      walk->init();
      if (sg_tempFieldList.size() != 0)
         walk->mFieldList = sg_tempFieldList;

      if(walk->mClassIdBase != -1)
         dynamicTable.push_back(walk);
   }
   dQsort((void *) &dynamicTable[0], dynamicTable.size(), sizeof(AbstractClassRep *), ACRCompare);

   S32 prevClassBase = -1;
   S32 curClassId = 0;
   
   for(i = 0; i < dynamicTable.size(); i++)
   {
      AbstractClassRep *cur = dynamicTable[i];
      if(cur->mClassIdBase != prevClassBase)
      {
         AssertFatal(cur->mClassIdBase >= curClassId, avar("Class range too small %d", prevClassBase));
         prevClassBase = cur->mClassIdBase;
         curClassId = cur->mClassIdBase;
      }
      cur->mClassId = curClassId++;
      classTable[cur->mClassId] = cur;
   }
   initialized = true;
   sg_tempFieldList.clear();
}

//------------------------------------------------------------------------------
//-------------------------------------- ConsoleObject
void ConsoleObject::addField(const char*  in_pFieldname,
                       const U32 in_fieldType,
                       const U32 in_fieldOffset,
                       const U32 in_elementCount,
                       EnumTable *in_table)
{
   AbstractClassRep::Field f;
   f.pFieldname   = StringTable->insert(in_pFieldname);
   f.type         = in_fieldType;
   f.offset       = in_fieldOffset;
   f.elementCount = in_elementCount;
   f.table        = in_table;

   sg_tempFieldList.push_back(f);
}

void ConsoleObject::addDepricatedField(const char *fieldName)
{
   AbstractClassRep::Field f;
   f.pFieldname   = StringTable->insert(fieldName);
   f.type         = AbstractClassRep::DepricatedFieldType;

   sg_tempFieldList.push_back(f);
}


bool ConsoleObject::removeField(const char* in_pFieldname)
{
   for (U32 i = 0; i < sg_tempFieldList.size(); i++) {
      if (dStricmp(in_pFieldname, sg_tempFieldList[i].pFieldname) == 0) {
         sg_tempFieldList.erase(i);
         return true;
      }
   }

   return false;
}



//--------------------------------------
void ConsoleObject::initPersistFields()
{
}   

//--------------------------------------
void ConsoleObject::consoleInit()
{
}   

ConsoleObject::~ConsoleObject()
{
}

//--------------------------------------
AbstractClassRep* ConsoleObject::getClassRep() const
{
   return NULL;
}


