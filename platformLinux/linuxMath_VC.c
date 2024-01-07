
/*
 Sigh, we're going to get around the stupid gcc math bugs by compiling
 this code with Visual C++ and then using the object file in our code.

 HACK HACK HACK
*/
 
#include <math.h>

#ifndef M_PI
#define M_PI          3.14159265358979323846
#endif

#define TRIBES2_NEW_MATH

#if defined(_MSC_VER)
#define __inline__ __inline
#else
#define __inline__ inline
#endif

typedef unsigned char U8;
typedef signed int S32;
typedef unsigned int U32;
typedef float F32;
typedef double F64;

/* Utility routines */

static __inline__ void* dMemcpy(void *dst, const void *src, unsigned size)
{
   return memcpy(dst,src,size);
}   

static __inline__ F32 getMin(F32 a, F32 b)
{
   return a>b ? b : a;
}

static __inline__ F32 getMax(F32 a, F32 b)
{
   return a>b ? a : b;
}

static __inline__ F32 mClampF(F32 val, F32 low, F32 high)
{
   return getMax(getMin(val, high), low);
}

static __inline__ F32 mFabs(const F32 val)
{
   return fabs(val);
}

static __inline__ F32 mSin(const F32 angle)
{
   return sin(angle);
}   

static __inline__ F32 mCos(const F32 angle)
{
   return cos(angle);   
}   

#define mSinCos(angle, s, c) \
{ \
   s = mSin(angle); \
   c = mCos(angle); \
}

static __inline__ F32 mAcos(const F32 val)
{
#if defined(_MSC_VER)
   extern double ACOS(double X);
   return ACOS(val);
#else
   return acos(val);
#endif
}   

static __inline__ F32 mSqrt(const F32 val)
{
   return sqrt(val);
}   

static __inline__ F64 mSqrtD(const F64 val)
{
   return sqrt(val);
}   

static __inline__ F32 mPow(const F32 x, const F32 y)
{
#if defined(_MSC_VER)
   extern double POW(double X, double Y);
   return POW(x, y);
#else
   return pow(x, y);
#endif
}

#define swap(x, y) { F32 t = y; y = x; x = t; }

static __inline__ F32 square( F32 d )
{
   return ( d * d );
}

static __inline__ F32 cube( F32 d )
{
   return ( d * d * d );
}


/* The actual math routines */

void m_matF_x_point3F_ASM(const F32 *m, const F32 *p, F32 *presult)
{
   presult[0] = m[0]*p[0] + m[1]*p[1] + m[2]*p[2]  + m[3];   
   presult[1] = m[4]*p[0] + m[5]*p[1] + m[6]*p[2]  + m[7];   
   presult[2] = m[8]*p[0] + m[9]*p[1] + m[10]*p[2] + m[11];   
}

void m_matF_x_vectorF_ASM(const F32 *m, const F32 *v, F32 *vresult)
{
   vresult[0] = m[0]*v[0] + m[1]*v[1] + m[2]*v[2];   
   vresult[1] = m[4]*v[0] + m[5]*v[1] + m[6]*v[2];   
   vresult[2] = m[8]*v[0] + m[9]*v[1] + m[10]*v[2];   
}

void m_quatF_set_matF_ASM( F32 x, F32 y, F32 z, F32 w, F32* m )
{
#define qidx(r,c) (r*4 + c)
      F32 xs = x * 2.0f;
      F32 ys = y * 2.0f;
      F32 zs = z * 2.0f;
      F32 wx = w * xs;
      F32 wy = w * ys;
      F32 wz = w * zs;
      F32 xx = x * xs;
      F32 xy = x * ys;
      F32 xz = x * zs;
      F32 yy = y * ys;
      F32 yz = y * zs;
      F32 zz = z * zs;
      m[qidx(0,0)] = 1.0f - (yy + zz);
      m[qidx(1,0)] = xy - wz;
      m[qidx(2,0)] = xz + wy;
      m[qidx(3,0)] = 0.0f;
      m[qidx(0,1)] = xy + wz;
      m[qidx(1,1)] = 1.0f - (xx + zz);
      m[qidx(2,1)] = yz - wx;
      m[qidx(3,1)] = 0.0f;
      m[qidx(0,2)] = xz - wy;
      m[qidx(1,2)] = yz + wx;
      m[qidx(2,2)] = 1.0f - (xx + yy);
      m[qidx(3,2)] = 0.0f;

      m[qidx(0,3)] = 0.0f;
      m[qidx(1,3)] = 0.0f;
      m[qidx(2,3)] = 0.0f;
      m[qidx(3,3)] = 1.0f;
#undef qidx
}

