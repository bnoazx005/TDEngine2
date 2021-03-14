#include "../../../include/graphics/animation/CAnimationCurve.h"
#include "../../../include/math/MathUtils.h"
#include <algorithm>


namespace TDEngine2
{
	CAnimationCurve::CAnimationCurve() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CAnimationCurve::Init(const TRectF32& bounds)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mBounds = bounds;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CAnimationCurve::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		--mRefCounter;

		if (!mRefCounter)
		{
			delete this;
		}

		return RC_OK;
	}

	E_RESULT_CODE CAnimationCurve::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}
	
	E_RESULT_CODE CAnimationCurve::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}

	E_RESULT_CODE CAnimationCurve::AddPoint(const TKeyFrame& point)
	{
		// \note Check up if there is same point already
		auto it = std::find_if(mPoints.cbegin(), mPoints.cend(), [t = point.mTime](const TKeyFrame& other) { return CMathUtils::Abs(other.mTime - t) < 1e-3f; });
		if (it != mPoints.cend())
		{
			return RC_FAIL;
		}

		// \note Insert a new point, but keep mPoints array in sorted state
		mPoints.insert(std::find_if(mPoints.cbegin(), mPoints.cend(), [t = point.mTime](const TKeyFrame& other) { return other.mTime > t; }), point);
		
		// \note Update boundaries of the curve

		return RC_OK;
	}

	E_RESULT_CODE CAnimationCurve::RemovePoint(U32 index)
	{
		if (index >= static_cast<U32>(mPoints.size()))
		{
			return RC_INVALID_ARGS;
		}

		mPoints.erase(mPoints.cbegin() + index);

		return RC_OK;
	}

	F32 CAnimationCurve::Sample(F32 t) const
	{
		if (mPoints.empty())
		{
			return 0.0f;
		}

		return 0.0f;
	}

	const TRectF32& CAnimationCurve::GetBounds() const
	{
		return mBounds;
	}

	U32 CAnimationCurve::_getFrameIndexByTime(F32 time) const
	{
		if (mPoints.size() <= 1)
		{
			return -1;
		}

		F32 t = time;

		const F32 startTime = mPoints.front().mTime;

		if (CMathUtils::IsLessOrEqual(time, startTime))
		{
			return 0;
		}

		if (CMathUtils::IsGreatOrEqual(time, mPoints[mPoints.size() - 2].mTime))
		{
			return static_cast<I32>(mPoints.size()) - 2;
		}

		for (auto it = mPoints.rbegin(); it != mPoints.rend(); ++it)
		{
			if (CMathUtils::IsGreatOrEqual(time, it->mTime))
			{
				return static_cast<I32>(std::distance(mPoints.rend(), it));
			}
		}

		TDE2_UNREACHABLE();
		return -1;
	}

	F32 CAnimationCurve::_adjustTrackTime(F32 time) const
	{
		if (mPoints.size() <= 1)
		{
			return 0.0f;
		}

		const F32 startTime = mPoints.front().mTime;
		const F32 endTime = mPoints[mPoints.size() - 1].mTime;
		const F32 duration = endTime - startTime;

		if (CMathUtils::IsLessOrEqual(duration, 0.0f))
		{
			return 0.0f;
		}

		if (CMathUtils::IsLessOrEqual(time, startTime))
		{
			return startTime;
		}

		if (CMathUtils::IsGreatOrEqual(time, mPoints[mPoints.size() - 1].mTime))
		{
			return endTime;
		}

		return time;
	}


	CAnimationCurve* CreateAnimationCurve(const TRectF32& bounds, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CAnimationCurve, CAnimationCurve, result, bounds);
	}
}