#include "../../include/scene/components/AudioComponents.h"


namespace TDEngine2
{
	TDE2_DEFINE_FLAG_COMPONENT(AudioListenerComponent);


	CAudioSourceComponent::CAudioSourceComponent() :
		CBaseComponent(), mVolume(1.0f), mPanning(0.0f)
	{
	}

	E_RESULT_CODE CAudioSourceComponent::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
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

	const std::string& CAudioSourceComponent::GetAudioClipId(const std::string& id) const
	{
		return mAudioClipId;
	}

	bool CAudioSourceComponent::IsMuted(bool value) const
	{
		return mIsMuted;
	}

	bool CAudioSourceComponent::IsPaused(bool value) const
	{
		return mIsPaused;
	}

	bool CAudioSourceComponent::IsLooped(bool value) const
	{
		return mIsLooped;
	}

	F32 CAudioSourceComponent::GetVolume(F32 value) const
	{
		return mVolume;
	}

	F32 CAudioSourceComponent::GetPanning(F32 value) const
	{
		return mPanning;
	}


	IComponent* CreateAudioSourceComponent(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CAudioSourceComponent, result);
	}


	CAudioSourceComponentFactory::CAudioSourceComponentFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CAudioSourceComponentFactory::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CAudioSourceComponentFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IComponent* CAudioSourceComponentFactory::Create(const TBaseComponentParameters* pParams) const
	{
		if (!pParams)
		{
			return nullptr;
		}

		E_RESULT_CODE result = RC_OK;

		CAudioSourceComponent* pAudioSource = dynamic_cast<CAudioSourceComponent*>(CreateAudioSourceComponent(result));
		if (pAudioSource)
		{
			if (const TAudioSourceComponentParameters* params = static_cast<const TAudioSourceComponentParameters*>(pParams))
			{
				pAudioSource->SetAudioClipId(params->mAudioClipId);
				
				pAudioSource->SetMuted(params->mIsMuted);
				pAudioSource->SetLooped(params->mIsLooped);
				pAudioSource->SetPaused(params->mIsPaused);

				pAudioSource->SetVolume(params->mVolume);
				pAudioSource->SetPanning(params->mPanning);
			}
		}

		return pAudioSource;
	}

	IComponent* CAudioSourceComponentFactory::CreateDefault(const TBaseComponentParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return CreateAudioSourceComponent(result);
	}

	TypeId CAudioSourceComponentFactory::GetComponentTypeId() const
	{
		return CAudioSourceComponent::GetTypeId();
	}


	IComponentFactory* CreateAudioSourceComponentFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CAudioSourceComponentFactory, result);
	}
}