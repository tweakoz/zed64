#include <unittest++/UnitTest++.h>
#include <cmath>
#include <limits>
#include <ork/cvector3.h>
#include <ork/cvector4.h>
#include <ork/cmatrix4.h>
#include <ork/math_misc.h>


using namespace ork;
static const float KEPSILON = 5.0e-07f;//std::numeric_limits<float>::epsilon();

TEST(CVector3DefaultConstructor)
{
	CVector3 v;
	CHECK_CLOSE(0.0f, v.GetX(), KEPSILON);
	CHECK_CLOSE(0.0f, v.GetY(), KEPSILON);
	CHECK_CLOSE(0.0f, v.GetZ(), KEPSILON);
}

TEST(CVector3Constructor)
{
	CVector3 v(1.0f, 2.0f, 3.0f);
	CHECK_CLOSE(1.0f, v.GetX(), KEPSILON);
	CHECK_CLOSE(2.0f, v.GetY(), KEPSILON);
	CHECK_CLOSE(3.0f, v.GetZ(), KEPSILON);
}

TEST(CVector3CopyConstructor)
{
	CVector3 v(1.0f, 2.0f, 3.0f);
	CVector3 copy(v);
	CHECK_CLOSE(1.0f, copy.GetX(), KEPSILON);
	CHECK_CLOSE(2.0f, copy.GetY(), KEPSILON);
	CHECK_CLOSE(3.0f, copy.GetZ(), KEPSILON);
}

TEST(CVector3RGBAConstructor)
{
	CVector3 v(0xFF00FFFF);
	CHECK_CLOSE(1.0f, v.GetX(), KEPSILON);
	CHECK_CLOSE(0.0f, v.GetY(), KEPSILON);
	CHECK_CLOSE(1.0f, v.GetZ(), KEPSILON);
}

TEST(CVector3RotateX)
{
	CVector3 v(0.0f, 1.0f, 0.0f);
	v.RotateX(PI / 2.0f);
	CHECK_CLOSE(0.0f, v.GetX(), KEPSILON);
	CHECK_CLOSE(0.0f, v.GetY(), KEPSILON);
	CHECK_CLOSE(1.0f, v.GetZ(), KEPSILON);
}

TEST(CVector3RotateY)
{
	CVector3 v(1.0f, 0.0f, 0.0f);
	v.RotateY(PI / float(2.0f));
	CHECK_CLOSE(0.0f, v.GetX(), KEPSILON);
	CHECK_CLOSE(0.0f, v.GetY(), KEPSILON);
	CHECK_CLOSE(1.0f, v.GetZ(), KEPSILON);
}

TEST(CVector3RotateZ)
{
	CVector3 v(1.0f, 0.0f, 0.0f);
	v.RotateZ(PI / float(2.0f));
	CHECK_CLOSE(0.0f, v.GetX(), KEPSILON);
	CHECK_CLOSE(1.0f, v.GetY(), KEPSILON);
	CHECK_CLOSE(0.0f, v.GetZ(), KEPSILON);
}

TEST(CVector3Add)
{
	CVector3 v(1.0f, 0.0f, 0.0f);
	CVector3 res = v + CVector3(0.0f, 1.0f, 0.0f);
	CHECK_CLOSE(1.0f, res.GetX(), KEPSILON);
	CHECK_CLOSE(1.0f, res.GetY(), KEPSILON);
	CHECK_CLOSE(0.0f, res.GetZ(), KEPSILON);
}

TEST(CVector3AddTo)
{
	CVector3 v(1.0f, 0.0f, 0.0f);
	v += CVector3(0.0f, 1.0f, 0.0f);
	CHECK_CLOSE(1.0f, v.GetX(), KEPSILON);
	CHECK_CLOSE(1.0f, v.GetY(), KEPSILON);
	CHECK_CLOSE(0.0f, v.GetZ(), KEPSILON);
}

