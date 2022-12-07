#include "../../include/graphics/CBaseTexture2D.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/IJobManager.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/metadata.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <string>
#include <cassert>


namespace TDEngine2
{
	struct TTextureParametersArchiveKeys
	{
		static const std::string mFilterTypeKeyId;
		static const std::string mAddressModeKeyId;
		static const std::string mMipMapsEnabledKeyId;
		static const std::string mMipMapsCountKeyId;
		static const std::string mSamplesCountKeyId;
	};

	const std::string TTextureParametersArchiveKeys::mFilterTypeKeyId = "filter_type";
	const std::string TTextureParametersArchiveKeys::mAddressModeKeyId = "address_mode";
	const std::string TTextureParametersArchiveKeys::mMipMapsEnabledKeyId = "mips_enabled";
	const std::string TTextureParametersArchiveKeys::mMipMapsCountKeyId = "mips_count";
	const std::string TTextureParametersArchiveKeys::mSamplesCountKeyId = "samples_count";


	/*!
		\note The declaration of TTexture2DParameters is placed at ITexture2D.h
	*/

	TTexture2DParameters::TTexture2DParameters(U32 width, U32 height, E_FORMAT_TYPE format, U32 mipLevelsCount, U32 samplesCount, U32 samplingQuality):
		mWidth(width), mHeight(height), mFormat(format), mNumOfMipLevels(mipLevelsCount), mNumOfSamples(samplesCount), mSamplingQuality(samplingQuality)
	{
	}

	E_RESULT_CODE TTexture2DParameters::Load(IArchiveReader* pReader)
	{
		mTexSamplerDesc.mFilteringType = Meta::EnumTrait<E_TEXTURE_FILTER_TYPE>::FromString(pReader->GetString(TTextureParametersArchiveKeys::mFilterTypeKeyId));
		
		mTexSamplerDesc.mUAddressMode = Meta::EnumTrait<E_ADDRESS_MODE_TYPE>::FromString(pReader->GetString(TTextureParametersArchiveKeys::mAddressModeKeyId));
		mTexSamplerDesc.mVAddressMode = mTexSamplerDesc.mUAddressMode;
		mTexSamplerDesc.mWAddressMode = mTexSamplerDesc.mUAddressMode;

		mTexSamplerDesc.mUseMipMaps = pReader->GetBool(TTextureParametersArchiveKeys::mMipMapsEnabledKeyId, true);
		mNumOfMipLevels = mTexSamplerDesc.mUseMipMaps ? pReader->GetUInt32(TTextureParametersArchiveKeys::mMipMapsCountKeyId, 1) : 1;

		mNumOfSamples = pReader->GetUInt32(TTextureParametersArchiveKeys::mSamplesCountKeyId, 1);

		return RC_OK;
	}

	E_RESULT_CODE TTexture2DParameters::Save(IArchiveWriter* pWriter)
	{
		pWriter->SetUInt32("type_id", static_cast<U32>(TDE2_TYPE_ID(TTexture2DParameters)));

		pWriter->SetString(TTextureParametersArchiveKeys::mFilterTypeKeyId, Meta::EnumTrait<E_TEXTURE_FILTER_TYPE>::ToString(mTexSamplerDesc.mFilteringType));
		pWriter->SetString(TTextureParametersArchiveKeys::mAddressModeKeyId, Meta::EnumTrait<E_ADDRESS_MODE_TYPE>::ToString(mTexSamplerDesc.mUAddressMode));
		
		pWriter->SetBool(TTextureParametersArchiveKeys::mMipMapsEnabledKeyId, mTexSamplerDesc.mUseMipMaps);
		pWriter->SetUInt32(TTextureParametersArchiveKeys::mMipMapsCountKeyId, mNumOfMipLevels);
		
		pWriter->SetUInt32(TTextureParametersArchiveKeys::mSamplesCountKeyId, mNumOfSamples);

		return RC_OK;
	}


