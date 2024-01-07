//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "console/console.h"
#include "dgl/dgl.h"
#include "console/simBase.h"
#include "GUI/guiCanvas.h"
#include "GUI/guiEditCtrl.h"
#include "Platform/event.h"
#include "Core/fileStream.h"

GuiEditCtrl::GuiEditCtrl()
{
   VECTOR_SET_ASSOCIATION(mSelectedControls);

   mActive = true;
   mCurrentAddSet = NULL;
   mGridSnap.set(0, 0);
}


static void cGuiEditCtrlSetRoot(SimObject *obj, S32, const char **argv)
{
   GuiControl *ctrl;
   if(!Sim::findObject(argv[2], ctrl))
      return;
   ((GuiEditCtrl *) obj)->setRoot(ctrl);
}

static void cGuiEditCtrlAdd(SimObject *obj, S32, const char **argv)
{
   GuiControl *ctrl;
   if(!Sim::findObject(argv[2], ctrl))
      return;
   ((GuiEditCtrl *) obj)->addNewControl(ctrl);
}

static void cGuiEditCtrlSelect(SimObject *obj, S32, const char **argv)
{
   GuiControl *ctrl;
   if(!Sim::findObject(argv[2], ctrl))
      return;
   ((GuiEditCtrl *) obj)->select(ctrl);
}

static void cGuiEditCtrlSetAddSet(SimObject *obj, S32, const char **argv)
{
   GuiEditCtrl *editor = static_cast<GuiEditCtrl*>(obj);
   GuiControl *addSet = NULL;
   SimObject *target = Sim::findObject(argv[2]);
   if (target)
      addSet = dynamic_cast<GuiControl*>(target);
   if (! addSet)
   {
      Con::printf("%s(): Invalid control: %s", argv[0], argv[2]);
      return;
   }
   editor->setCurrentAddSet(addSet);
}

static void cGuiEditCtrlToggle(SimObject *obj, S32, const char **)
{
   GuiEditCtrl *editor = static_cast<GuiEditCtrl*>(obj);
   editor->setEditMode(! editor->mActive);
}

static void cGuiEditCtrlJustify(SimObject *obj, S32, const char **argv)
{
   GuiEditCtrl *editor = static_cast<GuiEditCtrl*>(obj);
   editor->justifySelection((GuiEditCtrl::Justification)dAtoi(argv[2]));
}

static void cGuiEditCtrlBringToFront(SimObject *obj, S32, const char **)
{
   GuiEditCtrl *editor = static_cast<GuiEditCtrl*>(obj);
   editor->bringToFront();
}

static void cGuiEditCtrlPushToBack(SimObject *obj, S32, const char **)
{
   GuiEditCtrl *editor = static_cast<GuiEditCtrl*>(obj);
   editor->pushToBack();
}

void GuiEditCtrl::consoleInit()
{
   Con::addCommand("GuiEditCtrl", "addNewCtrl",         cGuiEditCtrlAdd,          "editCtrl.addNewCtrl(ctrl)",          3, 3);
   Con::addCommand("GuiEditCtrl", "select",             cGuiEditCtrlSelect,       "editCtrl.select(ctrl)",              3, 3);
   Con::addCommand("GuiEditCtrl", "setRoot",            cGuiEditCtrlSetRoot,      "editCtrl.setRoot(root)",             3, 3);
   Con::addCommand("GuiEditCtrl", "setCurrentAddSet",   cGuiEditCtrlSetAddSet,    "editCtrl.setCurrentAddSet(ctrl)",    3, 3);
   Con::addCommand("GuiEditCtrl", "toggle",             cGuiEditCtrlToggle,       "editCtrl.toggle()",                  2, 2);
   Con::addCommand("GuiEditCtrl", "justify",            cGuiEditCtrlJustify,      "editCtrl.justify(mode)",             3, 3);
   Con::addCommand("GuiEditCtrl", "bringToFront",       cGuiEditCtrlBringToFront, "editCtrl.bringToFront()",            2, 2);
   Con::addCommand("GuiEditCtrl", "pushToBack",         cGuiEditCtrlPushToBack,   "editCtrl.pushToBack()",              2, 2);
}

bool GuiEditCtrl::onWake()
{
   if (! Parent::onWake())
      return false;
   setEditMode(true);   
   return true;
}

void GuiEditCtrl::setRoot(GuiControl *root)
{
   mContentControl = root;
}

enum { GUI_BLACK = 0, GUI_WHITE = 255 };
enum { NUT_SIZE = 3 };

