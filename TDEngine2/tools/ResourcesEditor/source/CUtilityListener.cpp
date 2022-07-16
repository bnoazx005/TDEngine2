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


E_RESULT_CODE CUtilityListener::OnStart()
{
	E_RESULT_CODE result = RC_OK;

	mpEditorWindow = dynamic_cast<CEditorWindow*>(CreateEditorWindow(
														mpResourceManager.Get(), 
														mpEngineCoreInstance->GetSubsystem<IInputContext>().Get(), 
														mpWindowSystem.Get(),
														mpEngineCoreInstance->GetSubsystem<IFileSystem>().Get(), 
														result));
	
	return RC_OK;
}


const std::vector<std::tuple<std::string, std::string>> FileExtensionsFilter
{
	{ "Resources Manifest", "*.manifest" }
};


static TResult<TPtr<CResourcesBuildManifest>> OpenFromFile(TPtr<IWindowSystem> pWindowSystem, TPtr<IFileSystem> pFileSystem, std::string& filePath)
{
	if (!pWindowSystem || !pFileSystem)
	{
		return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
	}

	auto filePathResult = pWindowSystem->ShowOpenFileDialog(FileExtensionsFilter);
	if (filePathResult.HasError())
	{
		return Wrench::TErrValue<E_RESULT_CODE>(filePathResult.GetError());
	}

	E_RESULT_CODE result = RC_OK;

	TPtr<CResourcesBuildManifest> pResourcesManifest = TPtr<CResourcesBuildManifest>(CreateResourcesBuildManifest(result));
	TDE2_ASSERT(RC_OK == result);

	if (pResourcesManifest)
	{
		filePath = filePathResult.Get();

		auto openFileResult = pFileSystem->Open<IYAMLFileReader>(filePath);
		if (openFileResult.HasError())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(openFileResult.GetError());
		}

		result = pResourcesManifest->Load(pFileSystem->Get<IYAMLFileReader>(openFileResult.Get()));
		TDE2_ASSERT(RC_OK == result);

		pResourcesManifest->SetBaseResourcesPath(fs::path(filePath).parent_path().string());
	}

	return Wrench::TOkValue<TPtr<CResourcesBuildManifest>>(pResourcesManifest);
}


template <typename T>
static E_RESULT_CODE SaveToFile(TPtr<IFileSystem> pFileSystem, const TPtr<T>& pObject, const std::string& destFilePath)
{
	if (destFilePath.empty() || !pFileSystem || !pObject)
	{
		return RC_INVALID_ARGS;
	}

	E_RESULT_CODE result = RC_OK;

	if (auto openFileResult = pFileSystem->Open<IYAMLFileWriter>(destFilePath, true))
	{
		if (auto pFileWriter = pFileSystem->Get<IYAMLFileWriter>(openFileResult.Get()))
		{
			if (RC_OK != (result = pObject->Save(pFileWriter)))
			{
				return result;
			}

			if (RC_OK != (result = pFileWriter->Close()))
			{
				return result;
			}
		}
	}

	return RC_OK;
}


static std::unique_ptr<TTexture2DParameters> CreateTexture2DRuntimeParamsFromMetaInfo(const TTexture2DResourceBuildInfo& textureInfo)
{
	std::unique_ptr<TTexture2DParameters> pTextureRuntimeParams = std::make_unique<TTexture2DParameters>();

	pTextureRuntimeParams->mTexSamplerDesc.mFilteringType = textureInfo.mFilteringType;

	pTextureRuntimeParams->mTexSamplerDesc.mUAddressMode = textureInfo.mAddressMode;
	pTextureRuntimeParams->mTexSamplerDesc.mVAddressMode = textureInfo.mAddressMode;
	pTextureRuntimeParams->mTexSamplerDesc.mWAddressMode = textureInfo.mAddressMode;

	return std::move(pTextureRuntimeParams);
}


