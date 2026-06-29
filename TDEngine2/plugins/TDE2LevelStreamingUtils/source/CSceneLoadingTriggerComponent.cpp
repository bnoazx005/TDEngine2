#include "../include/CSceneLoadingTriggerComponent.h"
#include <editor/Inspectors.h>
#include <core/IImGUIContext.h>
#define META_EXPORT_SCENE_LOADING_ECS_PLUGIN_SECTION
#include "../../include/metadata.h"


namespace TDEngine2
{
	TDE2_DEFINE_COMPONENT_META(TSceneLoadingTriggerComponentData)


	CSceneLoadingTriggerComponent::CSceneLoadingTriggerComponent():
		CBaseComponentT()
	{
	}

	E_RESULT_CODE CSceneLoadingTriggerComponent::SetScenePath(const std::string& scenePath)
	{
		if (scenePath.empty())
		{
			return RC_INVALID_ARGS;
		}

		mData.mScenePath = scenePath;
		mData.mIsDirty = true;

		return RC_OK;
	}

	E_RESULT_CODE CSceneLoadingTriggerComponent::SetVolumeOffset(const TVector3& value)
	{
		mData.mOffset = value;
		mData.mIsDirty = true;

		return RC_OK;
	}

	E_RESULT_CODE CSceneLoadingTriggerComponent::SetVolumeSizes(const TVector3& value)
	{
		if (value.x < 0.0f || value.y < 0.0f || value.z < 0.0f)
		{
			TDE2_ASSERT(false);
			return RC_INVALID_ARGS;
		}

		mData.mSizes = value;
		mData.mIsDirty = true;

		return RC_OK;
	}

	void CSceneLoadingTriggerComponent::SetDirtyFlag(bool value)
	{
		mData.mIsDirty = value;
	}
	
	void CSceneLoadingTriggerComponent::SetOverlappingState(bool value)
	{
		mData.mOverlappingState = value;
	}

	const std::string& CSceneLoadingTriggerComponent::GetScenePath() const
	{
		return mData.mScenePath;
	}

	const TVector3& CSceneLoadingTriggerComponent::GetVolumeOffset() const
	{
		return mData.mOffset;
	}

	const TVector3& CSceneLoadingTriggerComponent::GetVolumeSizes() const
	{
		return mData.mSizes;
	}

	bool CSceneLoadingTriggerComponent::IsDirty() const
	{
		return mData.mIsDirty;
	}

	bool CSceneLoadingTriggerComponent::GetOverlappingState() const
	{
		return mData.mOverlappingState;
	}


#if TDE2_EDITORS_ENABLED
	
	void CSceneLoadingTriggerComponent::DrawInspectorGUI(const TEditorContext& context)
	{
		CDefaultInspectorsRegistry::DrawInspectorHeader("Scene Loading Trigger", context, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			CSceneLoadingTriggerComponent& component = dynamic_cast<CSceneLoadingTriggerComponent&>(editorContext.mComponent);

			/// \note A scene that should be loaded
			{
				std::string scenePath = component.GetScenePath();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Scene Path:");
				imguiContext.TextField("##ScenePath", scenePath, [&component](auto&& value) { component.SetScenePath(value); });
				imguiContext.EndHorizontal();
			}

			/// \note Offset
			{
				TVector3 offset = component.GetVolumeOffset();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Offset");
				imguiContext.Vector3Field("##Offset", offset, [&component, &offset]
				{
					component.SetVolumeOffset(offset);
				});
				imguiContext.EndHorizontal();
			}

			/// \note Sizes
			{
				TVector3 sizes = component.GetVolumeSizes();

				imguiContext.BeginHorizontal();
				imguiContext.Label("Sizes");
				imguiContext.Vector3Field("##Sizes", sizes, [&component, &sizes]
				{
					component.SetVolumeSizes(sizes);
				});
				imguiContext.EndHorizontal();
			}
		});
	}

#endif


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