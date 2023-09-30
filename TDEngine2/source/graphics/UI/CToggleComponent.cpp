#include "../../../include/graphics/UI/CToggleComponent.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateToggleFactory)


	CToggle::CToggle() :
		CBaseComponent()
	{
	}


	struct TToggleArchiveKeys
	{
		static const std::string mMarkerEntityRefKey;
		static const std::string mStateKey;
	};


	const std::string TToggleArchiveKeys::mMarkerEntityRefKey = "marker_entity_ref";
	const std::string TToggleArchiveKeys::mStateKey = "state";

	E_RESULT_CODE CToggle::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		mMarkerEntityRef = static_cast<TEntityId>(pReader->GetUInt32(TToggleArchiveKeys::mMarkerEntityRefKey));
		mCurrState = pReader->GetBool(TToggleArchiveKeys::mStateKey);

		return RC_OK;
	}

	E_RESULT_CODE CToggle::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CToggle::GetTypeId()));

			pWriter->SetUInt32(TToggleArchiveKeys::mMarkerEntityRefKey, static_cast<U32>(mMarkerEntityRef));			
			pWriter->SetBool(TToggleArchiveKeys::mStateKey, mCurrState);
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CToggle::PostLoad(CEntityManager* pEntityManager, const TEntitiesMapper& entitiesIdentifiersRemapper)
	{
		mMarkerEntityRef = entitiesIdentifiersRemapper.Resolve(mMarkerEntityRef);

		return CBaseComponent::PostLoad(pEntityManager, entitiesIdentifiersRemapper);
	}

	E_RESULT_CODE CToggle::Clone(IComponent*& pDestObject) const
	{
		if (auto pComponent = dynamic_cast<CToggle*>(pDestObject))
		{
			pComponent->mMarkerEntityRef = mMarkerEntityRef;

			return RC_OK;
		}

		return RC_FAIL;
	}

	void CToggle::SetState(bool state)
	{
		mCurrState = state;
	}

	void CToggle::SetMarkerEntityId(TEntityId markerId)
	{
		mMarkerEntityRef = markerId;
	}

	bool CToggle::GetState() const
	{
		return mCurrState;
	}

	TEntityId CToggle::GetMarkerEntityId() const
	{
		return mMarkerEntityRef;
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