//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "console/consoleTypes.h"
#include "console/console.h"
#include "Platform/event.h"
#include "dgl/gBitmap.h"
#include "dgl/dgl.h"
#include "Sim/actionMap.h"
#include "GUI/guiCanvas.h"
#include "GUI/guiControl.h"
#include "console/consoleInternal.h"
#include "console/objectTypes.h"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

//used to locate the next/prev responder when tab is pressed
GuiControl *GuiControl::gPrevResponder = NULL;
GuiControl *GuiControl::gCurResponder = NULL;

GuiControl::GuiControl()
{
   mLayer = 0;
   mBounds.set(0, 0, 64, 64);
   mMinExtent.set(8, 8);
   mHelpTag = 0;
   mHelpText = StringTable->insert("");
   
   mProfile = NULL;
   
   mConsoleVariable = StringTable->insert("");
   mConsoleCommand = StringTable->insert("");
   mAltConsoleCommand = StringTable->insert("");
   mAcceleratorKey = StringTable->insert("");
   
   mFirstResponder = NULL;
   
	mVisible = true;
   mActive = false;
   mAwake = false;
   
   mHorizSizing = horizResizeRight;
   mVertSizing = vertResizeBottom;
                
   mTypeMask |= GuiControlObjectType;
}

GuiControl::~GuiControl()
{
}

bool GuiControl::onAdd()
{
   if(!Parent::onAdd())
      return false;
   const char *name = getName();
   if(name && name[0] && getClassRep())
   {
      Namespace *parent = getClassRep()->getNameSpace();
      Con::linkNamespaces(parent->mName, name);
      mNameSpace = Con::lookupNamespace(name);
   
   }
   Sim::getGuiGroup()->addObject(this);
   Con::executef(this, 1, "onAdd");
   return true;
}


static EnumTable::Enums horzEnums[] = 
{
	{ GuiControl::horizResizeRight,      "right"     },
	{ GuiControl::horizResizeWidth,      "width"     },
	{ GuiControl::horizResizeLeft,       "left"      },
   { GuiControl::horizResizeCenter,     "center"    },
   { GuiControl::horizResizeRelative,   "relative"  }
};
static EnumTable gHorizSizingTable(5, &horzEnums[0]); 

static EnumTable::Enums vertEnums[] = 
{
	{ GuiControl::vertResizeBottom,      "bottom"     },
	{ GuiControl::vertResizeHeight,      "height"     },
	{ GuiControl::vertResizeTop,         "top"        },
   { GuiControl::vertResizeCenter,      "center"     },
   { GuiControl::vertResizeRelative,    "relative"   }
};
static EnumTable gVertSizingTable(5, &vertEnums[0]); 

