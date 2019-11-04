/*!
	\file IGraphicsContext.h
	\date 20.09.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "IEngineSubsystem.h"
#include "./../utils/Color.h"
#include "./../math/TMatrix4.h"
#include "./../graphics/IIndexBuffer.h"


namespace TDEngine2
{
	class IWindowSystem;
	class IGraphicsObjectManager;
	class IRenderTarget;


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

			TDE2_API virtual E_RESULT_CODE Init(IWindowSystem* pWindowSystem) = 0;

			/*!
				\brief The method clears up back buffer with specified color

				\param[in] color The new color of a back buffer
			*/

			TDE2_API virtual void ClearBackBuffer(const TColor32F& color) = 0;

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

				\return The method computes a orthographic projection matrix specific for a graphics context
			*/

			TDE2_API virtual TMatrix4 CalcOrthographicMatrix(F32 left, F32 top, F32 right, F32 bottom, F32 zn, F32 zf) = 0;

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
				\brief The method binds a given render target object to rendering pipeline

				\param[in, out] pRenderTarget A pointer to IRenderTarget implementation
			*/

			TDE2_API virtual void BindRenderTarget(IRenderTarget* pRenderTarget) = 0;

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

			TDE2_API static E_ENGINE_SUBSYSTEM_TYPE GetTypeID() { return EST_GRAPHICS_CONTEXT; }
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IGraphicsContext)
	};
}