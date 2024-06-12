/*!
	/file TPlane.h
	/date 28.01.2020
	/authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../utils/Config.h"
#include "./../math/TVector3.h"


namespace TDEngine2
{
	/*!
		struct TPlane

		\brief The structure represents 3D plane
	*/

	template <typename T>
	struct TPlane
	{
		T a, b, c, d;

		TDE2_API TPlane() = default;
		TDE2_API TPlane(const TPlane& plane) = default;
		TDE2_API TPlane(TPlane&& plane) = default;

		/*!
			\brief The constructor which defines a plane based on its normal and distance's value

			\param[in] normal A 3d unit-length vector which defines a direction and front-face of the plane
			\param[in] distance A value on which the plane is stands off from the origin
		*/

		TDE2_API TPlane(const TVector3& normal, F32 distance):
			a(normal.x), b(normal.y), c(normal.z), d(distance)
		{
		}

		/*!
			\brief The constructor defines a plane based on three points that lie on it

			\param[in] points An array of three 3d points
		*/

		TDE2_API TPlane(TVector3 points[3])
		{
			TVector3 e1 = points[1] - points[0];
			TVector3 e2 = points[2] - points[0];

			TVector3 n = Normalize(Cross(e2, e1));

			a = n.x;
			b = n.y;
			c = n.z;
			d = Dot(n, points[0]);
		}

		TDE2_API TPlane& operator= (const TPlane& plane) = default;
		TDE2_API TPlane& operator= (TPlane&& plane) = default;
	};


	typedef TPlane<F32> TPlaneF32;


	/*!
		\brief The function returns distance from plane to given point. The zero value means
		that point lies on the plane; negative value is that the point stands behind it

		\return The function returns distance from plane to given point
	*/

	TDE2_API F32 CalcDistanceFromPlaneToPoint(const TPlaneF32& plane, const TVector3& point);
}
