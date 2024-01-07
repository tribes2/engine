//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _MPOINT_H_
#define _MPOINT_H_

#define POINT_EPSILON (1e-4)

//-------------------------------------- Note: because of a circular dependancy,
//                                        the mMathFn.h header is below the point
//                                        class declarations... DMM
#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif

//------------------------------------------------------------------------------
class Point2I
{
   //-------------------------------------- Public data
  public:
   S32 x;
   S32 y;

   //-------------------------------------- Public interface
  public:
   Point2I();
   Point2I(const Point2I&);
   Point2I(const S32 in_x, const S32 in_y);

   //-------------------------------------- Non-math mutators and misc functions
   void set(const S32 in_x, const S32 in_y);
   void setMin(const Point2I&);
   void setMax(const Point2I&);

   //-------------------------------------- Math mutators
   void neg();
   void convolve(const Point2I&);

   //-------------------------------------- Queries
   bool isZero() const;
   F32  len() const;

   //-------------------------------------- Overloaded operators
  public:
   // Comparison operators
   bool operator==(const Point2I&) const;
   bool operator!=(const Point2I&) const;

   // Arithmetic w/ other points
   Point2I  operator+(const Point2I&) const;
   Point2I  operator-(const Point2I&) const;
   Point2I& operator+=(const Point2I&);
   Point2I& operator-=(const Point2I&);

   // Arithmetic w/ scalars
   Point2I  operator*(const S32) const;
   Point2I& operator*=(const S32);
   Point2I  operator/(const S32) const;
   Point2I& operator/=(const S32);

   // Unary operators
   Point2I operator-() const;
};

//------------------------------------------------------------------------------
class Point3I
{
   //-------------------------------------- Public data
  public:
   S32 x;
   S32 y;
   S32 z;

   //-------------------------------------- Public interface
  public:
   Point3I();
   Point3I(const Point3I&);
   Point3I(const S32 in_x, const S32 in_y, const S32 in_z);

   //-------------------------------------- Non-math mutators and misc functions
   void set(const S32 in_x, const S32 in_y, const S32 in_z);
   void setMin(const Point3I&);
   void setMax(const Point3I&);

   //-------------------------------------- Math mutators
   void neg();
   void convolve(const Point3I&);

   //-------------------------------------- Queries
   bool isZero() const;
   F32  len() const;

   //-------------------------------------- Overloaded operators
  public:
   // Comparison operators
   bool operator==(const Point3I&) const;
   bool operator!=(const Point3I&) const;

   // Arithmetic w/ other points
   Point3I  operator+(const Point3I&) const;
   Point3I  operator-(const Point3I&) const;
   Point3I& operator+=(const Point3I&);
   Point3I& operator-=(const Point3I&);

   // Arithmetic w/ scalars
   Point3I  operator*(const S32) const;
   Point3I& operator*=(const S32);
   Point3I  operator/(const S32) const;
   Point3I& operator/=(const S32);

   // Unary operators
   Point3I operator-() const;
};



//------------------------------------------------------------------------------
class Point2F
{
   //-------------------------------------- Public data
  public:
   F32 x;
   F32 y;

  public:
   Point2F();
   Point2F(const Point2F&);
   Point2F(const F32 _x, const F32 _y);

   //-------------------------------------- Non-math mutators and misc functions
  public:
   void set(const F32 _x, const F32 _y);

   void setMin(const Point2F&);
   void setMax(const Point2F&);

   void interpolate(const Point2F&, const Point2F&, const F32);

   operator F32*() { return (&x); }
   operator const F32*() const { return (&x); }

   //-------------------------------------- Queries
  public:
   bool  isZero() const;
   F32 len()    const;
   F32 lenSquared() const;

   //-------------------------------------- Mathematical mutators
  public:
   void neg();
   void normalize();
   void normalize(F32 val);
   void convolve(const Point2F&);
   void convolveInverse(const Point2F&);

   //-------------------------------------- Overloaded operators
  public:
   // Comparison operators
   bool operator==(const Point2F&) const;
   bool operator!=(const Point2F&) const;

   // Arithmetic w/ other points
   Point2F  operator+(const Point2F&) const;
   Point2F  operator-(const Point2F&) const;
   Point2F& operator+=(const Point2F&);
   Point2F& operator-=(const Point2F&);

