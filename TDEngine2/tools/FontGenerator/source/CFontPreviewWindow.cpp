#include "../include/CFontPreviewWindow.h"
#include <tuple>
#include <vector>
#include <string>


namespace TDEngine2
{
	CFontPreviewWindow::CFontPreviewWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CFontPreviewWindow::Init(IResourceManager* pResourceManager, IInputContext* pInputContext, IWindowSystem* pWindowSystem)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pResourceManager || !pResourceManager || !pWindowSystem)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;
		mpInputContext = dynamic_cast<IDesktopInputContext*>(pInputContext);
		mpWindowSystem = pWindowSystem;

		mIsInitialized = true;
		mIsVisible = true;

		return RC_OK;
	}

	E_RESULT_CODE CFontPreviewWindow::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		--mRefCounter;

		if (!mRefCounter)
		{
			mIsInitialized = false;
			delete this;
		}

		return RC_OK;
	}

	void CFontPreviewWindow::SetTextureAtlasResourceHandle(TResourceId atlasHandle)
	{
		mAtlasResourceHandle = atlasHandle;
	}

	void CFontPreviewWindow::_onDraw()
	{
		bool isEnabled = mIsVisible;

		static const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			TVector2(150.0f, 150.0f),
			TVector2(1000.0f, 1000.0f),
		};

		if (mpImGUIContext->BeginWindow("Preview Window", isEnabled, params))
		{
			if (ITextureAtlas* pAtlasTexture = mpResourceManager->GetResource<ITextureAtlas>(mAtlasResourceHandle))
			{
				if (IResource* pTextureResource = dynamic_cast<IResource*>(pAtlasTexture->GetTexture()))
				{
					mpImGUIContext->Image(pTextureResource->GetId(), TVector2(mpImGUIContext->GetWindowWidth() - 40.0f, mpImGUIContext->GetWindowHeight() - 40.0f));
				}
			}
		}

		mpImGUIContext->EndWindow();

		mIsVisible = isEnabled;
	}


	TDE2_API IEditorWindow* CreateFontPreviewWindow(IResourceManager* pResourceManager, IInputContext* pInputContext, IWindowSystem* pWindowSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CFontPreviewWindow, result, pResourceManager, pInputContext, pWindowSystem);
	}
}