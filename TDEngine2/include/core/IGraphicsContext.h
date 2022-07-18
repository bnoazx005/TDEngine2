/*!
	\file IGraphicsContext.h
	\date 20.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "IEngineSubsystem.h"
#include "../utils/Color.h"
#include "../math/TMatrix4.h"
#include "../math/TRect.h"
#include "../math/TAABB.h"
#include "../graphics/IIndexBuffer.h"


namespace TDEngine2
{
	class IWindowSystem;
	class IGraphicsObjectManager;
	class IRenderTarget;
	class IDepthBufferTarget;


	TDE2_DECLARE_SCOPED_PTR(IWindowSystem)


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

		\todo More proper set up of context's initial state should be done (depth/stencil buffer configuration,
		MSAA, etc)
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
				\brief The method invokes a computation kernel over a given threads grid. A compute shader should be bound to the device first

				\param[in] groupsCountX A number of groups over X axis
				\param[in] groupsCountY A number of groups over Y axis
				\param[in] groupsCountZ A number of groups over Z axis
			*/

			TDE2_API virtual void DispatchCompute(U32 groupsCountX, U32 groupsCountY, U32 groupsCountZ) = 0;

			/*!
				\brief The method binds a given texture sampler to a specified slot

				\param[in] slot An input slot's index

				\param[in] samplerId An identifier of a texture sampler
			*/

			TDE2_API virtual void BindTextureSampler(U32 slot, TTextureSamplerId samplerId) = 0;

			/*!
				\brief The method binds a given blend state to rendering pipeline

				\param[in] blendStateId An identifier of a blend state
			*/

			TDE2_API virtual void BindBlendState(TBlendStateId blendStateId) = 0;

			/*!
				\brief The method binds a given depth-stencil state to rendering pipeline

				\param[in] depthStencilStateId An identifier of a depth-stencil state
			*/

			TDE2_API virtual void BindDepthStencilState(TDepthStencilStateId depthStencilStateId) = 0;

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
				\brief The method binds a given depth buffer to rendering pipeline

				\param[in, out] pDepthBufferTarget A pointer to IDepthBufferTarget implementation
				\param[in] disableRTWrite A flag determines whether the write to RT should be enabled or not
			*/

			TDE2_API virtual void BindDepthBufferTarget(IDepthBufferTarget* pDepthBufferTarget, bool disableRTWrite = false) = 0;

			/*!
				\brief The method disables or enables a depth buffer usage

				\param[in] value If true the depth buffer will be used, false turns off it
			*/

			TDE2_API virtual void SetDepthBufferEnabled(bool value) = 0;

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