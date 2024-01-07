//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "Editor/terrainActions.h"
#include "Platform/event.h"
#include "GUI/guiCanvas.h"

//------------------------------------------------------------------------------

void SelectAction::process(Selection * sel, const Gui3DMouseEvent & event, bool selChanged, Type type)
{
   if(sel == mTerrainEditor->getCurrentSel())
      return;

   if(type == Process)
      return;

   if(selChanged)
   {
      if(event.modifier & SI_CTRL)
      {
         for(U32 i = 0; i < sel->size(); i++)
            mTerrainEditor->getCurrentSel()->remove((*sel)[i]);
      }
      else
      {
         for(U32 i = 0; i < sel->size(); i++)
         {
            GridInfo gInfo;
            if(mTerrainEditor->getCurrentSel()->getInfo((*sel)[i].mGridPos, gInfo))
            {
               if(!gInfo.mPrimarySelect)
                  gInfo.mPrimarySelect = (*sel)[i].mPrimarySelect;

               if(gInfo.mWeight < (*sel)[i].mWeight)
                  gInfo.mWeight = (*sel)[i].mWeight;
                  
               mTerrainEditor->getCurrentSel()->setInfo(gInfo);   
            }
            else
               mTerrainEditor->getCurrentSel()->add((*sel)[i]);
         }
      }
   }
}

//------------------------------------------------------------------------------

void SoftSelectAction::process(Selection * sel, const Gui3DMouseEvent &, bool selChanged, Type type)
{
   // allow process of current selection
   Selection tmpSel;
   if(sel == mTerrainEditor->getCurrentSel())
   {
      tmpSel = *sel;
      sel = &tmpSel;
   }

   if(type == Begin || type == Process)
      mFilter.set(1, &mTerrainEditor->mSoftSelectFilter);

   //
   if(selChanged)
   {
      F32 radius = mTerrainEditor->mSoftSelectRadius;
      if(radius == 0.f)
         return;

      S32 squareSize = mTerrainEditor->getTerrainBlock()->getSquareSize();
      U32 offset = U32(radius / F32(squareSize)) + 1;

      for(U32 i = 0; i < sel->size(); i++)
      {
         GridInfo & info = (*sel)[i];

         info.mPrimarySelect = true;
         info.mWeight = mFilter.getValue(0);

         if(!mTerrainEditor->getCurrentSel()->add(info))
            mTerrainEditor->getCurrentSel()->setInfo(info);

         Point2F infoPos(info.mGridPos.x, info.mGridPos.y);
            
         //
         for(S32 x = info.mGridPos.x - offset; x < info.mGridPos.x + (offset << 1); x++)
            for(S32 y = info.mGridPos.y - offset; y < info.mGridPos.y + (offset << 1); y++)
            {
               //
               Point2F pos(x, y);

               F32 dist = Point2F(pos - infoPos).len() * F32(squareSize);

               if(dist > radius)
                  continue;

               F32 weight = mFilter.getValue(dist / radius);

               //
               GridInfo gInfo;
               if(mTerrainEditor->getCurrentSel()->getInfo(Point2I(x, y), gInfo))
               {
                  if(gInfo.mPrimarySelect)
                     continue;

                  if(gInfo.mWeight < weight)
                  {
                     gInfo.mWeight = weight;
                     mTerrainEditor->getCurrentSel()->setInfo(gInfo);
                  }
               }
               else
               {
                  mTerrainEditor->getGridInfo(Point2I(x, y), gInfo);
                  gInfo.mWeight = weight;
                  gInfo.mPrimarySelect = false;
                  mTerrainEditor->getCurrentSel()->add(gInfo);
               }
            }
      }
   }
}

//------------------------------------------------------------------------------

void OutlineSelectAction::process(Selection * sel, const Gui3DMouseEvent & event, bool, Type type)
{
   sel;event;type;
   switch(type)
   {
      case Begin:
         if(event.modifier & SI_SHIFT)
            break;
         
         mTerrainEditor->getCurrentSel()->reset();
         break;

      case End:
      case Update:

      default:
         return;
   }

   mLastEvent = event;
}

//------------------------------------------------------------------------------

void RaiseHeightAction::process(Selection * sel, const Gui3DMouseEvent &, bool selChanged, Type)
{
   if(selChanged)
      for(U32 i = 0; i < (*sel).size(); i++)
      {
         mTerrainEditor->getUndoSel()->add((*sel)[i]);
         (*sel)[i].mHeight += mTerrainEditor->mAdjustHeightVal * (*sel)[i].mWeight;
         mTerrainEditor->setGridInfo((*sel)[i]);
      }
}

//------------------------------------------------------------------------------