TEST(CVector3Subtract)
{
	CVector3 v(1.0f, 0.0f, 0.0f);
	CVector3 res = v - CVector3(0.0f, 1.0f, 0.0f);
	CHECK_CLOSE(1.0f, res.GetX(), KEPSILON);
	CHECK_CLOSE(-1.0f, res.GetY(), KEPSILON);
	CHECK_CLOSE(0.0f, res.GetZ(), KEPSILON);
}

TEST(CVector3SubtractFrom)
{
	CVector3 v(1.0f, 0.0f, 0.0f);
	v -= CVector3(0.0f, 1.0f, 0.0f);
	CHECK_CLOSE(1.0f, v.GetX(), KEPSILON);
	CHECK_CLOSE(-1.0f, v.GetY(), KEPSILON);
	CHECK_CLOSE(0.0f, v.GetZ(), KEPSILON);
}

TEST(CVector3Multiply)
{
	CVector3 v(1.0f, 2.0f, 3.0f);
	CVector3 res = v * CVector3(3.0f, 2.0f, 1.0f);
	CHECK_CLOSE(3.0f, res.GetX(), KEPSILON);
	CHECK_CLOSE(4.0f, res.GetY(), KEPSILON);
	CHECK_CLOSE(3.0f, res.GetZ(), KEPSILON);
}

TEST(CVector3MultiplyTo)
{
	CVector3 v(1.0f, 2.0f, 3.0f);
	v *= CVector3(3.0f, 2.0f, 1.0f);
	CHECK_CLOSE(3.0f, v.GetX(), KEPSILON);
	CHECK_CLOSE(4.0f, v.GetY(), KEPSILON);
	CHECK_CLOSE(3.0f, v.GetZ(), KEPSILON);
}

TEST(CVector3Divide)
{
	CVector3 v(1.0f, 2.0f, 3.0f);
	CVector3 res = v / CVector3(3.0f, 2.0f, 1.0f);
	CHECK_CLOSE(.333333f, res.GetX(), KEPSILON);
	CHECK_CLOSE(1.0f, res.GetY(), KEPSILON);
	CHECK_CLOSE(3.0f, res.GetZ(), KEPSILON);
}

TEST(CVector3DivideTo)
{
	CVector3 v(1.0f, 2.0f, 3.0f);
	v /= CVector3(3.0f, 2.0f, 1.0f);
	CHECK_CLOSE(0.333333f, v.GetX(), KEPSILON);
	CHECK_CLOSE(1.0f, v.GetY(), KEPSILON);
	CHECK_CLOSE(3.0f, v.GetZ(), KEPSILON);
}

TEST(CVector3Scale)
{
	CVector3 v(1.0f, 2.0f, 3.0f);
	CVector3 res = v * float(3.0f);
	CHECK_CLOSE(3.0f, res.GetX(), KEPSILON);
	CHECK_CLOSE(6.0f, res.GetY(), KEPSILON);
	CHECK_CLOSE(9.0f, res.GetZ(), KEPSILON);
}

TEST(CVector3ScaleTo)
{
	CVector3 v(1.0f, 2.0f, 3.0f);
	v *= float(3.0f);
	CHECK_CLOSE(3.0f, v.GetX(), KEPSILON);
	CHECK_CLOSE(6.0f, v.GetY(), KEPSILON);
	CHECK_CLOSE(9.0f, v.GetZ(), KEPSILON);
}

TEST(CVector3ScalePre)
{
	CVector3 v(1.0f, 2.0f, 3.0f);
	CVector3 res = float(3.0f) * v;
	CHECK_CLOSE(3.0f, res.GetX(), KEPSILON);
	CHECK_CLOSE(6.0f, res.GetY(), KEPSILON);
	CHECK_CLOSE(9.0f, res.GetZ(), KEPSILON);
}

