/*!
	\file CD3D12GraphicsContext.h
	\date 21.09.2024
	\authors Kasimov Ildar
*/
#pragma once


#include <core/IGraphicsContext.h>
#include <core/Event.h>
#include <core/CBaseObject.h>
#include <mutex>

#if defined(TDE2_USE_WINPLATFORM) /// Used only on Windows platform

#include <d3d12.h>
#include <wrl.h>


namespace D3D12MA
{
	class Allocator;
	class Allocation;
}


namespace TDEngine2
{
	class IEventManager;
	class CD3D12DeviceContext;
	class CD3D12Swapchain;
	class CD3D12CommandBuffer;
	class CD3D12GraphicsObjectManager;


	TDE2_DECLARE_SCOPED_PTR(IEventManager);
	TDE2_DECLARE_SCOPED_PTR(CD3D12DeviceContext);
	TDE2_DECLARE_SCOPED_PTR(CD3D12Swapchain);
	TDE2_DECLARE_SCOPED_PTR(CD3D12CommandBuffer);


	class CD3D12GraphicsContext : public IGraphicsContext, public IEventHandler, public CBaseObject
	{
		public:
			friend IGraphicsContext* CreateD3D12GraphicsContext(TPtr<IWindowSystem>, E_RESULT_CODE&);
		public:
			struct TGarbageEntity
			{
				Microsoft::WRL::ComPtr<ID3D12Resource> mpResource = nullptr;
				D3D12MA::Allocation*                   mpAllocation = nullptr;
			};

			typedef std::vector<TGarbageEntity> TGarbageCollection;
		public:
			TDE2_REGISTER_TYPE(CD3D12GraphicsContext)

			/*!
				\brief The method initializes an initial state of the object

				\param[in] pWindowSystem A pointer to implementation of IWindowSystem interface

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			E_RESULT_CODE Init(TPtr<IWindowSystem> pWindowSystem) override;

			void BeginFrame() override;

			/*!
				\brief The method swaps a current buffer with a back one
			*/

			void Present() override;

			E_RESULT_CODE DestroyObjectDeffered(Microsoft::WRL::ComPtr<ID3D12Resource> pResource, D3D12MA::Allocation* pAllocation);

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

			TDE2_API void MemoryAccessBarrier(const std::variant<TBufferHandleId, TTextureHandleId> resourceHandle) override;
			TDE2_API void TransitionBarrier(const TBufferTransitionBarrierInfo& barrierInfo) override;
			TDE2_API void TransitionBarrier(const TTextureTransitionBarrierInfo& barrierInfo) override;
			TDE2_API void DebugBarrier() override;

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
				\brief The pair of methods BeginRenderPass/EndRenderPass are intended to replace separate calls of BindRenderTarget/BindDepthBufferTarget/SetDepthBufferEnabled and others
				to provide single point configuration of all targets that participate in rendering. The concept should be familiar for ones who worked with OpenGL/Vulkan GAPIs
			*/

			TDE2_API E_RESULT_CODE BeginRenderPass(const TFramebufferInfo& framebufferInfo) override;
			TDE2_API E_RESULT_CODE EndRenderPass() override;

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

			ID3D12Device* GetDeviceContext() const;
			D3D12MA::Allocator* GetMemoryAllocator() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CD3D12GraphicsContext)

			E_RESULT_CODE _onFreeInternal() override;

			void _waitForIdle();

			ID3D12GraphicsCommandList* _getCurrCommandListPtr();
		public:
			TDE2_STATIC_CONSTEXPR      U32 BACK_BUFFERS_COUNT = 2;
		protected:

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
			TPtr<IGraphicsObjectManager> mpGraphicsObjectManager = nullptr;
			CD3D12GraphicsObjectManager* mpGraphicsObjectManagerD3D12Impl = nullptr;
			
			TPtr<IWindowSystem>       mpWindowSystem = nullptr;
			TPtr<IEventManager>       mpEventManager = nullptr;

			TPtr<CD3D12DeviceContext> mpDeviceContext = nullptr;
			TPtr<CD3D12Swapchain>     mpSwapchain = nullptr;

			TPtr<CD3D12CommandBuffer> mpImmediateCommandBuffer = nullptr;

			std::array<TPtr<CD3D12CommandBuffer>, BACK_BUFFERS_COUNT> mpCommandBuffers {};
			std::array<TGarbageCollection, BACK_BUFFERS_COUNT>        mAwaitingDeletionObjects {};

			bool                                                      mIsRenderPassActive = false;

			mutable std::mutex                                        mGarbageCollectorMutex {};

			TDescriptorsBindingsTable                                 mDescriptorsBindingsTable{};
			// 
			//		ID3D12RenderTargetView* mpRenderTargets[mMaxNumOfRenderTargets];
			//		U8                      mCurrNumOfActiveRenderTargets = 0;
	};


	/*!
		\brief A factory function for creation objects of CD3D12GraphicsContext's type

		\return A pointer to CD3D12GraphicsContext's implementation
	*/

	IGraphicsContext* CreateD3D12GraphicsContext(TPtr<IWindowSystem> pWindowSystem, E_RESULT_CODE& result);
}

#endif