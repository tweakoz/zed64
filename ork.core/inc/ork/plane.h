///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#ifndef __PLANEH__
#define __PLANEH__

#include <ork/cvector3.h>
#include <ork/cvector4.h>
#include <ork/cmatrix4.h>
#include <ork/line.h>

////////////////////////////////////////////////////////////////////////////////
//	misc
////////////////////////////////////////////////////////////////////////////////
namespace ork{
///////////////////////////////////////////////////////////////////////////////

//! templatized 3D plane manipulation class
template <typename T> class TPlane
{
	static T Abs( T in );
	static T Epsilon();

	public: //

    //////////
    TVector3<T> n;
    T d;
    //////////
    TPlane();//! set explicitly to 0,0,0,0
    TPlane( const TVector4<T> &vec );//! set explicitly
    TPlane( const TVector3<T> &vec, T dist );//! set explicitly
    TPlane( T nx, T ny, T nz, T dist );//! set explicitly
    TPlane( T *f32p ); //! set explicitly
    TPlane( const TVector3<T> &NormalVec, const TVector3<T> &PosVec ); //! calc given normal and position of plane origin
    void CalcFromNormalAndOrigin( const TVector3<T> &NormalVec, const TVector3<T> &PosVec ); //! calc given normal and position of plane origin
    ~TPlane();
    void Reset(void);
    bool IsPointInFront( const TVector3<T> &pt ) const;
    bool IsPointBehind( const TVector3<T> &pt ) const;
    void CalcD( const TVector3<T> &pt );
    bool IsOn( const TVector3<T> &pt ) const;
    void CalcNormal( const TVector3<T> &pta, const TVector3<T> &ptb, const TVector3<T> &ptc);
   
	//////////////////////////////////
	
	bool Intersect( const TLineSegment3<T>& seg, T &dis, TVector3<T> &res) const;
	bool Intersect( const TRay3<T>& ray, T &dis, TVector3<T> &res ) const;
	bool Intersect( const TRay3<T>& ray, T &dis ) const;

	//////////////////////////////////

	T GetPointDistance( const TVector3<T> &pt ) const;
    const TVector3<T> &GetNormal(void) const;
    const T &GetD(void) const;
    void crossProduct( F64 ii1, F64 jj1, F64 kk1, F64 ii2, F64 jj2, F64 kk2, F64 &iicp, F64 &jjcp, F64 &kkcp) const;
    void CalcPlaneFromTriangle( const TVector3<T> & p0, const TVector3<T> & p1, const TVector3<T> & p2, f64 ftolerance=KEPSILON );

	bool IsCoPlanar( const TPlane<T> & OtherPlane ) const;

	bool PlaneIntersect( const TPlane<T>& oth, TVector3<T>& outpos, TVector3<T>& outdir );

	template< typename PolyType >
	bool ClipPoly( const PolyType& PolyToClip, PolyType& OutPoly );

	template< typename PolyType >
	bool ClipPoly( const PolyType& PolyToClip, PolyType& OutPolyFront, PolyType& OutPolyBack );

	void SimpleTransform(const TMatrix4<T>& transform);

	TPlane<T> operator-() const;

	//////////////////////////////////
	void EndianSwap();
	//////////////////////////////////

};

typedef TPlane<float> CPlane;

///////////////////////////////////////////////////////////////////////////////
} // namespace ork
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

#endif
