#include "../../../include/ecs/components/CBoundsComponent.h"
#define META_EXPORT_ECS_SECTION
#include "../../include/metadata.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateBoundsComponentFactory)
	TDE2_DEFINE_COMPONENT_META(TBoundsComponentData)


	CBoundsComponent::CBoundsComponent():
		CBaseComponentT()
	{
	}

	void CBoundsComponent::SetBounds(const TAABB& aabbBounds)
	{
		mData.mBounds = aabbBounds;
	}

	void CBoundsComponent::SetDirty(bool value)
	{
		mData.mIsDirty = value;
	}

	const TAABB& CBoundsComponent::GetBounds() const
	{
		return mData.mBounds;
	}

	bool CBoundsComponent::IsDirty() const
	{
		return mData.mIsDirty;
	}

	const std::string& CBoundsComponent::GetTypeName() const
	{
		static const std::string id { "bounds" };
		return id;
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