void GuiEditCtrl::setEditMode(bool value)
{
   mActive = value;
   mSelectedControls.clear();
   if (mActive && mAwake)
      mCurrentAddSet = NULL;
}

void GuiEditCtrl::setCurrentAddSet(GuiControl *ctrl)
{
   if (ctrl != mCurrentAddSet)
   {
      mSelectedControls.clear();
      mCurrentAddSet = ctrl;
   }
}

void GuiEditCtrl::setSelection(GuiControl *ctrl, bool inclusive)
{
   //sanity check
   if (! ctrl)
      return;
   
   // otherwise, we hit a new control...
   GuiControl *newAddSet = ctrl->getParent();
   
   //see if we should clear the old selection set
   if (newAddSet != mCurrentAddSet || (! inclusive))
      mSelectedControls.clear();
   
   //set the selection
   mCurrentAddSet = newAddSet;
   mSelectedControls.push_back(ctrl);
}

void GuiEditCtrl::addNewControl(GuiControl *ctrl)
{
   if (! mCurrentAddSet)
      mCurrentAddSet = mContentControl;

   mCurrentAddSet->addObject(ctrl);
   mSelectedControls.clear();
   mSelectedControls.push_back(ctrl);
}

void GuiEditCtrl::drawNut(const Point2I &nut, bool multisel)
{
   RectI r(nut.x - NUT_SIZE, nut.y - NUT_SIZE, 2 * NUT_SIZE + 1, 2 * NUT_SIZE + 1);
   dglDrawRect(r, multisel ? ColorI(0, 0, 0) : ColorI(255, 255, 255));
   r.point += Point2I(1, 1);
   r.extent -= Point2I(1, 1);
   dglDrawRectFill(r, multisel ? ColorI(255, 255, 255) : ColorI(0, 0, 0));
}

static inline bool inNut(const Point2I &pt, S32 x, S32 y)
{
   S32 dx = pt.x - x;
   S32 dy = pt.y - y;
   return dx <= NUT_SIZE && dx >= -NUT_SIZE && dy <= NUT_SIZE && dy >= -NUT_SIZE;
}

S32 GuiEditCtrl::getSizingHitKnobs(const Point2I &pt, const RectI &box)
{
   S32 lx = box.point.x, rx = box.point.x + box.extent.x - 1;
   S32 cx = (lx + rx) >> 1;
   S32 ty = box.point.y, by = box.point.y + box.extent.y - 1;
   S32 cy = (ty + by) >> 1;

   if (inNut(pt, lx, ty))
      return sizingLeft | sizingTop;
   if (inNut(pt, cx, ty))
      return sizingTop;
   if (inNut(pt, rx, ty))
      return sizingRight | sizingTop;
   if (inNut(pt, lx, by))
      return sizingLeft | sizingBottom;
   if (inNut(pt, cx, by))
      return sizingBottom;
   if (inNut(pt, rx, by))
      return sizingRight | sizingBottom;
   if (inNut(pt, lx, cy))
      return sizingLeft;
   if (inNut(pt, rx, cy))
      return sizingRight;
   return sizingNone;
}

void GuiEditCtrl::drawNuts(RectI &box, bool multisel)
{
   S32 lx = box.point.x, rx = box.point.x + box.extent.x - 1;
   S32 cx = (lx + rx) >> 1;
   S32 ty = box.point.y, by = box.point.y + box.extent.y - 1;
   S32 cy = (ty + by) >> 1;
   drawNut(Point2I(lx, ty), multisel);
   drawNut(Point2I(lx, cy), multisel);
   drawNut(Point2I(lx, by), multisel);
   drawNut(Point2I(rx, ty), multisel);
   drawNut(Point2I(rx, cy), multisel);
   drawNut(Point2I(rx, by), multisel);
   drawNut(Point2I(cx, ty), multisel);
   drawNut(Point2I(cx, by), multisel);
}

void GuiEditCtrl::getDragRect(RectI &box)
{
   box.point.x = getMin(mLastMousePos.x, mSelectionAnchor.x);
   box.extent.x = getMax(mLastMousePos.x, mSelectionAnchor.x) - box.point.x + 1;
   box.point.y = getMin(mLastMousePos.y, mSelectionAnchor.y);
   box.extent.y = getMax(mLastMousePos.y, mSelectionAnchor.y) - box.point.y + 1;
}

void GuiEditCtrl::onPreRender()
{
   setUpdate();
}

