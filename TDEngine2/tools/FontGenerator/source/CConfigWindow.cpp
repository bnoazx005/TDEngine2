#include "../include/CConfigWindow.h"
#include <tuple>
#include <vector>
#include <string>


namespace TDEngine2
{
	typedef std::vector<std::tuple<std::string, std::string>> TFileFiltersArray;

	const TFileFiltersArray FileExtensionsFilter
	{
		{ "Font info", "*.info" }
	};


	static TResult<TResourceId> OpenFromFile(IWindowSystem* pWindowSystem, IFileSystem* pFileSystem, IResourceManager* pResourceManager, const TFileFiltersArray& filters,
		const std::function<TResult<TResourceId>(const std::string&)>& onSuccessAction)
	{
		if (!pWindowSystem || !pFileSystem || !pResourceManager)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
		}

		auto openFileResult = pWindowSystem->ShowOpenFileDialog(filters);
		if (openFileResult.HasError())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(openFileResult.GetError());
		}

		return onSuccessAction(openFileResult.Get());
	}


	static E_RESULT_CODE SaveToFile(IFileSystem* pFileSystem, IResourceManager* pResourceManager, TResourceId resourceId, const std::string& destFilePath)
	{
		if (destFilePath.empty() || (TResourceId::Invalid == resourceId) || !pFileSystem || !pResourceManager)
		{
			return RC_INVALID_ARGS;
		}

		if (auto pAtlasTexture = pResourceManager->GetResource<ITextureAtlas>(resourceId))
		{
			return CTextureAtlas::Serialize(pFileSystem, pAtlasTexture, destFilePath);
		}

		return RC_FAIL;
	}


	CConfigWindow::CConfigWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CConfigWindow::Init(const TConfigWindowParams& params)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!params.mpResourceManager || !params.mpWindowSystem)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = params.mpResourceManager;
		mpInputContext = dynamic_cast<IDesktopInputContext*>(params.mpInputContext);
		mpWindowSystem = params.mpWindowSystem;
		mpFileSystem = params.mpFileSystem;

		mIsInitialized = true;
		mIsVisible = true;

		return RC_OK;
	}

	E_RESULT_CODE CConfigWindow::Free()
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

	void CConfigWindow::_onDraw()
	{
		bool isEnabled = mIsVisible;

		static const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			TVector2(300.0f, 150.0f),
			TVector2(300.0f, 1000.0f),
		};

		if (mpImGUIContext->BeginWindow("Settings", isEnabled, params))
		{
			_fontSelectionToolbar();
		}

		mpImGUIContext->EndWindow();

		mIsVisible = isEnabled;
	}

	void CConfigWindow::_fontSelectionToolbar()
	{
		mpImGUIContext->BeginHorizontal();
		{
			const TVector2 buttonSizes{ mpImGUIContext->GetWindowWidth() * 0.45f, 25.0f };

			mpImGUIContext->Button("Open", buttonSizes, [this]
			{
				if (auto openFileResult = OpenFromFile(mpWindowSystem, mpFileSystem, mpResourceManager, FileExtensionsFilter, [this](auto&& path)
				{
					if (IResource* pAtlas = mpResourceManager->GetResource<IResource>(mFontResourceId))
					{
						pAtlas->Unload();
					}

					return Wrench::TOkValue<TResourceId>(mpResourceManager->Load<ITextureAtlas>(path));
				}))
				{
					mFontResourceId = openFileResult.Get();
					mFontFileName = mpResourceManager->GetResource(mFontResourceId)->GetName();

					mLastSavedPath = Wrench::StringUtils::GetEmptyStr();
				}
			});

			mpImGUIContext->Button("Save", buttonSizes, [this]
			{
				if (!mLastSavedPath.empty())
				{
					SaveToFile(mpFileSystem, mpResourceManager, mFontResourceId, mLastSavedPath);
					return;
				}

				if (auto saveFileDialogResult = mpWindowSystem->ShowSaveFileDialog(FileExtensionsFilter))
				{
					mLastSavedPath = saveFileDialogResult.Get();
					SaveToFile(mpFileSystem, mpResourceManager, mFontResourceId, mLastSavedPath);
				}
			});
		}		
		mpImGUIContext->EndHorizontal();

		if (!mFontFileName.empty())
		{
			mpImGUIContext->Label(mFontFileName);
		}
	}


	TDE2_API IEditorWindow* CreateConfigWindow(const TConfigWindowParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CConfigWindow, result, params);
	}
}