///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <ork/cvector2.h>
#include <list>

// 2D Perlin Noise
namespace ork {

///////////////////////////////////////////////////////////////////////////////

class NoiseCache2D
{
private:
	const u32 mkSamplesPerSide; //Must be a power of two
	float* mCache;
	
	float SmoothT(float t);
	float& CacheAt(u32 x, u32 y);
	float AverageAt(u32 x, u32 y);
	float CosInterpAt(float x, float y);

public:
	NoiseCache2D(int seed = 0, u32 samplesPerSide = 64);
	void SmoothCache();
	~NoiseCache2D();
	float ValueAt(const CVector2& pos, const CVector2& offset, float amplitude, float frequency);
};

///////////////////////////////////////////////////////////////////////////////

class PerlinNoiseGenerator
{
private:
	struct Octave
	{
		NoiseCache2D* mNoise;
		float mFrequency;
		float mAmplitude;
		CVector2 mOffset;
	};

	std::list<Octave> mOctaves;

public:
	PerlinNoiseGenerator();
	~PerlinNoiseGenerator();

	void AddOctave(float frequency, float amplitude, const CVector2& offset, int seed, u32 dimensions);

	float ValueAt(const CVector2& pos);
};

///////////////////////////////////////////////////////////////////////////////

class OldPerlin2D
{
	public:

	//static const float ONEDIVNOISETABLESAMPLES =		0.03125f;
	static const int NOISETABLESAMPLES =			32;
	static const int NOISETABLESIZE =				66;			// (NOISETABLESAMPLES*2+2)
	static const int NOISETABLESIZEF4 =				264;		// NOISETABLESIZE*4 (CG float4)

	static const int B =							0x1000;
	static const int BM =							0xfff;
	static const int N =							0x1000;

	static int* p;
	static float* g2;

	void static pnnormalize2( float* v )
	{
		float s;
		s = std::sqrt( v[0] * v[0] + v[1] * v[1] );
		v[0] = v[0] / s;
		v[1] = v[1] / s;
	}

	inline static float at2( float rx, float ry, float *q )
	{
		return float( rx * q[0] + ry * q[1] );
	}

	inline static float pnlerp( float t, float a, float b )
	{
		return float( a + t * (b - a) );
	}

	// this is the smoothstep function f(t) = 3t^2 - 2t^3, without the normalization
	inline static float s_curve( float t )
	{
		return float(t * t * ( 3.0f - ( 2.0f * t ) ));
	}

	inline static void pnsetup( int i, int& b0, int& b1, float& r0, float& r1, float &t, float *vec )
	{
		t = vec[i] + N;
		b0 = ((int)t) & BM;
		b1 = (b0+1) & BM;
		r0 = t - (int)t;
		r1 = r0 - 1.0f;
	}

	static void GenerateNoiseTable( void )
	{
		p = new int[ B + B + 2 ];
		g2 = new float[ (B + B + 2)*2 ];

		int i, j, k;
		for ( i = 0 ; i < B ; i++ )
		{
			p[i] = i;
			for ( j = 0 ; j < 2 ; j++ )
			{
				g2[(i*2)+j] = ( float ) ( ( rand() % ( B + B ) ) - B ) / B;
			}

			pnnormalize2( & g2[(i*2)] );
		}
		while ( --i )
		{
			k = p[i];
            j = rand() % B;
			p[i] = p[j];
			p[j] = k;
		}
		for ( i = 0 ; i < B + 2 ; i++ )
		{
			p[B + i] = p[i];
			for ( j = 0 ; j < 2 ; j++ )
				g2[ ((B + i)*2)+j ] = g2[(i*2)+j ];
		}
	}

	static float PlaneNoiseFunc( float fu, float fv, float fou, float fov, float fAmp, float fFrq )
	{
		static bool bINIT = true;

		if( bINIT )
		{
			GenerateNoiseTable();
			bINIT = false;
		}

		int bx0, bx1, by0, by1, b00, b10, b01, b11;
		float rx0, rx1, ry0, ry1, * q, sx, sy, a, b, t, u, v;
		int i, j;
		
		float vec[2] =
		{
			fou + ( fu* fFrq ), fov + ( fv* fFrq )
		};

		pnsetup( 0, bx0, bx1, rx0, rx1, t, vec );
		pnsetup( 1, by0, by1, ry0, ry1, t, vec );

		i = p[bx0];
		j = p[bx1];
		b00 = p[i + by0];
		b10 = p[j + by0];
		b01 = p[i + by1];
		b11 = p[j + by1];
		sx = s_curve( rx0 );
		sy = s_curve( ry0 );
		q = & g2[(b00*2)] ;
		u = at2( rx0, ry0, q );
		q = & g2[(b10*2)] ;
		v = at2( rx1, ry0, q );
		a = pnlerp( sx, u, v );
		q = & g2[(b01*2)] ;
		u = at2( rx0, ry1, q );
		q = & g2[(b11*2)] ;
		v = at2( rx1, ry1, q );
		b = pnlerp( sx, u, v );

		float val = pnlerp( sy, a, b );

		return val * fAmp;
	}

};

///////////////////////////////////////////////////////////////////////////////

} //namespace ork

