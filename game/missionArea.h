//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _MISSIONAREA_H_
#define _MISSIONAREA_H_

#ifndef _NETOBJECT_H_
#include "Sim/netObject.h"
#endif

class MissionArea : public NetObject
{
  private:
   typedef NetObject Parent;
   RectI             mArea;

   F32 mFlightCeiling;
   F32 mFlightCeilingRange;
   
  public:
   MissionArea();

   static RectI      smMissionArea;

   static const MissionArea * getServerObject();

   F32 getFlightCeiling()      const { return mFlightCeiling;      }
   F32 getFlightCeilingRange() const { return mFlightCeilingRange; }

   //            
   const RectI & getArea(){return(mArea);}
   void setArea(const RectI & area);
      
   // SimObject
   bool onAdd();

   static void initPersistFields();
   static void consoleInit();
      
   // NetObject
   enum NetMaskBits {
      UpdateMask     = BIT(0)
   };

   void unpackUpdate(NetConnection *, BitStream * stream);
   U32 packUpdate(NetConnection *, U32 mask, BitStream * stream);
      
   DECLARE_CONOBJECT(MissionArea);
};

#endif