   // Arithmetic w/ scalars
   Point2F  operator*(const F32) const;
   Point2F  operator/(const F32) const;
   Point2F& operator*=(const F32);
   Point2F& operator/=(const F32);

   // Unary operators
   Point2F operator-() const;
};


//------------------------------------------------------------------------------
class Point2D
{
   //-------------------------------------- Public data
  public:
   F64 x;
   F64 y;

  public:
   Point2D();
   Point2D(const Point2D&);
   Point2D(const F64 _x, const F64 _y);

   //-------------------------------------- Non-math mutators and misc functions
  public:
   void set(const F64 _x, const F64 _y);

   void setMin(const Point2D&);
   void setMax(const Point2D&);

   void interpolate(const Point2D&, const Point2D&, const F64);

   operator F64*() { return (&x); }
   operator const F64*() const { return (&x); }

   //-------------------------------------- Queries
  public:
   bool  isZero() const;
   F64 len()    const;
   F64 lenSquared() const;

   //-------------------------------------- Mathematical mutators
  public:
   void neg();
   void normalize();
   void normalize(F64 val);
   void convolve(const Point2D&);
   void convolveInverse(const Point2D&);

   //-------------------------------------- Overloaded operators
  public:
   // Comparison operators
   bool operator==(const Point2D&) const;
   bool operator!=(const Point2D&) const;

   // Arithmetic w/ other points
   Point2D  operator+(const Point2D&) const;
   Point2D  operator-(const Point2D&) const;
   Point2D& operator+=(const Point2D&);
   Point2D& operator-=(const Point2D&);

   // Arithmetic w/ scalars
   Point2D  operator*(const F64) const;
   Point2D  operator/(const F64) const;
   Point2D& operator*=(const F64);
   Point2D& operator/=(const F64);

   // Unary operators
   Point2D operator-() const;
};


//------------------------------------------------------------------------------
class Point3F
{
   //-------------------------------------- Public data
  public:
   F32 x;
   F32 y;
   F32 z;

  public:
   Point3F();
   Point3F(const Point3F&);
   Point3F(const F32 _x, const F32 _y, const F32 _z);

   //-------------------------------------- Non-math mutators and misc functions
  public:
   void set(const F32 _x, const F32 _y, const F32 _z);
   void set(const Point3F&);
   
   void setMin(const Point3F&);
   void setMax(const Point3F&);

   void interpolate(const Point3F&, const Point3F&, const F32);
   void zero();

   operator F32*() { return (&x); }
   operator const F32*() const { return (&x); }

   //-------------------------------------- Queries
  public:
   bool  isZero() const;
   F32   len()    const;
   F32   lenSquared() const;
   F32   magnitudeSafe();
   bool  equal( Point3F &compare );

   //-------------------------------------- Mathematical mutators
  public:
   void neg();
   void normalize();
   void normalizeSafe();
   void normalize(F32 val);
   void convolve(const Point3F&);
   void convolveInverse(const Point3F&);

   //-------------------------------------- Overloaded operators
  public:
   // Comparison operators
   bool operator==(const Point3F&) const;
   bool operator!=(const Point3F&) const;

   // Arithmetic w/ other points
   Point3F  operator+(const Point3F&) const;
   Point3F  operator-(const Point3F&) const;
   Point3F& operator+=(const Point3F&);
   Point3F& operator-=(const Point3F&);

   // Arithmetic w/ scalars
   Point3F  operator*(const F32) const;
   Point3F  operator/(const F32) const;
   Point3F& operator*=(const F32);
   Point3F& operator/=(const F32);

   Point3F  operator*(const Point3F&) const;
   Point3F& operator*=(const Point3F&);

   // Unary operators
   Point3F operator-() const;
};


typedef Point3F VectorF;
typedef Point3F EulerF;


//------------------------------------------------------------------------------
class Point3D
{
   //-------------------------------------- Public data
  public:
   F64 x;
   F64 y;
   F64 z;

  public:
   Point3D();
   Point3D(const Point3D&);
   Point3D(const F64 _x, const F64 _y, const F64 _z);

   //-------------------------------------- Non-math mutators and misc functions
  public:
   void set(const F64 _x, const F64 _y, const F64 _z);

   void setMin(const Point3D&);
   void setMax(const Point3D&);

   void interpolate(const Point3D&, const Point3D&, const F64);

   operator F64*() { return (&x); }
   operator const F64*() const { return (&x); }

   //-------------------------------------- Queries
  public:
   bool  isZero() const;
   F64 len()    const;
   F64 lenSquared() const;

