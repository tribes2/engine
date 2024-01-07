//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "Platform/platform.h"
#include "console/console.h"
#include "Math/mMathFn.h"
#include "Math/mRandom.h"

static const char *cSolveQuadratic(SimObject *, S32, const char **argv)
{
   char * retBuffer = Con::getReturnBuffer(256);
   F32 x[2];
   U32 sol = mSolveQuadratic(dAtof(argv[1]), dAtof(argv[2]), dAtof(argv[3]), x);
   dSprintf(retBuffer, 256, "%d %g %g", sol, x[0], x[1]);
   return retBuffer;
}

static const char *cSolveCubic(SimObject *, S32, const char **argv)
{
   char * retBuffer = Con::getReturnBuffer(256);
   F32 x[3];
   U32 sol = mSolveCubic(dAtof(argv[1]), dAtof(argv[2]), dAtof(argv[3]), dAtof(argv[4]), x);
   dSprintf(retBuffer, 256, "%d %g %g %g", sol, x[0], x[1], x[2]);
   return retBuffer;
}

static const char *cSolveQuartic(SimObject *, S32, const char **argv)
{
   char * retBuffer = Con::getReturnBuffer(256);
   F32 x[4];
   U32 sol = mSolveQuartic(dAtof(argv[1]), dAtof(argv[2]), dAtof(argv[3]), dAtof(argv[4]), dAtof(argv[5]), x);
   dSprintf(retBuffer, 256, "%d %g %g %g %g", sol, x[0], x[1], x[2], x[3]);
   return retBuffer;
}

static S32 cFloor(SimObject *, S32, const char **argv)
{
   return (S32)mFloor(dAtof(argv[1]));
}

static S32 cCeil(SimObject *, S32, const char **argv)
{
   return (S32)mCeil(dAtof(argv[1]));
}

static const char * cFloatLength(SimObject *, S32, const char **argv)
{
   char * outBuffer = Con::getReturnBuffer(256);
   char fmtString[8] = "%.0f";
   U32 precision = dAtoi(argv[2]);
   if (precision > 9)
      precision = 9;
   fmtString[2] = '0' + precision;

   dSprintf(outBuffer, 255, fmtString, dAtof(argv[1]));
   return outBuffer;
}

//------------------------------------------------------------------------------

static F32 cAbs(SimObject *, S32, const char ** argv)
{
   return(mFabs(dAtof(argv[1])));
}

static F32 cSqrt(SimObject *, S32, const char ** argv)
{
   return(mSqrt(dAtof(argv[1])));
}

static F32 cPow(SimObject *, S32, const char ** argv)
{
   return(mPow(dAtof(argv[1]), dAtof(argv[2])));
}

static F32 cLog(SimObject *, S32, const char ** argv)
{
   return(mLog(dAtof(argv[1])));
}

static F32 cSin(SimObject *, S32, const char ** argv)
{
   return(mSin(dAtof(argv[1])));
}

static F32 cCos(SimObject *, S32, const char ** argv)
{
   return(mCos(dAtof(argv[1])));
}

static F32 cTan(SimObject *, S32, const char ** argv)
{
   return(mTan(dAtof(argv[1])));
}

static F32 cAsin(SimObject *, S32, const char ** argv)
{
   return(mAsin(dAtof(argv[1])));
}

static F32 cAcos(SimObject *, S32, const char ** argv)
{
   return(mAcos(dAtof(argv[1])));
}

static F32 cAtan(SimObject *, S32, const char ** argv)
{
   return(mAtan(dAtof(argv[1]), dAtof(argv[2])));
}

static F32 cRadToDeg(SimObject *, S32, const char ** argv)
{
   return(mRadToDeg(dAtof(argv[1])));
}

static F32 cDegToRad(SimObject *, S32, const char ** argv)
{
   return(mDegToRad(dAtof(argv[1])));
}

//------------------------------------------------------------------------------

void MathConsoleInit()
{
   Con::addCommand("mSolveQuadratic", cSolveQuadratic, "mSolveQuadratic(a,b,c)", 4, 4);
   Con::addCommand("mSolveCubic", cSolveCubic, "mSolveCubic(a,b,c,d)", 5, 5);
   Con::addCommand("mSolveQuartic", cSolveQuartic, "mSolveQuartic(a,b,c,d,e)", 6, 6);

   Con::addCommand("mFloor", cFloor, "mFloor(float)", 2, 2);
   Con::addCommand("mCeil", cCeil, "mCeil(float)", 2, 2);
   Con::addCommand("mFloatLength", cFloatLength, "mFloatLength(float, numDecimals)", 3, 3);

   //
   Con::addCommand("mAbs",  cAbs,  "mAbs(float)", 2, 2);
   Con::addCommand("mSqrt", cSqrt, "mSqrt(float)", 2, 2);
   Con::addCommand("mPow", cPow, "mPow(float, float)", 3, 3);
   Con::addCommand("mLog", cLog, "mLog(float)", 2, 2);
   Con::addCommand("mSin", cSin, "mSin(float)", 2, 2);
   Con::addCommand("mCos", cCos, "mCos(float)", 2, 2);
   Con::addCommand("mTan", cTan, "mTan(float)", 2, 2);
   Con::addCommand("mAsin", cAsin, "mAsin(float)", 2, 2);
   Con::addCommand("mAcos", cAcos, "mAcos(float)", 2, 2);
   Con::addCommand("mAtan", cAtan, "mAtan(float, float)", 3, 3);
   Con::addCommand("mRadToDeg", cRadToDeg, "mRadToDeg(float)", 2, 2);
   Con::addCommand("mDegToRad", cDegToRad, "mDegToRad(float)", 2, 2);
}
