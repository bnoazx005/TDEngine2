#include "../../../include/ecs/components/CBoundsComponent.h"


namespace TDEngine2
{
	CBoundsComponent::CBoundsComponent():
		CBaseComponent(), mIsDirty(true)
	{
	}

	E_RESULT_CODE CBoundsComponent::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	void CBoundsComponent::SetBounds(const TAABB& aabbBounds)
	{
		mBounds = aabbBounds;
	}

	void CBoundsComponent::SetDirty(bool value)
	{
		mIsDirty = value;
	}

	const TAABB& CBoundsComponent::GetBounds() const
	{
		return mBounds;
	}

	bool CBoundsComponent::IsDirty() const
	{
		return mIsDirty;
	}


	IComponent* CreateBoundsComponent(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CBoundsComponent, result);
	}


	CBoundsComponentFactory::CBoundsComponentFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CBoundsComponentFactory::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CBoundsComponentFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IComponent* CBoundsComponentFactory::Create(const TBaseComponentParameters* pParams) const
	{
		if (!pParams)
		{
			return nullptr;
		}

		E_RESULT_CODE result = RC_OK;

		CBoundsComponent* pBounds = dynamic_cast<CBoundsComponent*>(CreateBoundsComponent(result));
		if (pBounds)
		{
			if (const TBoundsComponentParameters* params = static_cast<const TBoundsComponentParameters*>(pParams))
			{
				pBounds->SetBounds(params->mBounds);
			}
		}

		return pBounds;
	}

	IComponent* CBoundsComponentFactory::CreateDefault(const TBaseComponentParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return CreateBoundsComponent(result);
	}

	TypeId CBoundsComponentFactory::GetComponentTypeId() const
	{
		return CBoundsComponent::GetTypeId();
	}


	IComponentFactory* CreateBoundsComponentFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CBoundsComponentFactory, result);
	}
}