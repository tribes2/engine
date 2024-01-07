//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "console/console.h"
#include "console/consoleTypes.h"
#include "console/consoleInternal.h"
#include "Core/bitStream.h"
#include "ai/aiObjective.h"

IMPLEMENT_CO_NETOBJECT_V1(AIObjective);

Sphere AIObjective::smSphere(Sphere::Octahedron);

//---------------------------------------------------------------------------//
//AIObjective Methods

AIObjective::AIObjective()
{
   mTypeMask = StaticObjectType;
   mDescription = StringTable->insert("");
   mMode = StringTable->insert("");
   mTargetClient = StringTable->insert("");
   mTargetObject = StringTable->insert("");
   mTargetClientId = -1;
   mTargetObjectId = -1;
   mLocation.set(0, 0, 0);
   
	mWeightLevel1 = 0;
	mWeightLevel2 = 0;
	mWeightLevel3 = 0;
	mWeightLevel4 = 0;
	mOffense = false;
	mDefense = false;
   
   mRequiredEquipment = StringTable->insert("");
   mDesiredEquipment = StringTable->insert("");
   mBuyEquipmentSets = StringTable->insert("");
   mCannedChat = StringTable->insert("");

	mIssuedByHuman = false;
	mIssuedByClientId = -1;
	mForceClientId = -1;
   
   mLocked = false;
   
   //this will allow us to access the persist fields from script
   setModStaticFields(true);
}

//----------------------------------------------------------------------------

bool AIObjective::onAdd()
{
   if(!Parent::onAdd())
      return(false);

   //set the task namespace
   const char *name = getName();
   if(name && name[0] && getClassRep())
   {
      Namespace *parent = getClassRep()->getNameSpace();
      Con::linkNamespaces(parent->mName, name);
      mNameSpace = Con::lookupNamespace(name);
   }

   if(!isClientObject())
      setMaskBits(UpdateSphereMask);
   return(true);
}

//----------------------------------------------------------------------------

void AIObjective::inspectPostApply()
{
   Parent::inspectPostApply();
   setMaskBits(UpdateSphereMask);
}

void AIObjective::consoleInit()
{
   Parent::consoleInit();
}

//----------------------------------------------------------------------------

void AIObjective::initPersistFields()
{
   Parent::initPersistFields();
   addField("description",       TypeString,    Offset(mDescription,       AIObjective));
   addField("mode",              TypeString,    Offset(mMode,              AIObjective));
   addField("targetClient",      TypeString,    Offset(mTargetClient,      AIObjective));
   addField("targetObject",      TypeString,    Offset(mTargetObject,      AIObjective));
   addField("targetClientId",    TypeS32,       Offset(mTargetClientId,    AIObjective));
   addField("targetObjectId",    TypeS32,       Offset(mTargetObjectId,    AIObjective));
   addField("location",          TypePoint3F,   Offset(mLocation,          AIObjective));
   
   addField("weightLevel1",      TypeS32,       Offset(mWeightLevel1,      AIObjective));
   addField("weightLevel2",      TypeS32,       Offset(mWeightLevel2,      AIObjective));
   addField("weightLevel3",      TypeS32,       Offset(mWeightLevel3,      AIObjective));
   addField("weightLevel4",      TypeS32,       Offset(mWeightLevel4,      AIObjective));
   addField("offense",           TypeBool,      Offset(mOffense,           AIObjective));
   addField("defense",           TypeBool,      Offset(mDefense,           AIObjective));
   
   addField("equipment",         TypeString,    Offset(mRequiredEquipment, AIObjective));
   addField("desiredEquipment",  TypeString,    Offset(mDesiredEquipment,  AIObjective));
   addField("buyEquipmentSet",  	TypeString,    Offset(mBuyEquipmentSets,  AIObjective));

   addField("chat",  				TypeString,    Offset(mCannedChat,  		AIObjective));

   addField("issuedByHuman",     TypeBool,      Offset(mIssuedByHuman,     AIObjective));
   addField("issuedByClientId",   TypeS32,      Offset(mIssuedByClientId,  AIObjective));
   addField("forceClientId",   	TypeS32,      	Offset(mForceClientId,   	AIObjective));
   
   addField("locked",  				TypeBool,      Offset(mLocked,  		      AIObjective));
   
}

//---------------------------------------------------------------------------//

U32 AIObjective::packUpdate(NetConnection * con, U32 mask, BitStream * stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   // 
   if(stream->writeFlag(mask & UpdateSphereMask))
   {
   }
   return(retMask);
}

void AIObjective::unpackUpdate(NetConnection * con, BitStream * stream)
{
   Parent::unpackUpdate(con, stream);
   if(stream->readFlag())
   {
   }
}

//---------------------------------------------------------------------------//
//AIObjectiveQ Methods

IMPLEMENT_CONOBJECT(AIObjectiveQ);

static void cAIQSortByWeight(SimObject *obj, S32, const char **)
{
   AIObjectiveQ *objectiveQ = static_cast<AIObjectiveQ*>(obj);
   objectiveQ->sortByWeight();  
}

void AIObjectiveQ::consoleInit()
{
   Parent::consoleInit();

   Con::addCommand("AIObjectiveQ", "sortByWeight", cAIQSortByWeight, "aiQ.sortByWeight()", 2, 2);
}

void AIObjectiveQ::addObject(SimObject *obj)
{
	//only aioObjectives can be part of an objective Queue
	AIObjective *objective = dynamic_cast<AIObjective*>(obj);
	if (! objective)
	{
	   Con::printf("Error AIObjective::addObject() - attempting to add something other than an AIObjective!");
		//print the error, but don't return - in case I need to revert the ai scripts...
	   //return;
	}
	Parent::addObject(obj);
}

S32 QSORT_CALLBACK AIObjectiveQ::compareWeight(const void* a,const void* b)
{
   return (*reinterpret_cast<const AIObjective* const*>(b))->getSortWeight() -
      (*reinterpret_cast<const AIObjective* const*>(a))->getSortWeight();
}

void AIObjectiveQ::sortByWeight()
{
 	dQsort(objectList.address(), objectList.size(), sizeof(SimObject *), compareWeight);
}
