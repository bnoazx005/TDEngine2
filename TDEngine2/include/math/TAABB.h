/*!
	/file TAABB.h
	/date 29.01.2020
	/authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../utils/Config.h"
#include "../math/TVector3.h"


namespace TDEngine2
{
	/*!
		struct TAABB

		\brief The structure represents an axis aligned bounding box
	*/

	struct TAABB
	{
		TVector3 min = TVector3{};
		TVector3 max = TVector3{};

		TDE2_API constexpr TAABB() = default;
		TDE2_API constexpr TAABB(const TAABB& aabb) = default;
		TDE2_API constexpr TAABB(TAABB&& aabb) = default;

		/*!
			\brief The constructor defines AABB based on two points

			\param[in] _min 
			\param[in] _max
		*/

		TDE2_API constexpr TAABB(const TVector3& _min, const TVector3& _max)
		{
			this->min = _min;
			this->max = _max;
		}

		/*!
			\brief The constructor defines AABB based on a point and desired
			width and height of the box

			\param[in] center A pointer which will be a center of the box
			\param[in] width A width of the box
			\param[in] height A height of the box
			\param[in] depth A depth of the box
		*/

		TDE2_API constexpr TAABB(const TVector3& center, F32 width, F32 height, F32 depth)
		{
			F32 halfWidth = 0.5f * width;
			F32 halfHeight = 0.5f * height;
			F32 halfDepth = 0.5f * depth;

			this->min = TVector3(center.x - halfHeight, center.y - halfHeight, center.z - halfDepth);
			this->max = TVector3(center.x + halfHeight, center.y + halfHeight, center.z + halfDepth);
		}

		TDE2_API constexpr TAABB& operator= (const TAABB& aabb) = default;
		TDE2_API constexpr TAABB& operator= (TAABB&& aabb) = default;

		TDE2_API constexpr F32 GetVolume() const
		{
			TVector3 diag = max - min;
			return CMathUtils::Abs(diag.x * diag.y * diag.z);
		}
	};


	static_assert(std::is_trivially_copyable_v<TAABB>, "TAABB should be trivially copyable");

	
	TDE2_API bool operator== (const TAABB& left, const TAABB& right);
	TDE2_API bool operator!= (const TAABB& left, const TAABB& right);


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
