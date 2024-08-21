/*!
	\file IGraphicsContext.h
	\date 20.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IEngineSubsystem.h"
#include "../utils/Types.h"
#include "../utils/Color.h"
#include "../math/TMatrix4.h"
#include "../math/TRect.h"
#include "../math/TAABB.h"


namespace TDEngine2
{
	class IWindowSystem;
	class IGraphicsObjectManager;
	class IRenderTarget;
	class IDepthBufferTarget;


	TDE2_DECLARE_SCOPED_PTR(IWindowSystem)
	TDE2_DECLARE_SCOPED_PTR(IGraphicsObjectManager)


	enum class TBufferHandleId : U32;
	enum class TTextureHandleId : U32;
	enum class E_INDEX_FORMAT_TYPE : U8;
	enum class E_CUBEMAP_FACE : U8;


	/*!
		enum class E_PRIMITIVE_TOPOLOGY_TYPE

		\brief The enumeration contains all avaiable topologies which can be used
		to render data that is stored within vertex buffers
	*/

	enum class E_PRIMITIVE_TOPOLOGY_TYPE
	{
		PTT_POINT_LIST,
		PTT_LINE_LIST,
		PTT_TRIANGLE_LIST,
		PTT_TRIANGLE_STRIP,
		PTT_TRIANGLE_FAN
	};


	/*!
		struct TGraphicsContextInfo

		\brief The type contains information about the graphics context's environment
	*/

	typedef struct TGraphicsContextInfo
	{
		TAABB mNDCBox; ///< Determines sizes of NDC specific for the graphics API
		bool mIsTextureYCoordInverted; /// Under GL context it equals to true
		E_GRAPHICS_CONTEXT_GAPI_TYPE mGapiType;
	} TGraphicsContextInfo, *TGraphicsContextInfoPtr;


	/*!
		interface IGraphicsContext

		\brief The interface represents functionality of a low-level wrapper over GAPI
	*/

	class IGraphicsContext : public IEngineSubsystem
	{
		public:
			/*!
				\brief The method initializes an initial state of the object

				\param[in] pWindowSystem A pointer to implementation of IWindowSystem interface

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(TPtr<IWindowSystem> pWindowSystem) = 0;

			/*!
				\brief The method clears up back buffer with specified color

				\param[in] color The new color of a back buffer
			*/

			TDE2_API virtual void ClearBackBuffer(const TColor32F& color) = 0;

			/*!
				\brief The method clears up render target with specified color

				\param[in, out] pRenderTarget A pointer to IRenderTarget implementation
				\param[in] color The new color of a render target
			*/

			TDE2_API virtual void ClearRenderTarget(IRenderTarget* pRenderTarget, const TColor32F& color) = 0;

			/*!
				\brief The method clears up render target with specified color

				\param[in] slot A slot into which the render target that should be cleared up is bound
				\param[in] color The new color of a render target
			*/

			TDE2_API virtual void ClearRenderTarget(U8 slot, const TColor32F& color) = 0;

			/*!
				\brief The method clears up given depth buffer with specified values

				\param[in] value The depth buffer will be cleared with this value
				\param[in] stencilValue The stencil buffer will be cleared with this value
			*/

			TDE2_API virtual void ClearDepthBufferTarget(IDepthBufferTarget* pDepthBufferTarget, F32 value, U8 stencilValue) = 0;

			/*!
				\brief The method clears up depth buffer with specified values

				\param[in] value The depth buffer will be cleared with this value
			*/

			TDE2_API virtual void ClearDepthBuffer(F32 value) = 0;

			/*!
				\brief The method clears up stencil buffer with specified values
				
				\param[in] value The stencil buffer will be cleared with this value
			*/

			TDE2_API virtual void ClearStencilBuffer(U8 value) = 0;

			TDE2_API virtual void BeginFrame() = 0;

			/*!
				\brief The method swaps a current buffer with a back one
			*/

			TDE2_API virtual void Present() = 0;

			/*!
				\brief The method sets up a viewport's parameters

				\param[in] x x position of left hand side of a viewport
				\param[in] y y position of left (top/bottom varies on GAPI) corner of a viewport
				\param[in] width width of a viewport
				\param[in] height height of a viewport
				\param[in] minDepth minimum depth of a viewport
				\param[in] maxDepth maximum depth of a viewport

			*/

			TDE2_API virtual void SetViewport(F32 x, F32 y, F32 width, F32 height, F32 minDepth, F32 maxDepth) = 0;

			/*!
				\brief The method specifies rectangle for scissor testing, all the geometry outside of which
				is rejected

				\param[in] scissorRect A rectangle for scissor test
			*/

			TDE2_API virtual void SetScissorRect(const TRectU32& scissorRect) = 0;

			/*!
				\brief The method computes a perspective projection matrix specific for a graphics context

				\param[in] fov A field of view

				\param[in] aspect An aspect ratio of a screen

				\param[in] zn A z value of a near clip plance

				\param[in] zf A z value of a far clip plane

				\return The method computes a perspective projection matrix specific for a graphics context
			*/

			TDE2_API virtual TMatrix4 CalcPerspectiveMatrix(F32 fov, F32 aspect, F32 zn, F32 zf) = 0;

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

			TDE2_API virtual TMatrix4 CalcOrthographicMatrix(F32 left, F32 top, F32 right, F32 bottom, F32 zn, F32 zf, bool isDepthless = false) = 0;

			TDE2_API virtual E_RESULT_CODE SetVertexBuffer(U32 slot, TBufferHandleId vertexBufferHandle, U32 offset, U32 strideSize) = 0;
			TDE2_API virtual E_RESULT_CODE SetIndexBuffer(TBufferHandleId indexBufferHandle, U32 offset) = 0;
			TDE2_API virtual E_RESULT_CODE SetConstantBuffer(U32 slot, TBufferHandleId constantsBufferHandle) = 0;
			TDE2_API virtual E_RESULT_CODE SetStructuredBuffer(U32 slot, TBufferHandleId bufferHandle, bool isWriteEnabled = false) = 0;

			TDE2_API virtual E_RESULT_CODE SetTexture(U32 slot, TTextureHandleId textureHandle, bool isWriteEnabled = false) = 0;
			TDE2_API virtual E_RESULT_CODE SetSampler(U32 slot, TTextureSamplerId samplerHandle) = 0;

			TDE2_API virtual E_RESULT_CODE UpdateTexture2D(TTextureHandleId textureHandle, U32 mipLevel, const TRectI32& regionRect, const void* pData, USIZE dataSize) = 0;
			TDE2_API virtual E_RESULT_CODE UpdateTexture2DArray(TTextureHandleId textureHandle, U32 index, const TRectI32& regionRect, const void* pData, USIZE dataSize) = 0;
			TDE2_API virtual E_RESULT_CODE UpdateCubemapTexture(TTextureHandleId textureHandle, E_CUBEMAP_FACE face, const TRectI32& regionRect, const void* pData, USIZE dataSize) = 0;
			TDE2_API virtual E_RESULT_CODE UpdateTexture3D(TTextureHandleId textureHandle, U32 depthFrom, U32 depthTo, const TRectI32& regionRect, const void* pData, USIZE dataSize) = 0;

			// Blit one texture to another
			TDE2_API virtual E_RESULT_CODE CopyResource(TTextureHandleId sourceHandle, TTextureHandleId destHandle) = 0;
			// Upload data into texture
			TDE2_API virtual E_RESULT_CODE CopyResource(TBufferHandleId sourceHandle, TTextureHandleId destHandle) = 0;
			// Readback data from texture into a buffer
			TDE2_API virtual E_RESULT_CODE CopyResource(TTextureHandleId sourceHandle, TBufferHandleId destHandle) = 0;
			// Copy buffer to buffer
			TDE2_API virtual E_RESULT_CODE CopyResource(TBufferHandleId sourceHandle, TBufferHandleId destHandle) = 0;

			/*!
				\brief The method copies counter of sourceHandle buffer into destHandle's one

				\param[in] sourceHandle A handle to structured appendable buffer resource that should be created with unordered access write flag
				\param[in] destHandle A handle to any buffer resource
				\param[in] offset An offset within dest buffer
			*/

			TDE2_API virtual E_RESULT_CODE CopyCount(TBufferHandleId sourceHandle, TBufferHandleId destHandle, U32 offset) = 0;

			/*!
				\brief The method generates all mip levels for the specified texture
			*/
			
			TDE2_API virtual E_RESULT_CODE GenerateMipMaps(TTextureHandleId textureHandle) = 0;

			/*!
				\brief The method sends a command to a GPU to draw current attached graphics data

				\param[in] topology A primitive topology's type

				\param[in] startVertex An index of a vertex within a vertex buffer

				\param[in] numOfVertices A total number of vertices should be drawn
			*/

			TDE2_API virtual void Draw(E_PRIMITIVE_TOPOLOGY_TYPE topology, U32 startVertex, U32 numOfVertices) = 0;

			/*!
				\brief The method sends a command to a GPU to draw current attached graphics data using
				index buffer to group primitives
				
				\param[in] topology A primitive topology's type

				\param[in] indexFormatType A format of a single index

				\param[in] baseVertex A value that will be added to each index

				\param[in] startIndex A first index that will be read by GPU

				\param[in] numOfIndices A total number of indices
			*/

			TDE2_API virtual void DrawIndexed(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, U32 baseVertex, U32 startIndex, U32 numOfIndices) = 0;

			/*!
				\brief The method sends a command to a GPU to draw current attached graphics data that uses instancing

				\param[in] topology A primitive topology's type

				\param[in] startVertex An index of a vertex within a vertex buffer

				\param[in] verticesPerInstance A number of vertices per instance

				\param[in] startInstance An offset value which is added to every index before read the data

				\param[in] numOfInstances A total number of instances should be drawn
			*/

			TDE2_API virtual void DrawInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, U32 startVertex, U32 verticesPerInstance, U32 startInstance, U32 numOfInstances) = 0;

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

			TDE2_API virtual void DrawIndexedInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, U32 baseVertex, U32 startIndex, 
													   U32 startInstance, U32 indicesPerInstance, U32 numOfInstances) = 0;
			
			/*!
				\brief The method sends a command to a GPU with enabled instancing but all the parameters are stored within GPU buffer

				\param[in] topology A primitive topology's type
				\param[in] argsBufferHandle A GPU buffer that holds arguments for draw command
				\param[in] alignedOffset Aligned offset to the beginning of GPU data
			*/

			TDE2_API virtual void DrawIndirectInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, TBufferHandleId argsBufferHandle, U32 alignedOffset) = 0;

			/*!
				\brief The method sends a command to a GPU with enabled instancing but all the parameters are stored within GPU buffer

				\param[in] topology A primitive topology's type
				\param[in] indexFormatType A format of a single index
				\param[in] argsBufferHandle A GPU buffer that holds arguments for draw command
				\param[in] alignedOffset Aligned offset to the beginning of GPU data
			*/

			TDE2_API virtual void DrawIndirectIndexedInstanced(E_PRIMITIVE_TOPOLOGY_TYPE topology, E_INDEX_FORMAT_TYPE indexFormatType, TBufferHandleId argsBufferHandle, U32 alignedOffset) = 0;

			/*!
				\brief The method invokes a computation kernel over a given threads grid. A compute shader should be bound to the device first

				\param[in] groupsCountX A number of groups over X axis
				\param[in] groupsCountY A number of groups over Y axis
				\param[in] groupsCountZ A number of groups over Z axis
			*/

			TDE2_API virtual void DispatchCompute(U32 groupsCountX, U32 groupsCountY, U32 groupsCountZ) = 0;

			/*!
				\brief The method invokes a computation kernel over a given threads grid. A compute shader should be bound to the device first
				
				\param[in] argsBufferHandle A GPU buffer that holds arguments for draw command
				\param[in] alignedOffset Aligned offset to the beginning of GPU data
			*/

			TDE2_API virtual void DispatchIndirectCompute(TBufferHandleId argsBufferHandle, U32 alignedOffset) = 0;

			/*!
				\brief The method binds a given blend state to rendering pipeline

				\param[in] blendStateId An identifier of a blend state
			*/

			TDE2_API virtual void BindBlendState(TBlendStateId blendStateId) = 0;

			/*!
				\brief The method binds a given depth-stencil state to rendering pipeline

				\param[in] depthStencilStateId An identifier of a depth-stencil state
				\param[in] stencilRef A reference value to perform against when doing a depth-stencil test
			*/

			TDE2_API virtual void BindDepthStencilState(TDepthStencilStateId depthStencilStateId, U8 stencilRef = 0x0) = 0;

			/*!
				\brief The method binds a given rasteriazer state to rendering pipeline

				\param[in] rasterizerStateId An identifier of a rasterizer state
			*/

			TDE2_API virtual void BindRasterizerState(TRasterizerStateId rasterizerStateId) = 0;

			/*!
				\brief The method binds a given render target object to rendering pipeline

				\param[in] slot An index of the slot into which the render target will be bound

				\param[in, out] pRenderTarget A pointer to IRenderTarget implementation
			*/

			TDE2_API virtual void BindRenderTarget(U8 slot, IRenderTarget* pRenderTarget) = 0;

			/*!
				\brief The method binds a given render target object to rendering pipeline

				\param[in] slot An index of the slot into which the render target will be bound

				\param[in] targetHandle Handle to texture object that's created as a render target
			*/

			TDE2_API virtual void BindRenderTarget(U8 slot, TTextureHandleId targetHandle) = 0;

			/*!
				\brief The method binds a given depth buffer to rendering pipeline

				\param[in, out] pDepthBufferTarget A pointer to IDepthBufferTarget implementation
				\param[in] disableRTWrite A flag determines whether the write to RT should be enabled or not
			*/

			TDE2_API virtual void BindDepthBufferTarget(IDepthBufferTarget* pDepthBufferTarget, bool disableRTWrite = false) = 0;

			/*!
				\brief The method binds a given depth buffer to rendering pipeline

				\param[in] targetHandle Handle to texture object that's created as a depth buffer
				\param[in] disableRTWrite A flag determines whether the write to RT should be enabled or not
			*/

			TDE2_API virtual void BindDepthBufferTarget(TTextureHandleId targetHandle, bool disableRTWrite = false) = 0;

			/*!
				\brief The method disables or enables a depth buffer usage

				\param[in] value If true the depth buffer will be used, false turns off it
			*/

			TDE2_API virtual void SetDepthBufferEnabled(bool value) = 0;