	/*!
		\brief CBaseTexture2D's definition
	*/

	CBaseTexture2D::CBaseTexture2D() :
		CBaseResource(), mTextureSamplerParams()
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

		mLoadingPolicy = params.mLoadingPolicy;
		
		mWidth                = params.mWidth;
		mHeight               = params.mHeight;
		mFormat               = params.mFormat;
		mNumOfMipLevels       = params.mNumOfMipLevels;
		mNumOfSamples         = params.mNumOfSamples;
		mSamplingQuality      = params.mSamplingQuality;
		mTextureSamplerParams = params.mTexSamplerDesc;
		mIsRandomlyWriteable  = params.mIsWriteable;

		mTextureSamplerParams.mUseMipMaps = mNumOfMipLevels > 1;

		mIsInitialized = true;

		return _createInternalTextureHandler(mpGraphicsContext, mWidth, mHeight, mFormat, 
											 mNumOfMipLevels, mNumOfSamples, mSamplingQuality, mIsRandomlyWriteable); /// create a texture's object within video memory using GAPI
	}

	void CBaseTexture2D::Bind(U32 slot)
	{
		if (mCurrTextureSamplerHandle == TTextureSamplerId::Invalid)
		{
			mCurrTextureSamplerHandle = GetTextureSampleHandle(mpGraphicsContext, mTextureSamplerParams);
		}

		mpGraphicsContext->BindTextureSampler(slot, mCurrTextureSamplerHandle);
	}

	void CBaseTexture2D::SetUWrapMode(const E_ADDRESS_MODE_TYPE& mode)
	{
		mTextureSamplerParams.mUAddressMode = mode;
		mCurrTextureSamplerHandle = TTextureSamplerId::Invalid; /// \note Reset current sampler to update it in Bind method later
	}

	void CBaseTexture2D::SetVWrapMode(const E_ADDRESS_MODE_TYPE& mode)
	{
		mTextureSamplerParams.mVAddressMode = mode;
		mCurrTextureSamplerHandle = TTextureSamplerId::Invalid; /// \note Reset current sampler to update it in Bind method later
	}

	void CBaseTexture2D::SetWWrapMode(const E_ADDRESS_MODE_TYPE& mode)
	{
		mTextureSamplerParams.mWAddressMode = mode;
		mCurrTextureSamplerHandle = TTextureSamplerId::Invalid; /// \note Reset current sampler to update it in Bind method later
	}

	void CBaseTexture2D::SetFilterType(const E_TEXTURE_FILTER_TYPE& type)
	{
		mTextureSamplerParams.mFilteringType = type;
		mCurrTextureSamplerHandle = TTextureSamplerId::Invalid; /// \note Reset current sampler to update it in Bind method later
	}

	U32 CBaseTexture2D::GetWidth() const
	{
		return mWidth;
	}

	U32 CBaseTexture2D::GetHeight() const
	{
		return mHeight;
	}

	E_FORMAT_TYPE CBaseTexture2D::GetFormat() const
	{
		return mFormat;
	}

	TRectF32 CBaseTexture2D::GetNormalizedTextureRect() const
	{
		return { 0.0f, 0.0f, 1.0f, 1.0f };
	}

	TTextureSamplerId CBaseTexture2D::GetTextureSampleHandle(IGraphicsContext* pGraphicsContext, const TTextureSamplerDesc& params)
	{
		IGraphicsObjectManager* pGraphicsObjectManager = pGraphicsContext->GetGraphicsObjectManager();
		assert(pGraphicsObjectManager);

		auto result = pGraphicsObjectManager->CreateTextureSampler(params);
		return result.IsOk() ? result.Get() : TTextureSamplerId::Invalid;
	}

	const TPtr<IResourceLoader> CBaseTexture2D::_getResourceLoader()
	{
		return mpResourceManager->GetResourceLoader<ITexture2D>();
	}


	CBaseTexture2DLoader::CBaseTexture2DLoader() :
		CBaseObject()
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

	E_RESULT_CODE CBaseTexture2DLoader::LoadResource(IResource* pResource) const
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		IJobManager* pJobManager = mpFileSystem->GetJobManager();
		
		I32 width = 0;
		I32 height = 0;
		I32 format = 0;

		const std::string filename = mpFileSystem->ResolveVirtualPath(pResource->GetName(), false);

		U8* pTextureData = nullptr;

		if (!stbi_info(filename.c_str(), &width, &height, &format))
		{
			return RC_FILE_NOT_FOUND;
		}

		auto loadTextureRoutine = [pResource, pJobManager, w = width, h = height, fmt = format, filename, this](auto&&)
		{
			U8* pTextureData = nullptr;

			I32 width = w;
			I32 height = h;
			I32 format = fmt;

			pTextureData = stbi_load(filename.c_str(), &width, &height, &format, (format < 3 ? format : 4));/// D3D11 doesn't work with 24 bits textures

			if (!pTextureData)
			{
				return;
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

			pJobManager->ExecuteInMainThread([pResource, pTextureData, width, height, internalFormat, this]()
			{
				/// reset old texture data
				E_RESULT_CODE result = pResource->Reset();
				if (RC_OK != result)
				{
					TDE2_ASSERT(false);
					return;
				}

				/// create new internal texture
				ITexture2D* pTextureResource = dynamic_cast<ITexture2D*>(pResource);

				const TTexture2DParameters* pTextureMetaInfo = dynamic_cast<const TTexture2DParameters*>(mpResourceManager->GetResourceMeta(pResource->GetName()));
				TTexture2DParameters textureParams = pTextureMetaInfo ? *pTextureMetaInfo : TTexture2DParameters { static_cast<U32>(width), static_cast<U32>(height), internalFormat, 1, 1, 0 };

				textureParams.mWidth  = static_cast<U32>(width);
				textureParams.mHeight = static_cast<U32>(height);
				textureParams.mFormat = internalFormat;

				/// \todo replace magic constants with proper computations
				result = pTextureResource->Init(mpResourceManager, mpGraphicsContext, pResource->GetName(), textureParams);

				if (RC_OK != result)
				{
					/// \todo restore default 2x2 black texture if something went wrong
					TDE2_ASSERT(false);
					return;
				}

				/// update subresource
				if (RC_OK != (result = pTextureResource->WriteData({ 0, 0, width, height }, pTextureData)))
				{
					TDE2_ASSERT(false);
					return;
				}

				pResource->SetState(E_RESOURCE_STATE_TYPE::RST_LOADED);

				/// \note Update parameters of the texture
				if (pTextureMetaInfo)
				{
					pTextureResource->SetUWrapMode(textureParams.mTexSamplerDesc.mUAddressMode);
					pTextureResource->SetVWrapMode(textureParams.mTexSamplerDesc.mVAddressMode);
					pTextureResource->SetWWrapMode(textureParams.mTexSamplerDesc.mWAddressMode);

					pTextureResource->SetFilterType(textureParams.mTexSamplerDesc.mFilteringType);
				}

				stbi_image_free(pTextureData);
			});
		};

		if (E_RESOURCE_LOADING_POLICY::STREAMING != pResource->GetLoadingPolicy())
		{
			loadTextureRoutine(TJobArgs{});
			return RC_OK;
		}

		pJobManager->SubmitJob(nullptr, loadTextureRoutine);
		
		return RC_OK;
	}

	TypeId CBaseTexture2DLoader::GetResourceTypeId() const
	{
		return ITexture2D::GetTypeId();
	}


	TDE2_API IResourceLoader* CreateBaseTexture2DLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceLoader, CBaseTexture2DLoader, result, pResourceManager, pGraphicsContext, pFileSystem);
	}
}