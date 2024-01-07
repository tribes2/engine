//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _LIGHTMANAGER_H_
#define _LIGHTMANAGER_H_

#ifndef _MMATH_H_
#include "math/mMath.h"
#endif
#ifndef _COLOR_H_
#include "core/color.h"
#endif
#ifndef _TVECTOR_H_
#include "core/tVector.h"
#endif
#ifndef _DATACHUNKER_H_
#include "core/dataChunker.h"
#endif

class LightInfo
{
   friend class LightManager;

   public:
      enum Type {
         Point    = 0,
         Spot     = 1,
         Vector   = 2,
         Ambient  = 3
      };
      Type        mType;

      Point3F     mPos;
      VectorF     mDirection;
      ColorF      mColor;
      ColorF      mAmbient;
      F32         mRadius;

   private:
      S32         mScore;
};
typedef Vector<LightInfo*> LightInfoList;

//------------------------------------------------------------------------------
class SimObject;
class SceneObject;
static void cResetLighting(SimObject *, S32, const char **);

class LightManager
{
   friend void cResetLighting(SimObject *, S32, const char **);

   public:
      
      enum {
         DefaultMaxLights     = 8
      };
      
      LightManager();

      void setMaxGLLights(U32);
      void registerLights(bool);
      void addLight(LightInfo *);
      void removeLight(LightInfo *);

      U32 installGLLights(const SphereF &);
      U32 installGLLights(const Box3F &);
      void uninstallGLLights();

      S32  getNumLights() const { return mLights.size(); }
      void getLights(LightInfoList& lightList) { lightList = mLights; }
      void getLights(LightInfo** lightArray)
      {
         for (U32 i = 0; i < mLights.size(); i++)
            lightArray[i] = mLights[i];
      }

      void getBestLights(const PlaneF *, const U32, const Point3F &, LightInfoList &, const U32 max = 0xffffffff);
      void getBestLights(const SphereF &, LightInfoList&, const U32 max = 0xffffffff);
      void getBestLights(const Box3F &, LightInfoList&, const U32 max = 0xffffffff);

      void setVectorLightsEnabled(bool enable)     {mVectorLightsEnabled = enable;}
      bool getVectorLightsEnabled()                {return(mVectorLightsEnabled);}
      
      void setVectorLightsAttenuation(F32 factor)  {mVectorLightsAttenuation = factor;}
      F32 getVectorLightsAttenuation()             {return(mVectorLightsAttenuation);}

      void setAmbientColor(const ColorF & col)     {mAmbientLightColor = col;}
      const ColorF & getAmbientColor()             {return(mAmbientLightColor);}
      
   private:

      void resetGL();
      U32 installGLLights();
      void installGLLight(LightInfo *);

      void fillLightList(LightInfoList&, U32);
      void scoreLight(LightInfo *, const SphereF &) const;
      void scoreLight(LightInfo *, const PlaneF *, const U32, const Point3F &) const;
      static S32 QSORT_CALLBACK compareLights(const void *, const void *);

      LightInfoList           mLights;
      U32                     mMaxGLLights;
      U32                     mGLLightCount;
      bool                    mGLLightsInstalled;
      bool                    mGLInitialized;
      bool                    mVectorLightsEnabled;
      F32                     mVectorLightsAttenuation;
      ColorF                  mAmbientLightColor;
};

#endif
