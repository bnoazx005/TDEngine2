/*!
	/file TRay.h
	/date 05.04.2020
	/authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "TVector2.h"
#include "TVector3.h"
#include "TPlane.h"
#include "MathUtils.h"
#include <tuple>
#include <cmath>


namespace TDEngine2
{
	/*!
		struct TRay

		\brief The structure represents template implementation of a ray
	*/

	template <typename T>
	struct TRay
	{
		TRay() = default;
		TRay(const T& origin, const T& direction) :
			origin(origin), dir(Normalize(direction))
		{
		}

		T origin;
		T dir;

		/*!
			\brief The operator is an implementation of sampling algorithm for the ray

			\param[in] t A float parameter which tells how far from the origin the sampled point should lie

			\return A point which is computed by the following formula origin + t x dir
		*/

		inline T operator() (F32 t) const
		{
			return origin + t * dir;
		}
	};


	typedef TRay<TVector2> TRay2D;
	typedef TRay<TVector3> TRay3D;


	/*!
		\brief The function returns shortest distance between a line (ray) and a point in the space

		\param[in] line A ray with same dimension as the given point
		\param[in] point A nD vector which represents a point in the space
	*/

	template <typename T>
	F32 CalcDistanceBetweenRayAndPoint(const TRay<T>& line, const T& point)
	{
		const T dir = point - line.origin;

		const F32 cosTheta        = Dot(Normalize(dir), line.dir);
		const F32 dirLength       = Length(dir);
		const F32 projDirOntoLine = Length(line.dir * (cosTheta * dirLength));

		return sqrtf(dirLength * dirLength - projDirOntoLine * projDirOntoLine);
	}


	/*!
		\brief The function returns shortest distance between two lines

		\param[in] line1 
		\param[in] line2

		\return The function returns a tuple which consists of the following values: a distance between two lines and two parameters of given rays
	*/

	template <typename T>
	std::tuple<F32, F32, F32> CalcShortestDistanceBetweenLines(const TRay<T>& line1, const TRay<T>& line2)
	{
		T dir = line2.origin - line1.origin;

		// \note all formulas below assume that line1.dir and line2.dir are normalized so their lengths equal to 1
		const F32 v1v2 = Dot(line1.dir, line2.dir);

		const F32 det = v1v2 * v1v2 - 1.0f; 

		// \note lines are parallel
		if (CMathUtils::IsLessOrEqual(std::abs(det), (std::numeric_limits<F32>::min)()))
		{
			return { CalcDistanceBetweenRayAndPoint<T>(line1, line2.origin), 0.0f, 0.0f };
		}

		const F32 invDet = 1.0f / det;

		const F32 dotDirV1 = Dot(dir, line1.dir);
		const F32 dotDirV2 = Dot(dir, line2.dir);

		const F32 t1 = invDet * (v1v2 * dotDirV2 - dotDirV1);
		const F32 t2 = invDet * (dotDirV2 - v1v2 * dotDirV1);

		const T a = dir + line2.dir * t2 - line1.dir * t1;
		return { Length(a), t1, t2 };
	}


	/*!
		\brief The function checks whether ray and plane intersect each other

		\return Returns true if ray intersects given plane and t parameter for the ray, false in other case
	*/

	template <typename T>
	std::tuple<bool, F32> CheckRayPlaneIntersection(const TRay<T>& ray, const TPlane<F32>& plane)
	{
		const T planeNormal{ plane.a, plane.b, plane.c };

		const F32 dotProduct = Dot(planeNormal, ray.dir);
		const F32 numerator  = Dot(planeNormal, ray.origin) + plane.d;

		if (std::abs(dotProduct) < 1e-3f)
		{
			return { std::abs(numerator) < 1e-3f, 0.0f };
		}

		const F32 t = -numerator / dotProduct;

		return { CMathUtils::IsGreatOrEqual(t, 0.0f), t };
	}
}