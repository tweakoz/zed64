#include <unittest++/UnitTest++.h>
#include <cmath>
#include <limits>
#include <ork/cvector3.h>
#include <ork/cvector4.h>
#include <ork/cmatrix4.h>
#include <ork/math_misc.h>

using namespace ork;
static const float KEPSILON = 5.0e-07f;//std::numeric_limits<float>::epsilon();

TEST(CVector4DefaultConstructor)
{
	CVector4 v;
	CHECK_CLOSE(0.0f, v.GetX(), KEPSILON);
	CHECK_CLOSE(0.0f, v.GetY(), KEPSILON);
	CHECK_CLOSE(0.0f, v.GetZ(), KEPSILON);
	CHECK_CLOSE(1.0f, v.GetW(), KEPSILON);
}

TEST(CVector4Constructor)
{
	CVector4 v(1.0f, 2.0f, 3.0f, 4.0f);
	CHECK_CLOSE(1.0f, v.GetX(), KEPSILON);
	CHECK_CLOSE(2.0f, v.GetY(), KEPSILON);
	CHECK_CLOSE(3.0f, v.GetZ(), KEPSILON);
	CHECK_CLOSE(4.0f, v.GetW(), KEPSILON);
}

TEST(CVector4CopyConstructor)
{
	CVector4 v(1.0f, 2.0f, 3.0f, 4.0f);
	CVector4 copy(v);
	CHECK_CLOSE(1.0f, copy.GetX(), KEPSILON);
	CHECK_CLOSE(2.0f, copy.GetY(), KEPSILON);
	CHECK_CLOSE(3.0f, copy.GetZ(), KEPSILON);
	CHECK_CLOSE(4.0f, copy.GetW(), KEPSILON);
}

TEST(CVector4RGBAConstructor)
{
	CVector4 v(0xFF00FFFF);
	CHECK_CLOSE(1.0f, v.GetX(), KEPSILON);
	CHECK_CLOSE(0.0f, v.GetY(), KEPSILON);
	CHECK_CLOSE(1.0f, v.GetZ(), KEPSILON);
	CHECK_CLOSE(1.0f, v.GetW(), KEPSILON);
}

TEST(CVector4RotateX)
{
	CVector4 v(0.0f, 1.0f, 0.0f);
	v.RotateX(PI / float(2.0f));
	CHECK_CLOSE(0.0f, v.GetX(), KEPSILON);
	CHECK_CLOSE(0.0f, v.GetY(), KEPSILON);
	CHECK_CLOSE(1.0f, v.GetZ(), KEPSILON);
	CHECK_CLOSE(1.0f, v.GetW(), KEPSILON);
}

TEST(CVector4RotateY)
{
	CVector4 v(1.0f, 0.0f, 0.0f);
	v.RotateY(PI / float(2.0f));
	CHECK_CLOSE(0.0f, v.GetX(), KEPSILON);
	CHECK_CLOSE(0.0f, v.GetY(), KEPSILON);
	CHECK_CLOSE(1.0f, v.GetZ(), KEPSILON);
	CHECK_CLOSE(1.0f, v.GetW(), KEPSILON);
}

TEST(CVector4RotateZ)
{
	CVector4 v(1.0f, 0.0f, 0.0f);
	v.RotateZ(PI / float(2.0f));
	CHECK_CLOSE(0.0f, v.GetX(), KEPSILON);
	CHECK_CLOSE(1.0f, v.GetY(), KEPSILON);
	CHECK_CLOSE(0.0f, v.GetZ(), KEPSILON);
	CHECK_CLOSE(1.0f, v.GetW(), KEPSILON);
}

TEST(CVector4Add)
{
	CVector4 v(1.0f, 0.0f, 0.0f);
	CVector4 res = v + CVector4(0.0f, 1.0f, 0.0f);
	CHECK_CLOSE(1.0f, res.GetX(), KEPSILON);
	CHECK_CLOSE(1.0f, res.GetY(), KEPSILON);
	CHECK_CLOSE(0.0f, res.GetZ(), KEPSILON);
	CHECK_CLOSE(2.0f, res.GetW(), KEPSILON);
}

