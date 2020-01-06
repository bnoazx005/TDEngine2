/*!
	/file MathUtils.h
	/date 22.11.2018
	/authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Config.h"
#include "./../utils/Types.h"


namespace TDEngine2
{
	/*!
		\brief The static class contains common mathematical constants
		that are often used
	*/

	class CMathConstants
	{
		public:
			constexpr static F32 Pi      = 3.141592f;

			constexpr static F32 Exp     = 2.728281f;

			constexpr static F32 Deg2Rad = Pi / 180.0f;

			constexpr static F32 Rad2Deg = 180.0f / Pi;
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
	};


}