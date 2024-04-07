/*!
	\file CForwardRenderer.h
	\date 07.12.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Utils.h"
#include "../utils/Types.h"
#include "../core/CBaseObject.h"
#include "IRenderer.h"
#include "InternalShaderData.h"


namespace TDEngine2
{
	class IGlobalShaderProperties;
	class IDebugUtility;
	class IFramePostProcessor;
	class CFrameGraph;


	TDE2_DECLARE_SCOPED_PTR(IGlobalShaderProperties);
	TDE2_DECLARE_SCOPED_PTR(CRenderQueue);
	TDE2_DECLARE_SCOPED_PTR(IFramePostProcessor)
	TDE2_DECLARE_SCOPED_PTR(CFrameGraph)


	/*!
		\brief A factory function for creation objects of CForwardRenderer's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CForwardRenderer's implementation
	*/

	TDE2_API IRenderer* CreateForwardRenderer(const TRendererInitParams& params, E_RESULT_CODE& result);


	/*!
		interface CForwardRenderer

		\brief The interface represents a functionality of a renderer
	*/

	class CForwardRenderer : public IRenderer, public CBaseObject
	{
		public:
			friend TDE2_API IRenderer* CreateForwardRenderer(const TRendererInitParams&, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an internal state of a renderer

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(const TRendererInitParams& params) override;
			
			/*!
				\brief The method sends all accumulated commands into GPU driver

				\param[in] currTime Time elapsed since application launch
				\param[in] deltaTime Time elapsed since last frame was rendered

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Draw(F32 currTime, F32 deltaTime) override;

			/*!
				\brief The method attaches a camera to the renderer

				\param[in] pCaemra A pointer to ICamera implementation
			*/

			TDE2_API void SetCamera(const ICamera* pCamera) override;

			/*!
				\brief The method assigns a processing profile that defines post processing parameters

				\param[in] pProfileResource A pointer to IPostProcessingProfile implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetPostProcessProfile(const IPostProcessingProfile* pProfileResource) override;

			/*!
				\brief The method stores given data that will be passed into the shaders to compute lighting and shadows

				\param[in] commonLightData An object contains common lighting data plus sun light's information
				\param[in] activeLightSources The vector contains all active light sources including point/spot lights

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetLightingData(const TLightingShaderData& commonLightData, const TLightsDataArray& activeLightSources) override;

			/*!
				\brief The method sets up a pointer to selection manager

				\param[in, out] pSelectionManager A pointer to ISelectionManager implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetSelectionManager(ISelectionManager* pSelectionManager) override;

			/*!
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			TDE2_API E_ENGINE_SUBSYSTEM_TYPE GetType() const override;

			/*!
				\brief The method returns a pointer to CRenderQueue which contains objects of specific group

				\param[in] queueType A type of objects that the given queue stores

				\return The method returns a pointer to CRenderQueue which contains objects of specific group
			*/

			TDE2_API CRenderQueue* GetRenderQueue(E_RENDER_QUEUE_GROUP queueType) override;

			/*!
				\brief The method returns a pointer to an instance of IResourceManager which is attached to
				the renderer

				\return The method returns a pointer to an instance of IResourceManager which is attached to
				the renderer
			*/

			TDE2_API TPtr<IResourceManager> GetResourceManager() const override;

			/*!
				\return The method returns a pointer to an object which holds global shader uniforms
			*/

			TDE2_API TPtr<IGlobalShaderProperties> GetGlobalShaderProperties() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CForwardRenderer)

			TDE2_API void _prepareFrame(F32 currTime, F32 deltaTime);
		protected:
			TPtr<IGraphicsContext>        mpGraphicsContext;
			TPtr<IResourceManager>        mpResourceManager;
			TPtr<IWindowSystem>           mpWindowSystem;
							         
			const ICamera*                mpMainCamera;
							         
			TPtr<CRenderQueue>            mpRenderQueues[NumOfRenderQueuesGroup];
							         
			TPtr<IAllocator>              mpTempAllocator;

			TPtr<IGlobalShaderProperties> mpGlobalShaderProperties;

			IDebugUtility*                mpDebugUtility;

			TPtr<IFramePostProcessor>     mpFramePostProcessor;

			ISelectionManager*            mpSelectionManager;

			TLightingShaderData           mCommonLightingData;
			TLightsDataArray              mActiveLightSources;

			TPtr<CFrameGraph>             mpFrameGraph;
	};
}