TEST(CVector4AddTo)
{
	CVector4 v(1.0f, 0.0f, 0.0f);
	v += CVector4(0.0f, 1.0f, 0.0f);
	CHECK_CLOSE(1.0f, v.GetX(), KEPSILON);
	CHECK_CLOSE(1.0f, v.GetY(), KEPSILON);
	CHECK_CLOSE(0.0f, v.GetZ(), KEPSILON);
	CHECK_CLOSE(2.0f, v.GetW(), KEPSILON);
}

TEST(CVector4Subtract)
{
	CVector4 v(1.0f, 0.0f, 0.0f);
	CVector4 res = v - CVector4(0.0f, 1.0f, 0.0f);
	CHECK_CLOSE(1.0f, res.GetX(), KEPSILON);
	CHECK_CLOSE(-1.0f, res.GetY(), KEPSILON);
	CHECK_CLOSE(0.0f, res.GetZ(), KEPSILON);
	CHECK_CLOSE(0.0f, res.GetW(), KEPSILON);
}

TEST(CVector4SubtractFrom)
{
	CVector4 v(1.0f, 0.0f, 0.0f);
	v -= CVector4(0.0f, 1.0f, 0.0f);
	CHECK_CLOSE(1.0f, v.GetX(), KEPSILON);
	CHECK_CLOSE(-1.0f, v.GetY(), KEPSILON);
	CHECK_CLOSE(0.0f, v.GetZ(), KEPSILON);
	CHECK_CLOSE(0.0f, v.GetW(), KEPSILON);
}

TEST(CVector4Multiply)
{
	CVector4 v(1.0f, 2.0f, 3.0f);
	CVector4 res = v * CVector4(3.0f, 2.0f, 1.0f);
	CHECK_CLOSE(3.0f, res.GetX(), KEPSILON);
	CHECK_CLOSE(4.0f, res.GetY(), KEPSILON);
	CHECK_CLOSE(3.0f, res.GetZ(), KEPSILON);
	CHECK_CLOSE(1.0f, res.GetW(), KEPSILON);
}

TEST(CVector4MultiplyTo)
{
	CVector4 v(1.0f, 2.0f, 3.0f);
	v *= CVector4(3.0f, 2.0f, 1.0f);
	CHECK_CLOSE(3.0f, v.GetX(), KEPSILON);
	CHECK_CLOSE(4.0f, v.GetY(), KEPSILON);
	CHECK_CLOSE(3.0f, v.GetZ(), KEPSILON);
	CHECK_CLOSE(1.0f, v.GetW(), KEPSILON);
}

TEST(CVector4Divide)
{
	CVector4 v(1.0f, 2.0f, 3.0f);
	CVector4 res = v / CVector4(3.0f, 2.0f, 1.0f);
	CHECK_CLOSE(.333333f, res.GetX(), KEPSILON);
	CHECK_CLOSE(1.0f, res.GetY(), KEPSILON);
	CHECK_CLOSE(3.0f, res.GetZ(), KEPSILON);
	CHECK_CLOSE(1.0f, res.GetW(), KEPSILON);
}

TEST(CVector4DivideTo)
{
	CVector4 v(1.0f, 2.0f, 3.0f);
	v /= CVector4(3.0f, 2.0f, 1.0f);
	CHECK_CLOSE(0.333333f, v.GetX(), KEPSILON);
	CHECK_CLOSE(1.0f, v.GetY(), KEPSILON);
	CHECK_CLOSE(3.0f, v.GetZ(), KEPSILON);
	CHECK_CLOSE(1.0f, v.GetW(), KEPSILON);
}

