#include "../include/CD3D11GraphicsContext.h"
#include "../include/CD3D11Utils.h"
#include "../include/CD3D11GraphicsObjectManager.h"
#include "../include/CD3D11Mappings.h"
#include "../include/CD3D11RenderTarget.h"
#include "../include/CD3D11DepthBufferTarget.h"
#include "../include/CD3D11Buffer.h"
#include "../include/CD3D11Texture.h"
#include "stringUtils.hpp"
#define DEFER_IMPLEMENTATION
#include "deferOperation.hpp"
#include <utils/CFileLogger.h>
#include <utils/Utils.h>
#include <core/IEventManager.h>
#include <core/IWindowSystem.h>
#include <editor/CPerfProfiler.h>
#include <unordered_map>
#include <d3d11_1.h>


#if defined(TDE2_USE_WINPLATFORM)

#pragma comment(lib, "d3d11.lib")
#if TDE2_DEBUG_MODE
	#pragma comment(lib, "dxguid.lib") 
#endif


namespace TDEngine2
{
#if TDE2_DEBUG_MODE
	static ID3DUserDefinedAnnotation* pAnnotation = nullptr;
#endif


	class CD3D11GraphicsContext : public IGraphicsContext, public IEventHandler, public CBaseObject
	{
		public:
			friend IGraphicsContext* CreateD3D11GraphicsContext(TPtr<IWindowSystem>, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_TYPE(CD3D11GraphicsContext)

			/*!
				\brief The method initializes an initial state of the object

				\param[in] pWindowSystem A pointer to implementation of IWindowSystem interface

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(TPtr<IWindowSystem> pWindowSystem) override;

			/*!
				\brief The method clears up back buffer with specified color

				\param[in] color The new color of a back buffer
			*/

			TDE2_API void ClearBackBuffer(const TColor32F& color) override;

			/*!
				\brief The method clears up render target with specified color

				\param[in, out] pRenderTarget A pointer to IRenderTarget implementation
				\param[in] color The new color of a render target
			*/

			TDE2_API void ClearRenderTarget(IRenderTarget* pRenderTarget, const TColor32F& color) override;

			/*!
				\brief The method clears up render target with specified color

				\param[in] slot A slot into which the render target that should be cleared up is bound
				\param[in] color The new color of a render target
			*/

			TDE2_API void ClearRenderTarget(U8 slot, const TColor32F& color) override;

			/*!
				\brief The method clears up given depth buffer with specified values

				\param[in] value The depth buffer will be cleared with this value
				\param[in] stencilValue The stencil buffer will be cleared with this value
			*/

			TDE2_API void ClearDepthBufferTarget(IDepthBufferTarget* pDepthBufferTarget, F32 value, U8 stencilValue) override;

			/*!
				\brief The method clears up depth buffer with specified values

				\param[in] value The depth buffer will be cleared with this value
			*/

			TDE2_API void ClearDepthBuffer(F32 value) override;

			/*!
				\brief The method clears up stencil buffer with specified values

				\param[in] value The stencil buffer will be cleared with this value
			*/

			TDE2_API void ClearStencilBuffer(U8 value) override;

			TDE2_API void BeginFrame() override;

			/*!
				\brief The method swaps a current buffer with a back one
			*/

			TDE2_API void Present() override;

			/*!
				\brief The method sets up a viewport's parameters

				\param[in] x x position of left hand side of a viewport
				\param[in] y y position of a left top corner of a viewport
				\param[in] width width of a viewport
				\param[in] height height of a viewport
				\param[in] minDepth minimum depth of a viewport
				\param[in] maxDepth maximum depth of a viewport
			*/

			TDE2_API void SetViewport(F32 x, F32 y, F32 width, F32 height, F32 minDepth, F32 maxDepth) override;

			/*!
				\brief The method specifies rectangle for scissor testing, all the geometry outside of which
				is rejected

				\param[in] scissorRect A rectangle for scissor test
			*/

			TDE2_API void SetScissorRect(const TRectU32& scissorRect) override;

			/*!
				\brief The method computes a perspective projection matrix specific for a graphics context

				\param[in] fov A field of view

				\param[in] aspect An aspect ratio of a screen

				\param[in] zn A z value of a near clip plance

				\param[in] zf A z value of a far clip plane

				\return The method computes a perspective projection matrix specific for a graphics context
			*/

			TDE2_API TMatrix4 CalcPerspectiveMatrix(F32 fov, F32 aspect, F32 zn, F32 zf) override;

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

			TDE2_API TMatrix4 CalcOrthographicMatrix(F32 left, F32 top, F32 right, F32 bottom, F32 zn, F32 zf, bool isDepthless = false) override;

			TDE2_API E_RESULT_CODE SetVertexBuffer(U32 slot, TBufferHandleId vertexBufferHandle, U32 offset, U32 strideSize) override;
			TDE2_API E_RESULT_CODE SetIndexBuffer(TBufferHandleId indexBufferHandle, U32 offset) override;
			TDE2_API E_RESULT_CODE SetConstantBuffer(U32 slot, TBufferHandleId constantsBufferHandle) override;
			TDE2_API E_RESULT_CODE SetStructuredBuffer(U32 slot, TBufferHandleId bufferHandle, bool isWriteEnabled = false) override;

			TDE2_API E_RESULT_CODE SetTexture(U32 slot, TTextureHandleId textureHandle, bool isWriteEnabled = false) override;
			TDE2_API E_RESULT_CODE SetSampler(U32 slot, TTextureSamplerId samplerHandle) override;

			TDE2_API E_RESULT_CODE UpdateTexture2D(TTextureHandleId textureHandle, U32 mipLevel, const TRectI32& regionRect, const void* pData, USIZE dataSize) override;
			TDE2_API E_RESULT_CODE UpdateTexture2DArray(TTextureHandleId textureHandle, U32 index, const TRectI32& regionRect, const void* pData, USIZE dataSize) override;
			TDE2_API E_RESULT_CODE UpdateCubemapTexture(TTextureHandleId textureHandle, E_CUBEMAP_FACE face, const TRectI32& regionRect, const void* pData, USIZE dataSize) override;
			TDE2_API E_RESULT_CODE UpdateTexture3D(TTextureHandleId textureHandle, U32 depthFrom, U32 depthTo, const TRectI32& regionRect, const void* pData, USIZE dataSize) override;

			// Blit one texture to another
			TDE2_API E_RESULT_CODE CopyResource(TTextureHandleId sourceHandle, TTextureHandleId destHandle) override;
			// Upload data into texture
			TDE2_API E_RESULT_CODE CopyResource(TBufferHandleId sourceHandle, TTextureHandleId destHandle) override;
			// Readback data from texture into a buffer
			TDE2_API E_RESULT_CODE CopyResource(TTextureHandleId sourceHandle, TBufferHandleId destHandle) override;
			// Copy buffer to buffer
			TDE2_API E_RESULT_CODE CopyResource(TBufferHandleId sourceHandle, TBufferHandleId destHandle) override;

			/*!
				\brief The method copies counter of sourceHandle buffer into destHandle's one

				\param[in] sourceHandle A handle to structured appendable buffer resource that should be created with unordered access write flag
				\param[in] destHandle A handle to any buffer resource
				\param[in] offset An offset within dest buffer
			*/

			TDE2_API E_RESULT_CODE CopyCount(TBufferHandleId sourceHandle, TBufferHandleId destHandle, U32 offset) override;

			/*!
				\brief The method generates all mip levels for the specified texture
			*/

			TDE2_API E_RESULT_CODE GenerateMipMaps(TTextureHandleId textureHandle) override;

			/*!
				\brief The method sends a command to a GPU to draw current attached graphics data

				\param[in] topology A primitive topology's type

				\param[in] startVertex An index of a vertex within a vertex buffer

				\param[in] numOfVertices A total number of vertices should be drawn
			*/

			TDE2_API void Draw(E_PRIMITIVE_TOPOLOGY_TYPE topology, U32 startVertex, U32 numOfVertices) override;

			/*!
				\brief The method sends a command to a GPU to draw current attached graphics data using
				index buffer to group primitives

				\param[in] topology A primitive topology's type

				\param[in] indexFormatType A format of a single index

				\param[in] baseVertex A value that will be added to each index

				\param[in] startIndex A first index that will be read by GPU

				\param[in] numOfIndices A total number of indices
			*/

			TDE2_API void DrawIndexed(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, U32 baseVertex, U32 startIndex, U32 numOfIndices) override;

			/*!
				\brief The method sends a command to a GPU to draw current attached graphics data that uses instancing

				\param[in] topology A primitive topology's type

				\param[in] startVertex An index of a vertex within a vertex buffer

				\param[in] verticesPerInstance A number of vertices per instance

				\param[in] startInstance An offset value which is added to every index before read the data

				\param[in] numOfInstances A total number of instances should be drawn
			*/

			TDE2_API void DrawInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, U32 startVertex, U32 verticesPerInstance, U32 startInstance, U32 numOfInstances) override;

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

