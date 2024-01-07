//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _RIGID_H_
#define _RIGID_H_

#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif
#ifndef _MPOINT_H_
#include "Math/mPoint.h"
#endif
#ifndef _MMATRIX_H_
#include "Math/mMatrix.h"
#endif
#ifndef _MQUAT_H_
#include "Math/mQuat.h"
#endif

//----------------------------------------------------------------------------

class Rigid
{
public:
   struct Impulse {
      Point3F force;
      Point3F torque;
   };
   struct State {
      Point3F force;
      Point3F torque;

      Point3F linVelocity;
      Point3F linPosition;
      Point3F linMomentum;
      Point3F angVelocity;
      QuatF   angPosition;
      Point3F angMomentum;

      MatrixF invWorldInertia;
      
      void getVelocity(const Point3F &r,Point3F* v);
      void getTransform(MatrixF* mat);
      void setTransform(const MatrixF& mat);
   };
   struct Coeficients {
      F32 e;
      F32 u;
   };

   State state;
   MatrixF invObjectInertia;
   F32 oneOverMass;
   F32 mass;
   F32 restitution;
   F32 friction;
   F32 angDamping;
   F32 linDamping;

   int microCount;
   F32 lastDelta;
   bool atRest;


   Rigid();
   void clearForces();

   void updateVelocity(State& t);
   void integrate(State& t,F32 delta);
   void applyImpulse(State& t, const Point3F &v,const Point3F &impulse);
//   bool resolveCollision(State&,const Point3F& p,Point3F normal,Rigid*);
   bool resolveCollision(State&,const Point3F& p,Point3F normal);
   void getVelocity(const Point3F &r, Point3F* v);
   F32  getZeroImpulse(State& t,const Point3F& r,const Point3F& normal);
   void setObjectInertia(const Point3F& r);
   F32  getKineticEnergy(const MatrixF& wto);

   bool checkRestCondition();
   void setAtRest();
};


#endif
