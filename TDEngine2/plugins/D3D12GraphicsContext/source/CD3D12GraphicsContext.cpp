#include "../include/CD3D12GraphicsContext.h"
#include "../include/CD3D12Mappings.h"
#include "../include/CD3D12GraphicsObjectManager.h"
#include "../include/CD3D12Resources.h"
#include <core/IGraphicsContext.h>
#include <core/IEventManager.h>
#include <core/CBaseObject.h>
#include <core/IWindowSystem.h>
#include <editor/CPerfProfiler.h>
#include <tuple>


#if defined(TDE2_USE_WINPLATFORM) /// Used only on Windows platform

#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include "../deps/D3D12MemAlloc/D3D12MemAlloc.h"
#include "../deps/dx12/d3dx12_barriers.h"
#include "../deps/dx12/d3dx12_root_signature.h"


template <typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#if TDE2_DEBUG_MODE
	#pragma comment(lib, "dxguid.lib") 
#endif


namespace TDEngine2
{
#if TDE2_DEBUG_MODE

	static TResult<ComPtr<ID3D12Debug1>> InitDebugLayer()
	{
		ComPtr<ID3D12Debug> tempDebugController = nullptr;

		if (FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&tempDebugController))))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		ComPtr<ID3D12Debug1> outputDebugController = nullptr;

		if (FAILED(tempDebugController->QueryInterface(IID_PPV_ARGS(&outputDebugController))))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		outputDebugController->EnableDebugLayer();
		outputDebugController->SetEnableGPUBasedValidation(true);

		return Wrench::TOkValue<ComPtr<ID3D12Debug1>>(outputDebugController);
	}

#endif


	static TResult<std::tuple<ComPtr<ID3D12Device>, ComPtr<IDXGIAdapter1>>> InitPhysicalDevice(ComPtr<IDXGIFactory4> pFactory, D3D_FEATURE_LEVEL& selectedFeatureLevel)
	{
		TDE2_PROFILER_SCOPE("D3D12CreateDevice");

		static const std::array<D3D_FEATURE_LEVEL, 5> FEATURE_LEVELS
		{
			D3D_FEATURE_LEVEL_12_2,
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0
		};

		ComPtr<IDXGIAdapter1> pAdapter = nullptr;
		ComPtr<ID3D12Device> p3dDevice = nullptr;

		for (UINT i = 0; pFactory->EnumAdapters1(i, pAdapter.GetAddressOf()) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			DXGI_ADAPTER_DESC1 adapterDesc;
			pAdapter->GetDesc1(&adapterDesc);

			// Ignore software adapters
			if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				continue;
			}

			for (const D3D_FEATURE_LEVEL& currFeatureLevel : FEATURE_LEVELS)
			{
				if (SUCCEEDED(D3D12CreateDevice(pAdapter.Get(), currFeatureLevel, IID_PPV_ARGS(&p3dDevice))))
				{
					selectedFeatureLevel = currFeatureLevel;
					return Wrench::TOkValue<std::tuple<ComPtr<ID3D12Device>, ComPtr<IDXGIAdapter1>>>({ p3dDevice, pAdapter });
				}
			}
		}

		return Wrench::TOkValue<std::tuple<ComPtr<ID3D12Device>, ComPtr<IDXGIAdapter1>>>({ nullptr, nullptr });
	}


	static TResult<ComPtr<ID3D12CommandQueue>> CreateCommandQueue(ComPtr<ID3D12Device> p3dDevice)
	{
		ComPtr<ID3D12CommandQueue> pCommandQueue = nullptr;

		D3D12_COMMAND_QUEUE_DESC commandQueueCreateDesc{};
		commandQueueCreateDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		commandQueueCreateDesc.Type  = D3D12_COMMAND_LIST_TYPE_DIRECT;

		if (FAILED(p3dDevice->CreateCommandQueue(&commandQueueCreateDesc, IID_PPV_ARGS(&pCommandQueue))))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<ComPtr<ID3D12CommandQueue>>(pCommandQueue);
	}


	static TResult<ComPtr<ID3D12CommandAllocator>> CreateCommandAllocator(ComPtr<ID3D12Device> p3dDevice)
	{
		ComPtr<ID3D12CommandAllocator> pCommandAllocator = nullptr;

		if (FAILED(p3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pCommandAllocator))))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<ComPtr<ID3D12CommandAllocator>>(pCommandAllocator);
	}


	static TResult<ComPtr<ID3D12GraphicsCommandList>> CreateCommandList(ComPtr<ID3D12Device> p3dDevice, ComPtr<ID3D12CommandAllocator> pCommandAllocator)
	{
		ComPtr<ID3D12GraphicsCommandList> pCommandList = nullptr;

		if (FAILED(p3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, pCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&pCommandList))))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		pCommandList->Close();

		return Wrench::TOkValue<ComPtr<ID3D12GraphicsCommandList>>(pCommandList);
	}


	static TResult<ComPtr<D3D12MA::Allocator>> CreateMemoryAllocator(ComPtr<ID3D12Device> p3dDevice, ComPtr<IDXGIAdapter1> pAdapter)
	{
		D3D12MA::ALLOCATOR_DESC allocatorDesc {};
		allocatorDesc.pDevice  = p3dDevice.Get();
		allocatorDesc.pAdapter = pAdapter.Get();
		allocatorDesc.Flags    = static_cast<D3D12MA::ALLOCATOR_FLAGS>(D3D12MA::ALLOCATOR_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED | D3D12MA::ALLOCATOR_FLAG_DEFAULT_POOLS_NOT_ZEROED);

		D3D12MA::Allocator* pAllocator = nullptr;
		if (FAILED(D3D12MA::CreateAllocator(&allocatorDesc, &pAllocator)))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<ComPtr<D3D12MA::Allocator>>(pAllocator);
	}


	CD3D12DeviceContext* CreateD3D12DeviceContext(TPtr<IWindowSystem>, E_RESULT_CODE&);
	CD3D12Swapchain* CreateD3D12Swapchain(CD3D12DeviceContext*, TPtr<IWindowSystem>, E_RESULT_CODE&);
	CD3D12CommandBuffer* CreateD3D12CommandBuffer(CD3D12DeviceContext*, E_RESULT_CODE&);


	class CD3D12Fence
	{
		public:
			CD3D12Fence() = default;
			CD3D12Fence(CD3D12DeviceContext* pDeviceContext, U64 initialValue = 0);
			~CD3D12Fence();

			void WaitForSignal();
		private:
			CD3D12DeviceContext* mpDeviceContext = nullptr;
			ComPtr<ID3D12Fence>  mpFence = nullptr;
			HANDLE               mEventHandle = nullptr;
			U64                  mFenceValue = 0;
	};


	/*!
		\brief CD3D12DeviceContext's definition
	*/

	class CD3D12DeviceContext : public CBaseObject
	{
		public:
			friend CD3D12DeviceContext* CreateD3D12DeviceContext(TPtr<IWindowSystem>, E_RESULT_CODE&);
		public:
			E_RESULT_CODE Init(TPtr<IWindowSystem> pWindowSystem);

			TPtr<CD3D12Swapchain> CreateSwapchain();

			CD3D12Fence CreateFence(bool signaled = true);
			//VkSemaphore CreateSemaphore(VkSemaphoreCreateFlags flags = 0x0);

			//E_RESULT_CODE SubmitCommands(TPtr<CD3D12CommandBuffer> pCommandBuffer, VkSemaphore waitSemaphore = VK_NULL_HANDLE, VkSemaphore signalSemaphore = VK_NULL_HANDLE, VkFence fence = VK_NULL_HANDLE);
			void WaitForIdle();

			ComPtr<ID3D12Device> GetDevice() const { return mp3dDevice; }
			//const VkPhysicalDevice GetPhysicalDevice() const { return mPhysicalDevice; }
			//const VkDevice GetDevice() const { return mDevice; }
			//const VkSurfaceKHR GetSwapchainSurface() const { return mSwapChainSurface; }
			//const VkInstance GetInstance() const { return mInstance; }

			ComPtr<ID3D12CommandQueue> GetCommandQueue() { return mpCommandQueue; }

			ComPtr<IDXGIFactory4> GetFactory() { return mpObjectsFactory; }

			ComPtr<D3D12MA::Allocator> GetMemoryAllocator() const { return mpMemoryAllocator; }
		private:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D12DeviceContext)

			E_RESULT_CODE _onFreeInternal() override;
		private:
			TPtr<IWindowSystem>         mpWindowSystem = nullptr;

			D3D_FEATURE_LEVEL           mCurrFeatureLevel = D3D_FEATURE_LEVEL_12_2;
			ComPtr<ID3D12Device>        mp3dDevice = nullptr;
			ComPtr<IDXGIAdapter1>       mpAdapter = nullptr;
									    