			TDE2_API void DrawIndexedInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, U32 baseVertex, U32 startIndex,
				U32 startInstance, U32 indicesPerInstance, U32 numOfInstances) override;

			/*!
				\brief The method sends a command to a GPU with enabled instancing but all the parameters are stored within GPU buffer

				\param[in] topology A primitive topology's type
				\param[in] argsBufferHandle A GPU buffer that holds arguments for draw command
				\param[in] alignedOffset Aligned offset to the beginning of GPU data
			*/

			TDE2_API void DrawIndirectInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, TBufferHandleId argsBufferHandle, U32 alignedOffset) override;

			/*!
				\brief The method sends a command to a GPU with enabled instancing but all the parameters are stored within GPU buffer

				\param[in] topology A primitive topology's type
				\param[in] indexFormatType A format of a single index
				\param[in] argsBufferHandle A GPU buffer that holds arguments for draw command
				\param[in] alignedOffset Aligned offset to the beginning of GPU data
			*/

			TDE2_API void DrawIndirectIndexedInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, TBufferHandleId argsBufferHandle, U32 alignedOffset) override;

			/*!
				\brief The method invokes a computation kernel over a given threads grid. A compute shader should be bound to the device first

				\param[in] groupsCountX A number of groups over X axis
				\param[in] groupsCountY A number of groups over Y axis
				\param[in] groupsCountZ A number of groups over Z axis
			*/

			TDE2_API void DispatchCompute(U32 groupsCountX, U32 groupsCountY, U32 groupsCountZ) override;

			/*!
				\brief The method invokes a computation kernel over a given threads grid. A compute shader should be bound to the device first

				\param[in] argsBufferHandle A GPU buffer that holds arguments for draw command
				\param[in] alignedOffset Aligned offset to the beginning of GPU data
			*/

			TDE2_API void DispatchIndirectCompute(TBufferHandleId argsBufferHandle, U32 alignedOffset) override;

			/*!
				\brief The method binds a given blend state to rendering pipeline

				\param[in] blendStateId An identifier of a blend state
			*/

			TDE2_API void BindBlendState(TBlendStateId blendStateId) override;

			/*!
				\brief The method binds a given depth-stencil state to rendering pipeline

				\param[in] depthStencilStateId An identifier of a depth-stencil state
				\param[in] stencilRef A reference value to perform against when doing a depth-stencil test
			*/

			TDE2_API void BindDepthStencilState(TDepthStencilStateId depthStencilStateId, U8 stencilRef = 0x0) override;

			/*!
				\brief The method binds a given rasteriazer state to rendering pipeline

				\param[in] rasterizerStateId An identifier of a rasterizer state
			*/

			TDE2_API void BindRasterizerState(TRasterizerStateId rasterizerStateId) override;

			/*!
				\brief The method binds a given render target object to rendering pipeline

				\param[in] slot An index of the slot into which the render target will be bound

				\param[in, out] pRenderTarget A pointer to IRenderTarget implementation
			*/

			TDE2_API void BindRenderTarget(U8 slot, IRenderTarget* pRenderTarget) override;

			/*!
				\brief The method binds a given render target object to rendering pipeline

				\param[in] slot An index of the slot into which the render target will be bound

				\param[in] targetHandle Handle to texture object that's created as a render target
			*/

			TDE2_API void BindRenderTarget(U8 slot, TTextureHandleId targetHandle) override;

			/*!
				\brief The method binds a given depth buffer to rendering pipeline

				\param[in, out] pDepthBufferTarget A pointer to IDepthBufferTarget implementation
				\param[in] disableRTWrite A flag determines whether the write to RT should be enabled or not
			*/

			TDE2_API void BindDepthBufferTarget(IDepthBufferTarget* pDepthBufferTarget, bool disableRTWrite = false) override;

			/*!
				\brief The method binds a given depth buffer to rendering pipeline

				\param[in] targetHandle Handle to texture object that's created as a depth buffer
				\param[in] disableRTWrite A flag determines whether the write to RT should be enabled or not
			*/

			TDE2_API void BindDepthBufferTarget(TTextureHandleId targetHandle, bool disableRTWrite = false) override;

			/*!
				\brief The method disables or enables a depth buffer usage

				\param[in] value If true the depth buffer will be used, false turns off it
			*/

			TDE2_API void SetDepthBufferEnabled(bool value) override;

			/*!
				\brief The method returns an object that contains internal handlers that are used by the system.

				The structure of the object and its members can vary on different platforms.

				return The method returns an object that contains internal handlers that are used by the system
			*/

			TDE2_API const TGraphicsCtxInternalData& GetInternalData() const override;

			/*!
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			TDE2_API E_ENGINE_SUBSYSTEM_TYPE GetType() const override;

			/*!
				\brief The method returns a pointer to IGraphicsObjectManager implementation

				\return The method returns a pointer to IGraphicsObjectManager implementation
			*/

			TDE2_API IGraphicsObjectManager* GetGraphicsObjectManager() const override;

			/*!
				\brief The method returns 1 if a Z axis is collinear to (0; 0; 1) direction, -1 in other case

				\return The method returns 1 if a Z axis is collinear to (0; 0; 1) direction, -1 in other case
			*/

			TDE2_API F32 GetPositiveZAxisDirection() const override;

			/*!
				\brief The method receives a given event and processes it

				\param[in] pEvent A pointer to event data

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE OnEvent(const TBaseEvent* pEvent);

	#if TDE2_DEBUG_MODE
			TDE2_API void BeginSectionMarker(const std::string& id) override;
			TDE2_API void EndSectionMarker() override;
	#endif

			/*!
				\brief The method returns an identifier of a listener

				\return The method returns an identifier of a listener
			*/

			TDE2_API TEventListenerId GetListenerId() const;

			/*!
				\brief The method returns an information about currently using video adapter

				\return The method returns an information about currently using video adapter
			*/

			TDE2_API TVideoAdapterInfo GetInfo() const override;

			/*
				\bief The method returns information about GAPI environment

				\return The method returns data that describes all GAPI related information
			*/

			TDE2_API const TGraphicsContextInfo& GetContextInfo() const override;

			/*!
				\return The method returns a pointer to IWindowSystem
			*/

			TDE2_API TPtr<IWindowSystem> GetWindowSystem() const override;

			TDE2_API std::vector<U8> GetBackBufferData() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D11GraphicsContext)

			TDE2_API IDXGIAdapter* _getDXGIAdapter(ID3D11Device* p3dDevice) const;

			TDE2_API E_RESULT_CODE _createSwapChain(const IWindowSystem* pWindowSystem, ID3D11Device* p3dDevice);

			TDE2_API E_RESULT_CODE _createBackBuffer(IDXGISwapChain* pSwapChain, ID3D11Device* p3dDevice);

			TDE2_API E_RESULT_CODE _createDepthBuffer(U32 width, U32 height, IDXGISwapChain* pSwapChain, ID3D11Device* p3dDevice,
				ID3D11DepthStencilView** ppDepthStencilView, ID3D11Texture2D** pDepthStencilBuffer);

			TDE2_API E_RESULT_CODE _onFreeInternal() override;
		protected:
			constexpr static U8      mMaxNumOfRenderTargets = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;

			ID3D11Device*            mp3dDevice;
			ID3D11DeviceContext*     mp3dDeviceContext;
			TGraphicsCtxInternalData mInternalDataObject;

			IDXGISwapChain* mpSwapChain;

			ID3D11RenderTargetView* mpBackBufferView;

			ID3D11DepthStencilView* mpCurrDepthStencilView;
			ID3D11DepthStencilView* mpDefaultDepthStencilView;
			ID3D11DepthStencilView* mpPrevDepthStencilView;
			ID3D11Texture2D*        mpDefaultDepthStencilBuffer;

			D3D_FEATURE_LEVEL        mCurrFeatureLevel;
	#if _DEBUG
			ID3D11Debug* mpDebuggerInstance;
	#endif

			bool                     mIsVSyncEnabled;

			DXGI_FORMAT              mCurrBackBufferFormat;

			TPtr<IGraphicsObjectManager> mpGraphicsObjectManager = nullptr;
			CD3D11GraphicsObjectManager* mpGraphicsObjectManagerD3D11Impl = nullptr;

			TPtr<IWindowSystem>      mpWindowSystem;
			TPtr<IEventManager>      mpEventManager;

			ID3D11RenderTargetView* mpRenderTargets[mMaxNumOfRenderTargets];
			U8                      mCurrNumOfActiveRenderTargets = 0;
	};


	CD3D11GraphicsContext::CD3D11GraphicsContext() :
		CBaseObject(), mpCurrDepthStencilView(nullptr)
	{
	}
	
	E_RESULT_CODE CD3D11GraphicsContext::Init(TPtr<IWindowSystem> pWindowSystem)
	{
		TDE2_PROFILER_SCOPE("CD3D11GraphicsContext::Init");

		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pWindowSystem)
		{
			return RC_INVALID_ARGS;
		}

		mpWindowSystem = pWindowSystem;

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
		{
			TDE2_PROFILER_SCOPE("D3D11CreateDevice");

			d3dCallsResult = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels, numOfFeatureLevels,
				D3D11_SDK_VERSION, &mp3dDevice, &mCurrFeatureLevel, &mp3dDeviceContext);
		}

		if (FAILED(d3dCallsResult))
		{
			return RC_FAIL;
		}

		if (mCurrFeatureLevel < D3D_FEATURE_LEVEL_11_0)
		{
			return RC_GAPI_IS_NOT_SUPPORTED;
		}

