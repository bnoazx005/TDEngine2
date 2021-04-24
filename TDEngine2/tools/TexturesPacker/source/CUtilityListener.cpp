#include "../include/CUtilityListener.h"
#include "../../include/metadata.h"
#include <functional>


using namespace TDEngine2;


static const std::string AtlasParametrizationModalWindowName = "CreateNewAtlasWindow";


std::vector<std::string> CUtilityListener::mAvailableFormats {};


E_RESULT_CODE CUtilityListener::OnStart()
{
	E_RESULT_CODE result = RC_OK;

	mCurrEditableAtlasId = mpResourceManager->Create<ITextureAtlas>("NewAtlas", TTexture2DParameters{ 512, 512, FT_NORM_UBYTE4, 1, 1, 0 });

	mpEditorWindow = dynamic_cast<CEditorWindow*>(TDEngine2::CreateEditorWindow(mpResourceManager, mpEngineCoreInstance->GetSubsystem<IInputContext>(), mpWindowSystem, result));
	mpEditorWindow->SetTextureAtlasResourceHandle(mCurrEditableAtlasId);

	for (auto&& currFieldInfo : Meta::EnumTrait<E_FORMAT_TYPE>::GetFields())
	{
		mAvailableFormats.push_back(currFieldInfo.name);
	}

	return RC_OK;
}

E_RESULT_CODE CUtilityListener::OnUpdate(const float& dt)
{
	mpEditorWindow->Draw(mpEngineCoreInstance->GetSubsystem<IImGUIContext>(), dt);
	
	_drawMainMenu();
	_createNewAtlasModalWindow();

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

	bool showNewAtlasPopupWindow = false;

	pImGUIContext->DisplayMainMenu([this, pFileSystem, &showNewAtlasPopupWindow](IImGUIContext& imguiContext)
	{
		imguiContext.MenuGroup("File", [this, pFileSystem, &showNewAtlasPopupWindow](IImGUIContext& imguiContext)
		{
			imguiContext.MenuItem("New", "CTRL+N", [this, &imguiContext, &showNewAtlasPopupWindow]
			{
				showNewAtlasPopupWindow = true;
			});

			imguiContext.MenuItem("Open", "CTRL+O", [this, pFileSystem]
			{
				if (auto openFileResult = OpenFromFile(mpWindowSystem, pFileSystem, mpResourceManager, FileExtensionsFilter, [this](auto&& path)
				{
					if (IResource* pAtlas = mpResourceManager->GetResource<IResource>(mCurrEditableAtlasId))
					{
						pAtlas->Unload();
					}

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

	if (showNewAtlasPopupWindow)
	{
		pImGUIContext->ShowModalWindow(AtlasParametrizationModalWindowName);
	}
}

void CUtilityListener::_createNewAtlasModalWindow()
{
	auto pImGUIContext = mpEngineCoreInstance->GetSubsystem<IImGUIContext>();

	static const TVector2 buttonSizes{ 100.0f, 25.0f };

	I32 formatIndex = static_cast<I32>(mNewAtlasFormat);

	if (pImGUIContext->BeginModalWindow(AtlasParametrizationModalWindowName))
	{
		pImGUIContext->Label("New atlas parameters");

		pImGUIContext->BeginHorizontal();
		pImGUIContext->Label("Name: ");
		pImGUIContext->TextField("##AtlasId", mNewAtlasName);
		pImGUIContext->EndHorizontal();

		pImGUIContext->BeginHorizontal();
		{
			pImGUIContext->Label("Sizes (W, H):");
			pImGUIContext->IntField("##AtlasWidth", mNewAtlasWidth);
			pImGUIContext->IntField("##AtlasHeight", mNewAtlasHeight);
		}
		pImGUIContext->EndHorizontal();

		pImGUIContext->BeginHorizontal();
		pImGUIContext->Label("Format:");
		mNewAtlasFormat = static_cast<E_FORMAT_TYPE>(pImGUIContext->Popup("##Format", formatIndex, mAvailableFormats));
		pImGUIContext->EndHorizontal();

		pImGUIContext->BeginHorizontal();
		{
			pImGUIContext->Button("Create", buttonSizes, [this, pImGUIContext]
			{
				TTexture2DParameters params{ static_cast<U32>(mNewAtlasWidth), static_cast<U32>(mNewAtlasHeight), mNewAtlasFormat, 1, 1, 0 };

				mCurrEditableAtlasId = mpResourceManager->Create<ITextureAtlas>(mNewAtlasName, params);
				mLastSavedPath = Wrench::StringUtils::GetEmptyStr();

				mpEditorWindow->SetTextureAtlasResourceHandle(mCurrEditableAtlasId);

				// \todo Add reset of the app's state here

				pImGUIContext->CloseCurrentModalWindow();
			});

			pImGUIContext->Button("Cancel", buttonSizes, [this, pImGUIContext]
			{
				pImGUIContext->CloseCurrentModalWindow();
			});
		}
		pImGUIContext->EndHorizontal();

		pImGUIContext->EndModalWindow();
	}
}