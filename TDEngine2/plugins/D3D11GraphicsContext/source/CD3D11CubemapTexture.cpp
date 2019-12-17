#include "./../include/CD3D11CubemapTexture.h"
#include "./../include/CD3D11GraphicsContext.h"
#include "./../include/CD3D11Mappings.h"
#include "./../include/CD3D11Utils.h"
#include "./../include/CD3D11Texture2D.h"
#include <utils/Utils.h>
#include <cstring>
#include <algorithm>


#if defined (TDE2_USE_WIN32PLATFORM)

namespace TDEngine2
{
	CD3D11CubemapTexture::CD3D11CubemapTexture() :
		CBaseCubemapTexture()
	{
	}

	void CD3D11CubemapTexture::Bind(U32 slot)
	{
		CBaseCubemapTexture::Bind(slot);

		mp3dDeviceContext->VSSetShaderResources(slot, 1, &mpShaderTextureView);
		mp3dDeviceContext->PSSetShaderResources(slot, 1, &mpShaderTextureView);
		mp3dDeviceContext->GSSetShaderResources(slot, 1, &mpShaderTextureView);
	}

	E_RESULT_CODE CD3D11CubemapTexture::Load()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		const IResourceLoader* pResourceLoader = mpResourceManager->GetResourceLoader<CBaseCubemapTexture>();

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

	E_RESULT_CODE CD3D11CubemapTexture::Unload()
	{
		return Reset();
	}

	E_RESULT_CODE CD3D11CubemapTexture::Reset()
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

	E_RESULT_CODE CD3D11CubemapTexture::WriteData(E_CUBEMAP_FACE face, const TRectI32& regionRect, const U8* pData)
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

		mp3dDeviceContext->UpdateSubresource(mpTexture, D3D11CalcSubresource(0, CD3D11Mappings::GetCubemapFace(face), mNumOfMipLevels), &region, 
											 pData, rowPitch, rowPitch * regionRect.height);

		return RC_OK;
	}

	E_RESULT_CODE CD3D11CubemapTexture::_createInternalTextureHandler(IGraphicsContext* pGraphicsContext, U32 width, U32 height, E_FORMAT_TYPE format,
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

	TResult<ID3D11Texture2D*> CD3D11CubemapTexture::_createD3D11TextureResource(IGraphicsContext* pGraphicsContext, U32 width, U32 height, E_FORMAT_TYPE format,
																				U32 mipLevelsCount, U32 samplesCount, U32 samplingQuality,
																				U32 accessType)
	{
		TGraphicsCtxInternalData graphicsInternalData = mpGraphicsContext->GetInternalData();

#if _HAS_CXX17
		mp3dDevice        = std::get<TD3D11CtxInternalData>(graphicsInternalData).mp3dDevice;
		mp3dDeviceContext = std::get<TD3D11CtxInternalData>(graphicsInternalData).mp3dDeviceContext;
#else
		mp3dDevice        = graphicsInternalData.mD3D11.mp3dDevice;
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
		textureDesc.ArraySize          = 6; // this is a cubemap with 6 faces (textures)
		textureDesc.BindFlags          = isCPUAccessible ? 0x0 : D3D11_BIND_SHADER_RESOURCE; /// default binding type for simple 2d textures
		textureDesc.CPUAccessFlags     = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
		textureDesc.Usage              = isCPUAccessible ? D3D11_USAGE_STAGING : D3D11_USAGE_DEFAULT; /// \todo replace it with corresponding mapping
		textureDesc.MiscFlags          = D3D11_RESOURCE_MISC_TEXTURECUBE;

		ID3D11Texture2D* pTexture = nullptr;

		/// create blank texture with specified parameters
		if (FAILED(mp3dDevice->CreateTexture2D(&textureDesc, nullptr, &pTexture))) /// \todo Implement HRESULT -> E_RESULT_CODE converter function
		{
			return TErrorValue<E_RESULT_CODE>(RC_FAIL);
		}

		return TOkValue<ID3D11Texture2D*>(pTexture);
	}

	E_RESULT_CODE CD3D11CubemapTexture::_createShaderTextureView(ID3D11Device* p3dDevice, E_FORMAT_TYPE format, U32 mipLevelsCount)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;

		memset(&viewDesc, 0, sizeof(viewDesc));

		viewDesc.Format        = CD3D11Mappings::GetDXGIFormat(format);
		viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;

		viewDesc.TextureCube.MipLevels = mipLevelsCount;

		if (FAILED(p3dDevice->CreateShaderResourceView(mpTexture, &viewDesc, &mpShaderTextureView)))
		{
			return RC_FAIL;
		}

		return RC_OK;
	}


	TDE2_API ICubemapTexture* CreateD3D11CubemapTexture(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result)
	{
		CD3D11CubemapTexture* pCubemapTextureInstance = new (std::nothrow) CD3D11CubemapTexture();

		if (!pCubemapTextureInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pCubemapTextureInstance->Init(pResourceManager, pGraphicsContext, name);

		if (result != RC_OK)
		{
			delete pCubemapTextureInstance;

			pCubemapTextureInstance = nullptr;
		}

		return pCubemapTextureInstance;
	}


	TDE2_API ICubemapTexture* CreateD3D11CubemapTexture(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
														const TTexture2DParameters& params, E_RESULT_CODE& result)
	{
		CD3D11CubemapTexture* pTexture2DInstance = new (std::nothrow) CD3D11CubemapTexture();

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


	CD3D11CubemapTextureFactory::CD3D11CubemapTextureFactory() :
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CD3D11CubemapTextureFactory::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext)
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

	E_RESULT_CODE CD3D11CubemapTextureFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IResource* CD3D11CubemapTextureFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		const TTexture2DParameters& texParams = static_cast<const TTexture2DParameters&>(params);

		return dynamic_cast<IResource*>(CreateD3D11CubemapTexture(mpResourceManager, mpGraphicsContext, name, texParams, result));
	}

	IResource* CD3D11CubemapTextureFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;
		
		// create blank texture, which sizes equals to 2 x 2 pixels of RGBA format
		return dynamic_cast<IResource*>(CreateD3D11CubemapTexture(mpResourceManager, mpGraphicsContext, name, { 2, 2, FT_NORM_UBYTE4, 1, 1, 0 }, result));
	}

	U32 CD3D11CubemapTextureFactory::GetResourceTypeId() const
	{
		return CBaseCubemapTexture::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateD3D11CubemapTextureFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		CD3D11CubemapTextureFactory* pCubemapTextureFactoryInstance = new (std::nothrow) CD3D11CubemapTextureFactory();

		if (!pCubemapTextureFactoryInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pCubemapTextureFactoryInstance->Init(pResourceManager, pGraphicsContext);

		if (result != RC_OK)
		{
			delete pCubemapTextureFactoryInstance;

			pCubemapTextureFactoryInstance = nullptr;
		}

		return pCubemapTextureFactoryInstance;
	}
}

#endif