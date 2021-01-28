#include "../../include/graphics/animation/AnimationTracks.h"


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
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CVector2AnimationTrack::Load(IArchiveReader* pReader)
	{
		E_RESULT_CODE result = CBaseAnimationTrack<TVector2KeyFrame>::Load(pReader);

		return result;
	}

	E_RESULT_CODE CVector2AnimationTrack::_saveKeyFrameValue(const TVector2KeyFrame& value, IArchiveWriter* pWriter)
	{
		E_RESULT_CODE result = pWriter->BeginGroup("value");
		result = result | SaveVector2(pWriter, value.mValue);
		result = result | pWriter->EndGroup();

		return result;
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
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CVector3AnimationTrack::Load(IArchiveReader* pReader)
	{
		E_RESULT_CODE result = CBaseAnimationTrack<TVector3KeyFrame>::Load(pReader);

		return result;
	}

	E_RESULT_CODE CVector3AnimationTrack::_saveKeyFrameValue(const TVector3KeyFrame& value, IArchiveWriter* pWriter)
	{
		E_RESULT_CODE result = pWriter->BeginGroup("value");
		result = result | SaveVector3(pWriter, value.mValue);
		result = result | pWriter->EndGroup();

		return result;
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
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CQuaternionAnimationTrack::Load(IArchiveReader* pReader)
	{
		E_RESULT_CODE result = CBaseAnimationTrack<TQuaternionKeyFrame>::Load(pReader);

		return result;
	}

	E_RESULT_CODE CQuaternionAnimationTrack::_saveKeyFrameValue(const TQuaternionKeyFrame& value, IArchiveWriter* pWriter)
	{
		E_RESULT_CODE result = pWriter->BeginGroup("value");
		result = result | SaveQuaternion(pWriter, value.mValue);
		result = result | pWriter->EndGroup();

		return result;
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
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CColorAnimationTrack::Load(IArchiveReader* pReader)
	{
		E_RESULT_CODE result = CBaseAnimationTrack<TColorKeyFrame>::Load(pReader);

		return result;
	}

	E_RESULT_CODE CColorAnimationTrack::_saveKeyFrameValue(const TColorKeyFrame& value, IArchiveWriter* pWriter)
	{
		E_RESULT_CODE result = pWriter->BeginGroup("value");
		result = result | SaveColor32F(pWriter, value.mValue);
		result = result | pWriter->EndGroup();

		return result;
	}


	TDE2_API IAnimationTrack* CreateColorAnimationTrack(IAnimationClip* pOwner, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IAnimationTrack, CColorAnimationTrack, result, pOwner);
	}


	/*!
		\brief CBooleanAnimatioNTrack's definition
	*/

	CBooleanAnimationTrack::CBooleanAnimationTrack() :
		CBaseAnimationTrack()
	{
	}

	E_RESULT_CODE CBooleanAnimationTrack::Apply(IPropertyWrapper* pPropertyWrapper, F32 time)
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CBooleanAnimationTrack::Load(IArchiveReader* pReader)
	{
		E_RESULT_CODE result = CBaseAnimationTrack<TBooleanKeyFrame>::Load(pReader);

		return result;
	}

	E_RESULT_CODE CBooleanAnimationTrack::_saveKeyFrameValue(const TBooleanKeyFrame& value, IArchiveWriter* pWriter)
	{
		return pWriter->SetBool("value", value.mValue);
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
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CFloatAnimationTrack::Load(IArchiveReader* pReader)
	{
		E_RESULT_CODE result = CBaseAnimationTrack<TFloatKeyFrame>::Load(pReader);

		return result;
	}

	E_RESULT_CODE CFloatAnimationTrack::_saveKeyFrameValue(const TFloatKeyFrame& value, IArchiveWriter* pWriter)
	{
		return pWriter->SetFloat("value", value.mValue);
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
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CIntegerAnimationTrack::Load(IArchiveReader* pReader)
	{
		E_RESULT_CODE result = CBaseAnimationTrack<TIntegerKeyFrame>::Load(pReader);

		return result;
	}

	E_RESULT_CODE CIntegerAnimationTrack::_saveKeyFrameValue(const TIntegerKeyFrame& value, IArchiveWriter* pWriter)
	{
		return pWriter->SetInt32("value", value.mValue);
	}


	TDE2_API IAnimationTrack* CreateIntegerAnimationTrack(IAnimationClip* pOwner, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IAnimationTrack, CIntegerAnimationTrack, result, pOwner);
	}
}