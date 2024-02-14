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
	};


	const std::string TUIVideoContainerArchiveKeys::mVideoResourceIdKeyId = "video_id";
	const std::string TUIVideoContainerArchiveKeys::mIsLoopedKeyId = "is_looped";
	const std::string TUIVideoContainerArchiveKeys::mIsAutoplayEnabledKeyId = "autoplay";


	CUIVideoContainerComponent::CUIVideoContainerComponent():
		CBaseComponent()
	{
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
		
		mIsDirty = true;

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
		}
		pWriter->EndGroup();

		return RC_OK;
	}


#if TDE2_EDITORS_ENABLED
	
	void CUIVideoContainerComponent::DrawInspectorGUI(const TEditorContext& context)
	{
		CDefaultInspectorsRegistry::DrawInspectorHeader("UI Video Container", context, [](const TEditorContext& editorContext)
		{
			IImGUIContext& imguiContext = editorContext.mImGUIContext;
			CUIVideoContainerComponent& component = dynamic_cast<CUIVideoContainerComponent&>(editorContext.mComponent);

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