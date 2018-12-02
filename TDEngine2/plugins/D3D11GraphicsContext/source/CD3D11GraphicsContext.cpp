#include "./../include/CD3D11GraphicsContext.h"
#include "./../include/CD3D11Utils.h"
#include "./../include/CD3D11GraphicsObjectManager.h"
#include "./../include/CD3D11Mappings.h"
#include <core/IWindowSystem.h>


#if defined(TDE2_USE_WIN32PLATFORM)

#pragma comment(lib, "d3d11.lib")


namespace TDEngine2
{
	CD3D11GraphicsContext::CD3D11GraphicsContext() :
		mIsInitialized(false)
	{
	}
	
	E_RESULT_CODE CD3D11GraphicsContext::Init(IWindowSystem* pWindowSystem)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pWindowSystem)
		{
			return RC_INVALID_ARGS;
		}

		const D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0
		};

		const U32 numOfFeatureLevels = sizeof(featureLevels) / sizeof(featureLevels[0]);

		HRESULT d3dCallsResult = S_OK;

		UINT flags = 0x0;

#if defined(_DEBUG)
		flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		d3dCallsResult = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels, numOfFeatureLevels,
											D3D11_SDK_VERSION, &mp3dDevice, &mCurrFeatureLevel, &mp3dDeviceContext);

		if (FAILED(d3dCallsResult))
		{
			return RC_FAIL;
		}

		if (mCurrFeatureLevel < D3D_FEATURE_LEVEL_11_0)
		{
			return RC_GAPI_IS_NOT_SUPPORTED;
		}

#if _DEBUG	/// Acquiring ID3D11Debug interface
		mp3dDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&mpDebuggerInstance));
#endif

		/// create a swap chain
		E_RESULT_CODE result = _createSwapChain(pWindowSystem, mp3dDevice);

		if (result != RC_OK)
		{
			return result;
		}

		/// create a back buffer
		if ((result = _createBackBuffer(mpSwapChain, mp3dDevice)) != RC_OK)
		{
			return result;
		}

		/// create a depth buffer
		UINT width  = pWindowSystem->GetWidth();
		UINT height = pWindowSystem->GetHeight();
		
		if ((result = _createDepthBuffer(width, height, mpSwapChain, mp3dDevice, &mpDefaultDepthStencilView, &mpDefaultDepthStencilBuffer)) != RC_OK)
		{
			return result;
		}

		mp3dDeviceContext->OMSetRenderTargets(1, &mpBackBufferView, mpDefaultDepthStencilView);

		/// set up a default viewport
		SetViewport(0.0f, 0.0f, width, height, 0.0f, 1.0f);

#if _HAS_CXX17
		mInternalDataObject = TD3D11CtxInternalData { mp3dDevice, mp3dDeviceContext };
#else
		mInternalDataObject.mD3D11 = { mp3dDevice, mp3dDeviceContext };
#endif

		mpGraphicsObjectManager = CreateD3D11GraphicsObjectManager(this, result);

		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}
	
	E_RESULT_CODE CD3D11GraphicsContext::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		if ((result = SafeReleaseCOMPtr<ID3D11Device>(&mp3dDevice)) != RC_OK							||
			(result = SafeReleaseCOMPtr<ID3D11DeviceContext>(&mp3dDeviceContext)) != RC_OK				||
			(result = SafeReleaseCOMPtr<ID3D11DepthStencilView>(&mpDefaultDepthStencilView)) != RC_OK	||
			(result = SafeReleaseCOMPtr<ID3D11Texture2D>(&mpDefaultDepthStencilBuffer)) != RC_OK		||
			(result = SafeReleaseCOMPtr<ID3D11RenderTargetView>(&mpBackBufferView)) != RC_OK			||
			(result = SafeReleaseCOMPtr<IDXGISwapChain>(&mpSwapChain)) != RC_OK)
		{
			return result;
		}

#if _DEBUG
		if (mpDebuggerInstance)
		{
			mpDebuggerInstance->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);

			if ((result = SafeReleaseCOMPtr<ID3D11Debug>(&mpDebuggerInstance)) != RC_OK)
			{
				return result;
			}
		}
