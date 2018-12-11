/*!
	\file IRenderer.h
	\date 07.12.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Utils.h"
#include "./../utils/Types.h"
#include "./../core/IEngineSubsystem.h"


namespace TDEngine2
{
	class IRenderable;
	class ICamera;
	class CRenderQueue;
	class IAllocator;


	enum class E_RENDER_QUEUE_GROUP: U8
	{
		RQG_OPAQUE_GEOMETRY,
		RQG_TRANSPARENT_GEOMETRY,
		RQG_SPRITES,
		RQG_DEBUG,
		RQG_OVERLAY,
		RQG_LAST_GROUP
	};


	/*!
		interface IRenderer

		\brief The interface represents a functionality of a renderer
	*/

	class IRenderer: public IEngineSubsystem
	{
		public:
			/*!
				\brief The method initializes an internal state of a renderer
				
				\param[in, out] pTempAllocator A pointer to IAllocator object which will be used
				for temporary allocations

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IAllocator* pTempAllocator) = 0;

			/*!
				\brief The method submits a given renderable into the renderer

				\param[in] group A group's identifier value

				\param[in] pRenderable A  pointer to IRenderable implementation
			*/

			TDE2_API virtual void SubmitToRender(U8 group, const IRenderable* pRenderable) = 0;

			/*!
				\brief The method attaches a camera to the renderer

				\param[in] pCaemra A pointer to ICamera implementation
			*/

			TDE2_API virtual void SetCamera(const ICamera* pCamera) = 0;

			/*!
				\brief The method returns a pointer to CRenderQueue which contains objects of specific group

				\param[in] queueType A type of objects that the given queue stores

				\return The method returns a pointer to CRenderQueue which contains objects of specific group
			*/

			TDE2_API virtual CRenderQueue* GetRenderQueue(E_RENDER_QUEUE_GROUP queueType) const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IRenderer)
	};
}