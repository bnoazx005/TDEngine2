#include "../include/CUtilityListener.h"
#include "../../include/metadata.h"
#include "../deps/argparse/argparse.h"
#include <functional>
#include <experimental/filesystem>


using namespace TDEngine2;


static const std::string AtlasParametrizationModalWindowName = "CreateNewAtlasWindow";


std::vector<std::string> CUtilityListener::mAvailableFormats {};


CUtilityListener::CUtilityListener(const TUtilityOptions& options):
	mOptions(options)
{
}

E_RESULT_CODE CUtilityListener::OnStart()
{
	if (!mOptions.mIsDefault)
	{
		/// \note If everything goes ok just quit the utility, otherwise run the graphics mode
		if (RC_OK == _processInNonGraphicalMode())
		{
			return mpEngineCoreInstance->Quit();
		}
	}

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

E_RESULT_CODE CUtilityListener::_processInNonGraphicalMode()
{
	TTexture2DParameters params
	{
		 mOptions.mAtlasWidth, 
		 mOptions.mAtlasHeight, 
		 Meta::EnumTrait<E_FORMAT_TYPE>::FromString(mOptions.mFormatStr),
		 1, 1, 0
	};

	/// \note Create a new atlas
	TResourceId textureAtlasHandle = mpResourceManager->Create<ITextureAtlas>(mOptions.mAtlasName, params);
	if (TResourceId::Invalid == textureAtlasHandle)
	{
		return RC_FAIL;
	}

	ITextureAtlas* pTextureAtlas = mpResourceManager->GetResource<ITextureAtlas>(textureAtlasHandle);

	E_RESULT_CODE result = RC_OK;

	std::string processedTexturePath;

	/// \note Fill it with textures
	for (const std::string& currFilename : mOptions.mInputFiles)
	{
		processedTexturePath = currFilename;

		if (processedTexturePath.find(mOptions.mBasePath) != std::string::npos)
		{
			/// \note exluced base path from the filename
			processedTexturePath.replace(0, mOptions.mBasePath.length(), Wrench::StringUtils::GetEmptyStr());
		}

		pTextureAtlas->AddTexture(mpResourceManager->Load<ITexture2D>(processedTexturePath));
	}	

	if (RC_OK != (result = pTextureAtlas->Bake()))
	{
		return result;
	}

	/// \note Serialize into the file
	return CTextureAtlas::Serialize(mpEngineCoreInstance->GetSubsystem<IFileSystem>(), pTextureAtlas, mOptions.mOutputFilename);
}


constexpr const char* Usage[] =
{
	"TDE2TexturesPacker <input> .. <input> [options]",
	"where <input> - single texture's path",
	0
};


TDEngine2::TResult<TUtilityOptions> ParseOptions(int argc, const char** argv)
{
	int showVersion = 0;

	// flags
	int width = 0, height = 0;

	const char* pBasePathDirectory = nullptr;
	const char* pOutputFilename = nullptr;
	const char* pAtlasFormat = nullptr;

	struct argparse_option options[] = {
		OPT_HELP(),
		OPT_GROUP("Basic options"),
		OPT_BOOLEAN('V', "version", &showVersion, "Print version info and exit"),
		OPT_STRING('o', "outfile", &pOutputFilename, "Output file's name <filename>"),
		OPT_STRING(0, "base-path", &pBasePathDirectory, "A path that will be excluded from input files absolute paths"),
		OPT_INTEGER('w', "width", &width, "Width of the atlas"),
		OPT_INTEGER('h', "height", &height, "Height of the atlas"),
		OPT_STRING(0, "format", &pAtlasFormat, "Format of the atlas"),
		OPT_END(),
	};

	struct argparse argparse;
	argparse_init(&argparse, options, Usage, 0);
	argparse_describe(&argparse, "\nThe utility is an archiver of resources which is a part of TDE2 game engine that gathers them together in single peace (package)", "\n");
	argc = argparse_parse(&argparse, argc, argv);

	if (showVersion)
	{
		std::cout << "TDE2TexturesPacker, version " << ToolVersion.mMajor << "." << ToolVersion.mMinor << std::endl;
		exit(0);
	}

	TUtilityOptions utilityOptions;

	// \note parse input files before any option, because argparse library will remove all argv's values after it processes that
	if (argc >= 1)
	{
		auto& sources = utilityOptions.mInputFiles;
		sources.clear();

		for (int i = 0; i < argc; ++i)
		{
			sources.push_back(argparse.out[i]);
		}

		utilityOptions.mIsDefault = false;
	}

	if (pOutputFilename)
	{
		utilityOptions.mOutputFilename = pOutputFilename;
	}

	if (pBasePathDirectory)
	{
		utilityOptions.mBasePath = std::experimental::filesystem::path(pBasePathDirectory).string();
	}

	if (width < 0)
	{
		std::cerr << "Error: width coudn't be less than zero";
		return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
	}

	if (height < 0)
	{
		std::cerr << "Error: height coudn't be less than zero";
		return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
	}

	utilityOptions.mFormatStr = pAtlasFormat ? pAtlasFormat : Meta::EnumTrait<E_FORMAT_TYPE>::ToString(E_FORMAT_TYPE::FT_NORM_UBYTE4);
	utilityOptions.mAtlasWidth = static_cast<U32>(width);
	utilityOptions.mAtlasHeight = static_cast<U32>(height);

	return Wrench::TOkValue<TUtilityOptions>(utilityOptions);
}