#if TDE2_DEBUG_MODE
			TDE2_API virtual void BeginSectionMarker(const std::string& id) = 0;
			TDE2_API virtual void EndSectionMarker() = 0;
#endif

			/*!
				\brief The method returns an object that contains internal handlers that are used by the system.

				The structure of the object and its members can vary on different platforms.

				return The method returns an object that contains internal handlers that are used by the system
			*/

			TDE2_API virtual const TGraphicsCtxInternalData& GetInternalData() const = 0;

			/*!
				\brief The method returns a pointer to IGraphicsObjectManager implementation

				\return The method returns a pointer to IGraphicsObjectManager implementation
			*/

			TDE2_API virtual IGraphicsObjectManager* GetGraphicsObjectManager() const = 0;

			/*!
				\brief The method returns 1 if a Z axis is collinear to (0; 0; 1) direction, -1 in other case

				\return The method returns 1 if a Z axis is collinear to (0; 0; 1) direction, -1 in other case
			*/

			TDE2_API virtual float GetPositiveZAxisDirection() const = 0;

			/*!
				\brief The method returns an information about currently using video adapter

				\return The method returns an information about currently using video adapter
			*/

			TDE2_API virtual TVideoAdapterInfo GetInfo() const = 0;

			/*
				\bief The method returns information about GAPI environment

				\return The method returns data that describes all GAPI related information
			*/

			TDE2_API virtual const TGraphicsContextInfo& GetContextInfo() const = 0;

			TDE2_API virtual std::vector<U8> GetBackBufferData() const = 0;

			/*!
				\return The method returns a pointer to IWindowSystem
			*/

			TDE2_API virtual TPtr<IWindowSystem> GetWindowSystem() const = 0;

			TDE2_API static E_ENGINE_SUBSYSTEM_TYPE GetTypeID() { return EST_GRAPHICS_CONTEXT; }
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IGraphicsContext)
	};


	TDE2_DECLARE_SCOPED_PTR(IGraphicsContext);
}