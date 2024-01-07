//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "Core/bitRender.h"

U32 openTable[32] =  { 0xFFFFFFFF,0xFFFFFFFE,0xFFFFFFFC,0xFFFFFFF8,
                       0xFFFFFFF0,0xFFFFFFE0,0xFFFFFFC0,0xFFFFFF80,
                       0xFFFFFF00,0xFFFFFE00,0xFFFFFC00,0xFFFFF800,
                       0xFFFFF000,0xFFFFE000,0xFFFFC000,0xFFFF8000,
                       0xFFFF0000,0xFFFE0000,0xFFFC0000,0xFFF80000,
                       0xFFF00000,0xFFE00000,0xFFC00000,0xFF800000,
                       0xFF000000,0xFE000000,0xFC000000,0xF8000000,
                       0xF0000000,0xE0000000,0xC0000000,0x80000000 };

U32 closeTable[32] = { 0x00000001,0x00000003,0x00000007,0x0000000F,
                       0x0000001F,0x0000003F,0x0000007F,0x000000FF,
                       0x000001FF,0x000003FF,0x000007FF,0x00000FFF,
                       0x00001FFF,0x00003FFF,0x00007FFF,0x0000FFFF,
                       0x0001FFFF,0x0003FFFF,0x0007FFFF,0x000FFFFF,
                       0x001FFFFF,0x003FFFFF,0x007FFFFF,0x00FFFFFF,
                       0x01FFFFFF,0x03FFFFFF,0x07FFFFFF,0x0FFFFFFF,
                       0x1FFFFFFF,0x3FFFFFFF,0x7FFFFFFF,0xFFFFFFFF };

struct DrawStruct
{
   S16 start;
   S16 num;
};

