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
	class IGraphicsContext;
	class ICamera;
	class CRenderQueue;
	class IAllocator;
	class IResourceManager;
	class IFramePostProcessor;


	enum class E_RENDER_QUEUE_GROUP: U8
	{
		RQG_OPAQUE_GEOMETRY,
		RQG_TRANSPARENT_GEOMETRY,
		RQG_SPRITES,
		RQG_DEBUG,
		RQG_OVERLAY,
		RQG_LAST_GROUP  = RQG_OVERLAY,
		RQG_FIRST_GROUP = RQG_OPAQUE_GEOMETRY
	};

	constexpr U8 NumOfRenderQueuesGroup = static_cast<U32>(E_RENDER_QUEUE_GROUP::RQG_LAST_GROUP) + 1;


	/*!
		\brief The enumeration contains flags that are used to define subgroups within a group which is formed by E_RENDER_QUEUE_GROUP 
	*/

	enum class E_GEOMETRY_SUBGROUP_TAGS: U32
	{
		BASE   = 0x0,
		SKYBOX = 0x1000,
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
				
				\param[in, out] pGraphicsContext A pointer to IGraphicsContext implementation

				\param[in, out] pResourceManager A pointer to IResourceManager implementation

				\param[in, out] pTempAllocator A pointer to IAllocator object which will be used
				for temporary allocations

				\param[in, out] pFramePostProcessor A pointer to IFramePostProcessor implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, IAllocator* pTempAllocator,
												IFramePostProcessor* pFramePostProcessor) = 0;
			
			/*!
				\brief The method sends all accumulated commands into GPU driver

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Draw() = 0;

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

			/*!
				\brief The method returns a pointer to an instance of IResourceManager which is attached to
				the renderer

				\return The method returns a pointer to an instance of IResourceManager which is attached to
				the renderer
			*/

			TDE2_API virtual IResourceManager* GetResourceManager() const = 0;

			TDE2_API static E_ENGINE_SUBSYSTEM_TYPE GetTypeID() { return EST_RENDERER; }
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IRenderer)
	};
}