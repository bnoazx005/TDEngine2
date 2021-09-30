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

	const std::string& CBoundsComponent::GetTypeName() const
	{
		static const std::string id { "bounds" };
		return id;
	}

	IPropertyWrapperPtr CBoundsComponent::GetProperty(const std::string& propertyName)
	{
		return CBaseComponent::GetProperty(propertyName);
	}

	const std::vector<std::string>& CBoundsComponent::GetAllProperties() const
	{
		static const std::vector<std::string> properties 
		{

		};

		return properties;
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