#if TDE2_DEBUG_MODE/// Acquiring ID3D11Debug interface
		mp3dDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&mpDebuggerInstance));
		mp3dDeviceContext->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), reinterpret_cast<void**>(&pAnnotation));
#endif

		/// create a swap chain
		E_RESULT_CODE result = _createSwapChain(pWindowSystem.Get(), mp3dDevice);

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

		mpCurrDepthStencilView = mpDefaultDepthStencilView;

		mp3dDeviceContext->OMSetRenderTargets(1, &mpBackBufferView, mpDefaultDepthStencilView);

		/// set up a default viewport
		SetViewport(0.0f, 0.0f, static_cast<F32>(width), static_cast<F32>(height), 0.0f, 1.0f);

#if _HAS_CXX17
		mInternalDataObject = TD3D11CtxInternalData { mp3dDevice, mp3dDeviceContext };
#else
		mInternalDataObject.mD3D11 = { mp3dDevice, mp3dDeviceContext };
#endif

		mpGraphicsObjectManager = TPtr<IGraphicsObjectManager>(CreateD3D11GraphicsObjectManager(this, result));
		mpGraphicsObjectManagerD3D11Impl = dynamic_cast<CD3D11GraphicsObjectManager*>(mpGraphicsObjectManager.Get());

		if (result != RC_OK)
		{
			return result;
		}

		mpEventManager = pWindowSystem->GetEventManager();

		if (!mpEventManager)
		{
			return RC_FAIL;
		}

		mpEventManager->Subscribe(TOnWindowResized::GetTypeId(), this);

		mIsInitialized = true;

		return RC_OK;
	}
	
	E_RESULT_CODE CD3D11GraphicsContext::_onFreeInternal()
	{
		TDE2_PROFILER_SCOPE("CD3D11GraphicsContext::_onFreeInternal");

		E_RESULT_CODE result = RC_OK;

		if (mp3dDeviceContext)
		{
			mp3dDeviceContext->Flush();
			mp3dDeviceContext->ClearState();
		}

#if TDE2_DEBUG_MODE
		pAnnotation->Release();
#endif

		mpGraphicsObjectManager = nullptr;
		
		result = result | SafeReleaseCOMPtr<ID3D11DepthStencilView>(&mpDefaultDepthStencilView);
		result = result | SafeReleaseCOMPtr<ID3D11Texture2D>(&mpDefaultDepthStencilBuffer);
		result = result | SafeReleaseCOMPtr<ID3D11RenderTargetView>(&mpBackBufferView);
		result = result | SafeReleaseCOMPtr<IDXGISwapChain>(&mpSwapChain);
		result = result | SafeReleaseCOMPtr<ID3D11DeviceContext>(&mp3dDeviceContext);

#if TDE2_DEBUG_MODE
		if (mpDebuggerInstance)
		{
			mpDebuggerInstance->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
			mpDebuggerInstance->Release();
		}
#endif

		result = result | SafeReleaseCOMPtr<ID3D11Device>(&mp3dDevice);
		
		return result;
	}
	
	void CD3D11GraphicsContext::ClearBackBuffer(const TColor32F& color)
	{
		const F32 clearColorArray[4] { color.r, color.g, color.b, color.a };

		mp3dDeviceContext->ClearRenderTargetView(mpBackBufferView, clearColorArray);
	}

	void CD3D11GraphicsContext::ClearRenderTarget(IRenderTarget* pRenderTarget, const TColor32F& color)
	{
		if (!pRenderTarget)
		{
			return;
		}

		const F32 clearColorArray[4]{ color.r, color.g, color.b, color.a };
	
		auto pD3D11RenderTarget = dynamic_cast<CD3D11RenderTarget*>(pRenderTarget);
		if (!pD3D11RenderTarget)
		{
			return;
		}

		mp3dDeviceContext->ClearRenderTargetView(pD3D11RenderTarget->GetRenderTargetView(), clearColorArray);
	}

	void CD3D11GraphicsContext::ClearRenderTarget(U8 slot, const TColor32F& color)
	{
		TDE2_ASSERT(slot < mMaxNumOfRenderTargets);
		if (slot >= mMaxNumOfRenderTargets)
		{
			return;
		}

		if (!mpRenderTargets[slot])
		{
			LOG_WARNING(Wrench::StringUtils::Format("[CD3D11GraphicsContext] Try to clear the render target in slot {0}, but it's empty", slot));
			return;
		}

		const F32 clearColorArray[4]{ color.r, color.g, color.b, color.a };

		mp3dDeviceContext->ClearRenderTargetView(mpRenderTargets[slot], clearColorArray);
	}

	void CD3D11GraphicsContext::ClearDepthBufferTarget(IDepthBufferTarget* pDepthBufferTarget, F32 value, U8 stencilValue)
	{
		if (!pDepthBufferTarget)
		{
			return;
		}

		ID3D11DepthStencilView* pD3D11DepthBuffer = dynamic_cast<CD3D11DepthBufferTarget*>(pDepthBufferTarget)->GetDepthBufferTargetView();

		mp3dDeviceContext->ClearDepthStencilView(pD3D11DepthBuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, value, stencilValue);
	}

	void CD3D11GraphicsContext::ClearDepthBuffer(F32 value)
	{
		mp3dDeviceContext->ClearDepthStencilView(mpCurrDepthStencilView, D3D11_CLEAR_DEPTH, value, 0);
	}

	void CD3D11GraphicsContext::ClearStencilBuffer(U8 value)
	{
		mp3dDeviceContext->ClearDepthStencilView(mpCurrDepthStencilView, D3D11_CLEAR_STENCIL, 0.0f, value);
	}

	void CD3D11GraphicsContext::BeginFrame()
	{
	}

	void CD3D11GraphicsContext::Present()
	{
		TDE2_PROFILER_SCOPE("CD3D11GraphicsContext::Present");

		mpSwapChain->Present(mIsVSyncEnabled, 0);

		mCurrNumOfActiveRenderTargets = 1;

		/// \note Restore texture slots
		mp3dDeviceContext->ClearState();
	}

	void CD3D11GraphicsContext::SetViewport(F32 x, F32 y, F32 width, F32 height, F32 minDepth, F32 maxDepth)
	{
		D3D11_VIEWPORT viewport;

		memset(&viewport, 0, sizeof(viewport));

		viewport.TopLeftX = x;
		viewport.TopLeftY = y;
		viewport.MinDepth = minDepth;
		viewport.MaxDepth = maxDepth;
		viewport.Width    = width;
		viewport.Height   = height;

		mp3dDeviceContext->RSSetViewports(1, &viewport);
	}
	
	void CD3D11GraphicsContext::SetScissorRect(const TRectU32& scissorRect)
	{
		D3D11_RECT internalScissorRect;

		internalScissorRect.left   = static_cast<LONG>(scissorRect.x);
		internalScissorRect.top    = static_cast<LONG>(scissorRect.y);
		internalScissorRect.right  = static_cast<LONG>(scissorRect.x + scissorRect.width);
		internalScissorRect.bottom = static_cast<LONG>(scissorRect.y + scissorRect.height);

		mp3dDeviceContext->RSSetScissorRects(1, &internalScissorRect);
	}
	
	TMatrix4 CD3D11GraphicsContext::CalcPerspectiveMatrix(F32 fov, F32 aspect, F32 zn, F32 zf)
	{
		return PerspectiveProj(fov, aspect, zn, zf, 0.0f, 1.0f, -1.0f);
	}

	TMatrix4 CD3D11GraphicsContext::CalcOrthographicMatrix(F32 left, F32 top, F32 right, F32 bottom, F32 zn, F32 zf, bool isDepthless)
	{
		return OrthographicProj(left, top, right, bottom, zn, zf, 0.0f, 1.0f, -1.0f, isDepthless);
	}

	E_RESULT_CODE CD3D11GraphicsContext::SetVertexBuffer(U32 slot, TBufferHandleId vertexBufferHandle, U32 offset, U32 strideSize)
	{
		auto pBuffer = mpGraphicsObjectManagerD3D11Impl->GetD3D11BufferPtr(vertexBufferHandle);
		if (!pBuffer)
		{
			ID3D11Buffer* pNullD3D11BufferImpl = nullptr;
			mp3dDeviceContext->IASetVertexBuffers(slot, 1, &pNullD3D11BufferImpl, &strideSize, &offset);

			return RC_FAIL;
		}

		TDE2_ASSERT(E_BUFFER_TYPE::VERTEX == pBuffer->GetParams().mBufferType);

		ID3D11Buffer* pD3D11BufferImpl = pBuffer->GetD3D11Buffer();

		mp3dDeviceContext->IASetVertexBuffers(slot, 1, &pD3D11BufferImpl, &strideSize, &offset);

		return RC_OK;
	}

	E_RESULT_CODE CD3D11GraphicsContext::SetIndexBuffer(TBufferHandleId indexBufferHandle, U32 offset)
	{
		auto pBuffer = mpGraphicsObjectManagerD3D11Impl->GetD3D11BufferPtr(indexBufferHandle);
		if (!pBuffer)
		{
			return RC_FAIL;
		}

		TDE2_ASSERT(E_BUFFER_TYPE::INDEX == pBuffer->GetParams().mBufferType);

		ID3D11Buffer* pD3D11BufferImpl = pBuffer->GetD3D11Buffer();

		mp3dDeviceContext->IASetIndexBuffer(pD3D11BufferImpl, CD3D11Mappings::GetIndexFormat(pBuffer->GetParams().mIndexFormat), offset);

		return RC_OK;
	}

	E_RESULT_CODE CD3D11GraphicsContext::SetConstantBuffer(U32 slot, TBufferHandleId constantsBufferHandle)
	{
		auto pBuffer = mpGraphicsObjectManagerD3D11Impl->GetD3D11BufferPtr(constantsBufferHandle);
		if (!pBuffer)
		{
			return RC_FAIL;
		}

		TDE2_ASSERT(E_BUFFER_TYPE::CONSTANT == pBuffer->GetParams().mBufferType);

		ID3D11Buffer* pD3D11BufferImpl = pBuffer->GetD3D11Buffer();

		mp3dDeviceContext->VSSetConstantBuffers(slot, 1, &pD3D11BufferImpl);
		mp3dDeviceContext->PSSetConstantBuffers(slot, 1, &pD3D11BufferImpl);
		mp3dDeviceContext->GSSetConstantBuffers(slot, 1, &pD3D11BufferImpl);
		mp3dDeviceContext->CSSetConstantBuffers(slot, 1, &pD3D11BufferImpl);

		return RC_OK;
	}

	E_RESULT_CODE CD3D11GraphicsContext::SetStructuredBuffer(U32 slot, TBufferHandleId bufferHandle, bool isWriteEnabled)
	{
		if (TBufferHandleId::Invalid == bufferHandle)
		{
			if (isWriteEnabled)
			{
				ID3D11UnorderedAccessView* pNullUAV{ nullptr };
				mp3dDeviceContext->CSSetUnorderedAccessViews(slot, 1, &pNullUAV, nullptr);
			}
			else
			{
				ID3D11ShaderResourceView* pNullSRV{ nullptr };

				mp3dDeviceContext->VSSetShaderResources(slot, 1, &pNullSRV);
				mp3dDeviceContext->PSSetShaderResources(slot, 1, &pNullSRV);
				mp3dDeviceContext->GSSetShaderResources(slot, 1, &pNullSRV);
				mp3dDeviceContext->CSSetShaderResources(slot, 1, &pNullSRV);
			}

			return RC_OK;
		}

		auto pBuffer = mpGraphicsObjectManagerD3D11Impl->GetD3D11BufferPtr(bufferHandle);
		if (!pBuffer)
		{
			return RC_FAIL;
		}

		TDE2_ASSERT(E_BUFFER_TYPE::STRUCTURED == pBuffer->GetParams().mBufferType);

		if (pBuffer->GetParams().mIsUnorderedAccessResource)
		{
			if (isWriteEnabled)
			{
				auto pUAVResource = pBuffer->GetWriteableShaderView();
				mp3dDeviceContext->CSSetUnorderedAccessViews(slot, 1, &pUAVResource, nullptr);

				return RC_OK;
			}
		}

		auto pBufferSRV = pBuffer->GetShaderView();

		mp3dDeviceContext->VSSetShaderResources(slot, 1, &pBufferSRV);
		mp3dDeviceContext->PSSetShaderResources(slot, 1, &pBufferSRV);
		mp3dDeviceContext->GSSetShaderResources(slot, 1, &pBufferSRV);
		mp3dDeviceContext->CSSetShaderResources(slot, 1, &pBufferSRV);

		return RC_OK;
	}

	E_RESULT_CODE CD3D11GraphicsContext::SetTexture(U32 slot, TTextureHandleId textureHandle, bool isWriteEnabled)
	{
		auto pTexture = mpGraphicsObjectManagerD3D11Impl->GetD3D11TexturePtr(textureHandle);
		if (!pTexture)
		{
			if (isWriteEnabled)
			{
				ID3D11UnorderedAccessView* pNullUAV{ nullptr };
				mp3dDeviceContext->CSSetUnorderedAccessViews(slot, 1, &pNullUAV, nullptr);
			}
			else
			{
				ID3D11ShaderResourceView* pNullSRV{ nullptr };

				mp3dDeviceContext->VSSetShaderResources(slot, 1, &pNullSRV);
				mp3dDeviceContext->PSSetShaderResources(slot, 1, &pNullSRV);
				mp3dDeviceContext->GSSetShaderResources(slot, 1, &pNullSRV);
				mp3dDeviceContext->CSSetShaderResources(slot, 1, &pNullSRV);
			}

			return RC_FAIL;
		}

		if (pTexture->GetParams().mIsWriteable)
		{
			if (isWriteEnabled)
			{
				auto pTextureUAV = pTexture->GetUnorderedAccessView();
				mp3dDeviceContext->CSSetUnorderedAccessViews(slot, 1, &pTextureUAV, nullptr);

				return RC_OK;
			}
		}

		auto pTextureSRV = pTexture->GetShaderResourceView();

		mp3dDeviceContext->VSSetShaderResources(slot, 1, &pTextureSRV);
		mp3dDeviceContext->PSSetShaderResources(slot, 1, &pTextureSRV);
		mp3dDeviceContext->GSSetShaderResources(slot, 1, &pTextureSRV);
		mp3dDeviceContext->CSSetShaderResources(slot, 1, &pTextureSRV);

		return RC_OK;
	}

	E_RESULT_CODE CD3D11GraphicsContext::SetSampler(U32 slot, TTextureSamplerId samplerHandle)
	{
		if (samplerHandle == TTextureSamplerId::Invalid)
		{
			mp3dDeviceContext->VSSetSamplers(slot, 1, nullptr);
			mp3dDeviceContext->PSSetSamplers(slot, 1, nullptr);
			mp3dDeviceContext->GSSetSamplers(slot, 1, nullptr);
			mp3dDeviceContext->CSSetSamplers(slot, 1, nullptr);

			return RC_OK;
		}

		ID3D11SamplerState* pSamplerState = mpGraphicsObjectManagerD3D11Impl->GetTextureSampler(samplerHandle).Get();

		mp3dDeviceContext->VSSetSamplers(slot, 1, &pSamplerState);
		mp3dDeviceContext->PSSetSamplers(slot, 1, &pSamplerState);
		mp3dDeviceContext->GSSetSamplers(slot, 1, &pSamplerState);
		mp3dDeviceContext->CSSetSamplers(slot, 1, &pSamplerState);

		return RC_OK;
	}

	E_RESULT_CODE CD3D11GraphicsContext::UpdateTexture2D(TTextureHandleId textureHandle, U32 mipLevel, const TRectI32& regionRect, const void* pData, USIZE dataSize)
	{
		auto pTexture = mpGraphicsObjectManagerD3D11Impl->GetD3D11TexturePtr(textureHandle);
		if (!pTexture)
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

		U32 rowPitch = regionRect.width * CD3D11Mappings::GetFormatSize(pTexture->GetParams().mFormat);

		mp3dDeviceContext->UpdateSubresource(pTexture->GetTextureResource(), mipLevel, &region, pData, rowPitch, rowPitch * regionRect.height);

		return RC_OK;
	}

	E_RESULT_CODE CD3D11GraphicsContext::UpdateTexture2DArray(TTextureHandleId textureHandle, U32 index, const TRectI32& regionRect, const void* pData, USIZE dataSize)
	{
		auto pTexture = mpGraphicsObjectManagerD3D11Impl->GetD3D11TexturePtr(textureHandle);
		if (!pTexture)
		{
			return RC_FAIL;
		}

		return UpdateTexture2D(textureHandle, D3D11CalcSubresource(0, index, pTexture->GetParams().mNumOfMipLevels), regionRect, pData, dataSize);
	}

	E_RESULT_CODE CD3D11GraphicsContext::UpdateCubemapTexture(TTextureHandleId textureHandle, E_CUBEMAP_FACE face, const TRectI32& regionRect, const void* pData, USIZE dataSize)
	{
		auto pTexture = mpGraphicsObjectManagerD3D11Impl->GetD3D11TexturePtr(textureHandle);
		if (!pTexture)
		{
			return RC_FAIL;
		}

		return UpdateTexture2D(textureHandle, D3D11CalcSubresource(0, CD3D11Mappings::GetCubemapFace(face), pTexture->GetParams().mNumOfMipLevels), regionRect, pData, dataSize);
	}

	E_RESULT_CODE CD3D11GraphicsContext::UpdateTexture3D(TTextureHandleId textureHandle, U32 depthFrom, U32 depthTo, const TRectI32& regionRect, const void* pData, USIZE dataSize)
	{
		auto pTexture = mpGraphicsObjectManagerD3D11Impl->GetD3D11TexturePtr(textureHandle);
		if (!pTexture)
		{
			return RC_FAIL;
		}

		D3D11_BOX region;

		region.left = regionRect.x;
		region.top = regionRect.y;
		region.right = regionRect.x + regionRect.width;
		region.bottom = regionRect.y + regionRect.height;
		region.front = depthFrom;
		region.back = region.front + (depthTo - depthFrom);

		U32 rowPitch = regionRect.width * CD3D11Mappings::GetNumOfChannelsOfFormat(pTexture->GetParams().mFormat);

		mp3dDeviceContext->UpdateSubresource(pTexture->GetTextureResource(),
			D3D11CalcSubresource(0, 0, pTexture->GetParams().mNumOfMipLevels), &region, pData, rowPitch, rowPitch * regionRect.height);

		return RC_OK;
	}

	E_RESULT_CODE CD3D11GraphicsContext::CopyResource(TTextureHandleId sourceHandle, TTextureHandleId destHandle)
	{
		auto pSourceTexture = mpGraphicsObjectManagerD3D11Impl->GetD3D11TexturePtr(sourceHandle);
		auto pDestTexture = mpGraphicsObjectManagerD3D11Impl->GetD3D11TexturePtr(destHandle);

		if (!pSourceTexture || !pDestTexture)
		{
			return RC_FAIL;
		}

		mp3dDeviceContext->CopyResource(pDestTexture->GetTextureResource(), pSourceTexture->GetTextureResource());

		return RC_OK;
	}

	E_RESULT_CODE CD3D11GraphicsContext::CopyResource(TBufferHandleId sourceHandle, TTextureHandleId destHandle)
	{
		return RC_OK;
	}

	E_RESULT_CODE CD3D11GraphicsContext::CopyResource(TTextureHandleId sourceHandle, TBufferHandleId destHandle)
	{
		return RC_OK;
	}

	E_RESULT_CODE CD3D11GraphicsContext::CopyResource(TBufferHandleId sourceHandle, TBufferHandleId destHandle)
	{
		auto pSourceBuffer = mpGraphicsObjectManagerD3D11Impl->GetD3D11BufferPtr(sourceHandle);
		auto pDestBuffer = mpGraphicsObjectManagerD3D11Impl->GetD3D11BufferPtr(destHandle);

		if (!pSourceBuffer || !pDestBuffer)
		{
			return RC_FAIL;
		}

		mp3dDeviceContext->CopyResource(pDestBuffer->GetD3D11Buffer(), pSourceBuffer->GetD3D11Buffer());
		
		return RC_OK;
	}

	E_RESULT_CODE CD3D11GraphicsContext::CopyCount(TBufferHandleId sourceHandle, TBufferHandleId destHandle, U32 offset)
	{
		auto pSourceBuffer = mpGraphicsObjectManagerD3D11Impl->GetD3D11BufferPtr(sourceHandle);
		auto pDestBuffer = mpGraphicsObjectManagerD3D11Impl->GetD3D11BufferPtr(destHandle);

		if (!pSourceBuffer || !pDestBuffer)
		{
			return RC_FAIL;
		}

		const TInitBufferParams& sourceBufferParams = pSourceBuffer->GetParams();

		if (E_STRUCTURED_BUFFER_TYPE::APPENDABLE != sourceBufferParams.mStructuredBufferType ||
			E_BUFFER_TYPE::STRUCTURED != sourceBufferParams.mBufferType ||
			!sourceBufferParams.mIsUnorderedAccessResource)
		{
			return RC_INVALID_ARGS;
		}

		mp3dDeviceContext->CopyStructureCount(pDestBuffer->GetD3D11Buffer(), offset, pSourceBuffer->GetWriteableShaderView());

		return RC_OK;
	}

	E_RESULT_CODE CD3D11GraphicsContext::GenerateMipMaps(TTextureHandleId textureHandle)
	{
		if (TTextureHandleId::Invalid == textureHandle)
		{
			return RC_FAIL;
		}

		auto&& pTexture = mpGraphicsObjectManagerD3D11Impl->GetD3D11TexturePtr(textureHandle);
		if (!pTexture)
		{
			return RC_FAIL;
		}

		mp3dDeviceContext->GenerateMips(pTexture->GetShaderResourceView());

		return RC_OK;
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

	void CD3D11GraphicsContext::DrawInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, U32 startVertex, U32 verticesPerInstance, U32 startInstance, U32 numOfInstances)
	{
		mp3dDeviceContext->IASetPrimitiveTopology(CD3D11Mappings::GetPrimitiveTopology(topology));

		mp3dDeviceContext->DrawInstanced(verticesPerInstance, numOfInstances, startVertex, startInstance);
	}

	void CD3D11GraphicsContext::DrawIndexedInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, U32 baseVertex, U32 startIndex,
													 U32 startInstance, U32 indicesPerInstance, U32 numOfInstances)
	{
		mp3dDeviceContext->IASetPrimitiveTopology(CD3D11Mappings::GetPrimitiveTopology(topology));

		mp3dDeviceContext->DrawIndexedInstanced(indicesPerInstance, numOfInstances, startIndex, baseVertex, startInstance);
	}

	void CD3D11GraphicsContext::DrawIndirectInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, TBufferHandleId argsBufferHandle, U32 alignedOffset)
	{
		mp3dDeviceContext->IASetPrimitiveTopology(CD3D11Mappings::GetPrimitiveTopology(topology));

		auto pArgsBuffer = mpGraphicsObjectManagerD3D11Impl->GetD3D11BufferPtr(argsBufferHandle);
		mp3dDeviceContext->DrawInstancedIndirect(pArgsBuffer ? pArgsBuffer->GetD3D11Buffer() : nullptr, alignedOffset);
	}

	void CD3D11GraphicsContext::DrawIndirectIndexedInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, TBufferHandleId argsBufferHandle, U32 alignedOffset)
	{
		mp3dDeviceContext->IASetPrimitiveTopology(CD3D11Mappings::GetPrimitiveTopology(topology));

		auto pArgsBuffer = mpGraphicsObjectManagerD3D11Impl->GetD3D11BufferPtr(argsBufferHandle);
		mp3dDeviceContext->DrawIndexedInstancedIndirect(pArgsBuffer ? pArgsBuffer->GetD3D11Buffer() : nullptr, alignedOffset);
	}

	void CD3D11GraphicsContext::DispatchCompute(U32 groupsCountX, U32 groupsCountY, U32 groupsCountZ)
	{
		mp3dDeviceContext->Dispatch(groupsCountX, groupsCountY, groupsCountZ);
	}

	void CD3D11GraphicsContext::DispatchIndirectCompute(TBufferHandleId argsBufferHandle, U32 alignedOffset)
	{
		auto pArgsBuffer = mpGraphicsObjectManagerD3D11Impl->GetD3D11BufferPtr(argsBufferHandle);
		mp3dDeviceContext->DispatchIndirect(pArgsBuffer ? pArgsBuffer->GetD3D11Buffer() : nullptr, alignedOffset);
	}

	void CD3D11GraphicsContext::BindBlendState(TBlendStateId blendStateId)
	{
		if (blendStateId == TBlendStateId::Invalid)
		{
			mp3dDeviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
			return;
		}

		ID3D11BlendState* pBlendState = mpGraphicsObjectManagerD3D11Impl->GetBlendState(blendStateId).Get();
		
		// \todo the second argument is not used now, but later it should be parametrized
		mp3dDeviceContext->OMSetBlendState(pBlendState, nullptr, 0xFFFFFFFF);
	}

	void CD3D11GraphicsContext::BindDepthStencilState(TDepthStencilStateId depthStencilStateId, U8 stencilRef)
	{
		if (depthStencilStateId == TDepthStencilStateId::Invalid)
		{
			mp3dDeviceContext->OMSetDepthStencilState(nullptr, stencilRef);
			return;
		}

		ID3D11DepthStencilState* pDepthStencilState = mpGraphicsObjectManagerD3D11Impl->GetDepthStencilState(depthStencilStateId).Get();

		// \todo replace 0xff with second argument for the method
		mp3dDeviceContext->OMSetDepthStencilState(pDepthStencilState, stencilRef);
	}

	void CD3D11GraphicsContext::BindRasterizerState(TRasterizerStateId rasterizerStateId)
	{
		if (rasterizerStateId == TRasterizerStateId::Invalid)
		{
			mp3dDeviceContext->RSSetState(nullptr);
			return;
		}

		ID3D11RasterizerState* pRasterizerState = mpGraphicsObjectManagerD3D11Impl->GetRasterizerState(rasterizerStateId).Get();
		mp3dDeviceContext->RSSetState(pRasterizerState);
	}

	void CD3D11GraphicsContext::BindRenderTarget(U8 slot, IRenderTarget* pRenderTarget)
	{
		TDE2_ASSERT(slot < mMaxNumOfRenderTargets);
		if (slot >= mMaxNumOfRenderTargets)
		{
			LOG_WARNING("[CD3D11GraphicsContext] Render target's slot goes out of limits");
			return;
		}

		if (!slot && !pRenderTarget)
		{
			mp3dDeviceContext->OMSetRenderTargets(1, &mpBackBufferView, mpCurrDepthStencilView);

			return;
		}

		mpRenderTargets[slot] = dynamic_cast<CD3D11RenderTarget*>(pRenderTarget)->GetRenderTargetView();
		pRenderTarget->UnbindFromShader();
		
		mCurrNumOfActiveRenderTargets = std::max<U8>(mCurrNumOfActiveRenderTargets, slot + 1);

		mp3dDeviceContext->OMSetRenderTargets(mCurrNumOfActiveRenderTargets, mpRenderTargets, mpCurrDepthStencilView);
	}

	void CD3D11GraphicsContext::BindRenderTarget(U8 slot, TTextureHandleId targetHandle)
	{
		TDE2_ASSERT(slot < mMaxNumOfRenderTargets);
		if (slot >= mMaxNumOfRenderTargets)
		{
			LOG_WARNING("[CD3D11GraphicsContext] Render target's slot goes out of limits");
			return;
		}

		if (!slot && TTextureHandleId::Invalid == targetHandle)
		{
			mp3dDeviceContext->OMSetRenderTargets(1, &mpBackBufferView, mpCurrDepthStencilView);
			return;
		}

		auto pRenderTargetTexture = mpGraphicsObjectManagerD3D11Impl->GetD3D11TexturePtr(targetHandle);
		if (!pRenderTargetTexture)
		{
			TDE2_ASSERT(false);
			return;
		}

		if (E_BIND_GRAPHICS_TYPE::BIND_RENDER_TARGET != (pRenderTargetTexture->GetParams().mBindFlags & E_BIND_GRAPHICS_TYPE::BIND_RENDER_TARGET))
		{
			TDE2_ASSERT_MSG(false, "[CD3D11GraphicsContext] Try to bind texture that is not a render target to render target slot");
			return;
		}
		
		mpRenderTargets[slot] = pRenderTargetTexture->GetRenderTargetView();

		mCurrNumOfActiveRenderTargets = std::max<U8>(mCurrNumOfActiveRenderTargets, slot + 1);
		mp3dDeviceContext->OMSetRenderTargets(mCurrNumOfActiveRenderTargets, mpRenderTargets, mpCurrDepthStencilView);
	}

	void CD3D11GraphicsContext::BindDepthBufferTarget(IDepthBufferTarget* pDepthBufferTarget, bool disableRTWrite)
	{
		if (!pDepthBufferTarget)
		{
			mpCurrDepthStencilView = mpDefaultDepthStencilView;
			mp3dDeviceContext->OMSetRenderTargets(1, &mpBackBufferView, mpCurrDepthStencilView);

			return;
		}

		mpCurrDepthStencilView = dynamic_cast<CD3D11DepthBufferTarget*>(pDepthBufferTarget)->GetDepthBufferTargetView();

		static ID3D11RenderTargetView* pNullRT[] { nullptr };

		mp3dDeviceContext->OMSetRenderTargets(disableRTWrite ? 1 : mCurrNumOfActiveRenderTargets, disableRTWrite ? pNullRT : mpRenderTargets, mpCurrDepthStencilView);
	}

	void CD3D11GraphicsContext::BindDepthBufferTarget(TTextureHandleId targetHandle, bool disableRTWrite)
	{
		if (TTextureHandleId::Invalid == targetHandle)
		{
			mpCurrDepthStencilView = mpDefaultDepthStencilView;
			mp3dDeviceContext->OMSetRenderTargets(1, &mpBackBufferView, mpCurrDepthStencilView);

			return;
		}

		auto pDepthTargetTexture = mpGraphicsObjectManagerD3D11Impl->GetD3D11TexturePtr(targetHandle);
		if (!pDepthTargetTexture)
		{
			TDE2_ASSERT(false);
			return;
		}

		if (E_BIND_GRAPHICS_TYPE::BIND_DEPTH_BUFFER != (pDepthTargetTexture->GetParams().mBindFlags & E_BIND_GRAPHICS_TYPE::BIND_DEPTH_BUFFER))
		{
			TDE2_ASSERT_MSG(false, "[CD3D11GraphicsContext] Try to bind texture that is not a depth buffer to depth buffer slot");
			return;
		}

		mpCurrDepthStencilView = pDepthTargetTexture->GetDepthStencilView();

		static ID3D11RenderTargetView* pNullRT[]{ nullptr };

		mp3dDeviceContext->OMSetRenderTargets(disableRTWrite ? 1 : mCurrNumOfActiveRenderTargets, disableRTWrite ? pNullRT : mpRenderTargets, mpCurrDepthStencilView);
	}

	void CD3D11GraphicsContext::SetDepthBufferEnabled(bool value)
	{
		auto pPrevDepthView = mpCurrDepthStencilView;
		mpCurrDepthStencilView = value ? mpPrevDepthStencilView : nullptr;
		mpPrevDepthStencilView = pPrevDepthView;
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
		return mpGraphicsObjectManagerD3D11Impl;
	}
	
	F32 CD3D11GraphicsContext::GetPositiveZAxisDirection() const
	{
		return 1.0f;
	}

	E_RESULT_CODE CD3D11GraphicsContext::OnEvent(const TBaseEvent* pEvent)
	{
		TDE2_PROFILER_SCOPE("CD3D11GraphicsContext::OnEvent");

		if (pEvent->GetEventType() != TOnWindowResized::GetTypeId())
		{
			return RC_OK;
		}

		const TOnWindowResized* pOnWindowResizedEvent = dynamic_cast<const TOnWindowResized*>(pEvent);

		HRESULT internalResult = S_OK;

		E_RESULT_CODE result = RC_OK;

		U32 width  = pOnWindowResizedEvent->mWidth;
		U32 height = pOnWindowResizedEvent->mHeight;

		if (mpSwapChain)
		{
			mp3dDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
			mp3dDeviceContext->ClearState();

			const bool isDefaultDepthBufferInUse = mpCurrDepthStencilView == mpDefaultDepthStencilView;

			SafeReleaseCOMPtr<ID3D11RenderTargetView>(&mpBackBufferView, true);
			SafeReleaseCOMPtr<ID3D11Texture2D>(&mpDefaultDepthStencilBuffer, true);
			SafeReleaseCOMPtr<ID3D11DepthStencilView>(&mpDefaultDepthStencilView, true);
			
			if (FAILED(internalResult = mpSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0)))
			{
				return RC_FAIL;
			}

			if ((result = _createBackBuffer(mpSwapChain, mp3dDevice)) != RC_OK)
			{
				return result;
			}

			if ((result = _createDepthBuffer(width, height, mpSwapChain, mp3dDevice, &mpDefaultDepthStencilView, &mpDefaultDepthStencilBuffer)) != RC_OK)
			{
				return result;
			}

			TDE2_ASSERT(mpBackBufferView && mpDefaultDepthStencilView);

			mp3dDeviceContext->OMSetRenderTargets(1, &mpBackBufferView, mpDefaultDepthStencilView);

			if (isDefaultDepthBufferInUse)
			{
				mpCurrDepthStencilView = mpDefaultDepthStencilView;
			}

			mpPrevDepthStencilView = nullptr;
		}

		SetViewport(0.0f, 0.0f, static_cast<F32>(width), static_cast<F32>(height), 0.0f, 1.0f);
		
		return RC_OK;
	}

