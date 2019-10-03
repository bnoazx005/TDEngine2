#include "./../../include/graphics/CBaseCubemapTexture.h"
#include "./../../include/core/IGraphicsContext.h"
#include "./../../include/core/IResourceManager.h"
#include "./../../include/core/IFileSystem.h"
#include <stb_image.h>
#include <string>


namespace TDEngine2
{
	CBaseCubemapTexture::CBaseCubemapTexture() :
		CBaseResource()
	{
	}

	E_RESULT_CODE CBaseCubemapTexture::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name)
	{
		E_RESULT_CODE result = _init(pResourceManager, name);

		if (result != RC_OK)
		{
			return result;
		}

		if (!pGraphicsContext)
		{
			return RC_INVALID_ARGS;
		}

		mpGraphicsContext = pGraphicsContext;

		mIsInitialized = true;

		return RC_OK;
	}

	U32 CBaseCubemapTexture::GetWidth() const
	{
		return mWidth;
	}

	U32 CBaseCubemapTexture::GetHeight() const
	{
		return mHeight;
	}

	E_FORMAT_TYPE CBaseCubemapTexture::GetFormat() const
	{
		return mFormat;
	}


	CBaseCubemapTextureLoader::CBaseCubemapTextureLoader() :
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CBaseCubemapTextureLoader::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pResourceManager || !pGraphicsContext || !pFileSystem)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;

		mpFileSystem = pFileSystem;

		mpGraphicsContext = pGraphicsContext;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CBaseCubemapTextureLoader::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CBaseCubemapTextureLoader::LoadResource(IResource* pResource) const
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		TDE2_UNIMPLEMENTED();

		return RC_OK;
	}

	U32 CBaseCubemapTextureLoader::GetResourceTypeId() const
	{
		return CBaseCubemapTexture::GetTypeId();
	}


	TDE2_API IResourceLoader* CreateBaseCubemapTextureLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		CBaseCubemapTextureLoader* pCubemapTextureLoaderInstance = new (std::nothrow) CBaseCubemapTextureLoader();

		if (!pCubemapTextureLoaderInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pCubemapTextureLoaderInstance->Init(pResourceManager, pGraphicsContext, pFileSystem);

		if (result != RC_OK)
		{
			delete pCubemapTextureLoaderInstance;

			pCubemapTextureLoaderInstance = nullptr;
		}

		return pCubemapTextureLoaderInstance;
	}
}