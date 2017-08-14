///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <ork/cvector3.h>
#include <ork/line.h>

///////////////////////////////////////////////////////////////////////////////
namespace ork {
///////////////////////////////////////////////////////////////////////////////

struct Sphere
{
	CVector3	mCenter;
	float		mRadius;

	Sphere( const CVector3& pos, float r ) : mCenter(pos), mRadius(r) {}
	Sphere( const CVector3& boxmin, const CVector3& boxmax );

    void SupportMapping( const CVector3& v, CVector3& result) const;

	bool Intersect( const Ray3& ray, CVector3& isect_in, CVector3& isect_out, CVector3& sphnormal ) const;


};

///////////////////////////////////////////////////////////////////////////////

inline void Sphere::SupportMapping( const CVector3& v, CVector3& result) const
{
}

inline bool Sphere::Intersect( const Ray3& ray, CVector3& isect_in, CVector3& isect_out, CVector3& isect_normal ) const
{
	CVector3 L = mCenter-ray.mOrigin;
	float tca = L.Dot(ray.mDirection);
	if( tca<0.0f ) return false;
	float d2 = L.Dot(L) - (tca*tca);
	float r2 = (mRadius*mRadius);
	if( d2>r2 ) return false;
	float thc = std::sqrt(r2-d2);
	isect_in = ray.mOrigin + ray.mDirection*(tca-thc);
	isect_out = ray.mOrigin + ray.mDirection*(tca+thc);
	isect_normal = (isect_in-mCenter).Normal();
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct Circle
{
	CVector2	mCenter;
	float		mRadius;

	Circle( const CVector2& pos, float r ) : mCenter(pos), mRadius(r) {}
};

//////////////////////////
// TRIANGLE INTERSECT

//  http://www.cs.princeton.edu/courses/archive/fall00/cs426/lectures/raycast/sld019.htm
//  http://www.cs.princeton.edu/courses/archive/fall00/cs426/lectures/raycast/sld021.htm

///////////////////////////////////////////////////////////////////////////////
}
///////////////////////////////////////////////////////////////////////////////
