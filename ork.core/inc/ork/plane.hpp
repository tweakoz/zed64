///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#include <cmath>

///////////////////////////////////////////////////////////////////////////////
namespace ork
{

template< typename T> bool TPlane<T>::IsCoPlanar( const TPlane<T> & OtherPlane ) const
{
	T fdot = Abs( n.Dot( OtherPlane.n ) - T(1.0f) );
	T fDelD = Abs( d-OtherPlane.d );
	return ( (fdot<T(0.01f)) && (fDelD<T(0.01f)) );
}

///////////////////////////////////////////////////////////////////////////////

template<typename T> TPlane<T>::TPlane()
	: n()
	, d( T(0) )
{
}

///////////////////////////////////////////////////////////////////////////////

template< typename T> TPlane<T>::TPlane( const TVector4<T> &vec )
	: n( vec.GetXYZ() )
	, d( vec.GetW() )
{
}

///////////////////////////////////////////////////////////////////////////////

template< typename T> TPlane<T>::TPlane( const TVector3<T> &vec, T nd )
	: n( vec )
    , d( nd )
{
}

///////////////////////////////////////////////////////////////////////////////

template< typename T> TPlane<T>::TPlane( T nx, T ny, T nz, T dist )
	: n( nx, ny, nz )
	, d( dist )
{

}

///////////////////////////////////////////////////////////////////////////////

template< typename T> TPlane<T>::TPlane( T *Tp ) //! set explicitly
{
    n = TVector3<T>( Tp[0], Tp[1], Tp[2] );
    d = Tp[3];
}

///////////////////////////////////////////////////////////////////////////////

template< typename T> TPlane<T>::TPlane( const TVector3<T> &NormalVec, const TVector3<T> &PosVec ) //! calc given normal and position of plane origin
{
    CalcFromNormalAndOrigin( NormalVec, PosVec );
}

///////////////////////////////////////////////////////////////////////////////

template< typename T> TPlane<T>::~TPlane()
{
}

///////////////////////////////////////////////////////////////////////////////

template< typename T> void TPlane<T>::CalcFromNormalAndOrigin( const TVector3<T> &NormalVec, const TVector3<T> &PosVec ) //! calc given normal and position of plane origin
{
    n = NormalVec;
    d = T(0.0f);
    d =  GetPointDistance( PosVec ) * T(-1.0f);
}

///////////////////////////////////////////////////////////////////////////////

template< typename T> void TPlane<T>::Reset(void)
{
    n = TVector3<T>( T(0.0f), T(0.0f), T(0.0f) );
    d = T(0.0f);
}

///////////////////////////////////////////////////////////////////////////////

template< typename T> bool TPlane<T>::IsPointInFront( const TVector3<T> &point ) const
{
	T distance = GetPointDistance(point);
    return (distance >= T(0.0f));
}

///////////////////////////////////////////////////////////////////////////////

template< typename T> bool TPlane<T>::IsPointBehind( const TVector3<T> &point ) const
{
	return (!IsPointInFront(point));
}

///////////////////////////////////////////////////////////////////////////////

template< typename T> void TPlane<T>::CalcD( const TVector3<T> &pt )
{
    d = - pt.Dot( n );
}

///////////////////////////////////////////////////////////////////////////////

template< typename T> T TPlane<T>::GetPointDistance( const TVector3<T> &pt ) const
{
    return n.Dot(pt) + d;
}

///////////////////////////////////////////////////////////////////////////////

template< typename T> const TVector3<T> &TPlane<T>::GetNormal(void) const
{
    return n;
}

///////////////////////////////////////////////////////////////////////////////

template< typename T> const T &TPlane<T>::GetD(void) const
{
    return d;
}

///////////////////////////////////////////////////////////////////////////////

template< typename T> void TPlane<T>::crossProduct( F64 ii1, F64 jj1, F64 kk1, F64 ii2, F64 jj2, F64 kk2, F64 &iicp, F64 &jjcp, F64 &kkcp) const
{	iicp= (jj1*kk2) - (jj2*kk1);
	jjcp= (ii2*kk1) - (ii1*kk2);
	kkcp= (ii1*jj2) - (ii2*jj1);
}
	
///////////////////////////////////////////////////////////////////////////////

template< typename T> static bool AreValuesCloseEnoughtToBeEqual( f64 a,f64 b, f64 ftolerance )
{
	return (std::fabs(a-b) >= ftolerance) ? false : true;
}

///////////////////////////////////////////////////////////////////////////////
// ftolerance = smallest distance to consider a point colinear

template <typename T> void TPlane<T>::CalcPlaneFromTriangle( const TVector3<T> & p0, const TVector3<T> & p1, const TVector3<T> & p2, F64 ftolerance )
{
    F64 ii1,jj1,kk1;
    F64 ii2,jj2,kk2;
    F64 iicp, jjcp, kkcp;
    F64 len0,len1,len2;

	TVector4<T> p1mp0 = p1-p0;
	TVector4<T> p2mp1 = p2-p1;
	TVector4<T> p0mp2 = p0-p2;

    len0=p1mp0.Mag();
    len1=p2mp1.Mag();
    len2=p0mp2.Mag();

    if((len0>=len1) && (len0>=len2))
    {
        ii1= (p0.GetX() - p2.GetX());
        ii2= (p1.GetX() - p2.GetX());
        jj1= (p0.GetY() - p2.GetY());
        jj2= (p1.GetY() - p2.GetY());
        kk1= (p0.GetZ() - p2.GetZ());
        kk2= (p1.GetZ() - p2.GetZ());
    }
    else if((len1>=len0) && (len1>=len2))
    {
        ii1= (p1.GetX() - p0.GetX());
        ii2= (p2.GetX() - p0.GetX());
        jj1= (p1.GetY() - p0.GetY());
        jj2= (p2.GetY() - p0.GetY());
        kk1= (p1.GetZ() - p0.GetZ());
        kk2= (p2.GetZ() - p0.GetZ());
    }
    else
    {
        ii1= (p2.GetX() - p1.GetX());
        ii2= (p0.GetX() - p1.GetX());
        jj1= (p2.GetY() - p1.GetY());
        jj2= (p0.GetY() - p1.GetY());
        kk1= (p2.GetZ() - p1.GetZ());
        kk2= (p0.GetZ() - p1.GetZ());
    }

    crossProduct(ii2,jj2,kk2,ii1,jj1,kk1,iicp,jjcp,kkcp);

    //   assert(!(IS_EQ(iicp,0.0) && IS_EQ(jjcp,0.0) && IS_EQ(kkcp,0.0)));
    if(		AreValuesCloseEnoughtToBeEqual<T>(iicp,0.0,ftolerance)
		&&	AreValuesCloseEnoughtToBeEqual<T>(jjcp,0.0,ftolerance)
		&&	AreValuesCloseEnoughtToBeEqual<T>(kkcp,0.0,ftolerance)
	  )
    {
		//orkprintf( "Whoops, 3 colinear points in a quad.\n");
        return;
    }
	//if(IS_EQ_TIGHT(plane->aa,0)&&IS_EQ_TIGHT(plane->bb,0)&&IS_EQ_TIGHT(plane->cc,0))
	//{	//messageh( DEBUG_PANE, "Plane error\n");
	//}

    // get plane normal

	n.SetXYZ( (T) iicp, (T) jjcp, (T) kkcp );
	n.Normalize();
    
	// get plane distance from origin
	d = T(0.0f);
    d =  GetPointDistance( p0 ) * T(-1.0f);




}

///////////////////////////////////////////////////////////////////////////////

template< typename T> bool TPlane<T>::IsOn( const TVector3<T> &pt ) const
{
    T d = GetPointDistance(pt);
    return (Abs(d) < Epsilon()) ? true : false;
}

///////////////////////////////////////////////////////////////////////////////

template< typename T> void TPlane<T>::CalcNormal( const TVector3<T> &pta, const TVector3<T> &ptb, const TVector3<T> &ptc)
{
    TVector3<T> bminusa = (ptb - pta);
    TVector3<T> cminusa = (ptc - pta);

    n = bminusa.Cross( cminusa );
    n.Normalize();
    d = -ptc.Dot(n);
}

///////////////////////////////////////////////////////////////////////////////

template< typename T> bool TPlane<T>::Intersect(  const TLineSegment3<T>& lseg, T& dis, TVector3<T> &result ) const
{
	TVector3<T> dif = (lseg.mEnd-lseg.mStart);
	T length = dif.Mag();

	TRay3<T> ray;
	ray.mDirection = dif*(T(1.0f)/length); // cheaper normalize since we need the length anyway
	ray.mOrigin = lseg.mStart;

	bool bOK = Intersect( ray, dis, result ); 

	bOK &= ( (dis>=0.0f) && (dis<length) );

	return bOK;
}

///////////////////////////////////////////////////////////////////////////////

template< typename T> bool TPlane<T>::Intersect( const TRay3<T>& ray, T& dis, TVector3<T> &res ) const
{
    bool bOK = Intersect( ray, dis ); 

	if( bOK )
	{
		res = ray.mOrigin + (ray.mDirection*dis);
	}
	return bOK;
}

///////////////////////////////////////////////////////////////////////////////

template< typename T> bool TPlane<T>::Intersect( const TRay3<T>& ray, T &dis ) const
{
    T denom = n.Dot(ray.mDirection);
    // Line is parallel to the plane or plane normal faces away from ray
	if( Abs(denom) < Epsilon())
        return false;

	T pointdist = GetPointDistance(ray.mOrigin);
	T u = -pointdist/(denom);
	
	dis = u;

	return true;
}

///////////////////////////////////////////////////////////////////////////////

template< typename T>
template< typename PolyType >
bool TPlane<T>::ClipPoly( const PolyType& PolyToClip, PolyType& OutPoly )
{
	const int inuminverts = (int) PolyToClip.GetNumVertices();
	if( inuminverts )
	{
		const typename PolyType::VertexType& StartVtx = PolyToClip.GetVertex(0);
		bool IsVtxAIn = IsPointInFront( StartVtx.Pos() );
		//get the side of each vert to the plane
		for(int iva=0;iva<inuminverts;iva++)
		{
			int ivb = ((iva == inuminverts-1) ? 0 : iva+1);
			const typename PolyType::VertexType& vA = PolyToClip.GetVertex(iva);
			const typename PolyType::VertexType& vB = PolyToClip.GetVertex(ivb);
			if(IsVtxAIn)
			{
				OutPoly.AddVertex( vA );
			}
			bool IsVtxBIn  = IsPointInFront( vB.Pos() ); 
			if(IsVtxBIn != IsVtxAIn)
			{
				CVector3 vPos;
				T isectdist;
				LineSegment3 lseg( vA.Pos(), vB.Pos() );
				if( Intersect( lseg, isectdist, vPos ) )
				{
					T fDist = (vA.Pos()-vB.Pos()).Mag();
					T fDist2 = (vA.Pos()-vPos).Mag();
					T fScalar = (Abs(fDist)<Epsilon()) ? 0.0f : fDist2 / fDist;
					typename PolyType::VertexType LerpedVertex;
					LerpedVertex.Lerp( vA, vB, fScalar );
					OutPoly.AddVertex( LerpedVertex );
				}
			}
			IsVtxAIn = IsVtxBIn;
		}
	}
	return (OutPoly.GetNumVertices() >= 3);
}

///////////////////////////////////////////////////////////////////////////////

template< typename T>
template< typename PolyType >
bool TPlane<T>::ClipPoly( const PolyType& PolyToClip, PolyType& OutPolyFront, PolyType& OutPolyBack )
{
	const int inuminverts = PolyToClip.GetNumVertices();
	const typename PolyType::VertexType& StartVtx = PolyToClip.GetVertex(0);
	bool IsVtxAIn = IsPointInFront( StartVtx.Pos() );
	//get the side of each vert to the plane
	for(int iva=0;iva<inuminverts;iva++)
	{
		int ivb = ((iva == inuminverts-1) ? 0 : iva+1);
		const typename PolyType::VertexType& vA = PolyToClip.GetVertex(iva);
		const typename PolyType::VertexType& vB = PolyToClip.GetVertex(ivb);
		if(IsVtxAIn)
		{
			OutPolyFront.AddVertex( vA );
		}
		else
		{
			OutPolyBack.AddVertex( vA );
		}
		bool IsVtxBIn  = IsPointInFront( vB.Pos() ); 
		if(IsVtxBIn != IsVtxAIn)
		{
			TVector3<T> vPos;
			T isectdist;
			TLineSegment3<T> lseg( vA.Pos(), vB.Pos() );
			if( Intersect( lseg, isectdist, vPos ) )
			{
				T fDist = (vA.Pos()-vB.Pos()).Mag();
				T fDist2 = (vA.Pos()-vPos).Mag();
				T fScalar = (Abs(fDist)<Epsilon()) ? T(0.0) : fDist2 / fDist;
				typename PolyType::VertexType LerpedVertex;
				LerpedVertex.Lerp( vA, vB, fScalar );
				OutPolyFront.AddVertex( LerpedVertex );
				OutPolyBack.AddVertex( LerpedVertex );
			}
		}
		IsVtxAIn = IsVtxBIn;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
/*
template< typename T> void TPlane<T>::EndianSwap()
{
	swapbytes_dynamic( n[0] );
	swapbytes_dynamic( n[1] );
	swapbytes_dynamic( n[2] );
	swapbytes_dynamic( d );

}*/

///////////////////////////////////////////////////////////////////////////////

template< typename T> void TPlane<T>::SimpleTransform(const TMatrix4<T>& transform)
{
	TVector3<T> point(n * -d);
	point = point.Transform(transform).GetXYZ();
	n = n.Transform3x3(transform);
	n.Normalize();
	d = -n.Dot(point);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T> TPlane<T> TPlane<T>::operator-() const
{
	return TPlane<T>(-n, n * -d);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T> T TPlane<T>::Abs(T in)
{
	return ork::abs<T>(in);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T> T TPlane<T>::Epsilon()
{
	return ork::KEPSILON;
}

///////////////////////////////////////////////////////////////////////////////

template< typename T>
bool TPlane<T>::PlaneIntersect( const TPlane<T>& oth, TVector3<T>& outpos, TVector3<T>& outdir )
{
    outdir = GetNormal().Cross( oth.GetNormal() );
    T num = outdir.MagSquared();
        TVector3<T> c1 = (GetD()*oth.GetNormal()) + (oth.GetD()*GetNormal());
        outpos = c1.Cross( outdir ) * T(1.0)/num;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace ork