void BitRender::render_strips(const U8 * draw, S32 numDraw, S32 szDraw, const U16 * indices, const Point2I * points, S32 dim, U32 * bits)
{
   const U8 * drawEnd = draw + numDraw*szDraw;

   // loop through strips...
   for (; draw<drawEnd; draw += szDraw)
   {
      const DrawStruct * drawStruct = (DrawStruct*)draw;
      const U16 * icurrent = indices + drawStruct->start;
      const U16 * iend     = icurrent + drawStruct->num;

      const Point2I * vv0 = points + *(icurrent++);
      const Point2I * vv1;
      const Point2I * vv2 = points + *(icurrent++);
      const Point2I **nextPt = &vv1;

      while (icurrent<iend)
      {
         *nextPt = vv2;
         nextPt = (const Point2I**)( (S32)nextPt ^ (S32)&vv0 ^ (S32)&vv1 );
         vv2 = points + *(icurrent++);

         // skip degenerate triangles...
         if (vv0==vv1 || vv1==vv2 || vv2==vv0)
            continue;
         // following copied from BitRender::render...indented to highlight this fact, no function to indentation
         {
            const Point2I * v0 = vv0;
            const Point2I * v1 = vv1;
            const Point2I * v2 = vv2;

            AssertFatal( v0->x>=0 && v0->x<dim && v0->y>=0 && v0->y<dim,"BitRender::render: v0 out of range");
            AssertFatal( v1->x>=0 && v1->x<dim && v1->y>=0 && v1->y<dim,"BitRender::render: v1 out of range");
            AssertFatal( v2->x>=0 && v2->x<dim && v2->y>=0 && v2->y<dim,"BitRender::render: v2 out of range");

            // back-face cull
            if ((v0->x-v1->x)*(v2->y-v1->y) > (v0->y-v1->y)*(v2->x-v1->x))
               continue;

            // rotate so that v0 holds topmost y coord
            // Note: The particular inequalities used ( <=, <, then <=) are important.
            //       They guarantee that if there are two verts on the top row, that
            //       they will be vert v0 and v1 (also could be three).
            if (v1->y <= v2->y)
            {
               if (v1->y < v0->y)
               {
                  const Point2I * tmp = v0;
                  v0 = v1;
                  v1 = v2;
                  v2 = tmp;
               }
            }
            else
            {
               if (v2->y <= v0->y)
               {
                  const Point2I * tmp = v0;
                  v0 = v2;
                  v2 = v1;
                  v1 = tmp;
               }
            }

            // all the control variables we have to set up...
            S32 leftDeltaY, xLeftInc, xLeftErrInc, xLeftDir, xLeft, xLeftErr = 0;
            S32 rightDeltaY, xRightInc, xRightErrInc, xRightDir, xRight, xRightErr = 0;
            S32 * changeThisDelta;
            S32 * changeThisInc;
            S32 * changeThisErrInc;
            S32 * changeThisDir;
            S32 toThisDelta;
            S32 toThisInc;
            S32 toThisErrInc;
            S32 toThisDir;
            S32 ySwitch;
            S32 yBottom;

            // check for special case where top row holds two verts
            if (v0->y==v1->y)
            {
               leftDeltaY = v2->y-v0->y;
               rightDeltaY = v2->y-v1->y;
               ySwitch = v2->y;
               yBottom = v2->y;

               if (v1->y==v2->y)
               {
                  // special-special case where top row holds all three verts
                  xLeft = getMin(getMin(v0->x,v1->x),v2->x);
                  xRight = getMax(getMax(v0->x,v1->x),v2->x);
                  xLeftErrInc = xRightErrInc = 0;
               }
               else
               {
                  // standard-special case...v2 on different row from v0 and v1
                  xLeft = v0->x;
                  xLeftInc = (v2->x-v0->x) / leftDeltaY;
                  xLeftErrInc = (v2->x-v0->x) - leftDeltaY*xLeftInc;
                  xLeftDir = v2->x-v0->x > 0 ? 1 : -1;

                  xRight = v1->x;
                  xRightInc = (v2->x-v1->x) / rightDeltaY;
                  xRightErrInc = (v2->x-v1->x) - rightDeltaY*xRightInc;
                  xRightDir = v2->x-v1->x > 0 ? 1 : -1;
               }

               // set these variables to avoid a crash...
               changeThisDelta = &toThisDelta;
               changeThisInc = &toThisInc;
               changeThisErrInc = &toThisErrInc;
               changeThisDir = &toThisDir;
            }
            else
            {
               leftDeltaY = v2->y-v0->y;
               xLeftInc = (v2->x-v0->x) / leftDeltaY;
               xLeftErrInc = (v2->x-v0->x) - leftDeltaY*xLeftInc;
               xLeftDir = v2->x-v0->x > 0 ? 1 : -1;

               rightDeltaY = v1->y-v0->y;
               xRightInc = (v1->x-v0->x) / rightDeltaY;
               xRightErrInc = (v1->x-v0->x) - rightDeltaY*xRightInc;
               xRightDir = v1->x-v0->x > 0 ? 1 : -1;
               
               xLeft = xRight = v0->x;

               if (v1->y<v2->y)
               {
                  // right edge bends
                  changeThisDelta = &rightDeltaY;
                  changeThisInc = &xRightInc;
                  changeThisErrInc = &xRightErrInc;
                  changeThisDir = &xRightDir;
                  toThisDelta = v2->y-v1->y;
                  toThisInc = (v2->x-v1->x) / toThisDelta;
                  toThisErrInc = (v2->x-v1->x) - toThisDelta * toThisInc;
                  toThisDir = v2->x-v1->x > 0 ? 1 : -1;
                  ySwitch = v1->y-1;
                  yBottom = v2->y;
               }
               else
               {
                  // left edge bends
                  changeThisDelta = &leftDeltaY;
                  changeThisInc = &xLeftInc;
                  changeThisErrInc = &xLeftErrInc;
                  changeThisDir = &xLeftDir;
                  toThisDelta = v1->y-v2->y;
                  toThisInc = toThisDelta ? (v1->x-v2->x) / toThisDelta : 0;
                  toThisErrInc = toThisDelta ? (v1->x-v2->x) - toThisDelta * toThisInc : 0;
                  toThisDir = v1->x-v2->x > 0 ? 1 : -1;
                  ySwitch = v2->y-1;
                  yBottom = v1->y;
               }
            }
            xLeftErrInc *= xLeftDir;
            xRightErrInc *= xRightDir;
            toThisErrInc *= toThisDir;

            U32 * rowStart = bits + v0->y * (dim>>5);
            for (S32 y=v0->y; y<=yBottom;)
            {
               do
               {
                  AssertFatal(xLeft<=xRight,"BitRender::render");

                  U32 open  = openTable[xLeft&31];
                  U32 close = closeTable[xRight&31];
                  if ( (xLeft^xRight) & ~31)
                  {
                     U32 * x = rowStart+(xLeft>>5);
                     *x |= open;
                     U32 * xEnd = rowStart+(xRight>>5);
                     while (++x<xEnd)
                        *x |= 0xFFFFFFFF;
                     *x |= close;
                  }
                  else
                     rowStart[xLeft>>5] |= open & close;

                  xLeft     += xLeftInc;
                  xLeftErr  += xLeftErrInc;
                  if (xLeftErr >= leftDeltaY)
                  {
                     xLeft += xLeftDir;
                     xLeftErr -= leftDeltaY;
                  }

                  xRight     += xRightInc;
                  xRightErr  += xRightErrInc;
                  if (xRightErr >= rightDeltaY)
                  {
                     xRight += xRightDir;
                     xRightErr -= rightDeltaY;
                  }

                  rowStart += dim>>5;

               } while (y++<ySwitch);

               ySwitch=yBottom; // get here at most twice
               *changeThisDelta = toThisDelta;
               *changeThisInc = toThisInc;
               *changeThisErrInc = toThisErrInc;
               *changeThisDir = toThisDir;
            }

         }
      }
   }
}

