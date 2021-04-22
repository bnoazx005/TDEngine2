#include "../include/CUtilityListener.h"
#include <functional>


using namespace TDEngine2;


E_RESULT_CODE CUtilityListener::OnStart()
{
	E_RESULT_CODE result = RC_OK;

	mCurrEditableAtlasId = mpResourceManager->Create<ITextureAtlas>("NewAtlas", TTexture2DParameters{ 512, 512, FT_NORM_UBYTE4, 1, 1, 0 });

	mpEditorWindow = dynamic_cast<CEditorWindow*>(TDEngine2::CreateEditorWindow(mpResourceManager, mpEngineCoreInstance->GetSubsystem<IInputContext>(), mpWindowSystem, result));
	mpEditorWindow->SetTextureAtlasResourceHandle(mCurrEditableAtlasId);

	return RC_OK;
}

E_RESULT_CODE CUtilityListener::OnUpdate(const float& dt)
{
	mpEditorWindow->Draw(mpEngineCoreInstance->GetSubsystem<IImGUIContext>(), dt);
	_drawMainMenu();

	return RC_OK;
}

E_RESULT_CODE CUtilityListener::OnFree()
{
	return RC_OK;
}

void CUtilityListener::SetEngineInstance(TDEngine2::IEngineCore* pEngineCore)
{
	if (!pEngineCore)
	{
		return;
	}

	mpEngineCoreInstance = pEngineCore;

	mpGraphicsContext = mpEngineCoreInstance->GetSubsystem<IGraphicsContext>();

	mpWindowSystem = mpEngineCoreInstance->GetSubsystem<IWindowSystem>();

	mpResourceManager = mpEngineCoreInstance->GetSubsystem<IResourceManager>();
}


typedef std::vector<std::tuple<std::string, std::string>> TFileFiltersArray;


const TFileFiltersArray FileExtensionsFilter
{
	{ "Texture atlases", "*.info" }
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


void CUtilityListener::_drawMainMenu()
{
	auto pImGUIContext = mpEngineCoreInstance->GetSubsystem<IImGUIContext>();
	auto pFileSystem = mpEngineCoreInstance->GetSubsystem<IFileSystem>();

	pImGUIContext->DisplayMainMenu([this, pFileSystem](IImGUIContext& imguiContext)
	{
		imguiContext.MenuGroup("File", [this, pFileSystem](IImGUIContext& imguiContext)
		{
			imguiContext.MenuItem("New", "CTRL+N", [this]
			{
				mLastSavedPath = Wrench::StringUtils::GetEmptyStr();
				// \todo Add reset of the app's state here
			});

			imguiContext.MenuItem("Open", "CTRL+O", [this, pFileSystem]
			{
				if (auto openFileResult = OpenFromFile(mpWindowSystem, pFileSystem, mpResourceManager, FileExtensionsFilter, [this](auto&& path)
				{
					return Wrench::TOkValue<TResourceId>(mpResourceManager->Load<ITextureAtlas>(path));
				}))
				{
					mCurrEditableAtlasId = openFileResult.Get();

					mpEditorWindow->SetTextureAtlasResourceHandle(mCurrEditableAtlasId);
				}
			});

			imguiContext.MenuItem("Save", "CTRL+S", [this, pFileSystem]
			{
				SaveToFile(pFileSystem, mpResourceManager, mCurrEditableAtlasId, mLastSavedPath);
			});

			imguiContext.MenuItem("Save As...", "SHIFT+CTRL+S", [this, pFileSystem]
			{
				if (ITextureAtlas* pAtlas = mpResourceManager->GetResource<ITextureAtlas>(mCurrEditableAtlasId))
				{
					pAtlas->Bake();
				}

				if (auto saveFileDialogResult = mpWindowSystem->ShowSaveFileDialog(FileExtensionsFilter))
				{
					mLastSavedPath = saveFileDialogResult.Get();
					SaveToFile(pFileSystem, mpResourceManager, mCurrEditableAtlasId, mLastSavedPath);
				}
			});

			imguiContext.MenuItem("Quit", "Ctrl+Q", [this] { mpEngineCoreInstance->Quit(); });
		});

		imguiContext.MenuGroup("View", [this, pFileSystem](IImGUIContext& imguiContext)
		{
			imguiContext.MenuItem("Refresh", "F5", [this] { 
				if (ITextureAtlas* pAtlas = mpResourceManager->GetResource<ITextureAtlas>(mCurrEditableAtlasId))
				{
					pAtlas->Bake();
				}
			});
		});
	});
}