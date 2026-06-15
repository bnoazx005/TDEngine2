#include "../../include/graphics/animation/CAnimationContainerComponent.h"
#define META_EXPORT_ECS_SECTION
#include "../../include/metadata.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateAnimationContainerComponentFactory)
	TDE2_DEFINE_COMPONENT_META(TAnimationContainerComponentData);


	CAnimationContainerComponent::CAnimationContainerComponent() :
		CBaseComponentT()
	{
	}

	E_RESULT_CODE CAnimationContainerComponent::Clone(IComponent*& pDestObject) const
	{
		if (auto pComponent = dynamic_cast<CAnimationContainerComponent*>(pDestObject))
		{
			pComponent->mData = mData;

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
		if (mData.mIsStarted || mData.mIsPlaying)
		{
			return RC_FAIL;
		}

		mData.mIsStarted = true;
		mData.mIsPlaying = false;
		mData.mIsPaused = false;
		mData.mIsStopped = false;

		mData.mCurrTime = 0.0f;

		return RC_OK;
	}

	E_RESULT_CODE CAnimationContainerComponent::SetAnimationClipId(const std::string& id)
	{
		if (id.empty())
		{
			return RC_INVALID_ARGS;
		}

		mData.mAnimationClipId = id;

		return RC_OK;
	}

	E_RESULT_CODE CAnimationContainerComponent::SetAnimationClipResourceId(const TResourceId& resourceId)
	{
		if (TResourceId::Invalid == resourceId)
		{
			return RC_INVALID_ARGS;
		}

		mData.mAnimationClipResourceId = resourceId;

		return RC_OK;
	}

	E_RESULT_CODE CAnimationContainerComponent::SetTime(F32 value)
	{
		if (value < 0.0f)
		{
			return RC_INVALID_ARGS;
		}

		mData.mCurrTime = value;

		return RC_OK;
	}

	E_RESULT_CODE CAnimationContainerComponent::SetDuration(F32 value)
	{
		if (value < 0.0f)
		{
			return RC_INVALID_ARGS;
		}

		mData.mDuration = value;

		return RC_OK;
	}

	void CAnimationContainerComponent::SetStartedFlag(bool value)
	{
		mData.mIsStarted = value;
	}

	void CAnimationContainerComponent::SetPlayingFlag(bool value)
	{
		mData.mIsPlaying = value;
	}

	void CAnimationContainerComponent::SetStoppedFlag(bool value)
	{
		mData.mIsStopped = value;
	}

	void CAnimationContainerComponent::SetPausedFlag(bool value)
	{
		mData.mIsPaused = value;
	}

	F32 CAnimationContainerComponent::GetTime() const
	{
		return mData.mCurrTime;
	}

	F32 CAnimationContainerComponent::GetDuration() const
	{
		return mData.mDuration;
	}

	bool CAnimationContainerComponent::IsPlaying() const
	{
		return mData.mIsPlaying;
	}

	bool CAnimationContainerComponent::IsStarted() const
	{
		return mData.mIsStarted;
	}

	bool CAnimationContainerComponent::IsStopped() const
	{
		return mData.mIsStopped;
	}

	bool CAnimationContainerComponent::IsPaused() const
	{
		return mData.mIsPaused;
	}

	CAnimationContainerComponent::TPropertiesTable& CAnimationContainerComponent::GetCachedPropertiesTable()
	{
		return mCachedProperties;
	}

	const std::string& CAnimationContainerComponent::GetAnimationClipId() const
	{
		return mData.mAnimationClipId;
	}

	const std::string& CAnimationContainerComponent::GetTypeName() const
	{
		static const std::string componentName{ "animation-container" };
		return componentName;
	}

	TResourceId CAnimationContainerComponent::GetAnimationClipResourceId() const
	{
		return mData.mAnimationClipResourceId;
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