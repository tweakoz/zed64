////////////////////////////////////////////////////////////////
// Orkid Media Engine
// Copyright 1996-2012, Michael T. Mayers.
// Distributed under the Boost Software License - Version 1.0 - August 17, 2003
// see http://www.boost.org/LICENSE_1_0.txt
//////////////////////////////////////////////////////////////// 
#if 0

#include <ork/pch.h>

#include <ork/math/misc_math.h>
#include <ork/math/cfixed.h>
#include <ork/math/cvector4.h>
#include <iostream>
#include <ork/reflect/Serialize.h>
#include <ork/reflect/BidirectionalSerializer.h>

///////////////////////////////////////////////////////////////////////////////
namespace ork {
///////////////////////////////////////////////////////////////////////////////

uint32_t PickIdToVertexColor( uint32_t pid )
{
	ork::CColor4 pickclr = ork::CColor4(pid);

	S32 a = U32(pickclr.GetX()*256.0f);
	S32 b = U32(pickclr.GetY()*256.0f);
	S32 g = U32(pickclr.GetZ()*256.0f);
	S32 r = U32(pickclr.GetW()*256.0f);

	if( r<0 ) r=0;
	if( g<0 ) g=0;
	if( b<0 ) b=0;
	if( a<0 ) a=0;

	uint32_t uobj = ( (r<<24)|(g<<16)|(b<<8)|a );

	return uobj;
}

/*template<> void reflect::Serialize( const CFloat*in, CFloat*out, reflect::BidirectionalSerializer& bidi )
{
	if( bidi.Serializing() )
	{
		bidi | in->FloatCast();
	}
	else
	{
		float val;
		bidi | val;
		*out = val;
	}
}*/

int* CPerlin2D::p = 0;
f32* CPerlin2D::g2 = 0;

///////////////////////////////////////////////////////////////////////////////

//Could add this function to the CVector4 class
XYZ RotatePointAboutLine(XYZ p, double theta, XYZ p1, XYZ p2)
{
   XYZ u,q1,q2;
   double d;

   /* Step 1 */
   q1.x = p.x - p1.x;
   q1.y = p.y - p1.y;
   q1.z = p.z - p1.z;

   u.x = p2.x - p1.x;
   u.y = p2.y - p1.y;
   u.z = p2.z - p1.z;
   Normalise(&u);
   d = std::sqrt(u.y*u.y + u.z*u.z);

   /* Step 2 */
   if (std::fabs(d) > 0.000001) {
      q2.x = q1.x;
      q2.y = q1.y * u.z / d - q1.z * u.y / d;
      q2.z = q1.y * u.y / d + q1.z * u.z / d;
   } else {
      q2 = q1;
   }

   /* Step 3 */
   q1.x = q2.x * d - q2.z * u.x;
   q1.y = q2.y;
   q1.z = q2.x * u.x + q2.z * d;

   /* Step 4 */
   q2.x = q1.x * std::cos(theta) - q1.y * std::sin(theta);
   q2.y = q1.x * std::sin(theta) + q1.y * std::cos(theta);
   q2.z = q1.z;

   /* Inverse of step 3 */
   q1.x =   q2.x * d + q2.z * u.x;
   q1.y =   q2.y;
   q1.z = - q2.x * u.x + q2.z * d;

   /* Inverse of step 2 */
   if (std::fabs(d) > 0.000001) {
      q2.x =   q1.x;
      q2.y =   q1.y * u.z / d + q1.z * u.y / d;
      q2.z = - q1.y * u.y / d + q1.z * u.z / d;
   } else {
      q2 = q1;
   }

   /* Inverse of step 1 */
   q1.x = q2.x + p1.x;
   q1.y = q2.y + p1.y;
   q1.z = q2.z + p1.z;
   return(q1);
}

void Normalise( XYZ *p )
{
	float ork = (float) std::sqrt( p->x * p->x + p->y * p->y + p->z * p->z );
	if( ork > 0.000001f )
	{
		p->x /= ork;
		p->y /= ork;
		p->z /= ork;
	}
	else
	{
		OrkAssert(false);
		/*
		if( p->x > p->y )
			if( p->x > p->z )
			{
				p->x = 1.0f; p->y = 0.0f; p->z = 0.0f;
			}
			else
			{
				p->x = 0.0f; p->y = 0.0f; p->z = 1.0f;
			}
		*/
	}
}

///////////////////////////////////////////////////////////////////////////////

CPolynomial CPolynomial::Differentiate() const
{
	CPolynomial result;

    result.SetCoefs(1, CReal(0));
    result.SetCoefs(2, coefs[0] * CReal(3));
    result.SetCoefs(3, coefs[1] * CReal(2));
    result.SetCoefs(4, coefs[2]);

    return result;
}

void CPolynomial::SetCoefs(const CReal *array)
{
	memcpy(coefs, array, 4 * sizeof(float));
}

void CPolynomial::SetCoefs(int i, CReal num)
{
	OrkAssert(i >= 1);
	OrkAssert(i <= 4);
	coefs[i-1] = num;
}

CReal CPolynomial::GetCoefs(int i) const
{
	OrkAssert(i >= 1);
	OrkAssert(i <= 4);
	return coefs[i-1];
}

CReal CPolynomial::Evaluate(CReal val) const
{
	return (((coefs[0]) * val + coefs[1]) * val + coefs[2]) * val + coefs[3];
}

CReal CPolynomial::operator()(CReal val) const
{
	return (((coefs[0]) * val + coefs[1]) * val + coefs[2]) * val + coefs[3];
}

CPolynomial CPolynomial::operator = ( const CPolynomial & a )
{
	SetCoefs(1, a.GetCoefs(1));
	SetCoefs(2, a.GetCoefs(2));
	SetCoefs(3, a.GetCoefs(3));
	SetCoefs(4, a.GetCoefs(4));
	return *this;
}

CPolynomial CPolynomial::operator + ( const CPolynomial & a )
{
	CPolynomial result;
	result.SetCoefs(1, a.GetCoefs(1) + coefs[0]);
	result.SetCoefs(1, a.GetCoefs(2) + coefs[1]);
	result.SetCoefs(1, a.GetCoefs(3) + coefs[2]);
	result.SetCoefs(1, a.GetCoefs(4) + coefs[3]);
	return result;

}

CPolynomial CPolynomial::operator - ( const CPolynomial & a )
{
	CPolynomial result;
	result.SetCoefs(1, a.GetCoefs(1) - GetCoefs(1));
	result.SetCoefs(1, a.GetCoefs(2) - GetCoefs(2));
	result.SetCoefs(1, a.GetCoefs(3) - GetCoefs(3));
	result.SetCoefs(1, a.GetCoefs(4) - GetCoefs(4));
	return result;
}

} // namespace ork
#endif
