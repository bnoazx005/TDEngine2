#include "./../include/CD3D11DepthBufferTarget.h"
#include "./../include/CD3D11GraphicsContext.h"
#include "./../include/CD3D11Mappings.h"
#include "./../include/CD3D11Utils.h"
#include "./../include/CD3D11Texture2D.h"
#include <graphics/ITexture2D.h>
#include <cstring>


#if defined (TDE2_USE_WIN32PLATFORM)

namespace TDEngine2
{
	CD3D11DepthBufferTarget::CD3D11DepthBufferTarget() :
		CBaseDepthBufferTarget(), mpDepthBufferTexture(nullptr), mpShaderTextureView(nullptr), mpDepthBufferTargetView(nullptr)
	{
	}

	E_RESULT_CODE CD3D11DepthBufferTarget::Load()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		const IResourceLoader* pResourceLoader = mpResourceManager->GetResourceLoader<IDepthBufferTarget>();

		if (!pResourceLoader)
		{
			return RC_FAIL;
		}

		return pResourceLoader->LoadResource(this);
	}

	E_RESULT_CODE CD3D11DepthBufferTarget::Unload()
	{
		return Reset();
	}

	E_RESULT_CODE CD3D11DepthBufferTarget::Reset()
	{
		mIsInitialized = false;

		E_RESULT_CODE result = RC_OK;

		if ((result = SafeReleaseCOMPtr<ID3D11Texture2D>(&mpDepthBufferTexture)) != RC_OK ||
			(result = SafeReleaseCOMPtr<ID3D11ShaderResourceView>(&mpShaderTextureView)) != RC_OK ||
			(result = SafeReleaseCOMPtr<ID3D11DepthStencilView>(&mpDepthBufferTargetView)) != RC_OK)
		{
			return result;
		}

		return RC_OK;
	}

	void CD3D11DepthBufferTarget::Bind(U32 slot)
	{
		CBaseDepthBufferTarget::Bind(slot);

		mp3dDeviceContext->VSSetShaderResources(slot, 1, &mpShaderTextureView);
		mp3dDeviceContext->PSSetShaderResources(slot, 1, &mpShaderTextureView);
		mp3dDeviceContext->GSSetShaderResources(slot, 1, &mpShaderTextureView);
	}

	E_RESULT_CODE CD3D11DepthBufferTarget::Blit(ITexture2D*& pDestTexture)
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

		mp3dDeviceContext->CopyResource(pD3D11DestTexture->GetInternalTexturePtr(), mpDepthBufferTexture);

		return RC_OK;
	}

	ID3D11DepthStencilView* CD3D11DepthBufferTarget::GetDepthBufferTargetView() const
	{
		return mpDepthBufferTargetView;
	}

	E_RESULT_CODE CD3D11DepthBufferTarget::_createInternalTextureHandler(IGraphicsContext* pGraphicsContext, U32 width, U32 height, E_FORMAT_TYPE format,
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
		textureDesc.Format             = CD3D11Mappings::GetTypelessVersionOfFormat(format);
		textureDesc.SampleDesc.Count   = samplesCount;
		textureDesc.SampleDesc.Quality = samplingQuality;
		textureDesc.MipLevels          = mipLevelsCount;
		textureDesc.ArraySize          = 1; //single texture
		textureDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
		textureDesc.CPUAccessFlags     = 0; // \note for now we deny access to depth buffer on CPU side
		textureDesc.Usage              = D3D11_USAGE_DEFAULT; /// \todo replace it with corresponding mapping

		/// create blank texture with specified parameters
		if (FAILED(mp3dDevice->CreateTexture2D(&textureDesc, nullptr, &mpDepthBufferTexture))) /// \todo Implement HRESULT -> E_RESULT_CODE converter function
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = _createDepthBufferTargetView(mp3dDevice, mFormat);

		if (result != RC_OK)
		{
			mIsInitialized = false;

			return result;
		}

		return _createShaderTextureView(mp3dDevice, CD3D11Mappings::GetBestFitStrongTypeFormat(format), mNumOfMipLevels);
	}

	E_RESULT_CODE CD3D11DepthBufferTarget::_createShaderTextureView(ID3D11Device* p3dDevice, E_FORMAT_TYPE format, U32 mipLevelsCount)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;

		memset(&viewDesc, 0, sizeof(viewDesc));

		viewDesc.Format        = CD3D11Mappings::GetDXGIFormat(format);
		viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

		viewDesc.Texture2D.MipLevels = 1;

		if (FAILED(p3dDevice->CreateShaderResourceView(mpDepthBufferTexture, &viewDesc, &mpShaderTextureView)))
		{
			return RC_FAIL;
		}

		return RC_OK;
	}

	E_RESULT_CODE CD3D11DepthBufferTarget::_createDepthBufferTargetView(ID3D11Device* p3dDevice, E_FORMAT_TYPE format)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc;

		memset(&viewDesc, 0, sizeof(viewDesc));

		viewDesc.Format        = CD3D11Mappings::GetDXGIFormat(format);
		viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

		viewDesc.Texture2D.MipSlice = 0;

		if (FAILED(p3dDevice->CreateDepthStencilView(mpDepthBufferTexture, &viewDesc, &mpDepthBufferTargetView)))
		{
			return RC_FAIL;
		}

		return RC_OK;
	}


	TDE2_API IDepthBufferTarget* CreateD3D11DepthBufferTarget(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
															  const TTexture2DParameters& params, E_RESULT_CODE& result)
	{
		CD3D11DepthBufferTarget* pDepthBufferTargetInstance = new (std::nothrow) CD3D11DepthBufferTarget();

		if (!pDepthBufferTargetInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pDepthBufferTargetInstance->Init(pResourceManager, pGraphicsContext, name, params);

		if (result != RC_OK)
		{
			delete pDepthBufferTargetInstance;

			pDepthBufferTargetInstance = nullptr;
		}

		return pDepthBufferTargetInstance;
	}


	CD3D11DepthBufferTargetFactory::CD3D11DepthBufferTargetFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CD3D11DepthBufferTargetFactory::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext)
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

	E_RESULT_CODE CD3D11DepthBufferTargetFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IResource* CD3D11DepthBufferTargetFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		const TTexture2DParameters& texParams = static_cast<const TTexture2DParameters&>(params);

		return dynamic_cast<IResource*>(CreateD3D11DepthBufferTarget(mpResourceManager, mpGraphicsContext, name, texParams, result));
	}

	IResource* CD3D11DepthBufferTargetFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		const TTexture2DParameters& texParams = static_cast<const TTexture2DParameters&>(params);

		return dynamic_cast<IResource*>(CreateD3D11DepthBufferTarget(mpResourceManager, mpGraphicsContext, name, texParams, result));
	}

	TypeId CD3D11DepthBufferTargetFactory::GetResourceTypeId() const
	{
		return IDepthBufferTarget::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateD3D11DepthBufferTargetFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		CD3D11DepthBufferTargetFactory* pDepthBufferTargetFactoryInstance = new (std::nothrow) CD3D11DepthBufferTargetFactory();

		if (!pDepthBufferTargetFactoryInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pDepthBufferTargetFactoryInstance->Init(pResourceManager, pGraphicsContext);

		if (result != RC_OK)
		{
			delete pDepthBufferTargetFactoryInstance;

			pDepthBufferTargetFactoryInstance = nullptr;
		}

		return pDepthBufferTargetFactoryInstance;
	}
}

#endif