void BitRender::render_tris(const U8 * draw, S32 numDraw, S32 szDraw, const U16 * indices, const Point2I * points, S32 dim, U32 * bits)
{
   const U8 * drawEnd = draw + numDraw*szDraw;

   // loop through strips...
   for (; draw<drawEnd; draw += szDraw)
   {
      const DrawStruct * drawStruct = (DrawStruct*)draw;
      const U16 * icurrent = indices  + drawStruct->start;
      const U16 * iend     = icurrent + drawStruct->num;

      while (icurrent<iend)
      {
         const Point2I * v0 = points + *(icurrent++);
         const Point2I * v1 = points + *(icurrent++);
         const Point2I * v2 = points + *(icurrent++);

         // following copied from BitRender::render...indented to highlight this fact, no function to indentation
         {
            AssertFatal( v0->x>=0 && v0->x<dim && v0->y>=0 && v0->y<dim,"BitRender::render: v0 out of range");
            AssertFatal( v1->x>=0 && v1->x<dim && v1->y>=0 && v1->y<dim,"BitRender::render: v1 out of range");
            AssertFatal( v2->x>=0 && v2->x<dim && v2->y>=0 && v2->y<dim,"BitRender::render: v2 out of range");

            // back-face cull
            if ((v0->x-v1->x)*(v2->y-v1->y) > (v0->y-v1->y)*(v2->x-v1->x))
               return;

            // rotate so that v0 holds topmost y coord
            // Note: The particular inequalities used ( <=, <, then <=) are important.
            //       They guarantee that if there are two verts on the top row, that
            //       they will be vert v0 and v1 (also could be three).
            if (v1->y <= v2->y)
            {
               if (v1->y < v0->y)
               {
                  const Point2I * tmp = v0;
                  v0 = v1;
                  v1 = v2;
                  v2 = tmp;
               }
            }
            else
            {
               if (v2->y <= v0->y)
               {
                  const Point2I * tmp = v0;
                  v0 = v2;
                  v2 = v1;
                  v1 = tmp;
               }
            }

            // all the control variables we have to set up...
            S32 leftDeltaY, xLeftInc, xLeftErrInc, xLeftDir, xLeft, xLeftErr = 0;
            S32 rightDeltaY, xRightInc, xRightErrInc, xRightDir, xRight, xRightErr = 0;
            S32 * changeThisDelta;
            S32 * changeThisInc;
            S32 * changeThisErrInc;
            S32 * changeThisDir;
            S32 toThisDelta;
            S32 toThisInc;
            S32 toThisErrInc;
            S32 toThisDir;
            S32 ySwitch;
            S32 yBottom;

            // check for special case where top row holds two verts
            if (v0->y==v1->y)
            {
               leftDeltaY = v2->y-v0->y;
               rightDeltaY = v2->y-v1->y;
               ySwitch = v2->y;
               yBottom = v2->y;

               if (v1->y==v2->y)
               {
                  // special-special case where top row holds all three verts
                  xLeft = getMin(getMin(v0->x,v1->x),v2->x);
                  xRight = getMax(getMax(v0->x,v1->x),v2->x);
                  xLeftErrInc = xRightErrInc = 0;
               }
               else
               {
                  // standard-special case...v2 on different row from v0 and v1
                  xLeft = v0->x;
                  xLeftInc = (v2->x-v0->x) / leftDeltaY;
                  xLeftErrInc = (v2->x-v0->x) - leftDeltaY*xLeftInc;
                  xLeftDir = v2->x-v0->x > 0 ? 1 : -1;

                  xRight = v1->x;
                  xRightInc = (v2->x-v1->x) / rightDeltaY;
                  xRightErrInc = (v2->x-v1->x) - rightDeltaY*xRightInc;
                  xRightDir = v2->x-v1->x > 0 ? 1 : -1;
               }

               // set these variables to avoid a crash...
               changeThisDelta = &toThisDelta;
               changeThisInc = &toThisInc;
               changeThisErrInc = &toThisErrInc;
               changeThisDir = &toThisDir;
            }
            else
            {
               leftDeltaY = v2->y-v0->y;
               xLeftInc = (v2->x-v0->x) / leftDeltaY;
               xLeftErrInc = (v2->x-v0->x) - leftDeltaY*xLeftInc;
               xLeftDir = v2->x-v0->x > 0 ? 1 : -1;

               rightDeltaY = v1->y-v0->y;
               xRightInc = (v1->x-v0->x) / rightDeltaY;
               xRightErrInc = (v1->x-v0->x) - rightDeltaY*xRightInc;
               xRightDir = v1->x-v0->x > 0 ? 1 : -1;
               
               xLeft = xRight = v0->x;

               if (v1->y<v2->y)
               {
                  // right edge bends
                  changeThisDelta = &rightDeltaY;
                  changeThisInc = &xRightInc;
                  changeThisErrInc = &xRightErrInc;
                  changeThisDir = &xRightDir;
                  toThisDelta = v2->y-v1->y;
                  toThisInc = (v2->x-v1->x) / toThisDelta;
                  toThisErrInc = (v2->x-v1->x) - toThisDelta * toThisInc;
                  toThisDir = v2->x-v1->x > 0 ? 1 : -1;
                  ySwitch = v1->y-1;
                  yBottom = v2->y;
               }
               else
               {
                  // left edge bends
                  changeThisDelta = &leftDeltaY;
                  changeThisInc = &xLeftInc;
                  changeThisErrInc = &xLeftErrInc;
                  changeThisDir = &xLeftDir;
                  toThisDelta = v1->y-v2->y;
                  toThisInc = toThisDelta ? (v1->x-v2->x) / toThisDelta : 0;
                  toThisErrInc = toThisDelta ? (v1->x-v2->x) - toThisDelta * toThisInc : 0;
                  toThisDir = v1->x-v2->x > 0 ? 1 : -1;
                  ySwitch = v2->y-1;
                  yBottom = v1->y;
               }
            }
            xLeftErrInc *= xLeftDir;
            xRightErrInc *= xRightDir;
            toThisErrInc *= toThisDir;

            U32 * rowStart = bits + v0->y * (dim>>5);
            for (S32 y=v0->y; y<=yBottom;)
            {
               do
               {
                  AssertFatal(xLeft<=xRight,"BitRender::render");

                  U32 open  = openTable[xLeft&31];
                  U32 close = closeTable[xRight&31];
                  if ( (xLeft^xRight) & ~31)
                  {
                     U32 * x = rowStart+(xLeft>>5);
                     *x |= open;
                     U32 * xEnd = rowStart+(xRight>>5);
                     while (++x<xEnd)
                        *x |= 0xFFFFFFFF;
                     *x |= close;
                  }
                  else
                     rowStart[xLeft>>5] |= open & close;

                  xLeft     += xLeftInc;
                  xLeftErr  += xLeftErrInc;
                  if (xLeftErr >= leftDeltaY)
                  {
                     xLeft += xLeftDir;
                     xLeftErr -= leftDeltaY;
                  }

                  xRight     += xRightInc;
                  xRightErr  += xRightErrInc;
                  if (xRightErr >= rightDeltaY)
                  {
                     xRight += xRightDir;
                     xRightErr -= rightDeltaY;
                  }

                  rowStart += dim>>5;

               } while (y++<ySwitch);

               ySwitch=yBottom; // get here at most twice
               *changeThisDelta = toThisDelta;
               *changeThisInc = toThisInc;
               *changeThisErrInc = toThisErrInc;
               *changeThisDir = toThisDir;
            }

         }
      }
   }
}

