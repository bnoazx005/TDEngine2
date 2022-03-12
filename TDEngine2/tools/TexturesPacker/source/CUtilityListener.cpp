#include "../include/CUtilityListener.h"
#include "../../include/metadata.h"
#include "../deps/argparse/argparse.h"
#include <functional>

#if _HAS_CXX17
	#include <filesystem>
	namespace fs = std::filesystem;
#else
	#include <experimental/filesystem>
	namespace fs = std::experimental::filesystem;
#endif

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

	mpEditorWindow = dynamic_cast<CEditorWindow*>(TDEngine2::CreateEditorWindow(mpResourceManager.Get(), mpEngineCoreInstance->GetSubsystem<IInputContext>().Get(), mpWindowSystem.Get(), result));
	mpEditorWindow->SetTextureAtlasResourceHandle(mCurrEditableAtlasId);

	for (auto&& currFieldInfo : Meta::EnumTrait<E_FORMAT_TYPE>::GetFields())
	{
		mAvailableFormats.push_back(currFieldInfo.name);
	}

	return RC_OK;
}

E_RESULT_CODE CUtilityListener::OnUpdate(const float& dt)
{
	mpEditorWindow->Draw(mpEngineCoreInstance->GetSubsystem<IImGUIContext>().Get(), dt);
	
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
		return CTextureAtlas::Serialize(pFileSystem, pAtlasTexture.Get(), destFilePath);
	}

	return RC_FAIL;
}


void CUtilityListener::_drawMainMenu()
{
	auto pImGUIContext = mpEngineCoreInstance->GetSubsystem<IImGUIContext>();
	auto pFileSystem = mpEngineCoreInstance->GetSubsystem<IFileSystem>().Get();

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
				if (auto openFileResult = OpenFromFile(mpWindowSystem.Get(), pFileSystem, mpResourceManager.Get(), FileExtensionsFilter, [this](auto&& path)
				{
					if (auto pAtlas = mpResourceManager->GetResource<IResource>(mCurrEditableAtlasId))
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
				SaveToFile(pFileSystem, mpResourceManager.Get(), mCurrEditableAtlasId, mLastSavedPath);
			});

			imguiContext.MenuItem("Save As...", "SHIFT+CTRL+S", [this, pFileSystem]
			{
				if (auto saveFileDialogResult = mpWindowSystem->ShowSaveFileDialog(FileExtensionsFilter))
				{
					mLastSavedPath = saveFileDialogResult.Get();
					SaveToFile(pFileSystem, mpResourceManager.Get(), mCurrEditableAtlasId, mLastSavedPath);
				}
			});

			imguiContext.MenuItem("Quit", "Ctrl+Q", [this] { mpEngineCoreInstance->Quit(); });
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

	auto pTextureAtlas = mpResourceManager->GetResource<ITextureAtlas>(textureAtlasHandle);

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

	/// \note Serialize into the file
	return CTextureAtlas::Serialize(mpEngineCoreInstance->GetSubsystem<IFileSystem>().Get(), pTextureAtlas.Get(), mOptions.mOutputFilename);
}


constexpr const char* Usage =
{
	"TDE2TexturesPacker <input> .. <input> [options]\n"
	"where <input> - single texture's path"
};


constexpr const char* VersionArgId = "version";
constexpr const char* OutputFileArgId = "outfile";
constexpr const char* BasePathArgId = "base-path";
constexpr const char* WidthArgId = "width";
constexpr const char* HeightArgId = "height";
constexpr const char* FormatArgId = "format";


TDEngine2::TResult<TUtilityOptions> ParseOptions(int argc, const char** argv)
{
	auto&& pProgramOptions = TDEngine2::CProgramOptions::Get();

	pProgramOptions->AddArgument({ 'V', VersionArgId, "Print version info and exit", TProgramOptionsArgument::E_VALUE_TYPE::BOOLEAN, {} });
	pProgramOptions->AddArgument({ 'o', OutputFileArgId, "Output file's name <filename>", TProgramOptionsArgument::E_VALUE_TYPE::STRING, {} });
	pProgramOptions->AddArgument({ '\0', BasePathArgId, "A path that will be excluded from input files absolute paths", TProgramOptionsArgument::E_VALUE_TYPE::STRING, {} });
	pProgramOptions->AddArgument({ 'w', WidthArgId, "Width of the atlas", TProgramOptionsArgument::E_VALUE_TYPE::INTEGER, {} });
	pProgramOptions->AddArgument({ 'h', HeightArgId, "Height of the atlas", TProgramOptionsArgument::E_VALUE_TYPE::INTEGER, {} });
	pProgramOptions->AddArgument({ '\0', FormatArgId, "Format of the atlas", TProgramOptionsArgument::E_VALUE_TYPE::STRING, {} });

	E_RESULT_CODE result = pProgramOptions->ParseArgs(
		{ 
			argc,
			argv,
			"\nThe utility is an archiver of resources which is a part of TDE2 game engine that gathers them together in single peace (package)", 
			Usage
		});

	if (RC_OK != result)
	{
		return Wrench::TErrValue<E_RESULT_CODE>(result);
	}

	if (pProgramOptions->GetValueOrDefault(VersionArgId, false))
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
			sources.push_back(pProgramOptions->GetPositionalArgValue(i));
		}

		utilityOptions.mIsDefault = false;
	}

	utilityOptions.mOutputFilename = pProgramOptions->GetValueOrDefault<std::string>(OutputFileArgId, "");
	utilityOptions.mBasePath       = std::experimental::filesystem::path(pProgramOptions->GetValueOrDefault<std::string>(BasePathArgId, "")).string();
	utilityOptions.mAtlasWidth     = pProgramOptions->GetValueOrDefault(WidthArgId, 1024);
	utilityOptions.mAtlasHeight    = pProgramOptions->GetValueOrDefault(HeightArgId, 1024);
	utilityOptions.mFormatStr      = pProgramOptions->GetValueOrDefault<std::string>(FormatArgId, Meta::EnumTrait<E_FORMAT_TYPE>::ToString(E_FORMAT_TYPE::FT_NORM_UBYTE4));

	if (utilityOptions.mAtlasWidth < 0)
	{
		std::cerr << "Error: width coudn't be less than zero";
		return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
	}

	if (utilityOptions.mAtlasHeight < 0)
	{
		std::cerr << "Error: height coudn't be less than zero";
		return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
	}

	return Wrench::TOkValue<TUtilityOptions>(utilityOptions);
}