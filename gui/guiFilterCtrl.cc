//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "console/console.h"
#include "console/consoleTypes.h"
#include "dgl/dgl.h"
#include "dgl/gTexManager.h"
#include "GUI/guiFilterCtrl.h"
#include "Platform/event.h"
#include "Math/mMath.h"


GuiFilterCtrl::GuiFilterCtrl()
{
   mControlPointRequest = 7;
   mFilter.setSize(7);
   identity();
}


void GuiFilterCtrl::initPersistFields()
{
   Parent::initPersistFields();
   addField("controlPoints", TypeS32, Offset(mControlPointRequest, GuiFilterCtrl));
   addField("filter", TypeF32Vector, Offset(mFilter, GuiFilterCtrl));
}

static const char* cGuiFilter_GetValue(SimObject *obj, S32, const char **argv)
{
   argv;
   static char buffer[512];
   const Filter *filter = static_cast<GuiFilterCtrl*>(obj)->get();
   *buffer = 0;

   for (U32 i=0; i < filter->size(); i++)
   {
      char value[32];
      dSprintf(value, 31, "%1.5f ", *(filter->begin()+i) );
      dStrcat(buffer, value);
   }

   return buffer;
}

static void cGuiFilter_SetValue(SimObject *obj, S32 argc, const char **argv)
{
   GuiFilterCtrl *ctrl = static_cast<GuiFilterCtrl*>(obj);
   Filter filter;

   argc -= 2;
   argv += 2;
   
   filter.set(argc, argv);
	ctrl->set(filter);
}

static void cGuiFilter_Identity(SimObject *obj, S32, const char **)
{
   GuiFilterCtrl *ctrl = static_cast<GuiFilterCtrl*>(obj);
   ctrl->identity();
}

void GuiFilterCtrl::consoleInit()
{
   Con::addCommand("GuiFilterCtrl", "getValue",  cGuiFilter_GetValue,   "guiFilterCtrl.getValue()",    2, 2);
   Con::addCommand("GuiFilterCtrl", "setValue",  cGuiFilter_SetValue,   "guiFilterCtrl.setValue(f1, f2, ...)", 3, 20);
   Con::addCommand("GuiFilterCtrl", "identity",  cGuiFilter_Identity,   "guiFilterCtrl.identity()",     2, 2);
}


bool GuiFilterCtrl::onWake()
{
   if (!Parent::onWake())
      return false;
   
   if (U32(mControlPointRequest) != mFilter.size())
   {
      mFilter.setSize(mControlPointRequest);
      identity();
   }

   return true;
}


void GuiFilterCtrl::identity()
{
   S32 size = mFilter.size()-1;
   for (U32 i=0; S32(i) <= size; i++)
      mFilter[i] = (F32)i/(F32)size;
}


void GuiFilterCtrl::onMouseDown(const GuiEvent &event)
{
   mouseLock();
   setFirstResponder();
      
   Point2I p = globalToLocalCoord(event.mousePoint);

   // determine which knot (offset same as in onRender)
   F32 w = F32(mBounds.extent.x-4) / F32(mFilter.size()-1);
   F32 val = (F32(p.x) + (w / 2.f)) / w;
   mCurKnot = S32(val);

   mFilter[mCurKnot] = 1.0f - F32(getMin(getMax(0, p.y), mBounds.extent.y)/(F32)mBounds.extent.y);
   setUpdate();
}   


void GuiFilterCtrl::onMouseDragged(const GuiEvent &event)
{
   mouseLock();   
   setFirstResponder();

   Point2I p = globalToLocalCoord(event.mousePoint);
   mFilter[mCurKnot] = 1.0f - F32(getMin(getMax(0, p.y), mBounds.extent.y)/(F32)mBounds.extent.y);
   setUpdate();
}   

void GuiFilterCtrl::onMouseUp(const GuiEvent &)
{
   mouseUnlock();
   if (mConsoleCommand[0])
      Con::evaluate(mConsoleCommand, false);
}   

void GuiFilterCtrl::onPreRender()
{
   if(U32(mControlPointRequest) != mFilter.size())
   {
      mFilter.setSize(mControlPointRequest);
      identity();
      setUpdate();
   }
}

void GuiFilterCtrl::onRender(Point2I offset, const RectI &updateRect, GuiControl *firstResponder)
{
   Point2I pos = offset; 
   Point2I ext = mBounds.extent;

   RectI r(pos, ext);
   dglDrawRectFill(r, ColorI(255,255,255));
   dglDrawRect(r, ColorI(0,0,0));

   // shrink by 2 pixels
   pos.x += 2;
   pos.y += 2;
   ext.x -= 4;
   ext.y -= 4;

   // draw the identity line
   glColor3f(0.9, 0.9, 0.9);
   glBegin(GL_LINES);
      glVertex2i(pos.x, pos.y+ext.y);
      glVertex2i(pos.x+ext.x, pos.y);
   glEnd();

   // draw the curv
   glColor3f(0.4, 0.4, 0.4);
   glBegin(GL_LINE_STRIP);
      
      F32 scale = 1.0f/F32(ext.x);
      for (U32 i=0; S32(i) < ext.x; i++)
      {
         F32 index = F32(i)*scale;
         S32 y = (S32)(ext.y*(1.0f-mFilter.getValue(index)));
         glVertex2i(pos.x+i, pos.y+y );      
      }
   glEnd();

   // draw the knots
   for (U32 k=0; k < mFilter.size(); k++)
   {
      RectI r;
      r.point.x = (S32)(((F32)ext.x/(F32)(mFilter.size()-1)*(F32)k));
      r.point.y = (S32)(ext.y - ((F32)ext.y * mFilter[k]));
      r.point += pos + Point2I(-2,-2);
      r.extent = Point2I(5,5);
      
      dglDrawRectFill(r, ColorI(255,0,0));
   }

   renderChildControls(offset, updateRect, firstResponder);
}



//--------------------------------------
void Filter::set(S32 argc, const char *argv[])
{
   setSize(0);
   if (argc == 1)
   {  // in the form of one string "1.0 1.0 1.0"
      char list[1024];
      dStrcpy(list, *argv);    // strtok modifies the string so we need to copy it
      char *value = dStrtok(list, " ");
      while (value)
      {
         push_back(dAtof(value));
         value = dStrtok(NULL, " ");
      }
   }
   else
   {  // in the form of seperate strings "1.0" "1.0" "1.0"
      for (; argc ; argc--, argv++)
         push_back(dAtof(*argv));
   }
}   


//--------------------------------------
F32 Filter::getValue(F32 x) const
{
   if (size() < 2)
      return 0.0f;

   x = mClampF(x, 0.0f, 1.0f);
   x *= F32(size()-1);

   F32 p0,p1,p2,p3;
   S32 i1 = (S32)mFloor(x);
   S32 i2 = i1+1;
   F32 dt = x - F32(i1);

   p1 = *(begin()+i1);   
   p2 = *(begin()+i2);   

   if (i1 == 0)
      p0 = p1 + (p1 - p2);
   else
      p0 = *(begin()+i1-1);

   if (i2 == S32(size()-1))
      p3 = p2 + (p2 - p1);
   else
      p3 = *(begin()+i2+1);

   return mClampF( mCatmullrom(dt, p0, p1, p2, p3), 0.0f, 1.0f );
}   






