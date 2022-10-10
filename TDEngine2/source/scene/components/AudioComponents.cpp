#include "../../include/scene/components/AudioComponents.h"


namespace TDEngine2
{
	TDE2_DEFINE_FLAG_COMPONENT(AudioListenerComponent);


	CAudioSourceComponent::CAudioSourceComponent() :
		CBaseComponent(), mVolume(1.0f), mPanning(0.0f), mIsLooped(false), mIsMuted(false), mIsPaused(false), mIsPlaying(false)
	{
	}

	E_RESULT_CODE CAudioSourceComponent::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		mAudioClipId = pReader->GetString("clip_id");

		mIsMuted = pReader->GetBool("is_muted");
		mIsPaused = pReader->GetBool("is_paused");
		mIsLooped = pReader->GetBool("is_looped");

		mVolume = pReader->GetFloat("volume");
		mPanning = pReader->GetFloat("panning");

		return RC_OK;
	}

	E_RESULT_CODE CAudioSourceComponent::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CAudioSourceComponent::GetTypeId()));
			pWriter->SetString("clip_id", mAudioClipId);

			pWriter->SetBool("is_muted", mIsMuted);
			pWriter->SetBool("is_paused", mIsPaused);
			pWriter->SetBool("is_looped", mIsLooped);

			pWriter->SetFloat("volume", mVolume);
			pWriter->SetFloat("panning", mPanning);
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CAudioSourceComponent::Clone(IComponent*& pDestObject) const
	{
		if (auto pComponent = dynamic_cast<CAudioSourceComponent*>(pDestObject))
		{
			pComponent->mAudioClipId = mAudioClipId;
			pComponent->mIsLooped = mIsLooped;
			pComponent->mIsMuted = mIsMuted;
			pComponent->mIsPaused = mIsPaused;
			pComponent->mIsPlaying = mIsPlaying;
			pComponent->mPanning = mPanning;
			pComponent->mVolume = mVolume;

			return RC_OK;
		}

		return RC_FAIL;
	}

	E_RESULT_CODE CAudioSourceComponent::SetAudioClipId(const std::string& id)
	{
		mAudioClipId = id;
		return RC_OK;
	}

	void CAudioSourceComponent::SetMuted(bool value)
	{
		mIsMuted = value;
	}

	void CAudioSourceComponent::SetPaused(bool value)
	{
		mIsPaused = value;
	}

	void CAudioSourceComponent::SetLooped(bool value)
	{
		mIsLooped = value;
	}

	void CAudioSourceComponent::SetPlaying(bool value)
	{
		mIsPlaying = value;
	}

	E_RESULT_CODE CAudioSourceComponent::SetVolume(F32 value)
	{
		if (value > 1.0f || value < 0.0f)
		{
			return RC_INVALID_ARGS;
		}

		mVolume = value;

		return RC_OK;
	}

	E_RESULT_CODE CAudioSourceComponent::SetPanning(F32 value)
	{
		if (value > 1.0f || value < -1.0f)
		{
			return RC_INVALID_ARGS;
		}

		mPanning = value;

		return RC_OK;
	}

	const std::string& CAudioSourceComponent::GetAudioClipId() const
	{
		return mAudioClipId;
	}

	bool CAudioSourceComponent::IsMuted() const
	{
		return mIsMuted;
	}

	bool CAudioSourceComponent::IsPaused() const
	{
		return mIsPaused;
	}

	bool CAudioSourceComponent::IsLooped() const
	{
		return mIsLooped;
	}

	bool CAudioSourceComponent::IsPlaying() const
	{
		return mIsPlaying;
	}

	F32 CAudioSourceComponent::GetVolume() const
	{
		return mVolume;
	}

	F32 CAudioSourceComponent::GetPanning() const
	{
		return mPanning;
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