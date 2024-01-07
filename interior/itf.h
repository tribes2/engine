//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _ITF_H_
#define _ITF_H_

#ifndef _TYPES_H_
#include "platform/types.h"
#endif
#ifndef _COLOR_H
#include "core/color.h"
#endif
#ifndef _MPOINT_H_
#include "math/mPoint.h"
#endif

#ifndef _INTERIOR_H_
// redecl struct here for now... interior.h brings in the whole fricking codebase.
struct ItrPaddedPoint
{
   Point3F point;
   union {
      F32   fogCoord;
      U8    fogColor[4];
   };
};
#endif

struct OutputPoint
{
   Point3F point;
   union {
      F32    fogCoord;
      U8     fogColor[4];
   };
   Point2F texCoord;
   Point2F lmCoord;
};

struct OutputPointFC_VB
{
   Point3F point;
   U8 currentColor[4];
   U8 fogColor[4];
   Point2F texCoord;
   Point2F lmCoord;
};

struct OutputPointSP_FC_VB
{
   Point3F point;
   U8 lmColor[4];
   U8 fogColor[4];
   Point2F texCoord;
};


#ifndef TARG_MACCARB // don't bracket on mac.
extern "C" {
#endif

   void processTriFan(OutputPoint*          dst,
                      const ItrPaddedPoint* srcPoints,
                      const U32*            srcIndices,
                      const U32             numIndices);
   void processTriFanSP(OutputPoint*          dst,
                        const ItrPaddedPoint* srcPoints,
                        const U32*            srcIndices,
                        const U32             numIndices,
                        const ColorI*         srcColors);
   void processTriFanVC_TF(OutputPoint*          dst,
                           const ItrPaddedPoint* srcPoints,
                           const U32*            srcIndices,
                           const U32             numIndices,
                           const ColorI*         srcColors);
   void processTriFanSP_FC(OutputPoint*          dst,
                           const ItrPaddedPoint* srcPoints,
                           const U32*            srcIndices,
                           const U32             numIndices,
                           const ColorI*         srcColors);
   void processTriFanFC_VB(OutputPointFC_VB*			dst,
                           const ItrPaddedPoint*	srcPoints,
                           const U32*					srcIndices,
                           const U32					numIndices);
   void processTriFanSP_FC_VB(OutputPointSP_FC_VB*		dst,
                              const ItrPaddedPoint*	srcPoints,
                              const U32*					srcIndices,
                              const U32					numIndices,
                              const ColorI*				srcColors);

extern F32   texGen0[8];
extern F32   texGen1[8];
extern void* fogCoordinatePointer;

#ifndef TARG_MACCARB // don't bracket on mac.
}
#endif


#endif
