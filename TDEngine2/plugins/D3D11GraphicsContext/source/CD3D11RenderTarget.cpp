#include "./../include/CD3D11RenderTarget.h"
#include "./../include/CD3D11GraphicsContext.h"
#include "./../include/CD3D11Mappings.h"
#include "./../include/CD3D11Utils.h"
#include "./../include/CD3D11Texture2D.h"
#include <graphics/ITexture2D.h>
#include <cstring>


#if defined (TDE2_USE_WINPLATFORM)

namespace TDEngine2
{
	CD3D11RenderTarget::CD3D11RenderTarget() :
		CBaseRenderTarget(), mpRenderTexture(nullptr), mpShaderTextureView(nullptr), mpRenderTargetView(nullptr)
	{
	}

	E_RESULT_CODE CD3D11RenderTarget::Load()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		auto&& pResourceLoader = mpResourceManager->GetResourceLoader<IRenderTarget>();

		if (!pResourceLoader)
		{
			return RC_FAIL;
		}

		return pResourceLoader->LoadResource(this);
	}

	E_RESULT_CODE CD3D11RenderTarget::Unload()
	{
		return Reset();
	}

	E_RESULT_CODE CD3D11RenderTarget::Reset()
	{
		mIsInitialized = false;

		E_RESULT_CODE result = RC_OK;

		if ((result = SafeReleaseCOMPtr<ID3D11Texture2D>(&mpRenderTexture)) != RC_OK ||
			(result = SafeReleaseCOMPtr<ID3D11ShaderResourceView>(&mpShaderTextureView)) != RC_OK ||
			(result = SafeReleaseCOMPtr<ID3D11RenderTargetView>(&mpRenderTargetView)) != RC_OK)
		{
			return result;
		}

		return RC_OK;
	}

	void CD3D11RenderTarget::Bind(U32 slot)
	{
		CBaseRenderTarget::Bind(slot);

		mp3dDeviceContext->VSSetShaderResources(slot, 1, &mpShaderTextureView);
		mp3dDeviceContext->PSSetShaderResources(slot, 1, &mpShaderTextureView);
		mp3dDeviceContext->GSSetShaderResources(slot, 1, &mpShaderTextureView);
	}

	E_RESULT_CODE CD3D11RenderTarget::Blit(ITexture2D*& pDestTexture)
	{
		if (!pDestTexture)
		{
			return RC_INVALID_ARGS;
		}

		CD3D11Texture2D* pD3D11DestTexture = dynamic_cast<CD3D11Texture2D*>(pDestTexture);
		if (!pD3D11DestTexture)
		{
			return RC_FAIL;
		}

		mp3dDeviceContext->CopyResource(pD3D11DestTexture->GetInternalTexturePtr(), mpRenderTexture);
		//D3D11_BOX sourceRegion;
		//sourceRegion.left = 0;
		//sourceRegion.right = mWidth;
		//sourceRegion.top = 0;
		//sourceRegion.bottom = mHeight;
		//sourceRegion.front = 0;
		//sourceRegion.back = 1;

		//mp3dDeviceContext->CopySubresourceRegion(pD3D11DestTexture->GetInternalTexturePtr(), 0, 0, 0, 0, mpRenderTexture, 0, &sourceRegion);

		return RC_OK;
	}

	ID3D11RenderTargetView* CD3D11RenderTarget::GetRenderTargetView() const
	{
		return mpRenderTargetView;
	}

	E_RESULT_CODE CD3D11RenderTarget::_createInternalTextureHandler(IGraphicsContext* pGraphicsContext, U32 width, U32 height, E_FORMAT_TYPE format,
																	U32 mipLevelsCount, U32 samplesCount, U32 samplingQuality)
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

		textureDesc.Width              = width;
		textureDesc.Height             = height;
		textureDesc.Format             = CD3D11Mappings::GetDXGIFormat(format);
		textureDesc.SampleDesc.Count   = samplesCount;
		textureDesc.SampleDesc.Quality = samplingQuality;
		textureDesc.MipLevels          = mipLevelsCount;
		textureDesc.ArraySize          = 1; //single texture
		textureDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		textureDesc.CPUAccessFlags     = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
		textureDesc.Usage              = D3D11_USAGE_DEFAULT; /// \todo replace it with corresponding mapping

		/// create blank texture with specified parameters
		if (FAILED(mp3dDevice->CreateTexture2D(&textureDesc, nullptr, &mpRenderTexture))) /// \todo Implement HRESULT -> E_RESULT_CODE converter function
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = _createRenderTargetView(mp3dDevice, mFormat);

		if (result != RC_OK)
		{
			mIsInitialized = false;

			return result;
		}

		return _createShaderTextureView(mp3dDevice, mFormat, mNumOfMipLevels);
	}

	E_RESULT_CODE CD3D11RenderTarget::_createShaderTextureView(ID3D11Device* p3dDevice, E_FORMAT_TYPE format, U32 mipLevelsCount)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;

		memset(&viewDesc, 0, sizeof(viewDesc));

		viewDesc.Format        = CD3D11Mappings::GetDXGIFormat(format);
		viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

		viewDesc.Texture2D.MipLevels = 1;

		if (FAILED(p3dDevice->CreateShaderResourceView(mpRenderTexture, &viewDesc, &mpShaderTextureView)))
		{
			return RC_FAIL;
		}

		return RC_OK;
	}

	E_RESULT_CODE CD3D11RenderTarget::_createRenderTargetView(ID3D11Device* p3dDevice, E_FORMAT_TYPE format)
	{
		D3D11_RENDER_TARGET_VIEW_DESC viewDesc;

		memset(&viewDesc, 0, sizeof(viewDesc));

		viewDesc.Format        = CD3D11Mappings::GetDXGIFormat(format);
		viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

		viewDesc.Texture2D.MipSlice = 0;

		if (FAILED(p3dDevice->CreateRenderTargetView(mpRenderTexture, &viewDesc, &mpRenderTargetView)))
		{
			return RC_FAIL;
		}

		return RC_OK;
	}


	TDE2_API IRenderTarget* CreateD3D11RenderTarget(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
													const TTexture2DParameters& params, E_RESULT_CODE& result)
	{
		CD3D11RenderTarget* pRenderTargetInstance = new (std::nothrow) CD3D11RenderTarget();

		if (!pRenderTargetInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pRenderTargetInstance->Init(pResourceManager, pGraphicsContext, name, params);

		if (result != RC_OK)
		{
			delete pRenderTargetInstance;

			pRenderTargetInstance = nullptr;
		}

		return pRenderTargetInstance;
	}


	CD3D11RenderTargetFactory::CD3D11RenderTargetFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CD3D11RenderTargetFactory::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext)
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

	IResource* CD3D11RenderTargetFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		const TTexture2DParameters& texParams = static_cast<const TTexture2DParameters&>(params);

		return dynamic_cast<IResource*>(CreateD3D11RenderTarget(mpResourceManager, mpGraphicsContext, name, texParams, result));
	}

	IResource* CD3D11RenderTargetFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		const TTexture2DParameters& texParams = static_cast<const TTexture2DParameters&>(params);

		return dynamic_cast<IResource*>(CreateD3D11RenderTarget(mpResourceManager, mpGraphicsContext, name, texParams, result));
	}

	TypeId CD3D11RenderTargetFactory::GetResourceTypeId() const
	{
		return IRenderTarget::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateD3D11RenderTargetFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		CD3D11RenderTargetFactory* pRenderTargetFactoryInstance = new (std::nothrow) CD3D11RenderTargetFactory();

		if (!pRenderTargetFactoryInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pRenderTargetFactoryInstance->Init(pResourceManager, pGraphicsContext);

		if (result != RC_OK)
		{
			delete pRenderTargetFactoryInstance;

			pRenderTargetFactoryInstance = nullptr;
		}

		return pRenderTargetFactoryInstance;
	}
}

#endif