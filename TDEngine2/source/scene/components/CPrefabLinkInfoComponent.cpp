#include "../../../include/scene/components/CPrefabLinkInfoComponent.h"
#include "../../../include/scene/CPrefabChangesList.h"
#include "../../include/scene/IScene.h"
#include <stringUtils.hpp>


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreatePrefabLinkInfoComponentFactory)


	CPrefabLinkInfoComponent::CPrefabLinkInfoComponent() :
		CBaseComponent()
	{
	}

	E_RESULT_CODE CPrefabLinkInfoComponent::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		mpChangesList = TPtr<CPrefabChangesList>(CreatePrefabChangesList(result));
		if (RC_OK != result)
		{
			return result;
		}

		result = result | mpChangesList->Load(pReader);

		return result;
	}

	E_RESULT_CODE CPrefabLinkInfoComponent::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CPrefabLinkInfoComponent::GetTypeId()));

			if (mIsChangesListSerializationEnabled && mpChangesList)
			{
				mpChangesList->Save(pWriter);
			}
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CPrefabLinkInfoComponent::Clone(IComponent*& pDestObject) const
	{
		if (auto pLinkInfo = dynamic_cast<CPrefabLinkInfoComponent*>(pDestObject))
		{
			pLinkInfo->mPrefabLinkId = mPrefabLinkId;
			pLinkInfo->mpChangesList = mpChangesList->Clone();

			return RC_OK;
		}

		return RC_FAIL;
	}

	void CPrefabLinkInfoComponent::SetPrefabLinkId(const std::string& id)
	{
		mPrefabLinkId = id;
	}

	const std::string& CPrefabLinkInfoComponent::GetPrefabLinkId() const
	{
		return mPrefabLinkId;
	}

	TPtr<CPrefabChangesList> CPrefabLinkInfoComponent::GetPrefabsChangesList() const
	{
		return mpChangesList;
	}

	bool CPrefabLinkInfoComponent::IsRuntimeOnly() const
	{
		return true;
	}

	const std::string& CPrefabLinkInfoComponent::GetTypeName() const
	{
		static const std::string typeName = "prefab_link";
		return typeName;
	}


	IComponent* CreatePrefabLinkInfoComponent(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CPrefabLinkInfoComponent, result);
	}


	/*!
		\brief CPrefabLinkInfoComponentFactory's definition
	*/

	CPrefabLinkInfoComponentFactory::CPrefabLinkInfoComponentFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CPrefabLinkInfoComponentFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreatePrefabLinkInfoComponent(result);
	}

	E_RESULT_CODE CPrefabLinkInfoComponentFactory::SetupComponent(CPrefabLinkInfoComponent* pComponent, const TPrefabLinkInfoComponentParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		pComponent->SetPrefabLinkId(params.mPrefabLinkId);

		return RC_OK;
	}


	IComponentFactory* CreatePrefabLinkInfoComponentFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CPrefabLinkInfoComponentFactory, result);
	}
}