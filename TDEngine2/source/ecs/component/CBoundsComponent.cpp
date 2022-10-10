#include "../../../include/ecs/components/CBoundsComponent.h"


namespace TDEngine2
{
	CBoundsComponent::CBoundsComponent():
		CBaseComponent(), mIsDirty(true)
	{
	}

	void CBoundsComponent::SetBounds(const TAABB& aabbBounds)
	{
		mBounds = aabbBounds;
	}

	E_RESULT_CODE CBoundsComponent::Clone(IComponent*& pDestObject) const
	{
		if (auto pComponent = dynamic_cast<CBoundsComponent*>(pDestObject))
		{
			pComponent->mBounds = mBounds;
			return RC_OK;
		}

		return RC_FAIL;
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


	/*!
		\brief CBoundsComponentFactory's definition
	*/

	CBoundsComponentFactory::CBoundsComponentFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CBoundsComponentFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateBoundsComponent(result);
	}

	E_RESULT_CODE CBoundsComponentFactory::SetupComponent(CBoundsComponent* pComponent, const TBoundsComponentParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		pComponent->SetBounds(params.mBounds);

		return RC_OK;
	}


	IComponentFactory* CreateBoundsComponentFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CBoundsComponentFactory, result);
	}
}