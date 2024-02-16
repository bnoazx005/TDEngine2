#include "../include/CUIVideoContainerComponent.h"
#include <editor/Inspectors.h>
#include <core/IImGUIContext.h>


namespace TDEngine2
{
	struct TUIVideoContainerArchiveKeys
	{
		static const std::string mVideoResourceIdKeyId;
		static const std::string mIsLoopedKeyId;
		static const std::string mIsAutoplayEnabledKeyId;
		static const std::string mFramesPerSecondKeyId;
	};


	const std::string TUIVideoContainerArchiveKeys::mVideoResourceIdKeyId = "video_id";
	const std::string TUIVideoContainerArchiveKeys::mIsLoopedKeyId = "is_looped";
	const std::string TUIVideoContainerArchiveKeys::mIsAutoplayEnabledKeyId = "autoplay";
	const std::string TUIVideoContainerArchiveKeys::mFramesPerSecondKeyId = "fps";


	CUIVideoContainerComponent::CUIVideoContainerComponent():
		CBaseComponent()
	{
		ResetState();
	}

	E_RESULT_CODE CUIVideoContainerComponent::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		mVideoResourceId = pReader->GetString(TUIVideoContainerArchiveKeys::mVideoResourceIdKeyId);
		mIsLooped = pReader->GetBool(TUIVideoContainerArchiveKeys::mIsLoopedKeyId);
		mIsAutoplayEnabled = pReader->GetBool(TUIVideoContainerArchiveKeys::mIsAutoplayEnabledKeyId);
		mFPS = pReader->GetUInt16(TUIVideoContainerArchiveKeys::mFramesPerSecondKeyId, 30);
		
		ResetState();

		return RC_OK;
	}

	E_RESULT_CODE CUIVideoContainerComponent::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CUIVideoContainerComponent::GetTypeId()));

			pWriter->SetString(TUIVideoContainerArchiveKeys::mVideoResourceIdKeyId, mVideoResourceId);
			pWriter->SetBool(TUIVideoContainerArchiveKeys::mIsLoopedKeyId, mIsLooped);
			pWriter->SetBool(TUIVideoContainerArchiveKeys::mIsAutoplayEnabledKeyId, mIsAutoplayEnabled);
			pWriter->SetUInt16(TUIVideoContainerArchiveKeys::mFramesPerSecondKeyId, mFPS);
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CUIVideoContainerComponent::Clone(IComponent*& pDestObject) const
	{
		if (auto pComponent = dynamic_cast<CUIVideoContainerComponent*>(pDestObject))
		{
			pComponent->mVideoResourceId = mVideoResourceId;
			pComponent->mFPS = mFPS;
			pComponent->mFrameTime = mFrameTime;
			pComponent->mIsLooped = mIsLooped;
			pComponent->mIsAutoplayEnabled = mIsAutoplayEnabled;

			pComponent->ResetState();

			return RC_OK;
		}

		return RC_FAIL;
	}

	void CUIVideoContainerComponent::StartPlayback()
	{
		ResetState();
		mIsPlaying = true;
	}

	void CUIVideoContainerComponent::StopPlayback()
	{
		mStopPlayback = true;
	}

	void CUIVideoContainerComponent::ResetState()
	{
		mIsDirty = true;
		mIsPlaying = mIsAutoplayEnabled;
		mCurrTime = 0.0f;

		mFrameTime = mFPS ? static_cast<U32>(1000.0f / static_cast<F32>(mFPS)) : 0;
	}


#if TDE2_EDITORS_ENABLED
	
	void CUIVideoContainerComponent::DrawInspectorGUI(const TEditorContext& context)
	{
		CDefaultInspectorsRegistry::DrawInspectorHeader("UI Video Container", context, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			CUIVideoContainerComponent& component = dynamic_cast<CUIVideoContainerComponent&>(editorContext.mComponent);

			/// \note loop
			{
				bool isLooped = component.mIsLooped;

				imguiContext.BeginHorizontal();
				imguiContext.Label("IsLooped: ");
				imguiContext.Checkbox("##IsLooped", isLooped);
				imguiContext.EndHorizontal();

				if (isLooped != component.mIsLooped)
				{
					component.mIsLooped = isLooped;
				}
			}

			/// \note autoplay
			{
				bool isAutoplayOnStart = component.mIsAutoplayEnabled;

				imguiContext.BeginHorizontal();
				imguiContext.Label("Autoplay: ");
				imguiContext.Checkbox("##Autoplay", isAutoplayOnStart);
				imguiContext.EndHorizontal();

				if (isAutoplayOnStart != component.mIsAutoplayEnabled)
				{
					component.mIsAutoplayEnabled = isAutoplayOnStart;
				}
			}

			imguiContext.BeginHorizontal();
			{
				if (imguiContext.Button(component.mIsPlaying ? "Stop" : "Play", TVector2(100.0f, 25.0f)))
				{
					if (component.mIsPlaying)
					{
						component.StopPlayback();
					}
					else
					{
						component.StartPlayback();
					}
				}
			}
			imguiContext.EndHorizontal();
		});
	}

#endif


	IComponent* CreateUIVideoContainerComponent(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CUIVideoContainerComponent, result);
	}


	/*!
		\brief CUIVideoContainerComponentFactory's definition
	*/

	CUIVideoContainerComponentFactory::CUIVideoContainerComponentFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CUIVideoContainerComponentFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateUIVideoContainerComponent(result);
	}

	E_RESULT_CODE CUIVideoContainerComponentFactory::SetupComponent(CUIVideoContainerComponent* pComponent, const TUIVideoContainerComponentParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}


	IComponentFactory* CreateUIVideoContainerComponentFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CUIVideoContainerComponentFactory, result);
	}
}