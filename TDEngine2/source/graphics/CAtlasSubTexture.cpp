#include "../../include/graphics/CAtlasSubTexture.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/ITextureAtlas.h"
#include "../../include/graphics/ITexture2D.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/CBaseFileSystem.h"
#include "../../include/utils/Utils.h"
#include "stringUtils.hpp"


namespace TDEngine2
{

	CAtlasSubTexture::CAtlasSubTexture() :
		CBaseResource(), mOwnerAtlasResourceHandle(TResourceId::Invalid)
	{
	}

	E_RESULT_CODE CAtlasSubTexture::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, const TAtlasSubTextureParameters& params)
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

		mOwnerAtlasResourceHandle = params.mTextureAtlasId;
		mTextureRectInfo = params.mTextureRectInfo;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CAtlasSubTexture::Reset()
	{
		return RC_OK;
	}

	void CAtlasSubTexture::Bind(U32 slot)
	{
		if (mCurrTextureSamplerHandle == TTextureSamplerId::Invalid)
		{
			mCurrTextureSamplerHandle = GetTextureSampleHandle(mpGraphicsContext, mTextureSamplerParams);
		}

		mpGraphicsContext->BindTextureSampler(slot, mCurrTextureSamplerHandle);

		/// \note Bind the source texture
		if (ITextureAtlas* pOwnerAtlas = mpResourceManager->GetResource<ITextureAtlas>(mOwnerAtlasResourceHandle))
		{
			if (ITexture2D* pTexture = pOwnerAtlas->GetTexture())
			{
				pTexture->Bind(slot);
			}
		}
	}

	void CAtlasSubTexture::SetUWrapMode(const E_ADDRESS_MODE_TYPE& mode)
	{
		mTextureSamplerParams.mUAddressMode = mode;
	}

	void CAtlasSubTexture::SetVWrapMode(const E_ADDRESS_MODE_TYPE& mode)
	{
		mTextureSamplerParams.mVAddressMode = mode;
	}

	void CAtlasSubTexture::SetWWrapMode(const E_ADDRESS_MODE_TYPE& mode)
	{
		mTextureSamplerParams.mWAddressMode = mode;
	}

	void CAtlasSubTexture::SetFilterType(const E_FILTER_TYPE& type)
	{
		TDE2_UNIMPLEMENTED();
		//mTextureSamplerParams.
	}

	U32 CAtlasSubTexture::GetWidth() const
	{
		if (ITextureAtlas* pOwnerAtlas = mpResourceManager->GetResource<ITextureAtlas>(mOwnerAtlasResourceHandle))
		{
			if (ITexture2D* pTexture = pOwnerAtlas->GetTexture())
			{
				return pTexture->GetWidth();
			}
		}

		return 0;
	}

	U32 CAtlasSubTexture::GetHeight() const
	{
		if (ITextureAtlas* pOwnerAtlas = mpResourceManager->GetResource<ITextureAtlas>(mOwnerAtlasResourceHandle))
		{
			if (ITexture2D* pTexture = pOwnerAtlas->GetTexture())
			{
				return pTexture->GetHeight();
			}
		}

		return 0;
	}

	E_FORMAT_TYPE CAtlasSubTexture::GetFormat() const
	{
		if (ITextureAtlas* pOwnerAtlas = mpResourceManager->GetResource<ITextureAtlas>(mOwnerAtlasResourceHandle))
		{
			if (ITexture2D* pTexture = pOwnerAtlas->GetTexture())
			{
				return pTexture->GetFormat();
			}
		}

		return E_FORMAT_TYPE::FT_UNKNOWN;
	}

	TRectF32 CAtlasSubTexture::GetNormalizedTextureRect() const
	{
		if (ITextureAtlas* pOwnerAtlas = mpResourceManager->GetResource<ITextureAtlas>(mOwnerAtlasResourceHandle))
		{
			if (ITexture2D* pTexture = pOwnerAtlas->GetTexture())
			{
				const F32 width = static_cast<F32>(pTexture->GetWidth());
				const F32 height = static_cast<F32>(pTexture->GetHeight());

				return { mTextureRectInfo.x / width, mTextureRectInfo.y / height, mTextureRectInfo.width / width, mTextureRectInfo.height / height };
			}
		}

		return { 0.0f, 0.0f, 1.0f, 1.0f };
	}

	TTextureSamplerId CAtlasSubTexture::GetTextureSampleHandle(IGraphicsContext* pGraphicsContext, const TTextureSamplerDesc& params)
	{
		IGraphicsObjectManager* pGraphicsObjectManager = pGraphicsContext->GetGraphicsObjectManager();
		TDE2_ASSERT(pGraphicsObjectManager);

		if (!pGraphicsObjectManager)
		{
			return TTextureSamplerId::Invalid;
		}

		auto result = pGraphicsObjectManager->CreateTextureSampler(params);
		return result.IsOk() ? result.Get() : TTextureSamplerId::Invalid;
	}

	const TPtr<IResourceLoader> CAtlasSubTexture::_getResourceLoader()
	{
		return mpResourceManager->GetResourceLoader<IAtlasSubTexture>();
	}


	TDE2_API IAtlasSubTexture* CreateAtlasSubTexture(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
													 const TAtlasSubTextureParameters& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IAtlasSubTexture, CAtlasSubTexture, result, pResourceManager, pGraphicsContext, name, params);
	}


	/*!
		\brief CAtlasSubTextureLoader's definition
	*/

	CAtlasSubTextureLoader::CAtlasSubTextureLoader() :
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CAtlasSubTextureLoader::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem)
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

	E_RESULT_CODE CAtlasSubTextureLoader::LoadResource(IResource* pResource) const
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		TDE2_UNIMPLEMENTED();
		return RC_NOT_IMPLEMENTED_YET;
		//return dynamic_cast<IAtlasSubTexture*>(pResource)->Deserialize(mpFileSystem, pResource->GetName() + ".info");
	}

	TypeId CAtlasSubTextureLoader::GetResourceTypeId() const
	{
		return IAtlasSubTexture::GetTypeId();
	}


	TDE2_API IResourceLoader* CreateAtlasSubTextureLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceLoader, CAtlasSubTextureLoader, result, pResourceManager, pGraphicsContext, pFileSystem);
	}


	/*!
		\brief CAtlasSubTextureFactory's definition
	*/

	CAtlasSubTextureFactory::CAtlasSubTextureFactory() :
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CAtlasSubTextureFactory::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext)
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

	IResource* CAtlasSubTextureFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return dynamic_cast<IResource*>(CreateAtlasSubTexture(mpResourceManager, mpGraphicsContext, name, dynamic_cast<const TAtlasSubTextureParameters&>(params), result));
	}

	IResource* CAtlasSubTextureFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		TAtlasSubTextureParameters subTextureParams;
		subTextureParams.mLoadingPolicy = params.mLoadingPolicy;

		return dynamic_cast<IResource*>(CreateAtlasSubTexture(mpResourceManager, mpGraphicsContext, name, subTextureParams, result));
	}

	TypeId CAtlasSubTextureFactory::GetResourceTypeId() const
	{
		return IAtlasSubTexture::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateAtlasSubTextureFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceFactory, CAtlasSubTextureFactory, result, pResourceManager, pGraphicsContext);
	}
}