TEST(CVector3InvScale)
{
	CVector3 v(1.0f, 2.0f, 3.0f);
	CVector3 res = v / float(3.0f);
	CHECK_CLOSE(0.333333f, res.GetX(), KEPSILON);
	CHECK_CLOSE(0.666667f, res.GetY(), KEPSILON);
	CHECK_CLOSE(1.0f, res.GetZ(), KEPSILON);
}

TEST(CVector3InvScaleTo)
{
	CVector3 v(1.0f, 2.0f, 3.0f);
	v /= float(3.0f);
	CHECK_CLOSE(0.333333f, v.GetX(), KEPSILON);
	CHECK_CLOSE(0.666667f, v.GetY(), KEPSILON);
	CHECK_CLOSE(1.0f, v.GetZ(), KEPSILON);
}

TEST(CVector3EqualCompare)
{
	CVector3 v1(1.0f, 2.0f, 3.0f);
	CVector3 v2(1.0f, 2.0f, 3.0f);
	CVector3 v3(4.0f, 3.0f, 2.0f);
	CHECK_EQUAL(true, v1 == v2);
	CHECK_EQUAL(false, v1 == v3);
}

TEST(CVector3NotEqualCompare)
{
	CVector3 v1(1.0f, 2.0f, 3.0f);
	CVector3 v2(1.0f, 2.0f, 3.0f);
	CVector3 v3(4.0f, 3.0f, 2.0f);
	CHECK_EQUAL(false, v1 != v2);
	CHECK_EQUAL(true, v1 != v3);
}

TEST(CVector3Dot)
{
	CVector3 v1(1.0f, 2.0f, 3.0f);
	CVector3 v2(4.0f, 3.0f, 2.0f);
	float res = v1.Dot(v2);
	CHECK_CLOSE(16.0f, res, KEPSILON);
}

TEST(CVector3Cross)
{
	CVector3 v1(1.0f, 2.0f, 3.0f);
	CVector3 v2(4.0f, 3.0f, 2.0f);
	CVector3 res = v1.Cross(v2);
	CHECK_CLOSE(-5.0f, res.GetX(), KEPSILON);
	CHECK_CLOSE(10.0f, res.GetY(), KEPSILON);
	CHECK_CLOSE(-5.0f, res.GetZ(), KEPSILON);
}

TEST(CVector3Normalize)
{
	CVector3 v(1.0f, 2.0f, 3.0f);
	v.Normalize();
	CHECK_CLOSE(0.267261f, v.GetX(), KEPSILON);
	CHECK_CLOSE(0.534522f, v.GetY(), KEPSILON);
	CHECK_CLOSE(0.801784f, v.GetZ(), KEPSILON);
}

TEST(CVector3Normal)
{
	CVector3 v(1.0f, 2.0f, 3.0f);
	CVector3 res = v.Normal();
	CHECK_CLOSE(0.267261f, res.GetX(), KEPSILON);
	CHECK_CLOSE(0.534522f, res.GetY(), KEPSILON);
	CHECK_CLOSE(0.801784f, res.GetZ(), KEPSILON);
}

TEST(CVector3Mag)
{
	CVector3 v(3.0f, 4.0f, 0.0f);
	float res = v.Mag();
	CHECK_CLOSE(5.0f, res, KEPSILON);
}

TEST(CVector3MagSquared)
{
	CVector3 v(3.0f, 4.0f, 0.0f);
	float res = v.MagSquared();
	CHECK_CLOSE(25.0f, res, KEPSILON);
}

TEST(CVector3Transform)
{
	CVector3 v(0.0f, 1.0f, 0.0f);
	CMatrix4 m;
	m.SetRotateX(PI / float(2.0f));
	CVector4 res = v.Transform(m);
	CHECK_CLOSE(0.0f, res.GetX(), KEPSILON);
	CHECK_CLOSE(0.0f, res.GetY(), KEPSILON);
	CHECK_CLOSE(1.0f, res.GetZ(), KEPSILON);
	CHECK_CLOSE(1.0f, res.GetW(), KEPSILON);
}
