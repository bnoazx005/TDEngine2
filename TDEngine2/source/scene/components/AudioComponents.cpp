#include "../../include/scene/components/AudioComponents.h"
#define META_EXPORT_ECS_SECTION
#include "../../include/metadata.h"


namespace TDEngine2
{
	TDE2_DEFINE_FLAG_COMPONENT(AudioListenerComponent);
	TDE2_REGISTER_COMPONENT_FACTORY(CreateAudioSourceComponentFactory)
	TDE2_DEFINE_COMPONENT_META(TAudioSourceComponentData)


	CAudioSourceComponent::CAudioSourceComponent() :
		CBaseComponentT()
	{
	}

	E_RESULT_CODE CAudioSourceComponent::SetAudioClipId(const std::string& id)
	{
		mData.mAudioClipId = id;
		return RC_OK;
	}

	void CAudioSourceComponent::SetMuted(bool value)
	{
		mData.mIsMuted = value;
	}

	void CAudioSourceComponent::SetPaused(bool value)
	{
		mData.mIsPaused = value;
	}

	void CAudioSourceComponent::SetLooped(bool value)
	{
		mData.mIsLooped = value;
	}

	void CAudioSourceComponent::SetPlaying(bool value)
	{
		mData.mIsPlaying = value;
	}

	E_RESULT_CODE CAudioSourceComponent::SetVolume(F32 value)
	{
		if (value > 1.0f || value < 0.0f)
		{
			return RC_INVALID_ARGS;
		}

		mData.mVolume = value;

		return RC_OK;
	}

	E_RESULT_CODE CAudioSourceComponent::SetPanning(F32 value)
	{
		if (value > 1.0f || value < -1.0f)
		{
			return RC_INVALID_ARGS;
		}

		mData.mPanning = value;

		return RC_OK;
	}

	const std::string& CAudioSourceComponent::GetAudioClipId() const
	{
		return mData.mAudioClipId;
	}

	bool CAudioSourceComponent::IsMuted() const
	{
		return mData.mIsMuted;
	}

	bool CAudioSourceComponent::IsPaused() const
	{
		return mData.mIsPaused;
	}

	bool CAudioSourceComponent::IsLooped() const
	{
		return mData.mIsLooped;
	}

	bool CAudioSourceComponent::IsPlaying() const
	{
		return mData.mIsPlaying;
	}

	F32 CAudioSourceComponent::GetVolume() const
	{
		return mData.mVolume;
	}

	F32 CAudioSourceComponent::GetPanning() const
	{
		return mData.mPanning;
	}


	IComponent* CreateAudioSourceComponent(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CAudioSourceComponent, result);
	}


	/*!
		\brief CAudioSourceComponentFactory's definition
	*/

	CAudioSourceComponentFactory::CAudioSourceComponentFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CAudioSourceComponentFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateAudioSourceComponent(result);
	}

	E_RESULT_CODE CAudioSourceComponentFactory::SetupComponent(CAudioSourceComponent* pComponent, const TAudioSourceComponentParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		pComponent->SetAudioClipId(params.mAudioClipId);

		pComponent->SetMuted(params.mIsMuted);
		pComponent->SetLooped(params.mIsLooped);
		pComponent->SetPaused(params.mIsPaused);

		pComponent->SetVolume(params.mVolume);
		pComponent->SetPanning(params.mPanning);

		return RC_OK;
	}


	IComponentFactory* CreateAudioSourceComponentFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CAudioSourceComponentFactory, result);
	}
}