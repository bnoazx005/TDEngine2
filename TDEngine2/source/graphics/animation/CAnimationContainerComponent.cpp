#include "../../include/graphics/animation/CAnimationContainerComponent.h"


namespace TDEngine2
{
	struct TAnimationContainerArchiveKeys
	{
		static const std::string mAnimationClipKeyId;
		static const std::string mAnimationDurationKeyId;
	};


	const std::string TAnimationContainerArchiveKeys::mAnimationClipKeyId = "clip_id";
	const std::string TAnimationContainerArchiveKeys::mAnimationDurationKeyId = "clip_duration";


	CAnimationContainerComponent::CAnimationContainerComponent() :
		CBaseComponent(), mCurrTime(0.0f)
	{
	}

	E_RESULT_CODE CAnimationContainerComponent::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}
		
		mAnimationClipId = pReader->GetString(TAnimationContainerArchiveKeys::mAnimationClipKeyId);
		mDuration = pReader->GetFloat(TAnimationContainerArchiveKeys::mAnimationDurationKeyId);

		mCurrTime = 0.0f;

		mIsPlaying = false;
		mIsStarted = false;
		mIsStopped = true;

		return RC_OK;
	}

	E_RESULT_CODE CAnimationContainerComponent::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CAnimationContainerComponent::GetTypeId()));

			pWriter->SetString(TAnimationContainerArchiveKeys::mAnimationClipKeyId, mAnimationClipId);
			pWriter->SetFloat(TAnimationContainerArchiveKeys::mAnimationDurationKeyId, mDuration);
			
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CAnimationContainerComponent::Clone(IComponent*& pDestObject) const
	{
		if (auto pComponent = dynamic_cast<CAnimationContainerComponent*>(pDestObject))
		{
			pComponent->mAnimationClipId = mAnimationClipId;
			pComponent->mAnimationClipResourceId = mAnimationClipResourceId;
			pComponent->mCurrTime = mCurrTime;
			pComponent->mDuration = mDuration;
			pComponent->mIsPaused = mIsPaused;
			pComponent->mIsPlaying = mIsPlaying;
			pComponent->mIsStarted = mIsStarted;
			pComponent->mIsStopped = mIsStopped;

			pComponent->mCachedProperties.clear();

			for (auto&& currProperty : mCachedProperties)
			{
				pComponent->mCachedProperties.insert({ currProperty.first, currProperty.second });
			}

			return RC_OK;
		}

		return RC_FAIL;
	}

	E_RESULT_CODE CAnimationContainerComponent::Play()
	{
		if (mIsStarted || mIsPlaying)
		{
			return RC_FAIL;
		}

		mIsStarted = true;
		mIsPlaying = false;
		mIsPaused = false;
		mIsStopped = false;

		mCurrTime = 0.0f;

		return RC_OK;
	}

	E_RESULT_CODE CAnimationContainerComponent::SetAnimationClipId(const std::string& id)
	{
		if (id.empty())
		{
			return RC_INVALID_ARGS;
		}

		mAnimationClipId = id;

		return RC_OK;
	}

	E_RESULT_CODE CAnimationContainerComponent::SetAnimationClipResourceId(const TResourceId& resourceId)
	{
		if (TResourceId::Invalid == resourceId)
		{
			return RC_INVALID_ARGS;
		}

		mAnimationClipResourceId = resourceId;

		return RC_OK;
	}

	E_RESULT_CODE CAnimationContainerComponent::SetTime(F32 value)
	{
		if (value < 0.0f)
		{
			return RC_INVALID_ARGS;
		}

		mCurrTime = value;

		return RC_OK;
	}

	E_RESULT_CODE CAnimationContainerComponent::SetDuration(F32 value)
	{
		if (value < 0.0f)
		{
			return RC_INVALID_ARGS;
		}

		mDuration = value;

		return RC_OK;
	}

	void CAnimationContainerComponent::SetStartedFlag(bool value)
	{
		mIsStarted = value;
	}

	void CAnimationContainerComponent::SetPlayingFlag(bool value)
	{
		mIsPlaying = value;
	}

	void CAnimationContainerComponent::SetStoppedFlag(bool value)
	{
		mIsStopped = value;
	}

	void CAnimationContainerComponent::SetPausedFlag(bool value)
	{
		mIsPaused = value;
	}

	F32 CAnimationContainerComponent::GetTime() const
	{
		return mCurrTime;
	}

	F32 CAnimationContainerComponent::GetDuration() const
	{
		return mDuration;
	}

	bool CAnimationContainerComponent::IsPlaying() const
	{
		return mIsPlaying;
	}

	bool CAnimationContainerComponent::IsStarted() const
	{
		return mIsStarted;
	}

	bool CAnimationContainerComponent::IsStopped() const
	{
		return mIsStopped;
	}

	bool CAnimationContainerComponent::IsPaused() const
	{
		return mIsPaused;
	}

	CAnimationContainerComponent::TPropertiesTable& CAnimationContainerComponent::GetCachedPropertiesTable()
	{
		return mCachedProperties;
	}

	const std::string& CAnimationContainerComponent::GetAnimationClipId() const
	{
		return mAnimationClipId;
	}

	const std::string& CAnimationContainerComponent::GetTypeName() const
	{
		static const std::string componentName{ "animation-container" };
		return componentName;
	}

	TResourceId CAnimationContainerComponent::GetAnimationClipResourceId() const
	{
		return mAnimationClipResourceId;
	}

	IComponent* CreateAnimationContainerComponent(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CAnimationContainerComponent, result);
	}


	/*!
		\brief CAnimationContainerComponentFactory's definition
	*/

	CAnimationContainerComponentFactory::CAnimationContainerComponentFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CAnimationContainerComponentFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateAnimationContainerComponent(result);
	}

	E_RESULT_CODE CAnimationContainerComponentFactory::SetupComponent(CAnimationContainerComponent* pComponent, const TAnimationContainerComponentParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}


	IComponentFactory* CreateAnimationContainerComponentFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CAnimationContainerComponentFactory, result);
	}
}