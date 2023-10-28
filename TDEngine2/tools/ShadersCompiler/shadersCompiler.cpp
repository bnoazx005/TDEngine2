#include "shadersCompiler.h"


#if _HAS_CXX17
	#include <filesystem>
	namespace fs = std::filesystem;
#else
	#include <experimental/filesystem>
	namespace fs = std::experimental::filesystem;
#endif


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	std::vector<std::string> GetDefaultShadersPaths() TDE2_NOEXCEPT
	{
		return { "Shaders/Default/", "Shaders/PostEffects/", "ProjectShaders/" };
	}


	std::vector<std::string> BuildFilesList(TPtr<IFileSystem> pFileSystem, const std::vector<std::string>& directories) TDE2_NOEXCEPT
	{
		if (directories.empty())
		{
			return {};
		}

		auto&& hasValidExtension = [=](const std::string& ext) { return ext == ".shader"; };

		std::unordered_set<std::string> processedPaths; // contains absolute paths that already have been processed 

		std::vector<std::string> filesPaths;

		for (auto&& currSource : directories)
		{
			auto&& resolvedBasePath = pFileSystem->ResolveVirtualPath(currSource);

			// files
			if (!fs::is_directory(resolvedBasePath))
			{
				auto&& path = fs::path{ resolvedBasePath };

				auto&& absPathStr = fs::canonical(resolvedBasePath).string();

				if (processedPaths.find(absPathStr) == processedPaths.cend() && hasValidExtension(path.extension().string()))
				{
					filesPaths.emplace_back(Wrench::StringUtils::ReplaceAll(path.string(), resolvedBasePath, currSource));
					processedPaths.emplace(absPathStr);
				}

				continue;
			}

			// directories
			for (auto&& directory : fs::recursive_directory_iterator{ resolvedBasePath })
			{
				auto&& path = directory.path();

				auto&& absPathStr = fs::canonical(path).string();

				if (processedPaths.find(absPathStr) == processedPaths.cend() && hasValidExtension(path.extension().string()))
				{
					filesPaths.emplace_back(Wrench::StringUtils::ReplaceAll(
						Wrench::StringUtils::ReplaceAll(path.string(), resolvedBasePath, currSource), "\\", "/"));

					processedPaths.emplace(absPathStr);
				}
			}
		}

		return filesPaths;
	}


	static E_RESULT_CODE CompileShader(TPtr<IResourceManager> pResourceManager, TPtr<IShaderCache> pShaderCache, const std::string& shaderPath)
	{
		std::unique_ptr<TBaseResourceParameters> pParameters = std::make_unique<TShaderParameters>();
		TShaderParametersPtr pShaderParams = dynamic_cast<TShaderParametersPtr>(pParameters.get());
		
		auto pShader = pResourceManager->GetResource<IShader>(pResourceManager->Load<IShader>(shaderPath));
		if (!pShader)
		{
			return RC_FAIL;
		}

		return pShaderCache->AddShaderEntity(DynamicPtrCast<CBaseResource>(pShader)->GetName(), pShader->GetShaderMetaData());
	}


	E_RESULT_CODE ProcessShaders(IEngineCore* pEngineCore, std::vector<std::string>&& files) TDE2_NOEXCEPT
	{
		E_RESULT_CODE result = RC_OK;

		auto pGraphicsContext = pEngineCore->GetSubsystem<IGraphicsContext>();
		auto pFileSystem = pEngineCore->GetSubsystem<IFileSystem>();
		auto pResourceManager = pEngineCore->GetSubsystem<IResourceManager>();
		
		auto pShaderCache = pGraphicsContext->GetGraphicsObjectManager()->CreateShaderCache(pFileSystem.Get(), false).Get();

		for (auto&& currShaderPath : files)
		{
			CompileShader(pResourceManager, pShaderCache, currShaderPath);
		}

		result = result | pShaderCache->Dump();

		return result;
	}
}

#endif