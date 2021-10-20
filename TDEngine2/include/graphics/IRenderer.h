/*!
	\file IRenderer.h
	\date 07.12.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Utils.h"
#include "../utils/Types.h"
#include "../core/IEngineSubsystem.h"


namespace TDEngine2
{
	class IGraphicsContext;
	class ICamera;
	class CRenderQueue;
	class IAllocator;
	class IResourceManager;
	class IFramePostProcessor;
	class ISelectionManager;
	struct TLightingShaderData;
	class IGlobalShaderProperties;


	TDE2_DECLARE_SCOPED_PTR(IGraphicsContext)
	TDE2_DECLARE_SCOPED_PTR(IResourceManager)
	TDE2_DECLARE_SCOPED_PTR(IAllocator)


	enum class E_RENDER_QUEUE_GROUP: U8
	{
		RQG_EDITOR_ONLY, ///< The queue is used for editor stuffs and not used in production builds
		RQG_SHADOW_PASS,
		RQG_OPAQUE_GEOMETRY,
		RQG_TRANSPARENT_GEOMETRY,
		RQG_SPRITES,
		RQG_OVERLAY,
		RQG_DEBUG,
		RQG_LAST_GROUP  = RQG_DEBUG,
		RQG_FIRST_GROUP = RQG_OPAQUE_GEOMETRY
	};

	constexpr U8 NumOfRenderQueuesGroup = static_cast<U32>(E_RENDER_QUEUE_GROUP::RQG_LAST_GROUP) + 1;


	/*!
		\brief The enumeration contains flags that are used to define subgroups within a group which is formed by E_RENDER_QUEUE_GROUP 
	*/

	enum class E_GEOMETRY_SUBGROUP_TAGS: U32
	{
		BASE              = 0x0,
		SKYBOX            = 0x1000,
		SELECTION_OUTLINE = 0xFFFFFFF0,
		IMAGE_EFFECTS     = 0xFFFF0000,
	};


	struct TRendererInitParams
	{
		TPtr<IGraphicsContext> mpGraphicsContext;
		TPtr<IResourceManager> mpResourceManager;
		TPtr<IAllocator>       mpTempAllocator;
		IFramePostProcessor*   mpFramePostProcessor;
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
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(const TRendererInitParams& params) = 0;
			
			/*!
				\brief The method sends all accumulated commands into GPU driver

				\param[in] currTime Time elapsed since application launch
				\param[in] deltaTime Time elapsed since last frame was rendered

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Draw(F32 currTime, F32 deltaTime) = 0;

			/*!
				\brief The method attaches a camera to the renderer

				\param[in] pCaemra A pointer to ICamera implementation
			*/

			TDE2_API virtual void SetCamera(const ICamera* pCamera) = 0;

			/*!
				\brief The method assigns a pointer to frame post-processor 

				\param[in, out] A pointer to IFramePostProcessor implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetFramePostProcessor(IFramePostProcessor* pFramePostProcessor) = 0;

			/*!
				\brief The method sets up a pointer to selection manager

				\param[in, out] pSelectionManager A pointer to ISelectionManager implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetSelectionManager(ISelectionManager* pSelectionManager) = 0;

			/*!
				\brief The method stores given data that will be passed into the shaders to compute lighting and shadows

				\param[in] lightingData A parameter that contains all information about light sources

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetLightingData(const TLightingShaderData& lightingData) = 0;

			/*!
				\brief The method returns a pointer to CRenderQueue which contains objects of specific group

				\param[in] queueType A type of objects that the given queue stores

				\return The method returns a pointer to CRenderQueue which contains objects of specific group
			*/

			TDE2_API virtual CRenderQueue* GetRenderQueue(E_RENDER_QUEUE_GROUP queueType) const = 0;

			/*!
				\brief The method returns a pointer to an instance of IResourceManager which is attached to
				the renderer
			*/

			TDE2_API virtual TPtr<IResourceManager> GetResourceManager() const = 0;

			/*!
				\return The method returns a pointer to an object which holds global shader uniforms
			*/

			TDE2_API virtual IGlobalShaderProperties* GetGlobalShaderProperties() const = 0;

			TDE2_API static E_ENGINE_SUBSYSTEM_TYPE GetTypeID() { return EST_RENDERER; }
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IRenderer)
	};


	TDE2_DECLARE_SCOPED_PTR_INLINED(IRenderer)
}