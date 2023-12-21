#include "../../include/math/MathUtils.h"
#include "../../include/math/TVector2.h"
#include "../../include/math/TVector3.h"
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>


namespace TDEngine2
{
	CPerlinNoise::CPerlinNoise(U32 seed)
	{
		for (USIZE i = 0; i < mPermutationTemplate.size(); i++)
		{
			mPermutationTemplate[i] = static_cast<U16>(i);
		}

		std::shuffle(mPermutationTemplate.begin(), mPermutationTemplate.end(), std::default_random_engine(seed));

		for (USIZE i = 0; i < mCurrPermutation.size(); i++)
		{
			mCurrPermutation[i] = mPermutationTemplate[i & mGridSize];
		}
	}


	static F32 GetGradient(U16 value, const TVector2& p)
	{
		static const std::array<TVector2, 4> vectors
		{
			TVector2(1.0f),
			TVector2(-1.0f, 1.0f),
			TVector2(-1.0f),
			TVector2(1.0f, -1.0f),
		};

		return Dot(vectors[value & 3], p);
	}


	F32 CPerlinNoise::Compute2D(const TVector2& point)
	{
		const U32 gridX = static_cast<U32>(floor(point.x)) & mGridSize;
		const U32 gridY = static_cast<U32>(floor(point.y)) & mGridSize;

		const F32 xFrac = point.x - floor(point.x);
		const F32 yFrac = point.y - floor(point.y);

		const F32 dotTopRight = GetGradient(mCurrPermutation[mCurrPermutation[gridX + 1] + gridY + 1], TVector2(xFrac - 1.0f, yFrac - 1.0f));
		const F32 dotTopLeft = GetGradient(mCurrPermutation[mCurrPermutation[gridX] + gridY + 1], TVector2(xFrac, yFrac - 1.0f));
		const F32 dotBottomRight = GetGradient(mCurrPermutation[mCurrPermutation[gridX + 1] + gridY], TVector2(xFrac - 1.0f, yFrac));
		const F32 dotBottomLeft = GetGradient(mCurrPermutation[mCurrPermutation[gridX] + gridY], TVector2(xFrac, yFrac));

		const F32 v = CMathUtils::EaseInOut(yFrac);

		return CMathUtils::Lerp(
			CMathUtils::Lerp(dotBottomLeft, dotTopLeft, v),
			CMathUtils::Lerp(dotBottomRight, dotTopRight, v),
			CMathUtils::EaseInOut(xFrac));
	}

	F32 CPerlinNoise::Compute2D(const TVector2& point, I32 octavesCount, F32 frequency)
	{
		F32 amplitude = 1.0f;

		F32 result = 0.0f;

		for (I32 i = 0; i < octavesCount; i++)
		{
			result += amplitude * Compute2D(frequency * point);

			amplitude *= 0.5f;
			frequency *= 2.0f;
		}

		return result;
	}


	static F32 GetGradient(U16 value, const TVector3& p)
	{
		const U16 index = value & 15;

		const F32 u = index < 8 ? p.x : p.y;
		const F32 v = index < 4 ? p.y : index == 12 || index == 14 ? p.x : p.z;

		return ((index & 1) ? -u : u) + ((index & 2) ? -v : v);
	}


	F32 CPerlinNoise::Compute3D(const TVector3& point)
	{
		const U32 gridX = static_cast<U32>(floor(point.x)) & mGridSize;
		const U32 gridY = static_cast<U32>(floor(point.y)) & mGridSize;
		const U32 gridZ = static_cast<U32>(floor(point.z)) & mGridSize;

		const F32 xFrac = point.x - floor(point.x);
		const F32 yFrac = point.y - floor(point.y);
		const F32 zFrac = point.z - floor(point.z);

		const F32 u = CMathUtils::EaseInOut(xFrac);
		const F32 v = CMathUtils::EaseInOut(yFrac);
		const F32 w = CMathUtils::EaseInOut(zFrac);

		const U32 A = mCurrPermutation[gridX] + gridY;
		const U32 AA = mCurrPermutation[A] + gridZ;
		const U32 AB = mCurrPermutation[A + 1] + gridZ;
		const U32 B = mCurrPermutation[gridX + 1] + gridY;
		const U32 BA = mCurrPermutation[B] + gridZ;
		const U32 BB = mCurrPermutation[B + 1] + gridZ;

		return CMathUtils::Lerp(
			CMathUtils::Lerp(
				CMathUtils::Lerp(
					GetGradient(mCurrPermutation[AA], TVector3(xFrac, yFrac, zFrac)),
					GetGradient(mCurrPermutation[BA], TVector3(xFrac - 1.0f, yFrac, zFrac)), u),
				CMathUtils::Lerp(
					GetGradient(mCurrPermutation[AB], TVector3(xFrac, yFrac - 1.0f, zFrac)),
					GetGradient(mCurrPermutation[BB], TVector3(xFrac - 1.0f, yFrac - 1.0f, zFrac)), u), v),
			CMathUtils::Lerp(
				CMathUtils::Lerp(
					GetGradient(mCurrPermutation[AA + 1], TVector3(xFrac, yFrac, zFrac - 1.0f)),
					GetGradient(mCurrPermutation[BA + 1], TVector3(xFrac - 1.0f, yFrac, zFrac - 1.0f)), u),
				CMathUtils::Lerp(
					GetGradient(mCurrPermutation[AB + 1], TVector3(xFrac, yFrac - 1.0f, zFrac - 1.0f)),
					GetGradient(mCurrPermutation[BB + 1], TVector3(xFrac - 1.0f, yFrac - 1.0f, zFrac - 1.0f)), u), v),
			w);
	}
	
	F32 CPerlinNoise::Compute3D(const TVector3& point, I32 octavesCount, F32 frequency)
	{
		F32 amplitude = 1.0f;
		F32 result = 0.0f;

		for (I32 i = 0; i < octavesCount; i++)
		{
			result += amplitude * Compute3D(frequency * point);

			amplitude *= 0.5f;
			frequency *= 2.0f;
		}

		return result;
	}
}