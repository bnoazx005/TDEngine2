#include "../../include/graphics/CBaseTexture3D.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/IJobManager.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#define META_EXPORT_GRAPHICS_SECTION
#include "../../include/metadata.h"
#include "../../include/editor/CPerfProfiler.h"
#include <stb_image.h>
#include <string>


namespace TDEngine2
{
	struct TTexture3DParametersArchiveKeys
	{
		static const std::string mSheetRowsCountKeyId;
		static const std::string mSheetColsCountKeyId;
	};

	const std::string TTexture3DParametersArchiveKeys::mSheetRowsCountKeyId = "input_sheet_rows_count";
	const std::string TTexture3DParametersArchiveKeys::mSheetColsCountKeyId = "input_sheet_cols_count";


	/*!
		\note The declaration of TTexture3DParameters is placed at ITexture3D.h
	*/

	TTexture3DParameters::TTexture3DParameters(U32 width, U32 height, U32 depth, E_FORMAT_TYPE format, U32 mipLevelsCount, U32 samplesCount, U32 samplingQuality):
		TTexture2DParameters(width, height, format, mipLevelsCount, samplesCount, samplingQuality), mDepth(depth)
	{
	}

	E_RESULT_CODE TTexture3DParameters::Load(IArchiveReader* pReader)
	{
		E_RESULT_CODE result = TTexture2DParameters::Load(pReader);

		mInputSheetRowsCount = pReader->GetUInt32(TTexture3DParametersArchiveKeys::mSheetRowsCountKeyId);
		mInputSheetColsCount = pReader->GetUInt32(TTexture3DParametersArchiveKeys::mSheetColsCountKeyId);

		return result;
	}

	E_RESULT_CODE TTexture3DParameters::Save(IArchiveWriter* pWriter)
	{
		E_RESULT_CODE result = TTexture2DParameters::Save(pWriter);
		
		result = result | pWriter->SetUInt32("type_id", static_cast<U32>(TDE2_TYPE_ID(TTexture3DParameters)));
		result = result | pWriter->SetUInt32(TTexture3DParametersArchiveKeys::mSheetRowsCountKeyId, mInputSheetRowsCount);
		result = result | pWriter->SetUInt32(TTexture3DParametersArchiveKeys::mSheetColsCountKeyId, mInputSheetColsCount);

		return result;
	}


	/*!
		\brief CBaseTexture3D's definition
	*/

	CBaseTexture3D::CBaseTexture3D() :
		CBaseResource(), mTextureSamplerParams(), mCurrTextureHandle(TTextureHandleId::Invalid)
	{
	}