void GuiControl::initPersistFields()
{
   Parent::initPersistFields();
   addField("profile",           TypeGuiProfile, Offset(mProfile, GuiControl));
   addField("horizSizing",       TypeEnum,         Offset(mHorizSizing, GuiControl), 1, &gHorizSizingTable);
   addField("vertSizing",        TypeEnum,         Offset(mVertSizing, GuiControl), 1, &gVertSizingTable);
   
   
   addField("position",          TypePoint2I,      Offset(mBounds.point, GuiControl));
   addField("extent",            TypePoint2I,      Offset(mBounds.extent, GuiControl));
   addField("minExtent",         TypePoint2I,      Offset(mMinExtent, GuiControl));
   
   addField("visible",           TypeBool,         Offset(mVisible, GuiControl));
   addDepricatedField("modal");
   addDepricatedField("setFirstResponder");
   
   addField("variable",          TypeString,       Offset(mConsoleVariable, GuiControl));
   addField("command",           TypeString,       Offset(mConsoleCommand, GuiControl));
   addField("altCommand",        TypeString,       Offset(mAltConsoleCommand, GuiControl));
   addField("accelerator",       TypeString,       Offset(mAcceleratorKey, GuiControl));
   
   addField("helpTag",           TypeS32,          Offset(mHelpTag, GuiControl));
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

void GuiControl::setHelp(S32 helpTag)
{
   mHelpText = StringTable->insert("");
   mHelpTag = helpTag;
}

void GuiControl::setHelp(const char *text)
{
   if (! text) return;
   
   mHelpText = StringTable->insert(text);
   mHelpTag = 0;
}

void GuiControl::addObject(SimObject *object)
{
   GuiControl *ctrl = dynamic_cast<GuiControl *>(object);
   if(!ctrl)
   {
      AssertWarn(0, "GuiControl::addObject: attempted to add NON GuiControl to set");
      return;
   }

   if(object->getGroup() == this)
      return;

	Parent::addObject(object);

   AssertFatal(!ctrl->isAwake(), "GuiControl::addObject: object is already awake before add");
   if(mAwake)
      ctrl->awaken();
}

void GuiControl::removeObject(SimObject *object)
{
   AssertFatal(mAwake == static_cast<GuiControl*>(object)->isAwake(), "GuiControl::removeObject: child control wake state is bad");
   if (mAwake)
      static_cast<GuiControl*>(object)->sleep();
	Parent::removeObject(object);
}

GuiControl *GuiControl::getParent()
{
	GuiControl *parent = NULL;
	SimObject *obj = getGroup();
	if (obj && (obj->getType() & GuiControlObjectType))
      parent = static_cast<GuiControl*>(obj);
	return parent;
}

GuiCanvas *GuiControl::getRoot()
{
   GuiControl *root = NULL;
	GuiControl *parent = getParent();
   while (parent)
   {
      root = parent;
      parent = parent->getParent();
   }
   if (root)
      return dynamic_cast<GuiCanvas*>(root);
   else
      return NULL;
}

void GuiControl::inspectPreApply()
{
   if(mAwake)
   {
      onSleep(); // release all our resources.
      mAwake = true;
   }
}

void GuiControl::inspectPostApply()
{
   if(mAwake)
   {
      mAwake = false;
      onWake();
   }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

Point2I GuiControl::localToGlobalCoord(const Point2I &src)
{
   Point2I ret = src;
   ret += mBounds.point;
   GuiControl *walk = getParent();
   while(walk)
   {
      ret += walk->getPosition();
      walk = walk->getParent();
   }
   return ret;
}

Point2I GuiControl::globalToLocalCoord(const Point2I &src)
{
   Point2I ret = src;
   ret -= mBounds.point;
   GuiControl *walk = getParent();
   while(walk)
   {
      ret -= walk->getPosition();
      walk = walk->getParent();
   }
   return ret;
}

//----------------------------------------------------------------

void GuiControl::resize(const Point2I &newPosition, const Point2I &newExtent)
{
   //call set update both before and after
   setUpdate();
   Point2I actualNewExtent = Point2I(getMax(mMinExtent.x, newExtent.x),
                                       getMax(mMinExtent.y, newExtent.y));
   iterator i;
   for(i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      ctrl->parentResized(mBounds.extent, actualNewExtent);
   }
   mBounds.set(newPosition, actualNewExtent);

   GuiControl *parent = getParent();
	if (parent)
      parent->childResized(this);
   setUpdate();
}

void GuiControl::childResized(GuiControl *child)
{
   child;
   // default to do nothing...
}

void GuiControl::parentResized(const Point2I &oldParentExtent, const Point2I &newParentExtent)
{
   Point2I newPosition = getPosition();
   Point2I newExtent = getExtent();

	S32 deltaX = newParentExtent.x - oldParentExtent.x;
 	S32 deltaY = newParentExtent.y - oldParentExtent.y;

	if (mHorizSizing == horizResizeCenter)
	   newPosition.x = (newParentExtent.x - mBounds.extent.x) >> 1;
	else if (mHorizSizing == horizResizeWidth)
		newExtent.x += deltaX;
	else if (mHorizSizing == horizResizeLeft)
      newPosition.x += deltaX;
   else if (mHorizSizing == horizResizeRelative && oldParentExtent.x != 0)
   {
      S32 newLeft = (newPosition.x * newParentExtent.x) / oldParentExtent.x;
      S32 newRight = ((newPosition.x + newExtent.x) * newParentExtent.x) / oldParentExtent.x;
      
      newPosition.x = newLeft;
      newExtent.x = newRight - newLeft;
   }

	if (mVertSizing == vertResizeCenter)
	   newPosition.y = (newParentExtent.y - mBounds.extent.y) >> 1;
	else if (mVertSizing == vertResizeHeight)
		newExtent.y += deltaY;
	else if (mVertSizing == vertResizeTop)
      newPosition.y += deltaY;
   else if(mVertSizing == vertResizeRelative && oldParentExtent.y != 0)
   {
      
      S32 newTop = (newPosition.y * newParentExtent.y) / oldParentExtent.y;
      S32 newBottom = ((newPosition.y + newExtent.y) * newParentExtent.y) / oldParentExtent.y;
      
      newPosition.y = newTop;
      newExtent.y = newBottom - newTop;
   }
	resize(newPosition, newExtent);
}

//----------------------------------------------------------------

void GuiControl::onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder)
{
   RectI ctrlRect(offset, mBounds.extent);
   
   //if opaque, fill the update rect with the fill color
   if (mProfile->mOpaque)
      dglDrawRectFill(ctrlRect, mProfile->mFillColor);
   
   //if there's a boarder, draw the boarder
   if (mProfile->mBorder)
      dglDrawRect(ctrlRect, mProfile->mBorderColor);
   
   renderChildControls(offset, updateRect, firstResponder);
}

void GuiControl::renderChildControls(Point2I offset, const RectI &updateRect, GuiControl *firstResponder)
{
   // offset is the upper-left corner of this control in screen coordinates
   // updateRect is the intersection rectangle in screen coords of the control
   // hierarchy.  This can be set as the clip rectangle in most cases.
   RectI clipRect = updateRect;

   iterator i;
   for(i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      if (ctrl->mVisible)
      {
         Point2I childPosition = offset + ctrl->getPosition();
         RectI childClip(childPosition, ctrl->getExtent());

         if (childClip.intersect(clipRect))
         {
            dglSetClipRect(childClip);
            glDisable(GL_CULL_FACE);
            ctrl->onRender(childPosition, childClip, firstResponder);
         }
      }
   }
}

void GuiControl::setUpdateRegion(Point2I pos, Point2I ext)
{
   Point2I upos = localToGlobalCoord(pos);
   GuiCanvas *root = getRoot();
   if (root)
   {
      root->addUpdateRegion(upos, ext);
   }
}

void GuiControl::setUpdate()
{
   setUpdateRegion(Point2I(0,0), mBounds.extent);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

void GuiControl::awaken()
{
   AssertFatal(!mAwake, "GuiControl::awaken: control is already awake");
   if(mAwake)
      return;

   iterator i;
   for(i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      
      AssertFatal(!ctrl->isAwake(), "GuiControl::awaken: child control is already awake");
      if(!ctrl->isAwake())
         ctrl->awaken();
   }
   
   AssertFatal(!mAwake, "GuiControl::awaken: should not be awake here");
   if(!mAwake)
   {   
      if(!onWake())
      {
         Con::errorf(ConsoleLogEntry::General, "GuiControl::awaken: failed onWake for obj: %s", getName());
         AssertFatal(0, "GuiControl::awaken: failed onWake");
         deleteObject();
      }
   }
}

void GuiControl::sleep()
{
   AssertFatal(mAwake, "GuiControl::sleep: control is not awake");
   if(!mAwake)
      return;
      
   iterator i;
   for(i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
   
      AssertFatal(ctrl->isAwake(), "GuiControl::sleep: child control is already asleep");
      if(ctrl->isAwake())   
         ctrl->sleep();
   }
   
   AssertFatal(mAwake, "GuiControl::sleep: should not be asleep here");
   if(mAwake)
      onSleep();
}

void GuiControl::preRender()
{
   AssertFatal(mAwake, "GuiControl::preRender: control is not awake");
   if(!mAwake)
      return;

   iterator i;
   for(i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      ctrl->preRender();
   }
   onPreRender();
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

bool GuiControl::onWake()
{
   AssertFatal(!mAwake, "GuiControl::onWake: control is already awake");
   if(mAwake)
      return false;

   //make sure we have a profile
   if (! mProfile)
   {
      SimObject *obj = Sim::findObject("GuiDefaultProfile");
      if (obj)
         mProfile = dynamic_cast<GuiControlProfile*>(obj);
      AssertFatal(mProfile, avar("GuiControl: %s created with no profile.", getName()));
   }
   
   //set the flag
   mAwake = true;
   
   //set the layer
   GuiCanvas *root = getRoot();
   AssertFatal(root, "Unable to get the root Canvas.");
   GuiControl *parent = getParent();
   if (parent && parent != root)
      mLayer = parent->mLayer;
   
   //make sure the first responder exists
   if (! mFirstResponder)
      mFirstResponder = findFirstTabable();
   
   //see if we should force this control to be the first responder
   if (mProfile->mTabable && mProfile->mCanKeyFocus)
      setFirstResponder();
   
   //increment the profile
   mProfile->incRefCount();
   return true;
}

void GuiControl::onSleep()
{
   AssertFatal(mAwake, "GuiControl::onSleep: control is not awake");
   if(!mAwake)
      return;

   //decrement the profile referrence
   mProfile->decRefCount();
   clearFirstResponder();
   mouseUnlock();
      
   //set the flag
   mAwake = false;
}

void GuiControl::setControlProfile(GuiControlProfile *prof)
{
   AssertFatal(prof, "GuiControl::setControlProfile: invalid profile");
   if(prof == mProfile)
      return;
   if(mAwake)
      mProfile->decRefCount();
   mProfile = prof;
   if(mAwake)
      mProfile->incRefCount();

}

void GuiControl::onPreRender()
{
   // do nothing.
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

static void cGuiControlSetValue(SimObject *obj, S32, const char **argv)
{
   GuiControl *ctrl = static_cast<GuiControl*>(obj);
   ctrl->setScriptValue(argv[2]);
}

static const char* cGuiControlGetValue(SimObject *obj, S32, const char **)
{
   GuiControl *ctrl = static_cast<GuiControl*>(obj);
   return ctrl->getScriptValue();
}

static void cGuiControlSetActive(SimObject *obj, S32, const char **argv)
{
   GuiControl *ctrl = static_cast<GuiControl*>(obj);
   ctrl->setActive(dAtob(argv[2]));
}

static bool cGuiControlIsActive(SimObject *obj, S32, const char **)
{
   GuiControl *ctrl = static_cast<GuiControl*>(obj);
   return ctrl->isActive();
}

static void cGuiControlSetVisible(SimObject *obj, S32, const char **argv)
{
   GuiControl *ctrl = static_cast<GuiControl*>(obj);
   ctrl->setVisible(dAtob(argv[2]));
}

static void cGuiControlMakeFirstResponder(SimObject *obj, S32, const char **argv)
{
   GuiControl *ctrl = static_cast<GuiControl*>(obj);
   ctrl->makeFirstResponder(dAtob(argv[2]));
}

static bool cGuiControlIsVisible(SimObject *obj, S32, const char **)
{
   GuiControl *ctrl = static_cast<GuiControl*>(obj);
   return ctrl->isVisible();
}

static bool cGuiControlIsAwake(SimObject *obj, S32, const char **)
{
   GuiControl *ctrl = static_cast<GuiControl*>(obj);
   return ctrl->isAwake();
}

static void cGuiControlSetProfile(SimObject *obj, S32, const char ** argv)
{
   GuiControl * ctrl = static_cast<GuiControl*>(obj);
   GuiControlProfile * profile = dynamic_cast<GuiControlProfile*>(Sim::findObject(argv[2]));
   if(profile)
      ctrl->setControlProfile(profile);
}

static void cGuiControlResize(SimObject *obj, S32, const char **argv)
{
   GuiControl *ctrl = static_cast<GuiControl*>(obj);
   Point2I newPos(dAtoi(argv[2]), dAtoi(argv[3]));
   Point2I newExt(dAtoi(argv[4]), dAtoi(argv[5]));
   ctrl->resize(newPos, newExt);
}

static const char *cGuiControlGetPosition(SimObject *obj, S32, const char **)
{
   GuiControl *ctrl = static_cast<GuiControl*>(obj);
   char *retBuffer = Con::getReturnBuffer(64);
   const Point2I &pos = ctrl->getPosition();
   dSprintf(retBuffer, 64, "%d %d", pos.x, pos.y);
   return retBuffer;
}

static const char *cGuiControlGetExtent(SimObject *obj, S32, const char **)
{
   GuiControl *ctrl = static_cast<GuiControl*>(obj);
   char *retBuffer = Con::getReturnBuffer(64);
   const Point2I &ext = ctrl->getExtent();
   dSprintf(retBuffer, 64, "%d %d", ext.x, ext.y);
   return retBuffer;
}

static const char *cGuiControlGetMinExtent(SimObject *obj, S32, const char **)
{
   GuiControl *ctrl = static_cast<GuiControl*>(obj);
   char *retBuffer = Con::getReturnBuffer(64);
   const Point2I &minExt = ctrl->getMinExtent();
   dSprintf(retBuffer, 64, "%d %d", minExt.x, minExt.y);
   return retBuffer;
}

void GuiControl::consoleInit()
{
   Con::addCommand("GuiControl", "getPosition", 	cGuiControlGetPosition, 	"ctrl.getPosition()",      2, 2);
   Con::addCommand("GuiControl", "getExtent",   	cGuiControlGetExtent,   	"ctrl.getExtent()",        2, 2);
   Con::addCommand("GuiControl", "getMinExtent",	cGuiControlGetMinExtent,	"ctrl.getMinExtent()",     2, 2);
   Con::addCommand("GuiControl", "resize",      	cGuiControlResize,      	"ctrl.resize(x,y,w,h)",    6, 6);
   Con::addCommand("GuiControl", "setValue",    	cGuiControlSetValue,    	"ctrl.setValue(value)",    3, 3);
   Con::addCommand("GuiControl", "getValue",    	cGuiControlGetValue,    	"ctrl.getValue()",         2, 2);
   Con::addCommand("GuiControl", "setActive",   	cGuiControlSetActive,   	"ctrl.setActive(value)",   3, 3);
   Con::addCommand("GuiControl", "isActive",    	cGuiControlIsActive,    	"ctrl.isActive()",         2, 2);
   Con::addCommand("GuiControl", "setVisible",  	cGuiControlSetVisible,  	"ctrl.setVisible(value)",  3, 3);
   Con::addCommand("GuiControl", "isVisible",   	cGuiControlIsVisible,   	"ctrl.isVisible()",        2, 2);
   Con::addCommand("GuiControl", "isAwake",   	   cGuiControlIsAwake,   	   "ctrl.isAwake()",          2, 2);

   Con::addCommand("GuiControl", "setProfile",     cGuiControlSetProfile,     "ctrl.setProfile(profileI)", 3, 3);
   Con::addCommand("GuiControl", "makeFirstResponder", cGuiControlMakeFirstResponder,  "ctrl.makeFirstResponder(value)",  3, 3);
}

void GuiControl::onRemove()
{
   Con::executef(this, 1, "onRemove");
   clearFirstResponder();
   Parent::onRemove();
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

const char *GuiControl::getScriptValue()
{
   return NULL;
}

void GuiControl::setScriptValue(const char *value)
{
   value;
}

void GuiControl::setConsoleVariable(const char *variable)
{
   if (variable)
   {
      mConsoleVariable = StringTable->insert(variable);
   }
   else
   {
      mConsoleVariable = StringTable->insert("");
   }
}   

void GuiControl::setConsoleCommand(const char *newCmd)
{
   if (newCmd)
      mConsoleCommand = StringTable->insert(newCmd);
   else
      mConsoleCommand = StringTable->insert("");
}   

const char * GuiControl::getConsoleCommand()
{
	return mConsoleCommand;
}

void GuiControl::setSizing(S32 horz, S32 vert)
{
	mHorizSizing = horz;
	mVertSizing = vert;
}


void GuiControl::setVariable(const char *value)
{
   if (mConsoleVariable[0])
      Con::setVariable(mConsoleVariable, value);
}

void GuiControl::setIntVariable(S32 value)
{
   if (mConsoleVariable[0])
      Con::setIntVariable(mConsoleVariable, value);
}

void GuiControl::setFloatVariable(F32 value)
{
   if (mConsoleVariable[0])
      Con::setFloatVariable(mConsoleVariable, value);
}

const char * GuiControl::getVariable()
{
   if (mConsoleVariable[0])
      return Con::getVariable(mConsoleVariable);
   else return NULL;
}

S32 GuiControl::getIntVariable()
{
   if (mConsoleVariable[0])
      return Con::getIntVariable(mConsoleVariable);
   else return 0;
}

F32 GuiControl::getFloatVariable()
{
   if (mConsoleVariable[0])
      return Con::getFloatVariable(mConsoleVariable);
   else return 0.0f;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

bool GuiControl::cursorInControl()
{
   GuiCanvas *root = getRoot();
   if (! root) return false;
   
   Point2I pt = root->getCursorPos();
   Point2I offset = localToGlobalCoord(Point2I(0, 0));
   if (pt.x >= offset.x && pt.y >= offset.y &&
      pt.x < offset.x + mBounds.extent.x && pt.y < offset.y + mBounds.extent.y)
   {
      return true;
   }
   else
   {
      return false;
   }
}

bool GuiControl::pointInControl(const Point2I& parentCoordPoint)
{
   S32 xt = parentCoordPoint.x - mBounds.point.x;
   S32 yt = parentCoordPoint.y - mBounds.point.y;
   return xt >= 0 && yt >= 0 && xt < mBounds.extent.x && yt < mBounds.extent.y;
}

GuiControl* GuiControl::findHitControl(const Point2I &pt, S32 initialLayer)
{
   Point2I ptemp = pt;
   iterator i = end(); // find in z order (last to first)
   while (i != begin())
   {
      i--;
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      if (initialLayer >= 0 && ctrl->mLayer > initialLayer)
      {
         continue;
      }
      else if (ctrl->mVisible && ctrl->pointInControl(ptemp))
      {
         ptemp.x -= ctrl->mBounds.point.x;
         ptemp.y -= ctrl->mBounds.point.y;
         return ctrl->findHitControl(ptemp);
      }
   }
   return this;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

void GuiControl::mouseLock(GuiControl *lockingControl)
{
   GuiCanvas *root = getRoot();
   if (root) root->mouseLock(lockingControl);
}

void GuiControl::mouseLock()
{
   GuiCanvas *root = getRoot();
   if (root) root->mouseLock(this);
}

void GuiControl::mouseUnlock()
{
   GuiCanvas *root = getRoot();
   if (root) root->mouseUnlock(this);
}

bool GuiControl::onInputEvent(const InputEvent &)
{
	// Do nothing by default...
   return( false );
}

void GuiControl::onMouseUp(const GuiEvent &)
{
   if (! mActive) return;
   
   mouseUnlock();

   setUpdate();
   
   //if we released the mouse within this control, perform the action
   if (cursorInControl())
   {
      onAction();
   }
}

void GuiControl::onMouseDown(const GuiEvent &)
{
   if ((! mActive) || (! mVisible) || (! mAwake)) return;
   
   if (mProfile->mCanKeyFocus)
      setFirstResponder();
   
   //lock the mouse
   mouseLock(); 
   
   //update
   setUpdate();
}

void GuiControl::onMouseMove(const GuiEvent &)
{
}

void GuiControl::onMouseDragged(const GuiEvent &)
{
}

void GuiControl::onMouseEnter(const GuiEvent &)
{
}

void GuiControl::onMouseLeave(const GuiEvent &)
{
}

bool GuiControl::onMouseWheelUp( const GuiEvent &event )
{
   //if this control is a dead end, make sure the event stops here
   if ( !mVisible || !mAwake )
      return true;
   
   //pass the event to the parent
   GuiControl *parent = getParent();
   if ( parent )
      return parent->onMouseWheelUp( event );
   else
      return false;
}

bool GuiControl::onMouseWheelDown( const GuiEvent &event )
{
   //if this control is a dead end, make sure the event stops here
   if ( !mVisible || !mAwake )
      return true;
   
   //pass the event to the parent
   GuiControl *parent = getParent();
   if ( parent )
      return parent->onMouseWheelDown( event );
   else
      return false;
}

void GuiControl::onRightMouseDown(const GuiEvent &)
{
}

void GuiControl::onRightMouseUp(const GuiEvent &)
{
}

void GuiControl::onRightMouseDragged(const GuiEvent &)
{
}

GuiControl* GuiControl::findFirstTabable()
{
   GuiControl *tabCtrl = NULL;
   iterator i;
   for (i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      tabCtrl = ctrl->findFirstTabable();
      if (tabCtrl)
      {
         mFirstResponder = tabCtrl;
         return tabCtrl;
      }
   }
   
   //nothing was found, therefore, see if this ctrl is tabable
   return ( mProfile->mTabable && mAwake && mVisible ) ? this : NULL;
}

GuiControl* GuiControl::findLastTabable(bool firstCall)
{
   //if this is the first call, clear the global
   if (firstCall)
      gPrevResponder = NULL;
   
   //if this control is tabable, set the global
   if (mProfile->mTabable)
      gPrevResponder = this;
   
   iterator i;
   for (i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      ctrl->findLastTabable(false);
   }
   
   //after the entire tree has been traversed, return the last responder found
   mFirstResponder = gPrevResponder;
   return gPrevResponder;
}

GuiControl *GuiControl::findNextTabable(GuiControl *curResponder, bool firstCall)
{
   //if this is the first call, clear the global
   if (firstCall)
      gCurResponder = NULL;
     
   //first find the current responder
   if (curResponder == this)
      gCurResponder = this;
   
   //if the first responder has been found, return the very next *tabable* control
   else if ( gCurResponder && mProfile->mTabable && mAwake && mVisible && mActive )
      return( this );

   //loop through, checking each child to see if it is the one that follows the firstResponder
   GuiControl *tabCtrl = NULL;
   iterator i;
   for (i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      tabCtrl = ctrl->findNextTabable(curResponder, false);
      if (tabCtrl) break;
   }
   mFirstResponder = tabCtrl;
   return tabCtrl;
}

GuiControl *GuiControl::findPrevTabable(GuiControl *curResponder, bool firstCall)
{
   if (firstCall)
      gPrevResponder = NULL;
     
   //if this is the current reponder, return the previous one
   if (curResponder == this)
      return gPrevResponder;
   
   //else if this is a responder, store it in case the next found is the current responder
   else if ( mProfile->mTabable && mAwake && mVisible && mActive )
      gPrevResponder = this;
   
   //loop through, checking each child to see if it is the one that follows the firstResponder
   GuiControl *tabCtrl = NULL;
   iterator i;
   for (i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      tabCtrl = ctrl->findPrevTabable(curResponder, false);
      if (tabCtrl) break;
   }
   mFirstResponder = tabCtrl;
   return tabCtrl;
}

void GuiControl::onLoseFirstResponder()
{
	// Since many controls have visual cues when they are the firstResponder...
	setUpdate();
}

bool GuiControl::ControlIsChild(GuiControl *child)
{
   //function returns true if this control, or one of it's children is the child control
   if (child == this)
      return true;
   
   //loop through, checking each child to see if it is ,or contains, the firstResponder
   iterator i;
   for (i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      if (ctrl->ControlIsChild(child)) return true;
   }
   
   //not found, therefore false
   return false;
}
   
void GuiControl::setFirstResponder( GuiControl* firstResponder )
{
   if ( firstResponder && firstResponder->mProfile->mCanKeyFocus )
      mFirstResponder = firstResponder;
   
   GuiControl *parent = getParent();
   if ( parent )
      parent->setFirstResponder( firstResponder );
}

void GuiControl::setFirstResponder()
{
	if ( mAwake && mVisible )
	{
	   GuiControl *parent = getParent();
	   if (mProfile->mCanKeyFocus && parent)
	      parent->setFirstResponder(this);
		// Since many controls have visual cues when they are the firstResponder...
		setUpdate();	
	}
}

void GuiControl::clearFirstResponder()
{
   GuiControl *parent = this;
   while((parent = parent->getParent()) != NULL)
   {
      if(parent->mFirstResponder == this)
         parent->mFirstResponder = NULL;
      else
         break;
   }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

void GuiControl::BuildAcceleratorMap()
{
   //add my own accel key
   AddAcceleratorKey();
   
   //add all my childrens keys
   iterator i;
   for(i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      ctrl->BuildAcceleratorMap();
   }
}

void GuiControl::AddAcceleratorKey()
{
   //see if we have an accelerator
   if (mAcceleratorKey == StringTable->insert(""))
      return;
   
   EventDescriptor accelEvent;
   ActionMap::createEventDescriptor(mAcceleratorKey, &accelEvent);
   
   //now we have a modifier, and a key, add them to the canvas
   GuiCanvas *root = getRoot();
   if (root)
      root->AddAcceleratorKey(this, accelEvent.eventCode, accelEvent.flags);
}

void GuiControl::AcceleratorKeyPress()
{
   onAction();
}

void GuiControl::AcceleratorKeyRelease()
{
   //do nothing
}
   
bool GuiControl::onKeyDown(const GuiEvent &event)
{
   //pass the event to the parent
   GuiControl *parent = getParent();
   if (parent)
      return parent->onKeyDown(event);
   else
      return false;
}

bool GuiControl::onKeyRepeat(const GuiEvent &event)
{
   // default to just another key down.
   return onKeyDown(event);
}

bool GuiControl::onKeyUp(const GuiEvent &event)
{
   //pass the event to the parent
   GuiControl *parent = getParent();
   if (parent)
      return parent->onKeyUp(event);
   else
      return false;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

void GuiControl::onAction()
{
   if (! mActive)
      return;
   
   //execute the console command
   if (mConsoleCommand[0])
   {
   	char buf[16];
      dSprintf(buf, sizeof(buf), "%d", getId());
      Con::setVariable("$ThisControl", buf);
      Con::evaluate(mConsoleCommand, false);
   }
   else
      Con::executef(this, 1, "onAction");
}

void GuiControl::onMessage(GuiControl *sender, S32 msg)
{
   sender;
   msg;
}

void GuiControl::messageSiblings(S32 message)
{
   GuiControl *parent = getParent();
   if (! parent) return;
   GuiControl::iterator i;
   for(i = parent->begin(); i != parent->end(); i++)
   {
      GuiControl *ctrl = dynamic_cast<GuiControl *>(*i);
      if (ctrl != this)
         ctrl->onMessage(this, message);
   }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

void GuiControl::onDialogPush()
{
}

void GuiControl::onDialogPop()
{
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

bool GuiControl::createBitmapArray(GBitmap *bmp, RectI *boundsArray, S32 numStates, S32 numBitmaps)
{
   //get the separator color
   ColorI sepColor;
   if ( !bmp->getColor( 0, 0, sepColor ) )
	{
		AssertFatal( false, "createBitmapArray - failed to get sepearator color!" );
      return false;
	}
   
   //now loop through all the scroll pieces, and find the bounding rectangle for each piece in each state
   U32 bmpYOffset = 0;
   for ( S32 i = 0; i < numBitmaps; i++ )
   {
      ColorI cmpColor;
      U32 bmpWidth;
      U32 bmpHeight;
      for ( bmpWidth = 0; bmpWidth < bmp->getWidth(); bmpWidth++ )
      {
         if ( !bmp->getColor( bmpWidth, bmpYOffset, cmpColor ) )
			{
				AssertFatal( false, avar( "createBitmapArray - getColor failed in width loop on (%d, %d)!\nBitmap index = %d", bmpWidth, bmpYOffset, i ) );
            return false;
			}
         if ( cmpColor != sepColor )
            break;
      }
      bmpYOffset += 1;
   
      for ( bmpHeight = 0; bmpHeight + bmpYOffset < bmp->getHeight(); bmpHeight++ )
      {
         if ( !bmp->getColor( 0, bmpHeight + bmpYOffset, cmpColor ) )
			{
				AssertFatal( false, avar( "createBitmapArray - getColor failed in height loop on (0, %d)!\nBitmap index = %d", ( bmpHeight + bmpYOffset ), i ) );
            return false;
			}
         if (cmpColor == sepColor)
            break;
      }
      //set the bounding rectangle for each piece
      for ( S32 j = 0; j < numStates; j++ )
      {
         boundsArray[numStates * i + j].set( j * bmpWidth, bmpYOffset, bmpWidth, bmpHeight );
			boundsArray[numStates * i + j].point += Point2I( 1, 1 );
			boundsArray[numStates * i + j].extent -= Point2I( 2, 2 );
         
         //make sure we don't go past the end
         if ( j * bmpWidth + bmpWidth > bmp->getWidth() + 1 )
			{
				AssertFatal( false, avar( "createBitmapArray - column widths don't match up in row %d!", i ) );
            return false;
			}
      }
      
      //update the offset
      bmpYOffset += bmpHeight;
   }
   
   //make sure we didn't go past the end
   if ( bmpYOffset > bmp->getHeight() + 1 )
	{
		AssertFatal( false, "createBitmapArray - y offset exceeded bitmap height!" );
      return false;
	}
   
   return true;
}

//------------------------------------------------------------------------------
void GuiControl::setVisible(bool value)
{ 
	mVisible = value; 
   iterator i;
   setUpdate();
   for(i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      ctrl->clearFirstResponder();
	}

	GuiControl *parent = getParent();
	if (parent)
	   parent->childResized(this);
}


void GuiControl::makeFirstResponder(bool value)
{
   if ( value )
      //setFirstResponder(this);
      setFirstResponder();
   else
      clearFirstResponder();
}

void GuiControl::setActive( bool value )
{
   mActive = value;

   if ( !mActive )
      clearFirstResponder();

   if ( mVisible && mAwake )
      setUpdate();
}

void renderRaisedBox(const Point2I &offset, const Point2I &extent, const ColorI &color)
{
   ColorI light, dark;
   light.interpolate(ColorI(255,255,255), color, 0.70);
   dark.interpolate( ColorI(255,255,255), color, 1.3);

   // fill
   dglDrawRectFill( offset, Point2I(extent.x+1, extent.y+1), color );
   
   // hilight
   dglDrawLine(offset.x, offset.y, extent.x, offset.y, light);
   dglDrawLine(offset.x, offset.y, offset.x, extent.y, light);

   // shadow
   dglDrawLine(offset.x+1, extent.y-1, extent.x, extent.y-1, dark);
   dglDrawLine(extent.x-1, offset.y+1, extent.x-1, extent.y-1, dark);

   // edge
   dglDrawLine(offset.x, extent.y, extent.x+1, extent.y, ColorI(0,0,0));
   dglDrawLine(extent.x, offset.y, extent.x, extent.y, ColorI(0,0,0));
}  

 
void renderLoweredBox(const Point2I &offset, const Point2I &extent, const ColorI &color)
{
   ColorI light, dark;
   light.interpolate(ColorI(255,255,255), color, 0.70);
   dark.interpolate( ColorI(255,255,255), color, 1.3);

   // fill
   dglDrawRectFill( offset, Point2I(extent.x+1, extent.y+1), color );

   // edge
   dglDrawLine(offset.x, offset.y, extent.x, offset.y, ColorI(0,0,0));
   dglDrawLine(offset.x, offset.y, offset.x, extent.y, ColorI(0,0,0));

   // shadow
   dglDrawLine(offset.x+1, offset.y+1, extent.x-1, offset.y+1, dark);
   dglDrawLine(offset.x+1, offset.y+1, offset.x+1, extent.y-1, dark);

   // hilight
   dglDrawLine(offset.x, extent.y, extent.x+1, extent.y, light);
   dglDrawLine(extent.x, offset.y, extent.x, extent.y, light);
}