#if TDE2_DEBUG_MODE					    
			ComPtr<ID3D12Debug1>        mpDebugController = nullptr;
			ComPtr<ID3D12DebugDevice>   mpDebugDevice = nullptr;
#endif								    
			    
			ComPtr<D3D12MA::Allocator>  mpMemoryAllocator = nullptr;

	//		VkInstance                  mInstance = VK_NULL_HANDLE;
	//		VkPhysicalDevice            mPhysicalDevice = VK_NULL_HANDLE;
	//		VkDevice                    mDevice = VK_NULL_HANDLE;

	//		VkSurfaceKHR                mSwapChainSurface = VK_NULL_HANDLE;

			ComPtr<IDXGIFactory4>       mpObjectsFactory = nullptr;

			// queues
			ComPtr<ID3D12CommandQueue>  mpCommandQueue = nullptr;
	//		VkQueue                     mGraphicsQueue = VK_NULL_HANDLE;
	//		VkQueue                     mPresentQueue = VK_NULL_HANDLE;

	//		TQueuesCreateInfo           mQueuesInfo{};
	};


	/*!
		\brief CD3D12Fence's definition
	*/

	CD3D12Fence::CD3D12Fence(CD3D12DeviceContext* pDeviceContext, U64 initialValue) :
		mpDeviceContext(pDeviceContext), mFenceValue(initialValue)
	{
		if (FAILED(pDeviceContext->GetDevice()->CreateFence(initialValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mpFence))))
		{
			TDE2_ASSERT(false);
		}

		mEventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (!mEventHandle)
		{
			TDE2_ASSERT(false);
		}
	}

	CD3D12Fence::~CD3D12Fence()
	{
		CloseHandle(mEventHandle);
	}

	void CD3D12Fence::WaitForSignal()
	{
		const UINT64 fenceValue = mFenceValue;

		HRESULT result = mpDeviceContext->GetCommandQueue()->Signal(mpFence.Get(), fenceValue);
		TDE2_ASSERT(SUCCEEDED(result));

		++mFenceValue;

		if (mpFence->GetCompletedValue() < fenceValue)
		{
			result = mpFence->SetEventOnCompletion(fenceValue, mEventHandle);
			TDE2_ASSERT(SUCCEEDED(result));

			WaitForSingleObject(mEventHandle, INFINITE);
		}
	}


	CD3D12DeviceContext::CD3D12DeviceContext():
		CBaseObject()
	{
	}

	E_RESULT_CODE CD3D12DeviceContext::Init(TPtr<IWindowSystem> pWindowSystem)
	{
		mpWindowSystem = pWindowSystem;

		UINT dxgiFactoryFlags = 0;

#if TDE2_DEBUG_MODE
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

		if (FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&mpObjectsFactory))))
		{
			return RC_FAIL;
		}

#if TDE2_DEBUG_MODE
		auto initDebugResult = InitDebugLayer();
		if (initDebugResult.HasError())
		{
			return initDebugResult.GetError();
		}

		mpDebugController = initDebugResult.Get();
#endif

		auto createDeviceResult = InitPhysicalDevice(mpObjectsFactory, mCurrFeatureLevel);
		if (createDeviceResult.HasError())
		{
			return createDeviceResult.GetError();
		}

		std::tie(mp3dDevice, mpAdapter) = createDeviceResult.Get();

		auto createAllocator = CreateMemoryAllocator(mp3dDevice, mpAdapter);
		if (createAllocator.HasError())
		{
			return createAllocator.GetError();
		}

		mpMemoryAllocator = createAllocator.Get();

#if TDE2_DEBUG_MODE
		if (FAILED(mp3dDevice->QueryInterface(IID_PPV_ARGS(&mpDebugDevice))))
		{
			return RC_FAIL;
		}
