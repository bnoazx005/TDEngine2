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
	class IGlobalShaderProperties;
	class IRenderTarget;


	TDE2_DECLARE_SCOPED_PTR(IResourceManager)


	enum class TBufferHandleId : U32;


	/*!
		\brief A factory function for creation objects of CFramePostProcessor's type

		/param[in] desc A set of parameters that're needed to initialize the object
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CFramePostProcessor's implementation
	*/

	TDE2_API IFramePostProcessor* CreateFramePostProcessor(const TFramePostProcessorParameters& desc, E_RESULT_CODE& result);


	/*!
		class CFramePostProcessor

		\brief The class is a basic implementation of default in-engine frame post-processor
	*/

	class CFramePostProcessor : public CBaseObject, public IFramePostProcessor
	{
		public:
			friend TDE2_API IFramePostProcessor* CreateFramePostProcessor(const TFramePostProcessorParameters&, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of the processor

				/param[in] desc A set of parameters that're needed to initialize the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(const TFramePostProcessorParameters& desc) override;

			/*!
				\brief The method assigns a processing profile into the processor

				\param[in] pProfileResource A pointer to IPostProcessingProfile implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetProcessingProfile(const IPostProcessingProfile* pProfileResource) override;

			/*!
				\brief he method renders all the geometry into an off-screen target that later will be used as post-processing
				source

				\param[in] onRenderFrameCallback A callback in which all the geometry of the scene should be rendered

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Render(const TRenderFrameCallback& onRenderFrameCallback, bool clearRenderTarget = true, bool bindDepthBuffer = false) override;

			TDE2_API E_RESULT_CODE PreRender() override;
			TDE2_API E_RESULT_CODE RunPostProcess() override;
			TDE2_API E_RESULT_CODE PostRender() override;

			TDE2_API E_RESULT_CODE RunVolumetricCloudsPass() override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CFramePostProcessor)

			TDE2_API void _submitFullScreenTriangle(CRenderQueue* pRenderQueue, TResourceId materialHandle, bool drawImmediately = false);
			TDE2_API void _renderTargetToTarget(TPtr<IRenderTarget> pSource, TPtr<IRenderTarget> pExtraSource, TPtr<IRenderTarget> pDest, TResourceId materialHandle);

			TDE2_API void _prepareRenderTargetsChain(U32 width, U32 height, bool isHDRSupport = false);
			TDE2_API void _resizeRenderTargetsChain(U32 width, U32 height);

			TDE2_API void _processBloomPass(TPtr<IRenderTarget> pFrontTarget, TPtr<IRenderTarget> pBackTarget, TPtr<IRenderTarget> pBloomTarget);

			TDE2_API TResourceId _getRenderTarget(U32 width, U32 height, bool isHDRSupport = false, bool isMainTarget = true);
		protected:
			CRenderQueue*                 mpPreUIRenderQueue;
			CRenderQueue*                 mpOverlayRenderQueue;

			const IPostProcessingProfile* mpCurrPostProcessingProfile;

			TResourceId                   mDefaultScreenSpaceMaterialHandle;
			TResourceId                   mBloomFilterMaterialHandle;
			TResourceId                   mBloomFinalPassMaterialHandle;
			TResourceId                   mGaussianBlurMaterialHandle;
			TResourceId                   mToneMappingPassMaterialHandle;
			TResourceId                   mGenerateLuminanceMaterialHandle;
			TResourceId                   mLuminanceAdaptationMaterialHandle;
			TResourceId                   mVolumetricCloudsComputeShaderHandle;

			TResourceId                   mRenderTargetHandle;
			TResourceId                   mBloomRenderTargetHandle;
			TResourceId                   mTemporaryRenderTargetHandle;
			TResourceId                   mMainDepthBufferHandle;
			TResourceId                   mLuminanceTargetHandle;
			TResourceId                   mVolumetricCloudsScreenBufferHandle;

			std::array<TResourceId, 2>    mFramesLuminanceHistoryTargets;
			USIZE                         mCurrActiveLuminanceFrameTargetIndex = 0;

			IVertexDeclaration*           mpVertexFormatDeclaration;

			TPtr<IResourceManager>        mpResourceManager;

			IWindowSystem*                mpWindowSystem;

			IGraphicsContext*             mpGraphicsContext;

			TBufferHandleId               mFullScreenTriangleVertexBufferHandle; // \note This buffer is only needed when we work with GL GAPI, D3D11 uses bufferless rendering
			
			IGlobalShaderProperties*      mpGlobalShaderProperties;
	};
}
