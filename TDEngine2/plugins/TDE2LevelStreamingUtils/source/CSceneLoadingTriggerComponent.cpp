#include "../include/CSceneLoadingTriggerComponent.h"
#include <editor/Inspectors.h>
#include <core/IImGUIContext.h>


namespace TDEngine2
{
	struct TSceneLoadingTriggerArchiveKeys
	{
		static const std::string mScenePathKeyId;
		static const std::string mOffsetKeyId;
		static const std::string mSizesKeyId;
	};


	const std::string TSceneLoadingTriggerArchiveKeys::mScenePathKeyId = "scene_path";
	const std::string TSceneLoadingTriggerArchiveKeys::mOffsetKeyId = "offset";
	const std::string TSceneLoadingTriggerArchiveKeys::mSizesKeyId = "sizes";


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

		pReader->BeginGroup(TSceneLoadingTriggerArchiveKeys::mOffsetKeyId);
		if (auto loadVecResult = LoadVector3(pReader))
		{
			mOffset = loadVecResult.Get();
		}
		pReader->EndGroup();

		pReader->BeginGroup(TSceneLoadingTriggerArchiveKeys::mSizesKeyId);
		if (auto loadVecResult = LoadVector3(pReader))
		{
			mSizes = loadVecResult.Get();
		}
		pReader->EndGroup();

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

			pWriter->BeginGroup(TSceneLoadingTriggerArchiveKeys::mOffsetKeyId);
			SaveVector3(pWriter, mOffset);
			pWriter->EndGroup();

			pWriter->BeginGroup(TSceneLoadingTriggerArchiveKeys::mSizesKeyId);
			SaveVector3(pWriter, mSizes);
			pWriter->EndGroup();
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

	E_RESULT_CODE CSceneLoadingTriggerComponent::SetVolumeOffset(const TVector3& value)
	{
		mOffset = value;
		return RC_OK;
	}

	E_RESULT_CODE CSceneLoadingTriggerComponent::SetVolumeSizes(const TVector3& value)
	{
		if (value.x < 0.0f || value.y < 0.0f || value.z < 0.0f)
		{
			TDE2_ASSERT(false);
			return RC_INVALID_ARGS;
		}

		mSizes = value;
		return RC_OK;
	}

	const std::string& CSceneLoadingTriggerComponent::GetScenePath() const
	{
		return mScenePath;
	}

	const TVector3& CSceneLoadingTriggerComponent::GetVolumeOffset() const
	{
		return mOffset;
	}

	const TVector3& CSceneLoadingTriggerComponent::GetVolumeSizes() const
	{
		return mSizes;
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