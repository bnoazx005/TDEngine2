#include "../include/CEditorWindow.h"
#include <tuple>
#include <vector>
#include <string>


namespace TDEngine2
{
	CEditorWindow::CEditorWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CEditorWindow::Init(IResourceManager* pResourceManager, IInputContext* pInputContext, IWindowSystem* pWindowSystem)
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

		const TVector2 sizes{ mpImGUIContext->GetWindowWidth() - 20.0f, mpImGUIContext->GetWindowHeight() - 80.0f };

		if (mpImGUIContext->BeginChildWindow("##ListWindow", sizes))
		{
			auto&& texturesItems = pAtlasTexture->GetTexturesIdentifiersList();

			for (U32 i = 0; i < texturesItems.size(); ++i)
			{
				if (mpImGUIContext->SelectableItem(texturesItems[i], static_cast<U32>(mCurrSelectedTextureItem) == i))
				{
					mCurrSelectedTextureItem = static_cast<I32>(i);
				}
			}
		}

		mpImGUIContext->EndWindow();

		/// \note Toolbar
		mpImGUIContext->BeginHorizontal();
		mpImGUIContext->SetCursorScreenPos(mpImGUIContext->GetCursorScreenPos() + TVector2(0.0f, mpImGUIContext->GetWindowHeight() - 60.0f));

		mpImGUIContext->Button("Add Texture", TVector2(sizes.x * 0.5f, 25.0f), std::bind(&CEditorWindow::_addTextureToAtlasEventHandler, this));
		mpImGUIContext->Button("Remove Texture", TVector2(sizes.x * 0.5f, 25.0f), std::bind(&CEditorWindow::_removeTextureFromAtlasEventHandler, this));

		mpImGUIContext->EndHorizontal();
	}

	void CEditorWindow::_addTextureToAtlasEventHandler()
	{
		static const std::vector<std::tuple<std::string, std::string>> filters
		{
			{ "Textures", "*.*" }
		};

		ITextureAtlas* pAtlasTexture = mpResourceManager->GetResource<ITextureAtlas>(mAtlasResourceHandle);

		if (auto openFileResult = mpWindowSystem->ShowOpenFileDialog(filters))
		{
			pAtlasTexture->AddTexture(mpResourceManager->Load<ITexture2D>(openFileResult.Get(), E_RESOURCE_LOADING_POLICY::SYNCED));
			pAtlasTexture->Bake();
			return;
		}
	}

	void CEditorWindow::_removeTextureFromAtlasEventHandler()
	{
		ITextureAtlas* pAtlasTexture = mpResourceManager->GetResource<ITextureAtlas>(mAtlasResourceHandle);
		if (!pAtlasTexture)
		{
			TDE2_ASSERT(false);
			return;
		}

		auto&& texturesItems = pAtlasTexture->GetTexturesIdentifiersList();

		if (mCurrSelectedTextureItem < 0 || mCurrSelectedTextureItem >= static_cast<I32>(texturesItems.size()))
		{
			TDE2_ASSERT(false);
			return;
		}

		E_RESULT_CODE result = pAtlasTexture->RemoveTexture(texturesItems[mCurrSelectedTextureItem]);
		result = result | pAtlasTexture->Bake();

		TDE2_ASSERT(RC_OK == result);
	}


	TDE2_API IEditorWindow* CreateEditorWindow(IResourceManager* pResourceManager, IInputContext* pInputContext, IWindowSystem* pWindowSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CEditorWindow, result, pResourceManager, pInputContext, pWindowSystem);
	}
}