// assumes coords are in range
void BitRender::render(const Point2I * v0, const Point2I * v1, const Point2I * v2, S32 dim, U32 * bits)
{
   AssertFatal( v0->x>=0 && v0->x<dim && v0->y>=0 && v0->y<dim,"BitRender::render: v0 out of range");
   AssertFatal( v1->x>=0 && v1->x<dim && v1->y>=0 && v1->y<dim,"BitRender::render: v1 out of range");
   AssertFatal( v2->x>=0 && v2->x<dim && v2->y>=0 && v2->y<dim,"BitRender::render: v2 out of range");

   // back-face cull
   if ((v0->x-v1->x)*(v2->y-v1->y) > (v0->y-v1->y)*(v2->x-v1->x))
      return;

   // rotate so that v0 holds topmost y coord
   // Note: The particular inequalities used ( <=, <, then <=) are important.
   //       They guarantee that if there are two verts on the top row, that
   //       they will be vert v0 and v1 (also could be three).
   if (v1->y <= v2->y)
   {
      if (v1->y < v0->y)
      {
         const Point2I * tmp = v0;
         v0 = v1;
         v1 = v2;
         v2 = tmp;
      }
   }
   else
   {
      if (v2->y <= v0->y)
      {
         const Point2I * tmp = v0;
         v0 = v2;
         v2 = v1;
         v1 = tmp;
      }
   }

   // all the control variables we have to set up...
   S32 leftDeltaY, xLeftInc, xLeftErrInc, xLeftDir, xLeft, xLeftErr = 0;
   S32 rightDeltaY, xRightInc, xRightErrInc, xRightDir, xRight, xRightErr = 0;
   S32 * changeThisDelta;
   S32 * changeThisInc;
   S32 * changeThisErrInc;
   S32 * changeThisDir;
   S32 toThisDelta;
   S32 toThisInc;
   S32 toThisErrInc;
   S32 toThisDir;
   S32 ySwitch;
   S32 yBottom;

   // check for special case where top row holds two verts
   if (v0->y==v1->y)
   {
      leftDeltaY = v2->y-v0->y;
      rightDeltaY = v2->y-v1->y;
      ySwitch = v2->y;
      yBottom = v2->y;

      if (v1->y==v2->y)
      {
         // special-special case where top row holds all three verts
         xLeft = getMin(getMin(v0->x,v1->x),v2->x);
         xRight = getMax(getMax(v0->x,v1->x),v2->x);
         xLeftErrInc = xRightErrInc = 0;
      }
      else
      {
         // standard-special case...v2 on different row from v0 and v1
         xLeft = v0->x;
         xLeftInc = (v2->x-v0->x) / leftDeltaY;
         xLeftErrInc = (v2->x-v0->x) - leftDeltaY*xLeftInc;
         xLeftDir = v2->x-v0->x > 0 ? 1 : -1;

         xRight = v1->x;
         xRightInc = (v2->x-v1->x) / rightDeltaY;
         xRightErrInc = (v2->x-v1->x) - rightDeltaY*xRightInc;
         xRightDir = v2->x-v1->x > 0 ? 1 : -1;
      }

      // set these variables to avoid a crash...
      changeThisDelta = &toThisDelta;
      changeThisInc = &toThisInc;
      changeThisErrInc = &toThisErrInc;
      changeThisDir = &toThisDir;
   }
   else
   {
      leftDeltaY = v2->y-v0->y;
      xLeftInc = (v2->x-v0->x) / leftDeltaY;
      xLeftErrInc = (v2->x-v0->x) - leftDeltaY*xLeftInc;
      xLeftDir = v2->x-v0->x > 0 ? 1 : -1;

      rightDeltaY = v1->y-v0->y;
      xRightInc = (v1->x-v0->x) / rightDeltaY;
      xRightErrInc = (v1->x-v0->x) - rightDeltaY*xRightInc;
      xRightDir = v1->x-v0->x > 0 ? 1 : -1;
      
      xLeft = xRight = v0->x;

      if (v1->y<v2->y)
      {
         // right edge bends
         changeThisDelta = &rightDeltaY;
         changeThisInc = &xRightInc;
         changeThisErrInc = &xRightErrInc;
         changeThisDir = &xRightDir;
         toThisDelta = v2->y-v1->y;
         toThisInc = (v2->x-v1->x) / toThisDelta;
         toThisErrInc = (v2->x-v1->x) - toThisDelta * toThisInc;
         toThisDir = v2->x-v1->x > 0 ? 1 : -1;
         ySwitch = v1->y-1;
         yBottom = v2->y;
      }
      else
      {
         // left edge bends
         changeThisDelta = &leftDeltaY;
         changeThisInc = &xLeftInc;
         changeThisErrInc = &xLeftErrInc;
         changeThisDir = &xLeftDir;
         toThisDelta = v1->y-v2->y;
         toThisInc = toThisDelta ? (v1->x-v2->x) / toThisDelta : 0;
         toThisErrInc = toThisDelta ? (v1->x-v2->x) - toThisDelta * toThisInc : 0;
         toThisDir = v1->x-v2->x > 0 ? 1 : -1;
         ySwitch = v2->y-1;
         yBottom = v1->y;
      }
   }
   xLeftErrInc *= xLeftDir;
   xRightErrInc *= xRightDir;
   toThisErrInc *= toThisDir;

   U32 * rowStart = bits + v0->y * (dim>>5);
   for (S32 y=v0->y; y<=yBottom;)
   {
      do
      {
         AssertFatal(xLeft<=xRight,"BitRender::render");

         U32 open  = openTable[xLeft&31];
         U32 close = closeTable[xRight&31];
         if ( (xLeft^xRight) & ~31)
         {
            U32 * x = rowStart+(xLeft>>5);
            *x |= open;
            U32 * xEnd = rowStart+(xRight>>5);
            while (++x<xEnd)
               *x |= 0xFFFFFFFF;
            *x |= close;
         }
         else
            rowStart[xLeft>>5] |= open & close;

         xLeft     += xLeftInc;
         xLeftErr  += xLeftErrInc;
         if (xLeftErr >= leftDeltaY)
         {
            xLeft += xLeftDir;
            xLeftErr -= leftDeltaY;
         }

         xRight     += xRightInc;
         xRightErr  += xRightErrInc;
         if (xRightErr >= rightDeltaY)
         {
            xRight += xRightDir;
            xRightErr -= rightDeltaY;
         }

         rowStart += dim>>5;

      } while (y++<ySwitch);

      ySwitch=yBottom; // get here at most twice
      *changeThisDelta = toThisDelta;
      *changeThisInc = toThisInc;
      *changeThisErrInc = toThisErrInc;
      *changeThisDir = toThisDir;
   }
}