#endif

		auto createCommandQueueResult = CreateCommandQueue(mp3dDevice);
		if (createCommandQueueResult.HasError())
		{
			return createCommandQueueResult.GetError();
		}

		mpCommandQueue = createCommandQueueResult.Get();

		mIsInitialized  = true;

		return RC_OK;
	}

	TPtr<CD3D12Swapchain> CD3D12DeviceContext::CreateSwapchain()
	{
		E_RESULT_CODE result = RC_OK;
		return TPtr<CD3D12Swapchain>(CreateD3D12Swapchain(this, mpWindowSystem, result));
	}

	CD3D12Fence CD3D12DeviceContext::CreateFence(bool signaled)
	{
		return CD3D12Fence(this, 0);
	}

	E_RESULT_CODE CD3D12DeviceContext::_onFreeInternal()
	{
		return RC_OK;
	}

	void CD3D12DeviceContext::WaitForIdle()
	{
	}


	CD3D12DeviceContext* CreateD3D12DeviceContext(TPtr<IWindowSystem> pWindowSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CD3D12DeviceContext, CD3D12DeviceContext, result, pWindowSystem);
	}


	TDE2_DEFINE_SCOPED_PTR(CD3D12DeviceContext);


	/*!
		\brief CD3D12Swapchain's definition
	*/

	class CD3D12Swapchain : public CBaseObject
	{
		public:
			friend CD3D12Swapchain* CreateD3D12Swapchain(CD3D12DeviceContext*, TPtr<IWindowSystem>, E_RESULT_CODE&);
		private:
			typedef std::array<ComPtr<ID3D12Resource>, CD3D12GraphicsContext::BACK_BUFFERS_COUNT> TRenderTargetsArray;
		public:
			E_RESULT_CODE Init(CD3D12DeviceContext* pDeviceContext, TPtr<IWindowSystem> pWindowSystem);

			//E_RESULT_CODE AcquireNextImage(VkSemaphore semaphore);
			E_RESULT_CODE Present();

			void InvalidateState() { mIsValid = false; }
			E_RESULT_CODE TryProcessInvalidateState();

			U32 GetBackBufferTargetIndex() const { return mpSwapChain->GetCurrentBackBufferIndex(); }
			ComPtr<ID3D12Resource> GetCurrRenderTarget() const { return mpRenderTargets[GetBackBufferTargetIndex()]; }
			D3D12_CPU_DESCRIPTOR_HANDLE GetCurrRenderTargetView() const { return CD3DX12_CPU_DESCRIPTOR_HANDLE(mpRenderTargetViewsHeap->GetCPUDescriptorHandleForHeapStart(), GetBackBufferTargetIndex(), mRTVDescriptorSize); }
			
			ComPtr<ID3D12Resource> GetDefaultDepthStencilTarget() const { return mpDefaulDepthStencilTarget; }
			//D3D12_CPU_DESCRIPTOR_HANDLE GetDefaultDepthStencilTargetView() const { return mpDefaulDepthStencilTarget; }

			E_FORMAT_TYPE GetBackBuffersFormat() const { return mBackBufferFormat; }

			ComPtr<IDXGISwapChain3> GetHandle() const { return mpSwapChain; }

			bool IsValid() const { return mIsValid; }
		private:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D12Swapchain)

			E_RESULT_CODE _onInitInternal();
			E_RESULT_CODE _onFreeInternal() override;
		private:
			CD3D12DeviceContext*         mpDeviceContext = nullptr;
			TPtr<IWindowSystem>          mpWindowSystem = nullptr;

			ComPtr<IDXGISwapChain3>      mpSwapChain = nullptr;
			TRenderTargetsArray          mpRenderTargets;
			ComPtr<ID3D12Resource>       mpDefaulDepthStencilTarget = nullptr;

			E_FORMAT_TYPE                mBackBufferFormat = E_FORMAT_TYPE::FT_UNKNOWN;

			ComPtr<ID3D12DescriptorHeap> mpRenderTargetViewsHeap = nullptr;
			U32                          mRTVDescriptorSize = 0;

			bool                         mIsVSyncEnabled = false;
			bool                         mIsValid = true;
	};


	CD3D12Swapchain::CD3D12Swapchain():
		CBaseObject()
	{
	}

	E_RESULT_CODE CD3D12Swapchain::Init(CD3D12DeviceContext* pDeviceContext, TPtr<IWindowSystem> pWindowSystem)
	{
		mpDeviceContext = pDeviceContext;
		mpWindowSystem  = pWindowSystem;

		return _onInitInternal();
	}

	E_RESULT_CODE CD3D12Swapchain::Present()
	{
		if (FAILED(mpSwapChain->Present(static_cast<UINT>(mIsVSyncEnabled), 0)))
		{
			return RC_FAIL;
		}

		return RC_OK;
	}

	E_RESULT_CODE CD3D12Swapchain::TryProcessInvalidateState()
	{
		if (mIsValid)
		{
			return RC_OK;
		}

		mpDeviceContext->WaitForIdle();

		E_RESULT_CODE result = RC_OK;

		result = result | _onFreeInternal();
		result = result | _onInitInternal();

		return result;
	}

	E_RESULT_CODE CD3D12Swapchain::_onInitInternal()
	{
		HWND hwnd = mpWindowSystem->GetInternalData().mWindowHandler;

		ComPtr<IDXGISwapChain1> pSwapChain = nullptr;

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
		swapChainDesc.Width              = mpWindowSystem->GetWidth();
		swapChainDesc.Height             = mpWindowSystem->GetHeight();
		swapChainDesc.BufferCount        = CD3D12GraphicsContext::BACK_BUFFERS_COUNT;
		swapChainDesc.SampleDesc.Count   = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.Format             = (mpWindowSystem->GetFlags() & P_HARDWARE_GAMMA_CORRECTION) ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_DISCARD;

		mBackBufferFormat = (mpWindowSystem->GetFlags() & P_HARDWARE_GAMMA_CORRECTION) ? E_FORMAT_TYPE::FT_NORM_BYTE4_SRGB : E_FORMAT_TYPE::FT_UBYTE4_BGRA_UNORM;

		ComPtr<IDXGIFactory4> pFactory = mpDeviceContext->GetFactory();

		if (FAILED(pFactory->CreateSwapChainForHwnd(mpDeviceContext->GetCommandQueue().Get(), hwnd, &swapChainDesc, nullptr, nullptr, pSwapChain.GetAddressOf())))
		{
			return RC_FAIL;
		}

		pFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
		pSwapChain.As(&mpSwapChain);

		// \note Create descriptor heap
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
		rtvHeapDesc.NumDescriptors = CD3D12GraphicsContext::BACK_BUFFERS_COUNT;
		rtvHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		ComPtr<ID3D12Device> p3dDevice = mpDeviceContext->GetDevice();

		if (FAILED(p3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mpRenderTargetViewsHeap))))
		{
			return RC_FAIL;
		}

		mRTVDescriptorSize = p3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		D3D12_CPU_DESCRIPTOR_HANDLE	rtvHandle(mpRenderTargetViewsHeap->GetCPUDescriptorHandleForHeapStart());

		for (U32 i = 0; i < rtvHeapDesc.NumDescriptors; ++i)
		{
			if (FAILED(mpSwapChain->GetBuffer(i, IID_PPV_ARGS(&mpRenderTargets[i]))))
			{
				return RC_FAIL;
			}

			p3dDevice->CreateRenderTargetView(mpRenderTargets[i].Get(), nullptr, rtvHandle);
			rtvHandle.ptr += mRTVDescriptorSize;
		}

		mIsVSyncEnabled = mpWindowSystem->GetFlags() & P_VSYNC;

		mIsInitialized = true;
		mIsValid = true;

		return RC_OK;
	}

	E_RESULT_CODE CD3D12Swapchain::_onFreeInternal()
	{
		return RC_OK;
	}


	CD3D12Swapchain* CreateD3D12Swapchain(CD3D12DeviceContext* pDeviceContext, TPtr<IWindowSystem> pWindowSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CD3D12Swapchain, CD3D12Swapchain, result, pDeviceContext, pWindowSystem);
	}


	TDE2_DEFINE_SCOPED_PTR(CD3D12Swapchain);

	
	/*!
		\brief CD3D12CommandBuffer's declaration
	*/

	class CD3D12CommandBuffer : public CBaseObject
	{
		public:
			friend CD3D12CommandBuffer* CreateD3D12CommandBuffer(CD3D12DeviceContext*, E_RESULT_CODE&);
		public:
			E_RESULT_CODE Init(CD3D12DeviceContext* pDeviceContext);

			E_RESULT_CODE Begin();
			E_RESULT_CODE End();

			E_RESULT_CODE Reset();

			ComPtr<ID3D12GraphicsCommandList> GetHandle() const { return mpCommandList; }
			const CD3D12Fence& GetFenceEntry() const { return mFenceEntry; }
		private:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D12CommandBuffer)

			E_RESULT_CODE _onFreeInternal() override;
		private:
			CD3D12DeviceContext*              mpDeviceContext = nullptr;

			ComPtr<ID3D12CommandAllocator>    mpCommandListAllocator = nullptr;
			ComPtr<ID3D12GraphicsCommandList> mpCommandList = nullptr;

			CD3D12Fence                       mFenceEntry;
	};


	/*!
		\brief CD3D12CommandBuffer's definition
	*/

	CD3D12CommandBuffer::CD3D12CommandBuffer() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CD3D12CommandBuffer::Init(CD3D12DeviceContext* pDeviceContext)
	{
		mpDeviceContext = pDeviceContext;

		ComPtr<ID3D12Device> p3dDevice = pDeviceContext->GetDevice();

		if (FAILED(p3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mpCommandListAllocator))))
		{
			return RC_FAIL;
		}

		if (FAILED(p3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mpCommandListAllocator.Get(), nullptr, IID_PPV_ARGS(&mpCommandList))))
		{
			return RC_FAIL;
		}

		mFenceEntry = mpDeviceContext->CreateFence();
		
		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CD3D12CommandBuffer::Begin()
	{
		return Reset();
	}

	E_RESULT_CODE CD3D12CommandBuffer::End()
	{
		return FAILED(mpCommandList->Close()) ? RC_FAIL : RC_OK;
	}

	E_RESULT_CODE CD3D12CommandBuffer::Reset()
	{
		return FAILED(mpCommandList->Reset(mpCommandListAllocator.Get(), nullptr)) ? RC_FAIL : RC_OK;
	}

	E_RESULT_CODE CD3D12CommandBuffer::_onFreeInternal()
	{
		return RC_OK;
	}


	CD3D12CommandBuffer* CreateD3D12CommandBuffer(CD3D12DeviceContext* pDeviceContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CD3D12CommandBuffer, CD3D12CommandBuffer, result, pDeviceContext);
	}


	TDE2_DEFINE_SCOPED_PTR(CD3D12CommandBuffer);



	CD3D12GraphicsContext::CD3D12GraphicsContext() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CD3D12GraphicsContext::Init(TPtr<IWindowSystem> pWindowSystem)
	{
		TDE2_PROFILER_SCOPE("CD3D12GraphicsContext::Init");

		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pWindowSystem)
		{
			return RC_INVALID_ARGS;
		}

		mpWindowSystem = pWindowSystem;

		E_RESULT_CODE result = RC_OK;

		mpDeviceContext = TPtr<CD3D12DeviceContext>(CreateD3D12DeviceContext(mpWindowSystem, result));
		if (RC_OK != result)
		{
			return result;
		}

		mpSwapchain = mpDeviceContext->CreateSwapchain();
		TDE2_ASSERT(mpSwapchain);

		auto createCommandAllocatorResult = CreateCommandAllocator(mpDeviceContext->GetDevice());
		if (createCommandAllocatorResult.HasError())
		{
			return createCommandAllocatorResult.GetError();
		}

		mpImmediateCommandBuffer = TPtr<CD3D12CommandBuffer>(CreateD3D12CommandBuffer(mpDeviceContext.Get(), result));
		if (RC_OK != result)
		{
			return result;
		}

		for (USIZE i = 0; i < BACK_BUFFERS_COUNT; ++i)
		{
			E_RESULT_CODE localResult = RC_OK;

			mpCommandBuffers[i] = TPtr<CD3D12CommandBuffer>(CreateD3D12CommandBuffer(mpDeviceContext.Get(), localResult));
			result = result | localResult;
		}

		if (RC_OK != result)
		{
			return result;
		}