   //-------------------------------------- Mathematical mutators
  public:
   void neg();
   void normalize();
   void normalize(F64 val);
   void convolve(const Point3D&);
   void convolveInverse(const Point3D&);

   //-------------------------------------- Overloaded operators
  public:
   // Comparison operators
   bool operator==(const Point3D&) const;
   bool operator!=(const Point3D&) const;

   // Arithmetic w/ other points
   Point3D  operator+(const Point3D&) const;
   Point3D  operator-(const Point3D&) const;
   Point3D& operator+=(const Point3D&);
   Point3D& operator-=(const Point3D&);

   // Arithmetic w/ scalars
   Point3D  operator*(const F64) const;
   Point3D  operator/(const F64) const;
   Point3D& operator*=(const F64);
   Point3D& operator/=(const F64);

   // Unary operators
   Point3D operator-() const;
};



//------------------------------------------------------------------------------
class Point4F
{
   //-------------------------------------- Public data
  public:
   F32 x;
   F32 y;
   F32 z;
   F32 w;

  public:
   Point4F();
   Point4F(const Point4F&);
   Point4F(const F32 _x, const F32 _y, const F32 _z, const F32 _w);

   void set(const F32 _x, const F32 _y, const F32 _z, const F32 _w);
   void interpolate(const Point4F& _pt1, const Point4F& _pt2, const F32 _factor);

   operator F32*() { return (&x); }
   operator const F32*() const { return (&x); }
};


typedef Point4F Vector4F;


#ifndef _MMATHFN_H_
#include "Math/mMathFn.h"
#endif

//------------------------------------------------------------------------------
//-------------------------------------- Inline functions inclusions


//------------------------------------------------------------------------------
//-------------------------------------- Point2I
//
inline Point2I::Point2I()
{
   //
}


inline Point2I::Point2I(const Point2I& _copy)
 : x(_copy.x), y(_copy.y)
{
   //
}


inline Point2I::Point2I(const S32 _x, const S32 _y)
 : x(_x), y(_y)
{
   //
}


inline void Point2I::set(const S32 _x, const S32 _y)
{
   x = _x;
   y = _y;
}


inline void Point2I::setMin(const Point2I& _test)
{
   x = (_test.x < x) ? _test.x : x;
   y = (_test.y < y) ? _test.y : y;
}


inline void Point2I::setMax(const Point2I& _test)
{
   x = (_test.x > x) ? _test.x : x;
   y = (_test.y > y) ? _test.y : y;
}


inline void Point2I::neg()
{
   x = -x;
   y = -y;
}

inline void Point2I::convolve(const Point2I& c)
{
   x *= c.x;
   y *= c.y;
}

inline bool Point2I::isZero() const
{
   return ((x == 0) && (y == 0));
}


inline F32 Point2I::len() const
{
   return mSqrt(F32(x*x + y*y));
}

inline bool Point2I::operator==(const Point2I& _test) const
{
   return ((x == _test.x) && (y == _test.y));
}


inline bool Point2I::operator!=(const Point2I& _test) const
{
   return (operator==(_test) == false);
}


inline Point2I Point2I::operator+(const Point2I& _add) const
{
   return Point2I(x + _add.x, y + _add.y);
}


inline Point2I Point2I::operator-(const Point2I& _rSub) const
{
   return Point2I(x - _rSub.x, y - _rSub.y);
}


inline Point2I& Point2I::operator+=(const Point2I& _add)
{
   x += _add.x;
   y += _add.y;

   return *this;
}


inline Point2I& Point2I::operator-=(const Point2I& _rSub)
{
   x -= _rSub.x;
   y -= _rSub.y;

   return *this;
}


inline Point2I Point2I::operator-() const
{
   return Point2I(-x, -y);
}


inline Point2I Point2I::operator*(const S32 mul) const
{
   return Point2I(x * mul, y * mul);
}

inline Point2I Point2I::operator/(const S32 div) const
{
   AssertFatal(div != 0, "Error, div by zero attempted");
   return Point2I(x/div, y/div);
}


inline Point2I& Point2I::operator*=(const S32 mul)
{
   x *= mul;
   y *= mul;

   return *this;
}


inline Point2I& Point2I::operator/=(const S32 div)
{
   AssertFatal(div != 0, "Error, div by zero attempted");

   x /= div;
   y /= div;

   return *this;
}




