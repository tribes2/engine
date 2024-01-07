//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "game/staticShape.h"
#include "game/gameConnection.h"

class ScopeAlwaysShape : public StaticShape
{
      typedef StaticShape Parent;

   public:
      ScopeAlwaysShape();
      static void initPersistFields();
      DECLARE_CONOBJECT(ScopeAlwaysShape);
};

ScopeAlwaysShape::ScopeAlwaysShape()
{
   mNetFlags.set(Ghostable|ScopeAlways);
}

void ScopeAlwaysShape::initPersistFields()
{
   Parent::initPersistFields();
}

IMPLEMENT_CO_NETOBJECT_V1(ScopeAlwaysShape);
