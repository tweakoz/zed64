///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#pragma once
#include <ork/line.h>
#include <ork/plane.h>

///////////////////////////////////////////////////////////////////////////////
namespace ork {
///////////////////////////////////////////////////////////////////////////////

struct Frustum
{
	typedef TRay3<float> ray_type;
	typedef TVector3<float> vec3_type;
	typedef TVector4<float> vec4_type;
	typedef TPlane<float> plane_type;
	typedef TMatrix4<float> mtx44_type;

	plane_type		mNearPlane;
	plane_type		mFarPlane;
	plane_type		mLeftPlane;
	plane_type		mRightPlane;
	plane_type		mTopPlane;
	plane_type		mBottomPlane;

	vec3_type	mNearCorners[4];
	vec3_type	mFarCorners[4];
	vec3_type	mCenter;
	vec3_type	mXNormal;
	vec3_type	mYNormal;
    vec3_type	mZNormal;

	void		SupportMapping(const vec3_type& v, vec3_type& result ) const;

	void		Set( const mtx44_type& VMatrix, const mtx44_type& PMatrix );
	void		Set( const mtx44_type& IVPMatrix );

	void		CalcCorners();

	bool		Contains(const vec3_type& v) const;

};

///////////////////////////////////////////////////////////////////////////////
}
///////////////////////////////////////////////////////////////////////////////
