///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <ork/plane.h>

///////////////////////////////////////////////////////////////////////////////
namespace ork {
///////////////////////////////////////////////////////////////////////////////

class AABox
{
	CVector3	mMin;
	CVector3	mMax;

	CPlane		mPlaneNX[2];
	CPlane		mPlaneNY[2];
	CPlane		mPlaneNZ[2];

	void ComputePlanes();

public:

    void SupportMapping( const CVector3& v, CVector3& result) const;

	CVector3 Corner(int n) const;

	bool Intersect( const Ray3& ray, CVector3& isect_in, CVector3& isect_out ) const;
	bool Contains(const CVector3& test_point) const;
	bool Contains(const float test_point_X, const float test_point_Z) const;
    void Constrain(float &test_point_X, float &test_point_Z) const;
    void Constrain(CVector3& test_point) const;

    AABox();
    AABox( const CVector3& vmin, const CVector3& vmax );
    AABox( const AABox& oth );
    void operator=(const AABox& oth );
    
	inline const CVector3& Min() const { return mMin; }
	inline const CVector3& Max() const { return mMax; }
    inline CVector3 GetSize() const { return Max()-Min(); }

	void BeginGrow();
	void Grow( const CVector3& vin );
	void EndGrow();

	void SetMinMax( const CVector3& vmin, const CVector3& vmax );

};

///////////////////////////////////////////////////////////////////////////////
}
///////////////////////////////////////////////////////////////////////////////