F32 m_matF_determinant_ASM(const F32 *m)
{
   return m[0] * (m[5] * m[10] - m[6] * m[9])  +
      m[4] * (m[2] * m[9]  - m[1] * m[10]) +  
      m[8] * (m[1] * m[6]  - m[2] * m[5])  ;  
}   

void m_matF_inverse_ASM(F32 *m)
{
   // using Cramers Rule find the Inverse
   // Minv = (1/det(M)) * adjoint(M)
   F32 det = m_matF_determinant_ASM( m );

   F32 invDet = 1.0f/det;
   F32 temp[16];

   temp[0] = (m[5] * m[10]- m[6] * m[9]) * invDet;
   temp[1] = (m[9] * m[2] - m[10]* m[1]) * invDet;
   temp[2] = (m[1] * m[6] - m[2] * m[5]) * invDet;

   temp[4] = (m[6] * m[8] - m[4] * m[10])* invDet;
   temp[5] = (m[10]* m[0] - m[8] * m[2]) * invDet;
   temp[6] = (m[2] * m[4] - m[0] * m[6]) * invDet;

   temp[8] = (m[4] * m[9] - m[5] * m[8]) * invDet;
   temp[9] = (m[8] * m[1] - m[9] * m[0]) * invDet;
   temp[10]= (m[0] * m[5] - m[1] * m[4]) * invDet;

   m[0] = temp[0];
   m[1] = temp[1];
   m[2] = temp[2];

   m[4] = temp[4];
   m[5] = temp[5];
   m[6] = temp[6];

   m[8] = temp[8];
   m[9] = temp[9];
   m[10] = temp[10];

   // invert the translation
   temp[0] = -m[3];
   temp[1] = -m[7];
   temp[2] = -m[11];
   m_matF_x_vectorF_ASM(m, temp, &temp[4]);
   m[3] = temp[4];
   m[7] = temp[5];
   m[11]= temp[6];
}

void m_matF_affineInverse_ASM(F32 *m)
{
   // Matrix class checks to make sure this is an affine transform before calling
   //  this function, so we can proceed assuming it is...
   F32 temp[16];
   dMemcpy(temp, m, 16 * sizeof(F32));

   // Transpose rotation
   m[1] = temp[4];
   m[4] = temp[1];
   m[2] = temp[8];
   m[8] = temp[2];
   m[6] = temp[9];
   m[9] = temp[6];

   m[3]  = -(temp[0]*temp[3] + temp[4]*temp[7] + temp[8]*temp[11]);
   m[7]  = -(temp[1]*temp[3] + temp[5]*temp[7] + temp[9]*temp[11]);
   m[11] = -(temp[2]*temp[3] + temp[6]*temp[7] + temp[10]*temp[11]);
}

void m_matF_x_matF_ASM(const F32 *a, const F32 *b, F32 *mresult)
{
   mresult[0] = a[0]*b[0] + a[1]*b[4] + a[2]*b[8]  + a[3]*b[12];
   mresult[1] = a[0]*b[1] + a[1]*b[5] + a[2]*b[9]  + a[3]*b[13];
   mresult[2] = a[0]*b[2] + a[1]*b[6] + a[2]*b[10] + a[3]*b[14];
   mresult[3] = a[0]*b[3] + a[1]*b[7] + a[2]*b[11] + a[3]*b[15];

   mresult[4] = a[4]*b[0] + a[5]*b[4] + a[6]*b[8]  + a[7]*b[12];
   mresult[5] = a[4]*b[1] + a[5]*b[5] + a[6]*b[9]  + a[7]*b[13];
   mresult[6] = a[4]*b[2] + a[5]*b[6] + a[6]*b[10] + a[7]*b[14];
   mresult[7] = a[4]*b[3] + a[5]*b[7] + a[6]*b[11] + a[7]*b[15];

   mresult[8] = a[8]*b[0] + a[9]*b[4] + a[10]*b[8] + a[11]*b[12];
   mresult[9] = a[8]*b[1] + a[9]*b[5] + a[10]*b[9] + a[11]*b[13];
   mresult[10]= a[8]*b[2] + a[9]*b[6] + a[10]*b[10]+ a[11]*b[14];
   mresult[11]= a[8]*b[3] + a[9]*b[7] + a[10]*b[11]+ a[11]*b[15];

   mresult[12]= a[12]*b[0]+ a[13]*b[4]+ a[14]*b[8] + a[15]*b[12];
   mresult[13]= a[12]*b[1]+ a[13]*b[5]+ a[14]*b[9] + a[15]*b[13];
   mresult[14]= a[12]*b[2]+ a[13]*b[6]+ a[14]*b[10]+ a[15]*b[14];
   mresult[15]= a[12]*b[3]+ a[13]*b[7]+ a[14]*b[11]+ a[15]*b[15];
}

