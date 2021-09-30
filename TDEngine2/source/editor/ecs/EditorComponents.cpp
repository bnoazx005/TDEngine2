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

	E_RESULT_CODE CSceneInfoComponent::Init(const std::string& id)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mSceneId = id;
		mIsInitialized = true;

		return RC_OK;
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


	IComponent* CreateSceneInfoComponent(const std::string& id, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CSceneInfoComponent, result, id);
	}


	CSceneInfoComponentFactory::CSceneInfoComponentFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CSceneInfoComponentFactory::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	IComponent* CSceneInfoComponentFactory::Create(const TBaseComponentParameters* pParams) const
	{
		if (!pParams)
		{
			return nullptr;
		}

		const TSceneInfoComponentParameters* params = static_cast<const TSceneInfoComponentParameters*>(pParams);

		E_RESULT_CODE result = RC_OK;

		return CreateSceneInfoComponent(params->mSceneId, result);
	}

	IComponent* CSceneInfoComponentFactory::CreateDefault(const TBaseComponentParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return CreateSceneInfoComponent(Wrench::StringUtils::GetEmptyStr(), result);
	}

	TypeId CSceneInfoComponentFactory::GetComponentTypeId() const
	{
		return CSceneInfoComponent::GetTypeId();
	}


	IComponentFactory* CreateSceneInfoComponentFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CSceneInfoComponentFactory, result);
	}
}

#endif