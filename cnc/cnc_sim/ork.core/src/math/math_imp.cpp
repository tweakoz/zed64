///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#include <ork/cmatrix3.h>
#include <ork/cmatrix3.hpp>
#include <ork/cmatrix4.h>
#include <ork/cmatrix4.hpp>
#include <ork/matrix_inverseGEMS.hpp>
#include <ork/cvector2.h>
#include <ork/cvector2.hpp>
#include <ork/cvector3.h>
#include <ork/cvector3.hpp>
#include <ork/cvector4.h>
#include <ork/cvector4.hpp>
#include <ork/quaternion.h>
#include <ork/quaternion.hpp>
#include <ork/plane.h>
#include <ork/plane.hpp>
#include <ork/perlin_noise.h>
#include <stdlib.h>

namespace ork {
math_table_1d gsintab;
math_table_1d gcostab;

int* OldPerlin2D::p = 0;
float* OldPerlin2D::g2 = 0;


///////////////////////////////////////////////////////////////

float frand( float fscale, int irez )
{
    float fr = float(rand()%65536)/65536.0f;
    return fr*fscale;
}

///////////////////////////////////////////////////////////////

math_table_1d::math_table_1d()
	: miSize( 0 )
	, mfRange(0.01)
	, mfSizeInvRange(0.0f)
{
}

///////////////////////////////////////////////////////////////

void math_table_1d::fill_in(int isize, float rangeX, fn_t function )
{
	miSize = isize;
	mfRange = rangeX;
	mfSizeInvRange = (float(isize)/rangeX);

    mpTable = new float[ isize ];
    for( int i=0; i<isize; i++ )
    {
        float fx = float(i)/mfSizeInvRange;
        mpTable[i] = function(fx);
    }
}

///////////////////////////////////////////////////////////////

float math_table_1d::operator()(float fin) const
{
    float fidx = fin*mfSizeInvRange;
    return mpTable[ int(fidx)%miSize ];
}

///////////////////////////////////////////////////////////////

	bool UsingOpenGl()
	{
		return true;
	}
	///////////////////////////////////////////////////////////

	template<> float abs( float inp )
	{
		return ::fabsf(inp);
	}
	template<> float sin( float inp )
	{
		return ::sinf(inp);
	}
	template<> float cos( float inp )
	{
		return ::cosf(inp);
	}
	template<> float tan( float inp )
	{
		return ::tanf(inp);
	}
	template<> float sqrt( float inp )
	{
		return ::sqrtf(inp);
	}
	template<> float arccos( float inp )
	{
		return ::acosf(inp);
	}
	float float_epsilon()
	{
		return std::numeric_limits<float>::epsilon();
	}

	///////////////////////////////////////////////////////////

	template<> double sin( double inp )
	{
		return ::sin(inp);
	}
	template<> double cos( double inp )
	{
		return ::cos(inp);
	}
	template<> double tan( double inp )
	{
		return ::tan(inp);
	}
	template<> double abs( double inp )
	{
		return ::fabs(inp);
	}
	template<> double sqrt( double inp )
	{
		return ::sqrt(inp);
	}
	double double_epsilon()
	{
		return std::numeric_limits<double>::epsilon();
	}

	///////////////////////////////////////////////////////////

	template class TQuaternion<float>;

	template class TVector2<float>;
	template class TVector3<float>;
	template class TVector4<float>;

	template class TMatrix3<float>;
	template class TMatrix4<float>;

	template class TVector2<double>;
	template class TVector3<double>;
	template class TVector4<double>;

	template class TMatrix3<double>;
	template class TMatrix4<double>;

	template class ork::TPlane<float>;

};