#ifdef TRIBES2_NEW_MATH

// Code taken from Math/mSolver.cc

//--------------------------------------------------------------------------
#define EQN_EPSILON     (1e-8)

static __inline__ int mIsZero(F32 val)
{
   return((val > -EQN_EPSILON) && (val < EQN_EPSILON));
}

static __inline__ F32 mCbrt(F32 val)
{
   if(val < 0.f)
      return(-mPow(-val, (F32)(1.f/3.f)));
   else
      return(mPow(val, (F32)(1.f/3.f)));
}

static __inline__ U32 mSolveLinear(F32 a, F32 b, F32 * x)
{
   if(mIsZero(a))
      return(0);

   x[0] = -b/a;
   return(1);
}

U32 mSolveQuadratic_ASM(F32 a, F32 b, F32 c, F32 * x)
{
   F32 desc;

   // really linear?
   if(mIsZero(a))
      return(mSolveLinear(b, c, x));
  
   // get the descriminant:   (b^2 - 4ac)
   desc = (b * b) - (4.f * a * c);
   
   // solutions:
   // desc < 0:   two imaginary solutions 
   // desc > 0:   two real solutions (b +- sqrt(desc)) / 2a
   // desc = 0:   one real solution (b / 2a)
   if(mIsZero(desc))
   {
      x[0] = b / (2.f * a);
      return(1);
   }
   else if(desc > 0.f)
   {
      F32 sqrdesc = mSqrt(desc);
      F32 den = (2.f * a);
      x[0] = (b + sqrdesc) / den;
      x[1] = (b - sqrdesc) / den;

      if(x[1] < x[0])
         swap(x[0], x[1]);

      return(2);
   }
   else 
      return(0);
}

//--------------------------------------------------------------------------
// from Graphics Gems I: pp 738-742
U32 mSolveCubic_ASM(F32 a, F32 b, F32 c, F32 d, F32 * x)
{
   F32 A, B, C, D;
   F32 A2, A3;
   F32 p, q;
   F32 p3, q2;
   U32 num;
   F32 sub;
   U32 i;
   S32 j, k;

   if(mIsZero(a))
      return(mSolveQuadratic_ASM(b, c, d, x));

   // normal form: x^3 + Ax^2 + BX + C = 0
   A = b / a;
   B = c / a;
   C = d / a;
   
   // substitute x = y - A/3 to eliminate quadric term and depress
   // the cubic equation to (x^3 + px + q = 0)
   A2 = A * A;
   A3 = A2 * A;

   p = (1.f/3.f) * (((-1.f/3.f) * A2) + B);
   q = (1.f/2.f) * (((2.f/27.f) * A3) - ((1.f/3.f) * A * B) + C);

   // use Cardano's fomula to solve the depressed cubic
   p3 = p * p * p;
   q2 = q * q;

   D = q2 + p3;

   num = 0;
   
   if(mIsZero(D))          // 1 or 2 solutions
   {
      if(mIsZero(q)) // 1 triple solution
      {
         x[0] = 0.f;
         num = 1;
      }
      else // 1 single and 1 double
      {
         F32 u = mCbrt(-q);
         x[0] = 2.f * u;
         x[1] = -u;
         num = 2;
      }
   }
   else if(D < 0.f)        // 3 solutions: casus irreducibilis
   {
      F32 phi = (1.f/3.f) * mAcos(-q / mSqrt(-p3));
      F32 t = 2.f * mSqrt(-p);
      
      x[0] = t * mCos(phi);
      x[1] = -t * mCos(phi + (M_PI / 3.f));
      x[2] = -t * mCos(phi - (M_PI / 3.f));
      num = 3;
   }
   else                    // 1 solution
   {
      F32 sqrtD = mSqrt(D);
      F32 u = mCbrt(sqrtD - q);
      F32 v = -mCbrt(sqrtD + q);

      x[0] = u + v;
      num = 1;
   }
   
   // resubstitute
   sub = (1.f/3.f) * A;
   for(i = 0; i < num; i++)
      x[i] -= sub;

   // sort the roots
   for(j = 0; j < (num - 1); j++)
      for(k = j + 1; k < num; k++)
         if(x[k] < x[j])
            swap(x[k], x[j]);

   return(num);
}

