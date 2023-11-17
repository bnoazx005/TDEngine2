#include "./../include/CD3D11Texture2D.h"
#include "./../include/CD3D11GraphicsContext.h"
#include "./../include/CD3D11Mappings.h"
#include "./../include/CD3D11Utils.h"
#include <utils/Utils.h>
#include <cstring>
#include <algorithm>


#if defined (TDE2_USE_WINPLATFORM)

namespace TDEngine2
{
	CD3D11Texture2D::CD3D11Texture2D() :
		CBaseTexture2D(), mpTexture(nullptr)
	{
	}

	void CD3D11Texture2D::Bind(U32 slot)
	{
		CBaseTexture2D::Bind(slot);

		mp3dDeviceContext->VSSetShaderResources(slot, 1, &mpShaderTextureView);
		mp3dDeviceContext->PSSetShaderResources(slot, 1, &mpShaderTextureView);
		mp3dDeviceContext->GSSetShaderResources(slot, 1, &mpShaderTextureView);
		mp3dDeviceContext->CSSetShaderResources(slot, 1, &mpShaderTextureView);

		if (mIsRandomlyWriteable)
		{
			mp3dDeviceContext->CSSetUnorderedAccessViews(slot, 1, &mpUavTextureView, nullptr);
		}
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

		region.left = regionRect.x;
		region.top = regionRect.y;
		region.right = regionRect.x + regionRect.width;
		region.bottom = regionRect.y + regionRect.height;
		region.back = 1;
		region.front = 0;

		U32 rowPitch = regionRect.width * CD3D11Mappings::GetNumOfChannelsOfFormat(mFormat);

		mp3dDeviceContext->UpdateSubresource(mpTexture, 0, &region, pData, rowPitch, rowPitch * regionRect.height);

		return RC_OK;
	}

	std::vector<U8> CD3D11Texture2D::GetInternalData()
	{
		/// \note create temporary texture with D3D11_USAGE_STAGING flag
		ID3D11Texture2D* pTempTexture = _createD3D11TextureResource(mpGraphicsContext, mWidth, mHeight, mFormat, mNumOfMipLevels, mNumOfSamples, mSamplingQuality, DTAT_CPU_READ).Get();

		/// \note copy data from actual texture into helper one
		mp3dDeviceContext->CopyResource(pTempTexture, mpTexture);
		//D3D11_BOX sourceRegion;
		//sourceRegion.left = 0;
		//sourceRegion.right = mWidth;
		//sourceRegion.top = 0;
		//sourceRegion.bottom = mHeight;
		//sourceRegion.front = 0;
		//sourceRegion.back = 1;

		//mp3dDeviceContext->CopySubresourceRegion(pTempTexture, 0, 0, 0, 0, mpTexture, 0, &sourceRegion);

		/// \note retrieve pointer to memory from temp texture
		D3D11_MAPPED_SUBRESOURCE mappedData;

		if (FAILED(mp3dDeviceContext->Map(pTempTexture, 0, D3D11_MAP_READ, 0x0, &mappedData)))
		{
			return {};
		}

		const size_t size = static_cast<size_t>(mWidth * mHeight * CD3D11Mappings::GetFormatSize(mFormat));

		std::vector<U8> pixelsData(size);

		memcpy(&pixelsData[0], mappedData.pData, size);

		mp3dDeviceContext->Unmap(pTempTexture, 0);

		pTempTexture->Release();

		return std::move(pixelsData);
	}

