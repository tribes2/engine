//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "editor/terrainEditor.h"
#include "game/collisionTest.h"
#include "terrain/terrData.h"
#include "gui/guiCanvas.h"
#include "console/consoleTypes.h"
#include "editor/terrainActions.h"
#include "interior/interiorInstance.h"
#include "interior/interior.h"
#include "game/gameConnection.h"
#include "sim/netObject.h"
#include "sim/frameAllocator.h"

IMPLEMENT_CONOBJECT(TerrainEditor);

Selection::Selection() :
   Vector<GridInfo>(__FILE__, __LINE__),
   mName(0),
   mUndoFlags(0),
   mHashListSize(1024)
{
   VECTOR_SET_ASSOCIATION(mHashLists);

   // clear the hash list
   mHashLists.setSize(mHashListSize);
   reset();
}

void Selection::reset()
{
   for(U32 i = 0; i < mHashListSize; i++)
      mHashLists[i] = -1;
   clear();
}

U32 Selection::getHashIndex(const Point2I & pos)
{
   Point2F pnt = Point2F(pos.x, pos.y) + Point2F(1.3f,3.5f);
   return( (U32)(mFloor(mHashLists.size() * mFmod(pnt.len() * 0.618f, 1))) );
}

S32 Selection::lookup(const Point2I & pos)
{
   U32 index = getHashIndex(pos);

   S32 entry = mHashLists[index];

   while(entry != -1)
   {
      if((*this)[entry].mGridPos == pos)
         return(entry);

      entry = (*this)[entry].mNext;
   }

   return(-1);
}

void Selection::insert(GridInfo & info)
{
   U32 index = getHashIndex(info.mGridPos);

   info.mNext = mHashLists[index];
   info.mPrev = -1;

   if(info.mNext != -1)
      (*this)[info.mNext].mPrev = size();
   
   mHashLists[index] = size();

   push_back(info);
}

bool Selection::remove(const GridInfo & info)
{
   U32 index = getHashIndex(info.mGridPos);

   S32 entry = mHashLists[index];

   if(entry == -1)
      return(false);
   
   // front?
   if((*this)[entry].mGridPos == info.mGridPos)
      mHashLists[index] = (*this)[entry].mNext;
      
   while(entry != -1)
   {
      if((*this)[entry].mGridPos == info.mGridPos)
      {
         if((*this)[entry].mPrev != -1)
            (*this)[(*this)[entry].mPrev].mNext = (*this)[entry].mNext;
         if((*this)[entry].mNext != -1)
            (*this)[(*this)[entry].mNext].mPrev = (*this)[entry].mPrev;

         // swap?
         if(entry != (size() - 1))
         {
            U32 last = size() - 1;

            (*this)[entry] = (*this)[size()-1];

            if((*this)[entry].mPrev != -1)
               (*this)[(*this)[entry].mPrev].mNext = entry;
            else
            {
               U32 idx = getHashIndex((*this)[entry].mGridPos);
               AssertFatal(mHashLists[idx] == ((*this).size() - 1), "doh");
               mHashLists[idx] = entry;
            }               
            if((*this)[entry].mNext != -1)
               (*this)[(*this)[entry].mNext].mPrev = entry;
         }

         pop_back();
         return(true);
      }

      entry = (*this)[entry].mNext;
   }
   return(false);
}

// add unique grid info into the selection - test uniqueness by grid position
bool Selection::add(GridInfo & info)
{
   S32 index = lookup(info.mGridPos);
   if(index != -1)
      return(false);

   insert(info);
   return(true);
}

bool Selection::getInfo(Point2I pos, GridInfo & info)
{
   S32 index = lookup(pos);
   if(index == -1)
      return(false);

   info = (*this)[index];
   return(true);
}

bool Selection::setInfo(GridInfo & info)
{
   S32 index = lookup(info.mGridPos);
   if(index == -1)
      return(false);

   S32 next = (*this)[index].mNext;
   S32 prev = (*this)[index].mPrev;
      
   (*this)[index] = info;
   (*this)[index].mNext = next;
   (*this)[index].mPrev = prev;

   return(true);
}

F32 Selection::getAvgHeight()
{
   if(!size())
      return(0);

   F32 avg = 0.f;
   for(U32 i = 0; i < size(); i++)
      avg += (*this)[i].mHeight;

   return(avg / size());
}

//------------------------------------------------------------------------------

Brush::Brush(TerrainEditor * editor) :
   mTerrainEditor(editor)
{
   mSize = mTerrainEditor->getBrushSize();
}

const Point2I & Brush::getPosition()
{
   return(mGridPos);
}

void Brush::setPosition(const Point3F & pos)
{
   Point2I gPos;
   mTerrainEditor->worldToGrid(pos, gPos);
   setPosition(gPos);
}

void Brush::setPosition(const Point2I & pos)
{
   mGridPos = pos;
   update();
}

//------------------------------------------------------------------------------

void Brush::update()
{
   rebuild();

   // soft selection?
   if(mTerrainEditor->mEnableSoftBrushes)
   {
      Gui3DMouseEvent event;
      TerrainAction * action = mTerrainEditor->lookupAction("softSelect");
      AssertFatal(action, "Brush::update: no 'softSelect' action found!");

      //
      mTerrainEditor->setCurrentSel(this);
      action->process(this, event, true, TerrainAction::Process);
      mTerrainEditor->resetCurrentSel();
   }
}

//------------------------------------------------------------------------------

void BoxBrush::rebuild()
{
   reset();
   
   //
   for(U32 x = 0; x < mSize.x; x++)
      for(U32 y = 0; y < mSize.y; y++)
      {
         GridInfo info;
         mTerrainEditor->getGridInfo(Point2I(mGridPos.x + x - (mSize.x / 2), mGridPos.y + y - (mSize.y / 2)), info);
         push_back(info);
      }
}

//------------------------------------------------------------------------------

void EllipseBrush::rebuild()
{
   reset();
   Point3F center(F32(mSize.x) / 2, F32(mSize.y) / 2, 0);
   
   for(U32 x = 0; x < mSize.x; x++)
      for(U32 y = 0; y < mSize.y; y++)
      {
         F32 a = mSize.x >= mSize.y ? F32(mSize.x) / 2 : F32(mSize.y) / 2;
         F32 b = mSize.x >= mSize.y ? F32(mSize.y) / 2 : F32(mSize.x) / 2;
         Point3F dir(mSize.x >= mSize.y ? 1 : 0, mSize.x >= mSize.y ? 0 : 1, 0);
         
         // first do quick check on minor 
         Point3F pos(F32(x) + 0.5, F32(y) + 0.5, 0);
         Point3F vec = pos - center;
         F32 len = vec.len();

         bool addPoint = false;
         if(len <= b)
            addPoint = true;
         else
         {
            F32 theta = mAcos(mDot(vec, dir));
            F32 as = a*a;
            F32 bs = b*b;
            
            F32 r = mSqrt((bs * as) / ((bs * (1+2*mCos(theta))) + (as * (1-2*mCos(theta)))));
            if(len <= r)
               addPoint = true;
         }
         
         if(addPoint)
         {
            GridInfo info;
            mTerrainEditor->getGridInfo(Point2I(mGridPos.x + x, mGridPos.y + y), info);
            push_back(info);
         }
      }
}

