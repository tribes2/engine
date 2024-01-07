//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "dgl/dgl.h"
#include "ai/graphFloorPlan.h"
#include "ai/graphGroundPlan.h"

extern void wireCube(F32 size,Point3F pos);

#define zone1 0.1
#define zone2 0.2
#define zone3 0.3
#define zone4 0.4
#define zone5 0.5
#define zone6 0.6
#define zone7 0.7
#define zone8 0.8
#define zone9 0.9
#define zone10 1.0
#define zone11 0.1
#define zone12 0.2
#define zone13 0.3
#define zone14 0.4
#define zone15 0.5
#define zone16 0.6
#define zone17 0.7
#define zone18 0.8
#define zone19 0.9

//----------------------------------------------------------------------------
// render methods - gives visual rep of generated graph
// * must be called during engine rendering phase
//----------------------------------------------------------------------------
void FloorPlan::render()
{
#if DEBUGMODE
   
   U32 i;
   for(i = 0; i < detail.mTraversable.size(); i++)
   {   
      // only draw this surface if it has no children
      if(detail.mSurfaces[detail.mTraversable[i]].mSubSurfCount == 0)
         drawSurface(&detail, detail.mSurfaces[detail.mTraversable[i]]);
   }
   //renderPolyPoints(&detail);
   //drawConnections(&detail);
   //drawInterfaces(&detail);
   //renderPolyPoints(&detail);
   
#endif
}

void FloorPlan::drawInterfaces(InteriorDetail *d)
{
   for(S32 i = 0; i < d->mConnections.size(); i++)
   {
      drawEdge(d->mConnections[i].segNodes[0], d->mConnections[i].segNodes[1]);     
   }
}

void FloorPlan::renderSpecialNodes(InteriorDetail *)
{
   U32 sCount = portalPoints.size(), j;
   for(j = 0; j < sCount; j++)
      wireCube(0.2f, portalPoints[j]);  
}

void FloorPlan::renderPolyPoints(InteriorDetail *d)
{
   U32 sCount = d->polyTestPoints.size(), j;
   for(j = 0; j < sCount; j++)
      wireCube(0.1, d->polyTestPoints[j]);  
   
}

//----------------------------------------------------------------------------

void FloorPlan::renderEdgeTable(InteriorDetail *d)
{
   DEdge    *walk;
   DPoint   p1, p2;
   
   U32 i;
   for(i = 0; i < d->mEdgeTable.tableSize(); i++)
   {
      walk = d->mEdgeTable.getBucket(i);
      while(walk)
      {
         if(walk->flags.test(DEdge::divGenerated))
         {
            p1 = d->mPoints[walk->start];
            p2 = d->mPoints[walk->end];
         
            bool p1ok = validatePoint(p1);
            bool p2ok = validatePoint(p2);
            AssertFatal(p1ok, "invalid Point3F");
            AssertFatal(p2ok, "invalid Point3F");
         
            drawEdge(p1, p2);
         }
         walk = walk->next;
      }
   }
}

//----------------------------------------------------------------------------

void FloorPlan::drawSurface(InteriorDetail *d, DSurf &surf)
{
   Point3F pts[32];
   
   U32 i;
   for(i = 0; i < surf.mNumEdges; i++)
   {  
      if(!surf.mFlipStates.test(BIT(i)))
         pts[i] = d->mPoints[surf.mEdges[i]->start];
      else
         pts[i] = d->mPoints[surf.mEdges[i]->end];
   }
   
   drawPoly(i, pts, surf.mFlags.test(DSurf::unObstructed), surf.mFlags.test(DSurf::tooSmall), surf.mZone);
}

//----------------------------------------------------------------------------

