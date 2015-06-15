///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#pragma once

//#include <ork/config/config.h>
#include <ork/plane.h>

///////////////////////////////////////////////////////////////////////////////
namespace ork {
///////////////////////////////////////////////////////////////////////////////

struct Frustum;
class AABox;
struct Sphere;
struct Circle;

class IAbstractCollidable
{
public:
	
	enum EColFlg
	{
		ECF_NOCOLLISION = 0,
		ECF_ENTERCOLLISION,
		ECF_EXITCOLLISION,
		ECF_TOTALCOLLISION
	};

	virtual EColFlg CollisionTest( const LineSegment3& seg, CVector3& cp, CVector3& vn ) = 0;
};

///////////////////////////////////////////////////////////////////////////////

struct  CollisionTester
{
	static bool FrustumSphereTest( const Frustum& frus, const Sphere& sph );
	static bool FrustumCircleXZTest( const Frustum& frus, const Circle& cir );
	static bool FrustumPointTest( const Frustum& frus, const CVector3& pnt );
	static bool FrustumAABoxTest( const Frustum& frus, const AABox& aab );
	static bool FrustumFrustumTest( const Frustum& frus1, const Frustum& frus2 );

	static bool SphereSphereTest( const Sphere& sph1, const Sphere& sph2 );
	static bool SphereAABoxTest( const Sphere& sph, const AABox& aab );

	static bool RayTriangleTest( const Ray3& ray, const CVector3& v0, const CVector3& v1, const CVector3& v2, CVector3& isect, float& s, float& t );
	static bool RayTriangleTest( const Ray3& ray, const CVector3& v0, const CVector3& v1, const CVector3& v2 );
    static bool RayTriangleTest(    const Ray3& ray, const CPlane& FacePlane,
                                    const CPlane& EdgePlane0, const CPlane& EdgePlane1, const CPlane& EdgePlane2,
                                    CVector3& isect );
    static bool RayTriangleTest(    const Ray3& ray, const CPlane& FacePlane,
                                    const CPlane& EdgePlane0, const CPlane& EdgePlane1, const CPlane& EdgePlane2,
                                    CVector3& isect, float &fdis );
    
	static bool RaySphereTest(const Ray3& ray, const Sphere& sph, float& t);

	static bool AbstractCollidableBisectionTest(	IAbstractCollidable& collidable,
													const float fs, const float fe,
													const LineSegment3& seg,
													CVector3& cp, CVector3& vn, float& fat );

};

///////////////////////////////////////////////////////////////////////////////
}
///////////////////////////////////////////////////////////////////////////////