//--------------------------------------------------------------------------
// from Graphics Gems I: pp 738-742
U32 mSolveQuartic_ASM(F32 a, F32 b, F32 c, F32 d, F32 e, F32 * x)
{
   F32 A, B, C, D;
   F32 A2, A3, A4;
   F32 p, q, r;
   U32 num;
   F32 sub;
   U32 i;
   S32 j, k;

   if(mIsZero(a))
      return(mSolveCubic_ASM(b, c, d, e, x));

   // normal form: x^4 + ax^3 + bx^2 + cx + d = 0
   A = b / a;
   B = c / a;
   C = d / a;
   D = e / a;

   // substitue x = y - A/4 to eliminate cubic term:
   // x^4 + px^2 + qx + r = 0
   A2 = A * A;
   A3 = A2 * A;
   A4 = A2 * A2;

   p = ((-3.f/8.f) * A2) + B;
   q = ((1.f/8.f) * A3) - ((1.f/2.f) * A * B) + C;
   r = ((-3.f/256.f) * A4) + ((1.f/16.f) * A2 * B) - ((1.f/4.f) * A * C) + D;

   num = 0;
   if(mIsZero(r)) // no absolute term: y(y^3 + py + q) = 0
   {
      num = mSolveCubic_ASM(1.f, 0.f, p, q, x);
      x[num++] = 0.f;
   }
   else 
   {
      F32 q2;
      F32 z;
      F32 u, v;

      // solve the resolvent cubic
      q2 = q * q;

      a = 1.f;
      b = (-1.f/2.f) * p;
      c = -r;
      d = ((1.f/2.f) * r * p) - ((1.f/8.f) * q2);
      
      mSolveCubic_ASM(a, b, c, d, x);
      
      z = x[0];
      
      // build 2 quadratic equations from the one solution
      u = (z * z) - r;
      v = (2.f * z) - p;

      if(mIsZero(u))
         u = 0.f;
      else if(u > 0.f)
         u = mSqrt(u);
      else
         return(0);
         
      if(mIsZero(v))
         v = 0.f;
      else if(v > 0.f)
         v = mSqrt(v);
      else
         return(0);
      
      // solve the two quadratics
      a = 1.f;
      b = v;
      c = z - u;
      num = mSolveQuadratic_ASM(a, b, c, x);
      
      a = 1.f;
      b = -v;
      c = z + u;
      num += mSolveQuadratic_ASM(a, b, c, x + num);
   }
   
   // resubstitute
   sub = (1.f/4.f) * A;
   for(i = 0; i < num; i++)
      x[i] -= sub;

   // sort the roots
   for(j = 0; j < (num - 1); j++)
      for(k = j + 1; k < num; k++)
         if(x[k] < x[j])
            swap(x[k], x[j]);

   return(num);
}

#else

/* Old cubic and quadratic math functions */

