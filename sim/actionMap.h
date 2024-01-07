//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _ACTIONMAP_H_
#define _ACTIONMAP_H_

#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif
#ifndef _TVECTOR_H_
#include "Core/tVector.h"
#endif
#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif

struct InputEvent;

struct EventDescriptor
{
   U8  flags;      // Combination of any Modifiers
   U8  eventType;  // SI_KEY, etc.
   U16 eventCode;  // From event.h
};

class ActionMap : public SimObject
{
   typedef SimObject Parent;

  protected:
   bool onAdd();

   struct Node {
      U32 modifiers;
      U32 action;

      enum Flags {
         Ranged      = 1 << 0,
         HasScale    = 1 << 1,
         HasDeadZone = 1 << 2,
         Inverted    = 1 << 3,
         BindCmd     = 1 << 4
      };

      U32 flags;
      F32 deadZoneBegin;
      F32 deadZoneEnd;
      F32 scaleFactor;

      StringTableEntry consoleFunction;
      
      char *makeConsoleCommand;
      char *breakConsoleCommand;
   };
   struct DeviceMap
   {
      U32 deviceType;
      U32 deviceInst;

      Vector<Node> nodeMap;
      DeviceMap() {
         VECTOR_SET_ASSOCIATION(nodeMap);
      }
      ~DeviceMap();
   };
   struct BreakEntry
   {
      U32 deviceType;
      U32 deviceInst;
      U32 objInst;
      StringTableEntry consoleFunction;
      char *breakConsoleCommand;

      // It's possible that the node could be deleted (unlikely, but possible,
      //  so we replicate the node flags here...
      //
      U32 flags;
      F32 deadZoneBegin;
      F32 deadZoneEnd;
      F32 scaleFactor;
   };


   Vector<DeviceMap*>        mDeviceMaps;
   static Vector<BreakEntry> smBreakTable;

   // Find: return NULL if not found in current map, Get: create if not
   //  found.
   const Node* findNode(const U32 inDeviceType, const U32 inDeviceInst,
                        const U32 inModifiers,  const U32 inAction);
   bool findBoundNode( const char* function, U32 &devMapIndex, U32 &nodeIndex );
   Node* getNode(const U32 inDeviceType, const U32 inDeviceInst,
                 const U32 inModifiers,  const U32 inAction);

   void removeNode(const U32 inDeviceType, const U32 inDeviceInst,
                 const U32 inModifiers,  const U32 inAction);

   void enterBreakEvent(const InputEvent* pEvent, const Node* pNode);

   static const char* getModifierString(const U32 modifiers);

  public:
   ActionMap();
   ~ActionMap();

   void dumpActionMap(const char* fileName, const bool append) const;

   static bool createEventDescriptor(const char* pEventString, EventDescriptor* pDescriptor);
   //static void consoleInit();

   bool processBind(const U32 argc, const char** argv);
   bool processBindCmd(const char *device, const char *action, const char *makeCmd, const char *breakCmd);
   bool processUnbind(const char *device, const char *action);

	// Console interface functions:
	const char* getBinding( const char* command ); // Find what the given command is bound to
	const char* getCommand( const char* device, const char* action ); // Find what command is bound to the given event descriptor 
	bool 	isInverted( const char* device, const char* action );
	F32	getScale( const char* device, const char* action );
	const char* getDeadZone( const char* device, const char* action );
	//

   static bool        getKeyString(const U32 action, char* buffer);
   static bool        getDeviceName(const U32 deviceType, const U32 deviceInstance, char* buffer);
	static const char* buildActionString( const InputEvent* event );

   bool processAction(const InputEvent*);

   static bool checkBreakTable(const InputEvent*);
   static bool handleEvent(const InputEvent*);
   static bool handleEventGlobal(const InputEvent*);

   static bool getDeviceTypeAndInstance(const char *device, U32 &deviceType, U32 &deviceInstance);

   DECLARE_CONOBJECT(ActionMap);
};

#endif // _ACTIONMAP_H_