U8 bitTable[16][4] =
{
   {  0,  0,  0,  0}, // 0
   {255,  0,  0,  0}, // 1
   {  0,255,  0,  0}, // 2
   {255,255,  0,  0}, // 3
   {  0,  0,255,  0}, // 4
   {255,  0,255,  0}, // 5
   {  0,255,255,  0}, // 6
   {255,255,255,  0}, // 7
   {  0,  0,  0,255}, // 8
   {255,  0,  0,255}, // 9
   {  0,255,  0,255}, // 10
   {255,255,  0,255}, // 11
   {  0,  0,255,255}, // 12
   {255,  0,255,255}, // 13
   {  0,255,255,255}, // 14
   {255,255,255,255}, // 15
};

void BitRender::bitTo8Bit(U32 * bits, U32 * eightBits, S32 dim)
{
   dim *= dim>>5;
   for (S32 i=0; i<dim; i++)
   {
      U32 val = *bits++;

      *eightBits++ = *(U32*)&bitTable[val&0xF];
      val >>= 4;

      *eightBits++ = *(U32*)&bitTable[val&0xF];
      val >>= 4;

      *eightBits++ = *(U32*)&bitTable[val&0xF];
      val >>= 4;

      *eightBits++ = *(U32*)&bitTable[val&0xF];
      val >>= 4;

      *eightBits++ = *(U32*)&bitTable[val&0xF];
      val >>= 4;

      *eightBits++ = *(U32*)&bitTable[val&0xF];
      val >>= 4;

      *eightBits++ = *(U32*)&bitTable[val&0xF];
      val >>= 4;

      *eightBits++ = *(U32*)&bitTable[val&0xF];
      val >>= 4;
   }
}


