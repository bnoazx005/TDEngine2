#include "../../../include/editor/ecs/EditorComponents.h"
#include "../../include/scene/IScene.h"
#include <stringUtils.hpp>
#define META_EXPORT_ECS_SECTION
#include "../../include/metadata.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	TDE2_DEFINE_RUNTIME_ONLY_FLAG_COMPONENT(SelectedEntityComponent)
	TDE2_REGISTER_COMPONENT_FACTORY(CreateSceneInfoComponentFactory)
	TDE2_DEFINE_COMPONENT_META(TSceneInfoComponentData);


	/*!
		\brief CSceneInfoComponent's definition
	*/

	CSceneInfoComponent::CSceneInfoComponent() :
		CBaseComponentT()
	{
	}

	void CSceneInfoComponent::SetSceneId(const std::string& id)
	{
		mData.mSceneId = id;
	}

	const std::string& CSceneInfoComponent::GetSceneId() const
	{
		return mData.mSceneId;
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
}

#endif