void FloorPlan::drawPoly(U8 n, Point3F *pts, bool, bool, S32 zone)
{
   switch(zone)
   {
      case -1:
      case 0:
         glColor3f(1, 1, 1);
         break;
      case 1:
         glColor3f(zone10, zone1, zone1);
         break;
      case 2:
         glColor3f(zone2, zone10, zone2);
         break;
      case 3:
         glColor3f(zone3, zone3, zone10);
         break;
      case 4:
         glColor3f(zone10, zone4, zone4);
         break;
      case 5:
         glColor3f(zone5, zone10, zone5);
         break;
      case 6:
         glColor3f(zone6, zone6, zone10);
         break;
      case 7:
         glColor3f(zone10, zone7, zone7);
         break;
      case 8:
         glColor3f(zone8, zone10, zone8);
         break;
      case 9:
         glColor3f(zone9, zone9, zone10);
         break;
      case 10:
         glColor3f(zone1, zone10, zone10);
         break;
      case 11:
         glColor3f(zone10, zone11, zone11);
         break;
      case 12:
         glColor3f(zone12, zone10, zone12);
         break;
      case 13:
         glColor3f(zone13, zone13, zone10);
         break;
      case 14:
         glColor3f(zone10, zone14, zone14);
         break;
      case 15:
         glColor3f(zone15, zone10, zone15);
         break;
      case 16:
         glColor3f(zone16, zone16, zone10);
         break;
      case 17:
         glColor3f(zone10, zone17, zone17);
         break;
      case 18:
         glColor3f(zone18, zone10, zone18);
         break;
      case 19:
         glColor3f(zone19, zone19, zone10);
         break;   
   }
   
   U32 i;
   glBegin(GL_POLYGON);
      for(i = 0; i < n; i++)
         glVertex3f(pts[i].x, pts[i].y, pts[i].z+.01);
   glEnd();
   
   // outline 
   glBegin(GL_LINE_LOOP);
      glColor3f(0, 0, 0);
      for(i = 0; i < n; i++)
         glVertex3f(pts[i].x, pts[i].y, pts[i].z+.02);
   glEnd();     
}

//----------------------------------------------------------------------------

void FloorPlan::drawEdge(Point3F p1, Point3F p2)
{
   glBegin(GL_LINES);
      glColor3f(0, 1, 0);
      glVertex3f(p1.x, p1.y, (p1.z+.2));
      glVertex3f(p2.x, p2.y, (p2.z+.2));
   glEnd();  
}

//----------------------------------------------------------------------------

void FloorPlan::drawNode(DPoint pos)
{
   glPointSize(10);
   glBegin(GL_POINTS);
      glColor3f(0, 0, 1);
      glVertex3f(pos.x, pos.y, pos.z+.2);
   glEnd();
}

//----------------------------------------------------------------------------

void FloorPlan::drawConnections(InteriorDetail *d)
{
   if(!sDrawConnections)
      return;
      
   DPoint p1, p2;
   
   U32 i;
   for(i = 0; i < d->mConnections.size(); i++)
   {   
      p1 = d->mConPoints[d->mConnections[i].start];
      p1.z += .1;
      p2 = d->mConPoints[d->mConnections[i].end];
      p2.z += .1;
      drawNode(p1);
      drawNode(p2);
      drawEdge(p1, p2);   
   }   
}

//----------------------------------------------------------------------------

void FloorPlan::drawVolumes(InteriorDetail *d)
{
   U32 i;
   for(i = 0; i < d->mVolumes.size(); i++)
   {   
      //drawEdge(d->mVolumes[i].surfacePtr->mCntr, d->mVolumes[i].capPt);
   }        
}

//----------------------------------------------------------------------------