U32 mSolveCubic_ASM(F32 A, F32 B, F32 C, F32 D, F32* x)
{
   int i;
   U32 nreal;
   F32 w, p, q, qp, dis, h, phi, phip;
   F32 AP, BP, CP, DP, XP;

   if( A != 0.0f ) {
      // is it a cubic?
      w = B / ( 3.0f * A );
      CP = C / ( 3.0f * A );
      p = cube( CP - square( w ) );
      CP = ( C * w ) - D;
      DP = CP / A;
      CP = 2.0f * cube( w );
      q = -0.5f *( CP - DP );
      dis = square( q ) + p;	// discriminant

      if( dis < 0.0 ) {
	 // 3 real solutions
         h = q / mSqrt( -p );
	 h = mClampF( h, -1.0f, 1.0f );
         phi = mAcos( h );
         p = 2.0f * mPow( -p, ( 1.0f / 6.0f ) );

         for( i = 0; i < 3; i++ ) {
	    //x[i] = p*cos((phi+2*i*M_PI)/3.0) - w;
	    phip = ( 2.0f * i ) * M_PI;
	    phip = ( phip + phi ) / 3.0f;
	    x[i] = ( p * mCos( phip ) ) - w;
	 }

	 // sort results
         if( x[1] < x[0] ) {
	    swap( x[0], x[1] );
	 }

         if( x[2] < x[1] ) {
	    swap( x[1], x[2] );
	 }

         if( x[1] < x[0] ) {
	    swap( x[0], x[1] );
	 }

         nreal = 3;
      } else {
	 // only one real solution
         dis = mSqrt( dis );
         //h = pow(fabs(q+dis), 1.f/3.f);
	 //p = pow(fabs(q-dis), 1.f/3.f);
	 qp = mFabs( q + dis );
         h = mPow( qp, ( 1.0f / 3.0f ) );
	 qp = mFabs( q - dis );
         p = mPow( qp, ( 1.0f / 3.0f ) );

         //x[0] = ((q+dis>0.0)? h : -h) + ((q-dis>0.0)? p : -p) -   w;
	 if( ( q + dis ) <= 0.0f ) {
	    h = -h;
	 }

	 if( ( q - dis ) <= 0.0f ) {
	    p = -p;
	 }

	 x[0] = ( h + p ) - w;
         nreal = 1;
      }

      // Perform one step of a Newton iteration in order to minimize
      // round-off errors
      for (i = 0; i < nreal; i++)
      {
	 //if ((h = C + x[i] * (2 * B + 3 * x[i] * A)) != 0.0) {
	 AP = x[i] * 3.0f * A;
	 BP = 2.0f * B;
	 XP = x[i] * ( BP + AP );
	 CP = C + XP;

	 h = CP;

	 if ( h != 0.0f ) {
            //x[i] -= (D + x[i] * (C + x[i] * (B + x[i] * A)))/h;
	    AP = x[i] * A;
	    BP = B + AP;
	    XP = x[i] * BP;
	    CP = C + XP;
	    XP = x[i] * CP;
	    DP = D + XP;
	    DP = DP / h;
	    x[i] = x[i] - DP;
	 }

      }

   } else if( B != 0.0f ) {
      // is it a quadratic?
      p = 0.5f * ( C / B );
      dis = square( p ) - ( D / B );

      if( dis >= 0.0 ) {
	 // are there solutions?
         dis = mSqrt( dis );
         x[0] = -p - dis;
         x[1] = -p + dis;
         nreal = 2;
      } else {
	 // no real solution
         nreal = 0;
      }

   } else if( C != 0.0f ) {
      // is it a linear?
      x[0] = -D / C;
      nreal = 1;
   } else {
      // not an equation
      nreal = 0;
   }

   return nreal;
}

U32 mSolveQuartic_ASM( F32 A, F32 B, F32 C, F32 D, F32 E, F32* x )
{
   int i;
   U32 nreal;
   F32 w, b0, b1, b2, d0, d1, d2, h, hw, t, z;
   F32* px;
   F32 w2;
   F32 CA, DA, EA;
   F32 CP, DP;
   F32 zp;
   F32 tp;

   // check if it's a cubic.
   if( A == 0.0f ) {
      return mSolveCubic_ASM( B, C, D, E, x );
   }

   //w = B/(4*A);                       // offset
   w = B / ( 4.0f * A );
   w2 = square( w );

   //b2 = -6*square(w) + C/A;           // coeffs. of shifted polynomial
   CA = C / A;
   b2 = ( -6.0f * w2 ) + CA;

   //b1 = (8*square(w) - 2*C/A)*w + D/A;
   DA = D / A;
   CP = 2.0f * CA;
   CP = ( 8.0f * w2 ) - CP;
   b1 = ( CP * w ) + DA;

   //b0 = ((-3*square(w) + C/A)*w - D/A)*w + E/A;
   EA = E / A;
   CP = ( -3.0f * w2 ) + CA;
   DP = ( CP * w ) - DA;
   b0 = ( DP * w ) + EA;

   // cubic resolvent
   {
      F32 cubeA = 1.0;
      F32 cubeB = b2;
      F32 cubeC = -4 * b0;
      F32 bp = 4 * ( b0 * b2 );
      F32 cubeD = square( b1 ) - bp;
   
      // only lowermost root needed
      mSolveCubic_ASM( cubeA, cubeB, cubeC, cubeD, x );
   }

   z = x[0]; 

   nreal = 0;
   px = x;
   zp = 0.25f * square( z );
   t = mSqrt( zp - b0 );

   for( i = -1; i <= 1; i += 2 ) {
      //d0 = -0.5*z + i*t;
      d0 = ( -0.5 * z ) + ( i * t );	// coeffs. of quadratic factor

      //d1 = (t!=0.0)? -i*0.5*b1/t : i*mSqrt(-z - b2);
      if( t != 0.0f ) {
	 tp = -i * 0.5f;
	 tp = tp * b1;
	 tp = tp / t;
      } else {
	 tp = mSqrt( -z - b2 );
	 tp = i * tp;
      }

      d1 = tp;
      h = ( 0.25f * square( d1 ) ) - d0;

      if( h >= 0.0f ) {
         h = mSqrt( h );
         nreal += 2;

	 //*px++ = -0.5*d1 - h - w;
	 //*px++ = -0.5*d1 + h - w;
	 d2 = -0.5f * d1;
	 hw = h - w;
         *px = d2 - hw;
	 px++;
         *px = d2 + hw;
	 px++;
      }

   }

   // sort results in ascending order
   if( nreal == 4 ) {

      if( x[2] < x[0] ) {
         swap( x[0], x[2] );
      }

      if( x[3] < x[1] ) {
         swap( x[1], x[3] );
      }

      if( x[1] < x[0] ) {
         swap( x[0], x[1] );
      }

      if( x[3] < x[2] ) {
         swap( x[2], x[3] );
      }

      if( x[2] < x[1] ) {
         swap( x[1], x[2] );
      }

   }

   return nreal;
}

