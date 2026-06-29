#include "../../../include/graphics/UI/CToggleComponent.h"
#define META_EXPORT_ECS_SECTION
#include "../../../include/metadata.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateToggleFactory)
	TDE2_DEFINE_COMPONENT_META(TToggleComponentData)


	CToggle::CToggle() :
		CBaseComponentT()
	{
	}

	E_RESULT_CODE CToggle::PostLoad(CEntityManager* pEntityManager, const TEntitiesMapper& entitiesIdentifiersRemapper)
	{
		mData.mMarkerEntityRef = entitiesIdentifiersRemapper.Resolve(mData.mMarkerEntityRef);

		return CBaseComponentT::PostLoad(pEntityManager, entitiesIdentifiersRemapper);
	}

	void CToggle::SetState(bool state)
	{
		mData.mCurrState = state;
	}

	void CToggle::SetMarkerEntityId(TEntityId markerId)
	{
		mData.mMarkerEntityRef = markerId;
	}

	bool CToggle::GetState() const
	{
		return mData.mCurrState;
	}

	TEntityId CToggle::GetMarkerEntityId() const
	{
		return mData.mMarkerEntityRef;
	}


	IComponent* CreateToggle(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CToggle, result);
	}


	/*!
		\brief CToggleFactory's definition
	*/

	CToggleFactory::CToggleFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CToggleFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateToggle(result);
	}

	E_RESULT_CODE CToggleFactory::SetupComponent(CToggle* pComponent, const TToggleParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}


	IComponentFactory* CreateToggleFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CToggleFactory, result);
	}
}