#if TDE2_DEBUG_MODE

	void CD3D11GraphicsContext::BeginSectionMarker(const std::string& id)
	{
		pAnnotation->BeginEvent(std::wstring(id.begin(), id.end()).c_str());
	}

	void CD3D11GraphicsContext::EndSectionMarker()
	{
		pAnnotation->EndEvent();
	}

#endif

	TEventListenerId CD3D11GraphicsContext::GetListenerId() const
	{
		return TEventListenerId(GetTypeId());
	}

	TVideoAdapterInfo CD3D11GraphicsContext::GetInfo() const
	{
		IDXGIAdapter* pDXGIAdapter = _getDXGIAdapter(mp3dDevice);

		DXGI_ADAPTER_DESC adapterInfo;
		HRESULT result = pDXGIAdapter->GetDesc(&adapterInfo);

		static const std::unordered_map<U32, TVideoAdapterInfo::E_VENDOR_TYPE> vendorsMap
		{
			{ 0x10DE, TVideoAdapterInfo::E_VENDOR_TYPE::NVIDIA },
			{ 0x1002, TVideoAdapterInfo::E_VENDOR_TYPE::AMD },
			{ 0x1022, TVideoAdapterInfo::E_VENDOR_TYPE::AMD },
			{ 0x163C, TVideoAdapterInfo::E_VENDOR_TYPE::INTEL },
			{ 0x8086, TVideoAdapterInfo::E_VENDOR_TYPE::INTEL },
			{ 0x8087, TVideoAdapterInfo::E_VENDOR_TYPE::INTEL },
		};

		TVideoAdapterInfo outputInfo;
		outputInfo.mAvailableVideoMemory = adapterInfo.DedicatedVideoMemory;
		outputInfo.mVendorType           = vendorsMap.find(adapterInfo.VendorId) == vendorsMap.cend() ? TVideoAdapterInfo::E_VENDOR_TYPE::UNKNOWN : vendorsMap.at(adapterInfo.VendorId);

		return outputInfo;
	}

	const TGraphicsContextInfo& CD3D11GraphicsContext::GetContextInfo() const
	{
		const static TGraphicsContextInfo infoData
		{
			{ { -1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f } },
			false,
			E_GRAPHICS_CONTEXT_GAPI_TYPE::GCGT_DIRECT3D11
		};

		return infoData;
	}

	TPtr<IWindowSystem> CD3D11GraphicsContext::GetWindowSystem() const
	{
		return mpWindowSystem;
	}

	std::vector<U8> CD3D11GraphicsContext::GetBackBufferData() const
	{
		std::vector<U8> backBufferData;

		ID3D11Texture2D* pBackBuffer;

		/// \note Retrieve back buffer's texture
		if (FAILED(mpSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer))))
		{
			return std::move(backBufferData);
		}

		D3D11_TEXTURE2D_DESC backBufferDesc;
		pBackBuffer->GetDesc(&backBufferDesc);

		/// \note Create a new temporary texture that could be read
		ID3D11Texture2D* pTempReadableTexture = nullptr;

		backBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		backBufferDesc.Usage = D3D11_USAGE_STAGING;
		backBufferDesc.BindFlags = 0x0;

		defer([pBackBuffer] { pBackBuffer->Release(); });

		if (FAILED(mp3dDevice->CreateTexture2D(&backBufferDesc, nullptr, &pTempReadableTexture)))
		{
			return std::move(backBufferData);
		}
		
		mp3dDeviceContext->CopyResource(pTempReadableTexture, pBackBuffer);

		defer([pTempReadableTexture] { pTempReadableTexture->Release(); });

		D3D11_MAPPED_SUBRESOURCE mappedData;

		if (FAILED(mp3dDeviceContext->Map(pTempReadableTexture, 0, D3D11_MAP_READ, 0x0, &mappedData)))
		{
			return std::move(backBufferData);
		}
		
		const size_t imageSize = backBufferDesc.Width * backBufferDesc.Height * sizeof(U32);
		
		/// \note Read the data of the texture
		backBufferData.resize(imageSize);
		memcpy(&backBufferData[0], mappedData.pData, imageSize);

		mp3dDeviceContext->Unmap(pTempReadableTexture, 0);

		/// \note Remove alpha channel
		/// \fixme Maybe there is a way to simplify this with proper format
		for (USIZE i = 3; i < backBufferData.size(); i += 4)
		{
			backBufferData[i] = 0xff;
		}

		return std::move(backBufferData);
	}

	IDXGIAdapter* CD3D11GraphicsContext::_getDXGIAdapter(ID3D11Device* p3dDevice) const
	{
		IDXGIDevice*  pDXGIDevice  = nullptr;
		IDXGIAdapter* pDXGIAdapter = nullptr;

		HRESULT result = p3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&pDXGIDevice));

		if (FAILED(result))
		{
			return nullptr;
		}

		result = pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&pDXGIAdapter));

		pDXGIDevice->Release();

		return pDXGIAdapter;
	}

	E_RESULT_CODE CD3D11GraphicsContext::_createSwapChain(const IWindowSystem* pWindowSystem, ID3D11Device* p3dDevice)
	{
		TDE2_PROFILER_SCOPE("CD3D11GraphicsContext::_createSwapChain");

		DXGI_SWAP_CHAIN_DESC swapChainDesc;

		memset(&swapChainDesc, 0, sizeof(swapChainDesc));

		U32 applicationFlags = pWindowSystem->GetFlags();

		mIsVSyncEnabled = applicationFlags & P_VSYNC;

		HWND hwnd = pWindowSystem->GetInternalData().mWindowHandler;

		UINT width  = pWindowSystem->GetWidth();
		UINT height = pWindowSystem->GetHeight();

		mCurrBackBufferFormat = (applicationFlags & P_HARDWARE_GAMMA_CORRECTION) ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;

		swapChainDesc.BufferDesc.Width                   = width;
		swapChainDesc.BufferDesc.Height                  = height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator   = 60;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferDesc.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.BufferDesc.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Format                  = mCurrBackBufferFormat;
		swapChainDesc.OutputWindow                       = hwnd;
		swapChainDesc.Windowed                           = !(applicationFlags & P_FULLSCREEN);
		swapChainDesc.Flags                              = 0;
		swapChainDesc.BufferCount                        = 1;
		swapChainDesc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.SampleDesc.Count                   = 1;

		IDXGIAdapter* pDXGIAdapter = _getDXGIAdapter(mp3dDevice);
		IDXGIFactory* pDXGIFactory = nullptr;
		
		if (!pDXGIAdapter)
		{
			return RC_FAIL;
		}

		HRESULT result = pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&pDXGIFactory));

		if (FAILED(result))
		{
			pDXGIAdapter->Release();

			return RC_FAIL;
		}

		result = pDXGIFactory->CreateSwapChain(mp3dDevice, &swapChainDesc, &mpSwapChain);

		defer([&pDXGIAdapter, &pDXGIFactory] 
		{
			pDXGIAdapter->Release();
			pDXGIFactory->Release();
		});

		if (FAILED(result))
		{
			return RC_FAIL;
		}

		return RC_OK;
	}

	E_RESULT_CODE CD3D11GraphicsContext::_createBackBuffer(IDXGISwapChain* pSwapChain, ID3D11Device* p3dDevice)
	{
		TDE2_PROFILER_SCOPE("CD3D11GraphicsContext::_createBackBuffer");

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
		TDE2_PROFILER_SCOPE("CD3D11GraphicsContext::_createDepthBuffer");

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

		if (FAILED(p3dDevice->CreateDepthStencilView(*pDepthStencilBuffer, nullptr, ppDepthStencilView)))
		{
			return RC_FAIL;
		}

		return RC_OK;
	}


	IGraphicsContext* CreateD3D11GraphicsContext(TPtr<IWindowSystem> pWindowSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IGraphicsContext, CD3D11GraphicsContext, result, pWindowSystem);
	}
}

#endif