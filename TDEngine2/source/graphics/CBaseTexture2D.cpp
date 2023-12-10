#include "../../include/graphics/CBaseTexture2D.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/IJobManager.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/utils/CFileLogger.h"
#define META_EXPORT_GRAPHICS_SECTION
#include "../../include/metadata.h"
#include "../../include/editor/CPerfProfiler.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define TINYEXR_USE_MINIZ 0
#include "zlib.h"
#define TINYEXR_IMPLEMENTATION
#include <tinyexr.h>
#include <string>


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
		CBaseResource(), mTextureSamplerParams(), mCurrTextureHandle(TTextureHandleId::Invalid)
	{
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
		
		mTextureSamplerParams = params.mTexSamplerDesc;
		mTextureSamplerParams.mUseMipMaps = params.mNumOfMipLevels > 1;

		auto pGraphicsObjectManager = pGraphicsContext->GetGraphicsObjectManager();

		TInitTextureImplParams createTextureParams{};
		createTextureParams.mWidth = params.mWidth;
		createTextureParams.mHeight = params.mHeight;
		createTextureParams.mFormat = params.mFormat;
		createTextureParams.mNumOfMipLevels = params.mNumOfMipLevels;
		createTextureParams.mNumOfSamples = params.mNumOfSamples;
		createTextureParams.mSamplingQuality = params.mSamplingQuality;
		createTextureParams.mType = E_TEXTURE_IMPL_TYPE::TEXTURE_2D;
		createTextureParams.mUsageType = E_TEXTURE_IMPL_USAGE_TYPE::STATIC;
		createTextureParams.mBindFlags = E_BIND_GRAPHICS_TYPE::BIND_SHADER_RESOURCE;
		createTextureParams.mName = mName;

		if (params.mIsWriteable)
		{
			createTextureParams.mBindFlags = createTextureParams.mBindFlags | E_BIND_GRAPHICS_TYPE::BIND_UNORDERED_ACCESS;
		}

		auto createTextureResult = pGraphicsObjectManager->CreateTexture(createTextureParams);
		if (createTextureResult.HasError())
		{
			return createTextureResult.GetError();
		}

		mCurrTextureHandle = createTextureResult.Get();

		mIsInitialized = true;

		return RC_OK;
	}

	void CBaseTexture2D::Bind(U32 slot)
	{
		if (mCurrTextureSamplerHandle == TTextureSamplerId::Invalid)
		{
			mCurrTextureSamplerHandle = GetTextureSampleHandle(mpGraphicsContext, mTextureSamplerParams);
		}

		mpGraphicsContext->SetSampler(slot, mCurrTextureSamplerHandle);
		mpGraphicsContext->SetTexture(slot, mCurrTextureHandle);
	}

	E_RESULT_CODE CBaseTexture2D::Reset()
	{
		auto pGraphicsObjectManager = mpGraphicsContext->GetGraphicsObjectManager();

		E_RESULT_CODE result = pGraphicsObjectManager->DestroyTexture(mCurrTextureHandle);
		mCurrTextureHandle = TTextureHandleId::Invalid;

		mIsInitialized = false;

		return result;
	}
	
	E_RESULT_CODE CBaseTexture2D::WriteData(const TRectI32& regionRect, const U8* pData)
	{
		return mpGraphicsContext->UpdateTexture2D(mCurrTextureHandle, 0, regionRect, pData, 0);
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
		auto pGraphicsObjectManager = mpGraphicsContext->GetGraphicsObjectManager();
		auto pTexture = pGraphicsObjectManager->GetTexturePtr(mCurrTextureHandle);

		return pTexture ? pTexture->GetParams().mWidth : 0;
	}

	U32 CBaseTexture2D::GetHeight() const
	{
		auto pGraphicsObjectManager = mpGraphicsContext->GetGraphicsObjectManager();
		auto pTexture = pGraphicsObjectManager->GetTexturePtr(mCurrTextureHandle);

		return pTexture ? pTexture->GetParams().mHeight : 0;
	}

	E_FORMAT_TYPE CBaseTexture2D::GetFormat() const
	{
		auto pGraphicsObjectManager = mpGraphicsContext->GetGraphicsObjectManager();
		auto pTexture = pGraphicsObjectManager->GetTexturePtr(mCurrTextureHandle);

		return pTexture ? pTexture->GetParams().mFormat : E_FORMAT_TYPE::FT_UNKNOWN;
	}

	TRectF32 CBaseTexture2D::GetNormalizedTextureRect() const
	{
		return { 0.0f, 0.0f, 1.0f, 1.0f };
	}

	std::vector<U8> CBaseTexture2D::GetInternalData()
	{
		auto pGraphicsObjectManager = mpGraphicsContext->GetGraphicsObjectManager();
		
		auto pTexture = pGraphicsObjectManager->GetTexturePtr(mCurrTextureHandle);
		if (!pTexture)
		{
			TDE2_ASSERT(false);
			return {};
		}

		return pTexture->ReadBytes(0);
	}

	TTextureHandleId CBaseTexture2D::GetHandle() const
	{
		return mCurrTextureHandle;
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


	ITexture2D* CreateTexture2D(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, const TTexture2DParameters& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ITexture2D, CBaseTexture2D, result, pResourceManager, pGraphicsContext, name, params);
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


	enum class E_TEXTURE_FORMAT_TYPE
	{
		DEFAULT,
		HDR,
		EXR
	};


	static E_TEXTURE_FORMAT_TYPE GetTypeFromFilename(const std::string& filename)
	{
		if (Wrench::StringUtils::EndsWith(filename, ".exr"))
		{
			return E_TEXTURE_FORMAT_TYPE::EXR;
		}
		
		if (stbi_is_hdr(filename.c_str()))
		{
			return E_TEXTURE_FORMAT_TYPE::EXR;
		}

		return E_TEXTURE_FORMAT_TYPE::DEFAULT;
	}


	E_RESULT_CODE CBaseTexture2DLoader::LoadResource(IResource* pResource) const
	{
		TDE2_PROFILER_SCOPE("CBaseTexture2DLoader::LoadResource");

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

		// \todo Replace all types of textures with internal engine format
		const E_TEXTURE_FORMAT_TYPE textureFormatType = GetTypeFromFilename(filename);

		switch (textureFormatType)
		{
			case E_TEXTURE_FORMAT_TYPE::DEFAULT:
			case E_TEXTURE_FORMAT_TYPE::HDR:
				/// \todo Reimplement this later using reading from in-memory block rather than using explicit file I/O
				if (!stbi_info(filename.c_str(), &width, &height, &format))
				{
					return RC_FILE_NOT_FOUND;
				}
				break;
		}

		auto loadTextureRoutine = [pResource, pJobManager, textureFormatType, w = width, h = height, fmt = format, filename, this](auto&&)
		{
			TDE2_PROFILER_SCOPE("LoadTextureRoutineJob");

			U8* pTextureData = nullptr;

			I32 width = w;
			I32 height = h;
			I32 format = fmt;

			F32* pExrData = nullptr;
			const C8* pExrErrorMessage = nullptr;

			switch (textureFormatType)
			{
				case E_TEXTURE_FORMAT_TYPE::EXR:
					if (TINYEXR_SUCCESS != LoadEXR(&pExrData, &width, &height, filename.c_str(), &pExrErrorMessage))
					{
						LOG_ERROR(Wrench::StringUtils::Format("[BaseTextureLoader] EXR texture loading failed, message: {0}", pExrErrorMessage));
						FreeEXRErrorMessage(pExrErrorMessage);
						TDE2_ASSERT(false);
						return;
					}
					
					pTextureData = reinterpret_cast<U8*>(pExrData);
					format = 4; // \note RGBA only
					break;

				case E_TEXTURE_FORMAT_TYPE::HDR:
					pTextureData = reinterpret_cast<U8*>(stbi_loadf(filename.c_str(), &width, &height, &format, (format < 3 ? format : 4))); /// D3D11 doesn't work with 24 bits textures
					break;

				default:
					pTextureData = stbi_load(filename.c_str(), &width, &height, &format, (format < 3 ? format : 4)); /// D3D11 doesn't work with 24 bits textures
					break;
			}

			if (!pTextureData)
			{
				return;
			}

			E_FORMAT_TYPE internalFormat = (E_TEXTURE_FORMAT_TYPE::DEFAULT != textureFormatType) ? FT_FLOAT4 : FT_NORM_UBYTE4;

			switch (format)
			{
				case 1:
					internalFormat = (E_TEXTURE_FORMAT_TYPE::DEFAULT != textureFormatType) ? FT_FLOAT1 : FT_NORM_UBYTE1;
					break;
				case 2:
					internalFormat = (E_TEXTURE_FORMAT_TYPE::DEFAULT != textureFormatType) ? FT_FLOAT2 : FT_NORM_UBYTE2;
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

		pJobManager->SubmitJob(nullptr, loadTextureRoutine, { E_JOB_PRIORITY_TYPE::NORMAL, true });
		
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


	/*!
		\brief CBaseTexture2DFactory's definition
	*/

	CBaseTexture2DFactory::CBaseTexture2DFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CBaseTexture2DFactory::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pGraphicsContext || !pResourceManager)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;
		mpGraphicsContext = pGraphicsContext;

		mIsInitialized = true;

		return RC_OK;
	}

	IResource* CBaseTexture2DFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		const TTexture2DParameters& texParams = static_cast<const TTexture2DParameters&>(params);

		return dynamic_cast<IResource*>(CreateTexture2D(mpResourceManager, mpGraphicsContext, name, texParams, result));
	}

	IResource* CBaseTexture2DFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		const static TTexture2DParameters defaultTextureParams{ 2, 2, FT_NORM_UBYTE4, 1, 1, 0 };

		TTexture2DParameters overridenParams = defaultTextureParams;
		overridenParams.mLoadingPolicy = params.mLoadingPolicy;

		// create blank texture, which sizes equals to 2 x 2 pixels of RGBA format
		return dynamic_cast<IResource*>(CreateTexture2D(mpResourceManager, mpGraphicsContext, name, overridenParams, result));
	}

	TypeId CBaseTexture2DFactory::GetResourceTypeId() const
	{
		return ITexture2D::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateBaseTexture2DFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceFactory, CBaseTexture2DFactory, result, pResourceManager, pGraphicsContext);
	}
}