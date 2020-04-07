/*!
	/file TRay.h
	/date 05.04.2020
	/authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "TVector2.h"
#include "TVector3.h"
#include "MathUtils.h"
#include <tuple>


namespace TDEngine2
{
	/*!
		struct TRay

		\brief The structure represents template implementation of a ray
	*/

	template <typename T>
	struct TRay
	{
		TRay(const T& origin, const T& direction) :
			origin(origin), dir(Normalize(direction))
		{
		}

		T origin;
		T dir;
	};


	typedef TRay<TVector2> TRay2D;
	typedef TRay<TVector3> TRay3D;


	/*!
		\brief The function returns shortest distance between two lines

		\param[in] line1 
		\param[in] line2

		\return The function returns a value of a distance between two lines
	*/

	template <typename T>
	F32 CalcShortestDistanceBetweenLines(const TRay<T>& line1, const TRay<T>& line2)
	{
		T dir = line2.origin - line1.origin;

		const T n = Cross(line1.dir, line2.dir);

		if (Length(n) < 1e-3f)
		{
			return Length(Cross(line1.dir, dir));
		}

		return std::abs(Dot(Cross(line1.dir, line2.dir), dir) / Length(Cross(line1.dir, line2.dir)));

#if 0
		const F32 sqrV1 = Dot(line1.dir, line1.dir);
		const F32 sqrV2 = Dot(line2.dir, line2.dir);
		const F32 v1v2  = Dot(line1.dir, line2.dir);

		const F32 det = v1v2 * v1v2 - sqrV1 * sqrV2;

		// \note lines are parallel
		if (CMathUtils::IsLessOrEqual(std::abs(det), (std::numeric_limits<F32>::min)()))
		{
			return Length(dir - Dot(dir, line1.dir) * line1.dir);
		}

		const F32 invDet = 1.0f / det;

		const F32 dotDirV1 = Dot(dir, line1.dir);
		const F32 dotDirV2 = Dot(dir, line2.dir);

		const F32 t1 = invDet * (sqrV2 * dotDirV1 - v1v2 * dotDirV2);
		const F32 t2 = invDet * (v1v2 * dotDirV1 - sqrV1 * dotDirV2);

		const T a = dir + line2.dir * t2 - line1.dir * t1;
		return std::sqrt(Dot(a, a));
#endif
	}
}