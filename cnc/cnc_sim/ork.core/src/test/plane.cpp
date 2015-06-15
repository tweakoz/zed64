///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <unittest++/UnitTest++.h>
#include <cmath>
#include <ork/cvector4.h>
#include <ork/plane.h>
#include <ork/plane.hpp>
#include <ork/line.h>
#include <vector>
//#include <ork/math/sphere.h>
//#include <ork/math/plane.hpp>
//#include <ork/math/collision_test.h>

///////////////////////////////////////////////////////////////////////////////
using namespace ork;
///////////////////////////////////////////////////////////////////////////////

TEST(PlanePlaneIntersection)
{
	CPlane xzplane( 0.0f, 1.0f, 0.0f,   0.0f );
	CPlane xyplane( 0.0f, 0.0f, 1.0f,   0.0f );
	CPlane yzplane( 1.0f, 0.0f, 0.0f,   0.0f );

	CVector3 vpos_xz_isect_xy;
	CVector3 vpos_xy_isect_xz;
	CVector3 vpos_yz_isect_xy;
	CVector3 vpos_xy_isect_yz;

	CVector3 vdir_xz_isect_xy;
	CVector3 vdir_xy_isect_xz;
	CVector3 vdir_yz_isect_xy;
	CVector3 vdir_xy_isect_yz;

	////////////////////////////////////////////////
	// Check that the planes intersect
	////////////////////////////////////////////////

	bool xz_isect_xy = xzplane.PlaneIntersect( xyplane, vpos_xz_isect_xy, vdir_xz_isect_xy );
	bool xy_isect_xz = xyplane.PlaneIntersect( xzplane, vpos_xz_isect_xy, vdir_xy_isect_xz );
	bool xy_isect_yz = xyplane.PlaneIntersect( yzplane, vpos_xy_isect_yz, vdir_xy_isect_yz );
	bool yz_isect_xy = yzplane.PlaneIntersect( xyplane, vpos_yz_isect_xy, vdir_yz_isect_xy );

	CHECK( xz_isect_xy );
	CHECK( xy_isect_xz );
	CHECK( xy_isect_yz );
	CHECK( yz_isect_xy );

	////////////////////////////////////////////////
	// Check that the planes intersect at lines about the origin
	////////////////////////////////////////////////

	float fdistfromorigin_xzxy =  (vpos_xz_isect_xy-CVector3(0.0f,0.0f,0.0f)).Mag();
	float fdistfromorigin_xyxz =  (vpos_xy_isect_xz-CVector3(0.0f,0.0f,0.0f)).Mag();
	float fdistfromorigin_yzxy =  (vpos_yz_isect_xy-CVector3(0.0f,0.0f,0.0f)).Mag();
	float fdistfromorigin_xyyz =  (vpos_xy_isect_yz-CVector3(0.0f,0.0f,0.0f)).Mag();
	const float dist_epsilon = 0.0001f;

	CHECK( fdistfromorigin_xzxy<dist_epsilon );
	CHECK( fdistfromorigin_xyxz<dist_epsilon );
	CHECK( fdistfromorigin_yzxy<dist_epsilon );
	CHECK( fdistfromorigin_xyyz<dist_epsilon );

	////////////////////////////////////////////////
	// Check that the planes intersection line is ON both planes
	//  for any line on a plane     (line.dirnormal).dot(plane.normal) == 0.0f
	////////////////////////////////////////////////

	const float dot_epsilon = 0.0001f;

	float fdot_xzxy_xz = vdir_xz_isect_xy.Dot( xzplane.GetNormal() );
	float fdot_xzxy_xy = vdir_xz_isect_xy.Dot( xyplane.GetNormal() );
	float fdot_xyxz_xy = vdir_xy_isect_xz.Dot( xzplane.GetNormal() );
	float fdot_xyxz_xz = vdir_xy_isect_xz.Dot( xyplane.GetNormal() );

	CHECK( ork::abs(fdot_xzxy_xz) < dot_epsilon );
	CHECK( ork::abs(fdot_xzxy_xy) < dot_epsilon );
	CHECK( ork::abs(fdot_xyxz_xy) < dot_epsilon );
	CHECK( ork::abs(fdot_xyxz_xz) < dot_epsilon );

	float fdot_yzxy_yz = vdir_yz_isect_xy.Dot( yzplane.GetNormal() );
	float fdot_yzxy_xy = vdir_yz_isect_xy.Dot( xyplane.GetNormal() );
	float fdot_xyyz_xy = vdir_xy_isect_yz.Dot( xyplane.GetNormal() );
	float fdot_xyyz_yz = vdir_xy_isect_yz.Dot( yzplane.GetNormal() );

	CHECK( ork::abs(fdot_yzxy_yz) < dot_epsilon );
	CHECK( ork::abs(fdot_yzxy_xy) < dot_epsilon );
	CHECK( ork::abs(fdot_xyyz_xy) < dot_epsilon );
	CHECK( ork::abs(fdot_xyyz_yz) < dot_epsilon );

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TEST(PlaneRayIntersection)
{
	CPlane xzplane( 0.0f, 1.0f, 0.0f,   0.0f );

	/////////////////////////////////////////////////
	// check ray facing plane normal (intersects with a positive distance)

	Ray3 ray;

	ray.mOrigin = CVector3( 0.0f, 10.0f, 0.0f );
	ray.mDirection = CVector3( 0.0f, -1.0f, 0.0f );
	CVector3 ray0_isect;
	float planedist;
	bool ray0_doesintersect = xzplane.Intersect( ray, planedist, ray0_isect );

	CHECK( ray0_doesintersect );
	CHECK( ork::abs(planedist-10.0f) < ork::float_epsilon() );

	/////////////////////////////////////////////////
	// check ray facing away from plane normal (intersects with a negative distance)

	ray.mOrigin = CVector3( 0.0f, 10.0f, 0.0f );
	ray.mDirection = CVector3( 0.0f, 1.0f, 0.0f );
	CVector3 ray1_isect;
	bool ray1_doesintersect = xzplane.Intersect( ray, planedist, ray1_isect );

	CHECK( ray1_doesintersect );
	CHECK( ork::abs(planedist+10.0f) < ork::float_epsilon() );

	/////////////////////////////////////////////////

	ray.mOrigin = CVector3( 0.0f, 10.0f, 0.0f );
	ray.mDirection = CVector3( 0.0f, -1.0f, -1.0f ).Normal();
	CVector3 ray2_isect;
	bool ray2_doesintersect = xzplane.Intersect( ray, planedist, ray2_isect );

	CHECK( ray2_doesintersect );
	CHECK( ork::abs(planedist-(10.0f*ork::sqrt<float>(2.0f))) < ork::float_epsilon() );

	/////////////////////////////////////////////////

	ray.mOrigin = CVector3( 0.0f, 10.0f, 0.0f );
	ray.mDirection = CVector3( -1.0f, -1.0f, -1.0f ).Normal();
	CVector3 ray3_isect;
	bool ray3_doesintersect = xzplane.Intersect( ray, planedist, ray3_isect );

	CHECK( ray3_doesintersect );
	CHECK( ork::abs(planedist-(10.0f*ork::sqrt<float>(3.0f))) < 1.0e-05f );

	/////////////////////////////////////////////////
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TEST(PlaneLineSegIntersection)
{
	CPlane xyplane( 0.0f, 0.0f,	1.0f,   0.0f );

	LineSegment3 lseg( CVector3( 0.0f, 0.0f, 10.0f ), CVector3( 0.0f, 0.0f, -20.0f ) );

	/////////////////////////////////////////////////
	// check ray facing plane normal (intersects with a positive distance)

	//lseg.mStart = CVector3( 0.0f, 0.0f, 10.0f );
	//lseg.mEnd = CVector3( 0.0f, 0.0f, -20.0f );
	CVector3 seg0_isect;
	float planedist;
	bool seg0_doesintersect = xyplane.Intersect( lseg, planedist, seg0_isect );
	
	CHECK( seg0_doesintersect );
	CHECK( ork::abs(planedist-10.0f) < ork::float_epsilon() );

	/////////////////////////////////////////////////
	// check ray facing plane normal (intersects with a positive distance)

	lseg.mStart = CVector3( 0.0f, 0.0f, -20.0f );
	lseg.mEnd = CVector3( 0.0f, 0.0f, 10.0f );
	CVector3 seg1_isect;
	bool seg1_doesintersect = xyplane.Intersect( lseg, planedist, seg1_isect );
	
	CHECK( seg1_doesintersect );
	CHECK( ork::abs(planedist-20.0f) < ork::float_epsilon() );

	/////////////////////////////////////////////////
	// check ray facing plane normal (intersects with a positive distance)

	lseg.mStart = CVector3( 0.0f, 0.0f, -20.0f );
	lseg.mEnd = CVector3( 0.0f, 0.0f, -5.0f );
	CVector3 seg2_isect;
	bool seg2_doesintersect = xyplane.Intersect( lseg, planedist, seg2_isect );
	
	CHECK( false == seg2_doesintersect );

	/////////////////////////////////////////////////
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct tvert
{
	CVector3 pos;
	CVector2 uv;

	tvert() {}

	tvert( float fx, float fy, float fz, float fu, float fv )
		: pos(fx,fy,fz)
		, uv( fu, fv )
	{
	}

	void Lerp( const tvert& va, const tvert& vb, float flerp )
	{
		pos.Lerp( va.pos, vb.pos, flerp );
		uv.Lerp( va.uv, vb.uv, flerp );
	}

	const CVector3& Pos() const { return pos; }


};

class tpoly
{
	std::vector<tvert>	mverts;

public:

	typedef tvert VertexType;

	void AddVertex( const tvert& v ) { mverts.push_back(v); }

	int GetNumVertices() const { return mverts.size(); }
	const tvert& GetVertex( int idx ) const { return mverts[idx]; }

};


TEST(PolyClipper)
{
	CPlane xzplane( 0.0f, 1.0f,	0.0f,   0.0f );

	tvert v0( 0.0f, 1.0f, 0.0f,       0.0f, 0.0f );
	tvert v1( 1.0f, -2.0f, 0.0f,      1.0f, 0.0f );
	tvert v2( -1.0f, -2.0f, 0.0f,    -1.0f, 0.0f );
	
	tpoly InPoly;
	InPoly.AddVertex( v0 );
	InPoly.AddVertex( v1 );
	InPoly.AddVertex( v2 );
	
	tpoly ClippedPoly;

	bool bOK = xzplane.ClipPoly( InPoly, ClippedPoly );

	CHECK( bOK );
	CHECK( ClippedPoly.GetNumVertices() == 3 );


	tpoly ClippedFrontPoly, ClippedBackPoly;

	bOK = xzplane.ClipPoly( InPoly, ClippedFrontPoly, ClippedBackPoly );

	CHECK( bOK );
	CHECK( ClippedFrontPoly.GetNumVertices() == 3 );
	CHECK( ClippedBackPoly.GetNumVertices() == 4 );

}


template bool ork::CPlane::ClipPoly<tpoly>(const tpoly& in, tpoly& out );
template bool ork::CPlane::ClipPoly<tpoly>(const tpoly& in, tpoly& outfront, tpoly& outback );