U8 bitTableA[16][4] =
{
   {  0,  0,  0,  0}, // 0
   {  0,  0,  0,  0}, // 1
   {  0,  0,  0,  0}, // 2
   {  0,  0,  0,  0}, // 3
   {  0,  0,  0,  0}, // 4
   {  0,  0,  0,  0}, // 5
   {  0,  0,  0,  0}, // 6
   {  0,  0,  0,  0}, // 7
   { 17,  0,  0,  0}, // 8
   { 17,  0,  0,  0}, // 9
   { 17,  0,  0,  0}, // 10
   { 17,  0,  0,  0}, // 11
   { 17,  0,  0,  0}, // 12
   { 17,  0,  0,  0}, // 13
   { 17,  0,  0,  0}, // 14
   { 17,  0,  0,  0}, // 15
};

U8 bitTableB[16][4] =
{
   {  0,  0,  0,  0}, // 0
   { 34, 17,  0,  0}, // 1
   { 17, 34, 17,  0}, // 2
   { 51, 51, 17,  0}, // 3
   {  0, 17, 34, 17}, // 4
   { 34, 34, 34, 17}, // 5
   { 17, 51, 51, 17}, // 6
   { 51, 68, 51, 17}, // 7
   {  0,  0, 17, 34}, // 8
   { 34, 17, 17, 34}, // 9
   { 17, 34, 34, 34}, // 10
   { 51, 51, 34, 34}, // 11
   {  0, 17, 51, 51}, // 12
   { 34, 34, 51, 51}, // 13
   { 17, 51, 68, 51}, // 14
   { 51, 68, 68, 51}, // 15
};