//------------------------------------------------------------------------------
//-------------------------------------- Point3I
//
inline Point3I::Point3I()
{
   //
}


inline Point3I::Point3I(const Point3I& _copy)
 : x(_copy.x), y(_copy.y), z(_copy.z)
{
   //
}


inline Point3I::Point3I(const S32 _x, const S32 _y, const S32 _z)
 : x(_x), y(_y), z(_z)
{
   //
}


inline void Point3I::set(const S32 _x, const S32 _y, const S32 _z)
{
   x = _x;
   y = _y;
   z = _z;
}


inline void Point3I::setMin(const Point3I& _test)
{
   x = (_test.x < x) ? _test.x : x;
   y = (_test.y < y) ? _test.y : y;
   z = (_test.z < z) ? _test.z : z;
}


inline void Point3I::setMax(const Point3I& _test)
{
   x = (_test.x > x) ? _test.x : x;
   y = (_test.y > y) ? _test.y : y;
   z = (_test.z > z) ? _test.z : z;
}


inline void Point3I::neg()
{
   x = -x;
   y = -y;
   z = -z;
}

inline F32 Point3I::len() const
{
   return mSqrt(F32(x*x + y*y + z*z));
}

inline void Point3I::convolve(const Point3I& c)
{
   x *= c.x;
   y *= c.y;
   z *= c.z;
}

inline bool Point3I::isZero() const
{
   return ((x == 0) && (y == 0) && (z == 0));
}


inline bool Point3I::operator==(const Point3I& _test) const
{
   return ((x == _test.x) && (y == _test.y) && (z == _test.z));
}


inline bool Point3I::operator!=(const Point3I& _test) const
{
   return (operator==(_test) == false);
}


inline Point3I Point3I::operator+(const Point3I& _add) const
{
   return Point3I(x + _add.x, y + _add.y, z + _add.z);
}


inline Point3I Point3I::operator-(const Point3I& _rSub) const
{
   return Point3I(x - _rSub.x, y - _rSub.y, z - _rSub.z);
}


inline Point3I& Point3I::operator+=(const Point3I& _add)
{
   x += _add.x;
   y += _add.y;
   z += _add.z;

   return *this;
}


inline Point3I& Point3I::operator-=(const Point3I& _rSub)
{
   x -= _rSub.x;
   y -= _rSub.y;
   z -= _rSub.z;

   return *this;
}


inline Point3I Point3I::operator-() const
{
   return Point3I(-x, -y, -z);
}


inline Point3I Point3I::operator*(const S32 mul) const
{
   return Point3I(x * mul, y * mul, z * mul);
}


inline Point3I Point3I::operator/(const S32 div) const
{
   AssertFatal(div != 0, "Error, div by zero attempted");
   return Point3I(x/div, y/div, z/div);
}


inline Point3I& Point3I::operator*=(const S32 mul)
{
   x *= mul;
   y *= mul;
   z *= mul;

   return *this;
}


inline Point3I& Point3I::operator/=(const S32 div)
{
   AssertFatal(div != 0, "Error, div by zero attempted");

   x /= div;
   y /= div;
   z /= div;

   return *this;
}






//------------------------------------------------------------------------------
//-------------------------------------- Point2F
//
inline Point2F::Point2F()
{
   //
}


inline Point2F::Point2F(const Point2F& _copy)
 : x(_copy.x), y(_copy.y)
{
   //
}


inline Point2F::Point2F(const F32 _x, const F32 _y)
 : x(_x), y(_y)
{
}


inline void Point2F::set(const F32 _x, const F32 _y)
{
   x = _x;
   y = _y;
}


inline void Point2F::setMin(const Point2F& _test)
{
   x = (_test.x < x) ? _test.x : x;
   y = (_test.y < y) ? _test.y : y;
}


inline void Point2F::setMax(const Point2F& _test)
{
   x = (_test.x > x) ? _test.x : x;
   y = (_test.y > y) ? _test.y : y;
}


inline void Point2F::interpolate(const Point2F& _rFrom, const Point2F& _to, const F32 _factor)
{
   AssertFatal(_factor >= 0.0f && _factor <= 1.0f, "Out of bound interpolation factor");
   x = (_rFrom.x * (1.0f - _factor)) + (_to.x * _factor);
   y = (_rFrom.y * (1.0f - _factor)) + (_to.y * _factor);
}


