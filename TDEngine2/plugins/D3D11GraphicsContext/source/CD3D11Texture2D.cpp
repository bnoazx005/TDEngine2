#include "./../include/CD3D11Texture2D.h"
#include "./../include/CD3D11GraphicsContext.h"
#include "./../include/CD3D11Mappings.h"
#include <cstring>


#if defined (TDE2_USE_WIN32PLATFORM)

namespace TDEngine2
{
	CD3D11Texture2D::CD3D11Texture2D() :
		CBaseTexture2D(), mpTexture(nullptr)
	{
	}

	E_RESULT_CODE CD3D11Texture2D::Load()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		const IResourceLoader* pResourceLoader = mpResourceManager->GetResourceLoader<CBaseTexture2D>();

		if (!pResourceLoader)
		{
			return RC_FAIL;
		}

		return pResourceLoader->LoadResource(this);
	}

	E_RESULT_CODE CD3D11Texture2D::Unload()
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CD3D11Texture2D::Reset()
	{
		mIsInitialized = false;

		if (mpTexture)
		{
			if (FAILED(mpTexture->Release()))
			{
				return RC_FAIL;
			}

			mpTexture = nullptr;
		}

		if (mpShaderTextureView)
		{
			if (FAILED(mpShaderTextureView->Release()))
			{
				return RC_FAIL;
			}

			mpShaderTextureView = nullptr;
		}

		return RC_OK;
	}

	E_RESULT_CODE CD3D11Texture2D::WriteData(const TRectI32& regionRect, const U8* pData)
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		D3D11_BOX region;

		region.left   = regionRect.left;
		region.right  = regionRect.right;
		region.top    = regionRect.top;
		region.bottom = regionRect.bottom;
		region.back   = 1;
		region.front  = 0;

		U32 rowPitch = regionRect.right * CD3D11Mappings::GetNumOfChannelsOfFormat(mFormat);

		mp3dDeviceContext->UpdateSubresource(mpTexture, 0, &region, pData, rowPitch, rowPitch * regionRect.bottom);

		return RC_OK;
	}

	E_RESULT_CODE CD3D11Texture2D::_createInternalTextureHandler(IGraphicsContext* pGraphicsContext, U32 width, U32 height, E_FORMAT_TYPE format,
																 U32 mipLevelsCount, U32 samplesCount, U32 samplingQuality)
	{
		TGraphicsCtxInternalData graphicsInternalData = mpGraphicsContext->GetInternalData();

#if _HAS_CXX17
		mp3dDevice = std::get<TD3D11CtxInternalData>(graphicsInternalData).mp3dDevice;

		mp3dDeviceContext = std::get<TD3D11CtxInternalData>(graphicsInternalData).mp3dDeviceContext;
#else
		mp3dDevice = graphicsInternalData.mD3D11.mp3dDevice;

		mp3dDeviceContext = std::get<TD3D11CtxInternalData>(graphicsInternalData).mp3dDeviceContext;
#endif

		D3D11_TEXTURE2D_DESC textureDesc;

		memset(&textureDesc, 0, sizeof(textureDesc));

		textureDesc.Width              = width;
		textureDesc.Height             = height;
		textureDesc.Format             = CD3D11Mappings::GetDXGIFormat(format);
		textureDesc.SampleDesc.Count   = samplesCount;
		textureDesc.SampleDesc.Quality = samplingQuality;
		textureDesc.MipLevels          = mipLevelsCount;
		textureDesc.ArraySize          = 1; //single texture
		textureDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE; /// default binding type for simple 2d textures
		textureDesc.CPUAccessFlags     = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
		textureDesc.Usage              = D3D11_USAGE_DEFAULT; /// \todo replace it with corresponding mapping

		/// create blank texture with specified parameters
		if (FAILED(mp3dDevice->CreateTexture2D(&textureDesc, nullptr, &mpTexture))) /// \todo Implement HRESULT -> E_RESULT_CODE converter function
		{
			return RC_FAIL;
		}

		return _createShaderTextureView(mp3dDevice, mFormat, mNumOfMipLevels);
	}

	E_RESULT_CODE CD3D11Texture2D::_createShaderTextureView(ID3D11Device* p3dDevice, E_FORMAT_TYPE format, U32 mipLevelsCount)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;

		memset(&viewDesc, 0, sizeof(viewDesc));

		viewDesc.Format        = CD3D11Mappings::GetDXGIFormat(format);
		viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

		viewDesc.Texture2D.MipLevels = mipLevelsCount;

		if (FAILED(p3dDevice->CreateShaderResourceView(mpTexture, &viewDesc, &mpShaderTextureView)))
		{
			return RC_FAIL;
		}

		return RC_OK;
	}


	TDE2_API ITexture2D* CreateD3D11Texture2D(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
											  TResourceId id, E_RESULT_CODE& result)
	{
		CD3D11Texture2D* pTexture2DInstance = new (std::nothrow) CD3D11Texture2D();

		if (!pTexture2DInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pTexture2DInstance->Init(pResourceManager, pGraphicsContext, name, id);

		if (result != RC_OK)
		{
			delete pTexture2DInstance;

			pTexture2DInstance = nullptr;
		}

		return pTexture2DInstance;
	}

	TDE2_API ITexture2D* CreateD3D11Texture2D(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
											  TResourceId id, U32 width, U32 height, E_FORMAT_TYPE format, U32 mipLevelsCount,
											  U32 samplesCount, U32 samplingQuality, E_RESULT_CODE& result)
	{
		CD3D11Texture2D* pTexture2DInstance = new (std::nothrow) CD3D11Texture2D();

		if (!pTexture2DInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pTexture2DInstance->Init(pResourceManager, pGraphicsContext, name, id, width, height, format, 
										  mipLevelsCount, samplesCount, samplingQuality);

		if (result != RC_OK)
		{
			delete pTexture2DInstance;

			pTexture2DInstance = nullptr;
		}

		return pTexture2DInstance;
	}


	CD3D11Texture2DFactory::CD3D11Texture2DFactory() :
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CD3D11Texture2DFactory::Init(IGraphicsContext* pGraphicsContext)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pGraphicsContext)
		{
			return RC_INVALID_ARGS;
		}

		mpGraphicsContext = pGraphicsContext;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CD3D11Texture2DFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IResource* CD3D11Texture2DFactory::Create(const TBaseResourceParameters* pParams) const
	{
		E_RESULT_CODE result = RC_OK;

		const TTexture2DParameters* pTexParams = static_cast<const TTexture2DParameters*>(pParams);

		return dynamic_cast<IResource*>(CreateD3D11Texture2D(pTexParams->mpResourceManager, pTexParams->mpGraphicsContext, pTexParams->mName, pTexParams->mId,
															 pTexParams->mWidth, pTexParams->mHeight, pTexParams->mFormat, pTexParams->mNumOfMipLevels, 
															 pTexParams->mNumOfSamples, pTexParams->mSamplingQuality, result));
	}

	IResource* CD3D11Texture2DFactory::CreateDefault(const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		// create blank texture, which sizes equals to 2 x 2 pixels of RGBA format
		return dynamic_cast<IResource*>(CreateD3D11Texture2D(params.mpResourceManager, mpGraphicsContext, params.mName, params.mId, 
															 2, 2, FT_NORM_UBYTE4, 1, 1, 0, result));
	}

	U32 CD3D11Texture2DFactory::GetResourceTypeId() const
	{
		return CBaseTexture2D::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateD3D11Texture2DFactory(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		CD3D11Texture2DFactory* pTexture2DFactoryInstance = new (std::nothrow) CD3D11Texture2DFactory();

		if (!pTexture2DFactoryInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pTexture2DFactoryInstance->Init(pGraphicsContext);

		if (result != RC_OK)
		{
			delete pTexture2DFactoryInstance;

			pTexture2DFactoryInstance = nullptr;
		}

		return pTexture2DFactoryInstance;
	}
}

#endif