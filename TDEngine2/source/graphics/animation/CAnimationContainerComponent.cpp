#include "../../include/graphics/animation/CAnimationContainerComponent.h"


namespace TDEngine2
{
	CAnimationContainerComponent::CAnimationContainerComponent() :
		CBaseComponent()
	{
	}

	E_RESULT_CODE CAnimationContainerComponent::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CAnimationContainerComponent::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}



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
			
		}
		pWriter->EndGroup();

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

	F32 CAnimationContainerComponent::GetTime() const
	{
		return mCurrTime;
	}

	const std::string& CAnimationContainerComponent::GetAnimationClipId() const
	{
		return mAnimationClipId;
	}

	TResourceId CAnimationContainerComponent::GetAnimationClipResourceId() const
	{
		return mAnimationClipResourceId;
	}

	IComponent* CreateAnimationContainerComponent(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CAnimationContainerComponent, result);
	}


	CAnimationContainerComponentFactory::CAnimationContainerComponentFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CAnimationContainerComponentFactory::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CAnimationContainerComponentFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IComponent* CAnimationContainerComponentFactory::Create(const TBaseComponentParameters* pParams) const
	{
		if (!pParams)
		{
			return nullptr;
		}

		const TAnimationContainerComponentParameters* pSunLightParams = static_cast<const TAnimationContainerComponentParameters*>(pParams);

		E_RESULT_CODE result = RC_OK;

		return CreateAnimationContainerComponent(result);
	}

	IComponent* CAnimationContainerComponentFactory::CreateDefault(const TBaseComponentParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return CreateAnimationContainerComponent(result);
	}

	TypeId CAnimationContainerComponentFactory::GetComponentTypeId() const
	{
		return CAnimationContainerComponent::GetTypeId();
	}


	IComponentFactory* CreateAnimationContainerComponentFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CAnimationContainerComponentFactory, result);
	}
}