/*!
	/file MathUtils.h
	/date 22.11.2018
	/authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Config.h"
#include "./../utils/Types.h"
#include "../core/Serialization.h"
#include <array>


namespace TDEngine2
{
	/*!
		\brief The static class contains common mathematical constants
		that are often used
	*/

	class CMathConstants
	{
		public:
			TDE2_API TDE2_STATIC_CONSTEXPR F32 Pi      = 3.141592f;

			TDE2_API TDE2_STATIC_CONSTEXPR F32 Exp     = 2.728281f;
			
			TDE2_API TDE2_STATIC_CONSTEXPR F32 Deg2Rad = Pi / 180.0f;

			TDE2_API TDE2_STATIC_CONSTEXPR F32 Rad2Deg = 180.0f / Pi;
	};


	/*!
		class CMathUtils

		\bruef The static class that contains common used mathematical functions
	*/

	class CMathUtils
	{
		public:
			TDE2_API constexpr static inline F32 Max(F32 a, F32 b) { return (a < b) ? b : a; }
			TDE2_API constexpr static inline F32 Min(F32 a, F32 b) { return (a < b) ? a : b; }
			TDE2_API constexpr static inline F32 Clamp(F32 leftBound, F32 rightBound, F32 value) { return Min(rightBound, Max(leftBound, value)); }
			TDE2_API constexpr static inline F32 Clamp01(F32 value) { return Min(1.0f, Max(0.0f, value)); }
			TDE2_API constexpr static inline F32 Lerp(F32 a, F32 b, F32 t) { return a * (1.0f - t) + b * t; }
			TDE2_API constexpr static inline F32 Abs(F32 value) { return value < 0.0f ? -value : value; }
			TDE2_API constexpr static inline bool IsGreatOrEqual(F32 a, F32 b, F32 epsilon = 1e-6f) { return a > b || Abs(a - b) < epsilon; }
			TDE2_API constexpr static inline bool IsLessOrEqual(F32 a, F32 b, F32 epsilon = 1e-6f) { return a < b || Abs(a - b) < epsilon; }
			TDE2_API constexpr static inline bool IsInInclusiveRange(F32 left, F32 right, F32 value) { return IsGreatOrEqual(value, left) && IsLessOrEqual(value, right); }

			template <typename T>
			TDE2_API static inline T Lerp(const T& a, const T& b, F32 t) { return static_cast<T>(a * (1.0f - t) + b * t); }

			template <typename T>
			TDE2_API static inline T CubicBezierInterpolation(F32 t, const T& p0, const T& t0, const T& p1, const T& t1)
			{
				const F32 invT = 1.0f - t;

				return p0 * (invT * invT * invT) + t0 * (3.0f * invT * invT * t) + t1 * (3.0f * invT * t * t) + p1 * (t * t * t);
			}

			template <typename T>
			TDE2_API static inline T CubicHermiteInterpolation(F32 t, const T& p0, const T& s0, const T& p1, const T& s1)
			{
				const F32 invT = 1.0f - t;

				return p0 * (invT * invT * (1.0f + 2.0f * t)) + s0 * (invT * invT * t) + p1 * ((3.0f - 2.0f * t) * t * t) + s1 * (t * t * (t - 1.0f));
			}

			TDE2_API constexpr static F32 EaseInOut(F32 t) { return ((6.0f * t - 15.0f) * t + 10.0f) * t * t * t; }
	};


	template <typename T>
	struct TRange
	{
		TDE2_API constexpr explicit TRange() :
			mLeft(0), mRight(0)
		{
		}

		TDE2_API constexpr explicit TRange(const T& initializer) :
			mLeft(initializer), mRight(initializer)
		{
		}

		TDE2_API constexpr TRange(const T& left, const T& right):
			mLeft(left), mRight(right)
		{
			TDE2_ASSERT(IsValid());
		}

		TDE2_API constexpr bool Contains(const T& value) { return value >= mLeft && value <= mRight; }

		TDE2_API constexpr bool IsValid() const { return mLeft <= mRight; }

		T mLeft, mRight;
	};


	typedef TRange<F32> TRangeF32;


	/*!
		\brief TRange<T>'s Serialization/Deserialization helpers
	*/


	template <typename T>
	TResult<TRange<T>> LoadRange(class IArchiveReader* pReader)
	{
		TRange<T> outputRange;

		E_RESULT_CODE result = RC_OK;

		{
			result = result | pReader->BeginGroup("left");

			auto getLeftResult = Deserialize<T>(pReader);
			if (getLeftResult.HasError())
			{
				result = result | getLeftResult.GetError();
			}

			result = result | pReader->EndGroup();

			outputRange.mLeft = getLeftResult.Get();
		}
		
		{
			result = result | pReader->BeginGroup("right");

			auto getRightResult = Deserialize<T>(pReader);
			if (getRightResult.HasError())
			{
				result = result | getRightResult.GetError();
			}

			result = result | pReader->EndGroup();

			outputRange.mRight = getRightResult.Get();
		}

		if (RC_OK != result)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		return Wrench::TOkValue<TRange<T>>(outputRange);
	}

	template <typename T>
	E_RESULT_CODE SaveRange(class IArchiveWriter* pWriter, const TRange<T>& range)
	{
		E_RESULT_CODE result = RC_OK;

		{
			result = result | pWriter->BeginGroup("left");
			result = result | Serialize<T>(pWriter, range.mRight);
			result = result | pWriter->EndGroup();
		}

		{
			result = result | pWriter->BeginGroup("right");
			result = result | Serialize<T>(pWriter, range.mLeft);
			result = result | pWriter->EndGroup();
		}

		return result;
	}


	class CRandomUtils
	{
		public:
			TDE2_API static F32 GetRandF32Value(const TRangeF32& range) { return range.mLeft + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (range.mRight - range.mLeft))); }
	};


	struct TVector2;
	struct TVector3;


	class CPerlinNoise
	{
		public:
			TDE2_API CPerlinNoise() = delete;
			TDE2_API explicit CPerlinNoise(U32 seed);

			TDE2_API F32 Compute2D(const TVector2& point);
			TDE2_API F32 Compute2D(const TVector2& point, I32 octavesCount, F32 frequency = 0.005f);

			TDE2_API F32 Compute3D(const TVector3& point);
			TDE2_API F32 Compute3D(const TVector3& point, I32 octavesCount, F32 frequency = 0.005f);
		private:
			static constexpr USIZE mGridSize = 255;
			std::array<U16, (mGridSize + 1)> mPermutationTemplate;
			std::array<U16, (mGridSize + 1) << 1> mCurrPermutation;
	};

}