#endif /* TRIBES2_NEW_MATH */

/* Added lots more math functions optimized by Visual C++ */

//--------------------------------------
void m_point2F_normalize_ASM(F32 *p)
{
   F32 factor = 1.0f / mSqrt(p[0]*p[0] + p[1]*p[1] );
   p[0] *= factor;   
   p[1] *= factor;   
}   

//--------------------------------------
void m_point2F_normalize_f_ASM(F32 *p, F32 val)
{
   F32 factor = val / mSqrt(p[0]*p[0] + p[1]*p[1] );
   p[0] *= factor;   
   p[1] *= factor;   
}   

//--------------------------------------
void m_point2D_normalize_ASM(F64 *p)
{
   F64 factor = 1.0f / mSqrtD(p[0]*p[0] + p[1]*p[1] );
   p[0] *= factor;   
   p[1] *= factor;   
}   

//--------------------------------------
void m_point2D_normalize_f_ASM(F64 *p, F64 val)
{
   F64 factor = val / mSqrtD(p[0]*p[0] + p[1]*p[1] );
   p[0] *= factor;   
   p[1] *= factor;   
}   

//--------------------------------------
void m_point3F_normalize_ASM(F32 *p)
{
   F32 squared = p[0]*p[0] + p[1]*p[1] + p[2]*p[2];
   if (squared != 0.0) {
      F32 factor = 1.0f / mSqrt(squared);
      p[0] *= factor;   
      p[1] *= factor;   
      p[2] *= factor;   
   } else {
      p[0] = 0;
      p[1] = 0;
      p[2] = 1;
   }
}   

//--------------------------------------
void m_point3F_normalize_f_ASM(F32 *p, F32 val)
{
   F32 factor = val / mSqrt(p[0]*p[0] + p[1]*p[1] + p[2]*p[2] );
   p[0] *= factor;   
   p[1] *= factor;   
   p[2] *= factor;   
}   

//--------------------------------------
void m_point3F_interpolate_ASM(const F32 *from, const F32 *to, F32 factor, F32 *result )
{
   F32 inverse = 1.0f - factor;
   result[0] = from[0] * inverse + to[0] * factor;
   result[1] = from[1] * inverse + to[1] * factor;
   result[2] = from[2] * inverse + to[2] * factor;
}   

//--------------------------------------
void m_point3D_normalize_ASM(F64 *p)
{
   F64 factor = 1.0f / mSqrtD(p[0]*p[0] + p[1]*p[1] + p[2]*p[2] );
   p[0] *= factor;   
   p[1] *= factor;   
   p[2] *= factor;   
}   


//--------------------------------------
void m_point3D_interpolate_ASM(const F64 *from, const F64 *to, F64 factor, F64 *result )
{
   F64 inverse = 1.0f - factor;
   result[0] = from[0] * inverse + to[0] * factor;
   result[1] = from[1] * inverse + to[1] * factor;
   result[2] = from[2] * inverse + to[2] * factor;
}   

void m_point3F_bulk_dot_ASM(const F32* refVector,
                            const F32* dotPoints,
                            const U32  numPoints,
                            const U32  pointStride,
                            F32*       output)
{
   U32 i;
   for (i = 0; i < numPoints; i++)
   {
      F32* pPoint = (F32*)(((U8*)dotPoints) + (pointStride * i));
      output[i] = ((refVector[0] * pPoint[0]) +
                   (refVector[1] * pPoint[1]) +
                   (refVector[2] * pPoint[2]));
   }
}

