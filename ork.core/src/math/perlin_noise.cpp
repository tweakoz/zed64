///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#include <ork/types.h>
#include <ork/math_misc.h>
#include <ork/perlin_noise.h>

using ork::CVector2;
using ork::TVector2;

namespace ork {


float LinearInterp(float t, float a, float b)
{
	return a + t * (b - a);
}

inline float CosInterp(float t, float a, float b)
{
	float f = (float(1) - std::cos(t * PI) * float(0.5f));
	return (a * (float(1) - f)) + (b * f);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//This is the smoothstep function f(t) = 3t^2 - 2t^3, without the normalization
float NoiseCache2D::SmoothT(float t)
{
	return t * t * (float(2.0f) * t - float(3.0f));
}

float& NoiseCache2D::CacheAt(u32 x, u32 y)
{
	x &= mkSamplesPerSide - 1;
	y &= mkSamplesPerSide - 1;
	
	return mCache[x * mkSamplesPerSide + y];
}

float NoiseCache2D::AverageAt(u32 x, u32 y)
{
	const float corners((CacheAt(x-1, y-1) + CacheAt(x-1, y+1) + CacheAt(x+1, y-1) + CacheAt(x+1, y+1)) * float(0.0625f));
	const float sides  ((CacheAt(x  , y-1) + CacheAt(x  , y+1) + CacheAt(x  , y-1) + CacheAt(x  , y+1)) * float(0.125f));
	const float center ((CacheAt(x  , y  ) * float(0.25f)));

	return corners + sides + center;
}

float NoiseCache2D::CosInterpAt(float x, float y)
{
	const u32 wholeX = u32(x);
	const u32 wholeY = u32(y);
	
	const float fracX = (x - wholeX);
	const float fracY = (y - wholeY);

	const float v1(AverageAt(wholeX    , wholeY    ));
	const float v2(AverageAt(wholeX + 1, wholeY    ));
	const float v3(AverageAt(wholeX    , wholeY + 1));
	const float v4(AverageAt(wholeX + 1, wholeY + 1));

	return CosInterp(CosInterp(v1, v2, fracX), CosInterp(v3, v4, fracX), fracY);
}

void NoiseCache2D::SmoothCache()
{
	float* newCache = new float[mkSamplesPerSide * mkSamplesPerSide];
	
	for (u32 x = 0; x < mkSamplesPerSide; ++x)
	{
		for (u32 y = 0; y < mkSamplesPerSide; ++y)
		{
			newCache[x * mkSamplesPerSide + y] = AverageAt(x, y);
		//	orkprintf("\tOld: %f\tNew: %f\n", CacheAt(x, y), AverageAt(x, y));
		}
	}
	
//	orkprintf("Smoothed Cache (%d)\n", mkSamplesPerSide);
			
	delete mCache;
	mCache = newCache;
}

NoiseCache2D::NoiseCache2D(int seed, u32 samplesPerSide)
	: mCache(NULL)
	, mkSamplesPerSide(samplesPerSide)

{
	OrkAssert(ork::IsPowerOfTwo(int(mkSamplesPerSide)));

	mCache = new float[mkSamplesPerSide * mkSamplesPerSide];

	for (int x = 0; x < int(mkSamplesPerSide); ++x)
	{
		for (int y = 0; y < int(mkSamplesPerSide); ++y)
		{
			int noise = x + (y * 57) + (seed * 131);
			noise = (noise << 13) ^ noise;

			CacheAt(u32(x), u32(y)) = float(1.0f) - float((noise * (noise * noise * 15731 + 789221) + 1376312589) & 0x7fffffff) *
				float(0.000000000931322574615478515625f); 
		}
	}
}

NoiseCache2D::~NoiseCache2D()
{
	delete[] mCache;
}

float NoiseCache2D::ValueAt(const CVector2& pos, const CVector2& offset, float amplitude, float frequency)
{
	uint32_t numSamplesMask = mkSamplesPerSide - 1;

	CVector2 adjPos = (pos + offset) * frequency;
	int iX = int(adjPos.GetX());
	int iY = int(adjPos.GetY());

	const float smoothXT(SmoothT(adjPos.GetX() - float(iX)));
	const float smoothYT(SmoothT(adjPos.GetY() - float(iY)));

	const float edgePos0(LinearInterp(smoothYT, CacheAt(uint32_t(iX),     uint32_t(iY)), CacheAt(uint32_t(iX),     uint32_t(iY + 1))));
	const float edgePos1(LinearInterp(smoothYT, CacheAt(uint32_t(iX + 1), uint32_t(iY)), CacheAt(uint32_t(iX + 1), uint32_t(iY + 1))));
	return amplitude * float(LinearInterp(smoothXT, edgePos0, edgePos1));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

PerlinNoiseGenerator::PerlinNoiseGenerator()
{
	new(&mOctaves) std::list<Octave>;
}

PerlinNoiseGenerator::~PerlinNoiseGenerator()
{
	while (!mOctaves.empty())
	{
		delete mOctaves.front().mNoise;
		mOctaves.pop_front();
	}
}

void PerlinNoiseGenerator::AddOctave(float frequency, float amplitude, const CVector2& offset, int seed, uint32_t dimensions)
{
	Octave octave;
	octave.mAmplitude = amplitude;
	octave.mFrequency = frequency;
	octave.mOffset = offset;
	octave.mNoise = new NoiseCache2D(seed, dimensions);
	octave.mNoise->SmoothCache();

	mOctaves.push_back(octave);
}

float PerlinNoiseGenerator::ValueAt(const CVector2& pos)
{
	float total(0.0f);

	for (std::list<Octave>::iterator it = mOctaves.begin(); it != mOctaves.end(); ++it) 
		total += it->mNoise->ValueAt(pos, it->mOffset, it->mAmplitude, it->mFrequency);

	return total;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

} //namespace ork