void GroundPlan::render(Point3F &camPos, bool drawClipped)
{
   U32 i;
   S32 dataBaseSize = mGridDatabase.size();
   if( ! dataBaseSize )
      return;
   
   // 2 methods for render
   if(drawClipped)
   {
      Point2I gPos;
      S32 index;
   
      worldToGrid(camPos, gPos);
      if(gPos.x < mGridOrigin.x || gPos.x > (mGridOrigin.x + mGridDims.x))
         return;

      index = gridToIndex(gPos);
      
      S32 start, end;
      S32 xspan, yspan;
      getClippedRegion(index, start, end, xspan, yspan);
   
      // begin rendering
      S32 x, y;
      for(y = 0; y < yspan; y++)
      {
         for(x = start + (y * mGridDims.x); x < ((start + (y * mGridDims.x)) + xspan); x++)
         {
            index = x;
            
            // don't draw invalid node indexes
            if(index > dataBaseSize || index < 0)
               continue;

            GridDetail &grid = mGridDatabase[index];
      
            // clear node
            if(grid.mNavFlag == GridDetail::clear)
            {   
               Point3F pos1 = grid.mWorld[0][0];
               pos1.z += 1;      
               wireCube(1, pos1);
            }
            else if(grid.mNavFlag == GridDetail::shadowed)
            {   
               Point3F pos2 = grid.mWorld[0][0];
               pos2.z += 1;      
               wireCube(1, pos2);
               drawShadow(grid, index);
            }
            drawObstruct(grid, index);

            // draw a network of connections for this node
            drawNeighbor(grid, index);
         } 
      }
   }
   
   // draw the whole graph
   else
   {
      for(i = 0; i < dataBaseSize; i++)
      {
         GridDetail &grid = mGridDatabase[i];
         
         // clear node
         /*if(grid.mNavFlag == GridDetail::clear)
            {   
            Point3F pos1 = grid.mWorld[0][0];
            pos1.z += 1;      
            wireCube(1, pos1);
         }
         else if(grid.mNavFlag == GridDetail::shadowed)
         {   
            Point3F pos2 = grid.mWorld[0][0];
            pos2.z += 1;      
            wireCube(1, pos2);
            drawShadow(grid, i);
         }*/
         drawObstruct(grid, i);
         //drawNeighbor(grid, i);
      }
   }
}

//----------------------------------------------------------------------------
// these were more for testing
void GroundPlan::drawShadow(GridDetail &grid, S32 index)
{
   Point2I *gPos;
   Point2I cPos;
   Point3F  wMin;
   Point3F  wMax;
   gPos = getPosition(index);   
   
   if(!gPos)
      return;
   
   cPos = *gPos;
   gridToWorld(cPos, wMin);
   gridToWorld(cPos + gridOffset[7], wMax);
   
   glBegin(GL_LINES);
      glColor3f(1.0, 0.0, 0.0);
      glVertex3f(grid.mWorld[0][0].x, grid.mWorld[0][0].y, grid.mWorld[0][0].z);
      glVertex3f(grid.mWorld[0][0].x, grid.mWorld[0][0].y, grid.mWorld[0][0].z+grid.mShadowHt);
   glEnd();   
}

//----------------------------------------------------------------------------

void GroundPlan::drawObstruct(GridDetail &grid, S32 index)
{
   Point2I *gPos;
   Point2I cPos;
   Point3F  wMin;
   Point3F  wMax;
   gPos = getPosition(index);   
   
   if(!gPos)
      return;
   
   cPos = *gPos;
   gridToWorld(cPos, wMin);
   gridToWorld(cPos + gridOffset[7], wMax);
   
   for(U16 i = 0; i < 2; i++)
   {
      if(grid.mNavFlags[i] == GridDetail::obstructed)
      {   
         glBegin(GL_TRIANGLES);
            glColor3f(1, 0, 0);
            glVertex3f(grid.mWorld[i][0].x, grid.mWorld[i][0].y, grid.mWorld[i][0].z+.5);
            glVertex3f(grid.mWorld[i][1].x, grid.mWorld[i][1].y, grid.mWorld[i][1].z+.5);
            glVertex3f(grid.mWorld[i][2].x, grid.mWorld[i][2].y, grid.mWorld[i][2].z+.5);
         glEnd();
      }
   }
}

//----------------------------------------------------------------------------

