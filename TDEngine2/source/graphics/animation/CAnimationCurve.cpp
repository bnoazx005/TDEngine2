#include "../../../include/graphics/animation/CAnimationCurve.h"
#include "../../../include/math/MathUtils.h"
#include <algorithm>


namespace TDEngine2
{
	struct TAnimationCurveKeys
	{
		const static std::string mPointsKeyId;
		const static std::string mTimeKeyId;
		const static std::string mValueKeyId;
		const static std::string mInTangentKeyId;
		const static std::string mOutTangentKeyId;
	};

	const std::string TAnimationCurveKeys::mPointsKeyId = "points";
	const std::string TAnimationCurveKeys::mTimeKeyId = "time";
	const std::string TAnimationCurveKeys::mValueKeyId = "value";
	const std::string TAnimationCurveKeys::mInTangentKeyId = "in_tangent";
	const std::string TAnimationCurveKeys::mOutTangentKeyId = "out_tangent";


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

		F32 time, value;
		TVector2 in, out;

		pReader->BeginGroup(TAnimationCurveKeys::mPointsKeyId);
		
		while (pReader->HasNextItem())
		{
			pReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());
			
			time = pReader->GetFloat(TAnimationCurveKeys::mTimeKeyId);
			value = pReader->GetFloat(TAnimationCurveKeys::mValueKeyId);

			pReader->BeginGroup(TAnimationCurveKeys::mInTangentKeyId);

			auto inTangentResult = LoadVector2(pReader);
			if (inTangentResult.HasError())
			{
				return inTangentResult.GetError();
			}

			in = inTangentResult.Get();

			pReader->EndGroup();

			pReader->BeginGroup(TAnimationCurveKeys::mOutTangentKeyId);

			auto outTangentResult = LoadVector2(pReader);
			if (outTangentResult.HasError())
			{
				return outTangentResult.GetError();
			}

			out = outTangentResult.Get();

			pReader->EndGroup();

			pReader->EndGroup();

			AddPoint({ time, value, in, out });
		}
		
		pReader->EndGroup();

		if (mPoints.empty())
		{
			// \note Use default parameters for the curve with no points
			AddPoint({ 0.0f, 0.0f, -0.5f * RightVector2, 0.5f * RightVector2 });
			AddPoint({ 1.0f, 1.0f, -0.5f * RightVector2, 0.5f * RightVector2 });
		}

		return _updateBounds();
	}
	
	E_RESULT_CODE CAnimationCurve::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_INVALID_ARGS;
		}

		pWriter->BeginGroup(TAnimationCurveKeys::mPointsKeyId, true);

		for (auto&& currPoint : mPoints)
		{
			pWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr());

			pWriter->SetFloat(TAnimationCurveKeys::mTimeKeyId, currPoint.mTime);
			pWriter->SetFloat(TAnimationCurveKeys::mValueKeyId, currPoint.mValue);

			pWriter->BeginGroup(TAnimationCurveKeys::mInTangentKeyId);
			SaveVector2(pWriter, currPoint.mInTangent);
			pWriter->EndGroup();

			pWriter->BeginGroup(TAnimationCurveKeys::mOutTangentKeyId);
			SaveVector2(pWriter, currPoint.mOutTangent);
			pWriter->EndGroup();

			pWriter->EndGroup();
		}

		pWriter->EndGroup();

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
		
		return _updateBounds(); // \note Update boundaries of the curve
	}

	E_RESULT_CODE CAnimationCurve::RemovePoint(U32 index)
	{
		if (index >= static_cast<U32>(mPoints.size()))
		{
			return RC_INVALID_ARGS;
		}

		mPoints.erase(mPoints.cbegin() + index);

		return _updateBounds();
	}

	F32 CAnimationCurve::Sample(F32 t) const
	{
		if (mPoints.empty())
		{
			return 0.0f;
		}

		const U32 index = _getFrameIndexByTime(t);
		if (index < 0 || index >= static_cast<U32>(mPoints.size()) - 1)
		{
			return index >= 0 ? mPoints.back().mValue : 0.0f;
		}

		const I32 nextIndex = index + 1;

		const TKeyFrame& currPoint = mPoints[index];
		const TKeyFrame& nextPoint = mPoints[nextIndex];

		const F32 trackTime = _adjustTrackTime(t);
		const F32 thisTime = currPoint.mTime;

		const F32 frameDelta = nextPoint.mTime - thisTime;

		if (CMathUtils::IsLessOrEqual(frameDelta, 0.0f))
		{
			return 0.0f;
		}

		return CMathUtils::CubicBezierInterpolation<TVector2>((trackTime - thisTime) / frameDelta, 
															  { currPoint.mTime, currPoint.mValue }, currPoint.mOutTangent, 
															  { nextPoint.mTime, nextPoint.mValue }, nextPoint.mInTangent).y;
	}

	const TRectF32& CAnimationCurve::GetBounds() const
	{
		return mBounds;
	}

	CAnimationCurve::TKeyFrame* CAnimationCurve::GetPoint(U32 index)
	{
		if (index >= static_cast<U32>(mPoints.size()))
		{
			return nullptr;
		}

		return &mPoints[index];
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

		if (CMathUtils::IsGreatOrEqual(time, mPoints.back().mTime))
		{
			return static_cast<U32>(mPoints.size()) - 1;
		}

		for (auto it = mPoints.rbegin(); it != mPoints.rend(); ++it)
		{
			if (CMathUtils::IsGreatOrEqual(time, it->mTime))
			{
				return static_cast<U32>(std::distance(it, mPoints.rend())) - 1;
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

	E_RESULT_CODE CAnimationCurve::_updateBounds()
	{
		TVector2 min{ (std::numeric_limits<F32>::max)() };
		TVector2 max{ -(std::numeric_limits<F32>::max)() };

		for (auto&& currPoint : mPoints)
		{
			min.x = std::min<F32>(min.x, currPoint.mTime);
			max.x = std::max<F32>(max.x, currPoint.mTime);

			min.y = std::min<F32>(min.y, currPoint.mValue);
			max.y = std::max<F32>(max.y, currPoint.mValue);
		}

		mBounds.x      = min.x;
		mBounds.y      = min.y;
		mBounds.width  = max.x - min.x;
		mBounds.height = max.y - min.y;

		return RC_OK;
	}


	CAnimationCurve* CreateAnimationCurve(const TRectF32& bounds, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CAnimationCurve, CAnimationCurve, result, bounds);
	}
}