inline bool Point2F::isZero() const
{
   return (x == 0.0f) && (y == 0.0f);
}


inline F32 Point2F::lenSquared() const
{
   return (x * x) + (y * y);
}


inline void Point2F::neg()
{
   x = -x;
   y = -y;
}

inline void Point2F::convolve(const Point2F& c)
{
   x *= c.x;
   y *= c.y;
}


inline void Point2F::convolveInverse(const Point2F& c)
{
   x /= c.x;
   y /= c.y;
}


inline bool Point2F::operator==(const Point2F& _test) const
{
   return (x == _test.x) && (y == _test.x);
}


inline bool Point2F::operator!=(const Point2F& _test) const
{
   return operator==(_test) == false;
}


inline Point2F Point2F::operator+(const Point2F& _add) const
{
   return Point2F(x + _add.x, y + _add.y);
}


inline Point2F Point2F::operator-(const Point2F& _rSub) const
{
   return Point2F(x - _rSub.x, y - _rSub.y);
}


inline Point2F& Point2F::operator+=(const Point2F& _add)
{
   x += _add.x;
   y += _add.y;

   return *this;
}


inline Point2F& Point2F::operator-=(const Point2F& _rSub)
{
   x -= _rSub.x;
   y -= _rSub.y;

   return *this;
}


inline Point2F Point2F::operator*(const F32 _mul) const
{
   return Point2F(x * _mul, y * _mul);
}


inline Point2F Point2F::operator/(const F32 _div) const
{
   AssertFatal(_div != 0.0f, "Error, div by zero attempted");

   F32 inv = 1.0f / _div;

   return Point2F(x * inv, y * inv);
}


inline Point2F& Point2F::operator*=(const F32 _mul)
{
   x *= _mul;
   y *= _mul;

   return *this;
}


inline Point2F& Point2F::operator/=(const F32 _div)
{
   AssertFatal(_div != 0.0f, "Error, div by zero attempted");

   F32 inv = 1.0f / _div;

   x *= inv;
   y *= inv;

   return *this;
}


inline Point2F Point2F::operator-() const
{
   return Point2F(-x, -y);
}

inline F32 Point2F::len() const
{
   return mSqrt(x*x + y*y);
}

inline void Point2F::normalize()
{
   m_point2F_normalize(*this);
}

inline void Point2F::normalize(F32 val)
{
   m_point2F_normalize_f(*this, val);
}


//------------------------------------------------------------------------------
//-------------------------------------- Point2D
//
inline Point2D::Point2D()
{
   //
}


inline Point2D::Point2D(const Point2D& _copy)
 : x(_copy.x), y(_copy.y)
{
   //
}


inline Point2D::Point2D(const F64 _x, const F64 _y)
 : x(_x), y(_y)
{
}


inline void Point2D::set(const F64 _x, const F64 _y)
{
   x = _x;
   y = _y;
}


inline void Point2D::setMin(const Point2D& _test)
{
   x = (_test.x < x) ? _test.x : x;
   y = (_test.y < y) ? _test.y : y;
}


inline void Point2D::setMax(const Point2D& _test)
{
   x = (_test.x > x) ? _test.x : x;
   y = (_test.y > y) ? _test.y : y;
}


inline void Point2D::interpolate(const Point2D& _rFrom, const Point2D& _to, const F64 _factor)
{
   AssertFatal(_factor >= 0.0f && _factor <= 1.0f, "Out of bound interpolation factor");
   x = (_rFrom.x * (1.0f - _factor)) + (_to.x * _factor);
   y = (_rFrom.y * (1.0f - _factor)) + (_to.y * _factor);
}


inline bool Point2D::isZero() const
{
   return (x == 0.0f) && (y == 0.0f);
}


inline F64 Point2D::lenSquared() const
{
   return (x * x) + (y * y);
}


inline void Point2D::neg()
{
   x = -x;
   y = -y;
}

inline void Point2D::convolve(const Point2D& c)
{
   x *= c.x;
   y *= c.y;
}

inline void Point2D::convolveInverse(const Point2D& c)
{
   x /= c.x;
   y /= c.y;
}

inline bool Point2D::operator==(const Point2D& _test) const
{
   return (x == _test.x) && (y == _test.x);
}


inline bool Point2D::operator!=(const Point2D& _test) const
{
   return operator==(_test) == false;
}


inline Point2D Point2D::operator+(const Point2D& _add) const
{
   return Point2D(x + _add.x, y + _add.y);
}


