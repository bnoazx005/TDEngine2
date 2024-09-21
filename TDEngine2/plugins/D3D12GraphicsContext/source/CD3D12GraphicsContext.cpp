#include "../include/CD3D12GraphicsContext.h"
#include <core/IGraphicsContext.h>
#include <core/IEventManager.h>
#include <core/CBaseObject.h>
#include <core/IWindowSystem.h>
#include <editor/CPerfProfiler.h>


#if defined(TDE2_USE_WINPLATFORM) /// Used only on Windows platform

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

				\param[in, out] pRenderTarget A pointer to IRenderTarget implementation
				\param[in] color The new color of a render target
			*/

			void ClearRenderTarget(IRenderTarget* pRenderTarget, const TColor32F& color) override;

			/*!
				\brief The method clears up render target with specified color

				\param[in] slot A slot into which the render target that should be cleared up is bound
				\param[in] color The new color of a render target
			*/

			void ClearRenderTarget(U8 slot, const TColor32F& color) override;

			/*!
				\brief The method clears up given depth buffer with specified values

				\param[in] value The depth buffer will be cleared with this value
				\param[in] stencilValue The stencil buffer will be cleared with this value
			*/

			void ClearDepthBufferTarget(IDepthBufferTarget* pDepthBufferTarget, F32 value, U8 stencilValue) override;

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

				\param[in, out] pRenderTarget A pointer to IRenderTarget implementation
			*/

			void BindRenderTarget(U8 slot, IRenderTarget* pRenderTarget) override;

			/*!
				\brief The method binds a given render target object to rendering pipeline

				\param[in] slot An index of the slot into which the render target will be bound

				\param[in] targetHandle Handle to texture object that's created as a render target
			*/

			void BindRenderTarget(U8 slot, TTextureHandleId targetHandle) override;

			/*!
				\brief The method binds a given depth buffer to rendering pipeline

				\param[in, out] pDepthBufferTarget A pointer to IDepthBufferTarget implementation
				\param[in] disableRTWrite A flag determines whether the write to RT should be enabled or not
			*/

			void BindDepthBufferTarget(IDepthBufferTarget* pDepthBufferTarget, bool disableRTWrite = false) override;

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

			/*IDXGIAdapter* _getDXGIAdapter(ID3D12Device* p3dDevice) const;

			E_RESULT_CODE _createSwapChain(const IWindowSystem* pWindowSystem, ID3D12Device* p3dDevice);

			E_RESULT_CODE _createBackBuffer(IDXGISwapChain* pSwapChain, ID3D12Device* p3dDevice);

			E_RESULT_CODE _createDepthBuffer(U32 width, U32 height, IDXGISwapChain* pSwapChain, ID3D12Device* p3dDevice,
				ID3D12DepthStencilView** ppDepthStencilView, ID3D12Texture2D** pDepthStencilBuffer);*/

			E_RESULT_CODE _onFreeInternal() override;
		protected:
	//		constexpr static U8      mMaxNumOfRenderTargets = D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;
	//
	//		ID3D12Device* mp3dDevice;
	//		ID3D12DeviceContext* mp3dDeviceContext;
	//		TGraphicsCtxInternalData mInternalDataObject;
	//
	//		IDXGISwapChain* mpSwapChain;
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
	//		TPtr<IWindowSystem>      mpWindowSystem;
	//		TPtr<IEventManager>      mpEventManager;
	//
	//		ID3D12RenderTargetView* mpRenderTargets[mMaxNumOfRenderTargets];
	//		U8                      mCurrNumOfActiveRenderTargets = 0;
	};


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

		//mpWindowSystem = pWindowSystem;
//
//		const D3D_FEATURE_LEVEL featureLevels[] =
//		{
//			D3D_FEATURE_LEVEL_11_1,
//			D3D_FEATURE_LEVEL_11_0,
//			D3D_FEATURE_LEVEL_10_1,
//			D3D_FEATURE_LEVEL_10_0
//		};
//
//		const U32 numOfFeatureLevels = sizeof(featureLevels) / sizeof(featureLevels[0]);
//
//		HRESULT d3dCallsResult = S_OK;
//
//		UINT flags = 0x0;
//
//#if defined(_DEBUG)
//		flags |= D3D12_CREATE_DEVICE_DEBUG;
//#endif
//		{
//			TDE2_PROFILER_SCOPE("D3D12CreateDevice");
//
//			d3dCallsResult = D3D12CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels, numOfFeatureLevels,
//				D3D12_SDK_VERSION, &mp3dDevice, &mCurrFeatureLevel, &mp3dDeviceContext);
//		}
//
//		if (FAILED(d3dCallsResult))
//		{
//			return RC_FAIL;
//		}
//
//		if (mCurrFeatureLevel < D3D_FEATURE_LEVEL_11_0)
//		{
//			return RC_GAPI_IS_NOT_SUPPORTED;
//		}
//
//#if TDE2_DEBUG_MODE/// Acquiring ID3D12Debug interface
//		mp3dDevice->QueryInterface(__uuidof(ID3D12Debug), reinterpret_cast<void**>(&mpDebuggerInstance));
//		mp3dDeviceContext->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), reinterpret_cast<void**>(&pAnnotation));
//#endif
//
//		/// create a swap chain
//		E_RESULT_CODE result = _createSwapChain(pWindowSystem.Get(), mp3dDevice);
//
//		if (result != RC_OK)
//		{
//			return result;
//		}
//
//		/// create a back buffer
//		if ((result = _createBackBuffer(mpSwapChain, mp3dDevice)) != RC_OK)
//		{
//			return result;
//		}
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
		}

		mpEventManager = pWindowSystem->GetEventManager();

		if (!mpEventManager)
		{
			return RC_FAIL;
		}

		mpEventManager->Subscribe(TOnWindowResized::GetTypeId(), this);*/

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CD3D12GraphicsContext::_onFreeInternal()
	{
		TDE2_PROFILER_SCOPE("CD3D12GraphicsContext::_onFreeInternal");

		E_RESULT_CODE result = RC_OK;

		return result;
	}

	void CD3D12GraphicsContext::ClearBackBuffer(const TColor32F& color)
	{
	}

	void CD3D12GraphicsContext::ClearRenderTarget(IRenderTarget* pRenderTarget, const TColor32F& color)
	{
	}

	void CD3D12GraphicsContext::ClearRenderTarget(U8 slot, const TColor32F& color)
	{
	}

	void CD3D12GraphicsContext::ClearDepthBufferTarget(IDepthBufferTarget* pDepthBufferTarget, F32 value, U8 stencilValue)
	{
	}

	void CD3D12GraphicsContext::ClearDepthBuffer(F32 value)
	{
	}

	void CD3D12GraphicsContext::ClearStencilBuffer(U8 value)
	{
	}

	void CD3D12GraphicsContext::BeginFrame()
	{
	}

	void CD3D12GraphicsContext::Present()
	{
		TDE2_PROFILER_SCOPE("CD3D12GraphicsContext::Present");
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

	void CD3D12GraphicsContext::BindRenderTarget(U8 slot, IRenderTarget* pRenderTarget)
	{
	}

	void CD3D12GraphicsContext::BindRenderTarget(U8 slot, TTextureHandleId targetHandle)
	{
	}

	void CD3D12GraphicsContext::BindDepthBufferTarget(IDepthBufferTarget* pDepthBufferTarget, bool disableRTWrite)
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
	}

	void CD3D12GraphicsContext::EndSectionMarker()
	{
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