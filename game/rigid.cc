//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "game/rigid.h"
#include "console/console.h"


//----------------------------------------------------------------------------

Rigid::Rigid()
{
   state.force = state.torque =
   state.linVelocity = 
   state.linPosition = 
   state.linMomentum =
   state.angVelocity = 
   state.angMomentum = Point3F(0,0,0);
   state.angPosition.identity();
   state.invWorldInertia.identity();

   mass = oneOverMass = 1.0;
   invObjectInertia.identity();
   restitution = 0.3;
   friction = 0.5;
   angDamping = 0;
   linDamping = 0;
   atRest = false;
   microCount = 0;
}

void Rigid::clearForces()
{
   state.force.set(0,0,0);
   state.torque.set(0,0,0);
}

void Rigid::integrate(State& t,F32 delta)
{
   lastDelta = delta;

   t.linPosition += t.linVelocity * delta;
   t.linMomentum += t.force * delta;
   t.linVelocity = t.linMomentum * oneOverMass;
   
//    // Linear momentum and velocity
//    t.linPosition = state.linPosition + (state.linVelocity * delta);
//    t.linMomentum = state.linMomentum + (state.force * delta);
//    t.linVelocity = t.linMomentum * oneOverMass;

   // Angular position is average velocity * delta
   F32 sinHalfAngle;
   F32 angle = state.angVelocity.len();
   if (angle != 0.0f) {
      QuatF dq;
      mSinCos(angle * delta * -0.5,
              sinHalfAngle,
              dq.w);
      sinHalfAngle *= 1 / angle;
      dq.x = t.angVelocity.x * sinHalfAngle;
      dq.y = t.angVelocity.y * sinHalfAngle;
      dq.z = t.angVelocity.z * sinHalfAngle;
      QuatF temp = t.angPosition;
      t.angPosition.mul(temp, dq);
      t.angPosition.normalize();
   }
   else
   {
      //t.angPosition = state.angPosition;
   }

   t.angMomentum += t.torque * delta;

   // Move angular momentum into world space
   MatrixF iv,qmat;
   t.angPosition.setMatrix(&qmat);
   iv.mul(qmat,invObjectInertia);
   qmat.transpose();
   t.invWorldInertia.mul(iv,qmat);
   t.invWorldInertia.mulV(t.angMomentum, &t.angVelocity);
}

void Rigid::updateVelocity(State& t)
{
   t.linVelocity.x = t.linMomentum.x * oneOverMass;
   t.linVelocity.y = t.linMomentum.y * oneOverMass;
   t.linVelocity.z = t.linMomentum.z * oneOverMass;
   t.invWorldInertia.mulV(t.angMomentum,&t.angVelocity);
}

void Rigid::applyImpulse(State& t, const Point3F &r, const Point3F &impulse)
{
   atRest = false;

   // Linear momentum and velocity
   t.linMomentum  += impulse;
   t.linVelocity.x = t.linMomentum.x * oneOverMass;
   t.linVelocity.y = t.linMomentum.y * oneOverMass;
   t.linVelocity.z = t.linMomentum.z * oneOverMass;

   // Rotational momentum and velocity
   Point3F tv;
   mCross(r,impulse,&tv);
   t.angMomentum += tv;
   t.invWorldInertia.mulV(t.angMomentum, &t.angVelocity);
}

// bool Rigid::resolveCollision(State& s, const Point3F& p, Point3F normal, Rigid* rigid)
// {
//    atRest = false;
//    Point3F v1,v2,r1,r2;
//    r1 = p - s.linPosition;
//    s.getVelocity(r1,&v1);
//    r2 = p - rigid->state.linPosition;
//    rigid->getVelocity(r2,&v2);

//    // Make sure they are converging
//    F32 nv = mDot(v1,normal);
//    nv -= mDot(v2,normal);
//    if (nv > 0)
//       return false;

