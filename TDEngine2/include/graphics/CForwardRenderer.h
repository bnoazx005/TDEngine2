/*!
	\file CForwardRenderer.h
	\date 07.12.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Utils.h"
#include "./../utils/Types.h"
#include "IRenderer.h"


namespace TDEngine2
{
	class IGlobalShaderProperties;


	/*!
		\brief A factory function for creation objects of CForwardRenderer's type

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext implementation

		\param[in, out] pGraphicsContext A pointer to IResourceManager implementation

		\param[in, out] pTempAllocator A pointer to IAllocator object which will be used
		for temporary allocations

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CForwardRenderer's implementation
	*/

	TDE2_API IRenderer* CreateForwardRenderer(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, IAllocator* pTempAllocator, E_RESULT_CODE& result);


	/*!
		interface CForwardRenderer

		\brief The interface represents a functionality of a renderer
	*/

	class CForwardRenderer : public IRenderer
	{
		public:
			friend TDE2_API IRenderer* CreateForwardRenderer(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, IAllocator* pTempAllocator, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a renderer

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext implementation

				\param[in, out] pGraphicsContext A pointer to IResourceManager implementation

				\param[in, out] pTempAllocator A pointer to IAllocator object which will be used
				for temporary allocations

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, IAllocator* pTempAllocator) override;
			
			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method sends all accumulated commands into GPU driver

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Draw() override;

			/*!
				\brief The method attaches a camera to the renderer

				\param[in] pCaemra A pointer to ICamera implementation
			*/

			TDE2_API void SetCamera(const ICamera* pCamera) override;

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
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CForwardRenderer)

			TDE2_API void _submitToDraw(CRenderQueue* pRenderQueue);

			TDE2_API void _prepareFrame();
		protected:
			bool                     mIsInitialized;
							         
			IGraphicsContext*        mpGraphicsContext;
							         
			IResourceManager*        mpResourceManager;
							         
			const ICamera*           mpMainCamera;
							         
			CRenderQueue*            mpRenderQueues[NumOfRenderQueuesGroup];
							         
			IAllocator*              mpTempAllocator;

			IGlobalShaderProperties* mpGlobalShaderProperties;
	};
}