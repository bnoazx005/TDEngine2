#include "../../include/graphics/CBaseShaderLoader.h"
#include "../../include/graphics/CBaseShader.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/IFile.h"
#include "../../include/platform/CTextFileReader.h"
#include "../../include/graphics/IShaderCompiler.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/graphics/CBaseGraphicsObjectManager.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/editor/CPerfProfiler.h"
#include <unordered_map>
#include <string>


namespace TDEngine2
{
	CBaseShaderLoader::CBaseShaderLoader() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CBaseShaderLoader::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem, 
										  TPtr<IShaderCompiler> pShaderCompiler)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pResourceManager || !pGraphicsContext || !pFileSystem || !pShaderCompiler)
		{
			return RC_INVALID_ARGS;
		}

		mIsInitialized = true;

		mpResourceManager = pResourceManager;

		mpFileSystem = pFileSystem;

		mpGraphicsContext = pGraphicsContext;

		mpShaderCompiler = pShaderCompiler;

		return RC_OK;
	}


	static E_RESULT_CODE CompileShader(const IShaderCompiler* pShaderCompiler, const std::string& resourceName,
		IShader* pShader, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem)
	{
		TDE2_PROFILER_SCOPE("CBaseShaderLoader::CompileShader");

		E_RESULT_CODE result = RC_OK;

		/// load source code
		TResult<TFileEntryId> shaderFileId = pFileSystem->Open<ITextFileReader>(resourceName);

		if (shaderFileId.HasError())
		{
			LOG_WARNING(std::string("[Shader Loader] Could not load the specified shader (").append(resourceName).append("), load default one instead..."));

			E_DEFAULT_SHADER_TYPE shaderType = CBaseGraphicsObjectManager::GetDefaultShaderTypeByName(resourceName);

			/// \note can't load file with the shader, so load default one
			return pShader->Compile(pShaderCompiler, pGraphicsContext->GetGraphicsObjectManager()->GetDefaultShaderCode(shaderType));
		}

		ITextFileReader* pShaderFileReader = dynamic_cast<ITextFileReader*>(pFileSystem->Get<ITextFileReader>(shaderFileId.Get()));

		std::string shaderSourceCode = pShaderFileReader->ReadToEnd();

		if ((result = pShaderFileReader->Close()) != RC_OK)
		{
			return result;
		}

		/// parse it and compile needed variant
		if ((result = pShader->Compile(pShaderCompiler, shaderSourceCode)) != RC_OK)
		{
			LOG_WARNING(std::string("[Shader Loader] Could not load the specified shader (").append(resourceName).append("), load default one instead..."));

			E_DEFAULT_SHADER_TYPE shaderType = CBaseGraphicsObjectManager::GetDefaultShaderTypeByName(resourceName);

			/// \note can't load file with the shader, so load default one
			return pShader->Compile(pShaderCompiler, pGraphicsContext->GetGraphicsObjectManager()->GetDefaultShaderCode(shaderType));
		}

		return result;
	}


	E_RESULT_CODE CBaseShaderLoader::LoadResource(IResource* pResource) const
	{
		TDE2_PROFILER_SCOPE("CBaseShaderLoader::LoadResource");

		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		IShader* pShader = dynamic_cast<IShader*>(pResource);

		if (!pShader)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = RC_OK;

		/// \note If there is meta information within the manifest try to read precompiled shader first
		const TShaderParameters* pShaderMetaInfo = dynamic_cast<const TShaderParameters*>(mpResourceManager->GetResourceMeta(pResource->GetName()));
		if (!pShaderMetaInfo)
		{
			return CompileShader(mpShaderCompiler.Get(), pResource->GetName(), pShader, mpGraphicsContext, mpFileSystem);
		}

		return RC_FAIL;
	}

	TypeId CBaseShaderLoader::GetResourceTypeId() const
	{
		return IShader::GetTypeId();
	}


	TDE2_API IResourceLoader* CreateBaseShaderLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext,IFileSystem* pFileSystem, 
													 TPtr<IShaderCompiler> pShaderCompiler, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceLoader, CBaseShaderLoader, result, pResourceManager, pGraphicsContext, pFileSystem, pShaderCompiler);
	}
}