#include "../../include/graphics/animation/AnimationTracks.h"
#include "../../include/core/Meta.h"
#include "../../include/utils/Utils.h"


namespace TDEngine2
{
	/*!
		\brief CVector2AnimationTrack's definition
	*/

	CVector2AnimationTrack::CVector2AnimationTrack() :
		CBaseAnimationTrack()
	{
	}

	E_RESULT_CODE CVector2AnimationTrack::Apply(IPropertyWrapper* pPropertyWrapper, F32 time)
	{
		if (!pPropertyWrapper)
		{
			return RC_INVALID_ARGS;
		}

		return pPropertyWrapper->Set<TVector2>(_sample(time).mValue);
	}

	E_RESULT_CODE CVector2AnimationTrack::AssignTrackForEditing(IAnimationTrackVisitor* pTrackEditor)
	{
		return pTrackEditor ? pTrackEditor->VisitVector2Track(this) : RC_FAIL;
	}

	E_RESULT_CODE CVector2AnimationTrack::_saveKeyFrameValue(const TVector2KeyFrame& value, IArchiveWriter* pWriter)
	{
		E_RESULT_CODE result = pWriter->BeginGroup("value");
		result = result | SaveVector2(pWriter, value.mValue);
		result = result | pWriter->EndGroup();

		return result;
	}

	E_RESULT_CODE CVector2AnimationTrack::_loadKeyFrameValue(TAnimationTrackKeyId keyHandle, IArchiveReader* pReader)
	{
		auto& frameValue = mKeys[mKeysHandlesMap[keyHandle]];

		pReader->BeginGroup("value");

		auto loadResult = LoadVector2(pReader);
		if (loadResult)
		{
			frameValue.mValue = loadResult.Get();
		}
		else
		{
			return loadResult.GetError();
		}

		pReader->EndGroup();

		return RC_OK;
	}

	TVector2KeyFrame CVector2AnimationTrack::_lerpKeyFrames(const TVector2KeyFrame& left, const TVector2KeyFrame& right, F32 t) const
	{
		return { CMathUtils::Lerp(left.mTime, right.mTime, t), left.mValue * t + right.mValue * (1.0f - t) };
	}

	TVector2KeyFrame CVector2AnimationTrack::_cubicInterpolation(const TVector2KeyFrame& left, const TVector2KeyFrame& right, F32 t, F32 frameDelta) const
	{
		return 
		{ 
			CMathUtils::Lerp(left.mTime, right.mTime, t), 
			{
				CMathUtils::CubicHermiteInterpolation(t, left.mValue.x, left.mOutTangents[0].y, right.mValue.x, right.mInTangents[0].y),
				CMathUtils::CubicHermiteInterpolation(t, left.mValue.y, left.mOutTangents[1].y, right.mValue.y, right.mInTangents[1].y)
			}
		};
	}