	ID3D11Texture2D* CD3D11Texture2D::GetInternalTexturePtr() const
	{
		return mpTexture;
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


	E_RESULT_CODE CD3D11Texture2D::_createInternalTextureHandler(IGraphicsContext* pGraphicsContext, U32 width, U32 height, E_FORMAT_TYPE format,
		U32 mipLevelsCount, U32 samplesCount, U32 samplingQuality, bool isWriteable)
	{
		auto textureResult = _createD3D11TextureResource(pGraphicsContext, width, height, format, mipLevelsCount, samplesCount, samplingQuality, isWriteable);

		if (textureResult.HasError())
		{
			return textureResult.GetError();
		}

		mpTexture = textureResult.Get();

		E_RESULT_CODE result = _createShaderTextureView(mp3dDevice, mFormat, mNumOfMipLevels);

		if (isWriteable)
		{
			auto uavResourceViewResult = CreateUnorderedAccessView(mpTexture, mp3dDevice, mFormat);
			if (uavResourceViewResult.HasError())
			{
				return uavResourceViewResult.GetError() | result;
			}

			mpUavTextureView = uavResourceViewResult.Get();
		}

		return result;
	}

	TResult<ID3D11Texture2D*> CD3D11Texture2D::_createD3D11TextureResource(IGraphicsContext* pGraphicsContext, U32 width, U32 height, E_FORMAT_TYPE format,
		U32 mipLevelsCount, U32 samplesCount, U32 samplingQuality,
		U32 accessType, bool isWriteable)
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

		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.Format = CD3D11Mappings::GetDXGIFormat(format);
		textureDesc.SampleDesc.Count = samplesCount;
		textureDesc.SampleDesc.Quality = samplingQuality;
		textureDesc.MipLevels = mipLevelsCount;
		textureDesc.ArraySize = 1; //single texture
		textureDesc.BindFlags = isCPUAccessible ? 0x0 : D3D11_BIND_SHADER_RESOURCE; /// default binding type for simple 2d textures
		textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
		textureDesc.Usage = isCPUAccessible ? D3D11_USAGE_STAGING : D3D11_USAGE_DEFAULT; /// \todo replace it with corresponding mapping

		if (isWriteable) /// \note If the texture is marked as writeable we make it UAV resource too
		{
			textureDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
		}

		ID3D11Texture2D* pTexture = nullptr;

		/// create blank texture with specified parameters
		if (FAILED(mp3dDevice->CreateTexture2D(&textureDesc, nullptr, &pTexture))) /// \todo Implement HRESULT -> E_RESULT_CODE converter function
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<ID3D11Texture2D*>(pTexture);
	}

	E_RESULT_CODE CD3D11Texture2D::_createShaderTextureView(ID3D11Device* p3dDevice, E_FORMAT_TYPE format, U32 mipLevelsCount)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;

		memset(&viewDesc, 0, sizeof(viewDesc));