void m_point3F_bulk_dot_indexed_ASM(const F32* refVector,
                                    const F32* dotPoints,
                                    const U32  numPoints,
                                    const U32  pointStride,
                                    const U32* pointIndices,
                                    F32*       output)
{
   U32 i;
   for (i = 0; i < numPoints; i++)
   {
      F32* pPoint = (F32*)(((U8*)dotPoints) + (pointStride * pointIndices[i]));
      output[i] = ((refVector[0] * pPoint[0]) +
                   (refVector[1] * pPoint[1]) +
                   (refVector[2] * pPoint[2]));
   }
}

//--------------------------------------
void m_matF_identity_ASM(F32 *m)
{
   *m++ = 1.0f;
   *m++ = 0.0f;
   *m++ = 0.0f;
   *m++ = 0.0f;

   *m++ = 0.0f;
   *m++ = 1.0f;
   *m++ = 0.0f;
   *m++ = 0.0f;

   *m++ = 0.0f;
   *m++ = 0.0f;
   *m++ = 1.0f;
   *m++ = 0.0f;

   *m++ = 0.0f;
   *m++ = 0.0f;
   *m++ = 0.0f;
   *m   = 1.0f;
}

//--------------------------------------
void m_matF_set_euler_ASM(const F32 *e, F32 *result)
{
   enum {
      AXIS_X   = (1<<0),
      AXIS_Y   = (1<<1),
      AXIS_Z   = (1<<2)
   };

   U32 axis = 0;
   if (e[0] != 0.0f) axis |= AXIS_X;
   if (e[1] != 0.0f) axis |= AXIS_Y;
   if (e[2] != 0.0f) axis |= AXIS_Z;

   switch (axis)
   {
      case 0:
         m_matF_identity_ASM(result);
         break;

      case AXIS_X:
      {
         F32 cx,sx;
         mSinCos( e[0], sx, cx );

         result[0] = 1.0f;
         result[1] = 0.0f;
         result[2] = 0.0f;
         result[3] = 0.0f;

         result[4] = 0.0f;
         result[5] = cx;
         result[6] = sx;
         result[7] = 0.0f;

         result[8] = 0.0f;
         result[9] = -sx;
         result[10]= cx;
         result[11]= 0.0f;

         result[12]= 0.0f;
         result[13]= 0.0f;
         result[14]= 0.0f;
         result[15]= 1.0f;
         break;
      }
      
      case AXIS_Y:
      {
         F32 cy,sy;
         mSinCos( e[1], sy, cy );

         result[0] = cy;
         result[1] = 0.0f;
         result[2] = -sy;
         result[3] = 0.0f;

         result[4] = 0.0f;
         result[5] = 1.0f;
         result[6] = 0.0f;
         result[7] = 0.0f;

         result[8] = sy;
         result[9] = 0.0f;
         result[10]= cy;
         result[11]= 0.0f;

         result[12]= 0.0f;
         result[13]= 0.0f;
         result[14]= 0.0f;
         result[15]= 1.0f;
         break;
      }
      
      case AXIS_Z:
      {
         // the matrix looks like this:
         //  r1 - (r4 * sin(x))     r2 + (r3 * sin(x))   -cos(x) * sin(y)
         //  -cos(x) * sin(z)       cos(x) * cos(z)      sin(x)
         //  r3 + (r2 * sin(x))     r4 - (r1 * sin(x))   cos(x) * cos(y)
         //
         // where:
         //  r1 = cos(y) * cos(z)
         //  r2 = cos(y) * sin(z)
         //  r3 = sin(y) * cos(z)
         //  r4 = sin(y) * sin(z)
         F32 cz,sz;
         F32 r1, r2, r3, r4;
         mSinCos( e[2], sz, cz );
         r1 = cz;
         r2 = sz;
         r3 = 0.0f;
         r4 = 0.0f;

         result[0] = cz;
         result[1] = sz;
         result[2] = 0.0f;
         result[3] = 0.0f;

         result[4] = -sz;
         result[5] = cz;
         result[6] = 0.0f;
         result[7] = 0.0f;

         result[8] = 0.0f;
         result[9] = 0.0f;
         result[10]= 1.0f;
         result[11]= 0.0f;

         result[12]= 0.0f;
         result[13]= 0.0f;
         result[14]= 0.0f;
         result[15]= 1.0f;
         break;
      }

      default:
      {
         // the matrix looks like this:
         //  r1 - (r4 * sin(x))     r2 + (r3 * sin(x))   -cos(x) * sin(y)
         //  -cos(x) * sin(z)       cos(x) * cos(z)      sin(x)
         //  r3 + (r2 * sin(x))     r4 - (r1 * sin(x))   cos(x) * cos(y)
         //
         // where:
         //  r1 = cos(y) * cos(z)
         //  r2 = cos(y) * sin(z)
         //  r3 = sin(y) * cos(z)
         //  r4 = sin(y) * sin(z)
         F32 cx,sx;
         F32 cy,sy;
         F32 cz,sz;
         F32 r1, r2, r3, r4;
         mSinCos( e[0], sx, cx );
         mSinCos( e[1], sy, cy );
         mSinCos( e[2], sz, cz );
         r1 = cy * cz;
         r2 = cy * sz;
         r3 = sy * cz;
         r4 = sy * sz;

         result[0] = r1 - (r4 * sx);
         result[1] = r2 + (r3 * sx);
         result[2] = -cx * sy;
         result[3] = 0.0f;

         result[4] = -cx * sz;
         result[5] = cx * cz;
         result[6] = sx;
         result[7] = 0.0f;

         result[8] = r3 + (r2 * sx);
         result[9] = r4 - (r1 * sx);
         result[10]= cx * cy;
         result[11]= 0.0f;

         result[12]= 0.0f;
         result[13]= 0.0f;
         result[14]= 0.0f;
         result[15]= 1.0f;
         break;
      }
   }
}  