//
//		/// create a depth buffer
//		UINT width = pWindowSystem->GetWidth();
//		UINT height = pWindowSystem->GetHeight();
//
//		if ((result = _createDepthBuffer(width, height, mpSwapChain, mp3dDevice, &mpDefaultDepthStencilView, &mpDefaultDepthStencilBuffer)) != RC_OK)
//		{
//			return result;
//		}
//
//		mpCurrDepthStencilView = mpDefaultDepthStencilView;
//
//		mp3dDeviceContext->OMSetRenderTargets(1, &mpBackBufferView, mpDefaultDepthStencilView);
//
//		/// set up a default viewport
//		SetViewport(0.0f, 0.0f, static_cast<F32>(width), static_cast<F32>(height), 0.0f, 1.0f);
//

//
		mpGraphicsObjectManager = TPtr<IGraphicsObjectManager>(CreateD3D12GraphicsObjectManager(this, result));
		mpGraphicsObjectManagerD3D12Impl = dynamic_cast<CD3D12GraphicsObjectManager*>(mpGraphicsObjectManager.Get());

		if (result != RC_OK)
		{
			return result;
		}

		mDescriptorsBindingsTable.Reset();

		mpEventManager = pWindowSystem->GetEventManager();

		if (!mpEventManager)
		{
			return RC_FAIL;
		}

		mpEventManager->Subscribe(TOnWindowResized::GetTypeId(), this);

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsContext::_onFreeInternal()
	{
		TDE2_PROFILER_SCOPE("CD3D12GraphicsContext::_onFreeInternal");

		_waitForIdle();

		return RC_OK;
	}

	void CD3D12GraphicsContext::_waitForIdle()
	{
		TDE2_PROFILER_SCOPE("CD3D12GraphicsContext::_waitForIdle");
		
		/*const UINT64 fence = mCurrentFence;
		HRESULT result = mpCommandQueue->Signal(mpFrameFence.Get(), fence);
		TDE2_ASSERT(SUCCEEDED(result));

		++mCurrentFence;

		if (mpFrameFence->GetCompletedValue() < fence)
		{
			result = mpFrameFence->SetEventOnCompletion(fence, mFenceEventHandle);
			TDE2_ASSERT(SUCCEEDED(result));

			WaitForSingleObject(mFenceEventHandle, INFINITE);
		}

		mCurrBackBufferIndex = mpSwapChain->GetCurrentBackBufferIndex();*/
	}

	ID3D12GraphicsCommandList* CD3D12GraphicsContext::_getCurrCommandListPtr()
	{
		return mpCommandBuffers[mpSwapchain->GetBackBufferTargetIndex()]->GetHandle().Get();
	}

	void CD3D12GraphicsContext::BeginFrame()
	{
		/*HRESULT result = mpCommandAllocator->Reset();
		TDE2_ASSERT(SUCCEEDED(result));

		result = mpCommandList->Reset(mpCommandAllocator.Get(), nullptr);*/

		{
			std::lock_guard<std::mutex> lock(mGarbageCollectorMutex);

			for (auto& currGarbageEntity : mAwaitingDeletionObjects[mpSwapchain->GetBackBufferTargetIndex()])
			{
				if (!currGarbageEntity.mpAllocation)
				{
					continue;
				}

				currGarbageEntity.mpAllocation->Release();
			}

			mAwaitingDeletionObjects[mpSwapchain->GetBackBufferTargetIndex()].clear();
		}

		mDescriptorsBindingsTable.Reset();
	}

	void CD3D12GraphicsContext::Present()
	{
		TDE2_PROFILER_SCOPE("CD3D12GraphicsContext::Present");
		/*
		CD3DX12_RESOURCE_BARRIER rtTransitionBarrier = CD3DX12_RESOURCE_BARRIER::Transition(mpRenderTargetViews[mCurrBackBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		mpCommandList->ResourceBarrier(1, &rtTransitionBarrier);
		mpCommandList->Close();

		ID3D12CommandList* ppCommandLists[] = { mpCommandList.Get() };
		mpCommandQueue->ExecuteCommandLists(1, ppCommandLists); */

		mpSwapchain->Present();
		_waitForIdle();
	}

	E_RESULT_CODE CD3D12GraphicsContext::DestroyObjectDeffered(Microsoft::WRL::ComPtr<ID3D12Resource> pResource, D3D12MA::Allocation* pAllocation)
	{
		if (!pResource)
		{
			return RC_INVALID_ARGS;
		}

		TGarbageEntity garbageEntity{};
		garbageEntity.mpResource   = pResource;
		garbageEntity.mpAllocation = pAllocation;

		std::lock_guard<std::mutex> lock(mGarbageCollectorMutex);
		mAwaitingDeletionObjects[mpSwapchain->GetBackBufferTargetIndex()].emplace_back(garbageEntity);

		return RC_OK;
	}

	void CD3D12GraphicsContext::SetViewport(F32 x, F32 y, F32 width, F32 height, F32 minDepth, F32 maxDepth)
	{
		D3D12_VIEWPORT viewport{};
		viewport.TopLeftX = x;
		viewport.TopLeftY = y;
		viewport.Width    = width;
		viewport.Height   = height;
		viewport.MinDepth = minDepth;
		viewport.MaxDepth = maxDepth;

		_getCurrCommandListPtr()->RSSetViewports(1, &viewport);
	}

	void CD3D12GraphicsContext::SetScissorRect(const TRectU32& scissorRect)
	{
		TDE2_ASSERT(mIsRenderPassActive); // \note To set up scissor rect outside of some render pass is meaningless

		D3D12_RECT internalScissorRect;

		internalScissorRect.left   = static_cast<LONG>(scissorRect.x);
		internalScissorRect.top    = static_cast<LONG>(scissorRect.y);
		internalScissorRect.right  = static_cast<LONG>(scissorRect.x + scissorRect.width);
		internalScissorRect.bottom = static_cast<LONG>(scissorRect.y + scissorRect.height);

		_getCurrCommandListPtr()->RSSetScissorRects(1, &internalScissorRect);
	}

	TMatrix4 CD3D12GraphicsContext::CalcPerspectiveMatrix(F32 fov, F32 aspect, F32 zn, F32 zf)
	{
		return PerspectiveProj(fov, aspect, zn, zf, 0.0f, 1.0f, -1.0f);
	}

	TMatrix4 CD3D12GraphicsContext::CalcOrthographicMatrix(F32 left, F32 top, F32 right, F32 bottom, F32 zn, F32 zf, bool isDepthless)
	{
		return OrthographicProj(left, top, right, bottom, zn, zf, 0.0f, 1.0f, -1.0f, isDepthless);
	}

	E_RESULT_CODE CD3D12GraphicsContext::SetVertexBuffer(U32 slot, TBufferHandleId vertexBufferHandle, U32 offset, U32 strideSize)
	{
		auto pBuffer = mpGraphicsObjectManagerD3D12Impl->GetD3D12BufferPtr(vertexBufferHandle);
		if (!pBuffer)
		{
			return RC_FAIL;
		}

		TDE2_ASSERT(E_BUFFER_TYPE::VERTEX == pBuffer->GetParams().mBufferType);

		//const VkBuffer vertexBuffers[] = { pBuffer->GetVulkanHandle() };
		//const VkDeviceSize offsets[] = { static_cast<USIZE>(offset) };
		TDE2_UNIMPLEMENTED();

		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsContext::SetIndexBuffer(TBufferHandleId indexBufferHandle, U32 offset)
	{
		auto pBuffer = mpGraphicsObjectManagerD3D12Impl->GetD3D12BufferPtr(indexBufferHandle);
		if (!pBuffer)
		{
			return RC_FAIL;
		}

		TDE2_ASSERT(E_BUFFER_TYPE::INDEX == pBuffer->GetParams().mBufferType);
		//_getCurrCommandListPtr()->IASetIndexBuffer(pBuffer->GetView());
		TDE2_UNIMPLEMENTED();

		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsContext::SetConstantBuffer(U32 slot, TBufferHandleId constantsBufferHandle)
	{
		auto pBuffer = mpGraphicsObjectManagerD3D12Impl->GetD3D12BufferPtr(constantsBufferHandle);
		if (!pBuffer)
		{
			return RC_FAIL;
		}

		TDE2_ASSERT(E_BUFFER_TYPE::CONSTANT == pBuffer->GetParams().mBufferType);
		TDE2_ASSERT(slot < TDescriptorsBindingsTable::MAX_CBV_COUNT);

		mDescriptorsBindingsTable.mConstantBuffers[slot] = constantsBufferHandle;

		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsContext::SetStructuredBuffer(U32 slot, TBufferHandleId bufferHandle, bool isWriteEnabled)
	{
		auto pBuffer = mpGraphicsObjectManagerD3D12Impl->GetD3D12BufferPtr(bufferHandle);
		if (!pBuffer)
		{
			return RC_OK;
		}

		TDE2_ASSERT(E_BUFFER_TYPE::STRUCTURED == pBuffer->GetParams().mBufferType);

		if (isWriteEnabled)
		{
			TDE2_ASSERT(slot < mDescriptorsBindingsTable.mUAVBuffers.size());

			mDescriptorsBindingsTable.mUAVBuffers[slot].mType = TDescriptorsBindingsTable::E_DESCRIPTOR_TYPE::BUFFER;
			mDescriptorsBindingsTable.mUAVBuffers[slot].mValue.mBuffer = bufferHandle;
		}
		else
		{
			TDE2_ASSERT(slot < mDescriptorsBindingsTable.mSRVBuffers.size());

			mDescriptorsBindingsTable.mSRVBuffers[slot].mType = TDescriptorsBindingsTable::E_DESCRIPTOR_TYPE::BUFFER;
			mDescriptorsBindingsTable.mSRVBuffers[slot].mValue.mBuffer = bufferHandle;
		}

		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsContext::SetTexture(U32 slot, TTextureHandleId textureHandle, bool isWriteEnabled)
	{
		auto pTexture = mpGraphicsObjectManagerD3D12Impl->GetD3D12TexturePtr(textureHandle);
		if (!pTexture)
		{
			return RC_FAIL;
		}

		if (isWriteEnabled)
		{
			TDE2_ASSERT(slot < mDescriptorsBindingsTable.mUAVBuffers.size());

			mDescriptorsBindingsTable.mUAVBuffers[slot].mType = TDescriptorsBindingsTable::E_DESCRIPTOR_TYPE::TEXTURE;
			mDescriptorsBindingsTable.mUAVBuffers[slot].mValue.mTexture = textureHandle;
		}
		else
		{
			TDE2_ASSERT(slot < mDescriptorsBindingsTable.mSRVBuffers.size());

			mDescriptorsBindingsTable.mSRVBuffers[slot].mType = TDescriptorsBindingsTable::E_DESCRIPTOR_TYPE::TEXTURE;
			mDescriptorsBindingsTable.mSRVBuffers[slot].mValue.mTexture = textureHandle;
		}

		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsContext::SetSampler(U32 slot, TTextureSamplerId samplerHandle)
	{
		mDescriptorsBindingsTable.mSamplers[slot] = samplerHandle;
		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsContext::UpdateTexture2D(TTextureHandleId textureHandle, U32 mipLevel, const TRectI32& regionRect, const void* pData, USIZE dataSize)
	{
		TDE2_UNIMPLEMENTED();
		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsContext::UpdateTexture2DArray(TTextureHandleId textureHandle, U32 index, const TRectI32& regionRect, const void* pData, USIZE dataSize)
	{
		TDE2_UNIMPLEMENTED();
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CD3D12GraphicsContext::UpdateCubemapTexture(TTextureHandleId textureHandle, E_CUBEMAP_FACE face, const TRectI32& regionRect, const void* pData, USIZE dataSize)
	{
		TDE2_UNIMPLEMENTED();
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CD3D12GraphicsContext::UpdateTexture3D(TTextureHandleId textureHandle, U32 depthFrom, U32 depthTo, const TRectI32& regionRect, const void* pData, USIZE dataSize)
	{
		TDE2_UNIMPLEMENTED();
		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsContext::CopyResource(TTextureHandleId sourceHandle, TTextureHandleId destHandle)
	{
		TDE2_UNIMPLEMENTED();
		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsContext::CopyResource(TBufferHandleId sourceHandle, TTextureHandleId destHandle)
	{
		TDE2_UNIMPLEMENTED();
		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsContext::CopyResource(TTextureHandleId sourceHandle, TBufferHandleId destHandle)
	{
		TDE2_UNIMPLEMENTED();
		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsContext::CopyResource(TBufferHandleId sourceHandle, TBufferHandleId destHandle)
	{
		TDE2_UNIMPLEMENTED();
		return RC_OK;
	}


	static D3D12_RESOURCE_STATES GetInternalResourceState(E_RESOURCE_LAYOUT layout)
	{
		switch (layout)
		{
			case E_RESOURCE_LAYOUT::RENDER_TARGET:
				return D3D12_RESOURCE_STATE_RENDER_TARGET;
			case E_RESOURCE_LAYOUT::DEPTH_STENCIL:
				return D3D12_RESOURCE_STATE_DEPTH_WRITE;
			case E_RESOURCE_LAYOUT::DEPTH_STENCIL_READONLY:
				return D3D12_RESOURCE_STATE_DEPTH_READ;
			case E_RESOURCE_LAYOUT::COPY_SRC:
				return D3D12_RESOURCE_STATE_COPY_SOURCE;
			case E_RESOURCE_LAYOUT::COPY_DEST:
				return D3D12_RESOURCE_STATE_COPY_DEST;
			case E_RESOURCE_LAYOUT::SHADER_RESOURCE:
				return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
			case E_RESOURCE_LAYOUT::INDIRECT_ARGS_BUFFER:
				return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
			case E_RESOURCE_LAYOUT::CONSTANT_BUFFER:
				return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
			case E_RESOURCE_LAYOUT::UAV_RESOURCE:
				return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		}

		return D3D12_RESOURCE_STATE_COMMON;
	}


	void CD3D12GraphicsContext::MemoryAccessBarrier(const std::variant<TBufferHandleId, TTextureHandleId> resourceHandle)
	{
		struct TVisitor
		{
			CD3D12GraphicsContext* mpGraphicsContext = nullptr;

			void operator()(TBufferHandleId bufferHandle)
			{
				TPtr<CD3D12Buffer> pBuffer = mpGraphicsContext->mpGraphicsObjectManagerD3D12Impl->GetD3D12BufferPtr(bufferHandle);
				if (!pBuffer)
				{
					GetBarriers().emplace_back(CD3DX12_RESOURCE_BARRIER::UAV(nullptr));
					return;
				}

				GetBarriers().emplace_back(CD3DX12_RESOURCE_BARRIER::UAV(pBuffer->GetHandle().Get()));
			}

			void operator()(TTextureHandleId textureHandle)
			{
				TPtr<CD3D12TextureImpl> pTextureImpl = mpGraphicsContext->mpGraphicsObjectManagerD3D12Impl->GetD3D12TexturePtr(textureHandle);
				if (!pTextureImpl)
				{
					GetBarriers().emplace_back(CD3DX12_RESOURCE_BARRIER::UAV(nullptr));
					return;
				}

				GetBarriers().emplace_back(CD3DX12_RESOURCE_BARRIER::UAV(pTextureImpl->GetHandle().Get()));
			}

			TBarriersArray& GetBarriers() { return mpGraphicsContext->mResourceBarriers[mpGraphicsContext->mpSwapchain->GetBackBufferTargetIndex()]; }
		};

		std::visit(TVisitor{ this }, resourceHandle);
	}

	void CD3D12GraphicsContext::TransitionBarrier(const TBufferTransitionBarrierInfo& barrierInfo)
	{
		TPtr<CD3D12Buffer> pBuffer = mpGraphicsObjectManagerD3D12Impl->GetD3D12BufferPtr(barrierInfo.mHandle);
		if (!pBuffer)
		{
			return;
		}

		mResourceBarriers[mpSwapchain->GetBackBufferTargetIndex()].emplace_back(
			CD3DX12_RESOURCE_BARRIER::Transition(pBuffer->GetHandle().Get(), GetInternalResourceState(barrierInfo.mCurrLayout), GetInternalResourceState(barrierInfo.mCurrLayout)));
	}

	void CD3D12GraphicsContext::TransitionBarrier(const TTextureTransitionBarrierInfo& barrierInfo)
	{
		TPtr<CD3D12TextureImpl> pTextureImpl = mpGraphicsObjectManagerD3D12Impl->GetD3D12TexturePtr(barrierInfo.mHandle);
		if (!pTextureImpl)
		{
			return;
		}

		const U32 subresourceId = barrierInfo.mMipLevel ? *barrierInfo.mMipLevel : D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		
		mResourceBarriers[mpSwapchain->GetBackBufferTargetIndex()].emplace_back(
			CD3DX12_RESOURCE_BARRIER::Transition(pTextureImpl->GetHandle().Get(), GetInternalResourceState(barrierInfo.mCurrLayout), GetInternalResourceState(barrierInfo.mCurrLayout), subresourceId));
	}

	void CD3D12GraphicsContext::DebugBarrier()
	{
#if TDE2_DEBUG_MODE
		mResourceBarriers[mpSwapchain->GetBackBufferTargetIndex()].emplace_back(CD3DX12_RESOURCE_BARRIER::UAV(nullptr));
#endif
	}

	void CD3D12GraphicsContext::FlushBarriers()
	{
		auto& resourceBarriers = mResourceBarriers[mpSwapchain->GetBackBufferTargetIndex()];
		if (resourceBarriers.empty())
		{
			return;
		}

		_getCurrCommandListPtr()->ResourceBarrier(static_cast<U32>(resourceBarriers.size()), resourceBarriers.data());

		resourceBarriers.clear();
	}

	E_RESULT_CODE CD3D12GraphicsContext::CopyCount(TBufferHandleId sourceHandle, TBufferHandleId destHandle, U32 offset)
	{
		TDE2_UNIMPLEMENTED();
		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsContext::GenerateMipMaps(TTextureHandleId textureHandle)
	{
		TDE2_UNIMPLEMENTED();
		return RC_OK;
	}

	void CD3D12GraphicsContext::Draw(E_PRIMITIVE_TOPOLOGY_TYPE topology, U32 startVertex, U32 numOfVertices)
	{
		ComPtr<ID3D12GraphicsCommandList> pCurrCommandBuffer = _getCurrCommandListPtr();

		pCurrCommandBuffer->IASetPrimitiveTopology(CD3D12Mappings::GetPrimitiveTopology(topology));
		pCurrCommandBuffer->DrawInstanced(numOfVertices, 1, startVertex, 0);
	}

	void CD3D12GraphicsContext::DrawIndexed(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, U32 baseVertex, U32 startIndex, U32 numOfIndices)
	{
		ComPtr<ID3D12GraphicsCommandList> pCurrCommandBuffer = _getCurrCommandListPtr();

		pCurrCommandBuffer->IASetPrimitiveTopology(CD3D12Mappings::GetPrimitiveTopology(topology));
		pCurrCommandBuffer->DrawIndexedInstanced(numOfIndices, 1, startIndex, baseVertex, 0);
	}

	void CD3D12GraphicsContext::DrawInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, U32 startVertex, U32 verticesPerInstance, U32 startInstance, U32 numOfInstances)
	{
		ComPtr<ID3D12GraphicsCommandList> pCurrCommandBuffer = _getCurrCommandListPtr();

		pCurrCommandBuffer->IASetPrimitiveTopology(CD3D12Mappings::GetPrimitiveTopology(topology));
		pCurrCommandBuffer->DrawInstanced(verticesPerInstance, numOfInstances, startVertex, startInstance);
	}

	void CD3D12GraphicsContext::DrawIndexedInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, U32 baseVertex, U32 startIndex,
		U32 startInstance, U32 indicesPerInstance, U32 numOfInstances)
	{
		ComPtr<ID3D12GraphicsCommandList> pCurrCommandBuffer = _getCurrCommandListPtr();

		pCurrCommandBuffer->IASetPrimitiveTopology(CD3D12Mappings::GetPrimitiveTopology(topology));
		pCurrCommandBuffer->DrawIndexedInstanced(indicesPerInstance, numOfInstances, startIndex, baseVertex, startInstance);
	}

	void CD3D12GraphicsContext::DrawIndirectInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, TBufferHandleId argsBufferHandle, U32 alignedOffset)
	{
		ComPtr<ID3D12GraphicsCommandList> pCurrCommandBuffer = _getCurrCommandListPtr();

		pCurrCommandBuffer->IASetPrimitiveTopology(CD3D12Mappings::GetPrimitiveTopology(topology));

		//auto pArgsBuffer = mpGraphicsObjectManagerD3D12Impl->GetD3D12BufferPtr(argsBufferHandle);
		//pCurrCommandBuffer->DrawInstanced(pArgsBuffer ? pArgsBuffer->Get() : nullptr, alignedOffset);
	}

	void CD3D12GraphicsContext::DrawIndirectIndexedInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, TBufferHandleId argsBufferHandle, U32 alignedOffset)
	{
		ComPtr<ID3D12GraphicsCommandList> pCurrCommandBuffer = _getCurrCommandListPtr();

		pCurrCommandBuffer->IASetPrimitiveTopology(CD3D12Mappings::GetPrimitiveTopology(topology));

		//auto pArgsBuffer = mpGraphicsObjectManagerD3D11Impl->GetD3D11BufferPtr(argsBufferHandle);
		//mp3dDeviceContext->DrawIndexedInstancedIndirect(pArgsBuffer ? pArgsBuffer->GetD3D11Buffer() : nullptr, alignedOffset);
	}

	void CD3D12GraphicsContext::DispatchCompute(U32 groupsCountX, U32 groupsCountY, U32 groupsCountZ)
	{
		TDE2_PROFILER_SCOPE("CD3D12GraphicsContext::DispatchCompute");

		ComPtr<ID3D12GraphicsCommandList> pCurrCommandBuffer = _getCurrCommandListPtr();

		pCurrCommandBuffer->Dispatch(groupsCountX, groupsCountY, groupsCountZ);
	}

	void CD3D12GraphicsContext::DispatchIndirectCompute(TBufferHandleId argsBufferHandle, U32 alignedOffset)
	{
		TDE2_PROFILER_SCOPE("CD3D12GraphicsContext::DispatchIndirectCompute");

		//auto pArgsBuffer = mpGraphicsObjectManagerD3D11Impl->GetD3D11BufferPtr(argsBufferHandle);
		//mp3dDeviceContext->DispatchIndirect(pArgsBuffer ? pArgsBuffer->GetD3D11Buffer() : nullptr, alignedOffset);
	}

	void CD3D12GraphicsContext::BindBlendState(TBlendStateId blendStateId)
	{
	}

	void CD3D12GraphicsContext::BindDepthStencilState(TDepthStencilStateId depthStencilStateId, U8 stencilRef)
	{
	}

	void CD3D12GraphicsContext::BindRasterizerState(TRasterizerStateId rasterizerStateId)
	{
	}

	E_RESULT_CODE CD3D12GraphicsContext::BeginRenderPass(const TFramebufferInfo& framebufferInfo)
	{
		mIsRenderPassActive = true;

		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsContext::EndRenderPass()
	{

		mIsRenderPassActive = false;

		return RC_OK;
	}

	const TGraphicsCtxInternalData& CD3D12GraphicsContext::GetInternalData() const
	{
		static TGraphicsCtxInternalData data;
		return data;
	}

	E_ENGINE_SUBSYSTEM_TYPE CD3D12GraphicsContext::GetType() const
	{
		return EST_GRAPHICS_CONTEXT;
	}

	IGraphicsObjectManager* CD3D12GraphicsContext::GetGraphicsObjectManager() const
	{
		return mpGraphicsObjectManagerD3D12Impl;
	}

	F32 CD3D12GraphicsContext::GetPositiveZAxisDirection() const
	{
		return 1.0f;
	}

	E_RESULT_CODE CD3D12GraphicsContext::OnEvent(const TBaseEvent* pEvent)
	{
		TDE2_PROFILER_SCOPE("CD3D12GraphicsContext::OnEvent");
		return RC_OK;
	}

#if TDE2_DEBUG_MODE

	void CD3D12GraphicsContext::BeginSectionMarker(const std::string& id)
	{
		_getCurrCommandListPtr()->BeginEvent(1, id.c_str(), static_cast<UINT>(id.length()));
	}

	void CD3D12GraphicsContext::EndSectionMarker()
	{
		_getCurrCommandListPtr()->EndEvent();
	}

#endif

	TEventListenerId CD3D12GraphicsContext::GetListenerId() const
	{
		return TEventListenerId(GetTypeId());
	}

	TVideoAdapterInfo CD3D12GraphicsContext::GetInfo() const
	{
		return {};
	}

	const TGraphicsContextInfo& CD3D12GraphicsContext::GetContextInfo() const
	{
		const static TGraphicsContextInfo infoData
		{
			{ { -1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f } },
			false,
			E_GRAPHICS_CONTEXT_GAPI_TYPE::GCGT_DIRECT3D12
		};

		return infoData;
	}

	TPtr<IWindowSystem> CD3D12GraphicsContext::GetWindowSystem() const
	{
		return mpWindowSystem;
	}

	std::vector<U8> CD3D12GraphicsContext::GetBackBufferData() const
	{
		std::vector<U8> backBufferData;

		return std::move(backBufferData);
	}

	ID3D12Device* CD3D12GraphicsContext::GetDeviceContext() const
	{
		return mpDeviceContext->GetDevice().Get();
	}

	D3D12MA::Allocator* CD3D12GraphicsContext::GetMemoryAllocator() const
	{
		return mpDeviceContext->GetMemoryAllocator().Get();
	}


	IGraphicsContext* CreateD3D12GraphicsContext(TPtr<IWindowSystem> pWindowSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IGraphicsContext, CD3D12GraphicsContext, result, pWindowSystem);
	}
}

#endif