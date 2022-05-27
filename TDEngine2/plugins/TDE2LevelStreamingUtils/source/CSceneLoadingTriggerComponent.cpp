#include "../include/CSceneLoadingTriggerComponent.h"


namespace TDEngine2
{
	struct TSceneLoadingTriggerArchiveKeys
	{
		static const std::string mScenePathKeyId;
	};


	const std::string TSceneLoadingTriggerArchiveKeys::mScenePathKeyId = "scene_path";


	CSceneLoadingTriggerComponent::CSceneLoadingTriggerComponent():
		CBaseComponent(), mScenePath()
	{
	}

	E_RESULT_CODE CSceneLoadingTriggerComponent::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		mScenePath = pReader->GetString(TSceneLoadingTriggerArchiveKeys::mScenePathKeyId);

		return RC_OK;
	}

	E_RESULT_CODE CSceneLoadingTriggerComponent::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CSceneLoadingTriggerComponent::GetTypeId()));
			pWriter->SetString(TSceneLoadingTriggerArchiveKeys::mScenePathKeyId, mScenePath);
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CSceneLoadingTriggerComponent::SetScenePath(const std::string& scenePath)
	{
		if (scenePath.empty())
		{
			return RC_INVALID_ARGS;
		}

		mScenePath = scenePath;

		return RC_OK;
	}

	const std::string& CSceneLoadingTriggerComponent::GetScenePath() const
	{
		return mScenePath;
	}


	IComponent* CreateSceneLoadingTriggerComponent(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CSceneLoadingTriggerComponent, result);
	}


	/*!
		\brief CSceneLoadingTriggerComponentFactory's definition
	*/

	CSceneLoadingTriggerComponentFactory::CSceneLoadingTriggerComponentFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CSceneLoadingTriggerComponentFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateSceneLoadingTriggerComponent(result);
	}

	E_RESULT_CODE CSceneLoadingTriggerComponentFactory::SetupComponent(CSceneLoadingTriggerComponent* pComponent, const TSceneLoadingTriggerComponentParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		pComponent->SetScenePath(params.mScenePath);

		return RC_OK;
	}


	IComponentFactory* CreateSceneLoadingTriggerComponentFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CSceneLoadingTriggerComponentFactory, result);
	}
}