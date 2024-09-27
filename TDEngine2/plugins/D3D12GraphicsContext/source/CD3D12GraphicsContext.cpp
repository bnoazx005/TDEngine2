#include "../include/CD3D12GraphicsContext.h"
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
	class CD3D12GraphicsContext : public IGraphicsContext, public IEventHandler, public CBaseObject
	{
		public:
			friend IGraphicsContext* CreateD3D12GraphicsContext(TPtr<IWindowSystem>, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_TYPE(CD3D12GraphicsContext)

			/*!
				\brief The method initializes an initial state of the object

				\param[in] pWindowSystem A pointer to implementation of IWindowSystem interface

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			E_RESULT_CODE Init(TPtr<IWindowSystem> pWindowSystem) override;

			/*!
				\brief The method clears up back buffer with specified color

				\param[in] color The new color of a back buffer
			*/

			void ClearBackBuffer(const TColor32F& color) override;

			/*!
				\brief The method clears up render target with specified color

				\param[in] slot A slot into which the render target that should be cleared up is bound
				\param[in] color The new color of a render target
			*/

			void ClearRenderTarget(U8 slot, const TColor32F& color) override;

			/*!
				\brief The method clears up depth buffer with specified values

				\param[in] value The depth buffer will be cleared with this value
			*/

			void ClearDepthBuffer(F32 value) override;

			/*!
				\brief The method clears up stencil buffer with specified values

				\param[in] value The stencil buffer will be cleared with this value
			*/

			void ClearStencilBuffer(U8 value) override;

			void BeginFrame() override;

			/*!
				\brief The method swaps a current buffer with a back one
			*/

			void Present() override;

			/*!
				\brief The method sets up a viewport's parameters

				\param[in] x x position of left hand side of a viewport
				\param[in] y y position of a left top corner of a viewport
				\param[in] width width of a viewport
				\param[in] height height of a viewport
				\param[in] minDepth minimum depth of a viewport
				\param[in] maxDepth maximum depth of a viewport
			*/

			void SetViewport(F32 x, F32 y, F32 width, F32 height, F32 minDepth, F32 maxDepth) override;

			/*!
				\brief The method specifies rectangle for scissor testing, all the geometry outside of which
				is rejected

				\param[in] scissorRect A rectangle for scissor test
			*/

			void SetScissorRect(const TRectU32& scissorRect) override;

			/*!
				\brief The method computes a perspective projection matrix specific for a graphics context

				\param[in] fov A field of view

				\param[in] aspect An aspect ratio of a screen

				\param[in] zn A z value of a near clip plance

				\param[in] zf A z value of a far clip plane

				\return The method computes a perspective projection matrix specific for a graphics context
			*/

			TMatrix4 CalcPerspectiveMatrix(F32 fov, F32 aspect, F32 zn, F32 zf) override;

			/*!
				\brief The method computes an orthographic projection matrix specific for a graphics context

				\param[in] left X coordinate of a left corner of a screen

				\param[in] top Y coordinate of a left top corner of a screen

				\param[in] right X coordinate of a right corner of a screen

				\param[in] bottom Y coordinate of a right bottom corner of a screen

				\param[in] zn A z value of a near clip plance

				\param[in] zf A z value of a far clip plane

				\param[in] isDepthless The flag determines whether the output matrix will process z axis or just to project onto some constant, which
				equals to average position between zn and zf

				\return The method computes a orthographic projection matrix specific for a graphics context
			*/

			TMatrix4 CalcOrthographicMatrix(F32 left, F32 top, F32 right, F32 bottom, F32 zn, F32 zf, bool isDepthless = false) override;

			E_RESULT_CODE SetVertexBuffer(U32 slot, TBufferHandleId vertexBufferHandle, U32 offset, U32 strideSize) override;
			E_RESULT_CODE SetIndexBuffer(TBufferHandleId indexBufferHandle, U32 offset) override;
			E_RESULT_CODE SetConstantBuffer(U32 slot, TBufferHandleId constantsBufferHandle) override;
			E_RESULT_CODE SetStructuredBuffer(U32 slot, TBufferHandleId bufferHandle, bool isWriteEnabled = false) override;

			E_RESULT_CODE SetTexture(U32 slot, TTextureHandleId textureHandle, bool isWriteEnabled = false) override;
			E_RESULT_CODE SetSampler(U32 slot, TTextureSamplerId samplerHandle) override;

			E_RESULT_CODE UpdateTexture2D(TTextureHandleId textureHandle, U32 mipLevel, const TRectI32& regionRect, const void* pData, USIZE dataSize) override;
			E_RESULT_CODE UpdateTexture2DArray(TTextureHandleId textureHandle, U32 index, const TRectI32& regionRect, const void* pData, USIZE dataSize) override;
			E_RESULT_CODE UpdateCubemapTexture(TTextureHandleId textureHandle, E_CUBEMAP_FACE face, const TRectI32& regionRect, const void* pData, USIZE dataSize) override;
			E_RESULT_CODE UpdateTexture3D(TTextureHandleId textureHandle, U32 depthFrom, U32 depthTo, const TRectI32& regionRect, const void* pData, USIZE dataSize) override;

			// Blit one texture to another
			E_RESULT_CODE CopyResource(TTextureHandleId sourceHandle, TTextureHandleId destHandle) override;
			// Upload data into texture
			E_RESULT_CODE CopyResource(TBufferHandleId sourceHandle, TTextureHandleId destHandle) override;
			// Readback data from texture into a buffer
			E_RESULT_CODE CopyResource(TTextureHandleId sourceHandle, TBufferHandleId destHandle) override;
			// Copy buffer to buffer
			E_RESULT_CODE CopyResource(TBufferHandleId sourceHandle, TBufferHandleId destHandle) override;

			/*!
				\brief The method copies counter of sourceHandle buffer into destHandle's one

				\param[in] sourceHandle A handle to structured appendable buffer resource that should be created with unordered access write flag
				\param[in] destHandle A handle to any buffer resource
				\param[in] offset An offset within dest buffer
			*/

			E_RESULT_CODE CopyCount(TBufferHandleId sourceHandle, TBufferHandleId destHandle, U32 offset) override;

			/*!
				\brief The method generates all mip levels for the specified texture
			*/

			E_RESULT_CODE GenerateMipMaps(TTextureHandleId textureHandle) override;

			/*!
				\brief The method sends a command to a GPU to draw current attached graphics data

				\param[in] topology A primitive topology's type

				\param[in] startVertex An index of a vertex within a vertex buffer

				\param[in] numOfVertices A total number of vertices should be drawn
			*/

			void Draw(E_PRIMITIVE_TOPOLOGY_TYPE topology, U32 startVertex, U32 numOfVertices) override;

			/*!
				\brief The method sends a command to a GPU to draw current attached graphics data using
				index buffer to group primitives

				\param[in] topology A primitive topology's type

				\param[in] indexFormatType A format of a single index

				\param[in] baseVertex A value that will be added to each index

				\param[in] startIndex A first index that will be read by GPU

				\param[in] numOfIndices A total number of indices
			*/

			void DrawIndexed(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, U32 baseVertex, U32 startIndex, U32 numOfIndices) override;

			/*!
				\brief The method sends a command to a GPU to draw current attached graphics data that uses instancing

				\param[in] topology A primitive topology's type

				\param[in] startVertex An index of a vertex within a vertex buffer

				\param[in] verticesPerInstance A number of vertices per instance

				\param[in] startInstance An offset value which is added to every index before read the data

				\param[in] numOfInstances A total number of instances should be drawn
			*/

			void DrawInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, U32 startVertex, U32 verticesPerInstance, U32 startInstance, U32 numOfInstances) override;

			/*!
				\brief The method sends a command to a GPU to draw current attached graphics data that uses instancing

				\param[in] topology A primitive topology's type

				\param[in] indexFormatType A format of a single index

				\param[in] baseVertex A value that will be added to each index

				\param[in] startIndex A first index that will be read by GPU

				\param[in] startInstance An offset value which is added to every index before read the data

				\param[in] indicesPerInstance A number of indices per instance

				\param[in] numOfInstances A total number of instances should be drawn
			*/

			void DrawIndexedInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, U32 baseVertex, U32 startIndex,
				U32 startInstance, U32 indicesPerInstance, U32 numOfInstances) override;

			/*!
				\brief The method sends a command to a GPU with enabled instancing but all the parameters are stored within GPU buffer

				\param[in] topology A primitive topology's type
				\param[in] argsBufferHandle A GPU buffer that holds arguments for draw command
				\param[in] alignedOffset Aligned offset to the beginning of GPU data
			*/

			void DrawIndirectInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, TBufferHandleId argsBufferHandle, U32 alignedOffset) override;

			/*!
				\brief The method sends a command to a GPU with enabled instancing but all the parameters are stored within GPU buffer

				\param[in] topology A primitive topology's type
				\param[in] indexFormatType A format of a single index
				\param[in] argsBufferHandle A GPU buffer that holds arguments for draw command
				\param[in] alignedOffset Aligned offset to the beginning of GPU data
			*/

			void DrawIndirectIndexedInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, TBufferHandleId argsBufferHandle, U32 alignedOffset) override;

			/*!
				\brief The method invokes a computation kernel over a given threads grid. A compute shader should be bound to the device first

				\param[in] groupsCountX A number of groups over X axis
				\param[in] groupsCountY A number of groups over Y axis
				\param[in] groupsCountZ A number of groups over Z axis
			*/

			void DispatchCompute(U32 groupsCountX, U32 groupsCountY, U32 groupsCountZ) override;

			/*!
				\brief The method invokes a computation kernel over a given threads grid. A compute shader should be bound to the device first

				\param[in] argsBufferHandle A GPU buffer that holds arguments for draw command
				\param[in] alignedOffset Aligned offset to the beginning of GPU data
			*/

			void DispatchIndirectCompute(TBufferHandleId argsBufferHandle, U32 alignedOffset) override;

			/*!
				\brief The method binds a given blend state to rendering pipeline

				\param[in] blendStateId An identifier of a blend state
			*/

			void BindBlendState(TBlendStateId blendStateId) override;

			/*!
				\brief The method binds a given depth-stencil state to rendering pipeline

				\param[in] depthStencilStateId An identifier of a depth-stencil state
				\param[in] stencilRef A reference value to perform against when doing a depth-stencil test
			*/

			void BindDepthStencilState(TDepthStencilStateId depthStencilStateId, U8 stencilRef = 0x0) override;

			/*!
				\brief The method binds a given rasteriazer state to rendering pipeline

				\param[in] rasterizerStateId An identifier of a rasterizer state
			*/

			void BindRasterizerState(TRasterizerStateId rasterizerStateId) override;

			/*!
				\brief The method binds a given render target object to rendering pipeline

				\param[in] slot An index of the slot into which the render target will be bound

				\param[in] targetHandle Handle to texture object that's created as a render target
			*/

			void BindRenderTarget(U8 slot, TTextureHandleId targetHandle) override;

			/*!
				\brief The method binds a given depth buffer to rendering pipeline

				\param[in] targetHandle Handle to texture object that's created as a depth buffer
				\param[in] disableRTWrite A flag determines whether the write to RT should be enabled or not
			*/

			void BindDepthBufferTarget(TTextureHandleId targetHandle, bool disableRTWrite = false) override;

			/*!
				\brief The method disables or enables a depth buffer usage

				\param[in] value If true the depth buffer will be used, false turns off it
			*/

			void SetDepthBufferEnabled(bool value) override;

			/*!
				\brief The method returns an object that contains internal handlers that are used by the system.

				The structure of the object and its members can vary on different platforms.

				return The method returns an object that contains internal handlers that are used by the system
			*/

			const TGraphicsCtxInternalData& GetInternalData() const override;

			/*!
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			E_ENGINE_SUBSYSTEM_TYPE GetType() const override;

			/*!
				\brief The method returns a pointer to IGraphicsObjectManager implementation

				\return The method returns a pointer to IGraphicsObjectManager implementation
			*/

			IGraphicsObjectManager* GetGraphicsObjectManager() const override;

			/*!
				\brief The method returns 1 if a Z axis is collinear to (0; 0; 1) direction, -1 in other case

				\return The method returns 1 if a Z axis is collinear to (0; 0; 1) direction, -1 in other case
			*/

			F32 GetPositiveZAxisDirection() const override;

			/*!
				\brief The method receives a given event and processes it

				\param[in] pEvent A pointer to event data

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			E_RESULT_CODE OnEvent(const TBaseEvent* pEvent);

	#if TDE2_DEBUG_MODE
			void BeginSectionMarker(const std::string& id) override;
			void EndSectionMarker() override;
	#endif

			/*!
				\brief The method returns an identifier of a listener

				\return The method returns an identifier of a listener
			*/

			TEventListenerId GetListenerId() const;

			/*!
				\brief The method returns an information about currently using video adapter

				\return The method returns an information about currently using video adapter
			*/

			TVideoAdapterInfo GetInfo() const override;

			/*
				\bief The method returns information about GAPI environment

				\return The method returns data that describes all GAPI related information
			*/

			const TGraphicsContextInfo& GetContextInfo() const override;

			/*!
				\return The method returns a pointer to IWindowSystem
			*/

			TPtr<IWindowSystem> GetWindowSystem() const override;

			std::vector<U8> GetBackBufferData() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D12GraphicsContext)

			E_RESULT_CODE _initDescriptorHeaps();

			E_RESULT_CODE _onFreeInternal() override;

			void _waitForIdle();
		protected:
			TDE2_STATIC_CONSTEXPR      U32 BACK_BUFFERS_COUNT = 2;
	//		constexpr static U8      mMaxNumOfRenderTargets = D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;
	//
	//		ID3D12Device* mp3dDevice;
	//		ID3D12DeviceContext* mp3dDeviceContext;
	//		TGraphicsCtxInternalData mInternalDataObject;
	//
	//
	//		ID3D12RenderTargetView* mpBackBufferView;
	//
	//		ID3D12DepthStencilView* mpCurrDepthStencilView;
	//		ID3D12DepthStencilView* mpDefaultDepthStencilView;
	//		ID3D12DepthStencilView* mpPrevDepthStencilView;
	//		ID3D12Texture2D* mpDefaultDepthStencilBuffer;
	//
	//		D3D_FEATURE_LEVEL        mCurrFeatureLevel;
	//#if _DEBUG
	//		ID3D12Debug* mpDebuggerInstance;
	//#endif
	//
	//		bool                     mIsVSyncEnabled;
	//
	//		DXGI_FORMAT              mCurrBackBufferFormat;
	//
	//		TPtr<IGraphicsObjectManager> mpGraphicsObjectManager = nullptr;
	//		CD3D12GraphicsObjectManager* mpGraphicsObjectManagerD3D12Impl = nullptr;
	//
			TPtr<IWindowSystem>               mpWindowSystem = nullptr;
			TPtr<IEventManager>               mpEventManager = nullptr;

			D3D_FEATURE_LEVEL                 mCurrFeatureLevel = D3D_FEATURE_LEVEL_12_2;
			ComPtr<ID3D12Device>              mp3dDevice = nullptr;
			ComPtr<IDXGIAdapter1>             mpAdapter = nullptr;

			ComPtr<IDXGISwapChain3>           mpSwapChain = nullptr;
			U32                               mCurrBackBufferIndex = 0;
			ComPtr<ID3D12Resource>            mpRenderTargetViews[BACK_BUFFERS_COUNT]{ nullptr };

			ComPtr<ID3D12CommandQueue>        mpCommandQueue = nullptr;
			ComPtr<ID3D12CommandAllocator>    mpCommandAllocator = nullptr;
			ComPtr<ID3D12GraphicsCommandList> mpCommandList = nullptr;

