#include "../../include/graphics/animation/CMeshAnimatorComponent.h"


namespace TDEngine2
{
	CMeshAnimatorComponent::CMeshAnimatorComponent() :
		CBaseComponent()
	{
	}

	E_RESULT_CODE CMeshAnimatorComponent::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CMeshAnimatorComponent::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}



		return RC_OK;
	}

	E_RESULT_CODE CMeshAnimatorComponent::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CMeshAnimatorComponent::GetTypeId()));
			
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	CMeshAnimatorComponent::TJointPose& CMeshAnimatorComponent::GetCurrAnimationPose()
	{
		return mCurrAnimationPose;
	}


	IComponent* CreateMeshAnimatorComponent(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CMeshAnimatorComponent, result);
	}


	CMeshAnimatorComponentFactory::CMeshAnimatorComponentFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CMeshAnimatorComponentFactory::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CMeshAnimatorComponentFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IComponent* CMeshAnimatorComponentFactory::Create(const TBaseComponentParameters* pParams) const
	{
		if (!pParams)
		{
			return nullptr;
		}

		const TMeshAnimatorComponentParameters* pAnimatorParams = static_cast<const TMeshAnimatorComponentParameters*>(pParams);

		E_RESULT_CODE result = RC_OK;

		return CreateMeshAnimatorComponent(result);
	}

	IComponent* CMeshAnimatorComponentFactory::CreateDefault(const TBaseComponentParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return CreateMeshAnimatorComponent(result);
	}

	TypeId CMeshAnimatorComponentFactory::GetComponentTypeId() const
	{
		return CMeshAnimatorComponent::GetTypeId();
	}


	IComponentFactory* CreateMeshAnimatorComponentFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CMeshAnimatorComponentFactory, result);
	}
}