#endif

		if (mpGraphicsObjectManager)
		{
			if ((result = mpGraphicsObjectManager->Free()) != RC_OK)
			{
				return result;
			}
		}

		delete this;

		mIsInitialized = false;

		return RC_FAIL;
	}
	
	void CD3D11GraphicsContext::ClearBackBuffer(const TColor32F& color)
	{
		const F32 clearColorArray[4] { color.r, color.g, color.b, color.a };

		mp3dDeviceContext->ClearRenderTargetView(mpBackBufferView, clearColorArray);
	}

	void CD3D11GraphicsContext::ClearDepthBuffer(F32 value)
	{
		mp3dDeviceContext->ClearDepthStencilView(mpDefaultDepthStencilView, D3D11_CLEAR_DEPTH, value, 0);
	}

	void CD3D11GraphicsContext::ClearStencilBuffer(U8 value)
	{
		mp3dDeviceContext->ClearDepthStencilView(mpDefaultDepthStencilView, D3D11_CLEAR_STENCIL, 0.0f, value);
	}

	void CD3D11GraphicsContext::Present()
	{
		mpSwapChain->Present(mIsVSyncEnabled, 0);
	}

	void CD3D11GraphicsContext::SetViewport(F32 x, F32 y, F32 width, F32 height, F32 minDepth, F32 maxDepth)
	{
		D3D11_VIEWPORT viewport;

		memset(&viewport, 0, sizeof(viewport));

		viewport.TopLeftX = x;
		viewport.TopLeftY = y;
		viewport.MinDepth = minDepth;
		viewport.MaxDepth = maxDepth;
		viewport.Width    = static_cast<float>(width);
		viewport.Height   = static_cast<float>(height);

		mp3dDeviceContext->RSSetViewports(1, &viewport);
	}
	
	TMatrix4 CD3D11GraphicsContext::CalcPerspectiveMatrix(F32 fov, F32 aspect, F32 zn, F32 zf)
	{
		return PerspectiveProj(fov, aspect, zn, zf, 0.0f, 1.0f, -1.0f);
	}

	TMatrix4 CD3D11GraphicsContext::CalcOrthographicMatrix(F32 left, F32 top, F32 right, F32 bottom, F32 zn, F32 zf)
	{
		return OrthographicProj(left, top, right, bottom, zn, zf, 0.0f, 1.0f, -1.0f);
	}

	void CD3D11GraphicsContext::Draw(E_PRIMITIVE_TOPOLOGY_TYPE topology, U32 startVertex, U32 numOfVertices)
	{
		mp3dDeviceContext->IASetPrimitiveTopology(CD3D11Mappings::GetPrimitiveTopology(topology));

		mp3dDeviceContext->Draw(numOfVertices, startVertex);
	}

	void CD3D11GraphicsContext::DrawIndexed(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, U32 baseVertex, U32 startIndex, U32 numOfIndices)
	{
		mp3dDeviceContext->IASetPrimitiveTopology(CD3D11Mappings::GetPrimitiveTopology(topology));

		mp3dDeviceContext->DrawIndexed(numOfIndices, startIndex, baseVertex);
	}

	const TGraphicsCtxInternalData& CD3D11GraphicsContext::GetInternalData() const
	{
		return mInternalDataObject;
	}

	E_ENGINE_SUBSYSTEM_TYPE CD3D11GraphicsContext::GetType() const
	{
		return EST_GRAPHICS_CONTEXT;
	}

	IGraphicsObjectManager* CD3D11GraphicsContext::GetGraphicsObjectManager() const
	{
		return mpGraphicsObjectManager;
	}

	E_RESULT_CODE CD3D11GraphicsContext::_createSwapChain(const IWindowSystem* pWindowSystem, ID3D11Device* p3dDevice)
	{
		DXGI_SWAP_CHAIN_DESC swapChainDesc;

		memset(&swapChainDesc, 0, sizeof(swapChainDesc));

		U32 applicationFlags = pWindowSystem->GetFlags();

		mIsVSyncEnabled = applicationFlags & P_VSYNC;

		HWND hwnd = pWindowSystem->GetInternalData().mWindowHandler;

		UINT width  = pWindowSystem->GetWidth();
		UINT height = pWindowSystem->GetHeight();

		swapChainDesc.BufferDesc.Width                   = width;
		swapChainDesc.BufferDesc.Height                  = height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator   = 60;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferDesc.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.BufferDesc.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Format                  = (applicationFlags & P_HARDWARE_GAMMA_CORRECTION) ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.OutputWindow                       = hwnd;
		swapChainDesc.Windowed                           = !(applicationFlags & P_FULLSCREEN);
		swapChainDesc.Flags                              = 0;
		swapChainDesc.BufferCount                        = 1;
		swapChainDesc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.SampleDesc.Count                   = 1;

		IDXGIDevice*  pDXGIDevice = nullptr;
		IDXGIAdapter* pDXGIAdapter = nullptr;
		IDXGIFactory* pDXGIFactory = nullptr;
		
		HRESULT result = mp3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&pDXGIDevice));

		if (FAILED(result))
		{
			return RC_FAIL;
		}

		result = pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&pDXGIAdapter));

		if (FAILED(result))
		{
			return RC_FAIL;
		}

		result = pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&pDXGIFactory));

		if (FAILED(result))
		{
			pDXGIDevice->Release();

			return RC_FAIL;
		}

		result = pDXGIFactory->CreateSwapChain(mp3dDevice, &swapChainDesc, &mpSwapChain);

		if (FAILED(result))
		{
			pDXGIDevice->Release();
			pDXGIAdapter->Release();

			return RC_FAIL;
		}

		pDXGIDevice->Release();
		pDXGIAdapter->Release();
		pDXGIFactory->Release();
		
		return RC_OK;
	}

	E_RESULT_CODE CD3D11GraphicsContext::_createBackBuffer(IDXGISwapChain* pSwapChain, ID3D11Device* p3dDevice)
	{
		ID3D11Texture2D* pBackBuffer;

		if (FAILED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer))))
		{
			return RC_FAIL;
		}

		if (FAILED(p3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &mpBackBufferView)))
		{
			return RC_FAIL;
		}

		pBackBuffer->Release();

		return RC_OK;
	}

	E_RESULT_CODE CD3D11GraphicsContext::_createDepthBuffer(U32 width, U32 height, IDXGISwapChain* pSwapChain, ID3D11Device* p3dDevice,
															ID3D11DepthStencilView** ppDepthStencilView, ID3D11Texture2D** pDepthStencilBuffer)
	{
		D3D11_TEXTURE2D_DESC depthBufferDesc;

		memset(&depthBufferDesc, 0, sizeof(depthBufferDesc));

		depthBufferDesc.Width            = width;
		depthBufferDesc.Height           = height;
		depthBufferDesc.MipLevels        = 1;
		depthBufferDesc.ArraySize        = 1;
		depthBufferDesc.Format           = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthBufferDesc.SampleDesc.Count = 1;
		depthBufferDesc.Usage            = D3D11_USAGE_DEFAULT;
		depthBufferDesc.BindFlags        = D3D11_BIND_DEPTH_STENCIL;

		if (FAILED(p3dDevice->CreateTexture2D(&depthBufferDesc, nullptr, pDepthStencilBuffer)))
		{
			return RC_FAIL;
		}

		if (FAILED(p3dDevice->CreateDepthStencilView(mpDefaultDepthStencilBuffer, nullptr, ppDepthStencilView)))
		{
			return RC_FAIL;
		}

		return RC_OK;
	}


	TDE2_API IGraphicsContext* CreateD3D11GraphicsContext(IWindowSystem* pWindowSystem, E_RESULT_CODE& result)
	{
		CD3D11GraphicsContext* pGraphicsContextInstance = new (std::nothrow) CD3D11GraphicsContext();

		if (!pGraphicsContextInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pGraphicsContextInstance->Init(pWindowSystem);

		if (result != RC_OK)
		{
			delete pGraphicsContextInstance;

			pGraphicsContextInstance = nullptr;
		}

		return dynamic_cast<IGraphicsContext*>(pGraphicsContextInstance);
	}
}

#endif