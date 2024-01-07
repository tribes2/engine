//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _CHANNELVECTOR_H_
#define _CHANNELVECTOR_H_

#ifndef _GUIMESSAGEVECTORCTRL_H_
#include "GUI/guiMessageVectorCtrl.h"
#endif

class ChannelVector : public MessageVector
{
  private:
   typedef MessageVector Parent;

   enum Constants {
      // Person flags
      PERSON_SPEAKER       = BIT(0),
      PERSON_OPERATOR      = BIT(1),
      PERSON_IGNORE        = BIT(2),
      PERSON_AWAY          = BIT(3),
   };

  public:
	enum {
		NickTag	= -3,
		TribeTag	= -2,
		ServerTag = -1,
	};

	struct SpecialMarkers {
      struct Special {
         S32 specialType;
         S32 start;
         S32 end;
			char *content;
      
      	Special() { content = NULL; };
			~Special() { delete [] content; };
      };

      U32      numSpecials;
      Special* specials;
   };

   ChannelVector();
   ~ChannelVector();

   DECLARE_CONOBJECT(ChannelVector);

	virtual void insertLine(const U32 position, const char*, const S32);
   virtual void deleteLine(const U32);

	const SpecialMarkers & getLineTags(U32 position)
	{
		return mLineTags[position];
	}

   bool addMember(S32 id, const char *nick, S32 flags = 0);
   bool removeMember(S32 id);
   static S32 QSORT_CALLBACK compareMembers(const void *a, const void *b);
   void sort();
   S32 findMember(S32 id);
   S32 numMembers();
   S32 getMemberId(S32 i);
   const char *getMemberNick(S32 i);
	bool setMemberNick(S32 i, const char *nick);
   void setFlags(S32 i, S32 flags, bool set);
   S32 getFlags(S32 i);

   static void initPersistFields();
   static void consoleInit();

  protected:
   struct Member
   {
   	S32 id;
		StringTableEntry nick;
		S32 flags;
   };

   bool onAdd();
   void onRemove();

	Vector<SpecialMarkers> mLineTags;
   Vector<Member> mMembers;
};


//------------------------------------------------------------------------------
inline bool ChannelVector::addMember(S32 id, const char *nick, S32 flags)
{
   Vector<Member>::iterator itr = mMembers.begin();

   for (; itr != mMembers.end(); itr++)
      if (id == itr->id)
         break;

   if (itr == mMembers.end())
   {
      mMembers.increment();

      Member *pm = mMembers.end()-1;

      pm->id = id;
      pm->nick = StringTable->insert(nick);
      pm->flags = flags;
      sort();

      return true;
   }

   return false;
}

//------------------------------------------------------------------------------
inline bool ChannelVector::removeMember(S32 id)
{
   Vector<Member>::iterator itr = mMembers.begin();

   for (; itr != mMembers.end(); itr++)
      if (id == itr->id)
      {
         mMembers.erase(itr);

         return true;
      }

   return false;
}

//------------------------------------------------------------------------------
inline S32 ChannelVector::findMember(S32 id)
{
   for (S32 i = 0; i < mMembers.size(); ++i)
      if (id == mMembers[i].id)
	     return i;

   return -1;
}

//------------------------------------------------------------------------------
inline S32 ChannelVector::numMembers()
{
   return mMembers.size();
}

//------------------------------------------------------------------------------
inline S32 ChannelVector::getMemberId(S32 i)
{
   if (i >= 0 && i < mMembers.size())
      return mMembers[i].id;
   else
      return 0;
}

//------------------------------------------------------------------------------
inline const char * ChannelVector::getMemberNick(S32 i)
{
   if (i >= 0 && i < mMembers.size())
      return mMembers[i].nick;
   else
	  return NULL;
}

//------------------------------------------------------------------------------
inline bool ChannelVector::setMemberNick(S32 i, const char *nick)
{
   if (i >= 0 && i < mMembers.size())
   {
      mMembers[i].nick = StringTable->insert(nick);
      
      return true;
   }
   else
	  return false;
}

//------------------------------------------------------------------------------
inline void ChannelVector::setFlags(S32 i, S32 flags, bool set)
{
   if (i >= 0 && i < mMembers.size())
      if (set)
	     mMembers[i].flags |= flags;
	  else
		 mMembers[i].flags &= ~flags;
}

//------------------------------------------------------------------------------
inline S32 ChannelVector::getFlags(S32 i)
{
   if (i >= 0 && i < mMembers.size())
      return mMembers[i].flags;
   else
      return 0;
}

#endif // _H_CHANNELVECTOR_