void LowerHeightAction::process(Selection * sel, const Gui3DMouseEvent &, bool selChanged, Type)
{
   if(selChanged)
      for(U32 i = 0; i < sel->size(); i++)
      {
         mTerrainEditor->getUndoSel()->add((*sel)[i]);
         (*sel)[i].mHeight -= mTerrainEditor->mAdjustHeightVal * (*sel)[i].mWeight;
         mTerrainEditor->setGridInfo((*sel)[i]);
      }
}

//------------------------------------------------------------------------------

void SetHeightAction::process(Selection * sel, const Gui3DMouseEvent &, bool selChanged, Type)
{
   if(selChanged)
      for(U32 i = 0; i < sel->size(); i++)
      {
         mTerrainEditor->getUndoSel()->add((*sel)[i]);
         (*sel)[i].mHeight = mTerrainEditor->mSetHeightVal;
         mTerrainEditor->setGridInfo((*sel)[i]);
      }
}

//------------------------------------------------------------------------------

void SetEmptyAction::process(Selection * sel, const Gui3DMouseEvent &, bool selChanged, Type)
{
   if(selChanged)
      for(U32 i = 0; i < sel->size(); i++)
      {
         mTerrainEditor->getUndoSel()->add((*sel)[i]);
         (*sel)[i].mMaterial.flags |= TerrainBlock::Material::Empty;
         mTerrainEditor->setGridInfo((*sel)[i]);
      }
}

//------------------------------------------------------------------------------

void ClearEmptyAction::process(Selection * sel, const Gui3DMouseEvent &, bool selChanged, Type)
{
   if(selChanged)
      for(U32 i = 0; i < sel->size(); i++)
      {
         mTerrainEditor->getUndoSel()->add((*sel)[i]);
         (*sel)[i].mMaterial.flags &= ~TerrainBlock::Material::Empty;
         mTerrainEditor->setGridInfo((*sel)[i]);
      }
}

//------------------------------------------------------------------------------

void SetModifiedAction::process(Selection * sel, const Gui3DMouseEvent &, bool selChanged, Type)
{
   if(selChanged)
      for(U32 i = 0; i < sel->size(); i++)
      {
         mTerrainEditor->getUndoSel()->add((*sel)[i]);
         (*sel)[i].mMaterial.flags |= TerrainBlock::Material::Modified;
         mTerrainEditor->setGridInfo((*sel)[i]);
      }
}

//------------------------------------------------------------------------------

void ClearModifiedAction::process(Selection * sel, const Gui3DMouseEvent &, bool selChanged, Type)
{
   if(selChanged)
      for(U32 i = 0; i < sel->size(); i++)
      {
         mTerrainEditor->getUndoSel()->add((*sel)[i]);
         (*sel)[i].mMaterial.flags &= ~TerrainBlock::Material::Modified;
         mTerrainEditor->setGridInfo((*sel)[i]);
      }
}

//------------------------------------------------------------------------------

void ScaleHeightAction::process(Selection * sel, const Gui3DMouseEvent &, bool selChanged, Type)
{
   if(selChanged)
      for(U32 i = 0; i < sel->size(); i++)
      {
         mTerrainEditor->getUndoSel()->add((*sel)[i]);
         (*sel)[i].mHeight *= mTerrainEditor->mScaleVal;
         mTerrainEditor->setGridInfo((*sel)[i]);
      }
}

void BrushAdjustHeightAction::process(Selection * sel, const Gui3DMouseEvent & event, bool, Type type)
{
   if(type == Process)
      return;

   //
   if(type == Begin)
   {
      mTerrainEditor->lockSelection(true);

      mFirstPos = mLastPos = event.mousePoint;
      Canvas->mouseLock(mTerrainEditor);

      // add to undo
      for(U32 i = 0; i < sel->size(); i++)
         mTerrainEditor->getUndoSel()->add((*sel)[i]);
   }
   else if(type == Update)
   {
      //
      F32 diff = (event.mousePoint.x - mLastPos.x) * mTerrainEditor->mAdjustHeightMouseScale;

      for(U32 i = 0; i < sel->size(); i++)
      {
         (*sel)[i].mHeight += diff * (*sel)[i].mWeight;

         // clamp it
         if((*sel)[i].mHeight < 0.f)
            (*sel)[i].mHeight = 0.f;
         if((*sel)[i].mHeight > 2047.f)
            (*sel)[i].mHeight = 2047.f;

         mTerrainEditor->setGridInfo((*sel)[i]);
      }

      mLastPos = event.mousePoint;
   } 
   else if(type == End)
   {  
      Canvas->mouseUnlock(mTerrainEditor);
      Canvas->setCursorPos(mFirstPos);
   }
}

//------------------------------------------------------------------------------

AdjustHeightAction::AdjustHeightAction(TerrainEditor * editor) : 
   TerrainAction(editor)
{
   mCursor = 0;
}

