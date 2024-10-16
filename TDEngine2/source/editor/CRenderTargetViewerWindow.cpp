#include "../../include/editor/CRenderTargetViewerWindow.h"
#include "../../include/core/IImGUIContext.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/graphics/CBaseRenderTarget.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CRenderTargetViewerEditorWindow::CRenderTargetViewerEditorWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CRenderTargetViewerEditorWindow::Init(IResourceManager* pResourceManager)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pResourceManager)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;

		mIsInitialized = true;

		return RC_OK;
	}

	void CRenderTargetViewerEditorWindow::_onDraw()
	{
		bool isEnabled = mIsVisible;

		static const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			TVector2(250.0f, 250.0f),
			TVector2(500.0f, 500.0f),
		};

		if (mpImGUIContext->BeginWindow("Render Target Viewer", isEnabled, params))
		{
			auto&& renderTargetsList = mpResourceManager->GetResourcesListByType<CBaseRenderTarget>();

			mCurrSelectedItem = mpImGUIContext->Popup("Render Target", mCurrSelectedItem, renderTargetsList);
			
			const TResourceId textureId = mpResourceManager->Load<IRenderTarget>(renderTargetsList[mCurrSelectedItem]);
			if (textureId != TResourceId::Invalid)
			{
				mpImGUIContext->Image(textureId, TVector2(mpImGUIContext->GetWindowWidth() - 25.0f, mpImGUIContext->GetWindowHeight() - 60.0f));
			}

			mpImGUIContext->EndWindow();
		}

		mIsVisible = isEnabled;
	}


	TDE2_API IEditorWindow* CreateRenderTargetViewerEditorWindow(IResourceManager* pResourceManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CRenderTargetViewerEditorWindow, result, pResourceManager);
	}
}

#endif