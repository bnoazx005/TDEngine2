#include "../../include/graphics/CBaseCubemapTexture.h"
#include "../../include/graphics/CBaseTexture2D.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/IJobManager.h"
#include "../../include/platform/CYAMLFile.h"
#include "../../include/utils/Utils.h"
#include "stringUtils.hpp"
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

		mFacesLoadingStatusBitset = 0;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CBaseCubemapTexture::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, const TTexture2DParameters& params)
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

		mLoadingPolicy = params.mLoadingPolicy;

		mWidth                = params.mWidth;
		mHeight               = params.mHeight;
		mFormat               = params.mFormat;
		mNumOfMipLevels       = params.mNumOfMipLevels;
		mNumOfSamples         = params.mNumOfSamples;
		mSamplingQuality      = params.mSamplingQuality;
		mTextureSamplerParams = params.mTexSamplerDesc;

		mIsInitialized = true;

		return _createInternalTextureHandler(mpGraphicsContext, mWidth, mHeight, mFormat,
											 mNumOfMipLevels, mNumOfSamples, mSamplingQuality); /// create a texture's object within video memory using GAPI
	}

	void CBaseCubemapTexture::Bind(U32 slot)
	{
		if (mCurrTextureSamplerHandle == TTextureSamplerId::Invalid)
		{
			mCurrTextureSamplerHandle = CBaseTexture2D::GetTextureSampleHandle(mpGraphicsContext, mTextureSamplerParams);
		}

		mpGraphicsContext->BindTextureSampler(slot, mCurrTextureSamplerHandle);
	}

	void CBaseCubemapTexture::SetUWrapMode(const E_ADDRESS_MODE_TYPE& mode)
	{
		mTextureSamplerParams.mUAddressMode = mode;
	}

	void CBaseCubemapTexture::SetVWrapMode(const E_ADDRESS_MODE_TYPE& mode)
	{
		mTextureSamplerParams.mVAddressMode = mode;
	}

	void CBaseCubemapTexture::SetWWrapMode(const E_ADDRESS_MODE_TYPE& mode)
	{
		mTextureSamplerParams.mWAddressMode = mode;
	}

	void CBaseCubemapTexture::SetFilterType(const E_TEXTURE_FILTER_TYPE& type)
	{
		TDE2_UNIMPLEMENTED();
		//mTextureSamplerParams.
	}

	void CBaseCubemapTexture::MarkFaceAsLoaded(E_CUBEMAP_FACE face)
	{		
		mFacesLoadingStatusBitset |= (1 << static_cast<U8>(face));
		if (mFacesLoadingStatusBitset >= 0x3f)
		{
			mState = E_RESOURCE_STATE_TYPE::RST_LOADED;
		}
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

	TRectF32 CBaseCubemapTexture::GetNormalizedTextureRect() const
	{
		return { 0.0f, 0.0f, 1.0f, 1.0f };
	}

	const TPtr<IResourceLoader> CBaseCubemapTexture::_getResourceLoader()
	{
		return mpResourceManager->GetResourceLoader<ICubemapTexture>();
	}


	CBaseCubemapTextureLoader::CBaseCubemapTextureLoader() :
		CBaseObject()
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

	E_RESULT_CODE CBaseCubemapTextureLoader::LoadResource(IResource* pResource) const
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		auto metaResult = _readCubemapInfo(pResource->GetName() + ".cubemap.info");

		if (metaResult.HasError())
		{
			return metaResult.GetError();
		}

		/// reset old texture data
		E_RESULT_CODE result = pResource->Reset();

		if (result != RC_OK)
		{
			return result;
		}

		/// \note create new internal texture
		ICubemapTexture* pTextureResource = dynamic_cast<ICubemapTexture*>(pResource);

		TCubemapMetaInfo metaInfo = metaResult.Get();

		TTexture2DParameters cubemapParams { metaInfo.mWidth, metaInfo.mHeight, metaInfo.mFormat, metaInfo.mMipLevelsCount, 1, 0 };
		cubemapParams.mLoadingPolicy = pResource->GetLoadingPolicy();

		/// \todo replace magic constants with proper computations
		if ((result = pTextureResource->Init(mpResourceManager, mpGraphicsContext, pResource->GetName(), cubemapParams)) != RC_OK)
		{
			return result;
		}

		/// \note load cubemap's faces
		for (U8 i = 0; i < 6; ++i)
		{
			if ((result = _loadFaceTexture(pTextureResource, metaInfo, static_cast<E_CUBEMAP_FACE>(i))) != RC_OK)
			{
				return result;
			}
		}

		return RC_OK;
	}

	TypeId CBaseCubemapTextureLoader::GetResourceTypeId() const
	{
		return ICubemapTexture::GetTypeId();
	}

	TResult<CBaseCubemapTextureLoader::TCubemapMetaInfo> CBaseCubemapTextureLoader::_readCubemapInfo(const std::string& filename) const
	{
		TResult<TFileEntryId> fileReadingResult = mpFileSystem->Open<IYAMLFileReader>(filename);

		if (fileReadingResult.HasError())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(fileReadingResult.GetError());
		}

		auto pYAMLFileReader = mpFileSystem->Get<IYAMLFileReader>(fileReadingResult.Get());

		E_RESULT_CODE result = RC_OK;

		TCubemapMetaInfo cubemapMeta;

		cubemapMeta.mFormat = CFormatUtils::GetFormatFromString(pYAMLFileReader->GetString("format"));

		cubemapMeta.mWidth          = pYAMLFileReader->GetUInt32("width");
		cubemapMeta.mHeight         = pYAMLFileReader->GetUInt32("height");
		cubemapMeta.mMipLevelsCount = pYAMLFileReader->GetUInt16("mip-map-levels");

		if ((result = pYAMLFileReader->BeginGroup("faces")) != RC_OK)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		for (U8 i = 0; i < 6; ++i)
		{
			cubemapMeta.mFaceTexturePaths[static_cast<U8>(E_CUBEMAP_FACE::POSITIVE_X) + i] = pYAMLFileReader->GetString(Wrench::StringUtils::GetEmptyStr());
		}

		if ((result = pYAMLFileReader->EndGroup()) != RC_OK)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		pYAMLFileReader->Close();

		return Wrench::TOkValue<TCubemapMetaInfo>(cubemapMeta);
	}

	E_RESULT_CODE CBaseCubemapTextureLoader::_loadFaceTexture(ICubemapTexture* pCubemapTexture, const TCubemapMetaInfo& info, E_CUBEMAP_FACE face) const
	{
		I32 width, height, format;

		const std::string& filename = mpFileSystem->ResolveVirtualPath(info.mFaceTexturePaths[static_cast<U8>(face)], false);

		if (!stbi_info(filename.c_str(), &width, &height, &format))
		{
			return RC_FILE_NOT_FOUND;
		}

		if (info.mWidth != width || info.mHeight != height)
		{
			return RC_FAIL;
		}

		IJobManager* pJobManager = mpFileSystem->GetJobManager();

		auto loadTextureRoutine = [this, pJobManager, pCubemapTexture, info, face, filename, w = width, h = height, fmt = format] 
		{
			I32 width = w, height = h, format = fmt;

			U8* pTextureData = stbi_load(filename.c_str(), &width, &height, &format, (format < 3 ? format : 4));/// D3D11 doesn't work with 24 bits textures

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

			if (!pTextureData || (internalFormat != info.mFormat))
			{
				TDE2_ASSERT(false);
				return;
			}

			pJobManager->ExecuteInMainThread([pTextureData, pCubemapTexture, face, width, height]
			{
				E_RESULT_CODE result = RC_OK;

				/// update subresource
				if ((result = pCubemapTexture->WriteData(face, { 0, 0, width, height }, pTextureData)) != RC_OK)
				{
					TDE2_ASSERT(false);
					return;
				}

				pCubemapTexture->MarkFaceAsLoaded(face);

				stbi_image_free(pTextureData);
			});
		};

		if (auto pResource = dynamic_cast<IResource*>(pCubemapTexture))
		{
			if (E_RESOURCE_LOADING_POLICY::STREAMING != pResource->GetLoadingPolicy())
			{
				loadTextureRoutine();
				return RC_OK;
			}

			pJobManager->SubmitJob(std::function<void()>(loadTextureRoutine));
		}
		
		return RC_OK;
	}


	TDE2_API IResourceLoader* CreateBaseCubemapTextureLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceLoader, CBaseCubemapTextureLoader, result, pResourceManager, pGraphicsContext, pFileSystem);
	}
}