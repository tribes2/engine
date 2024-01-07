//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _MMATRIX_H_
#define _MMATRIX_H_


#ifndef _MMATH_H_
#include "Math/mMath.h"
#endif


class MatrixF
{
private:
   F32 m[16];     // Note: this is stored in ROW MAJOR format.  OpenGL is
                  //  COLUMN MAJOR.  Transpose before sending down.

public:
   explicit MatrixF(bool identity=false);     // DEFAULT: UN-Initialized TRUE: Identity
   explicit MatrixF( const EulerF &e);              
   MatrixF( const EulerF &e, const Point3F& p);              

   static U32 idx(U32 i, U32 j) { return (i + j*4); }
   
   MatrixF& set( const EulerF &e);
   MatrixF& set( const EulerF &e, const Point3F& p);
   MatrixF& setCrossProduct( const Point3F &p);
   MatrixF& setTensorProduct( const Point3F &p, const Point3F& q);

   operator F32*() { return (m); }
   operator const F32*() const { return (m); }

   bool isAffine() const;
   bool isIdentity() const;

   MatrixF& identity();
   MatrixF& inverse();
   MatrixF& affineInverse();
   MatrixF& transpose();
   MatrixF& scale(const Point3F& p);                     // M * Matrix(p) -> M

   bool fullInverse(); // computes inverse of full 4x4 matrix...returns false and performs no inverse if det 0
                       // note: in most cases you want to use the normal inverse function.  This method should
                       //       be used if the matrix has something other than (0,0,0,1) in the bottom row.
   
   void transposeTo(F32 *matrix) const;
   
   void normalize();
   void getColumn(S32 col, Point4F *cptr) const;
   void getColumn(S32 col, Point3F *cptr) const;
   void setColumn(S32 col, const Point4F& cptr);
   void setColumn(S32 col, const Point3F& cptr);

   void getRow(S32 col, Point4F *cptr) const;
   void getRow(S32 col, Point3F *cptr) const;
   void setRow(S32 col, const Point4F& cptr);
   void setRow(S32 col, const Point3F& cptr);

   Point3F   getPosition() const;
   void      setPosition( const Point3F &pos ){ setColumn( 3, pos ); }

   MatrixF&  mul(const MatrixF &a);                    // M * a -> M
   MatrixF&  mul(const MatrixF &a, const MatrixF &b);  // a * b -> M

   // Scalar multiplies
   MatrixF&  mul(const F32 a);                         // M * a -> M
   MatrixF&  mul(const MatrixF &a, const F32 b);       // a * b -> M


   void mul( Point4F& p ) const;                    // M * p -> p (full [4x4] * [1x4])
   void mulP( Point3F& p ) const;             // M * p -> p (assume w = 1.0f)
   void mulP( const Point3F &p, Point3F *d) const;  // M * p -> d (assume w = 1.0f)
   void mulV( VectorF& p ) const;             // M * v -> v (assume w = 0.0f)
   void mulV( const VectorF &p, Point3F *d) const;  // M * v -> d (assume w = 0.0f)

   void mul(Box3F& b) const; // Axial box -> Axial Box


};

//--------------------------------------
// Inline Functions

inline MatrixF::MatrixF(bool _identity)
{
   if (_identity)
      identity();
}   

inline MatrixF::MatrixF( const EulerF &e )
{
   set(e);
}   

inline MatrixF::MatrixF( const EulerF &e, const Point3F& p )
{
   set(e,p);
}   

inline MatrixF& MatrixF::set( const EulerF &e)
{
   m_matF_set_euler( e, *this );   
   return (*this);
}   


inline MatrixF& MatrixF::set( const EulerF &e, const Point3F& p)
{
   m_matF_set_euler_point( e, p, *this );   
   return (*this);
}

inline MatrixF& MatrixF::setCrossProduct( const Point3F &p)
{
   m[1] = -(m[4] = p.z);
   m[8] = -(m[2] = p.y);
   m[6] = -(m[9] = p.x);
   m[0] = m[3] = m[5] = m[7] = m[10] = m[11] =
      m[12] = m[13] = m[14] = 0;
   m[15] = 1;
   return (*this);
}	

inline MatrixF& MatrixF::setTensorProduct( const Point3F &p, const Point3F &q)
{
   m[0] = p.x * q.x;
   m[1] = p.x * q.y;
   m[2] = p.x * q.z;
   m[4] = p.y * q.x;
   m[5] = p.y * q.y;
   m[6] = p.y * q.z;
   m[8] = p.z * q.x;
   m[9] = p.z * q.y;
   m[10] = p.z * q.z;
   m[3] = m[7] = m[11] = m[12] = m[13] = m[14] = 0;
   m[15] = 1;
   return (*this);
}

inline bool MatrixF::isIdentity() const
{
   return 
   m[0]  == 1.0f &&
   m[1]  == 0.0f &&
   m[2]  == 0.0f &&
   m[3]  == 0.0f &&
   m[4]  == 0.0f &&
   m[5]  == 1.0f &&
   m[6]  == 0.0f &&
   m[7]  == 0.0f &&
   m[8]  == 0.0f &&
   m[9]  == 0.0f &&
   m[10] == 1.0f &&
   m[11] == 0.0f &&
   m[12] == 0.0f &&
   m[13] == 0.0f &&
   m[14] == 0.0f &&
   m[15] == 1.0f;
}