//    // Compute impulse
//    F32 d, n = -nv * (1 + restitution) * rigid->restitution;
//    Point3F a1,b1,c1;
//    mCross(r1,normal,&a1);
//    s.invWorldInertia.mulV(a1,&b1);
//    mCross(b1,r1,&c1);

//    Point3F a2,b2,c2;
//    mCross(r2,normal,&a2);
//    rigid->state.invWorldInertia.mulV(a2,&b2);
//    mCross(b2,r2,&c2);

//    Point3F c3 = c1 + c2;
//    d = oneOverMass + rigid->oneOverMass + mDot(c3,normal);
//    Point3F impulse = normal * (n / d);

//    applyImpulse(s, r1,impulse);
//    impulse.neg();
//    rigid->applyImpulse(r2,impulse);
//    return true;
// }

bool Rigid::resolveCollision(State& s, const Point3F& p, Point3F normal)
{
   atRest = false;
   Point3F v1,r1 = p - s.linPosition;
   s.getVelocity(r1,&v1);
   F32 n = -mDot(v1,normal);
   if (n >= 0) {

      // Collision impulse
      F32 d = getZeroImpulse(s,r1,normal);
      F32 j = n * (1 + restitution) * d;
      Point3F impulse = normal * j;

      // Friction impulse
      Point3F uv = v1 + (normal * n);
      F32 ul = uv.len();
      if (ul) {
         uv /= -ul;
         F32 u = n * d * friction * getZeroImpulse(s,r1,uv);
         impulse += uv * u;
      }

      //
      applyImpulse(s, r1,impulse);
   }
   return true;
}   


F32 Rigid::getZeroImpulse(State& s,const Point3F& r,const Point3F& normal)
{
   Point3F a,b,c;
   mCross(r,normal,&a);
   s.invWorldInertia.mulV(a,&b);
   mCross(b,r,&c);
   return 1 / (oneOverMass + mDot(c,normal));
}

void Rigid::getVelocity(const Point3F &r, Point3F *v)
{
   state.getVelocity(r, v);
}

F32 Rigid::getKineticEnergy(const MatrixF& wto)
{
   Point3F w;
   wto.mulV(state.angVelocity,&w);
   const F32* f = invObjectInertia;
   return 0.5 * ((mass * mDot(state.linVelocity,state.linVelocity)) +
      w.x * w.x / f[0] +
      w.y * w.y / f[5] +
      w.z * w.z / f[10]);
}

void Rigid::State::getVelocity(const Point3F& r, Point3F* v)
{
   mCross(angVelocity, r, v);
   *v += linVelocity;
}

void Rigid::State::getTransform(MatrixF* mat)
{
   angPosition.setMatrix(mat);
   mat->setColumn(3,linPosition);
}

void Rigid::State::setTransform(const MatrixF& mat)
{
   angPosition.set(mat);
   mat.getColumn(3,&linPosition);
}   


void Rigid::setObjectInertia(const Point3F& r)
{
   // Rotational moment of inertia of a box
   F32 ot = mass/12;
   F32 a = r.x * r.x;
   F32 b = r.y * r.y;
   F32 c = r.z * r.z;

   F32* f = invObjectInertia;
   invObjectInertia.identity();
//   f[0]  = 1 / (ot * (b + c));
//   f[5]  = 1 / (ot * (c + a));
//   f[10] = 1 / (ot * (a + b));

   MatrixF iv,qmat;
   state.angPosition.setMatrix(&qmat);
   iv.mul(qmat,invObjectInertia);
   qmat.transpose();
   state.invWorldInertia.mul(iv,qmat);
}


//----------------------------------------------------------------------------

bool Rigid::checkRestCondition()
{
   if (state.linMomentum.len() < 0.1f &&
         state.angMomentum.len() < 0.05f) {
      setAtRest();
   }
   return atRest;
}   

void Rigid::setAtRest()
{
   atRest = true;
   state.linVelocity = 
   state.linMomentum =
   state.angVelocity = 
   state.angMomentum =
      Point3F(0,0,0);
}