inline Point2D Point2D::operator-(const Point2D& _rSub) const
{
   return Point2D(x - _rSub.x, y - _rSub.y);
}


inline Point2D& Point2D::operator+=(const Point2D& _add)
{
   x += _add.x;
   y += _add.y;

   return *this;
}


inline Point2D& Point2D::operator-=(const Point2D& _rSub)
{
   x -= _rSub.x;
   y -= _rSub.y;

   return *this;
}


inline Point2D Point2D::operator*(const F64 _mul) const
{
   return Point2D(x * _mul, y * _mul);
}


inline Point2D Point2D::operator/(const F64 _div) const
{
   AssertFatal(_div != 0.0f, "Error, div by zero attempted");

   F64 inv = 1.0f / _div;

   return Point2D(x * inv, y * inv);
}


inline Point2D& Point2D::operator*=(const F64 _mul)
{
   x *= _mul;
   y *= _mul;

   return *this;
}


inline Point2D& Point2D::operator/=(const F64 _div)
{
   AssertFatal(_div != 0.0f, "Error, div by zero attempted");

   F64 inv = 1.0f / _div;

   x *= inv;
   y *= inv;

   return *this;
}


inline Point2D Point2D::operator-() const
{
   return Point2D(-x, -y);
}

inline F64 Point2D::len() const
{
   return mSqrtD(x*x + y*y);
}

inline void Point2D::normalize()
{
   m_point2D_normalize(*this);
}

inline void Point2D::normalize(F64 val)
{
   m_point2D_normalize_f(*this, val);
}


//------------------------------------------------------------------------------
//-------------------------------------- Point3F
//
inline Point3F::Point3F()
#ifdef __linux
 : x(0.f), y(0.f), z(0.f)
#endif
{
// Uninitialized points are definitely a problem.
// Enable the following code to see how often they crop up.
#ifdef DEBUG_MATH
   *(U32 *)&x = 0x7FFFFFFA;
   *(U32 *)&y = 0x7FFFFFFB;
   *(U32 *)&z = 0x7FFFFFFC;
#endif
}


inline Point3F::Point3F(const Point3F& _copy)
 : x(_copy.x), y(_copy.y), z(_copy.z)
{
   //
}


inline Point3F::Point3F(const F32 _x, const F32 _y, const F32 _z)
 : x(_x), y(_y), z(_z)
{
   //
}


inline void Point3F::set(const F32 _x, const F32 _y, const F32 _z)
{
   x = _x;
   y = _y;
   z = _z;
}

inline void Point3F::set(const Point3F& copy)
{
   x = copy.x;
   y = copy.y;
   z = copy.z;
}

inline void Point3F::setMin(const Point3F& _test)
{
   x = (_test.x < x) ? _test.x : x;
   y = (_test.y < y) ? _test.y : y;
   z = (_test.z < z) ? _test.z : z;
}


inline void Point3F::setMax(const Point3F& _test)
{
   x = (_test.x > x) ? _test.x : x;
   y = (_test.y > y) ? _test.y : y;
   z = (_test.z > z) ? _test.z : z;
}


inline void Point3F::interpolate(const Point3F& _from, const Point3F& _to, const F32 _factor)
{
   AssertFatal(_factor >= 0.0f && _factor <= 1.0f, "Out of bound interpolation factor");
   m_point3F_interpolate( _from, _to, _factor, *this);
}

inline void Point3F::zero()
{
   x = y = z = 0.0f;
}

inline bool Point3F::isZero() const
{
   return ((x*x) <= POINT_EPSILON) && ((y*y) <= POINT_EPSILON) && ((z*z) <= POINT_EPSILON );
}

inline bool Point3F::equal( Point3F &compare )
{
   return( ( mFabs( x - compare.x ) < POINT_EPSILON ) &&
           ( mFabs( y - compare.y ) < POINT_EPSILON ) &&
           ( mFabs( z - compare.z ) < POINT_EPSILON ) );
}

inline void Point3F::neg()
{
   x = -x;
   y = -y;
   z = -z;
}

inline void Point3F::convolve(const Point3F& c)
{
   x *= c.x;
   y *= c.y;
   z *= c.z;
}

inline void Point3F::convolveInverse(const Point3F& c)
{
   x /= c.x;
   y /= c.y;
   z /= c.z;
}