TEST(CVector4Scale)
{
	CVector4 v(1.0f, 2.0f, 3.0f);
	CVector4 res = v * float(3.0f);
	CHECK_CLOSE(3.0f, res.GetX(), KEPSILON);
	CHECK_CLOSE(6.0f, res.GetY(), KEPSILON);
	CHECK_CLOSE(9.0f, res.GetZ(), KEPSILON);
	CHECK_CLOSE(3.0f, res.GetW(), KEPSILON);
}

TEST(CVector4ScaleTo)
{
	CVector4 v(1.0f, 2.0f, 3.0f);
	v *= float(3.0f);
	CHECK_CLOSE(3.0f, v.GetX(), KEPSILON);
	CHECK_CLOSE(6.0f, v.GetY(), KEPSILON);
	CHECK_CLOSE(9.0f, v.GetZ(), KEPSILON);
	CHECK_CLOSE(3.0f, v.GetW(), KEPSILON);
}

TEST(CVector4ScalePre)
{
	CVector4 v(1.0f, 2.0f, 3.0f);
	CVector4 res = float(3.0f) * v;
	CHECK_CLOSE(3.0f, res.GetX(), KEPSILON);
	CHECK_CLOSE(6.0f, res.GetY(), KEPSILON);
	CHECK_CLOSE(9.0f, res.GetZ(), KEPSILON);
	CHECK_CLOSE(1.0f, res.GetW(), KEPSILON);
}

TEST(CVector4InvScale)
{
	CVector4 v(1.0f, 2.0f, 3.0f);
	CVector4 res = v / float(3.0f);
	CHECK_CLOSE(0.333333f, res.GetX(), KEPSILON);
	CHECK_CLOSE(0.666667f, res.GetY(), KEPSILON);
	CHECK_CLOSE(1.0f, res.GetZ(), KEPSILON);
	CHECK_CLOSE(0.333333f, res.GetW(), KEPSILON);
}

TEST(CVector4InvScaleTo)
{
	CVector4 v(1.0f, 2.0f, 3.0f);
	v /= float(3.0f);
	CHECK_CLOSE(0.333333f, v.GetX(), KEPSILON);
	CHECK_CLOSE(0.666667f, v.GetY(), KEPSILON);
	CHECK_CLOSE(1.0f, v.GetZ(), KEPSILON);
	CHECK_CLOSE(0.333333f, v.GetW(), KEPSILON);
}

TEST(CVector4EqualCompare)
{
	CVector4 v1(1.0f, 2.0f, 3.0f, 4.0f);
	CVector4 v2(1.0f, 2.0f, 3.0f, 4.0f);
	CVector4 v3(4.0f, 3.0f, 2.0f, 1.0f);
	CHECK_EQUAL(true, v1 == v2);
	CHECK_EQUAL(false, v1 == v3);
}

TEST(CVector4NotEqualCompare)
{
	CVector4 v1(1.0f, 2.0f, 3.0f, 4.0f);
	CVector4 v2(1.0f, 2.0f, 3.0f, 4.0f);
	CVector4 v3(4.0f, 3.0f, 2.0f, 1.0f);
	CHECK_EQUAL(false, v1 != v2);
	CHECK_EQUAL(true, v1 != v3);
}

TEST(CVector4Dot)
{
	CVector4 v1(1.0f, 2.0f, 3.0f);
	CVector4 v2(4.0f, 3.0f, 2.0f);
	float res = v1.Dot(v2);
	CHECK_CLOSE(16.0f, res, KEPSILON);
}

TEST(CVector4Cross)
{
	CVector4 v1(1.0f, 2.0f, 3.0f);
	CVector4 v2(4.0f, 3.0f, 2.0f);
	CVector4 res = v1.Cross(v2);
	CHECK_CLOSE(-5.0f, res.GetX(), KEPSILON);
	CHECK_CLOSE(10.0f, res.GetY(), KEPSILON);
	CHECK_CLOSE(-5.0f, res.GetZ(), KEPSILON);
	CHECK_CLOSE(1.0f, res.GetW(), KEPSILON);
}

