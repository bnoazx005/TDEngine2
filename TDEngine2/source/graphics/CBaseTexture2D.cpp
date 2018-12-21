#include "./../../include/graphics/CBaseTexture2D.h"
#include "./../../include/core/IGraphicsContext.h"
#include "./../../include/core/IResourceManager.h"
#include "./../../include/core/IFileSystem.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <string>


namespace TDEngine2
{
	/*!
		\note The declaration of TTexture2DParameters is placed at ITexture2D.h
	*/

	TTexture2DParameters::TTexture2DParameters(U32 width, U32 height, E_FORMAT_TYPE format, U32 mipLevelsCount, U32 samplesCount, U32 samplingQuality):
		mWidth(width), mHeight(height), mFormat(format), mNumOfMipLevels(mipLevelsCount), mNumOfSamples(samplesCount), mSamplingQuality(samplingQuality)
	{
	}


	CBaseTexture2D::CBaseTexture2D() :
		CBaseResource()
	{
	}

	E_RESULT_CODE CBaseTexture2D::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name)
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

	E_RESULT_CODE CBaseTexture2D::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, const TTexture2DParameters& params)
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
		
		mWidth           = params.mWidth;
		mHeight          = params.mHeight;
		mFormat          = params.mFormat;
		mNumOfMipLevels  = params.mNumOfMipLevels;
		mNumOfSamples    = params.mNumOfSamples;
		mSamplingQuality = params.mSamplingQuality;

		mIsInitialized = true;

		return _createInternalTextureHandler(mpGraphicsContext, mWidth, mHeight, mFormat, 
											 mNumOfMipLevels, mNumOfSamples, mSamplingQuality); /// create a texture's object within video memory using GAPI
	}

	U32 CBaseTexture2D::GetWidth() const
	{
		return mWidth;
	}

	U32 CBaseTexture2D::GetHeight() const
	{
		return mHeight;
	}


	CBaseTexture2DLoader::CBaseTexture2DLoader() :
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CBaseTexture2DLoader::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem)
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

	E_RESULT_CODE CBaseTexture2DLoader::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CBaseTexture2DLoader::LoadResource(IResource* pResource) const
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		I32 width  = 0;
		I32 height = 0;
		I32 format = 0;

		std::string filename = mpFileSystem->ResolveVirtualPath(pResource->GetName());
		
		U8* pTextureData = nullptr;
		
		if (!stbi_info(filename.c_str(), &width, &height, &format))
		{
			return RC_FILE_NOT_FOUND;
		}

		pTextureData = stbi_load(filename.c_str(), &width, &height, &format, (format < 3 ? format : 4));/// D3D11 doesn't work with 24 bits textures

		if (!pTextureData)
		{
			return RC_FAIL;
		}
		
		E_FORMAT_TYPE internalFormat = FT_NORM_UBYTE4;

		switch (format)
		{
			case 1:
				internalFormat = FT_NORM_UBYTE1;
				break;
			case 2:
				internalFormat = FT_NORM_UBYTE2;
				break;
		}

		/// reset old texture data
		E_RESULT_CODE result = pResource->Reset();

		if (result != RC_OK)
		{
			return result;
		}

		/// create new internal texture
		ITexture2D* pTextureResource = dynamic_cast<ITexture2D*>(pResource);
		
		/// \todo replace magic constants with proper computations
		result = pTextureResource->Init(mpResourceManager, mpGraphicsContext, pResource->GetName(), { static_cast<U32>(width), static_cast<U32>(height), internalFormat, 1, 1, 0 });

		if (result != RC_OK)
		{
			return result;
		}

		/// update subresource
		if ((result = pTextureResource->WriteData({ 0, 0, width, height }, pTextureData)) != RC_OK)
		{
			return result;
		}

		stbi_image_free(pTextureData);
		
		return RC_OK;
	}

	U32 CBaseTexture2DLoader::GetResourceTypeId() const
	{
		return CBaseTexture2D::GetTypeId();
	}


	TDE2_API IResourceLoader* CreateBaseTexture2DLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		CBaseTexture2DLoader* pTexture2DLoaderInstance = new (std::nothrow) CBaseTexture2DLoader();

		if (!pTexture2DLoaderInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pTexture2DLoaderInstance->Init(pResourceManager, pGraphicsContext, pFileSystem);

		if (result != RC_OK)
		{
			delete pTexture2DLoaderInstance;

			pTexture2DLoaderInstance = nullptr;
		}

		return pTexture2DLoaderInstance;
	}
}