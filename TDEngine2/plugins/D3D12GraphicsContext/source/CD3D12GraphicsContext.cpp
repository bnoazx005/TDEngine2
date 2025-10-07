#include "../include/CD3D12GraphicsContext.h"
#include "../include/CD3D12Mappings.h"
#include "../include/CD3D12GraphicsObjectManager.h"
#include "../include/CD3D12Resources.h"
#include <core/IGraphicsContext.h>
#include <core/IEventManager.h>
#include <core/CBaseObject.h>
#include <core/IWindowSystem.h>
#include <editor/CPerfProfiler.h>
#include <utils/CContainers.h>
#include <tuple>
#include "deferOperation.hpp"


#if defined(TDE2_USE_WINPLATFORM) /// Used only on Windows platform

#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include "../deps/D3D12MemAlloc/D3D12MemAlloc.h"
#include "../deps/dx12/d3dx12_barriers.h"
#include "../deps/dx12/d3dx12_root_signature.h"
#include <WinPixEventRuntime/pix3.h>


template <typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#if TDE2_DEBUG_MODE
	#pragma comment(lib, "dxguid.lib") 
#endif

#pragma comment(lib, "WinPixEventRuntime")


namespace TDEngine2
{
	static std::array<U32, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> DESCRIPTORS_PER_BLOCK
	{
		512,
		128,
		64,
		32,
	};


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


	static TResult<std::tuple<ComPtr<ID3D12Device5>, ComPtr<IDXGIAdapter1>>> InitPhysicalDevice(ComPtr<IDXGIFactory4> pFactory, D3D_FEATURE_LEVEL& selectedFeatureLevel)
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
		ComPtr<ID3D12Device5> p3dDevice = nullptr;

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
					return Wrench::TOkValue<std::tuple<ComPtr<ID3D12Device5>, ComPtr<IDXGIAdapter1>>>({ p3dDevice, pAdapter });
				}
			}
		}

		return Wrench::TOkValue<std::tuple<ComPtr<ID3D12Device5>, ComPtr<IDXGIAdapter1>>>({ nullptr, nullptr });
	}


	static TResult<ComPtr<ID3D12CommandQueue>> CreateCommandQueue(ComPtr<ID3D12Device5> p3dDevice)
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


	static TResult<ComPtr<ID3D12CommandAllocator>> CreateCommandAllocator(ComPtr<ID3D12Device5> p3dDevice)
	{
		ComPtr<ID3D12CommandAllocator> pCommandAllocator = nullptr;

		if (FAILED(p3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pCommandAllocator))))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<ComPtr<ID3D12CommandAllocator>>(pCommandAllocator);
	}


	static TResult<ComPtr<ID3D12GraphicsCommandList>> CreateCommandList(ComPtr<ID3D12Device5> p3dDevice, ComPtr<ID3D12CommandAllocator> pCommandAllocator)
	{
		ComPtr<ID3D12GraphicsCommandList> pCommandList = nullptr;

		if (FAILED(p3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, pCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&pCommandList))))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		pCommandList->Close();

		return Wrench::TOkValue<ComPtr<ID3D12GraphicsCommandList>>(pCommandList);
	}


	static TResult<ComPtr<D3D12MA::Allocator>> CreateMemoryAllocator(ComPtr<ID3D12Device5> p3dDevice, ComPtr<IDXGIAdapter1> pAdapter)
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
	ID3D12CPUDescriptorsAllocator* CreateD3D12CPUDescriptorsAllocator(CD3D12DeviceContext*, D3D12_DESCRIPTOR_HEAP_TYPE, U32, E_RESULT_CODE&);
	CD3D12GPUDescriptorsHeap* CreateD3D12GPUDescriptorsHeap(CD3D12DeviceContext*, D3D12_DESCRIPTOR_HEAP_TYPE, U32, E_RESULT_CODE&);


	class CD3D12Fence
	{
		public:
			CD3D12Fence() = default;
			CD3D12Fence(CD3D12DeviceContext* pDeviceContext, U64 initialValue = 0);
			CD3D12Fence(CD3D12Fence&& other) noexcept;
			~CD3D12Fence();

			U64 Signal();
			void WaitForSignal(U64 signalValue);
			void WaitForIdle();

			void Reset(U64 value = 0) 
			{ 
				mpFence->Signal(value); 
				mFenceValue = value;
			}

			U64 GetNextValue() const { return mFenceValue; }
			U64 GetLastValue() const { return mpFence->GetCompletedValue(); }

			CD3D12Fence& operator= (CD3D12Fence&& other) noexcept;
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
			typedef std::array<TPtr<ID3D12CPUDescriptorsAllocator>, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> TDescriptorHeapsTable;
		public:
			E_RESULT_CODE Init(TPtr<IWindowSystem> pWindowSystem);

			TPtr<CD3D12Swapchain> CreateSwapchain();

			CD3D12Fence CreateFence(bool signaled = true);

			void WaitForIdle();

			ComPtr<ID3D12Device5> GetDevice() const { return mp3dDevice; }

			ComPtr<ID3D12CommandQueue> GetCommandQueue() { return mpCommandQueue; }

			ComPtr<IDXGIFactory4> GetFactory() { return mpObjectsFactory; }

			ComPtr<D3D12MA::Allocator> GetMemoryAllocator() const { return mpMemoryAllocator; }

			TPtr<ID3D12CPUDescriptorsAllocator> GetDescriptorsAllocator(D3D12_DESCRIPTOR_HEAP_TYPE heapType) const;

			ComPtr<ID3D12CommandSignature> GetDispatchIndirectCmdSignature() { return mpDispatchIndirectCmdSignature; }
			ComPtr<ID3D12CommandSignature> GetDrawInstancedIndirectCmdSignature() { return mpDrawInstancedIndirectCmdSignature; }
			ComPtr<ID3D12CommandSignature> GetDrawIndexedInstancedIndirectCmdSignature() { return mpDrawIndexedInstancedIndirectCmdSignature; }
		private:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D12DeviceContext)

			E_RESULT_CODE _onFreeInternal() override;
		private:
			TPtr<IWindowSystem>            mpWindowSystem = nullptr;

			D3D_FEATURE_LEVEL              mCurrFeatureLevel = D3D_FEATURE_LEVEL_12_2;
			ComPtr<ID3D12Device5>          mp3dDevice = nullptr;
			ComPtr<IDXGIAdapter1>          mpAdapter = nullptr;
									    
#if TDE2_DEBUG_MODE					    
			ComPtr<ID3D12Debug1>           mpDebugController = nullptr;
			ComPtr<ID3D12DebugDevice>      mpDebugDevice = nullptr;
