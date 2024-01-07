//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _SUN_H_
#define _SUN_H_

#ifndef _NETOBJECT_H_
#include "Sim/netObject.h"
#endif
#ifndef _COLOR_H_
#include "Core/color.h"
#endif
#ifndef _LIGHTMANAGER_H_
#include "sceneGraph/lightManager.h"
#endif

class Sun : public NetObject
{
   private:
      typedef NetObject Parent;

      LightInfo      mLight;
      
      void conformLight();

   public:

      Sun();

      // SimObject
      bool onAdd();
      void registerLights(LightManager *, bool);

      //
      void inspectPostApply();

      static void initPersistFields();

      // NetObject
      enum NetMaskBits {
         UpdateMask     = BIT(0)
      };

      void unpackUpdate(NetConnection *, BitStream * stream);
      U32 packUpdate(NetConnection *, U32 mask, BitStream * stream);

      DECLARE_CONOBJECT(Sun);
};

#endif