		viewDesc.Format = CD3D11Mappings::GetDXGIFormat(format);
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
		return CREATE_IMPL(ITexture2D, CD3D11Texture2D, result, pResourceManager, pGraphicsContext, name);
	}

	TDE2_API ITexture2D* CreateD3D11Texture2D(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
		const TTexture2DParameters& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ITexture2D, CD3D11Texture2D, result, pResourceManager, pGraphicsContext, name, params);
	}


	CD3D11Texture2DFactory::CD3D11Texture2DFactory() :
		CBaseObject()
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

	IResource* CD3D11Texture2DFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		const TTexture2DParameters& texParams = static_cast<const TTexture2DParameters&>(params);

		return dynamic_cast<IResource*>(CreateD3D11Texture2D(mpResourceManager, mpGraphicsContext, name, texParams, result));
	}

	IResource* CD3D11Texture2DFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		const static TTexture2DParameters defaultTextureParams{ 2, 2, FT_NORM_UBYTE4, 1, 1, 0 };

		TTexture2DParameters overridenParams = defaultTextureParams;
		overridenParams.mLoadingPolicy = params.mLoadingPolicy;

		// create blank texture, which sizes equals to 2 x 2 pixels of RGBA format
		return dynamic_cast<IResource*>(CreateD3D11Texture2D(mpResourceManager, mpGraphicsContext, name, overridenParams, result));
	}

	TypeId CD3D11Texture2DFactory::GetResourceTypeId() const
	{
		return ITexture2D::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateD3D11Texture2DFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceFactory, CD3D11Texture2DFactory, result, pResourceManager, pGraphicsContext);
	}


	/*!
		\brief CD3D11TextureImpl's definition
	*/


	static TResult<ID3D11Texture2D*> CreateTexture2DResourceInternal(ID3D11Device* p3dDevice, const TInitTextureImplParams& params)
	{
		D3D11_TEXTURE2D_DESC textureDesc;

		memset(&textureDesc, 0, sizeof(textureDesc));

		bool isCPUAccessible = params.mUsageType != E_TEXTURE_IMPL_USAGE_TYPE::STATIC;

		textureDesc.Width = params.mWidth;
		textureDesc.Height = params.mHeight;
		textureDesc.Format = CD3D11Mappings::GetDXGIFormat(params.mFormat);
		textureDesc.SampleDesc.Count = params.mNumOfSamples;
		textureDesc.SampleDesc.Quality = params.mSamplingQuality;
		textureDesc.MipLevels = params.mNumOfMipLevels;
		textureDesc.ArraySize = (params.mType == E_TEXTURE_IMPL_TYPE::CUBEMAP) ? 6 : params.mArraySize;

		textureDesc.BindFlags = 0x0;

		if (E_BIND_GRAPHICS_TYPE::BIND_SHADER_RESOURCE == (params.mBindFlags & E_BIND_GRAPHICS_TYPE::BIND_SHADER_RESOURCE))
		{
			textureDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
		}

		if (E_BIND_GRAPHICS_TYPE::BIND_DEPTH_BUFFER == (params.mBindFlags & E_BIND_GRAPHICS_TYPE::BIND_DEPTH_BUFFER))
		{
			textureDesc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
		}
		else if (E_BIND_GRAPHICS_TYPE::BIND_RENDER_TARGET == (params.mBindFlags & E_BIND_GRAPHICS_TYPE::BIND_RENDER_TARGET))
		{
			textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		}

		if (E_BIND_GRAPHICS_TYPE::BIND_UNORDERED_ACCESS == (params.mBindFlags & E_BIND_GRAPHICS_TYPE::BIND_UNORDERED_ACCESS))
		{
			textureDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
		}

		textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
		textureDesc.Usage = isCPUAccessible ? D3D11_USAGE_STAGING : D3D11_USAGE_DEFAULT;
		textureDesc.MiscFlags = (params.mType == E_TEXTURE_IMPL_TYPE::CUBEMAP) ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0x0;

		ID3D11Texture2D* pTexture = nullptr;

		/// create blank texture with specified parameters
		if (FAILED(p3dDevice->CreateTexture2D(&textureDesc, nullptr, &pTexture))) 
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<ID3D11Texture2D*>(pTexture);
	}


	static TResult<ID3D11ShaderResourceView*> CreateShaderResourceViewInternal(ID3D11Device* p3dDevice, ID3D11Resource* pTextureResource, const TInitTextureImplParams& params)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;

		memset(&viewDesc, 0, sizeof(viewDesc));

		const bool isCubemap = (params.mType == E_TEXTURE_IMPL_TYPE::CUBEMAP);

		viewDesc.Format = CD3D11Mappings::GetDXGIFormat(params.mFormat);
		viewDesc.ViewDimension = isCubemap ? D3D11_SRV_DIMENSION_TEXTURECUBE : (params.mArraySize > 1 ? D3D11_SRV_DIMENSION_TEXTURE2DARRAY : D3D11_SRV_DIMENSION_TEXTURE2D);

		switch (params.mType)
		{
			case E_TEXTURE_IMPL_TYPE::CUBEMAP:
				viewDesc.TextureCube.MipLevels = params.mNumOfMipLevels;
				viewDesc.TextureCube.MostDetailedMip = 0;
				break;
			case E_TEXTURE_IMPL_TYPE::TEXTURE_2D:
				viewDesc.Texture2D.MipLevels = params.mNumOfMipLevels;
				viewDesc.Texture2D.MostDetailedMip = 0;
				break;
			case E_TEXTURE_IMPL_TYPE::TEXTURE_2D_ARRAY:
				viewDesc.Texture2DArray.MipLevels = params.mNumOfMipLevels;
				viewDesc.Texture2DArray.ArraySize = params.mArraySize;
				viewDesc.Texture2DArray.FirstArraySlice = 0;
				viewDesc.Texture2DArray.MostDetailedMip = 0;
				break;
		}

		ID3D11ShaderResourceView* pShaderResourceView = nullptr;

		if (FAILED(p3dDevice->CreateShaderResourceView(pTextureResource, &viewDesc, &pShaderResourceView)))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<ID3D11ShaderResourceView*>(pShaderResourceView);
	}


	static TResult<ID3D11RenderTargetView*> CreateRenderTargetViewInternal(ID3D11Device* p3dDevice, ID3D11Resource* pTextureResource, const TInitTextureImplParams& params)
	{
		D3D11_RENDER_TARGET_VIEW_DESC viewDesc;

		memset(&viewDesc, 0, sizeof(viewDesc));

		const bool isCubemap = (params.mType == E_TEXTURE_IMPL_TYPE::CUBEMAP);

		viewDesc.Format = CD3D11Mappings::GetDXGIFormat(params.mFormat);
		viewDesc.ViewDimension = (params.mArraySize > 1 || isCubemap) ? D3D11_RTV_DIMENSION_TEXTURE2DARRAY : D3D11_RTV_DIMENSION_TEXTURE2D;

		switch (params.mType)
		{
			case E_TEXTURE_IMPL_TYPE::TEXTURE_2D:
				viewDesc.Texture2D.MipSlice = 0;
				break;
			case E_TEXTURE_IMPL_TYPE::TEXTURE_2D_ARRAY:
			case E_TEXTURE_IMPL_TYPE::CUBEMAP:
				viewDesc.Texture2DArray.ArraySize = isCubemap ? 6 : params.mArraySize;
				viewDesc.Texture2DArray.MipSlice = 0;
				viewDesc.Texture2DArray.FirstArraySlice = 0;
				break;
		}

		ID3D11RenderTargetView* pRenderTargetView = nullptr;

		if (FAILED(p3dDevice->CreateRenderTargetView(pTextureResource, &viewDesc, &pRenderTargetView)))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<ID3D11RenderTargetView*>(pRenderTargetView);
	}


	static TResult<ID3D11DepthStencilView*> CreateDepthStencilViewInternal(ID3D11Device* p3dDevice, ID3D11Resource* pTextureResource, const TInitTextureImplParams& params)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc;

		memset(&viewDesc, 0, sizeof(viewDesc));

		const bool isCubemap = (params.mType == E_TEXTURE_IMPL_TYPE::CUBEMAP);

		viewDesc.Format = CD3D11Mappings::GetDXGIFormat(params.mFormat);
		viewDesc.ViewDimension = (params.mArraySize > 1 || isCubemap) ? D3D11_DSV_DIMENSION_TEXTURE2DARRAY : D3D11_DSV_DIMENSION_TEXTURE2D;

		switch (params.mType)
		{
			case E_TEXTURE_IMPL_TYPE::TEXTURE_2D:
				viewDesc.Texture2D.MipSlice = 0;
				break;
			case E_TEXTURE_IMPL_TYPE::TEXTURE_2D_ARRAY:
			case E_TEXTURE_IMPL_TYPE::CUBEMAP:
				viewDesc.Texture2DArray.ArraySize = isCubemap ? 6 : params.mArraySize;
				viewDesc.Texture2DArray.MipSlice = 0;
				viewDesc.Texture2DArray.FirstArraySlice = 0;
				break;
		}

		ID3D11DepthStencilView* pDepthStencilView = nullptr;

		if (FAILED(p3dDevice->CreateDepthStencilView(pTextureResource, &viewDesc, &pDepthStencilView)))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<ID3D11DepthStencilView*>(pDepthStencilView);
	}


	static TResult<ID3D11UnorderedAccessView*> CreateUnorderedAccessViewInternal(ID3D11Device* p3dDevice, ID3D11Resource* pTextureResource, const TInitTextureImplParams& params)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC viewDesc;

		memset(&viewDesc, 0, sizeof(viewDesc));

		const bool isCubemap = (params.mType == E_TEXTURE_IMPL_TYPE::CUBEMAP);

		viewDesc.Format = CD3D11Mappings::GetDXGIFormat(params.mFormat);
		viewDesc.ViewDimension = (params.mArraySize > 1 || isCubemap) ? D3D11_UAV_DIMENSION_TEXTURE2DARRAY : D3D11_UAV_DIMENSION_TEXTURE2D;

		switch (params.mType)
		{
			case E_TEXTURE_IMPL_TYPE::TEXTURE_2D:
				viewDesc.Texture2D.MipSlice = 0;
				break;
			case E_TEXTURE_IMPL_TYPE::TEXTURE_2D_ARRAY:
			case E_TEXTURE_IMPL_TYPE::CUBEMAP:
				viewDesc.Texture2DArray.ArraySize = isCubemap ? 6 : params.mArraySize;
				viewDesc.Texture2DArray.MipSlice = 0;
				viewDesc.Texture2DArray.FirstArraySlice = 0;
				break;
		}

		ID3D11UnorderedAccessView* pUnorderedAccessView = nullptr;

		if (FAILED(p3dDevice->CreateUnorderedAccessView(pTextureResource, &viewDesc, &pUnorderedAccessView)))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<ID3D11UnorderedAccessView*>(pUnorderedAccessView);
	}


	CD3D11TextureImpl::CD3D11TextureImpl():
		CBaseObject()
	{
	}

	E_RESULT_CODE CD3D11TextureImpl::Init(IGraphicsContext* pGraphicsContext, const TInitTextureImplParams& params)
	{
		if (!pGraphicsContext || 
			!params.mWidth ||
			!params.mHeight ||
			params.mDepth < 1 ||
			params.mNumOfMipLevels < 1 ||
			!params.mArraySize)
		{
			return RC_INVALID_ARGS;
		}

		mInitParams = params;

		auto&& internalData = pGraphicsContext->GetInternalData();
		
		mp3dDeviceContext = internalData.mD3D11.mp3dDeviceContext;
		mp3dDevice = internalData.mD3D11.mp3dDevice;

		E_RESULT_CODE result = _onInitInternal();
		if (RC_OK != result)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CD3D11TextureImpl::Resize(U32 width, U32 height, U32 depth)
	{
		E_RESULT_CODE result = _onFreeInternal();

		mInitParams.mWidth = width;
		mInitParams.mHeight = height;
		mInitParams.mDepth = depth;

		return _onInitInternal();
	}

	ID3D11Resource* CD3D11TextureImpl::GetTextureResource()
	{
		return mpTextureResource;
	}

	ID3D11ShaderResourceView* CD3D11TextureImpl::GetShaderResourceView()
	{
		return mpShaderTextureView;
	}

	ID3D11RenderTargetView* CD3D11TextureImpl::GetRenderTargetView()
	{
		return mpRenderTargetView;
	}

	ID3D11DepthStencilView* CD3D11TextureImpl::GetDepthStencilView()
	{
		return mpDepthStencilView;
	}

	ID3D11UnorderedAccessView* CD3D11TextureImpl::GetUnorderedAccessView()
	{
		return mpUavTextureView;
	}

	const TInitTextureParams& CD3D11TextureImpl::GetParams() const
	{
		return mInitParams;
	}

	E_RESULT_CODE CD3D11TextureImpl::_onInitInternal()
	{
		auto createResourceResult = CreateTexture2DResourceInternal(mp3dDevice, mInitParams);
		if (createResourceResult.HasError())
		{
			return createResourceResult.GetError();
		}

		// \todo Add support of 3D textures

		mpTextureResource = createResourceResult.Get();

		auto createDefaultSrvResult = CreateShaderResourceViewInternal(mp3dDevice, mpTextureResource, mInitParams);
		if (createDefaultSrvResult.HasError())
		{
			return createDefaultSrvResult.GetError();
		}

		mpShaderTextureView = createDefaultSrvResult.Get();

		if (E_BIND_GRAPHICS_TYPE::BIND_DEPTH_BUFFER == (mInitParams.mBindFlags & E_BIND_GRAPHICS_TYPE::BIND_DEPTH_BUFFER))
		{
			auto createDsvResult = CreateDepthStencilViewInternal(mp3dDevice, mpTextureResource, mInitParams);
			if (createDsvResult.HasError())
			{
				return createDsvResult.GetError();
			}

			mpDepthStencilView = createDsvResult.Get();
		}
		else if (E_BIND_GRAPHICS_TYPE::BIND_RENDER_TARGET == (mInitParams.mBindFlags & E_BIND_GRAPHICS_TYPE::BIND_RENDER_TARGET))
		{
			auto createRtvResult = CreateRenderTargetViewInternal(mp3dDevice, mpTextureResource, mInitParams);
			if (createRtvResult.HasError())
			{
				return createRtvResult.GetError();
			}

			mpRenderTargetView = createRtvResult.Get();
		}

		if (E_BIND_GRAPHICS_TYPE::BIND_UNORDERED_ACCESS == (mInitParams.mBindFlags & E_BIND_GRAPHICS_TYPE::BIND_UNORDERED_ACCESS))
		{
			auto createUavResult = CreateUnorderedAccessViewInternal(mp3dDevice, mpTextureResource, mInitParams);
			if (createUavResult.HasError())
			{
				return createUavResult.GetError();
			}

			mpUavTextureView = createUavResult.Get();
		}

		return RC_OK;
	}

	E_RESULT_CODE CD3D11TextureImpl::_onFreeInternal()
	{
		E_RESULT_CODE result = RC_OK;

		result = result | SafeReleaseCOMPtr<ID3D11Resource>(&mpTextureResource);
		result = result | SafeReleaseCOMPtr<ID3D11ShaderResourceView>(&mpShaderTextureView);

		if (mpUavTextureView)
		{
			result = result | SafeReleaseCOMPtr<ID3D11UnorderedAccessView>(&mpUavTextureView);
		}

		if (mpRenderTargetView)
		{
			result = result | SafeReleaseCOMPtr<ID3D11RenderTargetView>(&mpRenderTargetView);
		}

		if (mpDepthStencilView)
		{
			result = result | SafeReleaseCOMPtr<ID3D11DepthStencilView>(&mpDepthStencilView);
		}

		return result;
	}


	TDE2_API ITextureImpl* CreateD3D11TextureImpl(IGraphicsContext* pGraphicsContext, const TInitTextureImplParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ITextureImpl, CD3D11TextureImpl, result, pGraphicsContext, params);
	}
}

#endif