void GuiEditCtrl::onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder)
{
   Point2I ctOffset;
   Point2I cext;

   if (mActive)
   {
      if (mCurrentAddSet)
      {
         // draw a white frame inset around the current add set.
         cext = mCurrentAddSet->getExtent();
         ctOffset = mCurrentAddSet->localToGlobalCoord(Point2I(0,0));
         RectI box(ctOffset.x + 1,ctOffset.y + 1, cext.x - 2, cext.y - 2);
         dglDrawRect(box, ColorI(255, 255, 255));
         box.point -= Point2I(1, 1);
         box.extent += Point2I(1, 1);
         dglDrawRect(box, ColorI(0, 0, 0));
      }
      Vector<GuiControl *>::iterator i;
      bool multisel = mSelectedControls.size() > 1;
      for(i = mSelectedControls.begin(); i != mSelectedControls.end(); i++)
      {
         GuiControl *ctrl = (*i);
         cext = ctrl->getExtent();
         ctOffset = ctrl->localToGlobalCoord(Point2I(0,0));
         RectI box(ctOffset.x,ctOffset.y, cext.x, cext.y);
         drawNuts(box, multisel);
      }
      if (mMouseDownMode == DragSelecting)
      {
         RectI b;
         getDragRect(b);
         dglDrawRect(b, ColorI(255, 255, 255));
      }
   }
   
   renderChildControls(offset, updateRect, firstResponder);
}

bool GuiEditCtrl::selectionContains(GuiControl *ctrl)
{
   Vector<GuiControl *>::iterator i;
   for (i = mSelectedControls.begin(); i != mSelectedControls.end(); i++)
      if (ctrl == *i) return true;
   return false;
}

void GuiEditCtrl::onRightMouseDown(const GuiEvent &event)
{
   if (! mActive)
   {
      Parent::onRightMouseDown(event);
      return;
   }
   setFirstResponder();
      
   //search for the control hit in any layer below the edit layer
   GuiControl *hitCtrl = mContentControl->findHitControl(event.mousePoint, mLayer - 1);
   if (hitCtrl != mCurrentAddSet)
   {
      mSelectedControls.clear();
      mCurrentAddSet = hitCtrl;
   }
}
void GuiEditCtrl::select(GuiControl *ctrl)
{
   mSelectedControls.clear();
   if(ctrl != mContentControl)
      mSelectedControls.push_back(ctrl);
   else
      mCurrentAddSet = mContentControl;
}
   
void GuiEditCtrl::onMouseDown(const GuiEvent &event)
{
   if (! mActive)
   {
      Parent::onMouseDown(event);
      return;
   }
   if(!mContentControl)
      return;
      
   setFirstResponder();
   //lock the mouse   
   mouseLock();
   
   Point2I ctOffset;
   Point2I cext;
   GuiControl *ctrl;

   mLastMousePos = event.mousePoint;
   
   // first see if we hit a sizing knob on the currently selected control...
   if (mSelectedControls.size() == 1)
   {
      ctrl = mSelectedControls.first();
      cext = ctrl->getExtent();
      ctOffset = ctrl->localToGlobalCoord(Point2I(0,0));
      RectI box(ctOffset.x,ctOffset.y,cext.x, cext.y);

      if ((mSizingMode = (GuiEditCtrl::sizingModes)getSizingHitKnobs(event.mousePoint, box)) != 0)
      {
         mMouseDownMode = SizingSelection;
         return;
      }
   }

   if(!mCurrentAddSet)
      mCurrentAddSet = mContentControl;
   
   //find the control we clicked
   ctrl = mContentControl->findHitControl(event.mousePoint, mCurrentAddSet->mLayer);
   
   if (selectionContains(ctrl))
   {
      //if we're holding shift, de-select the clicked ctrl
      if (event.modifier & SI_SHIFT)
      {
         Vector<GuiControl *>::iterator i;
         for(i = mSelectedControls.begin(); i != mSelectedControls.end(); i++)
         {
            if (*i == ctrl)
            {
               mSelectedControls.erase(i);
               break;
            }
         }
         
         //set the mode
         mMouseDownMode = Selecting;
      }
      
      //else we hit a ctrl we've already selected, so set the mode to moving
      else
         mMouseDownMode = MovingSelection;
   }
   
   //else we clicked on an unselected control
   else
   {
      //if we clicked in the current add set
      if (ctrl == mCurrentAddSet)
      {
         // start dragging a rectangle
         // if the shift is not down, nuke prior selection
         if (!(event.modifier & SI_SHIFT))
            mSelectedControls.clear();
         mSelectionAnchor = event.mousePoint;
         mMouseDownMode = DragSelecting;
      }
      else
      {
         //find the new add set
         GuiControl *newAddSet = ctrl->getParent();
         
         //if we're holding shift and the ctrl is in the same add set
         if (event.modifier & SI_SHIFT && newAddSet == mCurrentAddSet)
         {
            mSelectedControls.push_back(ctrl);
            mMouseDownMode = Selecting;
         }
         else if (ctrl != mContentControl)
         {
            //find and set the new add set
            mCurrentAddSet = ctrl->getParent();
            
            //clear and set the selected controls
            mSelectedControls.clear();
            mSelectedControls.push_back(ctrl);
            mMouseDownMode = Selecting;
         }
         else
            mMouseDownMode = Selecting;
      }
   }
}

