#include "./../../include/math/TAABB.h"
#include "./../../include/math/MathUtils.h"


namespace TDEngine2
{
	TAABB::TAABB(const TVector3& _min, const TVector3& _max)
	{
		this->min = _min;
		this->max = _max;
	}

	TAABB::TAABB(const TVector3& center, F32 width, F32 height, F32 depth)
	{
		F32 halfWidth  = 0.5f * width;
		F32 halfHeight = 0.5f * height;
		F32 halfDepth  = 0.5f * depth;

		this->min = TVector3(center.x - halfHeight, center.y - halfHeight, center.z - halfDepth);
		this->max = TVector3(center.x + halfHeight, center.y + halfHeight, center.z + halfDepth);
	}

	bool ContainsPoint(const TAABB& aabb, const TVector3& point)
	{
		const TVector3& min = aabb.min;
		const TVector3& max = aabb.max;

		if (CMathUtils::IsGreatOrEqual(min.x, point.x) && CMathUtils::IsLessOrEqual(max.x, point.x) &&
			CMathUtils::IsGreatOrEqual(min.y, point.y) && CMathUtils::IsLessOrEqual(max.y, point.y) &&
			CMathUtils::IsGreatOrEqual(min.z, point.z) && CMathUtils::IsLessOrEqual(max.z, point.z))
		{
			return true;
		}

		return false;
	}
}