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


	/*!
		\brief A factory function for creation objects of CForwardRenderer's type

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext implementation

		\param[in, out] pGraphicsContext A pointer to IResourceManager implementation

		\param[in, out] pTempAllocator A pointer to IAllocator object which will be used
		for temporary allocations

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CForwardRenderer's implementation
	*/

	TDE2_API IRenderer* CreateForwardRenderer(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, IAllocator* pTempAllocator, 
											  IFramePostProcessor* pFramePostProcessor, E_RESULT_CODE& result);


	/*!
		interface CForwardRenderer

		\brief The interface represents a functionality of a renderer
	*/

	class CForwardRenderer : public IRenderer, public CBaseObject
	{
		public:
			friend TDE2_API IRenderer* CreateForwardRenderer(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, IAllocator* pTempAllocator, 
															 IFramePostProcessor* pFramePostProcessor, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a renderer

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext implementation

				\param[in, out] pGraphicsContext A pointer to IResourceManager implementation

				\param[in, out] pTempAllocator A pointer to IAllocator object which will be used
				for temporary allocations

				\param[in, out] pFramePostProcessor A pointer to IFramePostProcessor implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, IAllocator* pTempAllocator,
										IFramePostProcessor* pFramePostProcessor) override;
			
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
				\brief The method assigns a pointer to frame post-processor

				\param[in, out] A pointer to IFramePostProcessor implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetFramePostProcessor(IFramePostProcessor* pFramePostProcessor) override;

			/*!
				\brief The method stores given data that will be passed into the shaders to compute lighting and shadows

				\param[in] lightingData A parameter that contains all information about light sources

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetLightingData(const TLightingShaderData& lightingData) override;

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

			TDE2_API CRenderQueue* GetRenderQueue(E_RENDER_QUEUE_GROUP queueType) const override;

			/*!
				\brief The method returns a pointer to an instance of IResourceManager which is attached to
				the renderer

				\return The method returns a pointer to an instance of IResourceManager which is attached to
				the renderer
			*/

			TDE2_API IResourceManager* GetResourceManager() const override;

			/*!
				\return The method returns a pointer to an object which holds global shader uniforms
			*/

			TDE2_API IGlobalShaderProperties* GetGlobalShaderProperties() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CForwardRenderer)

			TDE2_API void _submitToDraw(CRenderQueue* pRenderQueue, U32 upperRenderIndexLimit);

			TDE2_API void _prepareFrame(F32 currTime, F32 deltaTime);

			TDE2_API E_RESULT_CODE _onFreeInternal() override;
		protected:
			bool                     mIsInitialized;
							         
			IGraphicsContext*        mpGraphicsContext;
							         
			IResourceManager*        mpResourceManager;
							         
			const ICamera*           mpMainCamera;
							         
			CRenderQueue*            mpRenderQueues[NumOfRenderQueuesGroup];
							         
			IAllocator*              mpTempAllocator;

			IGlobalShaderProperties* mpGlobalShaderProperties;

			IDebugUtility*           mpDebugUtility;

			IFramePostProcessor*     mpFramePostProcessor;

			ISelectionManager*       mpSelectionManager;

			TLightingShaderData      mLightingData;

			TResourceId              mShadowMapHandle;

			U32                      mShadowMapWidth;
			U32                      mShadowMapHeight;
	};
}