inline MatrixF& MatrixF::identity()
{
   m[0]  = 1.0f;
   m[1]  = 0.0f;
   m[2]  = 0.0f;
   m[3]  = 0.0f;
   m[4]  = 0.0f;
   m[5]  = 1.0f;
   m[6]  = 0.0f;
   m[7]  = 0.0f;
   m[8]  = 0.0f;
   m[9]  = 0.0f;
   m[10] = 1.0f;
   m[11] = 0.0f;
   m[12] = 0.0f;
   m[13] = 0.0f;
   m[14] = 0.0f;
   m[15] = 1.0f;
   return (*this);
}   


inline MatrixF& MatrixF::inverse()
{
   m_matF_inverse(m);
   return (*this);
}   

inline MatrixF& MatrixF::affineInverse()
{
//   AssertFatal(isAffine() == true, "Error, this matrix is not an affine transform");
   m_matF_affineInverse(m);
   return (*this);
}

inline MatrixF& MatrixF::transpose()
{
   m_matF_transpose(m);
   return (*this);
}   

inline MatrixF& MatrixF::scale(const Point3F& p)
{
   m_matF_scale(m,p);
   return *this;
}

inline void MatrixF::normalize()
{
   m_matF_normalize(m);
}

inline MatrixF& MatrixF::mul( const MatrixF &a )
{  // M * a -> M
   MatrixF tempThis(*this);
   m_matF_x_matF(tempThis, a, *this);
   return (*this);
}                       


inline MatrixF& MatrixF::mul( const MatrixF &a, const MatrixF &b )
{  // a * b -> M
   m_matF_x_matF(a, b, *this);
   return (*this);
}     


inline MatrixF& MatrixF::mul(const F32 a)
{ 
   for (U32 i = 0; i < 16; i++)
      m[i] *= a;

   return *this;
}                       


inline MatrixF& MatrixF::mul(const MatrixF &a, const F32 b)
{
   *this = a;
   mul(b);

   return *this;
}     

inline void MatrixF::mul( Point4F& p ) const
{
   Point4F temp;
   m_matF_x_point4F(*this, &p.x, &temp.x);
   p = temp;
}

inline void MatrixF::mulP( Point3F& p) const
{
   // M * p -> d
   Point3F d;
   m_matF_x_point3F(*this, &p.x, &d.x);
   p = d;
}

inline void MatrixF::mulP( const Point3F &p, Point3F *d) const
{
   // M * p -> d
   m_matF_x_point3F(*this, &p.x, &d->x);
}

inline void MatrixF::mulV( VectorF& v) const
{
   // M * v -> v
   VectorF temp;
   m_matF_x_vectorF(*this, &v.x, &temp.x);
   v = temp;
}

inline void MatrixF::mulV( const VectorF &v, Point3F *d) const
{
   // M * v -> d
   m_matF_x_vectorF(*this, &v.x, &d->x);
}

inline void MatrixF::mul(Box3F& b) const
{
   m_matF_x_box3F(*this, &b.min.x, &b.max.x);
}

inline void MatrixF::getColumn(S32 col, Point4F *cptr) const
{
   cptr->x = m[col];
   cptr->y = m[col+4];
   cptr->z = m[col+8];
   cptr->w = m[col+12];
}

inline void MatrixF::getColumn(S32 col, Point3F *cptr) const
{
   cptr->x = m[col];
   cptr->y = m[col+4];
   cptr->z = m[col+8];
}

inline void MatrixF::setColumn(S32 col, const Point4F &cptr)
{
   m[col]   = cptr.x;
   m[col+4] = cptr.y; 
   m[col+8] = cptr.z; 
   m[col+12]= cptr.w; 
}

inline void MatrixF::setColumn(S32 col, const Point3F &cptr)
{
   m[col]   = cptr.x;
   m[col+4] = cptr.y; 
   m[col+8] = cptr.z; 
}


inline void MatrixF::getRow(S32 col, Point4F *cptr) const
{
	col *= 4;
   cptr->x = m[col++];
   cptr->y = m[col++];
   cptr->z = m[col++];
   cptr->w = m[col];
}

inline void MatrixF::getRow(S32 col, Point3F *cptr) const
{
	col *= 4;
   cptr->x = m[col++];
   cptr->y = m[col++];
   cptr->z = m[col];
}

inline void MatrixF::setRow(S32 col, const Point4F &cptr)
{
	col *= 4;
   m[col++] = cptr.x;
   m[col++] = cptr.y; 
   m[col++] = cptr.z; 
   m[col]   = cptr.w; 
}

inline void MatrixF::setRow(S32 col, const Point3F &cptr)
{
	col *= 4;
   m[col++] = cptr.x;
   m[col++] = cptr.y; 
   m[col]   = cptr.z; 
}

// not too speedy, but convienient
inline Point3F MatrixF::getPosition() const
{
   Point3F pos;
   getColumn( 3, &pos );
   return pos;
}

#endif //_MMATRIX_H_