void GuiEditCtrl::onMouseUp(const GuiEvent &event)
{
   if (! mActive)
   {
      Parent::onMouseUp(event);
      return;
   }
   
   //unlock the mouse
   mouseUnlock();
      
   mLastMousePos = event.mousePoint;
   if (mMouseDownMode == DragSelecting)
   {
      RectI b;
      getDragRect(b);
      GuiControl::iterator i;
      for(i = mCurrentAddSet->begin(); i != mCurrentAddSet->end(); i++)
      {
         GuiControl *ctrl = dynamic_cast<GuiControl *>(*i);
         Point2I upperL = ctrl->localToGlobalCoord(Point2I(0,0));
         Point2I lowerR = upperL + ctrl->mBounds.extent - Point2I(1, 1);

         if (b.pointInRect(upperL) && b.pointInRect(lowerR) && !selectionContains(ctrl))
            mSelectedControls.push_back(ctrl);
      }
   }
   if (mSelectedControls.size() == 1)
      Con::executef(this, 2, "onSelect", avar("%d", mSelectedControls[0]->getId()));
      
   setFirstResponder();
   //reset the mouse mode
   mMouseDownMode = Selecting;
}

void GuiEditCtrl::onMouseDragged(const GuiEvent &event)
{
   if (! mActive)
   {
      Parent::onMouseDragged(event);
      return;
   }
      
   if(!mCurrentAddSet)
      mCurrentAddSet = mContentControl;

   Point2I mousePoint = event.mousePoint;

   if (mMouseDownMode == SizingSelection)
   {
      if (mGridSnap.x)
         mousePoint.x -= mousePoint.x % mGridSnap.x;
      if (mGridSnap.y)
         mousePoint.y -= mousePoint.y % mGridSnap.y;

      GuiControl *ctrl = mSelectedControls.first();
      Point2I ctrlPoint = mCurrentAddSet->globalToLocalCoord(mousePoint);
      Point2I newPosition = ctrl->getPosition();
      Point2I newExtent = ctrl->getExtent();
      Point2I minExtent = ctrl->getMinExtent();
      
      if (mSizingMode & sizingLeft)
      {
         newPosition.x = ctrlPoint.x;
         newExtent.x = ctrl->mBounds.extent.x + ctrl->mBounds.point.x - ctrlPoint.x;
         if(newExtent.x < minExtent.x)
         {
            newPosition.x -= minExtent.x - newExtent.x;
            newExtent.x = minExtent.x;
         }
      }
      else if (mSizingMode & sizingRight)
      {
         newExtent.x = ctrlPoint.x - ctrl->mBounds.point.x;
         if(newExtent.x < minExtent.x)
            newExtent.x = minExtent.x;
      }   
      
      if (mSizingMode & sizingTop)
      {
         newPosition.y = ctrlPoint.y;
         newExtent.y = ctrl->mBounds.extent.y + ctrl->mBounds.point.y - ctrlPoint.y;
         if(newExtent.y < minExtent.y)
         {
            newPosition.y -= minExtent.y - newExtent.y;
            newExtent.y = minExtent.y;
         }
      }
      else if (mSizingMode & sizingBottom)
      {
         newExtent.y = ctrlPoint.y - ctrl->mBounds.point.y;
         if(newExtent.y < minExtent.y)
            newExtent.y = minExtent.y;
      }
      
      ctrl->resize(newPosition, newExtent);
      mCurrentAddSet->childResized(ctrl);
   } 
   else if (mMouseDownMode == MovingSelection && mSelectedControls.size())
   {
      Vector<GuiControl *>::iterator i = mSelectedControls.begin();
      Point2I minPos = (*i)->mBounds.point;
      for(; i != mSelectedControls.end(); i++)
      {
         if ((*i)->mBounds.point.x < minPos.x)
            minPos.x = (*i)->mBounds.point.x;
         if ((*i)->mBounds.point.y < minPos.y)
            minPos.y = (*i)->mBounds.point.y;
      }
      Point2I delta = mousePoint - mLastMousePos;
      delta += minPos; // find new minPos;

      if (mGridSnap.x)
         delta.x -= delta.x % mGridSnap.x;
      if (mGridSnap.y)
         delta.y -= delta.y % mGridSnap.y;

      delta -= minPos;
      moveSelection(delta);
      mLastMousePos += delta;
   }
   else
      mLastMousePos = mousePoint;
}

