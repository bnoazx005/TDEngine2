#include "../../include/utils/CGradientColor.h"
#include "../../include/math/MathUtils.h"
#include "stringUtils.hpp"
#include <algorithm>


namespace TDEngine2
{
	struct TGradientColorKeys
	{
		const static std::string mPointsKeyId;
		const static std::string mTimeKeyId;
		const static std::string mValueKeyId;
	};

	const std::string TGradientColorKeys::mPointsKeyId = "points";
	const std::string TGradientColorKeys::mTimeKeyId = "x";
	const std::string TGradientColorKeys::mValueKeyId = "color";


	CGradientColor::CGradientColor() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CGradientColor::Init(const TColor32F& leftBoundColor, const TColor32F& rightBoundColor)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = _addPointInternal({ 0.0f, leftBoundColor });
		result = result | _addPointInternal({ 1.0f, rightBoundColor });

		if (RC_OK != result)
		{
			return result;
		}
		
		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CGradientColor::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_INVALID_ARGS;
		}

		mPoints.clear();

		F32 time;
		
		TColor32F value;

		pReader->BeginGroup(TGradientColorKeys::mPointsKeyId);

		while (pReader->HasNextItem())
		{
			pReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());
			{
				time = pReader->GetFloat(TGradientColorKeys::mTimeKeyId);

				pReader->BeginGroup(TGradientColorKeys::mValueKeyId);
				{
					auto colorResult = LoadColor32F(pReader);
					if (colorResult.HasError())
					{
						return colorResult.GetError();
					}

					value = colorResult.Get();
				}
				pReader->EndGroup();
			}
			pReader->EndGroup();

			AddPoint({ time, value });
		}

		pReader->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CGradientColor::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_INVALID_ARGS;
		}

		pWriter->BeginGroup(TGradientColorKeys::mPointsKeyId, true);

		for (auto&& currPoint : mPoints)
		{
			pWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr());

			pWriter->SetFloat(TGradientColorKeys::mTimeKeyId, std::get<F32>(currPoint));

			pWriter->BeginGroup(TGradientColorKeys::mValueKeyId);
			SaveColor32F(pWriter, std::get<TColor32F>(currPoint));
			pWriter->EndGroup();

			pWriter->EndGroup();
		}

		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CGradientColor::AddPoint(const TColorSample& point)
	{
		return _addPointInternal(point);
	}

	E_RESULT_CODE CGradientColor::RemovePoint(U32 index)
	{
		if (mPoints.size() <= 2)
		{
			return RC_FAIL; // \note The object should contain at least two points
		}

		if (index >= static_cast<U32>(mPoints.size()))
		{
			return RC_INVALID_ARGS;
		}

		mPoints.erase(mPoints.cbegin() + index);

		return RC_OK;
	}

	TColor32F CGradientColor::Sample(F32 t) const
	{
		if (mPoints.empty())
		{
			return TColorUtils::mWhite;
		}

		const I32 index = _getFrameIndexByTime(t);
		if (index < 0 || index >= static_cast<I32>(mPoints.size()) - 1)
		{
			return index >= 0 ? std::get<TColor32F>(mPoints.back()) : std::get<TColor32F>(mPoints.front());
		}

		const I32 nextIndex = index + 1;

		const TColorSample& currPoint = mPoints[index];
		const TColorSample& nextPoint = mPoints[nextIndex];

		const F32 thisTime = std::get<F32>(currPoint);

		const F32 frameDelta = std::get<F32>(nextPoint) - thisTime;

		if (CMathUtils::IsLessOrEqual(frameDelta, 0.0f))
		{
			return TColorUtils::mWhite;
		}

		return LerpColors(std::get<TColor32F>(currPoint), std::get<TColor32F>(nextPoint), (t - thisTime) / frameDelta);
	}

	E_RESULT_CODE CGradientColor::SetColor(U32 index, F32 t, const TColor32F& color)
	{
		if (index >= static_cast<U32>(mPoints.size()))
		{
			return RC_INVALID_ARGS;
		}

		t = CMathUtils::Clamp01(t);

		std::get<F32>(mPoints[index]) = t;
		std::get<TColor32F>(mPoints[index]) = color;

		return RC_OK;
	}

	CGradientColor::TColorSample* CGradientColor::GetPoint(U32 index)
	{
		if (index >= static_cast<U32>(mPoints.size()))
		{
			return nullptr;
		}

		return &mPoints[index];
	}

	E_RESULT_CODE CGradientColor::_addPointInternal(const TColorSample& point)
	{
		// \note Check up if there is same point already
		auto it = std::find_if(mPoints.cbegin(), mPoints.cend(), [t = std::get<F32>(point)](const TColorSample& other) { return CMathUtils::Abs(std::get<F32>(other) - t) < 1e-3f; });
		if (it != mPoints.cend())
		{
			return RC_FAIL;
		}

		// \note Insert a new point, but keep mPoints array in sorted state
		mPoints.insert(std::find_if(mPoints.cbegin(), mPoints.cend(), [t = std::get<F32>(point)](const TColorSample& other) { return std::get<F32>(other) > t; }), point);

		return RC_OK;
	}

	I32 CGradientColor::_getFrameIndexByTime(F32 t) const
	{
		if (mPoints.size() <= 1)
		{
			return -1;
		}

		F32 time = t;

		const F32 startTime = std::get<F32>(mPoints.front());

		if (CMathUtils::IsLessOrEqual(time, startTime))
		{
			return (time < startTime) ? -1 : 0;
		}

		if (CMathUtils::IsGreatOrEqual(time, std::get<F32>(mPoints.back())))
		{
			return static_cast<U32>(mPoints.size()) - 1;
		}

		for (auto it = mPoints.rbegin(); it != mPoints.rend(); ++it)
		{
			if (CMathUtils::IsGreatOrEqual(time, std::get<F32>(*it)))
			{
				return static_cast<U32>(std::distance(it, mPoints.rend())) - 1;
			}
		}

		TDE2_UNREACHABLE();
		return -1;
	}

	void CGradientColor::SortPoints()
	{
		std::sort(mPoints.begin(), mPoints.end(), [](const TColorSample& left, const TColorSample& right) { return std::get<F32>(left) < std::get<F32>(right); });
	}


	CGradientColor* CreateGradientColor(const TColor32F& leftBoundColor, const TColor32F& rightBoundColor, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CGradientColor, CGradientColor, result, leftBoundColor, rightBoundColor);
	}
}