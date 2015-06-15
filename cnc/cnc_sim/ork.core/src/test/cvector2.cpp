#include <unittest++/UnitTest++.h>
#include <cmath>
#include <limits>
#include <ork/cvector2.h>
#include <ork/math_misc.h>

using namespace ork;

static const float KEPSILON = 5.0e-07f;//std::numeric_limits<float>::epsilon();

TEST(CVector2DefaultConstructor)
{
	CVector2 v;
	CHECK_CLOSE(0.0f, v.GetX(), KEPSILON);
	CHECK_CLOSE(0.0f, v.GetY(), KEPSILON);
}

TEST(CVector2Constructor)
{
	CVector2 v(1.0f, 2.0f);
	CHECK_CLOSE(1.0f, v.GetX(), KEPSILON);
	CHECK_CLOSE(2.0f, v.GetY(), KEPSILON);
}

TEST(CVector2CopyConstructor)
{
	CVector2 v(1.0f, 2.0f);
	CVector2 copy(v);
	CHECK_CLOSE(1.0f, copy.GetX(), KEPSILON);
	CHECK_CLOSE(2.0f, copy.GetY(), KEPSILON);
}

TEST(CVector2Rotate)
{
	CVector2 v(0.0f, 1.0f);
	v.Rotate(PI / 2.0f);
	CHECK_CLOSE(0.0f, v.GetX(), KEPSILON);
	CHECK_CLOSE(1.0f, v.GetY(), KEPSILON);
}

TEST(CVector2Add)
{
	CVector2 v(1.0f, 0.0f);
	CVector2 res = v + CVector2(0.0f, 1.0f);
	CHECK_CLOSE(1.0f, res.GetX(), KEPSILON);
	CHECK_CLOSE(1.0f, res.GetY(), KEPSILON);
}

TEST(CVector2AddTo)
{
	CVector2 v(1.0f, 0.0f);
	v += CVector2(0.0f, 1.0f);
	CHECK_CLOSE(1.0f, v.GetX(), KEPSILON);
	CHECK_CLOSE(1.0f, v.GetY(), KEPSILON);
}

TEST(CVector2Subtract)
{
	CVector2 v(1.0f, 0.0f);
	CVector2 res = v - CVector2(0.0f, 1.0f);
	CHECK_CLOSE(1.0f, res.GetX(), KEPSILON);
	CHECK_CLOSE(-1.0f, res.GetY(), KEPSILON);
}

TEST(CVector2SubtractFrom)
{
	CVector2 v(1.0f, 0.0f);
	v -= CVector2(0.0f, 1.0f);
	CHECK_CLOSE(1.0f, v.GetX(), KEPSILON);
	CHECK_CLOSE(-1.0f, v.GetY(), KEPSILON);
}

TEST(CVector2Multiply)
{
	CVector2 v(1.0f, 2.0f);
	CVector2 res = v * CVector2(3.0f, 2.0f);
	CHECK_CLOSE(3.0f, res.GetX(), KEPSILON);
	CHECK_CLOSE(4.0f, res.GetY(), KEPSILON);
}

TEST(CVector2MultiplyTo)
{
	CVector2 v(1.0f, 2.0f);
	v *= CVector2(3.0f, 2.0f);
	CHECK_CLOSE(3.0f, v.GetX(), KEPSILON);
	CHECK_CLOSE(4.0f, v.GetY(), KEPSILON);
}

TEST(CVector2Divide)
{
	CVector2 v(1.0f, 2.0f);
	CVector2 res = v / CVector2(3.0f, 2.0f);
	CHECK_CLOSE(0.333333f, res.GetX(), KEPSILON);
	CHECK_CLOSE(1.0f, res.GetY(), KEPSILON);
}

TEST(CVector2DivideTo)
{
	CVector2 v(1.0f, 2.0f);
	v /= CVector2(3.0f, 2.0f);
	CHECK_CLOSE(0.333333f, v.GetX(), KEPSILON);
	CHECK_CLOSE(1.0f, v.GetY(), KEPSILON);
}

TEST(CVector2Scale)
{
	CVector2 v(1.0f, 2.0f);
	CVector2 res = v * 3.0f;
	CHECK_CLOSE(3.0f, res.GetX(), KEPSILON);
	CHECK_CLOSE(6.0f, res.GetY(), KEPSILON);
}

TEST(CVector2ScaleTo)
{
	CVector2 v(1.0f, 2.0f);
	v *= 3.0f;
	CHECK_CLOSE(3.0f, v.GetX(), KEPSILON);
	CHECK_CLOSE(6.0f, v.GetY(), KEPSILON);
}

TEST(CVector2ScalePre)
{
	CVector2 v(1.0f, 2.0f);
	CVector2 res = 3.0f * v;
	CHECK_CLOSE(3.0f, res.GetX(), KEPSILON);
	CHECK_CLOSE(6.0f, res.GetY(), KEPSILON);
}

TEST(CVector2InvScale)
{
	CVector2 v(1.0f, 2.0f);
	CVector2 res = v / 3.0f;
	CHECK_CLOSE(0.333333f, res.GetX(), KEPSILON);
	CHECK_CLOSE(0.666667f, res.GetY(), KEPSILON);
}

TEST(CVector2InvScaleTo)
{
	CVector2 v(1.0f, 2.0f);
	v /= 3.0f;
	CHECK_CLOSE(0.333333f, v.GetX(), KEPSILON);
	CHECK_CLOSE(0.666667f, v.GetY(), KEPSILON);
}

TEST(CVector2EqualCompare)
{
	CVector2 v1(1.0f, 2.0f);
	CVector2 v2(1.0f, 2.0f);
	CVector2 v3(4.0f, 3.0f);
	CHECK_EQUAL(true, v1 == v2);
	CHECK_EQUAL(false, v1 == v3);
}

TEST(CVector2NotEqualCompare)
{
	CVector2 v1(1.0f, 2.0f);
	CVector2 v2(1.0f, 2.0f);
	CVector2 v3(4.0f, 3.0f);
	CHECK_EQUAL(false, v1 != v2);
	CHECK_EQUAL(true, v1 != v3);
}

TEST(CVector2Dot)
{
	CVector2 v1(1.0f, 2.0f);
	CVector2 v2(2.0f, 1.0f);
	float res = v1.Dot(v2);
	CHECK_CLOSE(4.0f, res, KEPSILON);
}

TEST(CVector2Normalize)
{
	CVector2 v(1.0f, 2.0f);
	v.Normalize();
	CHECK_CLOSE(0.447214f, v.GetX(), KEPSILON);
	CHECK_CLOSE(0.894427f, v.GetY(), KEPSILON);
}

TEST(CVector2Normal)
{
	CVector2 v(1.0f, 2.0f);
	CVector2 res = v.Normal();
	CHECK_CLOSE(0.447214f, res.GetX(), KEPSILON);
	CHECK_CLOSE(0.894427f, res.GetY(), KEPSILON);
}

TEST(CVector2Mag)
{
	CVector2 v(3.0f, 4.0f);
	float res = v.Mag();
	CHECK_CLOSE(5.0f, res, KEPSILON);
}

TEST(CVector2MagSquared)
{
	CVector2 v(3.0f, 4.0f);
	float res = v.MagSquared();
	CHECK_CLOSE(25.0f, res, KEPSILON);
}