void GuiEditCtrl::moveSelection(const Point2I &delta)
{
   Vector<GuiControl *>::iterator i;

   for(i = mSelectedControls.begin(); i != mSelectedControls.end(); i++)
      (*i)->resize((*i)->mBounds.point + delta, (*i)->mBounds.extent);
}

void GuiEditCtrl::justifySelection(Justification j)
{
   S32 minX, maxX;
   S32 minY, maxY;
   S32 extentX, extentY;

   if (mSelectedControls.size() < 2)
      return;

   Vector<GuiControl *>::iterator i = mSelectedControls.begin();
   minX = (*i)->mBounds.point.x;
   maxX = minX + (*i)->mBounds.extent.x;
   minY = (*i)->mBounds.point.y;
   maxY = minY + (*i)->mBounds.extent.y;
   extentX = (*i)->mBounds.extent.x;
   extentY = (*i)->mBounds.extent.y;
   i++;
   for(;i != mSelectedControls.end(); i++)
   {
      minX = getMin(minX, (*i)->mBounds.point.x);
      maxX = getMax(maxX, (*i)->mBounds.point.x + (*i)->mBounds.extent.x);
      minY = getMin(minY, (*i)->mBounds.point.y);
      maxY = getMax(maxY, (*i)->mBounds.point.y + (*i)->mBounds.extent.y);
      extentX += (*i)->mBounds.extent.x;
      extentY += (*i)->mBounds.extent.y;
   }
   S32 deltaX = maxX - minX;
   S32 deltaY = maxY - minY;
   switch(j)
   {
      case JUSTIFY_LEFT:
         for(i = mSelectedControls.begin(); i != mSelectedControls.end(); i++)
            (*i)->resize(Point2I(minX, (*i)->mBounds.point.y), (*i)->mBounds.extent);
         break;
      case JUSTIFY_TOP:
         for(i = mSelectedControls.begin(); i != mSelectedControls.end(); i++)
            (*i)->resize(Point2I((*i)->mBounds.point.x, minY), (*i)->mBounds.extent);
         break;
      case JUSTIFY_RIGHT:
         for(i = mSelectedControls.begin(); i != mSelectedControls.end(); i++)
            (*i)->resize(Point2I(maxX - (*i)->mBounds.extent.x + 1, (*i)->mBounds.point.y), (*i)->mBounds.extent);
         break;
      case JUSTIFY_BOTTOM:
         for(i = mSelectedControls.begin(); i != mSelectedControls.end(); i++)
            (*i)->resize(Point2I((*i)->mBounds.point.x, maxY - (*i)->mBounds.extent.y + 1), (*i)->mBounds.extent);
         break;
      case JUSTIFY_CENTER:
         for(i = mSelectedControls.begin(); i != mSelectedControls.end(); i++)
            (*i)->resize(Point2I(minX + ((deltaX - (*i)->mBounds.extent.x) >> 1), (*i)->mBounds.point.y),
                                                                                 (*i)->mBounds.extent);
         break;
      case SPACING_VERTICAL:
         {
            Vector<GuiControl *> sortedList;
            Vector<GuiControl *>::iterator k;
            for(i = mSelectedControls.begin(); i != mSelectedControls.end(); i++)
            {
               for(k = sortedList.begin(); k != sortedList.end(); k++)
               {
                  if ((*i)->mBounds.point.y < (*k)->mBounds.point.y)
                     break;
               }
               sortedList.insert(k, *i);
            }
            S32 space = (deltaY - extentY) / (mSelectedControls.size() - 1);
            S32 curY = minY;
            for(k = sortedList.begin(); k != sortedList.end(); k++)
            {
               (*k)->resize(Point2I((*k)->mBounds.point.x, curY), (*k)->mBounds.extent); 
               curY += (*k)->mBounds.extent.y + space;
            }
         }
         break;
      case SPACING_HORIZONTAL:
         {
            Vector<GuiControl *> sortedList;
            Vector<GuiControl *>::iterator k;
            for(i = mSelectedControls.begin(); i != mSelectedControls.end(); i++)
            {
               for(k = sortedList.begin(); k != sortedList.end(); k++)
               {
                  if ((*i)->mBounds.point.x < (*k)->mBounds.point.x)
                     break;
               }
               sortedList.insert(k, *i);
            }
            S32 space = (deltaX - extentX) / (mSelectedControls.size() - 1);
            S32 curX = minX;
            for(k = sortedList.begin(); k != sortedList.end(); k++)
            {
               (*k)->resize(Point2I(curX, (*k)->mBounds.point.y), (*k)->mBounds.extent); 
               curX += (*k)->mBounds.extent.x + space;
            }
         }
         break;
   }
}

