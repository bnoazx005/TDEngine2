#include "../../../include/scene/components/CPrefabLinkInfoComponent.h"
#include "../../../include/scene/CPrefabChangesList.h"
#include "../../include/scene/IScene.h"
#include <stringUtils.hpp>
#define META_EXPORT_ECS_SECTION
#include "../../include/metadata.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreatePrefabLinkInfoComponentFactory)
	TDE2_DEFINE_COMPONENT_META(TPrefabLinkComponentData)


	CPrefabLinkInfoComponent::CPrefabLinkInfoComponent() :
		CBaseComponentT()
	{
	}

	E_RESULT_CODE CPrefabLinkInfoComponent::Load(IArchiveReader* pReader)
	{
		E_RESULT_CODE result = CBaseComponentT::Load(pReader);
		if (result != RC_OK)
		{
			return result;
		}

		mData.mpChangesList = TPtr<CPrefabChangesList>(CreatePrefabChangesList(result));
		if (RC_OK != result)
		{
			return result;
		}

		result = result | mData.mpChangesList->Load(pReader);

		return result;
	}

	E_RESULT_CODE CPrefabLinkInfoComponent::Save(IArchiveWriter* pWriter)
	{
		E_RESULT_CODE result = CBaseComponentT::Save(pWriter);
		if (result != RC_OK)
		{
			return result;
		}

		pWriter->BeginGroup("component");
		{
			if (mData.mpChangesList)
			{
				mData.mpChangesList->Save(pWriter);
			}
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CPrefabLinkInfoComponent::Clone(IComponent*& pDestObject) const
	{
		if (auto pLinkInfo = dynamic_cast<CPrefabLinkInfoComponent*>(pDestObject))
		{
			pLinkInfo->mData = mData;
			pLinkInfo->mData.mpChangesList = mData.mpChangesList->Clone();

			return RC_OK;
		}

		return RC_FAIL;
	}

	void CPrefabLinkInfoComponent::SetPrefabLinkId(const std::string& id)
	{
		mData.mPrefabLinkId = id;
	}

	const std::string& CPrefabLinkInfoComponent::GetPrefabLinkId() const
	{
		return mData.mPrefabLinkId;
	}

	TPtr<CPrefabChangesList> CPrefabLinkInfoComponent::GetPrefabsChangesList() const
	{
		return mData.mpChangesList;
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