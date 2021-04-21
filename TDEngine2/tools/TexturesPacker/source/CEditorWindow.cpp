#include "../include/CEditorWindow.h"


namespace TDEngine2
{
	CEditorWindow::CEditorWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CEditorWindow::Init(IResourceManager* pResourceManager, IInputContext* pInputContext)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pResourceManager || !pResourceManager)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;
		mpInputContext = dynamic_cast<IDesktopInputContext*>(pInputContext);

		mIsInitialized = true;
		mIsVisible = true;

		return RC_OK;
	}

	E_RESULT_CODE CEditorWindow::Free()
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

	void CEditorWindow::SetTextureAtlasResourceHandle(TResourceId atlasHandle)
	{
		mAtlasResourceHandle = atlasHandle;
	}

	void CEditorWindow::_onDraw()
	{
		bool isEnabled = mIsVisible;

		static const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			TVector2(150.0f, 200.0f),
			TVector2(1000.0f, 550.0f),
		};

		if (mpImGUIContext->BeginWindow("Textures List", isEnabled, params))
		{
			_drawTexturesList();
		}

		mpImGUIContext->EndWindow();

		_drawTexturePreviewWindow();

		mIsVisible = isEnabled;
	}

	void CEditorWindow::_drawTexturePreviewWindow()
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
				IResource* pTextureResource = dynamic_cast<IResource*>(pAtlasTexture->GetTexture());
				mpImGUIContext->Image(pTextureResource->GetId(), TVector2(mpImGUIContext->GetWindowWidth() - 40.0f, mpImGUIContext->GetWindowHeight() - 40.0f));
			}
		}

		mpImGUIContext->EndWindow();
	}

	void CEditorWindow::_drawTexturesList()
	{
		ITextureAtlas* pAtlasTexture = mpResourceManager->GetResource<ITextureAtlas>(mAtlasResourceHandle);
		if (!pAtlasTexture)
		{
			return;
		}

		const TVector2 sizes { mpImGUIContext->GetWindowWidth() - 20.0f, mpImGUIContext->GetWindowHeight() - 80.0f };

		if (mpImGUIContext->BeginChildWindow("##ListWindow", sizes))
		{
			for (const std::string& currTextureId : pAtlasTexture->GetTexturesIdentifiersList())
			{
				mpImGUIContext->SelectableItem(currTextureId);
			}
		}

		mpImGUIContext->EndWindow();		

		/// \note Toolbar
		mpImGUIContext->BeginHorizontal();		
		mpImGUIContext->SetCursorScreenPos(mpImGUIContext->GetCursorScreenPos() + TVector2(0.0f, mpImGUIContext->GetWindowHeight() - 60.0f));

		mpImGUIContext->Button("Add Texture", TVector2(sizes.x * 0.5f, 25.0f), [this]
		{

		});
		mpImGUIContext->Button("Remove Texture", TVector2(sizes.x * 0.5f, 25.0f), [this]
		{

		});
		
		mpImGUIContext->EndHorizontal();
	}


	TDE2_API IEditorWindow* CreateEditorWindow(IResourceManager* pResourceManager, IInputContext* pInputContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CEditorWindow, result, pResourceManager, pInputContext);
	}
}