TEST(CVector4Normalize)
{
	CVector4 v(1.0f, 2.0f, 3.0f);
	v.Normalize();
	CHECK_CLOSE(0.267261f, v.GetX(), KEPSILON);
	CHECK_CLOSE(0.534522f, v.GetY(), KEPSILON);
	CHECK_CLOSE(0.801784f, v.GetZ(), KEPSILON);
	CHECK_CLOSE(1.0f, v.GetW(), KEPSILON);
}

TEST(CVector4Normal)
{
	CVector4 v(1.0f, 2.0f, 3.0f);
	CVector4 res = v.Normal();
	CHECK_CLOSE(0.267261f, res.GetX(), KEPSILON);
	CHECK_CLOSE(0.534522f, res.GetY(), KEPSILON);
	CHECK_CLOSE(0.801784f, res.GetZ(), KEPSILON);
	CHECK_CLOSE(1.0f, res.GetW(), KEPSILON);
}

TEST(CVector4Mag)
{
	CVector4 v(3.0f, 4.0f, 0.0f);
	float res = v.Mag();
	CHECK_CLOSE(5.0f, res, KEPSILON);
}

TEST(CVector4MagSquared)
{
	CVector4 v(3.0f, 4.0f, 0.0f);
	float res = v.MagSquared();
	CHECK_CLOSE(25.0f, res, KEPSILON);
}

TEST(CVector4Transform)
{
	CVector4 v(0.0f, 1.0f, 0.0f);
	CMatrix4 m;
	m.SetRotateX(PI / float(2.0f));
	CVector4 res = v.Transform(m);
	CHECK_CLOSE(0.0f, res.GetX(), KEPSILON);
	CHECK_CLOSE(0.0f, res.GetY(), KEPSILON);
	CHECK_CLOSE(1.0f, res.GetZ(), KEPSILON);
	CHECK_CLOSE(1.0f, res.GetW(), KEPSILON);
}

TEST(CVector4PerspectiveDivide)
{
	CVector4 v(4.0f, 8.0f, 12.0f, 4.0f);
	v.PerspectiveDivide();
	CHECK_CLOSE(1.0f, v.GetX(), KEPSILON);
	CHECK_CLOSE(2.0f, v.GetY(), KEPSILON);
	CHECK_CLOSE(3.0f, v.GetZ(), KEPSILON);
	CHECK_CLOSE(1.0f, v.GetW(), KEPSILON);
}


TEST(CVector4_YO)
{
	float fx0 = 100.0f;
	float fy0 = 110.0f;
	float fx1 = 300.0f;
	float fy1 = 320.0f;

	float fu0 = 0.0f;
	float fv0 = 0.0f;
	float fu1 = 1.0f;
	float fv1 = 1.0f;

	float fw_W = (fx1-fx0);
	float fw_H = (fy1-fy0);
	float fu_W = (fu1-fu0);
	float fu_H = (fv1-fv0);

	float fw2u_hsca = fu_W/fw_W;
	float fw2u_vsca = fu_H/fw_H;
	float fw2u_hoff = -fx0;
	float fw2u_voff = -fy0;

	fmtx4 w2u_mtx, w2u_Smtx, w2u_Tmtx;
	w2u_Smtx.SetScale(fw2u_hsca,fw2u_vsca,1.0f);
	w2u_Tmtx.SetTranslation( fw2u_hoff, fw2u_voff, 0.0f );
	w2u_mtx = w2u_Tmtx*w2u_Smtx;

	fvec4 v0 = fvec4(fx0, fy0, 0.0f).Transform(w2u_mtx);
	fvec4 v1 = fvec4(fx1, fy1, 0.0f).Transform(w2u_mtx);

	printf( "aa::V0<%f %f> V1<%f %f>\n", v0.GetX(), v0.GetY(), v1.GetX(), v1.GetY() );

	//CHECK_CLOSE(1.0f, v.GetW(), KEPSILON);
}