#endif								    
			    
			ComPtr<D3D12MA::Allocator>     mpMemoryAllocator = nullptr;
			ComPtr<IDXGIFactory4>          mpObjectsFactory = nullptr;

			// queues
			ComPtr<ID3D12CommandQueue>     mpCommandQueue = nullptr;
			CD3D12Fence                    mCommandQueueFence{};

			TDescriptorHeapsTable          mpDescriptorHeapsTable{};

			ComPtr<ID3D12CommandSignature> mpDispatchIndirectCmdSignature = nullptr;
			ComPtr<ID3D12CommandSignature> mpDrawInstancedIndirectCmdSignature = nullptr;
			ComPtr<ID3D12CommandSignature> mpDrawIndexedInstancedIndirectCmdSignature = nullptr;
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

		if (initialValue)
		{
			mpFence->Signal(initialValue);
		}
		
		mEventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);

		TDE2_ASSERT(INVALID_HANDLE_VALUE != mEventHandle);
		TDE2_ASSERT(mpFence->GetCompletedValue() == initialValue);
	}

	CD3D12Fence::CD3D12Fence(CD3D12Fence&& other) noexcept:
		mpDeviceContext(other.mpDeviceContext), mFenceValue(other.mFenceValue), mpFence(other.mpFence), mEventHandle(other.mEventHandle)
	{
	}

	CD3D12Fence::~CD3D12Fence()
	{
		if (mEventHandle)
		{
			CloseHandle(mEventHandle);
		}
	}

	U64 CD3D12Fence::Signal()
	{
		++mFenceValue;

		HRESULT result = mpDeviceContext->GetCommandQueue()->Signal(mpFence.Get(), mFenceValue);
		TDE2_ASSERT(SUCCEEDED(result));

		return mFenceValue;
	}

	void CD3D12Fence::WaitForSignal(U64 signalValue = 1)
	{
		HRESULT result = S_OK;

		if (mpFence->GetCompletedValue() < signalValue)
		{
			result = mpFence->SetEventOnCompletion(signalValue, mEventHandle);
			TDE2_ASSERT(SUCCEEDED(result));

			WaitForSingleObjectEx(mEventHandle, INFINITE, FALSE);
		}
	}

	void CD3D12Fence::WaitForIdle()
	{
		WaitForSignal(Signal());
	}

	CD3D12Fence& CD3D12Fence::operator= (CD3D12Fence&& other) noexcept
	{
		mpDeviceContext = other.mpDeviceContext;
		mFenceValue     = other.mFenceValue;
		mpFence         = other.mpFence;
		mEventHandle    = other.mEventHandle;

		other.mpDeviceContext = nullptr;
		other.mFenceValue     = 0;
		other.mpFence         = nullptr;
		other.mEventHandle    = nullptr;

		return *this;
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
		mCommandQueueFence = CD3D12Fence(this, 0);

		E_RESULT_CODE result = RC_OK;

		for (I32 heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; heapType < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++heapType)
		{
			mpDescriptorHeapsTable[heapType] = TPtr<ID3D12CPUDescriptorsAllocator>(CreateD3D12CPUDescriptorsAllocator(this, static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(heapType), DESCRIPTORS_PER_BLOCK[heapType], result));
		}

		const D3D12_INDIRECT_ARGUMENT_DESC dispatchArgs { D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH };
		const D3D12_INDIRECT_ARGUMENT_DESC drawInstancedArgs { D3D12_INDIRECT_ARGUMENT_TYPE_DRAW };
		const D3D12_INDIRECT_ARGUMENT_DESC drawIndexedInstancedArgs { D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED };

		std::array<std::tuple<D3D12_COMMAND_SIGNATURE_DESC, ID3D12CommandSignature**>, 3> commandSignatures
		{
			std::make_tuple(D3D12_COMMAND_SIGNATURE_DESC { sizeof(D3D12_DISPATCH_ARGUMENTS), 1, &dispatchArgs }, &mpDispatchIndirectCmdSignature),
			std::make_tuple(D3D12_COMMAND_SIGNATURE_DESC { sizeof(D3D12_DRAW_ARGUMENTS), 1, &drawInstancedArgs }, &mpDrawInstancedIndirectCmdSignature),
			std::make_tuple(D3D12_COMMAND_SIGNATURE_DESC { sizeof(D3D12_DRAW_INDEXED_ARGUMENTS), 1, &drawIndexedInstancedArgs }, &mpDrawIndexedInstancedIndirectCmdSignature),
		};

		for (auto& [currSignatureDesc, pCurrSignatureOutput] : commandSignatures)
		{
			if (FAILED(mp3dDevice->CreateCommandSignature(&currSignatureDesc, nullptr, IID_PPV_ARGS(pCurrSignatureOutput))))
			{
				result = result | RC_FAIL;
			}
		}

		if (RC_OK != result)
		{
			return result;
		}

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
		return CD3D12Fence(this, signaled ? 1 : 0);
	}

	E_RESULT_CODE CD3D12DeviceContext::_onFreeInternal()
	{
		mpMemoryAllocator = nullptr;

		mpCommandQueue = nullptr;
		mCommandQueueFence = {};

		mpObjectsFactory = nullptr;
		mpAdapter        = nullptr;

		std::fill(mpDescriptorHeapsTable.begin(), mpDescriptorHeapsTable.end(), nullptr);

#if TDE2_DEBUG_MODE
		if (mpDebugDevice)
		{
			mpDebugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
		}
#endif

		return RC_OK;
	}

	void CD3D12DeviceContext::WaitForIdle()
	{
		mCommandQueueFence.WaitForIdle();
	}

	TPtr<ID3D12CPUDescriptorsAllocator> CD3D12DeviceContext::GetDescriptorsAllocator(D3D12_DESCRIPTOR_HEAP_TYPE heapType) const
	{
		if (heapType >= D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES)
		{
			TDE2_ASSERT(false);
			return nullptr;
		}

		return mpDescriptorHeapsTable[static_cast<USIZE>(heapType)];
	}


	CD3D12DeviceContext* CreateD3D12DeviceContext(TPtr<IWindowSystem> pWindowSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CD3D12DeviceContext, CD3D12DeviceContext, result, pWindowSystem);
	}


	TDE2_DEFINE_SCOPED_PTR(CD3D12DeviceContext);


	/*!
		\brief CD3D12CPUDescriptorsAllocator's definition
	*/

	class CD3D12CPUDescriptorsAllocator : public CBaseObject, public virtual ID3D12CPUDescriptorsAllocator
	{
		public:
			friend ID3D12CPUDescriptorsAllocator* CreateD3D12CPUDescriptorsAllocator(CD3D12DeviceContext*, D3D12_DESCRIPTOR_HEAP_TYPE, U32, E_RESULT_CODE&);
		public:
			E_RESULT_CODE Init(CD3D12DeviceContext* pDeviceContext, D3D12_DESCRIPTOR_HEAP_TYPE type, U32 descriptorsPerBlock = 256) override;

			TD3D12ResourceDescriptor AllocDescriptor() override;
			E_RESULT_CODE FreeDescriptor(TD3D12ResourceDescriptor& descriptor) override;

			D3D12_DESCRIPTOR_HEAP_TYPE GetType() const override { return mType; }
			U32 GetDescriptorSize() const override { return mDescriptorSize; }
		private:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D12CPUDescriptorsAllocator)

			E_RESULT_CODE _allocateNewBlock();
		private:
			CD3D12DeviceContext*                 mpDeviceContext = nullptr;

			Vector<ComPtr<ID3D12DescriptorHeap>> mpHeapBlocks{};
			std::list<TD3D12ResourceDescriptor>  mFreeHandles{};

			D3D12_DESCRIPTOR_HEAP_TYPE           mType = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
			U32                                  mMaxDescriptorsPerBlock = 0;
			U32                                  mDescriptorSize = 0;
			U32                                  mActiveDescriptorsCount = 0;

			mutable std::mutex                   mMutex;
	};


	CD3D12CPUDescriptorsAllocator::CD3D12CPUDescriptorsAllocator():
		CBaseObject()
	{
	}

	E_RESULT_CODE CD3D12CPUDescriptorsAllocator::Init(CD3D12DeviceContext* pDeviceContext, D3D12_DESCRIPTOR_HEAP_TYPE type, U32 descriptorsPerBlock)
	{
		if (!descriptorsPerBlock)
		{
			return RC_INVALID_ARGS;
		}

		mpDeviceContext = pDeviceContext;
		mType = type;
		mMaxDescriptorsPerBlock = descriptorsPerBlock;

		mDescriptorSize = mpDeviceContext->GetDevice()->GetDescriptorHandleIncrementSize(type);

		E_RESULT_CODE result = _allocateNewBlock();
		if (RC_OK != result)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	TD3D12ResourceDescriptor CD3D12CPUDescriptorsAllocator::AllocDescriptor()
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (mFreeHandles.empty())
		{
			E_RESULT_CODE result = _allocateNewBlock();
			TDE2_ASSERT(RC_OK == result);
		}

		TD3D12ResourceDescriptor currDescriptor = mFreeHandles.front();
		mFreeHandles.pop_front();

		++mActiveDescriptorsCount;

		return currDescriptor;
	}
	
	E_RESULT_CODE CD3D12CPUDescriptorsAllocator::FreeDescriptor(TD3D12ResourceDescriptor& descriptor)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		mFreeHandles.emplace_back(descriptor);
		--mActiveDescriptorsCount;

		return RC_OK;
	}

	E_RESULT_CODE CD3D12CPUDescriptorsAllocator::_allocateNewBlock()
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
		heapDesc.NumDescriptors = mMaxDescriptorsPerBlock;
		heapDesc.Type = mType;

		ComPtr<ID3D12DescriptorHeap>& pNewDescriptorHeap = mpHeapBlocks.emplace_back();

		if (FAILED(mpDeviceContext->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(pNewDescriptorHeap.GetAddressOf()))))
		{
			return RC_FAIL;
		}

		D3D12_CPU_DESCRIPTOR_HANDLE firstCPUHandle = pNewDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

		for (U32 i = 0; i < mMaxDescriptorsPerBlock; ++i)
		{
			TD3D12ResourceDescriptor& currDescriptor = mFreeHandles.emplace_back();

			currDescriptor.mCPUHandle = { static_cast<USIZE>(firstCPUHandle.ptr + static_cast<U64>(i) * mDescriptorSize) };
			currDescriptor.mIndex     = i;
		}

		return RC_OK;
	}


	ID3D12CPUDescriptorsAllocator* CreateD3D12CPUDescriptorsAllocator(CD3D12DeviceContext* pDeviceContext, D3D12_DESCRIPTOR_HEAP_TYPE type, U32 descriptorsPerBlock, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ID3D12CPUDescriptorsAllocator, CD3D12CPUDescriptorsAllocator, result, pDeviceContext, type, descriptorsPerBlock);
	}


	TDE2_DEFINE_SCOPED_PTR(CD3D12CPUDescriptorsAllocator);


	/*!
		\brief CD3D12GPUDescriptorsHeap's definition
	*/

	class CD3D12GPUDescriptorsHeap : public CBaseObject
	{
	public:
		friend CD3D12GPUDescriptorsHeap* CreateD3D12GPUDescriptorsHeap(CD3D12DeviceContext*, D3D12_DESCRIPTOR_HEAP_TYPE, U32, E_RESULT_CODE&);
	public:
		E_RESULT_CODE Init(CD3D12DeviceContext* pDeviceContext, D3D12_DESCRIPTOR_HEAP_TYPE type, U32 descriptorsPerBlock = 256);

		TD3D12ResourceDescriptor AllocDescriptorsBlock(U32 count);
		TD3D12ResourceDescriptor GetReservedDescriptorByIndex(U32 index);
		E_RESULT_CODE Reset();

		ID3D12DescriptorHeap* GetNativeHandle() const { return mpHeap.Get(); }
		D3D12_DESCRIPTOR_HEAP_TYPE GetType() const { return mType; }
		U32 GetDescriptorSize() const { return mDescriptorSize; }
	private:
		DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D12GPUDescriptorsHeap)
	private:
		CD3D12DeviceContext*                 mpDeviceContext = nullptr;

		ComPtr<ID3D12DescriptorHeap>         mpHeap = nullptr;

		D3D12_DESCRIPTOR_HEAP_TYPE           mType = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
		U32                                  mMaxDescriptorsPerBlock = 0;
		U32                                  mDescriptorSize = 0;
		U32                                  mActiveDescriptorsCount = 0;

		mutable std::mutex                   mMutex;
	};


	CD3D12GPUDescriptorsHeap::CD3D12GPUDescriptorsHeap() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CD3D12GPUDescriptorsHeap::Init(CD3D12DeviceContext* pDeviceContext, D3D12_DESCRIPTOR_HEAP_TYPE type, U32 descriptorsPerBlock)
	{
		if (!descriptorsPerBlock)
		{
			return RC_INVALID_ARGS;
		}

		mpDeviceContext = pDeviceContext;
		mType = type;
		mMaxDescriptorsPerBlock = descriptorsPerBlock;

		mDescriptorSize = mpDeviceContext->GetDevice()->GetDescriptorHandleIncrementSize(type);

		D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
		heapDesc.NumDescriptors = mMaxDescriptorsPerBlock;
		heapDesc.Type           = mType;
		heapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		if (FAILED(mpDeviceContext->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mpHeap.GetAddressOf()))))
		{
			return RC_FAIL;
		}