void AdjustHeightAction::process(Selection *, const Gui3DMouseEvent & event, bool, Type type)
{
   if(type == Process)
      return;

   Selection * curSel = mTerrainEditor->getCurrentSel();

   //
   if(type == Begin)
   {
      mTerrainEditor->lockSelection(true);

      if(!mTerrainEditor->collide(event, mHitPos))
         return;

      if(!bool(mCursor))
         mCursor = dynamic_cast<GuiCursor*>(Sim::findObject("Editor_HandCursor"));

      if(bool(mCursor))
         mTerrainEditor->setCursor(mCursor);

      mLastPos = mHitPos;

      // add to undo
      for(U32 i = 0; i < curSel->size(); i++)
         mTerrainEditor->getUndoSel()->add((*curSel)[i]);
   }
   else if(type == Update)
   {
      // do a projection onto the z axis
      F64 dist = mSqrt((event.pos.x - mHitPos.x) * (event.pos.x - mHitPos.x) +
         (event.pos.y - mHitPos.y) * (event.pos.y - mHitPos.y));

      Point3F vec(mHitPos.x - event.pos.x, mHitPos.y - event.pos.y, 0.f);
      vec.normalize();

      F64 projDist = mDot(event.vec, vec);
      if(projDist == 0.f)
         return;

      F64 scale = dist / projDist;
      vec = event.pos + (event.vec * scale);

      for(U32 i = 0; i < curSel->size(); i++)
      {
         F32 diff = (vec.z - mLastPos.z) * (*curSel)[i].mWeight;
         (*curSel)[i].mHeight += diff;

         // clamp it
         if((*curSel)[i].mHeight < 0.f)
            (*curSel)[i].mHeight = 0.f;
         if((*curSel)[i].mHeight > 2047.f)
            (*curSel)[i].mHeight = 2047.f;

         mTerrainEditor->setGridInfo((*curSel)[i]);
      }

      mLastPos = vec;
   }
}

//------------------------------------------------------------------------------
// flatten the primary selection then blend in the rest...

void FlattenHeightAction::process(Selection * sel, const Gui3DMouseEvent &, bool selChanged, Type)
{
   if(!sel->size())
      return;

   if(selChanged)
   {
      F32 average = 0.f;

      // get the average height
      U32 cPrimary = 0;
      for(U32 k = 0; k < sel->size(); k++)
         if((*sel)[k].mPrimarySelect)
         {
            cPrimary++;
            average += (*sel)[k].mHeight;
         }

      average /= cPrimary;

      // set it
      for(U32 i = 0; i < sel->size(); i++)
      {
         mTerrainEditor->getUndoSel()->add((*sel)[i]);

         //
         if((*sel)[i].mPrimarySelect)
            (*sel)[i].mHeight = average;
         else
         {
            F32 h = average - (*sel)[i].mHeight;
            (*sel)[i].mHeight += (h * (*sel)[i].mWeight);
         }

         mTerrainEditor->setGridInfo((*sel)[i]);
      }
   }
}

//------------------------------------------------------------------------------

void SmoothHeightAction::process(Selection * sel, const Gui3DMouseEvent &, bool selChanged, Type)
{
   if(!sel->size())
      return;

   if(selChanged)
   {
      F32 avgHeight = 0.f;
      for(U32 k = 0; k < sel->size(); k++)
      {
         mTerrainEditor->getUndoSel()->add((*sel)[k]);
         avgHeight += (*sel)[k].mHeight;
      }

      avgHeight /= sel->size();

      // clamp the terrain smooth factor...
      if(mTerrainEditor->mSmoothFactor < 0.f)
         mTerrainEditor->mSmoothFactor = 0.f;
      if(mTerrainEditor->mSmoothFactor > 1.f)
         mTerrainEditor->mSmoothFactor = 1.f;

      // linear
      for(U32 i = 0; i < sel->size(); i++)
      {
         (*sel)[i].mHeight += (avgHeight - (*sel)[i].mHeight) * mTerrainEditor->mSmoothFactor * (*sel)[i].mWeight;
         mTerrainEditor->setGridInfo((*sel)[i]);
      }
   }
}

void SetMaterialGroupAction::process(Selection * sel, const Gui3DMouseEvent &, bool selChanged, Type)
{
   if(selChanged)
      for(U32 i = 0; i < sel->size(); i++)
      {
         mTerrainEditor->getUndoSel()->add((*sel)[i]);
 
         (*sel)[i].mMaterial.flags |= TerrainBlock::Material::Modified;
         (*sel)[i].mMaterialGroup = mTerrainEditor->mMaterialGroup;
         mTerrainEditor->setGridInfo((*sel)[i]);
      }
}