U8 bitTableC[16][4] =
{
   {  0,  0,  0,  0}, // 0
   {  0,  0,  0, 17}, // 1
   {  0,  0,  0,  0}, // 2
   {  0,  0,  0, 17}, // 3
   {  0,  0,  0,  0}, // 4
   {  0,  0,  0, 17}, // 5
   {  0,  0,  0,  0}, // 6
   {  0,  0,  0, 17}, // 7
   {  0,  0,  0,  0}, // 8
   {  0,  0,  0, 17}, // 9
   {  0,  0,  0,  0}, // 10
   {  0,  0,  0, 17}, // 11
   {  0,  0,  0,  0}, // 12
   {  0,  0,  0, 17}, // 13
   {  0,  0,  0,  0}, // 14
   {  0,  0,  0, 17}, // 15
};

U8 bitTableE[16][4] =
{
   {  0,  0,  0,  0}, // 0
   { 51, 34,  0,  0}, // 1
   { 34, 51, 34,  0}, // 2
   { 85, 85, 34,  0}, // 3
   {  0, 34, 51, 34}, // 4
   { 51, 68, 51, 34}, // 5
   { 34, 85, 85, 34}, // 6
   { 85,119, 85, 34}, // 7
   {  0,  0, 34, 51}, // 8
   { 51, 34, 34, 51}, // 9
   { 34, 51, 68, 51}, // 10
   { 85, 85, 68, 51}, // 11
   {  0, 34, 85, 85}, // 12
   { 51, 68, 85, 85}, // 13
   { 34, 85,119, 85}, // 14
   { 85,119,119, 85}, // 15
};