inline F32 Point3F::lenSquared() const
{
   return (x * x) + (y * y) + (z * z);
}


inline F32 Point3F::len() const
{
   return mSqrt(x*x + y*y + z*z);
}


inline void Point3F::normalize()
{
   m_point3F_normalize(*this);
}

inline F32 Point3F::magnitudeSafe()
{
   if( isZero() )
   {
      return 0.0f;
   }
   else
   {
      return len();
   }
}

inline void Point3F::normalizeSafe()
{
	F32 vmag = magnitudeSafe();

	if( vmag > POINT_EPSILON )
	{
		*this *= F32(1.0 / vmag);
	}
}


inline void Point3F::normalize(F32 val)
{
   m_point3F_normalize_f(*this, val);
}

inline bool Point3F::operator==(const Point3F& _test) const
{
   return (x == _test.x) && (y == _test.y) && (z == _test.z);
}


inline bool Point3F::operator!=(const Point3F& _test) const
{
   return operator==(_test) == false;
}


inline Point3F Point3F::operator+(const Point3F& _add) const
{
   return Point3F(x + _add.x, y + _add.y,  z + _add.z);
}


inline Point3F Point3F::operator-(const Point3F& _rSub) const
{
   return Point3F(x - _rSub.x, y - _rSub.y, z - _rSub.z);
}


inline Point3F& Point3F::operator+=(const Point3F& _add)
{
   x += _add.x;
   y += _add.y;
   z += _add.z;

   return *this;
}


inline Point3F& Point3F::operator-=(const Point3F& _rSub)
{
   x -= _rSub.x;
   y -= _rSub.y;
   z -= _rSub.z;

   return *this;
}


inline Point3F Point3F::operator*(const F32 _mul) const
{
   return Point3F(x * _mul, y * _mul, z * _mul);
}

inline Point3F Point3F::operator*(const Point3F &_vec) const
{
   return Point3F(x * _vec.x, y * _vec.y, z * _vec.z);
}

inline Point3F Point3F::operator/(const F32 _div) const
{
   AssertFatal(_div != 0.0f, "Error, div by zero attempted");

   F32 inv = 1.0f / _div;

   return Point3F(x * inv, y * inv, z * inv);
}


inline Point3F& Point3F::operator*=(const F32 _mul)
{
   x *= _mul;
   y *= _mul;
   z *= _mul;

   return *this;
}


inline Point3F& Point3F::operator*=(const Point3F &_vec)
{
   x *= _vec.x;
   y *= _vec.y;
   z *= _vec.z;
   return *this;
}


inline Point3F& Point3F::operator/=(const F32 _div)
{
   AssertFatal(_div != 0.0f, "Error, div by zero attempted");

   F32 inv = 1.0f / _div;
   x *= inv;
   y *= inv;
   z *= inv;

   return *this;
}


inline Point3F Point3F::operator-() const
{
   return Point3F(-x, -y, -z);
}


//------------------------------------------------------------------------------
//-------------------------------------- Point3D
//
inline Point3D::Point3D()
{
   //
}


inline Point3D::Point3D(const Point3D& _copy)
 : x(_copy.x), y(_copy.y), z(_copy.z)
{
   //
}


inline Point3D::Point3D(const F64 _x, const F64 _y, const F64 _z)
 : x(_x), y(_y), z(_z)
{
   //
}


inline void Point3D::set(const F64 _x, const F64 _y, const F64 _z)
{
   x = _x;
   y = _y;
   z = _z;
}


inline void Point3D::setMin(const Point3D& _test)
{
   x = (_test.x < x) ? _test.x : x;
   y = (_test.y < y) ? _test.y : y;
   z = (_test.z < z) ? _test.z : z;
}


inline void Point3D::setMax(const Point3D& _test)
{
   x = (_test.x > x) ? _test.x : x;
   y = (_test.y > y) ? _test.y : y;
   z = (_test.z > z) ? _test.z : z;
}


inline void Point3D::interpolate(const Point3D& _from, const Point3D& _to, const F64 _factor)
{
   AssertFatal(_factor >= 0.0f && _factor <= 1.0f, "Out of bound interpolation factor");
   m_point3D_interpolate( _from, _to, _factor, *this);
}


inline bool Point3D::isZero() const
{
   return (x == 0.0f) && (y == 0.0f) && (z == 0.0f);
}


inline void Point3D::neg()
{
   x = -x;
   y = -y;
   z = -z;
}

