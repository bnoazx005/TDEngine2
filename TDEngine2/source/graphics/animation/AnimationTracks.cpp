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
		return SaveVector2(pWriter, value.mValue);
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
		return SaveVector3(pWriter, value.mValue);
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
		return SaveQuaternion(pWriter, value.mValue);
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
		return SaveColor32F(pWriter, value.mValue);
	}


	TDE2_API IAnimationTrack* CreateColorAnimationTrack(IAnimationClip* pOwner, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IAnimationTrack, CColorAnimationTrack, result, pOwner);
	}
}