//--------------------------------------
void m_matF_set_euler_point_ASM(const F32 *e, const F32 *p, F32 *result)
{
   m_matF_set_euler_ASM(e, result);
   result[3] = p[0];
   result[7] = p[1];
   result[11]= p[2];
}   
 
//--------------------------------------
void m_matF_transpose_ASM(F32 *m)
{
   swap(m[1], m[4]);
   swap(m[2], m[8]);
   swap(m[3], m[12]);
   swap(m[6], m[9]);
   swap(m[7], m[13]);
   swap(m[11],m[14]);
}

//--------------------------------------
void m_matF_scale_ASM(F32 *m, const F32 *p)
{
   // Note, doesn't allow scaling w...

   m[0]  *= p[0];  m[1]  *= p[1];  m[2]  *= p[2];
   m[4]  *= p[0];  m[5]  *= p[1];  m[6]  *= p[2];
   m[8]  *= p[0];  m[9]  *= p[1];  m[10] *= p[2];
   m[12] *= p[0];  m[13] *= p[1];  m[14] *= p[2];
}   

static __inline__ void mCross(const F32* a, const F32* b, F32 *res)
{
   res[0] = (a[1] * b[2]) - (a[2] * b[1]);
   res[1] = (a[2] * b[0]) - (a[0] * b[2]);
   res[2] = (a[0] * b[1]) - (a[1] * b[0]);
}

//--------------------------------------
void m_matF_normalize_ASM(F32 *m)
{
   F32 col0[3], col1[3], col2[3];
   // extract columns 0 and 1
   col0[0] = m[0];
   col0[1] = m[4];
   col0[2] = m[8];

   col1[0] = m[1];
   col1[1] = m[5];
   col1[2] = m[9];

   // assure their relationsips to one another
   mCross(col0, col1, col2);
   mCross(col2, col0, col1);

   // assure their lengh is 1.0f
   m_point3F_normalize_ASM( col0 );
   m_point3F_normalize_ASM( col1 );
   m_point3F_normalize_ASM( col2 );

   // store the normalized columns
   m[0] = col0[0];
   m[4] = col0[1];
   m[8] = col0[2];
   
   m[1] = col1[0];
   m[5] = col1[1];
   m[9] = col1[2];
      
   m[2] = col2[0];
   m[6] = col2[1];
   m[10]= col2[2];
}

//--------------------------------------
void m_matF_x_point4F_ASM(const F32 *m, const F32 *p, F32 *presult)
{
   presult[0] = m[0]*p[0] + m[1]*p[1] + m[2]*p[2]  + m[3]*p[3];   
   presult[1] = m[4]*p[0] + m[5]*p[1] + m[6]*p[2]  + m[7]*p[3];   
   presult[2] = m[8]*p[0] + m[9]*p[1] + m[10]*p[2] + m[11]*p[3];   
   presult[3] = m[12]*p[0]+ m[13]*p[1]+ m[14]*p[2] + m[15]*p[3];   
}