void GuiEditCtrl::deleteSelection(void)
{

   Vector<GuiControl *>::iterator i;
   for(i = mSelectedControls.begin(); i != mSelectedControls.end(); i++)
      (*i)->deleteObject();
   mSelectedControls.clear();
}

void GuiEditCtrl::loadSelection(const char* filename)
{
   if (! mCurrentAddSet)
      mCurrentAddSet = mContentControl;

   Con::executef(2, "exec", filename);
   SimSet *set;
   if(!Sim::findObject("guiClipboard", set))
      return;
      
   if(set->size())
   {
      mSelectedControls.clear();
      for(U32 i = 0; i < set->size(); i++)
      {
         GuiControl *ctrl = dynamic_cast<GuiControl *>((*set)[i]);
         if(ctrl)
         {
            mCurrentAddSet->addObject(ctrl);
            mSelectedControls.push_back(ctrl);
         }
      }
   }
   set->deleteObject();
}

void GuiEditCtrl::saveSelection(const char* filename)
{
   FileStream stream;
   if(!ResourceManager->openFileForWrite(stream, NULL, filename))
      return;
   SimSet *clipboardSet = new SimSet;
   clipboardSet->registerObject();
   Sim::getRootGroup()->addObject(clipboardSet, "guiClipboard");
   
   Vector<GuiControl *>::iterator i;
   for(i = mSelectedControls.begin(); i != mSelectedControls.end(); i++)
      clipboardSet->addObject(*i);
   
   clipboardSet->write(stream, 0);
   clipboardSet->deleteObject();
}

void GuiEditCtrl::selectAll()
{
   GuiControl::iterator i;
   if (!mCurrentAddSet)
      return;
   mSelectedControls.clear();
   for(i = mCurrentAddSet->begin(); i != mCurrentAddSet->end(); i++)
   {
      GuiControl *ctrl = dynamic_cast<GuiControl *>(*i);
      mSelectedControls.push_back(ctrl);
   }
}

void GuiEditCtrl::bringToFront()
{
   if (mSelectedControls.size() != 1)
      return;
      
   GuiControl *ctrl = *(mSelectedControls.begin());
   mCurrentAddSet->bringObjectToFront(ctrl);
}

void GuiEditCtrl::pushToBack()
{
   if (mSelectedControls.size() != 1)
      return;
      
   GuiControl *ctrl = *(mSelectedControls.begin());
   mCurrentAddSet->pushObjectToBack(ctrl);
}

bool GuiEditCtrl::onKeyDown(const GuiEvent &event)
{
   if (! mActive)
      return Parent::onKeyDown(event);

   if (event.modifier & SI_CTRL)
   {
      switch(event.keyCode)
      {
         case KEY_A:
            selectAll();
            break;
         case KEY_C:
            saveSelection("gui/clipboard.gui");
            break;
         case KEY_X:
            saveSelection("gui/clipboard.gui");
            deleteSelection();
            break;
         case KEY_V:
            loadSelection("gui/clipboard.gui");
            break;
      }
   }
   else
   {
      S32 delta = (event.modifier & SI_SHIFT) ? 10 : 1;

      switch(event.keyCode)
      {
         case KEY_RIGHT:
            moveSelection(Point2I(delta, 0));
            break;
         case KEY_LEFT:
            moveSelection(Point2I(-delta, 0));
            break;
         case KEY_UP:
            moveSelection(Point2I(0, -delta));
            break;
         case KEY_DOWN:
            moveSelection(Point2I(0, delta));
            break;
         case KEY_BACKSPACE:
         case KEY_DELETE:
            deleteSelection();
            break;
      }
   }
   
   return false;
}
