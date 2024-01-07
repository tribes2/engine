//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GRAPHTHREATS_H_
#define _GRAPHTHREATS_H_

#ifndef _SHAPEBASE_H_
#include "game/shapeBase.h"
#endif

struct SearchThreat : public SphereF
{
   SearchThreat();
   SearchThreat(ShapeBase * threat, F32 R, S32 team);
   void                    updateNodes();
   bool                    checkEnableChange();
   SimObjectPtr<ShapeBase> mThreat;
   GraphNodeList           mEffects;
   bool                    mActive;
   S16                     mTeam;
   S16                     mSlot;
};

class SearchThreats
{
   public:
      enum{ MaxThreats  = (sizeof(GraphThreatSet) << 3) };
      
   protected:
      void           informOtherTeams(S32 onTeam, S32 thisThreat, bool isOn);
      S32            getThreatPtrs(const SearchThreat* L[MaxThreats], GraphThreatSet S) const;
   
   protected:
      SearchThreat   mThreats[MaxThreats];
      GraphThreatSet mTeamCaresAbout[GraphMaxTeams];
      S32            mCheck, mCount;
      U32            mSaveTimeMS;
      
   public:
      SearchThreats();
      void           pushTempThreat(GraphNode* node, const Point3F& loc, F32 rad);
      void           popTempThreat();
      
      bool           checkOne();
      bool           add(const SearchThreat&);
      GraphThreatSet getThreatSet(S32 forTeam) const;
      bool           sanction(const Point3F& from, const Point3F& to, S32 team) const;
      void           monitorThreats();
      S32            numThreats() const         {return mCount;}
};

#endif
