#include "./../include/CD3D11Texture2D.h"
#include "./../include/CD3D11GraphicsContext.h"
#include "./../include/CD3D11Mappings.h"
#include "./../include/CD3D11Utils.h"
#include <utils/Utils.h>
#include <cstring>
#include <algorithm>


#if defined (TDE2_USE_WIN32PLATFORM)

namespace TDEngine2
{
	CD3D11Texture2D::CD3D11Texture2D() :
		CBaseTexture2D(), mpTexture(nullptr)
	{
	}

	void CD3D11Texture2D::Bind(U32 slot)
	{
		mp3dDeviceContext->VSSetShaderResources(slot, 1, &mpShaderTextureView);
		mp3dDeviceContext->PSSetShaderResources(slot, 1, &mpShaderTextureView);
		mp3dDeviceContext->GSSetShaderResources(slot, 1, &mpShaderTextureView);
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

		E_RESULT_CODE result = pResourceLoader->LoadResource(this);

		if (result != RC_OK)
		{
			mState = RST_PENDING;

			return result;
		}

		mState = RST_LOADED;

		return result;
	}

	E_RESULT_CODE CD3D11Texture2D::Unload()
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CD3D11Texture2D::Reset()
	{
		mIsInitialized = false;

		E_RESULT_CODE result = RC_OK;

		if ((result = SafeReleaseCOMPtr<ID3D11Texture2D>(&mpTexture)) != RC_OK ||
			(result = SafeReleaseCOMPtr<ID3D11ShaderResourceView>(&mpShaderTextureView)) != RC_OK)
		{
			return result;
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

		region.left   = regionRect.x;
		region.top    = regionRect.y;
		region.right  = regionRect.x + regionRect.width;
		region.bottom = regionRect.y + regionRect.height;
		region.back   = 1;
		region.front  = 0;

		U32 rowPitch = regionRect.width * CD3D11Mappings::GetNumOfChannelsOfFormat(mFormat);

		mp3dDeviceContext->UpdateSubresource(mpTexture, 0, &region, pData, rowPitch, rowPitch * regionRect.height);

		return RC_OK;
	}

	std::unique_ptr<U8[]> CD3D11Texture2D::GetInternalData()
	{
		/// \note create temporary texture with D3D11_USAGE_STAGING flag
		ID3D11Texture2D* pTempTexture = _createD3D11TextureResource(mpGraphicsContext, mWidth, mHeight, mFormat, mNumOfMipLevels, mNumOfSamples, mSamplingQuality, DTAT_CPU_READ).Get();

		/// \note copy data from actual texture into helper one
		mp3dDeviceContext->CopyResource(pTempTexture, mpTexture);

		/// \note retrieve pointer to memory from temp texture
		D3D11_MAPPED_SUBRESOURCE mappedData;

		if (FAILED(mp3dDeviceContext->Map(pTempTexture, 0, D3D11_MAP_READ, 0x0, &mappedData)))
		{
			return {};
		}

		U64 size = mWidth * mHeight * CD3D11Mappings::GetFormatSize(mFormat);

		std::unique_ptr<U8[]> pPixelsData{ new U8[size] };

		memcpy(pPixelsData.get(), mappedData.pData, size);

		mp3dDeviceContext->Unmap(pTempTexture, 0);

		pTempTexture->Release();

		return std::move(pPixelsData);
	}

	E_RESULT_CODE CD3D11Texture2D::_createInternalTextureHandler(IGraphicsContext* pGraphicsContext, U32 width, U32 height, E_FORMAT_TYPE format,
																 U32 mipLevelsCount, U32 samplesCount, U32 samplingQuality)
	{
		auto textureResult = _createD3D11TextureResource(pGraphicsContext, width, height, format, mipLevelsCount, samplesCount, samplingQuality);

		if (textureResult.HasError())
		{
			return textureResult.GetError();
		}

		mpTexture = textureResult.Get();

		return _createShaderTextureView(mp3dDevice, mFormat, mNumOfMipLevels);
	}

	TResult<ID3D11Texture2D*> CD3D11Texture2D::_createD3D11TextureResource(IGraphicsContext* pGraphicsContext, U32 width, U32 height, E_FORMAT_TYPE format,
																		   U32 mipLevelsCount, U32 samplesCount, U32 samplingQuality,
																		   U32 accessType)
	{
		TGraphicsCtxInternalData graphicsInternalData = mpGraphicsContext->GetInternalData();

#if _HAS_CXX17
		mp3dDevice = std::get<TD3D11CtxInternalData>(graphicsInternalData).mp3dDevice;

		mp3dDeviceContext = std::get<TD3D11CtxInternalData>(graphicsInternalData).mp3dDeviceContext;
#else
		mp3dDevice = graphicsInternalData.mD3D11.mp3dDevice;

		mp3dDeviceContext = graphicsInternalData.mD3D11.mp3dDeviceContext;
#endif

		D3D11_TEXTURE2D_DESC textureDesc;

		memset(&textureDesc, 0, sizeof(textureDesc));

		bool isCPUAccessible = (accessType & DTAT_CPU_READ);

		textureDesc.Width              = width;
		textureDesc.Height             = height;
		textureDesc.Format             = CD3D11Mappings::GetDXGIFormat(format);
		textureDesc.SampleDesc.Count   = samplesCount;
		textureDesc.SampleDesc.Quality = samplingQuality;
		textureDesc.MipLevels          = mipLevelsCount;
		textureDesc.ArraySize          = 1; //single texture
		textureDesc.BindFlags          = isCPUAccessible ? 0x0 : D3D11_BIND_SHADER_RESOURCE; /// default binding type for simple 2d textures
		textureDesc.CPUAccessFlags     = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
		textureDesc.Usage              = isCPUAccessible ? D3D11_USAGE_STAGING : D3D11_USAGE_DEFAULT; /// \todo replace it with corresponding mapping

		ID3D11Texture2D* pTexture = nullptr;

		/// create blank texture with specified parameters
		if (FAILED(mp3dDevice->CreateTexture2D(&textureDesc, nullptr, &pTexture))) /// \todo Implement HRESULT -> E_RESULT_CODE converter function
		{
			return TErrorValue(RC_FAIL);
		}

		return TOkValue(pTexture);
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


	TDE2_API ITexture2D* CreateD3D11Texture2D(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result)
	{
		CD3D11Texture2D* pTexture2DInstance = new (std::nothrow) CD3D11Texture2D();

		if (!pTexture2DInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pTexture2DInstance->Init(pResourceManager, pGraphicsContext, name);

		if (result != RC_OK)
		{
			delete pTexture2DInstance;

			pTexture2DInstance = nullptr;
		}

		return pTexture2DInstance;
	}

	TDE2_API ITexture2D* CreateD3D11Texture2D(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
											  const TTexture2DParameters& params, E_RESULT_CODE& result)
	{
		CD3D11Texture2D* pTexture2DInstance = new (std::nothrow) CD3D11Texture2D();

		if (!pTexture2DInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pTexture2DInstance->Init(pResourceManager, pGraphicsContext, name, params);

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

	E_RESULT_CODE CD3D11Texture2DFactory::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext)
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

	IResource* CD3D11Texture2DFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		const TTexture2DParameters& texParams = static_cast<const TTexture2DParameters&>(params);

		return dynamic_cast<IResource*>(CreateD3D11Texture2D(mpResourceManager, mpGraphicsContext, name, texParams, result));
	}

	IResource* CD3D11Texture2DFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		// create blank texture, which sizes equals to 2 x 2 pixels of RGBA format
		return dynamic_cast<IResource*>(CreateD3D11Texture2D(mpResourceManager, mpGraphicsContext, name, { 2, 2, FT_NORM_UBYTE4, 1, 1, 0 }, result));
	}

	U32 CD3D11Texture2DFactory::GetResourceTypeId() const
	{
		return CBaseTexture2D::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateD3D11Texture2DFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		CD3D11Texture2DFactory* pTexture2DFactoryInstance = new (std::nothrow) CD3D11Texture2DFactory();

		if (!pTexture2DFactoryInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pTexture2DFactoryInstance->Init(pResourceManager, pGraphicsContext);

		if (result != RC_OK)
		{
			delete pTexture2DFactoryInstance;

			pTexture2DFactoryInstance = nullptr;
		}

		return pTexture2DFactoryInstance;
	}
}

#endif