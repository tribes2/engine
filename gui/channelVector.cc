//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "GUI/channelVector.h"

struct TempLineBreak
{
   S32 start;
   S32 end;
	char *content;
};


IMPLEMENT_CONOBJECT(ChannelVector);

//-------------------------------------- Console methods
ConsoleMethod(ChannelVector,AddMember,bool,4,5,"obj.addMember(id,nick,flags)")
{
   ChannelVector* pCV = static_cast<ChannelVector*>(object);
   
   if (argc == 4)
      return pCV->addMember(dAtoi(argv[2]),argv[3]);
   else
      return pCV->addMember(dAtoi(argv[2]),argv[3],dAtoi(argv[4]));
}

ConsoleMethod(ChannelVector,RemoveMember,bool,3,3,"obj.removeMember(id)")
{
   argc;
   ChannelVector* pCV = static_cast<ChannelVector*>(object);
   
   return pCV->removeMember(dAtoi(argv[2]));
}

ConsoleMethod(ChannelVector,Sort,void,2,2,"obj.sort()")
{
   argc; argv;
   ChannelVector* pCV = static_cast<ChannelVector*>(object);
   
   pCV->sort();
}

ConsoleMethod(ChannelVector,FindMember,S32,3,3,"obj.findMember(id)")
{
   argc;
   ChannelVector* pCV = static_cast<ChannelVector*>(object);
   
   return pCV->findMember(dAtoi(argv[2]));
}

ConsoleMethod(ChannelVector,NumMembers,S32,2,2,"obj.numMembers()")
{
   argc; argv;
   ChannelVector* pCV = static_cast<ChannelVector*>(object);
   
   return pCV->numMembers();
}

ConsoleMethod(ChannelVector,GetMemberId,S32,3,3,"obj.getMemberId(i)")
{
   argc;
   ChannelVector* pCV = static_cast<ChannelVector*>(object);
   
   return pCV->getMemberId(dAtoi(argv[2]));
}

ConsoleMethod(ChannelVector,GetMemberNick,const char *,3,3,"obj.getMemberNick(i)")
{
   argc;
   ChannelVector* pCV = static_cast<ChannelVector*>(object);
   
   return pCV->getMemberNick(dAtoi(argv[2]));
}

ConsoleMethod(ChannelVector,SetMemberNick,bool,4,4,"obj.setMemberNick(i,nick)")
{
   argc;
   ChannelVector* pCV = static_cast<ChannelVector*>(object);
   
   return pCV->setMemberNick(dAtoi(argv[2]),argv[3]);
}

ConsoleMethod(ChannelVector,SetFlags,void,5,5,"obj.setFlags(i,flags,set)")
{
   argc;
   ChannelVector* pCV = static_cast<ChannelVector*>(object);
   
   pCV->setFlags(dAtoi(argv[2]),dAtoi(argv[3]),dAtob(argv[4]));
}

ConsoleMethod(ChannelVector,GetFlags,S32,3,3,"obj.getFlags(i)")
{
   argc;
   ChannelVector* pCV = static_cast<ChannelVector*>(object);
   
   return pCV->getFlags(dAtoi(argv[2]));
}


//--------------------------------------------------------------------------
ChannelVector::ChannelVector()
{
	VECTOR_SET_ASSOCIATION(mLineTags);
   VECTOR_SET_ASSOCIATION(mMembers);
}


//--------------------------------------------------------------------------
ChannelVector::~ChannelVector()
{
	for (U32 i = 0; i < mLineTags.size(); ++i)
		delete [] mLineTags[i].specials;
	mLineTags.clear();
}