	E_RESULT_CODE CBaseTexture3D::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, const TTexture3DParameters& params)
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
		createTextureParams.mDepth = params.mDepth;
		createTextureParams.mNumOfMipLevels = params.mNumOfMipLevels;
		createTextureParams.mNumOfSamples = params.mNumOfSamples;
		createTextureParams.mSamplingQuality = params.mSamplingQuality;
		createTextureParams.mIsWriteable = params.mIsWriteable;
		createTextureParams.mType = E_TEXTURE_IMPL_TYPE::TEXTURE_3D;
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

	void CBaseTexture3D::Bind(U32 slot)
	{
		if (mCurrTextureSamplerHandle == TTextureSamplerId::Invalid)
		{
			mCurrTextureSamplerHandle = GetTextureSampleHandle(mpGraphicsContext, mTextureSamplerParams);
		}

		mpGraphicsContext->SetSampler(slot, mCurrTextureSamplerHandle);
		mpGraphicsContext->SetTexture(slot, mCurrTextureHandle, mIsWriteEnabled);
	}

	E_RESULT_CODE CBaseTexture3D::Reset()
	{
		auto pGraphicsObjectManager = mpGraphicsContext->GetGraphicsObjectManager();

		E_RESULT_CODE result = pGraphicsObjectManager->DestroyTexture(mCurrTextureHandle);
		mCurrTextureHandle = TTextureHandleId::Invalid;

		mIsInitialized = false;

		return result;
	}
	
	E_RESULT_CODE CBaseTexture3D::WriteData(const TRectI32& regionRect, U32 depthFirstSlice, U32 depthLastSlice, const U8* pData)
	{
		return mpGraphicsContext->UpdateTexture3D(mCurrTextureHandle, depthFirstSlice, depthLastSlice, regionRect, pData, 0);
	}

	void CBaseTexture3D::SetWriteable(bool value)
	{
		mIsWriteEnabled = value;
	}

	void CBaseTexture3D::SetUWrapMode(const E_ADDRESS_MODE_TYPE& mode)
	{
		mTextureSamplerParams.mUAddressMode = mode;
		mCurrTextureSamplerHandle = TTextureSamplerId::Invalid; /// \note Reset current sampler to update it in Bind method later
	}

	void CBaseTexture3D::SetVWrapMode(const E_ADDRESS_MODE_TYPE& mode)
	{
		mTextureSamplerParams.mVAddressMode = mode;
		mCurrTextureSamplerHandle = TTextureSamplerId::Invalid; /// \note Reset current sampler to update it in Bind method later
	}

	void CBaseTexture3D::SetWWrapMode(const E_ADDRESS_MODE_TYPE& mode)
	{
		mTextureSamplerParams.mWAddressMode = mode;
		mCurrTextureSamplerHandle = TTextureSamplerId::Invalid; /// \note Reset current sampler to update it in Bind method later
	}

	void CBaseTexture3D::SetFilterType(const E_TEXTURE_FILTER_TYPE& type)
	{
		mTextureSamplerParams.mFilteringType = type;
		mCurrTextureSamplerHandle = TTextureSamplerId::Invalid; /// \note Reset current sampler to update it in Bind method later
	}

	U32 CBaseTexture3D::GetWidth() const
	{
		auto pGraphicsObjectManager = mpGraphicsContext->GetGraphicsObjectManager();
		auto pTexture = pGraphicsObjectManager->GetTexturePtr(mCurrTextureHandle);

		return pTexture ? pTexture->GetParams().mWidth : 0;
	}

	U32 CBaseTexture3D::GetHeight() const
	{
		auto pGraphicsObjectManager = mpGraphicsContext->GetGraphicsObjectManager();
		auto pTexture = pGraphicsObjectManager->GetTexturePtr(mCurrTextureHandle);

		return pTexture ? pTexture->GetParams().mHeight : 0;
	}

	U32 CBaseTexture3D::GetDepth() const
	{
		auto pGraphicsObjectManager = mpGraphicsContext->GetGraphicsObjectManager();
		auto pTexture = pGraphicsObjectManager->GetTexturePtr(mCurrTextureHandle);

		return pTexture ? pTexture->GetParams().mDepth : 0;
	}

	E_FORMAT_TYPE CBaseTexture3D::GetFormat() const
	{
		auto pGraphicsObjectManager = mpGraphicsContext->GetGraphicsObjectManager();
		auto pTexture = pGraphicsObjectManager->GetTexturePtr(mCurrTextureHandle);

		return pTexture ? pTexture->GetParams().mFormat : E_FORMAT_TYPE::FT_UNKNOWN;
	}

	bool CBaseTexture3D::IsWriteable() const
	{
		return mIsWriteEnabled;
	}

	TRectF32 CBaseTexture3D::GetNormalizedTextureRect() const
	{
		return { 0.0f, 0.0f, 1.0f, 1.0f };
	}

	std::vector<U8> CBaseTexture3D::GetInternalData()
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

	TTextureHandleId CBaseTexture3D::GetHandle() const
	{
		return mCurrTextureHandle;
	}

	TTextureSamplerId CBaseTexture3D::GetTextureSampleHandle(IGraphicsContext* pGraphicsContext, const TTextureSamplerDesc& params)
	{
		IGraphicsObjectManager* pGraphicsObjectManager = pGraphicsContext->GetGraphicsObjectManager();
		assert(pGraphicsObjectManager);

		auto result = pGraphicsObjectManager->CreateTextureSampler(params);
		return result.IsOk() ? result.Get() : TTextureSamplerId::Invalid;
	}

	const TPtr<IResourceLoader> CBaseTexture3D::_getResourceLoader()
	{
		return mpResourceManager->GetResourceLoader<ITexture3D>();
	}


	ITexture3D* CreateTexture3D(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, const TTexture3DParameters& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ITexture3D, CBaseTexture3D, result, pResourceManager, pGraphicsContext, name, params);
	}


	CBaseTexture3DLoader::CBaseTexture3DLoader() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CBaseTexture3DLoader::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem)
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

	E_RESULT_CODE CBaseTexture3DLoader::LoadResource(IResource* pResource) const
	{
		TDE2_PROFILER_SCOPE("CBaseTexture3DLoader::LoadResource");

		if (!mIsInitialized)
		{
			return RC_FAIL;
		}
		/*
		IJobManager* pJobManager = mpFileSystem->GetJobManager();
		
		I32 width = 0;
		I32 height = 0;
		I32 format = 0;

		const std::string filename = mpFileSystem->ResolveVirtualPath(pResource->GetName(), false);

		U8* pTextureData = nullptr;

		/// \todo Reimplement this later using reading from in-memory block rather than using explicit file I/O
		if (!stbi_info(filename.c_str(), &width, &height, &format))
		{
			return RC_FILE_NOT_FOUND;
		}

		auto loadTextureRoutine = [pResource, pJobManager, w = width, h = height, fmt = format, filename, this](auto&&)
		{
			TDE2_PROFILER_SCOPE("LoadTextureRoutineJob");

			U8* pTextureData = nullptr;

			I32 width = w;
			I32 height = h;
			I32 format = fmt;

			const bool isHDREnabled = stbi_is_hdr(filename.c_str());

			if (isHDREnabled)
			{
				pTextureData = reinterpret_cast<U8*>(stbi_loadf(filename.c_str(), &width, &height, &format, (format < 3 ? format : 4))); /// D3D11 doesn't work with 24 bits textures
			}
			else
			{
				pTextureData = stbi_load(filename.c_str(), &width, &height, &format, (format < 3 ? format : 4)); /// D3D11 doesn't work with 24 bits textures
			}
			
			if (!pTextureData)
			{
				return;
			}

			E_FORMAT_TYPE internalFormat = isHDREnabled ? FT_FLOAT4 : FT_NORM_UBYTE4;

			switch (format)
			{
				case 1:
					internalFormat = isHDREnabled ? FT_FLOAT1 : FT_NORM_UBYTE1;
					break;
				case 2:
					internalFormat = isHDREnabled ? FT_FLOAT2 : FT_NORM_UBYTE2;
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
				ITexture3D* pTextureResource = dynamic_cast<ITexture3D*>(pResource);

				const TTexture3DParameters* pTextureMetaInfo = dynamic_cast<const TTexture3DParameters*>(mpResourceManager->GetResourceMeta(pResource->GetName()));
				TTexture3DParameters textureParams = pTextureMetaInfo ? *pTextureMetaInfo : TTexture3DParameters { static_cast<U32>(width), static_cast<U32>(height), internalFormat, 1, 1, 0 };

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
		*/
		return RC_OK;
	}

	TypeId CBaseTexture3DLoader::GetResourceTypeId() const
	{
		return ITexture3D::GetTypeId();
	}


	TDE2_API IResourceLoader* CreateBaseTexture3DLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceLoader, CBaseTexture3DLoader, result, pResourceManager, pGraphicsContext, pFileSystem);
	}


	/*!
		\brief CBaseTexture3DFactory's definition
	*/

	CBaseTexture3DFactory::CBaseTexture3DFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CBaseTexture3DFactory::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext)
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

	IResource* CBaseTexture3DFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		const TTexture3DParameters& texParams = static_cast<const TTexture3DParameters&>(params);

		return dynamic_cast<IResource*>(CreateTexture3D(mpResourceManager, mpGraphicsContext, name, texParams, result));
	}

	IResource* CBaseTexture3DFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		const static TTexture3DParameters defaultTextureParams{ 2, 2, 1, FT_NORM_UBYTE4, 1, 1, 0 };

		TTexture3DParameters overridenParams = defaultTextureParams;
		overridenParams.mLoadingPolicy = params.mLoadingPolicy;

		return dynamic_cast<IResource*>(CreateTexture3D(mpResourceManager, mpGraphicsContext, name, overridenParams, result));
	}

	TypeId CBaseTexture3DFactory::GetResourceTypeId() const
	{
		return ITexture3D::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateBaseTexture3DFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceFactory, CBaseTexture3DFactory, result, pResourceManager, pGraphicsContext);
	}
}