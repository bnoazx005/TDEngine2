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

	F32 TAABB::GetVolume() const
	{
		auto&& diag = max - min;
		return CMathUtils::Abs(diag.x * diag.y * diag.z);
	}


	bool ContainsPoint(const TAABB& aabb, const TVector3& point)
	{
		const TVector3& min = aabb.min;
		const TVector3& max = aabb.max;

		if (CMathUtils::IsGreatOrEqual(point.x, min.x) && CMathUtils::IsLessOrEqual(point.x, max.x) &&
			CMathUtils::IsGreatOrEqual(point.y, min.y) && CMathUtils::IsLessOrEqual(point.y, max.y) &&
			CMathUtils::IsGreatOrEqual(point.z, min.z) && CMathUtils::IsLessOrEqual(point.z, max.z))
		{
			return true;
		}

		return false;
	}


	TAABB UnionBoundingBoxes(const TAABB& left, const TAABB& right)
	{
		auto minVec3 = [](const TVector3& lvec3, const TVector3& rvec3)
		{
			return TVector3(std::min<F32>(lvec3.x, rvec3.x), std::min<F32>(lvec3.y, rvec3.y), std::min<F32>(lvec3.z, rvec3.z));
		};

		auto maxVec3 = [](const TVector3& lvec3, const TVector3& rvec3)
		{
			return TVector3(std::max<F32>(lvec3.x, rvec3.x), std::max<F32>(lvec3.y, rvec3.y), std::max<F32>(lvec3.z, rvec3.z));
		};

		return TAABB(minVec3(left.min, right.min), maxVec3(left.max, right.max));
	}
}