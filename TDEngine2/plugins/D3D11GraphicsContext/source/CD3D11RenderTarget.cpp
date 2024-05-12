#include "../include/CD3D11RenderTarget.h"
#include "../include/CD3D11GraphicsContext.h"
#include "../include/CD3D11Mappings.h"
#include "../include/CD3D11Utils.h"
#include "../include/CD3D11Texture.h"
#include <graphics/IGraphicsObjectManager.h>
#include <graphics/ITexture2D.h>
#include <cstring>


#if defined (TDE2_USE_WINPLATFORM)

namespace TDEngine2
{
	CD3D11RenderTarget::CD3D11RenderTarget() :
		CBaseRenderTarget(), mCurrTextureHandle(TTextureHandleId::Invalid)
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

		mpGraphicsContext->GetGraphicsObjectManager()->DestroyTexture(mCurrTextureHandle);
		mCurrTextureHandle = TTextureHandleId::Invalid;

		return RC_OK;
	}

	void CD3D11RenderTarget::Bind(U32 slot)
	{
		CBaseRenderTarget::Bind(slot);

		mpGraphicsContext->SetTexture(slot, mCurrTextureHandle);

		/*if (mIsRandomlyWriteable)
		{
			mp3dDeviceContext->CSSetUnorderedAccessViews(slot, 1, &mpUavTextureView, nullptr);
		}*/
	}

	void CD3D11RenderTarget::UnbindFromShader()
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

	E_RESULT_CODE CD3D11RenderTarget::Blit(ITexture2D*& pDestTexture)
	{
		if (!pDestTexture)
		{
			return RC_INVALID_ARGS;
		}

		mpGraphicsContext->CopyResource(mCurrTextureHandle, pDestTexture->GetHandle());

		return RC_OK;
	}

	E_RESULT_CODE CD3D11RenderTarget::GenerateMipMaps()
	{
		auto pGraphicsObjectManager = mpGraphicsContext->GetGraphicsObjectManager();
		auto pTexture = DynamicPtrCast<CD3D11TextureImpl>(pGraphicsObjectManager->GetTexturePtr(mCurrTextureHandle));

		if (!pTexture)
		{
			return RC_FAIL;
		}

		// \todo Replace with API of IGraphicsContext
		mp3dDeviceContext->GenerateMips(pTexture->GetShaderResourceView());

		return RC_OK;
	}

	ID3D11RenderTargetView* CD3D11RenderTarget::GetRenderTargetView() const
	{
		auto pGraphicsObjectManager = mpGraphicsContext->GetGraphicsObjectManager();
		auto pTexture = DynamicPtrCast<CD3D11TextureImpl>(pGraphicsObjectManager->GetTexturePtr(mCurrTextureHandle));

		return pTexture ? pTexture->GetRenderTargetView() : nullptr;
	}

	E_RESULT_CODE CD3D11RenderTarget::_createInternalTextureHandler(IGraphicsContext* pGraphicsContext, const TRenderTargetParameters& params)
	{
		mp3dDeviceContext = pGraphicsContext->GetInternalData().mD3D11.mp3dDeviceContext;

		TInitTextureImplParams createTextureParams{};
		createTextureParams.mWidth = params.mWidth;
		createTextureParams.mHeight = params.mHeight;
		createTextureParams.mFormat = params.mFormat;
		createTextureParams.mNumOfMipLevels = params.mNumOfMipLevels;
		createTextureParams.mNumOfSamples = params.mNumOfSamples;
		createTextureParams.mSamplingQuality = params.mSamplingQuality;
		createTextureParams.mType = E_TEXTURE_IMPL_TYPE::TEXTURE_2D;
		createTextureParams.mUsageType = E_TEXTURE_IMPL_USAGE_TYPE::STATIC;
		createTextureParams.mBindFlags = E_BIND_GRAPHICS_TYPE::BIND_SHADER_RESOURCE | E_BIND_GRAPHICS_TYPE::BIND_RENDER_TARGET;
		createTextureParams.mName = mName.c_str();

		if (params.mIsWriteable)
		{
			createTextureParams.mBindFlags = createTextureParams.mBindFlags | E_BIND_GRAPHICS_TYPE::BIND_UNORDERED_ACCESS;
		}

		auto createTextureResult = mpGraphicsContext->GetGraphicsObjectManager()->CreateTexture(createTextureParams);
		if (createTextureResult.HasError())
		{
			return createTextureResult.GetError();
		}

		mCurrTextureHandle = createTextureResult.Get();

		return RC_OK;
	}


	TDE2_API IRenderTarget* CreateD3D11RenderTarget(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
													const TRenderTargetParameters& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IRenderTarget, CD3D11RenderTarget, result, pResourceManager, pGraphicsContext, name, params);
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

		const TRenderTargetParameters& texParams = static_cast<const TRenderTargetParameters&>(params);

		return dynamic_cast<IResource*>(CreateD3D11RenderTarget(mpResourceManager, mpGraphicsContext, name, texParams, result));
	}

	IResource* CD3D11RenderTargetFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		const TRenderTargetParameters& texParams = static_cast<const TRenderTargetParameters&>(params);

		return dynamic_cast<IResource*>(CreateD3D11RenderTarget(mpResourceManager, mpGraphicsContext, name, texParams, result));
	}

	TypeId CD3D11RenderTargetFactory::GetResourceTypeId() const
	{
		return IRenderTarget::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateD3D11RenderTargetFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceFactory, CD3D11RenderTargetFactory, result, pResourceManager, pGraphicsContext);
	}
}

#endif