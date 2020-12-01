/*!
	\file CFramePostProcessor.h
	\date 11.02.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "IFramePostProcessor.h"
#include "../core/CBaseObject.h"


namespace TDEngine2
{
	class CRenderQueue;
	class IResourceManager;
	class IGraphicsContext;
	class IVertexDeclaration;
	class IVertexBuffer;


	/*!
		\brief A factory function for creation objects of CFramePostProcessor's type

		/param[in, out] pRenderer A pointer to IRenderer implementation
		/param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation
		/param[in, out] pWindowSystem A pointer to IWindowSystem implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CFramePostProcessor's implementation
	*/

	TDE2_API IFramePostProcessor* CreateFramePostProcessor(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, IWindowSystem* pWindowSystem, E_RESULT_CODE& result);


	/*!
		class CFramePostProcessor

		\brief The class is a basic implementation of default in-engine frame post-processor
	*/

	class CFramePostProcessor : public CBaseObject, public IFramePostProcessor
	{
		public:
			friend TDE2_API IFramePostProcessor* CreateFramePostProcessor(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, 
																		  IWindowSystem* pWindowSystem, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of the processor

				/param[in, out] pRenderer A pointer to IRenderer implementation
				/param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation
				/param[in, out] pWindowSystem A pointer to IWindowSystem implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, IWindowSystem* pWindowSystem) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method assigns a processing profile into the processor

				\param[in] pProfileResource A pointer to IPostProcessingProfile implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetProcessingProfile(const IPostProcessingProfile* pProfileResource) override;

			/*!
				\brief The method implements all the logic that should be done before the actual frame'll be drawn

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE PreRender() override;

			/*!
				\brief The method implements all the logic that should be done when all geometry is rendered and
				post processing effects are applied

				\param[in] onRenderFrameCallback A callback in which all the geometry of the scene should be rendered

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Render(const TRenderFrameCallback& onRenderFrameCallback) override;

			/*!
				\brief The method implements all the logic that should be done after the actual frame'll be drawn

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE PostRender() override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CFramePostProcessor)

			TDE2_API void _submitFullScreenTriangle(CRenderQueue* pRenderQueue);

			TDE2_API TResourceId _getRenderTarget(U32 width, U32 height, bool isHDRSupport = false);
		protected:
			CRenderQueue*                 mpOverlayRenderQueue;

			const IPostProcessingProfile* mpCurrPostProcessingProfile;

			TResourceId                   mDefaultScreenSpaceMaterialHandle;
			TResourceId                   mRenderTargetHandle;

			IVertexDeclaration*           mpVertexFormatDeclaration;

			IResourceManager*             mpResourceManager;

			IWindowSystem*                mpWindowSystem;

			IGraphicsContext*             mpGraphicsContext;

			IVertexBuffer*                mpFullScreenTriangleVertexBuffer; // \note This buffer is only needed when we work with GL GAPI, D3D11 uses bufferless rendering
	};
}
