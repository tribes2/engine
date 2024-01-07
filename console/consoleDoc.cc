
#include "platform/platform.h"
#include "console/console.h"

#include "console/ast.h"
#include "core/tAlgorithm.h"
#include "core/resManager.h"

#include "core/findMatch.h"
#include "console/consoleInternal.h"
#include "console/consoleObject.h"
#include "core/fileStream.h"
#include "console/compiler.h"

ConsoleFunction(dumpConsoleClasses, void, 1, 1, "()dumps all declared console classes to the console in C++ syntax for documenting with dOxygen or another auto documentation tool")
{
   Namespace::dumpClasses();
}

const char *typeNames[] = {
   "Script",
   "string",
   "int",
   "float",
   "void",
   "bool",
};

void Namespace::dumpClasses()
{
   Vector<Namespace *> vec;
   // the program
   trashCache();

   for(Namespace *walk = mNamespaceList; walk; walk = walk->mNext)
      walk->mHashSequence = 0;

   for(Namespace *walk = mNamespaceList; walk; walk = walk->mNext)
   {
      Vector<Namespace *> stack;
      Namespace *parentWalk = walk;
      while(parentWalk)
      {
         if(parentWalk->mHashSequence != 0)
            break;
         if(parentWalk->mPackage == 0)
         {
            parentWalk->mHashSequence = 1;
            stack.push_back(parentWalk);
         }
         parentWalk = parentWalk->mParent;
      }
      while(stack.size())
      {
         vec.push_back(stack[stack.size() - 1]);
         stack.pop_back();
      }
   }
   U32 i;
   for(i = 0; i < vec.size(); i++)
   {
      const char *className = vec[i]->mName;
      const char *superClassName = vec[i]->mParent ? vec[i]->mParent->mName : NULL;
      const char *virt = "virtual ";
		if(vec[i]->mEntryList == NULL && vec[i]->mClassRep == NULL)
			continue;

      if(!className)
      {
         Con::printf("namespace Global {");
         virt = "";
      }
      else if(!superClassName)
         Con::printf("class %s { public:", className);
      else
         Con::printf("class %s : public %s { public:", className, superClassName);

      
      for(Entry *ewalk = vec[i]->mEntryList; ewalk; ewalk = ewalk->mNext)
      {
         char buffer[1024];
         if(ewalk->mType > Entry::ScriptFunctionType)
         {
            if(ewalk->mUsage[0] != '(')
               Con::printf("  %s %s %s() {} // %s", virt, typeNames[ewalk->mType], ewalk->mFunctionName, ewalk->mUsage);
            else
            {
               const char *use = ewalk->mUsage;
               const char *end = dStrchr(use, ')');
               if(!end)
                  end = use + 1;
               use++;
               U32 len = end - use;
               dStrncpy(buffer, use, len);
               buffer[len] = 0;
               Con::printf("  %s %s %s(%s) {} // %s", virt, typeNames[ewalk->mType], ewalk->mFunctionName, buffer, end + 1);
            }
			}
			else if(ewalk->mFunctionOffset)
         {
            ewalk->mCode->getFunctionArgs(buffer, ewalk->mFunctionOffset);
            Con::printf("  %s void %s(%s) {} // declared script function", virt, ewalk->mFunctionName, buffer);
         }
      }
      AbstractClassRep *rep = vec[i]->mClassRep;
      AbstractClassRep::FieldList emptyList;
      AbstractClassRep::FieldList *parentList = &emptyList;
      AbstractClassRep::FieldList *fieldList = &emptyList;

      if(rep)
      {
         AbstractClassRep *parentRep = vec[i]->mParent ? vec[i]->mParent->mClassRep : NULL;
         if(parentRep)
            parentList = &(parentRep->mFieldList);
         fieldList = &(rep->mFieldList);
         for(U32 j = 0; j < fieldList->size(); j++)
         {
				if((*fieldList)[j].type == AbstractClassRep::DepricatedFieldType)
					continue;

            bool found = false;
            for(U32 k = 0; k < parentList->size(); k++)
            {
               if((*parentList)[k].pFieldname == (*fieldList)[j].pFieldname)
               {
                  found = true;
                  break;
               }
            }
            if(!found)
               Con::printf("  %s %s;", Con::getTypeName((*fieldList)[j].type), (*fieldList)[j].pFieldname);
         }
      }
      Con::printf("};");
   }
}