static TPtr<IResourcesRuntimeManifest> PrepareRuntimeManifest(TPtr<CResourcesBuildManifest>& pManifest)
{
	E_RESULT_CODE result = RC_OK;

	TPtr<IResourcesRuntimeManifest> pRuntimeManifest = TPtr<IResourcesRuntimeManifest>(CreateResourcesRuntimeManifest(result));
	TDE2_ASSERT(pRuntimeManifest);

	pManifest->ForEachRegisteredResource<TTexture2DResourceBuildInfo>([&pRuntimeManifest](const TResourceBuildInfo& info)
	{
		pRuntimeManifest->AddResourceMeta(info.mRelativePathToResource, CreateTexture2DRuntimeParamsFromMetaInfo(dynamic_cast<const TTexture2DResourceBuildInfo&>(info)));
		return true;
	});

	return pRuntimeManifest;
}


static void DrawMainMenu(IEngineCore* pEngineCore, TPtr<IWindowSystem> pWindowSystem, TPtr<IImGUIContext> pImGUIContext, TPtr<IFileSystem> pFileSystem, TPtr<CResourcesBuildManifest>& pManifest, std::string& outputSavePath)
{
	pImGUIContext->DisplayMainMenu([pFileSystem, pEngineCore, pWindowSystem, &pManifest, &outputSavePath](IImGUIContext& imguiContext)
	{
		imguiContext.MenuGroup("File", [pFileSystem, pEngineCore, pWindowSystem, &pManifest, &outputSavePath](IImGUIContext& imguiContext)
		{
			imguiContext.MenuItem("Open Resources Manifest", "CTRL+O", [pFileSystem, pWindowSystem, &pManifest, &outputSavePath]
			{
				if (auto manifestResult = OpenFromFile(pWindowSystem, pFileSystem, outputSavePath))
				{
					pManifest = std::move(manifestResult.Get());
				}
			});

			imguiContext.MenuItem("Save Manifest", "CTRL+S", [pFileSystem, &pManifest, &outputSavePath]
			{
				SaveToFile(pFileSystem, pManifest, outputSavePath);
			});

			imguiContext.MenuItem("Save Manifest As...", "SHIFT+CTRL+S", [pWindowSystem, pFileSystem, &pManifest, &outputSavePath]
			{
				if (auto saveFileDialogResult = pWindowSystem->ShowSaveFileDialog(FileExtensionsFilter))
				{
					outputSavePath = saveFileDialogResult.Get();
					SaveToFile(pFileSystem, pManifest, outputSavePath);
				}
			});

			imguiContext.MenuItem("Quit", "Ctrl+Q", [pEngineCore] { pEngineCore->Quit(); });
		});

		imguiContext.MenuGroup("Build", [pFileSystem, &pManifest, pWindowSystem](IImGUIContext& imguiContext)
		{
			imguiContext.MenuItem("Export to Runtime Manifest", Wrench::StringUtils::GetEmptyStr(), [pFileSystem, &pManifest, pWindowSystem]
			{
				if (auto saveFileDialogResult = pWindowSystem->ShowSaveFileDialog(FileExtensionsFilter))
				{
					SaveToFile(pFileSystem, PrepareRuntimeManifest(pManifest), saveFileDialogResult.Get());
				}
			});
		});
	});
}


E_RESULT_CODE CUtilityListener::OnUpdate(const float& dt)
{
	auto&& pImGUIContext = mpEngineCoreInstance->GetSubsystem<IImGUIContext>();
	auto&& pFileSystem = mpEngineCoreInstance->GetSubsystem<IFileSystem>();
	auto&& pWindowSystem = mpEngineCoreInstance->GetSubsystem<IWindowSystem>();

	DrawMainMenu(mpEngineCoreInstance, pWindowSystem, pImGUIContext, pFileSystem, mpCurrOpenedResourcesManifest, mLastSavedPath);

	mpEditorWindow->Draw(pImGUIContext.Get(), dt);
	mpEditorWindow->SetResourcesManifest(mpCurrOpenedResourcesManifest);

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