	TDE2_API IAnimationTrack* CreateVector2AnimationTrack(IAnimationClip* pOwner, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IAnimationTrack, CVector2AnimationTrack, result, pOwner);
	}


	/*!
		\brief CVector3AnimationTrack's definition
	*/

	CVector3AnimationTrack::CVector3AnimationTrack():
		CBaseAnimationTrack()
	{
	}

	E_RESULT_CODE CVector3AnimationTrack::Apply(IPropertyWrapper* pPropertyWrapper, F32 time)
	{
		if (!pPropertyWrapper)
		{
			return RC_INVALID_ARGS;
		}

		return pPropertyWrapper->Set<TVector3>(_sample(time).mValue);
	}

	E_RESULT_CODE CVector3AnimationTrack::AssignTrackForEditing(IAnimationTrackVisitor* pTrackEditor)
	{
		return pTrackEditor ? pTrackEditor->VisitVector3Track(this) : RC_FAIL;
	}

	E_RESULT_CODE CVector3AnimationTrack::_saveKeyFrameValue(const TVector3KeyFrame& value, IArchiveWriter* pWriter)
	{
		E_RESULT_CODE result = pWriter->BeginGroup("value");
		result = result | SaveVector3(pWriter, value.mValue);
		result = result | pWriter->EndGroup();

		return result;
	}

	E_RESULT_CODE CVector3AnimationTrack::_loadKeyFrameValue(TAnimationTrackKeyId keyHandle, IArchiveReader* pReader)
	{
		auto& frameValue = mKeys[mKeysHandlesMap[keyHandle]];

		pReader->BeginGroup("value");

		auto loadResult = LoadVector3(pReader);
		if (loadResult)
		{
			frameValue.mValue = loadResult.Get();
		}
		else
		{
			return loadResult.GetError();
		}

		pReader->EndGroup();

		return RC_OK;
	}

	TVector3KeyFrame CVector3AnimationTrack::_lerpKeyFrames(const TVector3KeyFrame& left, const TVector3KeyFrame& right, F32 t) const
	{
		return { CMathUtils::Lerp(left.mTime, right.mTime, t), Lerp(left.mValue, right.mValue, t) };
	}

	TVector3KeyFrame CVector3AnimationTrack::_cubicInterpolation(const TVector3KeyFrame& left, const TVector3KeyFrame& right, F32 t, F32 frameDelta) const
	{
		return
		{
			CMathUtils::Lerp(left.mTime, right.mTime, t),
			{
				CMathUtils::CubicHermiteInterpolation(t, left.mValue.x, left.mOutTangents[0].y, right.mValue.x, right.mInTangents[0].y),
				CMathUtils::CubicHermiteInterpolation(t, left.mValue.y, left.mOutTangents[1].y, right.mValue.y, right.mInTangents[1].y),
				CMathUtils::CubicHermiteInterpolation(t, left.mValue.z, left.mOutTangents[2].y, right.mValue.z, right.mInTangents[2].y),
			}
		};
	}


	TDE2_API IAnimationTrack* CreateVector3AnimationTrack(IAnimationClip* pOwner, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IAnimationTrack, CVector3AnimationTrack, result, pOwner);
	}


	/*!
		\brief CQuaternionAnimationTrack's definition
	*/


	CQuaternionAnimationTrack::CQuaternionAnimationTrack() :
		CBaseAnimationTrack()
	{
	}

	E_RESULT_CODE CQuaternionAnimationTrack::Apply(IPropertyWrapper* pPropertyWrapper, F32 time)
	{
		if (!pPropertyWrapper)
		{
			return RC_INVALID_ARGS;
		}

		return pPropertyWrapper->Set<TQuaternion>(_sample(time).mValue);
	}

	E_RESULT_CODE CQuaternionAnimationTrack::AssignTrackForEditing(IAnimationTrackVisitor* pTrackEditor)
	{
		return pTrackEditor ? pTrackEditor->VisitQuaternionTrack(this) : RC_FAIL;
	}

	E_RESULT_CODE CQuaternionAnimationTrack::_saveKeyFrameValue(const TQuaternionKeyFrame& value, IArchiveWriter* pWriter)
	{
		E_RESULT_CODE result = pWriter->BeginGroup("value");
		result = result | SaveQuaternion(pWriter, value.mValue);
		result = result | pWriter->EndGroup();

		return result;
	}

	E_RESULT_CODE CQuaternionAnimationTrack::_loadKeyFrameValue(TAnimationTrackKeyId keyHandle, IArchiveReader* pReader)
	{
		auto& frameValue = mKeys[mKeysHandlesMap[keyHandle]];

		pReader->BeginGroup("value");

		auto loadResult = LoadQuaternion(pReader);
		if (loadResult)
		{
			frameValue.mValue = loadResult.Get();
		}
		else
		{
			return loadResult.GetError();
		}

		pReader->EndGroup();

		return RC_OK;
	}

	TQuaternionKeyFrame CQuaternionAnimationTrack::_lerpKeyFrames(const TQuaternionKeyFrame& left, const TQuaternionKeyFrame& right, F32 t) const
	{
		return { CMathUtils::Lerp(left.mTime, right.mTime, t), Slerp(left.mValue, right.mValue, t) };
	}

	TQuaternionKeyFrame CQuaternionAnimationTrack::_cubicInterpolation(const TQuaternionKeyFrame& left, const TQuaternionKeyFrame& right, F32 t, F32 frameDelta) const
	{
		return
		{
			CMathUtils::Lerp(left.mTime, right.mTime, t),
			{
				CMathUtils::CubicHermiteInterpolation(t, left.mValue.x, left.mOutTangents[0].y, right.mValue.x, right.mInTangents[0].y),
				CMathUtils::CubicHermiteInterpolation(t, left.mValue.y, left.mOutTangents[1].y, right.mValue.y, right.mInTangents[1].y),
				CMathUtils::CubicHermiteInterpolation(t, left.mValue.z, left.mOutTangents[2].y, right.mValue.z, right.mInTangents[2].y),
				CMathUtils::CubicHermiteInterpolation(t, left.mValue.w, left.mOutTangents[3].y, right.mValue.w, right.mInTangents[3].y),
			}
		};
	}


	TDE2_API IAnimationTrack* CreateQuaternionAnimationTrack(IAnimationClip* pOwner, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IAnimationTrack, CQuaternionAnimationTrack, result, pOwner);
	}


	/*!
		\brief CColorAnimationTrack's definition
	*/

	CColorAnimationTrack::CColorAnimationTrack() :
		CBaseAnimationTrack()
	{
	}

	E_RESULT_CODE CColorAnimationTrack::Apply(IPropertyWrapper* pPropertyWrapper, F32 time)
	{
		if (!pPropertyWrapper)
		{
			return RC_INVALID_ARGS;
		}

		return pPropertyWrapper->Set<TColor32F>(_sample(time).mValue);
	}

	E_RESULT_CODE CColorAnimationTrack::AssignTrackForEditing(IAnimationTrackVisitor* pTrackEditor)
	{
		return pTrackEditor ? pTrackEditor->VisitColorTrack(this) : RC_FAIL;
	}

	E_RESULT_CODE CColorAnimationTrack::_saveKeyFrameValue(const TColorKeyFrame& value, IArchiveWriter* pWriter)
	{
		E_RESULT_CODE result = pWriter->BeginGroup("value");
		result = result | SaveColor32F(pWriter, value.mValue);
		result = result | pWriter->EndGroup();

		return result;
	}

	E_RESULT_CODE CColorAnimationTrack::_loadKeyFrameValue(TAnimationTrackKeyId keyHandle, IArchiveReader* pReader)
	{
		auto& frameValue = mKeys[mKeysHandlesMap[keyHandle]];

		pReader->BeginGroup("value");

		auto loadResult = LoadColor32F(pReader);
		if (loadResult)
		{
			frameValue.mValue = loadResult.Get();
		}
		else
		{
			return loadResult.GetError();
		}

		pReader->EndGroup();

		return RC_OK;
	}

	TColorKeyFrame CColorAnimationTrack::_lerpKeyFrames(const TColorKeyFrame& left, const TColorKeyFrame& right, F32 t) const
	{
		return { CMathUtils::Lerp(left.mTime, right.mTime, t), left.mValue/* * t + right.mValue * (1.0 - t)*/ };
	}

	TColorKeyFrame CColorAnimationTrack::_cubicInterpolation(const TColorKeyFrame& left, const TColorKeyFrame& right, F32 t, F32 frameDelta) const
	{
		TDE2_UNIMPLEMENTED();

		return
		{
			CMathUtils::Lerp(left.mTime, right.mTime, t),
			TColorUtils::mBlack //CMathUtils::CubicHermiteInterpolation<TColor>(t, left.mValue, left.mOutTangent * frameDelta, right.mValue, Normalize(right.mInTangent) * frameDelta)
		};
	}


	TDE2_API IAnimationTrack* CreateColorAnimationTrack(IAnimationClip* pOwner, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IAnimationTrack, CColorAnimationTrack, result, pOwner);
	}


	/*!
		\brief CBooleanAnimationTrack's definition
	*/

	CBooleanAnimationTrack::CBooleanAnimationTrack() :
		CBaseAnimationTrack()
	{
	}

	E_RESULT_CODE CBooleanAnimationTrack::Apply(IPropertyWrapper* pPropertyWrapper, F32 time)
	{
		if (!pPropertyWrapper)
		{
			return RC_INVALID_ARGS;
		}

		return pPropertyWrapper->Set<bool>(_sample(time).mValue);
	}

	E_RESULT_CODE CBooleanAnimationTrack::AssignTrackForEditing(IAnimationTrackVisitor* pTrackEditor)
	{
		return pTrackEditor ? pTrackEditor->VisitBooleanTrack(this) : RC_FAIL;
	}

	E_RESULT_CODE CBooleanAnimationTrack::_saveKeyFrameValue(const TBooleanKeyFrame& value, IArchiveWriter* pWriter)
	{
		return pWriter->SetBool("value", value.mValue);
	}

	E_RESULT_CODE CBooleanAnimationTrack::_loadKeyFrameValue(TAnimationTrackKeyId keyHandle, IArchiveReader* pReader)
	{
		auto& frameValue = mKeys[mKeysHandlesMap[keyHandle]];
		frameValue.mValue = pReader->GetBool("value");

		return RC_OK;
	}

	TBooleanKeyFrame CBooleanAnimationTrack::_lerpKeyFrames(const TBooleanKeyFrame& left, const TBooleanKeyFrame& right, F32 t) const
	{
		return { CMathUtils::Lerp(left.mTime, right.mTime, t), CMathUtils::Lerp(left.mValue, left.mValue, t) };
	}

	TBooleanKeyFrame CBooleanAnimationTrack::_cubicInterpolation(const TBooleanKeyFrame& left, const TBooleanKeyFrame& right, F32 t, F32 frameDelta) const
	{
		TDE2_UNREACHABLE();

		return
		{
			CMathUtils::Lerp(left.mTime, right.mTime, t),
			false
		};
	}


	TDE2_API IAnimationTrack* CreateBooleanAnimationTrack(IAnimationClip* pOwner, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IAnimationTrack, CBooleanAnimationTrack, result, pOwner);
	}


	/*!
		\brief CFloatAnimatioNTrack's definition
	*/

	CFloatAnimationTrack::CFloatAnimationTrack() :
		CBaseAnimationTrack()
	{
	}

	E_RESULT_CODE CFloatAnimationTrack::Apply(IPropertyWrapper* pPropertyWrapper, F32 time)
	{
		if (!pPropertyWrapper)
		{
			return RC_INVALID_ARGS;
		}

		return pPropertyWrapper->Set<F32>(_sample(time).mValue);
	}

	E_RESULT_CODE CFloatAnimationTrack::AssignTrackForEditing(IAnimationTrackVisitor* pTrackEditor)
	{
		return pTrackEditor ? pTrackEditor->VisitFloatTrack(this) : RC_FAIL;
	}

	E_RESULT_CODE CFloatAnimationTrack::_saveKeyFrameValue(const TFloatKeyFrame& value, IArchiveWriter* pWriter)
	{
		return pWriter->SetFloat("value", value.mValue);
	}

	E_RESULT_CODE CFloatAnimationTrack::_loadKeyFrameValue(TAnimationTrackKeyId keyHandle, IArchiveReader* pReader)
	{
		auto& frameValue = mKeys[mKeysHandlesMap[keyHandle]];
		frameValue.mValue = pReader->GetFloat("value");

		return RC_OK;
	}

	TFloatKeyFrame CFloatAnimationTrack::_lerpKeyFrames(const TFloatKeyFrame& left, const TFloatKeyFrame& right, F32 t) const
	{
		return { CMathUtils::Lerp(left.mTime, right.mTime, t), CMathUtils::Lerp(left.mValue, left.mValue, t) };
	}

	TFloatKeyFrame CFloatAnimationTrack::_cubicInterpolation(const TFloatKeyFrame& left, const TFloatKeyFrame& right, F32 t, F32 frameDelta) const
	{
		TDE2_UNIMPLEMENTED();
		return
		{
			CMathUtils::Lerp(left.mTime, right.mTime, t),
			0.0f//CMathUtils::CubicHermiteInterpolation<F32>(t, left.mValue, left.mOutTangent * frameDelta, right.mValue, Normalize(right.mInTangent) * frameDelta)
		};
	}


	TDE2_API IAnimationTrack* CreateFloatAnimationTrack(IAnimationClip* pOwner, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IAnimationTrack, CFloatAnimationTrack, result, pOwner);
	}


	/*!
		\brief CIntegerAnimatioNTrack's definition
	*/

	CIntegerAnimationTrack::CIntegerAnimationTrack() :
		CBaseAnimationTrack()
	{
	}

	E_RESULT_CODE CIntegerAnimationTrack::Apply(IPropertyWrapper* pPropertyWrapper, F32 time)
	{
		if (!pPropertyWrapper)
		{
			return RC_INVALID_ARGS;
		}

		return pPropertyWrapper->Set<I32>(_sample(time).mValue);
	}

	E_RESULT_CODE CIntegerAnimationTrack::AssignTrackForEditing(IAnimationTrackVisitor* pTrackEditor)
	{
		return pTrackEditor ? pTrackEditor->VisitIntegerTrack(this) : RC_FAIL;
	}

	E_RESULT_CODE CIntegerAnimationTrack::_saveKeyFrameValue(const TIntegerKeyFrame& value, IArchiveWriter* pWriter)
	{
		return pWriter->SetInt32("value", value.mValue);
	}

	E_RESULT_CODE CIntegerAnimationTrack::_loadKeyFrameValue(TAnimationTrackKeyId keyHandle, IArchiveReader* pReader)
	{
		auto& frameValue = mKeys[mKeysHandlesMap[keyHandle]];
		frameValue.mValue = pReader->GetInt32("value");

		return RC_OK;
	}

	TIntegerKeyFrame CIntegerAnimationTrack::_lerpKeyFrames(const TIntegerKeyFrame& left, const TIntegerKeyFrame& right, F32 t) const
	{
		return { CMathUtils::Lerp(left.mTime, right.mTime, t), CMathUtils::Lerp(left.mValue, left.mValue, t) };
	}

	TIntegerKeyFrame CIntegerAnimationTrack::_cubicInterpolation(const TIntegerKeyFrame& left, const TIntegerKeyFrame& right, F32 t, F32 frameDelta) const
	{
		TDE2_UNREACHABLE();
		return
		{
			CMathUtils::Lerp(left.mTime, right.mTime, t),
			0 //CMathUtils::CubicHermiteInterpolation<F32>(t, left.mValue, left.mOutTangent * frameDelta, right.mValue, Normalize(right.mInTangent) * frameDelta)
		};
	}


	TDE2_API IAnimationTrack* CreateIntegerAnimationTrack(IAnimationClip* pOwner, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IAnimationTrack, CIntegerAnimationTrack, result, pOwner);
	}


	/*!
		\brief CEventAnimationTrack's definition
	*/

	CEventAnimationTrack::CEventAnimationTrack() :
		CBaseAnimationTrack()
	{
	}

	E_RESULT_CODE CEventAnimationTrack::Apply(IPropertyWrapper* pPropertyWrapper, F32 time)
	{
		if (!pPropertyWrapper)
		{
			return RC_INVALID_ARGS;
		}

		auto&& frameData = _sample(time);
		if (frameData.mValue.empty())
		{
			return RC_OK;
		}

		if (std::abs(frameData.mTime - time) < 1e-2f)
		{
			return pPropertyWrapper->Set<std::string>(frameData.mValue);
		}

		return RC_OK;
	}

	E_RESULT_CODE CEventAnimationTrack::AssignTrackForEditing(IAnimationTrackVisitor* pTrackEditor)
	{
		return pTrackEditor ? pTrackEditor->VisitEventTrack(this) : RC_FAIL;
	}

	E_RESULT_CODE CEventAnimationTrack::_saveKeyFrameValue(const TEventKeyFrame& value, IArchiveWriter* pWriter)
	{
		return pWriter->SetString("value", value.mValue);
	}

	E_RESULT_CODE CEventAnimationTrack::_loadKeyFrameValue(TAnimationTrackKeyId keyHandle, IArchiveReader* pReader)
	{
		auto& frameValue = mKeys[mKeysHandlesMap[keyHandle]];
		frameValue.mValue = pReader->GetString("value");

		return RC_OK;
	}

	TEventKeyFrame CEventAnimationTrack::_lerpKeyFrames(const TEventKeyFrame& left, const TEventKeyFrame& right, F32 t) const
	{
		if (std::abs(t) < 1e-3f)
		{
			return left;
		}

		if (std::abs(t - 1.0f) < 1e-3f)
		{
			return right;
		}

		return TEventKeyFrame();
	}

	TEventKeyFrame CEventAnimationTrack::_cubicInterpolation(const TEventKeyFrame& left, const TEventKeyFrame& right, F32 t, F32 frameDelta) const
	{
		TDE2_UNREACHABLE();
		return
		{
			CMathUtils::Lerp(left.mTime, right.mTime, t),
			"" //CMathUtils::CubicHermiteInterpolation<F32>(t, left.mValue, left.mOutTangent * frameDelta, right.mValue, Normalize(right.mInTangent) * frameDelta)
		};
	}


	TDE2_API IAnimationTrack* CreateEventAnimationTrack(IAnimationClip* pOwner, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IAnimationTrack, CEventAnimationTrack, result, pOwner);
	}
}