void GroundPlan::drawNeighbor(GridDetail &grid, S32 i)
{
   Point3F to, nodeP;
   nodeP = grid.mWorld[0][0];
   nodeP.z += 1;
   GridDetail *g;
   
   bool isBottomRow = false;
   bool isLeftEdge = false;
   bool isRightEdge = false;
   bool isTopRow = false;
   
   if(i % mGridDims.x == 0)
      isLeftEdge = true;
   if(i < mGridDims.x)
      isBottomRow = true;
   if(((i+1) % mGridDims.x) == 0)
      isRightEdge = true;
   if(i >= ((mGridDims.x * mGridDims.y) - mGridDims.x))
      isTopRow = true;

   if(!isLeftEdge && !isBottomRow)
   {
      if(!isTopRow && !isRightEdge)
      {
         if(grid.mNeighbors.test(BIT(0)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 0)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
         if(grid.mNeighbors.test(BIT(1)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 1)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
         if(grid.mNeighbors.test(BIT(2)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 2)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
         if(grid.mNeighbors.test(BIT(3)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 3)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
         if(grid.mNeighbors.test(BIT(4)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 4)]);
            to = g->mWorld[0][0];
            to.z += 1;
           drawConnection(nodeP, to);   
         }
         if(grid.mNeighbors.test(BIT(5)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 5)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
         if(grid.mNeighbors.test(BIT(6)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 6)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
         if(grid.mNeighbors.test(BIT(7)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 7)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
      }
      else if(!isTopRow)
      {
         if(grid.mNeighbors.test(BIT(0)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 0)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
         if(grid.mNeighbors.test(BIT(1)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 1)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
         if(grid.mNeighbors.test(BIT(2)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 2)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
         if(grid.mNeighbors.test(BIT(4)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 4)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
         if(grid.mNeighbors.test(BIT(6)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 6)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
      }
      else if(!isRightEdge)
      {
         if(grid.mNeighbors.test(BIT(0)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 0)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
         if(grid.mNeighbors.test(BIT(1)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 1)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
         if(grid.mNeighbors.test(BIT(2)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 2)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
         if(grid.mNeighbors.test(BIT(3)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 3)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
         if(grid.mNeighbors.test(BIT(5)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 5)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
      }
      else
      {
         if(grid.mNeighbors.test(BIT(0)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 0)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
         if(grid.mNeighbors.test(BIT(1)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 1)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
         if(grid.mNeighbors.test(BIT(2)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 2)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
      }
   }
   else if(!isBottomRow)
   {
      if(!isTopRow)
      {
         if(grid.mNeighbors.test(BIT(1)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 1)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
         if(grid.mNeighbors.test(BIT(3)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 3)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
         if(grid.mNeighbors.test(BIT(5)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 5)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
         if(grid.mNeighbors.test(BIT(6)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 6)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
         if(grid.mNeighbors.test(BIT(7)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 7)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
      }
      else
      {
         if(grid.mNeighbors.test(BIT(3)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 3)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
         if(grid.mNeighbors.test(BIT(6)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 6)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
         if(grid.mNeighbors.test(BIT(7)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 7)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
      }
   }
   else if(!isLeftEdge)
   {
      if(!isRightEdge)
      {
         if(grid.mNeighbors.test(BIT(2)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 2)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
         if(grid.mNeighbors.test(BIT(4)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 4)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
         if(grid.mNeighbors.test(BIT(5)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 5)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
         if(grid.mNeighbors.test(BIT(6)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 6)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
         if(grid.mNeighbors.test(BIT(7)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 7)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
      }
      else
      {
         if(grid.mNeighbors.test(BIT(2)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 2)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
         if(grid.mNeighbors.test(BIT(4)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 4)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
         if(grid.mNeighbors.test(BIT(6)))
         {
            g = &(mGridDatabase[getNeighborDetails(i, 6)]);
            to = g->mWorld[0][0];
            to.z += 1;
            drawConnection(nodeP, to);   
         }
      }
   }
   else
   {
      if(grid.mNeighbors.test(BIT(5)))
      {
            g = &(mGridDatabase[getNeighborDetails(i, 5)]);
         to = g->mWorld[0][0];
         to.z += 1;
         drawConnection(nodeP, to);   
      }
      if(grid.mNeighbors.test(BIT(6)))
      {
            g = &(mGridDatabase[getNeighborDetails(i, 6)]);
         to = g->mWorld[0][0];
         to.z += 1;
         drawConnection(nodeP, to);   
      }
      if(grid.mNeighbors.test(BIT(7)))
      {
            g = &(mGridDatabase[getNeighborDetails(i, 7)]);
         to = g->mWorld[0][0];
         to.z += 1;
         drawConnection(nodeP, to);   
      }
   }
}

//----------------------------------------------------------------------------

void GroundPlan::drawConnection(Point3F &st, Point3F &ed)
{
   glBegin(GL_LINES);
      glColor3f(0.0, 0.0, 1.0);
      glVertex3f(st.x, st.y, st.z);
      glVertex3f(ed.x, ed.y, ed.z);
   glEnd();
}

//----------------------------------------------------------------------------