void BitRender::bitTo8Bit_3(U32 * bits, U32 * eightBits, S32 dim)
{
   // clear out first row of dest
   U32 * end32 = eightBits + (dim>>2);
   do { *eightBits++=0; } while (eightBits<end32);
   end32 += (dim>>2)*(dim-1);

   U8 * p0 = (U8*)bits;
   U8 bitLo10 = 0x0F & *p0;
   U8 bitHi10 = (*p0) >> 4;
   p0++;

   U8 * p1 = (U8*)bits + (dim>>3);
   U8 bitLo11 = 0x0F & *p1;
   U8 bitHi11 = (*p1) >> 4;
   p1++;

   U8 * p2 = (U8*)bits + (dim>>2);
   U8 bitLo12 = 0x0F & *p2;
   U8 bitHi12 = (*p2) >> 4;
   p2++;

   U8 bitLo20, bitHi20;
   U8 bitLo21, bitHi21;
   U8 bitLo22, bitHi22;
   U8 bitHi00 = 0;
   U8 bitHi01 = 0;
   U8 bitHi02 = 0;

   // go thru penultimate row (but stop before last entry in that row)
   U8 * end = (U8*)bits + dim*(dim>>3) - 1;
   do
   {
      bitLo20 = 0x0F & *p0;
      bitHi20 = (*p0) >> 4;
      bitLo21 = 0x0F & *p1;
      bitHi21 = (*p1) >> 4;
      bitLo22 = 0x0F & *p2;
      bitHi22 = (*p2) >> 4;

      *eightBits++ = *(U32*)&bitTableA[bitHi00]   + *(U32*)&bitTableB[bitLo10] + *(U32*)&bitTableC[bitHi10]   +
                     *(U32*)&bitTableA[bitHi01]*2 + *(U32*)&bitTableE[bitLo11] + *(U32*)&bitTableC[bitHi11]*2 +
                     *(U32*)&bitTableA[bitHi02]   + *(U32*)&bitTableB[bitLo12] + *(U32*)&bitTableC[bitHi12];
      *eightBits++ = *(U32*)&bitTableA[bitLo10]   + *(U32*)&bitTableB[bitHi10] + *(U32*)&bitTableC[bitLo20]   +
                     *(U32*)&bitTableA[bitLo11]*2 + *(U32*)&bitTableE[bitHi11] + *(U32*)&bitTableC[bitLo21]*2 +
                     *(U32*)&bitTableA[bitLo12]   + *(U32*)&bitTableB[bitHi12] + *(U32*)&bitTableC[bitLo22];

      bitHi00 = bitHi10;
      bitLo10 = bitLo20;
      bitHi10 = bitHi20;
      bitHi01 = bitHi11;
      bitLo11 = bitLo21;
      bitHi11 = bitHi21;
      bitHi02 = bitHi12;
      bitLo12 = bitLo22;
      bitHi12 = bitHi22;

      p0++;
      p1++;
      p2++;
   }
   while (p2<end);

   // clear out last row of dest
   do { *eightBits++=0; } while (eightBits<end32);
}



