#include "../include/CUIVideoContainerComponent.h"
#include "../include/CVideoProcessSystem.h"
#include <editor/Inspectors.h>
#include <core/IImGUIContext.h>
#define META_EXPORT_VIDEO_PLAYER_ECS_PLUGIN_SECTION
#include "../../include/metadata.h"


namespace TDEngine2
{
	TDE2_DEFINE_COMPONENT_META(TUIVideoContainerComponentData)


	CUIVideoContainerComponent::CUIVideoContainerComponent():
		CBaseComponentT(), mpInternalData(std::make_unique<TInternalVideoData>())
	{
		ResetState();
	}

	CUIVideoContainerComponent::~CUIVideoContainerComponent()
	{
	}

	E_RESULT_CODE CUIVideoContainerComponent::Load(IArchiveReader* pReader)
	{
		E_RESULT_CODE result = CBaseComponentT::Load(pReader);
		ResetState();

		return result;
	}

	E_RESULT_CODE CUIVideoContainerComponent::Clone(IComponent*& pDestObject) const
	{
		if (CUIVideoContainerComponent* pDestComponentPtr = dynamic_cast<CUIVideoContainerComponent*>(pDestObject))
		{
			pDestComponentPtr->mData = mData;
			pDestComponentPtr->ResetState();

			return RC_OK;
		}

		return RC_OK;
	}

	void CUIVideoContainerComponent::StartPlayback()
	{
		ResetState();
		mData.mIsPlaying = true;
	}

	void CUIVideoContainerComponent::StopPlayback()
	{
		mData.mStopPlayback = true;
	}

	void CUIVideoContainerComponent::ResetState()
	{
		mData.mIsDirty = true;
		mData.mIsPlaying = mData.mIsAutoplayEnabled;
		mData.mCurrTime = 0.0f;

		mData.mFrameTime = mData.mFPS ? static_cast<U32>(1000.0f / static_cast<F32>(mData.mFPS)) : 0;
	}


#if TDE2_EDITORS_ENABLED
	
	void CUIVideoContainerComponent::DrawInspectorGUI(const TEditorContext& context)
	{
		CDefaultInspectorsRegistry::DrawInspectorHeader("UI Video Container", context, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			CUIVideoContainerComponent& component = dynamic_cast<CUIVideoContainerComponent&>(editorContext.mComponent);
			TUIVideoContainerComponentData& videoComponentData = component.mData;

			/// \note loop
			{
				bool isLooped = videoComponentData.mIsLooped;

				imguiContext.BeginHorizontal();
				imguiContext.Label("IsLooped: ");
				imguiContext.Checkbox("##IsLooped", isLooped);
				imguiContext.EndHorizontal();

				if (isLooped != videoComponentData.mIsLooped)
				{
					videoComponentData.mIsLooped = isLooped;
				}
			}

			/// \note autoplay
			{
				bool isAutoplayOnStart = videoComponentData.mIsAutoplayEnabled;

				imguiContext.BeginHorizontal();
				imguiContext.Label("Autoplay: ");
				imguiContext.Checkbox("##Autoplay", isAutoplayOnStart);
				imguiContext.EndHorizontal();

				if (isAutoplayOnStart != videoComponentData.mIsAutoplayEnabled)
				{
					videoComponentData.mIsAutoplayEnabled = isAutoplayOnStart;
				}
			}

			imguiContext.BeginHorizontal();
			{
				if (imguiContext.Button(videoComponentData.mIsPlaying ? "Stop" : "Play", TVector2(100.0f, 25.0f)))
				{
					if (videoComponentData.mIsPlaying)
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