#include "../../../include/editor/ecs/EditorComponents.h"
#include "../../include/scene/IScene.h"
#include <stringUtils.hpp>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	TDE2_DEFINE_FLAG_COMPONENT(SelectedEntityComponent)

	/*!
		\brief CSceneInfoComponent's definition
	*/

	CSceneInfoComponent::CSceneInfoComponent() :
		CBaseComponent()
	{
	}

	E_RESULT_CODE CSceneInfoComponent::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		return RC_OK;
	}

	E_RESULT_CODE CSceneInfoComponent::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CSceneInfoComponent::GetTypeId()));
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	void CSceneInfoComponent::SetSceneId(const std::string& id)
	{
		mSceneId = id;
	}

	const std::string& CSceneInfoComponent::GetSceneId() const
	{
		return mSceneId;
	}


	IComponent* CreateSceneInfoComponent(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CSceneInfoComponent, result);
	}


	/*!
		\brief CSceneInfoComponentFactory's definition
	*/

	CSceneInfoComponentFactory::CSceneInfoComponentFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CSceneInfoComponentFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateSceneInfoComponent(result);
	}

	E_RESULT_CODE CSceneInfoComponentFactory::SetupComponent(CSceneInfoComponent* pComponent, const TSceneInfoComponentParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		pComponent->SetSceneId(params.mSceneId);

		return RC_OK;
	}


	IComponentFactory* CreateSceneInfoComponentFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CSceneInfoComponentFactory, result);
	}


	/*!
		\brief CPrefabLinkInfoComponent's definition
	*/

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

		return RC_OK;
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
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CPrefabLinkInfoComponent::Clone(IComponent*& pDestObject) const
	{
		if (auto pLinkInfo = dynamic_cast<CPrefabLinkInfoComponent*>(pDestObject))
		{
			pLinkInfo->mPrefabLinkId = mPrefabLinkId;
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

#endif