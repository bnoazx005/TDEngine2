#include "./../../include/graphics/CBaseShaderLoader.h"
#include "./../../include/graphics/CBaseShader.h"
#include "./../../include/core/IFileSystem.h"
#include "./../../include/core/IFile.h"
#include "./../../include/platform/CTextFileReader.h"
#include "./../../include/graphics/IShaderCompiler.h"
#include "./../../include/core/IGraphicsContext.h"
#include <string>


namespace TDEngine2
{
	CBaseShaderLoader::CBaseShaderLoader() :
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CBaseShaderLoader::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem, 
										  const IShaderCompiler* pShaderCompiler)
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

	E_RESULT_CODE CBaseShaderLoader::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CBaseShaderLoader::LoadResource(IResource* pResource) const
	{
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

		/// load source code
		TResult<TFileEntryId> shaderFileId = mpFileSystem->Open<CTextFileReader>(pResource->GetName());

		if (shaderFileId.HasError())
		{
			return shaderFileId.GetError();
		}

		ITextFileReader* pShaderFileReader = dynamic_cast<ITextFileReader*>(mpFileSystem->Get<CTextFileReader>(shaderFileId.Get()));
		
		std::string shaderSourceCode = pShaderFileReader->ReadToEnd();

		if ((result = pShaderFileReader->Close()) != RC_OK)
		{
			return result;
		}

		/// parse it and compile needed variant
		if ((result = pShader->Compile(mpShaderCompiler, shaderSourceCode)) != RC_OK)
		{
			return result;
		}

		return result;
	}

	U32 CBaseShaderLoader::GetResourceTypeId() const
	{
		return CBaseShader::GetTypeId();
	}


	TDE2_API IResourceLoader* CreateBaseShaderLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext,IFileSystem* pFileSystem, 
													 const IShaderCompiler* pShaderCompiler, E_RESULT_CODE& result)
	{
		CBaseShaderLoader* pShaderLoaderInstance = new (std::nothrow) CBaseShaderLoader();

		if (!pShaderLoaderInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pShaderLoaderInstance->Init(pResourceManager, pGraphicsContext, pFileSystem, pShaderCompiler);

		if (result != RC_OK)
		{
			delete pShaderLoaderInstance;

			pShaderLoaderInstance = nullptr;
		}

		return pShaderLoaderInstance;
	}
}