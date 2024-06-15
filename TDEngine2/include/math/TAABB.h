/*!
	/file TAABB.h
	/date 29.01.2020
	/authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../utils/Config.h"
#include "./../math/TVector3.h"


namespace TDEngine2
{
	/*!
		struct TAABB

		\brief The structure represents an axis aligned bounding box
	*/

	struct TAABB
	{
		TVector3 min;
		TVector3 max;

		TDE2_API TAABB() = default;
		TDE2_API TAABB(const TAABB& aabb) = default;
		TDE2_API TAABB(TAABB&& aabb) = default;

		/*!
			\brief The constructor defines AABB based on two points

			\param[in] _min 
			\param[in] _max
		*/

		TDE2_API TAABB(const TVector3& _min, const TVector3& _max);

		/*!
			\brief The constructor defines AABB based on a point and desired
			width and height of the box

			\param[in] center A pointer which will be a center of the box
			\param[in] width A width of the box
			\param[in] height A height of the box
			\param[in] depth A depth of the box
		*/

		TDE2_API TAABB(const TVector3& center, F32 width, F32 height, F32 depth);

		TDE2_API TAABB& operator= (const TAABB& aabb) = default;
		TDE2_API TAABB& operator= (TAABB&& aabb) = default;

		TDE2_API F32 GetVolume() const;
	};


	/*!
		\brief The function determines whether a point stays inside of an AABB or not

		\param[in] aabb An axis aligned bounding box for the test
		\param[in] point A tested point
	*/

	TDE2_API bool ContainsPoint(const TAABB& aabb, const TVector3& point);

	/*!
		\brief The function returns true if aabb1 stays inside the volume aabb0
	*/

	TDE2_API bool ContainsAABB(const TAABB& aabb0, const TAABB& aabb1);

	TDE2_API bool ContainsSphere(const TAABB& aabb, const TVector3& sphereCenter, F32 sphereRadius);

	TDE2_API TAABB UnionBoundingBoxes(const TAABB& left, const TAABB& right);

}