inline void Point3D::convolve(const Point3D& c)
{
   x *= c.x;
   y *= c.y;
   z *= c.z;
}

inline void Point3D::convolveInverse(const Point3D& c)
{
   x /= c.x;
   y /= c.y;
   z /= c.z;
}

inline F64 Point3D::lenSquared() const
{
   return (x * x) + (y * y) + (z * z);
}


inline F64 Point3D::len() const
{
   return mSqrtD(x*x + y*y + z*z);
}


inline void Point3D::normalize()
{
   m_point3D_normalize(*this);
}

inline void Point3D::normalize(F64 val)
{
   m_point3D_normalize_f(*this, val);
}

inline bool Point3D::operator==(const Point3D& _test) const
{
   return (x == _test.x) && (y == _test.y) && (z == _test.z);
}


inline bool Point3D::operator!=(const Point3D& _test) const
{
   return operator==(_test) == false;
}


inline Point3D Point3D::operator+(const Point3D& _add) const
{
   return Point3D(x + _add.x, y + _add.y,  z + _add.z);
}


inline Point3D Point3D::operator-(const Point3D& _rSub) const
{
   return Point3D(x - _rSub.x, y - _rSub.y, z - _rSub.z);
}


inline Point3D& Point3D::operator+=(const Point3D& _add)
{
   x += _add.x;
   y += _add.y;
   z += _add.z;

   return *this;
}


inline Point3D& Point3D::operator-=(const Point3D& _rSub)
{
   x -= _rSub.x;
   y -= _rSub.y;
   z -= _rSub.z;

   return *this;
}


inline Point3D Point3D::operator*(const F64 _mul) const
{
   return Point3D(x * _mul, y * _mul, z * _mul);
}


inline Point3D Point3D::operator/(const F64 _div) const
{
   AssertFatal(_div != 0.0f, "Error, div by zero attempted");

   F64 inv = 1.0f / _div;

   return Point3D(x * inv, y * inv, z * inv);
}


inline Point3D& Point3D::operator*=(const F64 _mul)
{
   x *= _mul;
   y *= _mul;
   z *= _mul;

   return *this;
}


inline Point3D& Point3D::operator/=(const F64 _div)
{
   AssertFatal(_div != 0.0f, "Error, div by zero attempted");

   F64 inv = 1.0f / _div;
   x *= inv;
   y *= inv;
   z *= inv;

   return *this;
}


inline Point3D Point3D::operator-() const
{
   return Point3D(-x, -y, -z);
}

//------------------------------------------------------------------------------
//-------------------------------------- Point4F
//
inline Point4F::Point4F()
{
   //
}


inline Point4F::Point4F(const Point4F& _copy)
 : x(_copy.x), y(_copy.y), z(_copy.z), w(_copy.w)
{
   //
}


inline Point4F::Point4F(const F32 _x, const F32 _y, const F32 _z, const F32 _w)
 : x(_x), y(_y), z(_z), w(_w)
{
   //
}


inline void Point4F::set(const F32 _x, const F32 _y, const F32 _z, const F32 _w)
{
   x = _x;
   y = _y;
   z = _z;
   w = _w;
}


inline void Point4F::interpolate(const Point4F& _from, const Point4F& _to, const F32 _factor)
{
   x = (_from.x * (1.0f - _factor)) + (_to.x * _factor);
   y = (_from.y * (1.0f - _factor)) + (_to.y * _factor);
   z = (_from.z * (1.0f - _factor)) + (_to.z * _factor);
   w = (_from.w * (1.0f - _factor)) + (_to.w * _factor);
}

//--------------------------------------------------------------------------
//-------------------------------------- NON-MEMBER Operators
//
inline Point2I operator*(const S32 mul, const Point2I& multiplicand)
{
   return multiplicand * mul;
}

inline Point3I operator*(const S32 mul, const Point3I& multiplicand)
{
   return multiplicand * mul;
}

inline Point2F operator*(const F32 mul, const Point2F& multiplicand)
{
   return multiplicand * mul;
}

inline Point3F operator*(const F32 mul, const Point3F& multiplicand)
{
   return multiplicand * mul;
}

inline Point2D operator*(const F64 mul, const Point2D& multiplicand)
{
   return multiplicand * mul;
}

inline Point3D operator*(const F64 mul, const Point3D& multiplicand)
{
   return multiplicand * mul;
}

#endif // _POINT_H_