#if TDE2_DEBUG_MODE
		const std::string& heapDebugName = Wrench::StringUtils::Format("{0} GPU descriptors heap", type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER ? "samplers" : "SRVs");
		mpHeap->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<U32>(heapDebugName.length()), heapDebugName.data());
#endif

		mIsInitialized = true;

		return RC_OK;
	}

	TD3D12ResourceDescriptor CD3D12GPUDescriptorsHeap::AllocDescriptorsBlock(U32 count)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		const U32 blockEnd = mActiveDescriptorsCount + count;
		if (blockEnd >= mMaxDescriptorsPerBlock)
		{
			TDE2_ASSERT_MSG(false, "[CD3D12GPUDescriptorsHeap] GPU descriptors heap runs out of memory");
			return {};
		}

		TD3D12ResourceDescriptor currDescriptor{};
		currDescriptor.mCPUHandle = mpHeap->GetCPUDescriptorHandleForHeapStart();
		currDescriptor.mGPUHandle = mpHeap->GetGPUDescriptorHandleForHeapStart();
		currDescriptor.mIndex     = mActiveDescriptorsCount;

		currDescriptor.mCPUHandle.ptr += static_cast<USIZE>(mActiveDescriptorsCount) * mDescriptorSize;
		currDescriptor.mGPUHandle.ptr += static_cast<USIZE>(mActiveDescriptorsCount) * mDescriptorSize;

		mActiveDescriptorsCount += count;

		return currDescriptor;
	}

	TD3D12ResourceDescriptor CD3D12GPUDescriptorsHeap::GetReservedDescriptorByIndex(U32 index)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		TDE2_ASSERT_MSG(index < mMaxDescriptorsPerBlock, "[CD3D12GPUDescriptorsHeap] Index goes out of boundaries");

		TD3D12ResourceDescriptor currDescriptor{};
		currDescriptor.mCPUHandle = mpHeap->GetCPUDescriptorHandleForHeapStart();
		currDescriptor.mGPUHandle = mpHeap->GetGPUDescriptorHandleForHeapStart();
		currDescriptor.mIndex     = index;

		currDescriptor.mCPUHandle.ptr += static_cast<USIZE>(index) * mDescriptorSize;
		currDescriptor.mGPUHandle.ptr += static_cast<USIZE>(index) * mDescriptorSize;

		return currDescriptor;
	}

	E_RESULT_CODE CD3D12GPUDescriptorsHeap::Reset()
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mActiveDescriptorsCount = 0;

		return RC_OK;
	}


	CD3D12GPUDescriptorsHeap* CreateD3D12GPUDescriptorsHeap(CD3D12DeviceContext* pDeviceContext, D3D12_DESCRIPTOR_HEAP_TYPE type, U32 descriptorsPerBlock, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CD3D12GPUDescriptorsHeap, CD3D12GPUDescriptorsHeap, result, pDeviceContext, type, descriptorsPerBlock);
	}


	TDE2_DEFINE_SCOPED_PTR(CD3D12GPUDescriptorsHeap);


	/*!
		\brief CD3D12Swapchain's definition
	*/

	class CD3D12Swapchain : public CBaseObject
	{
		public:
			friend CD3D12Swapchain* CreateD3D12Swapchain(CD3D12DeviceContext*, TPtr<IWindowSystem>, E_RESULT_CODE&);
		private:
			typedef std::array<ComPtr<ID3D12Resource>, CD3D12GraphicsContext::BACK_BUFFERS_COUNT> TRenderTargetsArray;
			typedef std::array<E_RESOURCE_LAYOUT, CD3D12GraphicsContext::BACK_BUFFERS_COUNT>      TRenderTargetsLayoutsArray;
		public:
			E_RESULT_CODE Init(CD3D12DeviceContext* pDeviceContext, TPtr<IWindowSystem> pWindowSystem);

			E_RESULT_CODE Present();

			void InvalidateState() { mIsValid = false; }
			E_RESULT_CODE TryProcessInvalidateState();

			U32 GetBackBufferTargetIndex() const { return mpSwapChain->GetCurrentBackBufferIndex(); }
			ComPtr<ID3D12Resource> GetCurrRenderTarget() const { return mpRenderTargets[GetBackBufferTargetIndex()]; }
			D3D12_CPU_DESCRIPTOR_HANDLE GetCurrRenderTargetView() const { return CD3DX12_CPU_DESCRIPTOR_HANDLE(mpRenderTargetViewsHeap->GetCPUDescriptorHandleForHeapStart(), GetBackBufferTargetIndex(), mRTVDescriptorSize); }
			
			void SetCurrRenderTargetLayout(E_RESOURCE_LAYOUT layout) { mRenderTargetsLayouts[GetBackBufferTargetIndex()] = layout; }
			E_RESOURCE_LAYOUT GetCurrRenderTargetLayout() const { return mRenderTargetsLayouts[GetBackBufferTargetIndex()]; }

			ComPtr<ID3D12Resource> GetDefaultDepthStencilTarget() const { return mpDefaulDepthStencilTarget; }
			D3D12_CPU_DESCRIPTOR_HANDLE GetDefaultDepthStencilTargetView() const { return mDefaultDepthStencilDescriptor.mCPUHandle; }

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
			TRenderTargetsLayoutsArray   mRenderTargetsLayouts;
			ComPtr<ID3D12Resource>       mpDefaulDepthStencilTarget = nullptr;
			D3D12MA::Allocation*         mpDefaultDepthStencilAllocation = nullptr;

			E_FORMAT_TYPE                mBackBufferFormat = E_FORMAT_TYPE::FT_UNKNOWN;

			ComPtr<ID3D12DescriptorHeap> mpRenderTargetViewsHeap = nullptr;
			U32                          mRTVDescriptorSize = 0;

			TD3D12ResourceDescriptor     mDefaultDepthStencilDescriptor{};

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
		return SUCCEEDED(mpSwapChain->Present(static_cast<UINT>(mIsVSyncEnabled), 0)) ? RC_OK : RC_FAIL;
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

		mBackBufferFormat = (mpWindowSystem->GetFlags() & P_HARDWARE_GAMMA_CORRECTION) ? E_FORMAT_TYPE::FT_NORM_BYTE4_SRGB : E_FORMAT_TYPE::FT_NORM_BYTE4;

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

		ComPtr<ID3D12Device5> p3dDevice = mpDeviceContext->GetDevice();

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

#if TDE2_DEBUG_MODE
			std::string currBackBufferResourceId = "SwapchainImage_" + std::to_string(i);

			mpRenderTargets[i]->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<U32>(currBackBufferResourceId.length()), currBackBufferResourceId.data());
#endif

			mRenderTargetsLayouts[i] = E_RESOURCE_LAYOUT::UNDEFINED;

			p3dDevice->CreateRenderTargetView(mpRenderTargets[i].Get(), nullptr, rtvHandle);
			rtvHandle.ptr += mRTVDescriptorSize;
		}

		D3D12_RESOURCE_DESC textureDesc{};
		textureDesc.Dimension        = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		textureDesc.Width            = mpWindowSystem->GetWidth();
		textureDesc.Height           = mpWindowSystem->GetHeight();
		textureDesc.DepthOrArraySize = 1;
		textureDesc.MipLevels        = 1;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Format           = DXGI_FORMAT_D24_UNORM_S8_UINT;
		textureDesc.Flags            = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12MA::ALLOCATION_DESC allocationDesc{};
		allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

		if (FAILED(mpDeviceContext->GetMemoryAllocator()->CreateResource(&allocationDesc, &textureDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, &mpDefaultDepthStencilAllocation, IID_PPV_ARGS(&mpDefaulDepthStencilTarget))))
		{
			return RC_FAIL;
		}

		mDefaultDepthStencilDescriptor = mpDeviceContext->GetDescriptorsAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_DSV)->AllocDescriptor();

		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		mpDeviceContext->GetDevice()->CreateDepthStencilView(mpDefaulDepthStencilTarget.Get(), &depthStencilViewDesc, mDefaultDepthStencilDescriptor.mCPUHandle);

		mIsVSyncEnabled = mpWindowSystem->GetFlags() & P_VSYNC;

		mIsInitialized = true;
		mIsValid = true;

		return RC_OK;
	}

	E_RESULT_CODE CD3D12Swapchain::_onFreeInternal()
	{
		if (mpDefaultDepthStencilAllocation)
		{
			mpDefaultDepthStencilAllocation->Release();
			mpDefaultDepthStencilAllocation = nullptr;
		}

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

			ComPtr<ID3D12GraphicsCommandList4> GetHandle() const 
			{
#if TDE2_DEBUG_MODE
				TDE2_ASSERT(mIsRecordStateActive);
#endif
				return mpCommandList; 
			}
			CD3D12Fence& GetFenceEntry() { return mFenceEntry; }
		private:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D12CommandBuffer)

			E_RESULT_CODE _onFreeInternal() override;
		private:
			CD3D12DeviceContext*               mpDeviceContext = nullptr;

			ComPtr<ID3D12CommandAllocator>     mpCommandListAllocator = nullptr;
			ComPtr<ID3D12GraphicsCommandList4> mpCommandList = nullptr;

			CD3D12Fence                        mFenceEntry{};

