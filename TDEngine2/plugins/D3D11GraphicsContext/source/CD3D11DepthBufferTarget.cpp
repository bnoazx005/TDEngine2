#include "./../include/CD3D11DepthBufferTarget.h"
#include "./../include/CD3D11GraphicsContext.h"
#include "./../include/CD3D11Mappings.h"
#include "./../include/CD3D11Utils.h"
#include "./../include/CD3D11Texture2D.h"
#include <graphics/ITexture2D.h>
#include <cstring>


#if defined (TDE2_USE_WINPLATFORM)

namespace TDEngine2
{
	CD3D11DepthBufferTarget::CD3D11DepthBufferTarget() :
		CBaseDepthBufferTarget(), mpDepthBufferTexture(nullptr), mpShaderTextureView(nullptr), mpDepthBufferTargetView(nullptr), mpUavDepthBufferView(nullptr)
	{
	}

	E_RESULT_CODE CD3D11DepthBufferTarget::Load()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		auto&& pResourceLoader = mpResourceManager->GetResourceLoader<IDepthBufferTarget>();

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
			(result = SafeReleaseCOMPtr<ID3D11DepthStencilView>(&mpDepthBufferTargetView)) != RC_OK ||
			(result = SafeReleaseCOMPtr<ID3D11UnorderedAccessView>(&mpUavDepthBufferView)) != RC_OK)
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
		mp3dDeviceContext->CSSetShaderResources(slot, 1, &mpShaderTextureView);

		if (mIsRandomlyWriteable)
		{
			mp3dDeviceContext->CSSetUnorderedAccessViews(slot, 1, &mpUavDepthBufferView, nullptr);
		}
	}

	void CD3D11DepthBufferTarget::UnbindFromShader()
	{
		ID3D11ShaderResourceView* pNullSRV = nullptr;
		ID3D11UnorderedAccessView* pNullUAV = nullptr;

		mp3dDeviceContext->VSSetShaderResources(mLastBindingSlot, 1, &pNullSRV);
		mp3dDeviceContext->PSSetShaderResources(mLastBindingSlot, 1, &pNullSRV);
		mp3dDeviceContext->GSSetShaderResources(mLastBindingSlot, 1, &pNullSRV);
		mp3dDeviceContext->CSSetShaderResources(mLastBindingSlot, 1, &pNullSRV);

		if (mIsRandomlyWriteable)
		{
			mp3dDeviceContext->CSSetUnorderedAccessViews(mLastBindingSlot, 1, &pNullUAV, nullptr);
		}
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


	static TResult<ID3D11UnorderedAccessView*> CreateUnorderedAccessView(ID3D11Texture2D* pTexture, ID3D11Device* p3dDevice, E_FORMAT_TYPE format)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC viewDesc = {};

		viewDesc.Format = CD3D11Mappings::GetDXGIFormat(format);
		viewDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;

		viewDesc.Texture2D.MipSlice = 0;

		ID3D11UnorderedAccessView* pView = nullptr;

		if (FAILED(p3dDevice->CreateUnorderedAccessView(pTexture, &viewDesc, &pView)))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<ID3D11UnorderedAccessView*>(pView);
	}


	E_RESULT_CODE CD3D11DepthBufferTarget::_createInternalTextureHandler(IGraphicsContext* pGraphicsContext, const TRenderTargetParameters& params)
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

		textureDesc.Width              = params.mWidth;
		textureDesc.Height             = params.mHeight;
		textureDesc.Format             = CD3D11Mappings::GetTypelessVersionOfFormat(params.mFormat);
		textureDesc.SampleDesc.Count   = params.mNumOfSamples;
		textureDesc.SampleDesc.Quality = params.mSamplingQuality;
		textureDesc.MipLevels          = params.mNumOfMipLevels;
		textureDesc.ArraySize          = params.mCreateAsCubemap ? 6 : 1; 
		textureDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
		textureDesc.CPUAccessFlags     = 0; // \note for now we deny access to depth buffer on CPU side
		textureDesc.Usage              = D3D11_USAGE_DEFAULT; /// \todo replace it with corresponding mapping

		if (params.mCreateAsCubemap)
		{
			textureDesc.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
		}

		if (params.mIsWriteable)
		{
			textureDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
		}

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

		if (params.mIsWriteable)
		{
			auto uavResourceCreationResult = CreateUnorderedAccessView(mpDepthBufferTexture, mp3dDevice, mFormat);
			if (uavResourceCreationResult.HasError())
			{
				return uavResourceCreationResult.GetError();
			}

			mpUavDepthBufferView = uavResourceCreationResult.Get();
		}

		return _createShaderTextureView(mp3dDevice, CD3D11Mappings::GetBestFitStrongTypeFormat(params.mFormat), mNumOfMipLevels);
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
															  const TRenderTargetParameters& params, E_RESULT_CODE& result)
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

	IResource* CD3D11DepthBufferTargetFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		const TRenderTargetParameters& texParams = static_cast<const TRenderTargetParameters&>(params);

		return dynamic_cast<IResource*>(CreateD3D11DepthBufferTarget(mpResourceManager, mpGraphicsContext, name, texParams, result));
	}

	IResource* CD3D11DepthBufferTargetFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		const TRenderTargetParameters& texParams = static_cast<const TRenderTargetParameters&>(params);

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