//------------------------------------------------------------------------------
S32 QSORT_CALLBACK ChannelVector::compareMembers(const void *a, const void *b)
{
   Member *memberA = (Member*) a;
   Member *memberB = (Member*) b;

   S32 ao = memberA->flags & (PERSON_SPEAKER|PERSON_OPERATOR);
   S32 bo = memberB->flags & (PERSON_SPEAKER|PERSON_OPERATOR);

   if (ao != bo)
      return (bo-ao);

	char atag[64];
	const char *anick;
	char btag[64];
	const char *bnick;

	dStrcpy(atag,memberA->nick);
	atag[7] = '\0';
	if (dStrcmp(atag,"<tribe:") == 0)
		anick = &memberA->nick[9];
	else
		anick = memberA->nick;
	dStrcpy(btag,memberB->nick);
	btag[7] = '\0';
	if (dStrcmp(btag,"<tribe:") == 0)
		bnick = &memberB->nick[9];
	else
		bnick = memberB->nick;


   return (dStricmp(anick, bnick));
}


//------------------------------------------------------------------------------
void ChannelVector::sort()
{
   dQsort((void*) &mMembers[0], mMembers.size(), sizeof(Member), compareMembers);   
}


//--------------------------------------------------------------------------
void ChannelVector::initPersistFields()
{
   Parent::initPersistFields();
}


//--------------------------------------------------------------------------
void ChannelVector::consoleInit()
{
}


//--------------------------------------------------------------------------
bool ChannelVector::onAdd()
{
   return Parent::onAdd();
}


//--------------------------------------------------------------------------
void ChannelVector::onRemove()
{
   Parent::onRemove();
}


//--------------------------------------------------------------------------
void ChannelVector::insertLine(const U32 position, const char *newMessage, const S32 newMessageTag)
{
	Vector<TempLineBreak> tempSpecials(__FILE__, __LINE__);
   Vector<S32>           tempTypes(__FILE__, __LINE__);

	char *copy = new char[dStrlen(newMessage) + 1];
	const char* cur = copy;

	dStrcpy(copy,newMessage);
	while(cur[0] != '\0')
	{
		const char *open;
		
		if ((open = dStrstr(cur,"<tribe:")) != NULL)
		{
			const char *close;
			
			tempTypes.push_back((open[7] == '0') ? NickTag : TribeTag);
			tempSpecials.increment();

			TempLineBreak &tag = tempSpecials.last();

         tag.start = open - copy;
			tag.content = NULL;
			open += 9;
			close = dStrstr(open,"</tribe>");
         tag.end = tag.start + (close-open) - 1;

			dMemcpy(&copy[tag.start],open,tag.end-tag.start+1);
			dStrcpy(&copy[tag.end+1],close+8);

			cur = &copy[tempSpecials.last().end+1];
		}
		else
			if ((open = dStrstr(cur,"<t2server:")) != NULL)
			{
				const char *close;

				tempTypes.push_back(ServerTag);
				tempSpecials.increment();

				TempLineBreak &tag = tempSpecials.last();

				tag.start = open - copy;

				const char *content = open+10;
				
				open = dStrstr(open,">");
				tag.content = new char[open-content+1];
				dMemcpy(tag.content,content,open-content);
				tag.content[open-content] = '\0';
				++open;
				close = dStrstr(open,"</t2server>");
				tag.end = tag.start + (close-open) - 1;

				dMemcpy(&copy[tag.start],open,tag.end-tag.start+1);
				dStrcpy(&copy[tag.end+1],close+11);

				cur = &copy[tag.end+1];
			}
			else
				break;
	}

	SpecialMarkers tags;

	if ((tags.numSpecials = tempSpecials.size()) != 0) {
      tags.specials = new SpecialMarkers::Special[tempSpecials.size()];
      for (U32 i = 0; i < tempSpecials.size(); i++) {
         tags.specials[i].specialType = tempTypes[i];
         tags.specials[i].start       = tempSpecials[i].start;
         tags.specials[i].end         = tempSpecials[i].end;
			tags.specials[i].content	  = tempSpecials[i].content;
      }
   } else
   	tags.specials = NULL;
	mLineTags.insert(position);
	mLineTags[position] = tags;

	Parent::insertLine(position,copy,newMessageTag);

	delete [] copy;
}


//--------------------------------------------------------------------------
void ChannelVector::deleteLine(const U32 position)
{
	delete [] mLineTags[position].specials;
   mLineTags.erase(position);
	
	Parent::deleteLine(position);
}