#if TDE2_DEBUG_MODE
			bool                               mIsRecordStateActive = false;
#endif
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

		ComPtr<ID3D12Device5> p3dDevice = pDeviceContext->GetDevice();

		if (FAILED(p3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mpCommandListAllocator))))
		{
			return RC_FAIL;
		}

		if (FAILED(p3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mpCommandListAllocator.Get(), nullptr, IID_PPV_ARGS(&mpCommandList))))
		{
			return RC_FAIL;
		}

		if (FAILED(mpCommandList->Close()))
		{
			return RC_FAIL;
		}

		mFenceEntry = mpDeviceContext->CreateFence();
		
		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CD3D12CommandBuffer::Begin()
	{
#if TDE2_DEBUG_MODE
		mIsRecordStateActive = true;
#endif
		return Reset();
	}

	E_RESULT_CODE CD3D12CommandBuffer::End()
	{
#if TDE2_DEBUG_MODE
		mIsRecordStateActive = false;
#endif
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

		mResourceBarriers[mpSwapchain->GetBackBufferTargetIndex()].emplace_back(CD3DX12_RESOURCE_BARRIER::Transition(mpSwapchain->GetDefaultDepthStencilTarget().Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_DEPTH_WRITE));

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

			mpShaderResourcesDescriptorsHeaps[i] = TPtr<CD3D12GPUDescriptorsHeap>(CreateD3D12GPUDescriptorsHeap(mpDeviceContext.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1, localResult));
			result = result | localResult;

			mpSamplersDescriptorsHeaps[i] = TPtr<CD3D12GPUDescriptorsHeap>(CreateD3D12GPUDescriptorsHeap(mpDeviceContext.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE, localResult));
			result = result | localResult;
		}

		if (RC_OK != result)
		{
			return result;
		}

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

		mpDeviceContext->WaitForIdle();

		std::fill(mpCommandBuffers.begin(), mpCommandBuffers.end(), nullptr);
		std::fill(mpShaderResourcesDescriptorsHeaps.begin(), mpShaderResourcesDescriptorsHeaps.end(), nullptr);
		std::fill(mpSamplersDescriptorsHeaps.begin(), mpSamplersDescriptorsHeaps.end(), nullptr);

		mpGraphicsObjectManager          = nullptr;
		mpGraphicsObjectManagerD3D12Impl = nullptr;

		// \note Clean up is invoked only after all resources are freed in mpGraphicsObjectManager
		for (TGarbageCollection& currGarbageCollection : mAwaitingDeletionObjects)
		{
			std::lock_guard<std::mutex> lock(mGarbageCollectorMutex);

			for (auto& currGarbageEntity : currGarbageCollection)
			{
				if (!currGarbageEntity.mpAllocation)
				{
					continue;
				}

				currGarbageEntity.mpAllocation->Release();
			}

			currGarbageCollection.clear();
		}

		mpImmediateCommandBuffer = nullptr;
		mpSwapchain              = nullptr;
		mpDeviceContext          = nullptr;

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


	static bool IsBindingTypeAllowed(const TD3D12PipelineLayoutInfo::TBindingInfo& bindingInfo, const TDescriptorsBindingsTable::TDescriptorHandle& descriptorHandle)
	{
		using E_BINDING_TYPE    = TD3D12PipelineLayoutInfo::TBindingInfo::E_TYPE;
		using E_DESCRIPTOR_TYPE = TDescriptorsBindingsTable::E_DESCRIPTOR_TYPE;

		const TDescriptorsBindingsTable::E_DESCRIPTOR_TYPE descriptorHandleType = descriptorHandle.mType;

		if ((bindingInfo.mType == E_BINDING_TYPE::BUFFER && descriptorHandleType == E_DESCRIPTOR_TYPE::BUFFER) ||
			(bindingInfo.mType == E_BINDING_TYPE::RAW_BUFFER && descriptorHandleType == E_DESCRIPTOR_TYPE::RAW_BUFFER) ||
			(bindingInfo.mType == E_BINDING_TYPE::TEXTURE && descriptorHandleType == E_DESCRIPTOR_TYPE::TEXTURE))
		{
			return true;
		}

		return false;
	}


	void CD3D12GraphicsContext::_preparePipelineState()
	{
		TDE2_PROFILER_SCOPE("CD3D12GraphicsContext::_preparePipelineState");

		const U32 currFrameIndex = mpSwapchain->GetBackBufferTargetIndex();
		if (!mpActivePipelineStates[currFrameIndex])
		{
			TDE2_ASSERT(false);
			return;
		}

		const auto pipelineType = mpActivePipelineStates[currFrameIndex]->GetType();
		if (E_PIPELINE_TYPE::GRAPHICS == pipelineType)
		{
			TDE2_ASSERT(mIsRenderPassActive);
		}
		
		const auto& currPipelineActiveSlots = mpActivePipelineStates[currFrameIndex]->GetLayoutInfo();

		ID3D12GraphicsCommandList4* pCurrCommandBuffer = _getCurrCommandListPtr();

		std::array<ID3D12DescriptorHeap*, 2> currDescriptorHeaps
		{
			mpShaderResourcesDescriptorsHeaps[currFrameIndex]->GetNativeHandle(),
			mpSamplersDescriptorsHeaps[currFrameIndex]->GetNativeHandle()
		};

		pCurrCommandBuffer->SetDescriptorHeaps(2, currDescriptorHeaps.data());

		for (U32 i = 0; i < currPipelineActiveSlots.mCBVActiveSlots.size(); ++i)
		{
			if (TPtr<CD3D12Buffer> pConstantBuffer = mpGraphicsObjectManagerD3D12Impl->GetD3D12BufferPtr(mDescriptorsBindingsTable.mConstantBuffers[currPipelineActiveSlots.mCBVActiveSlots[i]]))
			{
				switch (pipelineType)
				{
					case E_PIPELINE_TYPE::GRAPHICS:
						pCurrCommandBuffer->SetGraphicsRootConstantBufferView(i, pConstantBuffer->GetGPUAddress());
						break;
					case E_PIPELINE_TYPE::COMPUTE:
						pCurrCommandBuffer->SetComputeRootConstantBufferView(i, pConstantBuffer->GetGPUAddress());
						break;
				}
			}
		}

		const TD3D12ResourceDescriptor& srvResourcesBlock = mpShaderResourcesDescriptorsHeaps[currFrameIndex]->AllocDescriptorsBlock(static_cast<U32>(currPipelineActiveSlots.mSRVActiveSlots.size() + currPipelineActiveSlots.mUAVActiveSlots.size()));
		
		// \note Allows to iterate through SRVs and UAVs like they all are placed in single contiguous array [SRVs, UAVs]
		auto getResourceEntryByIndex = [this, &currPipelineActiveSlots](U32 index) -> const TDescriptorsBindingsTable::TDescriptorHandle&
			{
				static TDescriptorsBindingsTable::TDescriptorHandle invalidHandle{};

				if (index < currPipelineActiveSlots.mSRVActiveSlots.size())
				{
					return mDescriptorsBindingsTable.mSRVBuffers[currPipelineActiveSlots.mSRVActiveSlots[index].mSlot];
				}

				if (const USIZE localIndex = static_cast<USIZE>(index - currPipelineActiveSlots.mSRVActiveSlots.size()); localIndex < currPipelineActiveSlots.mUAVActiveSlots.size())
				{
					return mDescriptorsBindingsTable.mUAVBuffers[currPipelineActiveSlots.mUAVActiveSlots[localIndex].mSlot];
				}

				TDE2_UNREACHABLE();
				return invalidHandle;
			};

		auto getBindingInfoByIndex = [this, &currPipelineActiveSlots](U32 index) -> const TD3D12PipelineLayoutInfo::TBindingInfo&
			{
				static TD3D12PipelineLayoutInfo::TBindingInfo invalidHandle{};

				if (index < currPipelineActiveSlots.mSRVActiveSlots.size())
				{
					return currPipelineActiveSlots.mSRVActiveSlots[index];
				}

				if (const USIZE localIndex = static_cast<USIZE>(index - currPipelineActiveSlots.mSRVActiveSlots.size()); localIndex < currPipelineActiveSlots.mUAVActiveSlots.size())
				{
					return currPipelineActiveSlots.mUAVActiveSlots[localIndex];
				}

				TDE2_UNREACHABLE();
				return invalidHandle;
			};

		for (U32 i = 0; i < currPipelineActiveSlots.mSRVActiveSlots.size() + currPipelineActiveSlots.mUAVActiveSlots.size(); ++i)
		{
			const auto& currResourceEntity = getResourceEntryByIndex(i);
			const bool isNullDescriptor = TDescriptorsBindingsTable::E_DESCRIPTOR_TYPE::UNKNOWN == currResourceEntity.mType;

			if (isNullDescriptor || !IsBindingTypeAllowed(getBindingInfoByIndex(i), currResourceEntity)) // \note Process null descriptors
			{
				// \todo Implement assignment of null descriptors
				continue;
			}

			D3D12_CPU_DESCRIPTOR_HANDLE srcSrvCPUHandle{};

			switch (currResourceEntity.mType)
			{
				case TDescriptorsBindingsTable::E_DESCRIPTOR_TYPE::BUFFER:
				case TDescriptorsBindingsTable::E_DESCRIPTOR_TYPE::RAW_BUFFER:
				{
					TPtr<CD3D12Buffer> pBuffer = mpGraphicsObjectManagerD3D12Impl->GetD3D12BufferPtr(currResourceEntity.mValue.mBuffer);
					if (!pBuffer)
					{
						continue;
					}

					srcSrvCPUHandle = i < currPipelineActiveSlots.mSRVActiveSlots.size() ? pBuffer->GetShaderResourceViewHandle().mCPUHandle : pBuffer->GetUnorderedAccessViewHandle().mCPUHandle;
				}
				break;

				case TDescriptorsBindingsTable::E_DESCRIPTOR_TYPE::TEXTURE:
				{
					TPtr<CD3D12TextureImpl> pTexture = mpGraphicsObjectManagerD3D12Impl->GetD3D12TexturePtr(currResourceEntity.mValue.mTexture);
					if (!pTexture)
					{
						continue;
					}

					srcSrvCPUHandle  = i < currPipelineActiveSlots.mSRVActiveSlots.size() ? pTexture->GetShaderResourceDescriptor().mCPUHandle : pTexture->GetUnorderedAccessViewHandle().mCPUHandle;
				}
				break;
			}

			D3D12_CPU_DESCRIPTOR_HANDLE destSrvCPUHandle = srvResourcesBlock.mCPUHandle;
			destSrvCPUHandle.ptr += i * mpDeviceContext->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			mpDeviceContext->GetDevice()->CopyDescriptorsSimple(1, destSrvCPUHandle, srcSrvCPUHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}

		switch (pipelineType)
		{
			case E_PIPELINE_TYPE::GRAPHICS:
				pCurrCommandBuffer->SetGraphicsRootDescriptorTable(currPipelineActiveSlots.mResourcesTableRootIndex, srvResourcesBlock.mGPUHandle);
				break;
			case E_PIPELINE_TYPE::COMPUTE:
				pCurrCommandBuffer->SetComputeRootDescriptorTable(currPipelineActiveSlots.mResourcesTableRootIndex, srvResourcesBlock.mGPUHandle);
				break;
		}

		if (!currPipelineActiveSlots.mSamplersActiveSlots.empty())
		{
			const TD3D12ResourceDescriptor& samplersGPUDescriptorsBlock = mpSamplersDescriptorsHeaps[currFrameIndex]->AllocDescriptorsBlock(static_cast<U32>(currPipelineActiveSlots.mSamplersActiveSlots.size()));
			D3D12_CPU_DESCRIPTOR_HANDLE currSamplersDestDescriptor = samplersGPUDescriptorsBlock.mCPUHandle;

			for (U32 i = 0; i < currPipelineActiveSlots.mSamplersActiveSlots.size(); ++i)
			{
				const TTextureSamplerId currSamplerHandle = mDescriptorsBindingsTable.mSamplers[currPipelineActiveSlots.mSamplersActiveSlots[i]];
				if (TTextureSamplerId::Invalid == currSamplerHandle)
				{
					continue;
				}

				const auto& getSamplerResult = mpGraphicsObjectManagerD3D12Impl->GetTextureSampler(currSamplerHandle);
				if (getSamplerResult.HasError())
				{
					continue;
				}

				currSamplersDestDescriptor.ptr = samplersGPUDescriptorsBlock.mCPUHandle.ptr + i * mpDeviceContext->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
				mpDeviceContext->GetDevice()->CopyDescriptorsSimple(1, currSamplersDestDescriptor, getSamplerResult.Get().mCPUHandle, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
			}

			switch (pipelineType)
			{
				case E_PIPELINE_TYPE::GRAPHICS:
					pCurrCommandBuffer->SetGraphicsRootDescriptorTable(currPipelineActiveSlots.mSamplersTableRootIndex, samplersGPUDescriptorsBlock.mGPUHandle);
					break;
				case E_PIPELINE_TYPE::COMPUTE:
					pCurrCommandBuffer->SetComputeRootDescriptorTable(currPipelineActiveSlots.mSamplersTableRootIndex, samplersGPUDescriptorsBlock.mGPUHandle);
					break;
			}
		}

		TDE2_ASSERT(mpActivePipelineStates[currFrameIndex]);
		if (!mpActivePipelineStates[currFrameIndex])
		{
			return;
		}

		FlushBarriers();
	}

	ID3D12GraphicsCommandList4* CD3D12GraphicsContext::_getCurrCommandListPtr()
	{
		return mpCommandBuffers[mpSwapchain->GetBackBufferTargetIndex()]->GetHandle().Get();
	}

	void CD3D12GraphicsContext::BeginFrame()
	{
		const U32 currFrameId = mpSwapchain->GetBackBufferTargetIndex();

		TPtr<CD3D12CommandBuffer> pCurrCommandBuffer = mpCommandBuffers[currFrameId];

		CD3D12Fence& currCmdBufferFence = pCurrCommandBuffer->GetFenceEntry();
		currCmdBufferFence.WaitForSignal();
		currCmdBufferFence.Reset();

		{
			std::lock_guard<std::mutex> lock(mGarbageCollectorMutex);

			for (auto& currGarbageEntity : mAwaitingDeletionObjects[currFrameId])
			{
				if (!currGarbageEntity.mpAllocation)
				{
					continue;
				}

				currGarbageEntity.mpAllocation->Release();
			}

			mAwaitingDeletionObjects[currFrameId].clear();
		}

		pCurrCommandBuffer->Begin();

		mpShaderResourcesDescriptorsHeaps[currFrameId]->Reset();
		mpSamplersDescriptorsHeaps[currFrameId]->Reset();

		mDescriptorsBindingsTable.Reset();
	}

	void CD3D12GraphicsContext::Present()
	{
		TDE2_PROFILER_SCOPE("CD3D12GraphicsContext::Present");
		
		TPtr<CD3D12CommandBuffer> pCurrCommandBuffer = mpCommandBuffers[mpSwapchain->GetBackBufferTargetIndex()];
		ID3D12GraphicsCommandList* pCurrCommandList = pCurrCommandBuffer->GetHandle().Get();

		CD3DX12_RESOURCE_BARRIER rtTransitionBarrier = CD3DX12_RESOURCE_BARRIER::Transition(mpSwapchain->GetCurrRenderTarget().Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		pCurrCommandList->ResourceBarrier(1, &rtTransitionBarrier);

		mpSwapchain->SetCurrRenderTargetLayout(E_RESOURCE_LAYOUT::UNDEFINED);

		pCurrCommandBuffer->End();

		ID3D12CommandList* ppCommandLists[] = { pCurrCommandList };
		mpDeviceContext->GetCommandQueue()->ExecuteCommandLists(1, ppCommandLists);

		pCurrCommandBuffer->GetFenceEntry().Signal();

		mpSwapchain->Present();
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

	E_RESULT_CODE CD3D12GraphicsContext::ExecuteImmediate(const std::function<void(ID3D12GraphicsCommandList*)>& command)
	{
		if (!command)
		{
			return RC_INVALID_ARGS;
		}

		CD3D12Fence& fence = mpImmediateCommandBuffer->GetFenceEntry();

		E_RESULT_CODE result = mpImmediateCommandBuffer->Begin();

		ID3D12GraphicsCommandList* pInternalCommandList = mpImmediateCommandBuffer->GetHandle().Get();

		command(pInternalCommandList);

		result = result | mpImmediateCommandBuffer->End();

		ID3D12CommandList* ppCommandLists[] = { pInternalCommandList };
		mpDeviceContext->GetCommandQueue()->ExecuteCommandLists(1, ppCommandLists);

		fence.WaitForIdle();
		fence.Reset();

		return result;
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

		D3D12_VERTEX_BUFFER_VIEW vertexBufferViewDesc{};
		vertexBufferViewDesc.BufferLocation = pBuffer->GetGPUAddress();
		vertexBufferViewDesc.SizeInBytes    = static_cast<U32>(pBuffer->GetSize());
		vertexBufferViewDesc.StrideInBytes  = strideSize;

		_getCurrCommandListPtr()->IASetVertexBuffers(slot, 1, &vertexBufferViewDesc);

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

		D3D12_INDEX_BUFFER_VIEW indexBufferViewDesc{};
		indexBufferViewDesc.BufferLocation = pBuffer->GetGPUAddress();
		indexBufferViewDesc.SizeInBytes    = static_cast<U32>(pBuffer->GetSize());
		indexBufferViewDesc.Format         = CD3D12Mappings::GetIndexFormat(pBuffer->GetParams().mIndexFormat);

		_getCurrCommandListPtr()->IASetIndexBuffer(&indexBufferViewDesc);

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

			mDescriptorsBindingsTable.mUAVBuffers[slot].mType = pBuffer->GetParams().mStructuredBufferType == E_STRUCTURED_BUFFER_TYPE::INDIRECT_DRAW_BUFFER ? 
				TDescriptorsBindingsTable::E_DESCRIPTOR_TYPE::RAW_BUFFER : TDescriptorsBindingsTable::E_DESCRIPTOR_TYPE::BUFFER;

			mDescriptorsBindingsTable.mUAVBuffers[slot].mValue.mBuffer = bufferHandle;
		}
		else
		{
			TDE2_ASSERT(slot < mDescriptorsBindingsTable.mSRVBuffers.size());

			mDescriptorsBindingsTable.mSRVBuffers[slot].mType = pBuffer->GetParams().mStructuredBufferType == E_STRUCTURED_BUFFER_TYPE::INDIRECT_DRAW_BUFFER ?
				TDescriptorsBindingsTable::E_DESCRIPTOR_TYPE::RAW_BUFFER : TDescriptorsBindingsTable::E_DESCRIPTOR_TYPE::BUFFER;

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


	E_RESULT_CODE CD3D12GraphicsContext::UpdateTexture2D(TTextureHandleId textureHandle, U32 mipLevel, const TRectI32& regionRect, const void* pData, USIZE dataSize)
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		auto pTexture = mpGraphicsObjectManagerD3D12Impl->GetD3D12TexturePtr(textureHandle);
		if (!pTexture)
		{
			return RC_FAIL;
		}

		const USIZE textureSize = static_cast<USIZE>(regionRect.width * regionRect.height * CFormatUtils::GetFormatSize(pTexture->GetParams().mFormat));

		TPtr<IBuffer> pStagingBuffer = TPtr<IBuffer>(CreateD3D12Buffer(this, { E_BUFFER_USAGE_TYPE::DYNAMIC, E_BUFFER_TYPE::GENERIC, textureSize, nullptr }, result));
		if (RC_OK != result || !pStagingBuffer)
		{
			return result;
		}

		TPtr<CD3D12Buffer> pInternalStagingBuffer = DynamicPtrCast<CD3D12Buffer>(pStagingBuffer);

		result = pStagingBuffer->Map(E_BUFFER_MAP_TYPE::BMT_WRITE);
		if (RC_OK != result)
		{
			return result;
		}

		result = pStagingBuffer->Write(pData, textureSize);
		if (RC_OK != result)
		{
			return result;
		}

		pStagingBuffer->Unmap();
		
		result = ExecuteImmediate([=](ID3D12GraphicsCommandList* pCommandList)
			{
				CD3DX12_RESOURCE_BARRIER transitionBarrier = CD3DX12_RESOURCE_BARRIER::Transition(pTexture->GetHandle().Get(), GetInternalResourceState(pTexture->GetLayout()), D3D12_RESOURCE_STATE_COPY_DEST);
				pCommandList->ResourceBarrier(1, &transitionBarrier);

				const D3D12_RESOURCE_DESC& textureDesc = pTexture->GetHandle()->GetDesc();
				D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout{};

				mpDeviceContext->GetDevice()->GetCopyableFootprints(&textureDesc, 0, 1, 0, &layout, nullptr, nullptr, nullptr);

				layout.Footprint.Width    = regionRect.width;
				layout.Footprint.Height   = regionRect.height;
				layout.Footprint.RowPitch = regionRect.width * CFormatUtils::GetFormatSize(pTexture->GetParams().mFormat);

				D3D12_TEXTURE_COPY_LOCATION dst{ pTexture->GetHandle().Get(), D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX, {} };
				D3D12_TEXTURE_COPY_LOCATION src{ pInternalStagingBuffer->GetHandle().Get(), D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT, { layout } };

				pCommandList->CopyTextureRegion(&dst, static_cast<U32>(regionRect.x), static_cast<U32>(regionRect.y), 0, &src, nullptr);

				transitionBarrier = CD3DX12_RESOURCE_BARRIER::Transition(pTexture->GetHandle().Get(), D3D12_RESOURCE_STATE_COPY_DEST, GetInternalResourceState(pTexture->GetLayout()));
				pCommandList->ResourceBarrier(1, &transitionBarrier);
			});

		return result;
	}

	E_RESULT_CODE CD3D12GraphicsContext::UpdateTexture2DArray(TTextureHandleId textureHandle, U32 index, const TRectI32& regionRect, const void* pData, USIZE dataSize)
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		auto pTexture = mpGraphicsObjectManagerD3D12Impl->GetD3D12TexturePtr(textureHandle);
		if (!pTexture)
		{
			return RC_FAIL;
		}

		const USIZE textureSize = static_cast<USIZE>(regionRect.width * regionRect.height * CFormatUtils::GetFormatSize(pTexture->GetParams().mFormat));
		const U32 subresourceIndex = 0;// index* pTexture->GetParams().mNumOfMipLevels;

		TPtr<IBuffer> pStagingBuffer = TPtr<IBuffer>(CreateD3D12Buffer(this, { E_BUFFER_USAGE_TYPE::DYNAMIC, E_BUFFER_TYPE::GENERIC, textureSize, nullptr }, result));
		if (RC_OK != result || !pStagingBuffer)
		{
			return result;
		}

		TPtr<CD3D12Buffer> pInternalStagingBuffer = DynamicPtrCast<CD3D12Buffer>(pStagingBuffer);

		result = pStagingBuffer->Map(E_BUFFER_MAP_TYPE::BMT_WRITE);
		if (RC_OK != result)
		{
			return result;
		}

		result = pStagingBuffer->Write(pData, textureSize);
		if (RC_OK != result)
		{
			return result;
		}

		pStagingBuffer->Unmap();

		result = ExecuteImmediate([=](ID3D12GraphicsCommandList* pCommandList)
			{
				CD3DX12_RESOURCE_BARRIER transitionBarrier = CD3DX12_RESOURCE_BARRIER::Transition(pTexture->GetHandle().Get(), GetInternalResourceState(pTexture->GetLayout()), D3D12_RESOURCE_STATE_COPY_DEST);
				pCommandList->ResourceBarrier(1, &transitionBarrier);

				const D3D12_RESOURCE_DESC& textureDesc = pTexture->GetHandle()->GetDesc();
				D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout{};

				mpDeviceContext->GetDevice()->GetCopyableFootprints(&textureDesc, 0, 1, 0, &layout, nullptr, nullptr, nullptr);

				layout.Footprint.Width    = regionRect.width;
				layout.Footprint.Height   = regionRect.height;
				layout.Footprint.RowPitch = regionRect.width * CFormatUtils::GetFormatSize(pTexture->GetParams().mFormat);

				D3D12_TEXTURE_COPY_LOCATION dst{ pTexture->GetHandle().Get(), D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX, { subresourceIndex } };
				D3D12_TEXTURE_COPY_LOCATION src{ pInternalStagingBuffer->GetHandle().Get(), D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT, { layout } };

				pCommandList->CopyTextureRegion(&dst, static_cast<U32>(regionRect.x), static_cast<U32>(regionRect.y), 0, &src, nullptr);

				transitionBarrier = CD3DX12_RESOURCE_BARRIER::Transition(pTexture->GetHandle().Get(), D3D12_RESOURCE_STATE_COPY_DEST, GetInternalResourceState(pTexture->GetLayout()));
				pCommandList->ResourceBarrier(1, &transitionBarrier);
			});

		return result;
	}

	E_RESULT_CODE CD3D12GraphicsContext::UpdateCubemapTexture(TTextureHandleId textureHandle, E_CUBEMAP_FACE face, const TRectI32& regionRect, const void* pData, USIZE dataSize)
	{
		return UpdateTexture2DArray(textureHandle, static_cast<U32>(face), regionRect, pData, dataSize);
	}

	E_RESULT_CODE CD3D12GraphicsContext::UpdateTexture3D(TTextureHandleId textureHandle, U32 depthFrom, U32 depthTo, const TRectI32& regionRect, const void* pData, USIZE dataSize)
	{
		TDE2_UNIMPLEMENTED();
		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsContext::CopyResource(TTextureHandleId sourceHandle, TTextureHandleId destHandle)
	{
		auto pSourceTexture = mpGraphicsObjectManagerD3D12Impl->GetD3D12TexturePtr(sourceHandle);
		if (!pSourceTexture)
		{
			return RC_INVALID_ARGS;
		}

		auto pDestTexture = mpGraphicsObjectManagerD3D12Impl->GetD3D12TexturePtr(destHandle);
		if (!pDestTexture)
		{
			return RC_INVALID_ARGS;
		}

		const E_RESOURCE_LAYOUT currSourceLayout = pSourceTexture->GetLayout();
		const E_RESOURCE_LAYOUT currDestLayout = pDestTexture->GetLayout();

		E_RESULT_CODE result = RC_OK;

		result = result | pSourceTexture->Transition(E_RESOURCE_LAYOUT::COPY_SRC);
		result = result | pDestTexture->Transition(E_RESOURCE_LAYOUT::COPY_DEST);

		FlushBarriers();

		_getCurrCommandListPtr()->CopyResource(pDestTexture->GetHandle().Get(), pSourceTexture->GetHandle().Get());

		if (E_RESOURCE_LAYOUT::UNDEFINED != currSourceLayout)
		{
			result = result | pSourceTexture->Transition(currSourceLayout);
		}

		if (E_RESOURCE_LAYOUT::UNDEFINED != currDestLayout)
		{
			result = result | pDestTexture->Transition(currDestLayout);
		}

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
		auto pSourceBuffer = mpGraphicsObjectManagerD3D12Impl->GetD3D12BufferPtr(sourceHandle);
		if (!pSourceBuffer)
		{
			return RC_INVALID_ARGS;
		}

		auto pDestBuffer = mpGraphicsObjectManagerD3D12Impl->GetD3D12BufferPtr(destHandle);
		if (!pDestBuffer)
		{
			return RC_INVALID_ARGS;
		}

		const E_RESOURCE_LAYOUT currSourceLayout = pSourceBuffer->GetLayout();
		const E_RESOURCE_LAYOUT currDestLayout = pDestBuffer->GetLayout();

		E_RESULT_CODE result = RC_OK;

		result = result | pSourceBuffer->Transition(E_RESOURCE_LAYOUT::COPY_SRC);
		result = result | pDestBuffer->Transition(E_RESOURCE_LAYOUT::COPY_DEST);

		FlushBarriers();

		_getCurrCommandListPtr()->CopyResource(pDestBuffer->GetHandle().Get(), pSourceBuffer->GetHandle().Get());

		if (E_RESOURCE_LAYOUT::UNDEFINED != currSourceLayout)
		{
			result = result | pSourceBuffer->Transition(currSourceLayout);
		}

		if (E_RESOURCE_LAYOUT::UNDEFINED != currDestLayout)
		{
			result = result | pDestBuffer->Transition(currDestLayout);
		}

		return RC_OK;
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
			CD3DX12_RESOURCE_BARRIER::Transition(pBuffer->GetHandle().Get(), GetInternalResourceState(barrierInfo.mCurrLayout), GetInternalResourceState(barrierInfo.mNewLayout)));
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
			CD3DX12_RESOURCE_BARRIER::Transition(pTextureImpl->GetHandle().Get(), GetInternalResourceState(barrierInfo.mCurrLayout), GetInternalResourceState(barrierInfo.mNewLayout), subresourceId));
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

		for (int i = 0; i < resourceBarriers.size(); ++i)
			_getCurrCommandListPtr()->ResourceBarrier(1, &resourceBarriers[i]);

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
		_preparePipelineState();

		ComPtr<ID3D12GraphicsCommandList> pCurrCommandBuffer = _getCurrCommandListPtr();

		pCurrCommandBuffer->IASetPrimitiveTopology(CD3D12Mappings::GetPrimitiveTopology(topology));
		pCurrCommandBuffer->DrawInstanced(numOfVertices, 1, startVertex, 0);
	}

	void CD3D12GraphicsContext::DrawIndexed(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, U32 baseVertex, U32 startIndex, U32 numOfIndices)
	{
		_preparePipelineState();

		ComPtr<ID3D12GraphicsCommandList> pCurrCommandBuffer = _getCurrCommandListPtr();

		pCurrCommandBuffer->IASetPrimitiveTopology(CD3D12Mappings::GetPrimitiveTopology(topology));
		pCurrCommandBuffer->DrawIndexedInstanced(numOfIndices, 1, startIndex, baseVertex, 0);
	}

	void CD3D12GraphicsContext::DrawInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, U32 startVertex, U32 verticesPerInstance, U32 startInstance, U32 numOfInstances)
	{
		_preparePipelineState();

		ComPtr<ID3D12GraphicsCommandList> pCurrCommandBuffer = _getCurrCommandListPtr();

		pCurrCommandBuffer->IASetPrimitiveTopology(CD3D12Mappings::GetPrimitiveTopology(topology));
		pCurrCommandBuffer->DrawInstanced(verticesPerInstance, numOfInstances, startVertex, startInstance);
	}

	void CD3D12GraphicsContext::DrawIndexedInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, U32 baseVertex, U32 startIndex,
		U32 startInstance, U32 indicesPerInstance, U32 numOfInstances)
	{
		_preparePipelineState();

		ComPtr<ID3D12GraphicsCommandList> pCurrCommandBuffer = _getCurrCommandListPtr();

		pCurrCommandBuffer->IASetPrimitiveTopology(CD3D12Mappings::GetPrimitiveTopology(topology));
		pCurrCommandBuffer->DrawIndexedInstanced(indicesPerInstance, numOfInstances, startIndex, baseVertex, startInstance);
	}

	void CD3D12GraphicsContext::DrawIndirectInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, TBufferHandleId argsBufferHandle, U32 alignedOffset)
	{
		_preparePipelineState();

		ComPtr<ID3D12GraphicsCommandList> pCurrCommandBuffer = _getCurrCommandListPtr();

		auto pArgsBuffer = mpGraphicsObjectManagerD3D12Impl->GetD3D12BufferPtr(argsBufferHandle);

		// TODO: Add configurable commands count via either maxCount or pCountBuffer
		pCurrCommandBuffer->IASetPrimitiveTopology(CD3D12Mappings::GetPrimitiveTopology(topology));
		pCurrCommandBuffer->ExecuteIndirect(mpDeviceContext->GetDrawInstancedIndirectCmdSignature().Get(), 1, pArgsBuffer ? pArgsBuffer->GetHandle().Get() : nullptr, alignedOffset, nullptr, 0);
	}

	void CD3D12GraphicsContext::DrawIndirectIndexedInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, TBufferHandleId argsBufferHandle, U32 alignedOffset)
	{
		_preparePipelineState();

		ComPtr<ID3D12GraphicsCommandList> pCurrCommandBuffer = _getCurrCommandListPtr();

		auto pArgsBuffer = mpGraphicsObjectManagerD3D12Impl->GetD3D12BufferPtr(argsBufferHandle);

		// TODO: Add configurable commands count via either maxCount or pCountBuffer
		pCurrCommandBuffer->IASetPrimitiveTopology(CD3D12Mappings::GetPrimitiveTopology(topology));
		pCurrCommandBuffer->ExecuteIndirect(mpDeviceContext->GetDrawIndexedInstancedIndirectCmdSignature().Get(), 1, pArgsBuffer ? pArgsBuffer->GetHandle().Get() : nullptr, alignedOffset, nullptr, 0);
	}

	void CD3D12GraphicsContext::DispatchCompute(U32 groupsCountX, U32 groupsCountY, U32 groupsCountZ)
	{
		_preparePipelineState();

		TDE2_PROFILER_SCOPE("CD3D12GraphicsContext::DispatchCompute");

		ComPtr<ID3D12GraphicsCommandList> pCurrCommandBuffer = _getCurrCommandListPtr();

		pCurrCommandBuffer->Dispatch(groupsCountX, groupsCountY, groupsCountZ);
	}

	void CD3D12GraphicsContext::DispatchIndirectCompute(TBufferHandleId argsBufferHandle, U32 alignedOffset)
	{
		_preparePipelineState();

		TDE2_PROFILER_SCOPE("CD3D12GraphicsContext::DispatchIndirectCompute");

		auto pArgsBuffer = mpGraphicsObjectManagerD3D12Impl->GetD3D12BufferPtr(argsBufferHandle);
		_getCurrCommandListPtr()->ExecuteIndirect(mpDeviceContext->GetDispatchIndirectCmdSignature().Get(), 1, pArgsBuffer ? pArgsBuffer->GetHandle().Get() : nullptr, alignedOffset, nullptr, 0);
	}

	E_RESULT_CODE CD3D12GraphicsContext::BindPipelineState(CD3D12BasePipeline* pPipeline)
	{
		const bool isGraphicsPipeline = E_PIPELINE_TYPE::GRAPHICS == pPipeline->GetType();

		if (isGraphicsPipeline)
		{
			TDE2_ASSERT(mIsRenderPassActive);
		}

		mpActivePipelineStates[mpSwapchain->GetBackBufferTargetIndex()] = pPipeline;

		ComPtr<ID3D12PipelineState> pPSO = nullptr;
		ID3D12GraphicsCommandList4* pCurrCommandList = _getCurrCommandListPtr();

		if (isGraphicsPipeline)
		{
			pCurrCommandList->SetGraphicsRootSignature(pPipeline->GetRootSignature());

			const U64 pipelineHash = isGraphicsPipeline ? (static_cast<U64>(ComputeStateDescHash(mCurrRenderPassInfo)) << 32) | pPipeline->GetHash() : pPipeline->GetHash();
			
			auto&& it = mCachedPipelinesLibrary.find(pipelineHash);
			if (it == mCachedPipelinesLibrary.cend())
			{
				pPSO = dynamic_cast<CD3D12GraphicsPipeline*>(pPipeline)->GetPipelineForRenderPass(mCurrRenderPassInfo);
				mCachedPipelinesLibrary[pipelineHash] = pPSO;
			}
			else
			{
				pPSO = it->second;
			}
		}
		else
		{
			pCurrCommandList->SetComputeRootSignature(pPipeline->GetRootSignature());
			pPSO = pPipeline->GetNativePSO();
		}

		TDE2_ASSERT(pPSO);
		pCurrCommandList->SetPipelineState(pPSO.Get());

		return RC_OK;
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
		TDE2_ASSERT(!mIsRenderPassActive);

		mCurrRenderPassInfo = TRenderPassInfo(mpGraphicsObjectManager.Get(), framebufferInfo);

		CFixedVector<D3D12_RENDER_PASS_RENDER_TARGET_DESC, RENDER_TARGETS_MAX_COUNT> renderTargetsDescs{};
		D3D12_RENDER_PASS_DEPTH_STENCIL_DESC depthStencilBufferDesc{};

		TVector2 viewportSizes{};

		for (USIZE i = 0; i < framebufferInfo.mAttachments.size(); ++i)
		{
			const TFramebufferInfo::TAttachment& currAttachment = framebufferInfo.mAttachments[i];

			TPtr<CD3D12TextureImpl> pRenderTargetTexture = mpGraphicsObjectManagerD3D12Impl->GetD3D12TexturePtr(currAttachment.mTargetHandle);
			if (!pRenderTargetTexture) // \note The special corner case when there is no render target attached except the back buffer
			{
				mCurrRenderPassInfo.mRenderTargetFormats[0] = mpSwapchain->GetBackBuffersFormat();

				const TRectU32& windowRect = mpWindowSystem->GetClientRect();
				viewportSizes.x = static_cast<F32>(windowRect.width);
				viewportSizes.y = static_cast<F32>(windowRect.height);

				D3D12_RENDER_PASS_RENDER_TARGET_DESC currRenderTargetDesc{};

				currRenderTargetDesc.BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
				currRenderTargetDesc.cpuDescriptor        = mpSwapchain->GetCurrRenderTargetView();
				currRenderTargetDesc.EndingAccess.Type    = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;

				if (currAttachment.mClearValue)
				{
					const TColor32F& targetClearColorValue = std::get<TColor32F>(currAttachment.mClearValue.value());
					const F32 clearColor[] { targetClearColorValue.r, targetClearColorValue.g, targetClearColorValue.b, targetClearColorValue.a };

					currRenderTargetDesc.BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
					memcpy(&currRenderTargetDesc.BeginningAccess.Clear.ClearValue.Color, &clearColor, sizeof(TColor32F));
				}

				renderTargetsDescs.emplace_back(currRenderTargetDesc);

				//// \note Add barrier for current swapchain's image
				if (mpSwapchain->GetCurrRenderTargetLayout() != E_RESOURCE_LAYOUT::RENDER_TARGET)
				{
					mResourceBarriers[mpSwapchain->GetBackBufferTargetIndex()].emplace_back(CD3DX12_RESOURCE_BARRIER::Transition(mpSwapchain->GetCurrRenderTarget().Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));					
					mpSwapchain->SetCurrRenderTargetLayout(E_RESOURCE_LAYOUT::RENDER_TARGET);
				}

				continue;
			}

			if (!HasEnumFlag(pRenderTargetTexture->GetParams().mBindFlags, E_BIND_GRAPHICS_TYPE::BIND_RENDER_TARGET))
			{
				TDE2_ASSERT_MSG(false, "[CD3D12GraphicsContext] Try to use the texture that is not a render target as a color attachment");
				return RC_FAIL;
			}

			viewportSizes.x = static_cast<F32>(std::max<U32>(static_cast<U32>(viewportSizes.x), pRenderTargetTexture->GetParams().mWidth));
			viewportSizes.y = static_cast<F32>(std::max<U32>(static_cast<U32>(viewportSizes.y), pRenderTargetTexture->GetParams().mHeight));

			D3D12_RENDER_PASS_RENDER_TARGET_DESC currRenderTargetDesc{};

			currRenderTargetDesc.BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
			currRenderTargetDesc.cpuDescriptor        = pRenderTargetTexture->GetRenderTargetDescriptor().mCPUHandle; // \todo get CPU_HANDLE for back buffer view
			currRenderTargetDesc.EndingAccess.Type    = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;

			if (currAttachment.mClearValue)
			{
				const TColor32F& targetClearColorValue = std::get<TColor32F>(currAttachment.mClearValue.value());
				const F32 clearColor[]{ targetClearColorValue.r, targetClearColorValue.g, targetClearColorValue.b, targetClearColorValue.a };

				currRenderTargetDesc.BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
				memcpy(&currRenderTargetDesc.BeginningAccess.Clear.ClearValue.Color, &clearColor, sizeof(TColor32F));
			}

			renderTargetsDescs.emplace_back(currRenderTargetDesc);

			pRenderTargetTexture->Transition(E_RESOURCE_LAYOUT::RENDER_TARGET);
		}

		if (framebufferInfo.mDepthStencilAttachment)
		{
			const auto& depthStencilAttachment = framebufferInfo.mDepthStencilAttachment.value();

			TPtr<CD3D12TextureImpl> pDepthBufferTexture = mpGraphicsObjectManagerD3D12Impl->GetD3D12TexturePtr(depthStencilAttachment.mTargetHandle);

			if (!HasEnumFlag(pDepthBufferTexture->GetParams().mBindFlags, E_BIND_GRAPHICS_TYPE::BIND_DEPTH_BUFFER))
			{
				TDE2_ASSERT_MSG(false, "[CD3D12GraphicsContext] Try to bind the texture that is not a depth buffer as a depth-stencil attachment");
				return RC_FAIL;
			}

			if (framebufferInfo.mAttachments.empty())
			{
				viewportSizes.x = static_cast<F32>(std::max<U32>(static_cast<U32>(viewportSizes.x), pDepthBufferTexture->GetParams().mWidth));
				viewportSizes.y = static_cast<F32>(std::max<U32>(static_cast<U32>(viewportSizes.y), pDepthBufferTexture->GetParams().mHeight));
			}

			const bool hasDepthClearValue   = depthStencilAttachment.mDepthClearValue.has_value();
			const bool hasStencilClearValue = depthStencilAttachment.mStencilClearValue.has_value();

			depthStencilBufferDesc.DepthBeginningAccess.Type   = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
			depthStencilBufferDesc.StencilBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
			depthStencilBufferDesc.DepthEndingAccess.Type      = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
			depthStencilBufferDesc.StencilEndingAccess.Type    = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;

			if (hasDepthClearValue)
			{
				const auto& targetClearValueVariant = depthStencilAttachment.mDepthClearValue.value();

				depthStencilBufferDesc.DepthBeginningAccess.Clear.ClearValue.DepthStencil.Depth = std::get<F32>(targetClearValueVariant);
				depthStencilBufferDesc.DepthBeginningAccess.Type                                = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
			}

			if (hasStencilClearValue)
			{
				const auto& targetClearValueVariant = depthStencilAttachment.mStencilClearValue.value();

				depthStencilBufferDesc.StencilBeginningAccess.Clear.ClearValue.DepthStencil.Stencil = std::get<U8>(targetClearValueVariant);
				depthStencilBufferDesc.StencilBeginningAccess.Type                                  = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
			}

			if (pDepthBufferTexture)
			{
				depthStencilBufferDesc.cpuDescriptor = framebufferInfo.mDepthStencilAttachment ? pDepthBufferTexture->GetDepthBufferDescriptor().mCPUHandle : mpSwapchain->GetDefaultDepthStencilTargetView();
				pDepthBufferTexture->Transition(E_RESOURCE_LAYOUT::DEPTH_STENCIL);
			}
		}

		FlushBarriers();

		_getCurrCommandListPtr()->BeginRenderPass(static_cast<U32>(renderTargetsDescs.size()), renderTargetsDescs.data(), framebufferInfo.mDepthStencilAttachment ? &depthStencilBufferDesc : nullptr, D3D12_RENDER_PASS_FLAG_NONE);
		mIsRenderPassActive = true;
		
		SetScissorRect({ 0, 0, static_cast<U32>(viewportSizes.x), static_cast<U32>(viewportSizes.y) }); // \note Set up default scissor rect for the whole screen

		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsContext::EndRenderPass()
	{
		TDE2_ASSERT(mIsRenderPassActive);

		_getCurrCommandListPtr()->EndRenderPass();
		mIsRenderPassActive = false;

		return RC_OK;
	}

	const TGraphicsCtxInternalData& CD3D12GraphicsContext::GetInternalData() const
	{
		TDE2_UNREACHABLE();

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
		
		if (pEvent->GetEventType() != TOnWindowResized::GetTypeId())
		{
			return RC_OK;
		}

		const TOnWindowResized* pOnWindowResizedEvent = dynamic_cast<const TOnWindowResized*>(pEvent);

		return RC_OK;
	}

#if TDE2_DEBUG_MODE

	void CD3D12GraphicsContext::BeginSectionMarker(const std::string& id)
	{
		PIXBeginEvent(_getCurrCommandListPtr(), 0x000000ff, id.c_str());
	}

	void CD3D12GraphicsContext::EndSectionMarker()
	{
		PIXEndEvent(_getCurrCommandListPtr());
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

	Vector<U8> CD3D12GraphicsContext::GetBackBufferData() const
	{
		Vector<U8> backBufferData;

		TDE2_UNIMPLEMENTED();

		return std::move(backBufferData);
	}

	ID3D12Device5* CD3D12GraphicsContext::GetDeviceContext() const
	{
		return mpDeviceContext->GetDevice().Get();
	}

	D3D12MA::Allocator* CD3D12GraphicsContext::GetMemoryAllocator() const
	{
		return mpDeviceContext->GetMemoryAllocator().Get();
	}

	TPtr<ID3D12CPUDescriptorsAllocator> CD3D12GraphicsContext::GetDescriptorsAllocator(D3D12_DESCRIPTOR_HEAP_TYPE heapType) const
	{
		return mpDeviceContext->GetDescriptorsAllocator(heapType);
	}


	IGraphicsContext* CreateD3D12GraphicsContext(TPtr<IWindowSystem> pWindowSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IGraphicsContext, CD3D12GraphicsContext, result, pWindowSystem);
	}
}

#endif