//------------------------------------------------------------------------------

SelectionBrush::SelectionBrush(TerrainEditor * editor) :
   Brush(editor)
{
   //... grab the current selection
}

void SelectionBrush::rebuild()
{
   reset();
   //... move the selection
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

TerrainEditor::TerrainEditor() :
   mTerrainBlock(0),
   mMousePos(0,0,0),
   mMouseBrush(0),
   mInAction(false),
   mUndoLimit(20),
   mUndoSel(0),
   mRebuildEmpty(false),
   mRebuildTextures(false)
{
   VECTOR_SET_ASSOCIATION(mActions);
   VECTOR_SET_ASSOCIATION(mUndoList);
   VECTOR_SET_ASSOCIATION(mRedoList);
   VECTOR_SET_ASSOCIATION(mBaseMaterialInfos);

   //
   resetCurrentSel();   

   //
   mBrushSize.set(2,2);
   mMouseBrush = new BoxBrush(this);
   
   // add in all the actions here..
   mActions.push_back(new SelectAction(this));
   mActions.push_back(new SoftSelectAction(this));
   mActions.push_back(new OutlineSelectAction(this));
   mActions.push_back(new RaiseHeightAction(this));
   mActions.push_back(new LowerHeightAction(this));
   mActions.push_back(new SetHeightAction(this));
   mActions.push_back(new SetEmptyAction(this));
   mActions.push_back(new ClearEmptyAction(this));
   mActions.push_back(new ScaleHeightAction(this));
   mActions.push_back(new BrushAdjustHeightAction(this));
   mActions.push_back(new AdjustHeightAction(this));
   mActions.push_back(new FlattenHeightAction(this));
   mActions.push_back(new SmoothHeightAction(this));
   mActions.push_back(new SetMaterialGroupAction(this));
   mActions.push_back(new SetModifiedAction(this));
   mActions.push_back(new ClearModifiedAction(this));
   
   // set the default action
   mCurrentAction = mActions[0];
   mRenderBrush = mCurrentAction->useMouseBrush();

   // persist data defaults
   mRenderBorder = true;
   mBorderHeight = 10;
   mBorderFillColor.set(0,255,0,20);
   mBorderFrameColor.set(0,255,0,128);
   mBorderLineMode = false;
   mSelectionHidden = false;
   mEnableSoftBrushes = false;
   mRenderVertexSelection = false;
   mProcessUsesBrush = false;

   //
   mAdjustHeightVal = 10;
   mSetHeightVal = 100;
   mScaleVal = 1;
   mSmoothFactor = 0.1f;
   mMaterialGroup = 0;
   mSoftSelectRadius = 50.f;
   mAdjustHeightMouseScale = 0.1f;

   mSoftSelectDefaultFilter = StringTable->insert("1.000000 0.833333 0.666667 0.500000 0.333333 0.166667 0.000000");
   mSoftSelectFilter = mSoftSelectDefaultFilter;;
}

TerrainEditor::~TerrainEditor()
{
   // mouse
   delete mMouseBrush;
   
   // terrain actions
   U32 i;
   for(i = 0; i < mActions.size(); i++)
      delete mActions[i];
      
   // undo stuff
   clearUndo(mUndoList);
   clearUndo(mRedoList);
   delete mUndoSel;

   // base material infos
   for(i = 0; i < mBaseMaterialInfos.size(); i++)
      delete mBaseMaterialInfos[i];
}

//------------------------------------------------------------------------------

TerrainAction * TerrainEditor::lookupAction(const char * name)
{
   for(U32 i = 0; i < mActions.size(); i++)
      if(!dStricmp(mActions[i]->getName(), name))
         return(mActions[i]);
   return(0);
}

//------------------------------------------------------------------------------

bool TerrainEditor::onAdd()
{
   if(!Parent::onAdd())
      return(false);
   
   SimObject * obj = Sim::findObject("Editor_ArrowCursor");
   if(!obj)
   {
      Con::errorf(ConsoleLogEntry::General, "TerrainEditor::onAdd: failed to load cursor");
      return(false);
   }
   
   mDefaultCursor = dynamic_cast<GuiCursor*>(obj);

   return(true);
}

//------------------------------------------------------------------------------

void TerrainEditor::onDeleteNotify(SimObject * object)
{
   Parent::onDeleteNotify(object);
   
   if(mTerrainBlock != dynamic_cast<TerrainBlock*>(object))
      return;
  
   mTerrainBlock = 0;    
}

void TerrainEditor::setCursor(GuiCursor * cursor)
{
   Canvas->setCursor(cursor ? cursor : mDefaultCursor);
}

//------------------------------------------------------------------------------

TerrainBlock * TerrainEditor::getClientTerrain()
{
   // do the client..

   NetConnection * toServer = NetConnection::getServerConnection();
   NetConnection * toClient = NetConnection::getLocalClientConnection();

   S32 index = toClient->getGhostIndex(mTerrainBlock);

   return(dynamic_cast<TerrainBlock*>(toServer->resolveGhost(index)));
}

//------------------------------------------------------------------------------

bool TerrainEditor::gridToWorld(const Point2I & gPos, Point3F & wPos)
{
   const MatrixF & mat = mTerrainBlock->getTransform();
   Point3F origin;
   mat.getColumn(3, &origin);

   wPos.x = gPos.x * (float)mTerrainBlock->getSquareSize() + origin.x;
   wPos.y = gPos.y * (float)mTerrainBlock->getSquareSize() + origin.y;
   wPos.z = getGridHeight(gPos);
      
   return(!(gPos.x >> TerrainBlock::BlockShift || gPos.y >> TerrainBlock::BlockShift));
}

bool TerrainEditor::worldToGrid(const Point3F & wPos, Point2I & gPos)
{
   const MatrixF & mat = mTerrainBlock->getTransform();
   Point3F origin;
   mat.getColumn(3, &origin);

   float x = (wPos.x - origin.x) / (float)mTerrainBlock->getSquareSize();
   float y = (wPos.y - origin.y) / (float)mTerrainBlock->getSquareSize();
   
   gPos.x = (S32)mFloor(x);
   gPos.y = (S32)mFloor(y);
   
   return(!(gPos.x >> TerrainBlock::BlockShift || gPos.y >> TerrainBlock::BlockShift));
}

bool TerrainEditor::gridToCenter(const Point2I & gPos, Point2I & cPos)
{
   cPos.x = gPos.x & TerrainBlock::BlockMask;
   cPos.y = gPos.y & TerrainBlock::BlockMask;
   
   return(!(gPos.x >> TerrainBlock::BlockShift || gPos.y >> TerrainBlock::BlockShift));
}

//------------------------------------------------------------------------------

bool TerrainEditor::getGridInfo(const Point3F & wPos, GridInfo & info)
{
   Point2I gPos;
   bool center = worldToGrid(wPos, gPos);
   
   //
   info.mGridPos = gPos;
   info.mMaterial = getGridMaterial(gPos);
   info.mHeight = getGridHeight(gPos);
   info.mMaterialGroup = getGridMaterialGroup(gPos);
   info.mWeight = 1.f;
   info.mPrimarySelect = true;
   
   return(center);
}

bool TerrainEditor::getGridInfo(const Point2I & gPos, GridInfo & info)
{
   //
   info.mGridPos = gPos;
   info.mMaterial = getGridMaterial(gPos);
   info.mHeight = getGridHeight(gPos);
   info.mMaterialGroup = getGridMaterialGroup(gPos);
   info.mWeight = 1.f;
   info.mPrimarySelect = true;
   
   return(!(gPos.x >> TerrainBlock::BlockShift || gPos.y >> TerrainBlock::BlockShift));
}

void TerrainEditor::setGridInfo(const GridInfo & info)
{
   setGridHeight(info.mGridPos, info.mHeight);
   setGridMaterial(info.mGridPos, info.mMaterial);
   setGridMaterialGroup(info.mGridPos, info.mMaterialGroup);
}

//------------------------------------------------------------------------------

F32 TerrainEditor::getGridHeight(const Point2I & gPos)
{
   Point2I cPos;
   gridToCenter(gPos, cPos);
   return(fixedToFloat(mTerrainBlock->getHeight(cPos.x, cPos.y)));
}

void TerrainEditor::setGridHeight(const Point2I & gPos, const F32 height)
{
   Point2I cPos;
   gridToCenter(gPos, cPos);
   mTerrainBlock->setHeight(cPos, height);
}

TerrainBlock::Material TerrainEditor::getGridMaterial(const Point2I & gPos)
{
   Point2I cPos;
   gridToCenter(gPos, cPos);
   return(*mTerrainBlock->getMaterial(cPos.x, cPos.y));   
}

void TerrainEditor::setGridMaterial(const Point2I & gPos, const TerrainBlock::Material & material)
{
   Point2I cPos;
   gridToCenter(gPos, cPos);

   // check if empty has been altered...
   TerrainBlock::Material * mat = mTerrainBlock->getMaterial(cPos.x, cPos.y);

   if((mat->flags & TerrainBlock::Material::Empty) ^ (material.flags & TerrainBlock::Material::Empty))
      mRebuildEmpty = true;

   *mat = material;
}

U8 TerrainEditor::getGridMaterialGroup(const Point2I & gPos)
{
   Point2I cPos;
   gridToCenter(gPos, cPos);

   return(mTerrainBlock->getBaseMaterial(cPos.x, cPos.y));
}

// basematerials are shared through a resource... so work on client object
// so wont need to load textures....
   
void TerrainEditor::setGridMaterialGroup(const Point2I & gPos, const U8 group)
{
   Point2I cPos;
   gridToCenter(gPos, cPos);

   TerrainBlock * clientTerrain = getClientTerrain();

   clientTerrain->setBaseMaterial(cPos.x, cPos.y, group);
}

//------------------------------------------------------------------------------

bool TerrainEditor::collide(const Gui3DMouseEvent & event, Point3F & pos)
{
   if(!mTerrainBlock)
      return(false);
      
   // call the terrain block's ray collision routine directly
   Point3F startPnt = event.pos;
   Point3F endPnt = event.pos + event.vec * 1000;
   Point3F tStartPnt, tEndPnt;

   mTerrainBlock->getTransform().mulP(startPnt, &tStartPnt);
   mTerrainBlock->getTransform().mulP(endPnt, &tEndPnt);
   
   RayInfo ri;
   if(mTerrainBlock->castRayI(tStartPnt, tEndPnt, &ri, true))
   {
      ri.point.interpolate(startPnt, endPnt, ri.t);
      pos = ri.point;
      return(true);
   }
   return(false);
}

//------------------------------------------------------------------------------

void TerrainEditor::updateGuiInfo()
{
   char buf[128];

   // mouse num grids 
   // mouse avg height 
   // selection num grids
   // selection avg height 
   dSprintf(buf, sizeof(buf), "%d %f %d %f", 
      mMouseBrush->size(), mMouseBrush->getAvgHeight(), 
      mDefaultSel.size(), mDefaultSel.getAvgHeight());
   Con::executef(this, 2, "onGuiUpdate", buf);
}

//------------------------------------------------------------------------------

void TerrainEditor::renderScene(const RectI &)
{
   if(!mTerrainBlock)
      return;

   if(!mSelectionHidden)
      renderSelection(mDefaultSel, ColorF(1,0,0), ColorF(0,1,0), ColorF(0,0,1), ColorF(0,0,1), true, false);
   
   if(mRenderBrush)   
      renderSelection(*mMouseBrush, ColorF(1,0,0), ColorF(0,1,0), ColorF(0,0,1), ColorF(0,0,1), false, true);
   
   if(mRenderBorder)
      renderBorder();
}

//------------------------------------------------------------------------------

void TerrainEditor::renderSelection( const Selection & sel, const ColorF & inColorFull, const ColorF & inColorNone, const ColorF & outColorFull, const ColorF & outColorNone, bool renderFill, bool renderFrame )
{
   for(U32 i = 0; i < sel.size(); i++)
   {
      Point3F wPos;
      bool center = gridToWorld(sel[i].mGridPos, wPos);

      ColorF color;         
      if(center)
      {
         if(sel[i].mWeight < 0.f || sel[i].mWeight > 1.f)
            color = inColorFull;
         else
         {
            Point3F pnt;
            pnt.interpolate(Point3F(inColorNone.red, inColorNone.green, inColorNone.blue), 
               Point3F(inColorFull.red, inColorFull.green, inColorFull.blue),
               sel[i].mWeight);
            color.set(pnt.x, pnt.y, pnt.z);
         }
      }
      else
      {
         if(sel[i].mWeight < 0.f || sel[i].mWeight > 1.f)
            color = outColorFull;
         else
         {
            Point3F pnt;
            pnt.interpolate(Point3F(outColorFull.red, outColorFull.green, outColorFull.blue),
               Point3F(outColorNone.red, outColorNone.green, outColorNone.blue), sel[i].mWeight);
            color.set(pnt.x, pnt.y, pnt.z);
         }
      }   

      if(mRenderVertexSelection)
      {
         //
         if(renderFill)
         {
            glDisable(GL_CULL_FACE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);

            glBegin(GL_QUADS);
               glColor3f(color.red, color.green, color.blue);
               glVertex3f(wPos.x - 1, wPos.y - 1, wPos.z);
               glVertex3f(wPos.x + 1, wPos.y - 1, wPos.z);
               glVertex3f(wPos.x + 1, wPos.y + 1, wPos.z);
               glVertex3f(wPos.x - 1, wPos.y + 1, wPos.z);
            glEnd();

            glDisable(GL_BLEND);
         }

         if(renderFrame)
         {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glBegin(GL_QUADS);
               glColor3f(color.red, color.green, color.blue);
               glVertex3f(wPos.x - 1, wPos.y - 1, wPos.z);
               glVertex3f(wPos.x + 1, wPos.y - 1, wPos.z);
               glVertex3f(wPos.x + 1, wPos.y + 1, wPos.z);
               glVertex3f(wPos.x - 1, wPos.y + 1, wPos.z);
            glEnd();
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
         }
      }
      else
      {
         // walk the points in the selection
         for(U32 i = 0; i < sel.size(); i++)
         {
            Point3F wPos[4];
            Point2I gPos = sel[i].mGridPos;
      
            bool center = gridToWorld(gPos, wPos[0]);
            gridToWorld(Point2I(gPos.x + 1, gPos.y), wPos[1]);
            gridToWorld(Point2I(gPos.x + 1, gPos.y + 1), wPos[2]);
            gridToWorld(Point2I(gPos.x, gPos.y + 1), wPos[3]);
   
            // 
            glDisable(GL_CULL_FACE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
   
            if(renderFill)
            {
               glBegin(GL_QUADS);
                  glColor3f(color.red, color.green, color.blue);
                  for(U32 i = 0; i < 4; i++)
                     glVertex3f(wPos[i].x, wPos[i].y, wPos[i].z);
               glEnd();
            }

            glDisable(GL_BLEND);

            if(renderFrame)
            {
               glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
               glBegin(GL_QUADS);
                  glColor3f(color.red, color.green, color.blue);
                  for(U32 k = 0; k < 4; k++)
                     glVertex3f(wPos[k].x, wPos[k].y, wPos[k].z);
               glEnd();
               glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
         }
      }
   }
}

//------------------------------------------------------------------------------

void TerrainEditor::renderBorder()
{
   glDisable(GL_CULL_FACE);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   Point2I pos(0,0);
   Point2I dir[4] = {
      Point2I(1,0),
      Point2I(0,1),
      Point2I(-1,0),
      Point2I(0,-1)
   };
   
   //
   if(mBorderLineMode)
   {
      glColor4ub(mBorderFrameColor.red, mBorderFrameColor.green, mBorderFrameColor.blue, mBorderFrameColor.alpha);
      glBegin(GL_LINE_STRIP);
      for(U32 i = 0; i < 4; i++)
      {
         for(U32 j = 0; j < TerrainBlock::BlockSize; j++)
         {
            Point3F wPos;
            gridToWorld(pos, wPos);
            glVertex3f(wPos.x, wPos.y, wPos.z);
            pos += dir[i];
         }
      }                    
   
      Point3F wPos;
      gridToWorld(Point2I(0,0),  wPos);
      glVertex3f(wPos.x, wPos.y, wPos.z);
      glEnd();
   }
   else
   {
      glEnable(GL_DEPTH_TEST);
      GridSquare * gs = mTerrainBlock->findSquare(TerrainBlock::BlockShift, Point2I(0,0));
      F32 height = F32(gs->maxHeight) * 0.03125f + mBorderHeight;
      
      const MatrixF & mat = mTerrainBlock->getTransform();
      Point3F pos;
      mat.getColumn(3, &pos);

      Point2F min(pos.x, pos.y);
      Point2F max(pos.x + TerrainBlock::BlockSize * mTerrainBlock->getSquareSize(),
                  pos.y + TerrainBlock::BlockSize * mTerrainBlock->getSquareSize());

      ColorI & a = mBorderFillColor;
      ColorI & b = mBorderFrameColor;

      for(U32 i = 0; i < 2; i++)
      {
         //
         if(i){glColor4ub(a.red,a.green,a.blue,a.alpha);glBegin(GL_QUADS);} else {glColor4f(b.red,b.green,b.blue,b.alpha); glBegin(GL_LINE_LOOP);}
         glVertex3f(min.x, min.y, 0);
         glVertex3f(max.x, min.y, 0);
         glVertex3f(max.x, min.y, height);
         glVertex3f(min.x, min.y, height);
         glEnd();
   
         //
         if(i){glColor4ub(a.red,a.green,a.blue,a.alpha);glBegin(GL_QUADS);} else {glColor4f(b.red,b.green,b.blue,b.alpha); glBegin(GL_LINE_LOOP);}
         glVertex3f(min.x, max.y, 0);
         glVertex3f(max.x, max.y, 0);
         glVertex3f(max.x, max.y, height);
         glVertex3f(min.x, max.y, height);
         glEnd();
   
         //
         if(i){glColor4ub(a.red,a.green,a.blue,a.alpha);glBegin(GL_QUADS);} else {glColor4f(b.red,b.green,b.blue,b.alpha); glBegin(GL_LINE_LOOP);}
         glVertex3f(min.x, min.y, 0);
         glVertex3f(min.x, max.y, 0);
         glVertex3f(min.x, max.y, height);
         glVertex3f(min.x, min.y, height);
         glEnd();
   
         //
         if(i){glColor4ub(a.red,a.green,a.blue,a.alpha);glBegin(GL_QUADS);} else {glColor4f(b.red,b.green,b.blue,b.alpha); glBegin(GL_LINE_LOOP);}
         glVertex3f(max.x, min.y, 0);
         glVertex3f(max.x, max.y, 0);
         glVertex3f(max.x, max.y, height);
         glVertex3f(max.x, min.y, height);
         glEnd();
      }
      glDisable(GL_DEPTH_TEST);
   }
   
   glDisable(GL_BLEND);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void TerrainEditor::addUndo(Vector<Selection *> & list, Selection * sel)
{
   AssertFatal(sel, "TerrainEditor::addUndo - invalid selection");
   list.push_front(sel);
   if(list.size() == mUndoLimit)
   {
      Selection * undo = list[list.size()-1];
      delete undo;
      list.pop_back();
   }
}

void TerrainEditor::clearUndo(Vector<Selection *> & list)
{
   for(U32 i = 0; i < list.size(); i++)
      delete list[i];
   list.clear();
}

bool TerrainEditor::processUndo(Vector<Selection *> & src, Vector<Selection *> & dest)
{
   if(!src.size())
      return(false);
      
   Selection * task = src.front();
   src.pop_front();
   
   Selection * save = new Selection;
   for(U32 i = 0; i < task->size(); i++)
   {
      GridInfo info;
      getGridInfo((*task)[i].mGridPos, info);
      save->add(info);
      setGridInfo((*task)[i]);
   }
   
   delete task;
   addUndo(dest, save);

   rebuild();

   return(true);
}

//------------------------------------------------------------------------------
//
void TerrainEditor::rebuild()
{
   // empty
   if(mRebuildEmpty)
   {
      mTerrainBlock->rebuildEmptyFlags();
      mTerrainBlock->packEmptySquares();
      mRebuildEmpty = false;
   }

   // base texture gruop
   if(mRebuildTextures)
   {  
      mRebuildTextures = false;
 
      TerrainBlock * clientTerrain = getClientTerrain();
      if(clientTerrain)
         clientTerrain->buildMipMap();
   }
}

//------------------------------------------------------------------------------

void TerrainEditor::on3DMouseUp(const Gui3DMouseEvent & event)
{
   if(!mTerrainBlock)
      return;

   if(mInAction)
   {
      mouseUnlock();
      mCurrentAction->process(mMouseBrush, event, false, TerrainAction::End);
      setCursor(0);

      if(mUndoSel->size())
      {
         addUndo(mUndoList, mUndoSel);
         clearUndo(mRedoList);
      }
      else
         delete mUndoSel;

      mUndoSel = 0;
      mInAction = false;

      rebuild();
   }
}

void TerrainEditor::on3DMouseDown(const Gui3DMouseEvent & event)
{
   if(!mTerrainBlock)
      return;

   if(mInAction || mUndoSel)
      return;

   mSelectionLocked = false;

   mouseLock();
   mInAction = true;
   mUndoSel = new Selection;
   mCurrentAction->process(mMouseBrush, event, true, TerrainAction::Begin);
}

void TerrainEditor::on3DMouseMove(const Gui3DMouseEvent & event)
{
   if(!mTerrainBlock)
      return;

   Point3F pos;
   if(!collide(event, pos))
   {
      mInAction = false;
      mMouseBrush->reset();
      Canvas->showCursor(true);
   }
   else
   {
      //
      if(mRenderBrush)
         Canvas->showCursor(false);
      mMousePos = pos;

      mMouseBrush->setPosition(mMousePos);
   }
}

void TerrainEditor::on3DMouseDragged(const Gui3DMouseEvent & event)
{
   if(!mTerrainBlock)
      return;

   if(!mInAction)
      return;

   Point3F pos;
   if(!mSelectionLocked)
   {
      if(!collide(event, pos))
      {
         mMouseBrush->reset();
         Canvas->showCursor(true);
         return;
      }
   }
   
   if(mRenderBrush)
      Canvas->showCursor(false);
   
   // check if the mouse has actually moved in grid space
   bool selChanged = false;
   if(!mSelectionLocked)
   {
      Point2I gMouse;
      Point2I gLastMouse;
      worldToGrid(pos, gMouse);
      worldToGrid(mMousePos, gLastMouse);
   
      //
      mMousePos = pos;
      mMouseBrush->setPosition(mMousePos);

      selChanged = gMouse != gLastMouse;
   }
   
   mCurrentAction->process(mMouseBrush, event, selChanged, TerrainAction::Update);
}

void TerrainEditor::on3DMouseEnter(const Gui3DMouseEvent &)
{
   if(!mTerrainBlock)
      return;

   if(mRenderBrush)
      Canvas->showCursor(false);
}

void TerrainEditor::on3DMouseLeave(const Gui3DMouseEvent &)
{
   if(!mTerrainBlock)
      return;

   mInAction = false;
   Canvas->showCursor(true);
}

//------------------------------------------------------------------------------
// any console function which depends on a terrainBlock attached to the editor
// should call this
bool checkTerrainBlock(TerrainEditor * terrainEditor, const char * funcName)
{
   if(!terrainEditor->terrainBlockValid())
   {
      Con::errorf(ConsoleLogEntry::Script, "TerrainEditor::%s: not attached to a terrain block!", funcName);
      return(false);
   }
   return(true);
}

//------------------------------------------------------------------------------

static void cAttachTerrain(SimObject * obj, S32 argc, const char ** argv)
{
   TerrainEditor * terrainEditor = static_cast<TerrainEditor*>(obj);

   TerrainBlock * terrBlock = 0;
     
   SimSet * missionGroup = dynamic_cast<SimSet*>(Sim::findObject("MissionGroup"));
   if(!missionGroup)
   {
      Con::errorf(ConsoleLogEntry::Script, "TerrainEditor::attach: no mission group found");
      return;
   }

   // attach to first found terrainBlock
   if(argc == 2)
   {
      for(SimSetIterator itr(missionGroup); *itr; ++itr)
      {
         terrBlock = dynamic_cast<TerrainBlock*>(*itr);
         if(terrBlock)
            break;
      }

      if(!terrBlock)
         Con::errorf(ConsoleLogEntry::Script, "TerrainEditor::attach: no TerrainBlock objects found!");
   }
   else  // attach to named object
   {
      terrBlock = dynamic_cast<TerrainBlock*>(Sim::findObject(argv[2]));

      if(!terrBlock)
         Con::errorf(ConsoleLogEntry::Script, "TerrainEditor::attach: failed to attach to object '%s'", argv[2]);
   }

   if(terrBlock && !terrBlock->isServerObject())
   {
      Con::errorf(ConsoleLogEntry::Script, "TerrainEditor::attach: cannot attach to client TerrainBlock");
      terrBlock = 0;
   }

   //
   terrainEditor->mTerrainBlock = terrBlock;
}

//------------------------------------------------------------------------------

static void cSetBrushType(SimObject * obj, S32, const char **argv)
{
   TerrainEditor * terrainEditor = static_cast<TerrainEditor*>(obj);

   if(!dStricmp(argv[2], "box"))
   {
      delete terrainEditor->mMouseBrush;
      terrainEditor->mMouseBrush = new BoxBrush(terrainEditor);
   }   
   else if(!dStricmp(argv[2], "ellipse"))
   {
      delete terrainEditor->mMouseBrush;
      terrainEditor->mMouseBrush = new EllipseBrush(terrainEditor);
   }
   else if(!dStricmp(argv[2], "selection"))
   {
      delete terrainEditor->mMouseBrush;
      terrainEditor->mMouseBrush = new SelectionBrush(terrainEditor);
   }
   else {}
}

//------------------------------------------------------------------------------

static void cSetBrushSize(SimObject * obj, S32, const char **argv)
{
   TerrainEditor * terrainEditor = static_cast<TerrainEditor*>(obj);

   S32 w = dAtoi(argv[2]);
   S32 h = dAtoi(argv[3]);

   //
   if(w < 1 || w > Brush::MaxBrushDim || h < 1 || h > Brush::MaxBrushDim)
   {
      Con::errorf(ConsoleLogEntry::General, "TerrainEditor::cSetBrushSize: invalid brush dimension. [1-%d].", Brush::MaxBrushDim);
      return;
   }

   terrainEditor->mBrushSize.set(w, h);
   terrainEditor->mMouseBrush->setSize(terrainEditor->mBrushSize);
}

static const char * cGetBrushPos(SimObject * obj, S32, const char **)
{
   TerrainEditor * tEditor = static_cast<TerrainEditor*>(obj);
   AssertFatal(tEditor->mMouseBrush, "TerrainEditor::cGetBrushPos: no mouse brush!");
   
   Point2I pos = tEditor->mMouseBrush->getPosition();
   char * ret = Con::getReturnBuffer(32);
   dSprintf(ret, sizeof(ret), "%d %d", pos.x, pos.y);
   return(ret);
}

static void cSetBrushPos(SimObject * obj, S32 argc, const char ** argv)
{
   TerrainEditor * tEditor = static_cast<TerrainEditor*>(obj);

   //
   Point2I pos;
   if(argc == 3)
      dSscanf(argv[2], "%d %d", &pos.x, &pos.y);
   else
   {
      pos.x = dAtoi(argv[2]);
      pos.y = dAtoi(argv[3]);
   }

   AssertFatal(tEditor->mMouseBrush, "TerrainEditor::cSetBrushPos: no mouse brush!");
   tEditor->mMouseBrush->setPosition(pos);
}

//------------------------------------------------------------------------------

static void cSetAction(SimObject * obj, S32, const char **argv)
{
   TerrainEditor * terrainEditor = static_cast<TerrainEditor*>(obj);

   for(U32 i = 0; i < terrainEditor->mActions.size(); i++)
   {
      if(!dStricmp(terrainEditor->mActions[i]->getName(), argv[2]))
      {
         terrainEditor->mCurrentAction = terrainEditor->mActions[i];

         //
         terrainEditor->mRenderBrush = terrainEditor->mCurrentAction->useMouseBrush();
         return;   
      }
   }
}

//------------------------------------------------------------------------------

static const char * cGetActionName(SimObject * obj, S32, const char ** argv)
{
   TerrainEditor * terrainEditor = static_cast<TerrainEditor*>(obj);
   U32 index = dAtoi(argv[2]);
   if(index >= terrainEditor->mActions.size())
      return("");
   return(terrainEditor->mActions[index]->getName());
}

//------------------------------------------------------------------------------

static S32 cGetNumActions(SimObject * obj, S32, const char **)
{
   TerrainEditor * terrainEditor = static_cast<TerrainEditor*>(obj);
   return terrainEditor->mActions.size();
}

//------------------------------------------------------------------------------

static const char * cGetCurrentAction(SimObject * obj, S32, const char **)
{
   TerrainEditor *terrainEditor = static_cast<TerrainEditor*>(obj);
   return(terrainEditor->mCurrentAction->getName());
}

static void cResetSelWeights(SimObject * obj, S32, const char ** argv)
{
   TerrainEditor * tEditor = static_cast<TerrainEditor*>(obj);

   bool clear = dAtob(argv[2]);

   //
   if(!clear)
   {
      for(U32 i = 0; i < tEditor->mDefaultSel.size(); i++)
      {
         tEditor->mDefaultSel[i].mPrimarySelect = false;
         tEditor->mDefaultSel[i].mWeight = 1.f;
      }
      return;
   }

   Selection sel;

   U32 i;
   for(i = 0; i < tEditor->mDefaultSel.size(); i++)
   {
      if(tEditor->mDefaultSel[i].mPrimarySelect)
      {
         tEditor->mDefaultSel[i].mWeight = 1.f;
         sel.add(tEditor->mDefaultSel[i]);
      }
   }

   tEditor->mDefaultSel.reset();

   for(i = 0; i < sel.size(); i++)
      tEditor->mDefaultSel.add(sel[i]);
}

//------------------------------------------------------------------------------

static void cUndoAction(SimObject * obj, S32, const char **)
{
   TerrainEditor * terrainEditor = static_cast<TerrainEditor*>(obj);
   if(!checkTerrainBlock(terrainEditor, "undoAction"))
      return;

   terrainEditor->processUndo(terrainEditor->mUndoList, terrainEditor->mRedoList);
}

//------------------------------------------------------------------------------

static void cRedoAction(SimObject * obj, S32, const char **)
{
   TerrainEditor * terrainEditor = static_cast<TerrainEditor*>(obj);
   if(!checkTerrainBlock(terrainEditor, "redoAction"))
      return;

   terrainEditor->processUndo(terrainEditor->mRedoList, terrainEditor->mUndoList);
}

//------------------------------------------------------------------------------

static void cClearSelection(SimObject * obj, S32, const char **)
{
   TerrainEditor * terrainEditor = static_cast<TerrainEditor*>(obj);
   terrainEditor->mDefaultSel.reset();
}

//------------------------------------------------------------------------------

static void cProcessAction(SimObject * obj, S32 argc, const char ** argv)
{
   TerrainEditor * terrainEditor = static_cast<TerrainEditor*>(obj);
   if(!checkTerrainBlock(terrainEditor, "processAction"))
      return;

   TerrainAction * action = terrainEditor->mCurrentAction;
   if(argc == 3)
   {
      action = terrainEditor->lookupAction(argv[2]);

      if(!action)
      {
         Con::errorf(ConsoleLogEntry::General, "TerrainEditor::cProcessAction: invalid action name '%s'.", argv[2]);
         return;
      }   
   }

   if(!terrainEditor->getCurrentSel()->size() && !terrainEditor->mProcessUsesBrush)
      return;

   terrainEditor->mUndoSel = new Selection;

   Gui3DMouseEvent event;
   if(terrainEditor->mProcessUsesBrush)
      action->process(terrainEditor->mMouseBrush, event, true, TerrainAction::Process);
   else
      action->process(terrainEditor->getCurrentSel(), event, true, TerrainAction::Process);

   terrainEditor->rebuild();

   // check if should delete the undo
   if(terrainEditor->mUndoSel->size())
   {
      terrainEditor->addUndo(terrainEditor->mUndoList, terrainEditor->mUndoSel);
      terrainEditor->clearUndo(terrainEditor->mRedoList);
   }
   else
      delete terrainEditor->mUndoSel;

   terrainEditor->mUndoSel = 0;
}

static void cBuildMaterialMap(SimObject * obj, S32, const char **)
{
   TerrainEditor * terrainEditor = static_cast<TerrainEditor*>(obj);
   if(!checkTerrainBlock(terrainEditor, "buildMaterialMap"))
      return;
   terrainEditor->mTerrainBlock->buildMaterialMap();
}

static S32 cGetNumTextures(SimObject * obj, S32, const char **)
{
   TerrainEditor * terrainEditor = static_cast<TerrainEditor*>(obj);
   if(!checkTerrainBlock(terrainEditor, "getNumTextures"))
      return(0);

   // walk all the possible material lists and count them..
   U32 count = 0;
   for(U32 i = 0; i < TerrainBlock::MaterialGroups; i++)
      if(terrainEditor->mTerrainBlock->mMaterialFileName[i] &&
         *terrainEditor->mTerrainBlock->mMaterialFileName[i])
         count++;
   return count;
}

static const char * cGetTextureName(SimObject * obj, S32, const char ** argv)
{
   TerrainEditor * terrainEditor = static_cast<TerrainEditor*>(obj);
   if(!checkTerrainBlock(terrainEditor, "getTextureName"))
      return("");

   // textures only exist on the client..
   NetConnection * toServer = NetConnection::getServerConnection();
   NetConnection * toClient = NetConnection::getLocalClientConnection();

   S32 index = toClient->getGhostIndex(terrainEditor->mTerrainBlock);

   TerrainBlock * terrBlock = dynamic_cast<TerrainBlock*>(toServer->resolveGhost(index));
   if(!terrBlock)
      return("");

   // possibly in range?
   S32 group = dAtoi(argv[2]);
   if(group < 0 || group >= TerrainBlock::MaterialGroups)
      return("");

   // now find the i-th group
   U32 count = 0;
   bool found = false;
   for(U32 i = 0; !found && (i < TerrainBlock::MaterialGroups); i++)
   {
      // count it 
      if(terrBlock->mMaterialFileName[i] &&
         *terrBlock->mMaterialFileName[i])
         count++;   
      
      if((group + 1) == count)
      {
         group = i;
         found = true;
      }
   }

   if(!found)
      return("");

   char *retBuffer = Con::getReturnBuffer(256);
   dSprintf(retBuffer, 256, "terrain/%s", terrBlock->mMaterialFileName[group]);
   return retBuffer;
}

static void findObjectsCallback(SceneObject* obj, S32 val)
{  
   Vector<SceneObject*> * list = (Vector<SceneObject*>*)val;
   list->push_back(obj);
}

static void cMarkEmptySquares(SimObject * obj, S32, const char **)
{
   TerrainEditor * terrainEditor = static_cast<TerrainEditor*>(obj);
   if(!checkTerrainBlock(terrainEditor, "markEmptySquares"))
      return;

   // build a list of all the marked interiors
   Vector<InteriorInstance*> interiors;
   U32 mask = InteriorObjectType;
   gServerContainer.findObjects(mask, findObjectsCallback, (S32)&interiors);

   // walk the terrain and empty any grid which clips to an interior
   for(U32 x = 0; x < TerrainBlock::BlockSize; x++)
      for(U32 y = 0; y < TerrainBlock::BlockSize; y++)
      {
         TerrainBlock::Material * material = terrainEditor->mTerrainBlock->getMaterial(x,y);   
         material->flags |= ~(TerrainBlock::Material::Empty);
         
         Point3F a, b;
         terrainEditor->gridToWorld(Point2I(x,y), a);
         terrainEditor->gridToWorld(Point2I(x+1,y+1), b);

         Box3F box;
         box.min = a;
         box.max = b;
         
         box.min.setMin(b);
         box.max.setMax(a);

         const MatrixF & terrOMat = terrainEditor->mTerrainBlock->getTransform();
         const MatrixF & terrWMat = terrainEditor->mTerrainBlock->getWorldTransform();

         terrWMat.mulP(box.min);
         terrWMat.mulP(box.max);

         for(U32 i = 0; i < interiors.size(); i++)
         {
            MatrixF mat = interiors[i]->getWorldTransform();
            mat.scale(interiors[i]->getScale());
            mat.mul(terrOMat);

            U32 waterMark = FrameAllocator::getWaterMark();
            U16* zoneVector = (U16*)FrameAllocator::alloc(interiors[i]->getDetailLevel(0)->getNumZones());
            U32 numZones = 0;
            interiors[i]->getDetailLevel(0)->scanZones(box, mat,
                                                       zoneVector, &numZones);
            if (numZones != 0) 
            {
               Con::printf("%d %d", x, y);
               material->flags |= TerrainBlock::Material::Empty;
               FrameAllocator::setWaterMark(waterMark);
               break;
            }
            FrameAllocator::setWaterMark(waterMark);
         }
      }

   // rebuild stuff..
   terrainEditor->mTerrainBlock->buildGridMap();
   terrainEditor->mTerrainBlock->rebuildEmptyFlags();
   terrainEditor->mTerrainBlock->packEmptySquares();
}

static void cClearModifiedFlags(SimObject * obj, S32, const char **)
{
   TerrainEditor * terrainEditor = static_cast<TerrainEditor*>(obj);
   if(!checkTerrainBlock(terrainEditor, "clearModifiedFlags"))
      return;

   //   
   for(U32 i = 0; i < (TerrainBlock::BlockSize * TerrainBlock::BlockSize); i++)
      terrainEditor->mTerrainBlock->materialMap[i].flags &= ~TerrainBlock::Material::Modified;
}

static void cMirrorTerrain(SimObject * obj, S32, const char ** argv)
{
   TerrainEditor * terrainEditor = static_cast<TerrainEditor*>(obj);
   if(!checkTerrainBlock(terrainEditor, "mirrorTerrain"))
      return;
   
   TerrainBlock * terrain = terrainEditor->mTerrainBlock;
   S32 mirrorIndex = dAtoi(argv[2]);

   // 
   enum {
      top = BIT(0),
      bottom = BIT(1),
      left = BIT(2),
      right = BIT(3)
   };

   U32 sides[8] =
   {
      bottom,
      bottom | left,
      left,
      left | top,
      top,
      top | right,
      right,
      bottom | right
   };

   U32 n = TerrainBlock::BlockSize;
   U32 side = sides[mirrorIndex % 8];
   bool diag = mirrorIndex & 0x01;

   Point2I src((side & right) ? (n - 1) : 0, (side & bottom) ? (n - 1) : 0);
   Point2I dest((side & left) ? (n - 1) : 0, (side & top) ? (n - 1) : 0);
   Point2I origSrc(src);
   Point2I origDest(dest);

   // determine the run length
   U32 minStride = ((side & top) || (side & bottom)) ? n : n / 2;
   U32 majStride = ((side & left) || (side & right)) ? n : n / 2;

   Point2I srcStep((side & right) ? -1 : 1, (side & bottom) ? -1 : 1);
   Point2I destStep((side & left) ? -1 : 1, (side & top) ? -1 : 1);

   //   
   U16 * heights = terrain->getHeightAddress(0,0);
   U8 * baseMaterials = terrain->getBaseMaterialAddress(0,0);
   TerrainBlock::Material * materials = terrain->getMaterial(0,0);

   // create an undo selection
   Selection * undo = new Selection;
   
   // walk through all the positions
   for(U32 i = 0; i < majStride; i++)
   {
      for(U32 j = 0; j < minStride; j++)
      {
         // skip the same position
         if(src != dest)
         {   
            U32 si = src.x + (src.y << TerrainBlock::BlockShift);
            U32 di = dest.x + (dest.y << TerrainBlock::BlockShift);

            // add to undo selection
            GridInfo info;
            terrainEditor->getGridInfo(dest, info);
            undo->add(info);
            
            //... copy info... (height, basematerial, material)
            heights[di] = heights[si];
            baseMaterials[di] = baseMaterials[si];
            materials[di] = materials[si];
         }
         
         // get to the new position
         src.x += srcStep.x;
         diag ? (dest.y += destStep.y) : (dest.x += destStep.x);
      }
      
      // get the next position for a run
      src.y += srcStep.y;
      diag ? (dest.x += destStep.x) : (dest.y += destStep.y);

      // reset the minor run
      src.x = origSrc.x;
      diag ? (dest.y = origDest.y) : (dest.x = origDest.x);
               
      // shorten the run length for diag runs   
      if(diag)
         minStride--;
   }

   // rebuild stuff..
   terrain->buildGridMap();
   terrain->rebuildEmptyFlags();
   terrain->packEmptySquares();

   // add undo selection to undo list and clear redo
   terrainEditor->addUndo(terrainEditor->mUndoList, undo);
   terrainEditor->clearUndo(terrainEditor->mRedoList);
}

static void cPushBaseMaterialInfo(SimObject * obj, S32, const char **)
{
   TerrainEditor * terrainEditor = static_cast<TerrainEditor*>(obj);
   if(!checkTerrainBlock(terrainEditor, "pushMaterialInfo"))
      return;

   TerrainBlock * terrain = terrainEditor->mTerrainBlock;

   BaseMaterialInfo * info = new BaseMaterialInfo;

   // copy the material list names
   for(U32 i = 0; i < TerrainBlock::MaterialGroups; i++)
      info->mMaterialNames[i] = terrain->mMaterialFileName[i];

   // copy the base materials
   dMemcpy(info->mBaseMaterials, terrain->mBaseMaterialMap, 
      TerrainBlock::BlockSize * TerrainBlock::BlockSize);

   terrainEditor->mBaseMaterialInfos.push_front(info);
}

static void cPopBaseMaterialInfo(SimObject * obj, S32, const char **)
{
   TerrainEditor * terrainEditor = static_cast<TerrainEditor*>(obj);
   if(!checkTerrainBlock(terrainEditor, "popMaterialInfo"))
      return;

   if(!terrainEditor->mBaseMaterialInfos.size())
      return;

   TerrainBlock * terrain = terrainEditor->mTerrainBlock;

   BaseMaterialInfo * info = terrainEditor->mBaseMaterialInfos.front();

   // names
   for(U32 i = 0; i < TerrainBlock::MaterialGroups; i++)
      terrain->mMaterialFileName[i] = info->mMaterialNames[i];

   // base materials
   dMemcpy(terrain->mBaseMaterialMap, info->mBaseMaterials, 
      TerrainBlock::BlockSize * TerrainBlock::BlockSize);
   
   // kill it..
   delete info;
   terrainEditor->mBaseMaterialInfos.pop_front();

   // rebuild
   terrain->refreshMaterialLists();
   terrain->buildGridMap();
}

static void cSetLoneBaseMaterial(SimObject * obj, S32, const char ** argv)
{
   TerrainEditor * terrainEditor = static_cast<TerrainEditor*>(obj);
   if(!checkTerrainBlock(terrainEditor, "setLoneBaseMaterial"))
      return;
   
   TerrainBlock * terrain = terrainEditor->mTerrainBlock;
   
   // force the material group
   terrain->mMaterialFileName[0] = StringTable->insert(argv[2]);
   dMemset(terrain->getBaseMaterialAddress(0,0), 
      TerrainBlock::BlockSize * TerrainBlock::BlockSize, 0);

   terrain->refreshMaterialLists();
   terrain->buildGridMap();
}

//------------------------------------------------------------------------------

void TerrainEditor::consoleInit()
{
   Con::addCommand("TerrainEditor", "attachTerrain", cAttachTerrain, "terrainEditor.attachTerrain(<terrainObj>);", 2, 3);
   Con::addCommand("TerrainEditor", "setBrushType", cSetBrushType, "terrainEditor.setBrushType(box | ellipse | ...);", 3, 3);
   Con::addCommand("TerrainEditor", "setBrushSize", cSetBrushSize, "terrainEditor.setBrushSize(x, y);", 4, 4);
   Con::addCommand("TerrainEditor", "getBrushPos", cGetBrushPos, "terrainEditor.getBrushPos();", 2, 2);
   Con::addCommand("TerrainEditor", "setBrushPos", cSetBrushPos, "terrainEditor.setBrushPos(x, y);", 3, 4);
   Con::addCommand("TerrainEditor", "setAction", cSetAction, "terrainEditor.setAction(action_name);", 3, 3);
   Con::addCommand("TerrainEditor", "getNumActions", cGetNumActions, "terrainEditor.getNumActions();", 2, 2);
   Con::addCommand("TerrainEditor", "getActionName", cGetActionName, "terrainEditor.getActionName(num);", 3, 3);
   Con::addCommand("TerrainEditor", "getCurrentAction", cGetCurrentAction, "terrainEditor.getCurrentAction();", 2, 2);
   Con::addCommand("TerrainEditor", "resetSelWeights", cResetSelWeights, "terrainEditor.resetSelWeights(clear);", 3, 3);
   Con::addCommand("TerrainEditor", "undo", cUndoAction, "terrainEditor.undo();", 2, 2);
   Con::addCommand("TerrainEditor", "redo", cRedoAction, "terrainEditor.redo();", 2, 2);
   Con::addCommand("TerrainEditor", "clearSelection", cClearSelection, "terrainEditor.clearSelection();", 2, 2);
   Con::addCommand("TerrainEditor", "processAction", cProcessAction, "terrainEditor.processAction(<action>);", 2, 3);
   Con::addCommand("TerrainEditor", "buildMaterialMap", cBuildMaterialMap, "terrainEditor.buildMaterialMap();", 2, 2);
   Con::addCommand("TerrainEditor", "getNumTextures", cGetNumTextures, "terrainEditor.getNumTextures();", 2, 2);
   Con::addCommand("TerrainEditor", "getTextureName", cGetTextureName, "terrainEditor.getTextureName(index);", 3, 3);
   Con::addCommand("TerrainEditor", "markEmptySquares", cMarkEmptySquares, "terrainEditor.markEmptySquares();", 2, 2);
   Con::addCommand("TerrainEditor", "clearModifiedFlags", cClearModifiedFlags, "terrainEditor.clearModifiedFlags();", 2, 2);
   Con::addCommand("TerrainEditor", "mirrorTerrain", cMirrorTerrain, "terrainEditor.mirrorTerrain(dest octant index);", 3, 3);
   Con::addCommand("TerrainEditor", "pushBaseMaterialInfo", cPushBaseMaterialInfo, "terrainEditor.pushBaseMaterialInfo();", 2, 2);
   Con::addCommand("TerrainEditor", "popBaseMaterialInfo", cPopBaseMaterialInfo, "terrainEditor.popBaseMaterialInfo();", 2, 2);
   Con::addCommand("TerrainEditor", "setLoneBaseMaterial", cSetLoneBaseMaterial, "terrainEditor.setLoneBaseMaterial(material list base name);", 3, 3);
}

void TerrainEditor::initPersistFields()
{
   Parent::initPersistFields();
   addField("renderBorder", TypeBool, Offset(mRenderBorder, TerrainEditor));
   addField("borderHeight", TypeF32, Offset(mBorderHeight, TerrainEditor));
   addField("borderFillColor", TypeColorI, Offset(mBorderFillColor, TerrainEditor));
   addField("borderFrameColor", TypeColorI, Offset(mBorderFrameColor, TerrainEditor));
   addField("borderLineMode", TypeBool, Offset(mBorderLineMode, TerrainEditor));
   addField("selectionHidden", TypeBool, Offset(mSelectionHidden, TerrainEditor));
   addField("enableSoftBrushes", TypeBool, Offset(mEnableSoftBrushes, TerrainEditor));
   addField("renderVertexSelection", TypeBool, Offset(mRenderVertexSelection, TerrainEditor));
   addField("processUsesBrush", TypeBool, Offset(mProcessUsesBrush, TerrainEditor));

   // action values...
   addField("adjustHeightVal", TypeF32, Offset(mAdjustHeightVal, TerrainEditor));
   addField("setHeightVal", TypeF32, Offset(mSetHeightVal, TerrainEditor));
   addField("scaleVal", TypeF32, Offset(mScaleVal, TerrainEditor));
   addField("smoothFactor", TypeF32, Offset(mSmoothFactor, TerrainEditor));
   addField("materialGroup", TypeS32, Offset(mMaterialGroup, TerrainEditor));
   addField("softSelectRadius", TypeF32, Offset(mSoftSelectRadius, TerrainEditor));
   addField("softSelectFilter", TypeString, Offset(mSoftSelectFilter, TerrainEditor));
   addField("softSelectDefaultFilter", TypeString, Offset(mSoftSelectDefaultFilter, TerrainEditor));
   addField("adjustHeightMouseScale", TypeF32, Offset(mAdjustHeightMouseScale, TerrainEditor));
}
