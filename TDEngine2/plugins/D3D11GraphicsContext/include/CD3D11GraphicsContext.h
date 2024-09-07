/*!
	\file CD3D11GraphicsContext.h
	\date 21.09.2018
	\authors Kasimov Ildar
*/
#pragma once


#include <core/IGraphicsContext.h>
#include <core/Event.h>
#include <core/CBaseObject.h>


#if defined(TDE2_USE_WINPLATFORM) /// Used only on Windows platform

namespace TDEngine2
{
	class IEventManager;
	class CD3D11GraphicsObjectManager;


	TDE2_DECLARE_SCOPED_PTR(IEventManager)
	TDE2_DECLARE_SCOPED_PTR(IGraphicsObjectManager)


	class CD3D11GraphicsContext : public IGraphicsContext, public IEventHandler, public CBaseObject
	{
		public:
			friend TDE2_API IGraphicsContext* CreateD3D11GraphicsContext(TPtr<IWindowSystem>, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_TYPE(CD3D11GraphicsContext)

			/*!
				\brief The method initializes an initial state of the object

				\param[in] pWindowSystem A pointer to implementation of IWindowSystem interface

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(TPtr<IWindowSystem> pWindowSystem) override;

			TDE2_API E_RESULT_CODE AcquireWorkerThreads() override;

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

			IDXGISwapChain*          mpSwapChain;

			ID3D11RenderTargetView*  mpBackBufferView;

			ID3D11DepthStencilView*  mpCurrDepthStencilView;
			ID3D11DepthStencilView*  mpDefaultDepthStencilView;
			ID3D11DepthStencilView*  mpPrevDepthStencilView;
			ID3D11Texture2D*         mpDefaultDepthStencilBuffer;

			D3D_FEATURE_LEVEL        mCurrFeatureLevel; 
			#if _DEBUG
			ID3D11Debug*             mpDebuggerInstance;
			#endif

			bool                     mIsVSyncEnabled;

			DXGI_FORMAT              mCurrBackBufferFormat;

			TPtr<IGraphicsObjectManager> mpGraphicsObjectManager = nullptr;
			CD3D11GraphicsObjectManager* mpGraphicsObjectManagerD3D11Impl = nullptr;

			TPtr<IWindowSystem>      mpWindowSystem;
			TPtr<IEventManager>      mpEventManager;

			ID3D11RenderTargetView*  mpRenderTargets[mMaxNumOfRenderTargets];
			U8                       mCurrNumOfActiveRenderTargets = 0;
	};


	/*!
		\brief A factory function for creation objects of CD3D11GraphicsContext's type

		\return A pointer to CD3D11GraphicsContext's implementation
	*/

	TDE2_API IGraphicsContext* CreateD3D11GraphicsContext(TPtr<IWindowSystem> pWindowSystem, E_RESULT_CODE& result);
}

#endif