#if TDE2_DEBUG_MODE
			ComPtr<ID3D12Debug1>              mpDebugController = nullptr;
			ComPtr<ID3D12DebugDevice>         mpDebugDevice = nullptr;
#endif

			bool                              mIsVSyncEnabled = false;

			ComPtr<ID3D12Fence>               mpFrameFence = nullptr;
			UINT64                            mCurrentFence = 0;
			HANDLE                            mFenceEventHandle;

			// heaps
			ComPtr<ID3D12DescriptorHeap>      mpRenderTargetViewsHeap = nullptr;
			U32                               mRTVDescriptorSize = 0;

			ComPtr<D3D12MA::Allocator>        mpMemoryAllocator = nullptr;

	// 
	//		ID3D12RenderTargetView* mpRenderTargets[mMaxNumOfRenderTargets];
	//		U8                      mCurrNumOfActiveRenderTargets = 0;
	};


	CD3D12GraphicsContext::CD3D12GraphicsContext() :
		CBaseObject()
	{
	}


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

		D3D12_COMMAND_QUEUE_DESC commandQueueCreateDesc {};
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


	static TResult<ComPtr<IDXGISwapChain3>> CreateSwapChain(ComPtr<IDXGIFactory4> pFactory, ComPtr<ID3D12CommandQueue> pCommandQueue, TPtr<IWindowSystem> pWindowSystem, U32 backBuffersCount)
	{
		ComPtr<IDXGISwapChain1> pSwapChain = nullptr;

		HWND hwnd = pWindowSystem->GetInternalData().mWindowHandler;

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
		swapChainDesc.Width              = pWindowSystem->GetWidth();
		swapChainDesc.Height             = pWindowSystem->GetHeight();
		swapChainDesc.BufferCount        = backBuffersCount;
		swapChainDesc.SampleDesc.Count   = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.Format             = (pWindowSystem->GetFlags() & P_HARDWARE_GAMMA_CORRECTION) ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_DISCARD;

		if (FAILED(pFactory->CreateSwapChainForHwnd(pCommandQueue.Get(), hwnd, &swapChainDesc, nullptr, nullptr, pSwapChain.GetAddressOf())))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		pFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

		ComPtr<IDXGISwapChain3> pResultSwapChain = nullptr;
		pSwapChain.As(&pResultSwapChain);

		return Wrench::TOkValue<ComPtr<IDXGISwapChain3>>(pResultSwapChain);
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

		ComPtr<IDXGIFactory4> pFactory = nullptr;

		UINT dxgiFactoryFlags = 0;

#if TDE2_DEBUG_MODE
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

		if (FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&pFactory))))
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

		auto createDeviceResult = InitPhysicalDevice(pFactory, mCurrFeatureLevel);
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

		auto createCommandAllocatorResult = CreateCommandAllocator(mp3dDevice);
		if (createCommandAllocatorResult.HasError())
		{
			return createCommandAllocatorResult.GetError();
		}

		mpCommandAllocator = createCommandAllocatorResult.Get();

		auto createCommandListResult = CreateCommandList(mp3dDevice, mpCommandAllocator);
		if (createCommandListResult.HasError())
		{
			return createCommandListResult.GetError();
		}

		mpCommandList = createCommandListResult.Get();

		auto createSwapchainResult = CreateSwapChain(pFactory, mpCommandQueue, mpWindowSystem, BACK_BUFFERS_COUNT);
		if (createSwapchainResult.HasError())
		{
			return createSwapchainResult.GetError();
		}

		mpSwapChain = createSwapchainResult.Get();
		mCurrBackBufferIndex = mpSwapChain->GetCurrentBackBufferIndex();

		E_RESULT_CODE result = _initDescriptorHeaps();
		if (RC_OK != result)
		{
			return result;
		}

		mIsVSyncEnabled = mpWindowSystem->GetFlags() & P_VSYNC;

		if (FAILED(mp3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mpFrameFence))))
		{
			return RC_FAIL;
		}

		mFenceEventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (!mFenceEventHandle)
		{
			return RC_FAIL;
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
//#if _HAS_CXX17
//		mInternalDataObject = TD3D12CtxInternalData{ mp3dDevice, mp3dDeviceContext };
//#else
//		mInternalDataObject.mD3D12 = { mp3dDevice, mp3dDeviceContext };
//#endif
//
//		mpGraphicsObjectManager = TPtr<IGraphicsObjectManager>(CreateD3D12GraphicsObjectManager(this, result));
//		mpGraphicsObjectManagerD3D12Impl = dynamic_cast<CD3D12GraphicsObjectManager*>(mpGraphicsObjectManager.Get());

		/*if (result != RC_OK)
		{
			return result;
		}*/

		mpEventManager = pWindowSystem->GetEventManager();

		if (!mpEventManager)
		{
			return RC_FAIL;
		}

		mpEventManager->Subscribe(TOnWindowResized::GetTypeId(), this);

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsContext::_initDescriptorHeaps()
	{
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc {};
		rtvHeapDesc.NumDescriptors = BACK_BUFFERS_COUNT;
		rtvHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		if (FAILED(mp3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mpRenderTargetViewsHeap))))
		{
			return RC_FAIL;
		}

		mRTVDescriptorSize = mp3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		D3D12_CPU_DESCRIPTOR_HANDLE	rtvHandle(mpRenderTargetViewsHeap->GetCPUDescriptorHandleForHeapStart());

		for (U32 i = 0; i < BACK_BUFFERS_COUNT; ++i)
		{
			if (FAILED(mpSwapChain->GetBuffer(i, IID_PPV_ARGS(&mpRenderTargetViews[i]))))
			{
				return RC_FAIL;
			}

			mp3dDevice->CreateRenderTargetView(mpRenderTargetViews[i].Get(), nullptr, rtvHandle);
			rtvHandle.ptr += mRTVDescriptorSize;
		}

		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsContext::_onFreeInternal()
	{
		TDE2_PROFILER_SCOPE("CD3D12GraphicsContext::_onFreeInternal");

		_waitForIdle();
		CloseHandle(mFenceEventHandle);

		return RC_OK;
	}

	void CD3D12GraphicsContext::_waitForIdle()
	{
		TDE2_PROFILER_SCOPE("CD3D12GraphicsContext::_waitForIdle");
		
		const UINT64 fence = mCurrentFence;
		HRESULT result = mpCommandQueue->Signal(mpFrameFence.Get(), fence);
		TDE2_ASSERT(SUCCEEDED(result));

		++mCurrentFence;

		if (mpFrameFence->GetCompletedValue() < fence)
		{
			result = mpFrameFence->SetEventOnCompletion(fence, mFenceEventHandle);
			TDE2_ASSERT(SUCCEEDED(result));

			WaitForSingleObject(mFenceEventHandle, INFINITE);
		}

		mCurrBackBufferIndex = mpSwapChain->GetCurrentBackBufferIndex();
	}

	void CD3D12GraphicsContext::ClearBackBuffer(const TColor32F& color)
	{
		const F32 clearColorArray[4]{ color.r, color.g, color.b, color.a };

		CD3DX12_RESOURCE_BARRIER rtTransitionBarrier = CD3DX12_RESOURCE_BARRIER::Transition(mpRenderTargetViews[mCurrBackBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		mpCommandList->ResourceBarrier(1, &rtTransitionBarrier);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mpRenderTargetViewsHeap->GetCPUDescriptorHandleForHeapStart(), mCurrBackBufferIndex, mRTVDescriptorSize);

		mpCommandList->ClearRenderTargetView(rtvHandle, clearColorArray, 0, nullptr);
	}

	void CD3D12GraphicsContext::ClearRenderTarget(U8 slot, const TColor32F& color)
	{
		//m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		//CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);

		//// Record commands.
		//const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		//m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	}

	void CD3D12GraphicsContext::ClearDepthBuffer(F32 value)
	{
	}

	void CD3D12GraphicsContext::ClearStencilBuffer(U8 value)
	{
	}

	void CD3D12GraphicsContext::BeginFrame()
	{
		HRESULT result = mpCommandAllocator->Reset();
		TDE2_ASSERT(SUCCEEDED(result));

		result = mpCommandList->Reset(mpCommandAllocator.Get(), nullptr);
	}

	void CD3D12GraphicsContext::Present()
	{
		TDE2_PROFILER_SCOPE("CD3D12GraphicsContext::Present");
		
		CD3DX12_RESOURCE_BARRIER rtTransitionBarrier = CD3DX12_RESOURCE_BARRIER::Transition(mpRenderTargetViews[mCurrBackBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		mpCommandList->ResourceBarrier(1, &rtTransitionBarrier);
		mpCommandList->Close();

		ID3D12CommandList* ppCommandLists[] = { mpCommandList.Get() };
		mpCommandQueue->ExecuteCommandLists(1, ppCommandLists);

		mpSwapChain->Present(static_cast<UINT>(mIsVSyncEnabled), 0);
		_waitForIdle();
	}

	void CD3D12GraphicsContext::SetViewport(F32 x, F32 y, F32 width, F32 height, F32 minDepth, F32 maxDepth)
	{
	}

	void CD3D12GraphicsContext::SetScissorRect(const TRectU32& scissorRect)
	{
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
		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsContext::SetIndexBuffer(TBufferHandleId indexBufferHandle, U32 offset)
	{
		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsContext::SetConstantBuffer(U32 slot, TBufferHandleId constantsBufferHandle)
	{
		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsContext::SetStructuredBuffer(U32 slot, TBufferHandleId bufferHandle, bool isWriteEnabled)
	{
		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsContext::SetTexture(U32 slot, TTextureHandleId textureHandle, bool isWriteEnabled)
	{
		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsContext::SetSampler(U32 slot, TTextureSamplerId samplerHandle)
	{
		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsContext::UpdateTexture2D(TTextureHandleId textureHandle, U32 mipLevel, const TRectI32& regionRect, const void* pData, USIZE dataSize)
	{
		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsContext::UpdateTexture2DArray(TTextureHandleId textureHandle, U32 index, const TRectI32& regionRect, const void* pData, USIZE dataSize)
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CD3D12GraphicsContext::UpdateCubemapTexture(TTextureHandleId textureHandle, E_CUBEMAP_FACE face, const TRectI32& regionRect, const void* pData, USIZE dataSize)
	{
		return RC_NOT_IMPLEMENTED_YET;
	}

	E_RESULT_CODE CD3D12GraphicsContext::UpdateTexture3D(TTextureHandleId textureHandle, U32 depthFrom, U32 depthTo, const TRectI32& regionRect, const void* pData, USIZE dataSize)
	{
		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsContext::CopyResource(TTextureHandleId sourceHandle, TTextureHandleId destHandle)
	{
		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsContext::CopyResource(TBufferHandleId sourceHandle, TTextureHandleId destHandle)
	{
		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsContext::CopyResource(TTextureHandleId sourceHandle, TBufferHandleId destHandle)
	{
		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsContext::CopyResource(TBufferHandleId sourceHandle, TBufferHandleId destHandle)
	{
		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsContext::CopyCount(TBufferHandleId sourceHandle, TBufferHandleId destHandle, U32 offset)
	{
		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsContext::GenerateMipMaps(TTextureHandleId textureHandle)
	{
		return RC_OK;
	}

	void CD3D12GraphicsContext::Draw(E_PRIMITIVE_TOPOLOGY_TYPE topology, U32 startVertex, U32 numOfVertices)
	{
	}

	void CD3D12GraphicsContext::DrawIndexed(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, U32 baseVertex, U32 startIndex, U32 numOfIndices)
	{
	}

	void CD3D12GraphicsContext::DrawInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, U32 startVertex, U32 verticesPerInstance, U32 startInstance, U32 numOfInstances)
	{
	}

	void CD3D12GraphicsContext::DrawIndexedInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, U32 baseVertex, U32 startIndex,
		U32 startInstance, U32 indicesPerInstance, U32 numOfInstances)
	{
	}

	void CD3D12GraphicsContext::DrawIndirectInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, TBufferHandleId argsBufferHandle, U32 alignedOffset)
	{
	}

	void CD3D12GraphicsContext::DrawIndirectIndexedInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, TBufferHandleId argsBufferHandle, U32 alignedOffset)
	{
	}

	void CD3D12GraphicsContext::DispatchCompute(U32 groupsCountX, U32 groupsCountY, U32 groupsCountZ)
	{
	}

	void CD3D12GraphicsContext::DispatchIndirectCompute(TBufferHandleId argsBufferHandle, U32 alignedOffset)
	{
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

	void CD3D12GraphicsContext::BindRenderTarget(U8 slot, TTextureHandleId targetHandle)
	{
	}

	void CD3D12GraphicsContext::BindDepthBufferTarget(TTextureHandleId targetHandle, bool disableRTWrite)
	{
	}

	void CD3D12GraphicsContext::SetDepthBufferEnabled(bool value)
	{
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
		return nullptr;
		//return mpGraphicsObjectManagerD3D12Impl;
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
		mpCommandList->BeginEvent(1, id.c_str(), static_cast<UINT>(id.length()));
	}

	void CD3D12GraphicsContext::EndSectionMarker()
	{
		mpCommandList->EndEvent();
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
		return nullptr;
		//return mpWindowSystem;
	}

	std::vector<U8> CD3D12GraphicsContext::GetBackBufferData() const
	{
		std::vector<U8> backBufferData;

		return std::move(backBufferData);
	}


	IGraphicsContext* CreateD3D12GraphicsContext(TPtr<IWindowSystem> pWindowSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IGraphicsContext, CD3D12GraphicsContext, result, pWindowSystem);
	}
}

#endif