#include "../../include/graphics/CForwardRenderer.h"
#include "../../include/graphics/ICamera.h"
#include "../../include/graphics/CRenderQueue.h"
#include "../../include/graphics/CGlobalShaderProperties.h"
#include "../../include/graphics/InternalShaderData.h"
#include "../../include/graphics/CDebugUtility.h"
#include "../../include/graphics/IMaterial.h"
#include "../../include/graphics/IShader.h"
#include "../../include/graphics/IVertexDeclaration.h"
#include "../../include/graphics/IPostProcessingProfile.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/graphics/CBaseRenderTarget.h"
#include "../../include/graphics/CBaseCubemapTexture.h"
#include "../../include/graphics/CFrameGraph.h"
#include "../../include/core/memory/IAllocator.h"
#include "../../include/core/memory/CLinearAllocator.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IWindowSystem.h"
#include "../../include/core/CGameUserSettings.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/editor/CPerfProfiler.h"
#include "../../include/editor/CStatsCounters.h"
#if TDE2_EDITORS_ENABLED
	#include "../../include/editor/ISelectionManager.h"
#endif


namespace TDEngine2
{
	typedef struct TFramePostProcessorParameters
	{
		IRenderer* mpRenderer;
		IGraphicsObjectManager* mpGraphicsObjectManager;
		IWindowSystem* mpWindowSystem;
	} TFramePostProcessorParameters, * TFramePostProcessorParametersPtr;


	enum class E_FRAME_RENDER_PARAMS_FLAGS : U32
	{
		CLEAR_RENDER_TARGET = 1 << 0,
		BIND_DEPTH_BUFFER = 1 << 1,
		RENDER_MAIN = 1 << 2,
		RENDER_UI = 1 << 3,
		NONE = 0
	};


	TDE2_DECLARE_BITMASK_OPERATORS_INTERNAL(E_FRAME_RENDER_PARAMS_FLAGS);


	/*!
		interface IFramePostProcessor

		\brief The interface describes a functionality of frames post-processor
	*/

	class IFramePostProcessor : public virtual IBaseObject
	{
		public:
			typedef std::function<void()> TRenderFrameCallback;
		public:
			/*!
				\brief The method initializes an internal state of the processor

				/param[in] desc A set of parameters that're needed to initialize the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(const TFramePostProcessorParameters& desc) = 0;

			/*!
				\brief The method assigns a processing profile into the processor

				\param[in] pProfileResource A pointer to IPostProcessingProfile implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetProcessingProfile(const IPostProcessingProfile* pProfileResource) = 0;

			/*!
				\brief The method renders all the geometry into an off-screen target that later will be used as post-processing
				source. It could be used multiple times during single frame to accumulate new draw calls on top previous ones

				\param[in] onRenderFrameCallback A callback in which all the geometry of the scene should be rendered

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Render(const TRenderFrameCallback& onRenderFrameCallback, E_FRAME_RENDER_PARAMS_FLAGS flags) = 0;

			/*!
				\brief The method prepares render targets and materials that're involved into post-processings

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE PreRender() = 0;

			/*!
				\brief The actual post-processing stages (bloom, AA) are run through this call

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE RunPostProcess() = 0;

			/*!
				\brief The method prepares last draw call with full-screen quad with tone mapping material
				to output result onto the screen

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE PostRender() = 0;

			TDE2_API virtual E_RESULT_CODE RunVolumetricCloudsPass() = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IFramePostProcessor)
	};


	TDE2_DECLARE_SCOPED_PTR(IFramePostProcessor)
	TDE2_DEFINE_SCOPED_PTR(IFramePostProcessor)


	/*!
		\brief CFramePostProcessor's definition
	*/

	/*!
		\brief A factory function for creation objects of CFramePostProcessor's type

		/param[in] desc A set of parameters that're needed to initialize the object
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CFramePostProcessor's implementation
	*/

	static IFramePostProcessor* CreateFramePostProcessor(const TFramePostProcessorParameters& desc, E_RESULT_CODE& result);


	TDE2_STATIC_CONSTEXPR U32 LuminanceTargetSizes = 1024;


	/*!
		class CFramePostProcessor

		\brief The class is a basic implementation of default in-engine frame post-processor
	*/

	class CFramePostProcessor : public CBaseObject, public IFramePostProcessor
	{
		public:
			friend IFramePostProcessor* CreateFramePostProcessor(const TFramePostProcessorParameters&, E_RESULT_CODE& result);
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

			TDE2_API E_RESULT_CODE Render(const TRenderFrameCallback& onRenderFrameCallback, E_FRAME_RENDER_PARAMS_FLAGS flags) override;

			TDE2_API E_RESULT_CODE PreRender() override;
			TDE2_API E_RESULT_CODE RunPostProcess() override;
			TDE2_API E_RESULT_CODE PostRender() override;

			TDE2_API E_RESULT_CODE RunVolumetricCloudsPass() override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CFramePostProcessor)

			TDE2_API void _submitFullScreenTriangle(CRenderQueue* pRenderQueue, TResourceId materialHandle, bool drawImmediately = false);
			TDE2_API void _renderTargetToTarget(TPtr<IRenderTarget> pSource, ITexture* pExtraSource, TPtr<IRenderTarget> pDest, TResourceId materialHandle);

			TDE2_API void _prepareRenderTargetsChain(U32 width, U32 height, bool isHDRSupport = false);
			TDE2_API void _resizeRenderTargetsChain(U32 width, U32 height);

			TDE2_API void _processBloomPass(TPtr<IRenderTarget> pFrontTarget, TPtr<IRenderTarget> pBackTarget, TPtr<IRenderTarget> pBloomTarget);
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
			TResourceId                   mVolumetricCloudsUpsampleBlurShaderHandle;
			TResourceId                   mVolumetricCloudsComposeMaterialHandle;

			TResourceId                   mRenderTargetHandle;
			TResourceId                   mUITargetHandle;
			TResourceId                   mBloomRenderTargetHandle;
			TResourceId                   mTemporaryRenderTargetHandle;
			TResourceId                   mMainDepthBufferHandle;
			TResourceId                   mLuminanceTargetHandle;
			TResourceId                   mVolumetricCloudsScreenBufferHandle;
			TResourceId                   mVolumetricCloudsFullResScreenBufferHandle;

			std::array<TResourceId, 2>    mFramesLuminanceHistoryTargets;
			USIZE                         mCurrActiveLuminanceFrameTargetIndex = 0;

			IVertexDeclaration*           mpVertexFormatDeclaration;

			TPtr<IResourceManager>        mpResourceManager;

			IWindowSystem*                mpWindowSystem;

			IGraphicsContext*             mpGraphicsContext;

			TBufferHandleId               mFullScreenTriangleVertexBufferHandle; // \note This buffer is only needed when we work with GL GAPI, D3D11 uses bufferless rendering

			IGlobalShaderProperties*      mpGlobalShaderProperties;
	};


	static TResult<TResourceId> GetOrCreateLuminanceTarget(TPtr<IResourceManager> pResourceManager, const std::string& name, U32 sizes)
	{
		TRenderTargetParameters luminanceTargetParams;
		luminanceTargetParams.mWidth = sizes;
		luminanceTargetParams.mHeight = sizes;
		luminanceTargetParams.mFormat = FT_FLOAT1;
		luminanceTargetParams.mNumOfMipLevels = static_cast<U32>(log2(sizes)) + 1;
		luminanceTargetParams.mNumOfSamples = 1;
		luminanceTargetParams.mSamplingQuality = 0;
		luminanceTargetParams.mType = TRenderTargetParameters::E_TARGET_TYPE::TEXTURE2D;

		const TResourceId targetHandle = pResourceManager->Create<IRenderTarget>(name, luminanceTargetParams);
		if (TResourceId::Invalid == targetHandle)
		{
			TDE2_ASSERT(false);
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		if (auto pTarget = pResourceManager->GetResource<IRenderTarget>(targetHandle))
		{
			pTarget->SetUWrapMode(E_ADDRESS_MODE_TYPE::AMT_CLAMP);
			pTarget->SetVWrapMode(E_ADDRESS_MODE_TYPE::AMT_CLAMP);

			pTarget->SetFilterType(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);
		}

		return Wrench::TOkValue<TResourceId>(targetHandle);
	}


	static TResourceId GetRenderTarget(TPtr<IResourceManager> pResourceManager, U32 width, U32 height, bool isHDRSupport, E_FRAME_RENDER_PARAMS_FLAGS flags)
	{
		static U32 counter = 0;

		std::string renderTargetName;

		if (E_FRAME_RENDER_PARAMS_FLAGS::RENDER_MAIN == (E_FRAME_RENDER_PARAMS_FLAGS::RENDER_MAIN & flags))
		{
			renderTargetName = "MainRenderTarget";
		}
		else if (E_FRAME_RENDER_PARAMS_FLAGS::RENDER_UI == (E_FRAME_RENDER_PARAMS_FLAGS::RENDER_UI & flags))
		{
			renderTargetName = "UITarget";
		}
		else
		{
			renderTargetName = "SecondaryRenderTarget" + std::to_string(counter++);
		}

		return pResourceManager->Create<IRenderTarget>(renderTargetName, TTexture2DParameters{ width, height, isHDRSupport ? FT_FLOAT4 : FT_NORM_UBYTE4, 1, 1, 0 });
	}


	CFramePostProcessor::CFramePostProcessor() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CFramePostProcessor::Init(const TFramePostProcessorParameters& desc)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!desc.mpRenderer || !desc.mpGraphicsObjectManager || !desc.mpWindowSystem)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = desc.mpRenderer->GetResourceManager();
		mpGlobalShaderProperties = desc.mpRenderer->GetGlobalShaderProperties().Get();

		mpPreUIRenderQueue = desc.mpRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_SPRITES);
		mpOverlayRenderQueue = desc.mpRenderer->GetRenderQueue(E_RENDER_QUEUE_GROUP::RQG_OVERLAY);
		mpWindowSystem = desc.mpWindowSystem;
		mpGraphicsContext = desc.mpGraphicsObjectManager->GetGraphicsContext();
		mRenderTargetHandle = TResourceId::Invalid;
		mUITargetHandle = TResourceId::Invalid;

		auto luminanceTargetResult = GetOrCreateLuminanceTarget(mpResourceManager, "LuminanceTarget", LuminanceTargetSizes);
		if (luminanceTargetResult.HasError())
		{
			return luminanceTargetResult.GetError();
		}

		mLuminanceTargetHandle = luminanceTargetResult.Get();

		std::array<TResult<TResourceId>, 2> frameLuminanceBuffers
		{
			GetOrCreateLuminanceTarget(mpResourceManager, "LuminancePrevFrame", 1),
			GetOrCreateLuminanceTarget(mpResourceManager, "LuminanceCurrFrame", 1)
		};

		std::transform(frameLuminanceBuffers.begin(), frameLuminanceBuffers.end(), mFramesLuminanceHistoryTargets.begin(), [](const TResult<TResourceId>& result)
		{
			TDE2_ASSERT(result.IsOk());
			return result.Get();
		});

		// Used materials 
		// \todo Refactor this later
		mDefaultScreenSpaceMaterialHandle = mpResourceManager->Create<IMaterial>("DefaultScreenSpaceEffect.material", TMaterialParameters{ "Shaders/PostEffects/BlitTexture.shader", false, TDepthStencilStateDesc { false, false } });
		mBloomFilterMaterialHandle = mpResourceManager->Create<IMaterial>("BloomScreenSpaceEffect.material", TMaterialParameters{ "Shaders/PostEffects/Bloom.shader", false, TDepthStencilStateDesc { false, false } });
		mBloomFinalPassMaterialHandle = mpResourceManager->Create<IMaterial>("BloomFinalPassSpaceEffect.material", TMaterialParameters{ "Shaders/PostEffects/BloomFinal.shader", false, TDepthStencilStateDesc { false, false } });
		mGaussianBlurMaterialHandle = mpResourceManager->Create<IMaterial>("GaussianBlurSpaceEffect.material", TMaterialParameters{ "Shaders/PostEffects/GaussianBlur.shader", false, TDepthStencilStateDesc { false, false } });
		mToneMappingPassMaterialHandle = mpResourceManager->Create<IMaterial>("ToneMappingPass.material", TMaterialParameters{ "Shaders/PostEffects/ToneMapping.shader", false, TDepthStencilStateDesc { false, false } });
		mGenerateLuminanceMaterialHandle = mpResourceManager->Create<IMaterial>("GenerateLuminance.material", TMaterialParameters{ "Shaders/PostEffects/GenerateLuminance.shader", false, TDepthStencilStateDesc { false, false } });
		mLuminanceAdaptationMaterialHandle = mpResourceManager->Create<IMaterial>("AdaptLuminance.material", TMaterialParameters{ "Shaders/PostEffects/AdaptLuminance.shader", false, TDepthStencilStateDesc { false, false } });

		mVolumetricCloudsComputeShaderHandle = mpResourceManager->Load<IShader>("Shaders/Default/VolumetricClouds.cshader");
		mVolumetricCloudsUpsampleBlurShaderHandle = mpResourceManager->Load<IShader>("Shaders/Default/VolumetricCloudsBlur.cshader");
		mVolumetricCloudsComposeMaterialHandle = mpResourceManager->Create<IMaterial>("VolumetricCloudsCompose.material", TMaterialParameters{ "Shaders/Default/VolumetricCloudsCompose.shader", false, TDepthStencilStateDesc { false, false } });

		if (auto vertexFormatResult = desc.mpGraphicsObjectManager->CreateVertexDeclaration())
		{
			mpVertexFormatDeclaration = vertexFormatResult.Get();
		}

		TDE2_ASSERT(mpVertexFormatDeclaration);
		mpVertexFormatDeclaration->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_POSITION });

		const void* pScreenTriangleVerts = static_cast<const void*>(&desc.mpGraphicsObjectManager->GetScreenTriangleVertices()[0]);

		if (auto vertexBufferResult = desc.mpGraphicsObjectManager->CreateBuffer(
			{
				E_BUFFER_USAGE_TYPE::STATIC,
				E_BUFFER_TYPE::VERTEX,
				3 * sizeof(TVector4),
				pScreenTriangleVerts
			}))
		{
			mFullScreenTriangleVertexBufferHandle = vertexBufferResult.Get();
		}

		TDE2_ASSERT(TBufferHandleId::Invalid != mFullScreenTriangleVertexBufferHandle);

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CFramePostProcessor::SetProcessingProfile(const IPostProcessingProfile* pProfileResource)
	{
		if (!pProfileResource)
		{
			return RC_INVALID_ARGS;
		}

		mpCurrPostProcessingProfile = pProfileResource;

		return RC_OK;
	}


	static const std::string FrontFrameTextureUniformId = "FrameTexture";
	static const std::string BackFrameTextureUniformId = "FrameTexture1";


	E_RESULT_CODE CFramePostProcessor::Render(const TRenderFrameCallback& onRenderFrameCallback, E_FRAME_RENDER_PARAMS_FLAGS flags)
	{
		if (!onRenderFrameCallback)
		{
			LOG_WARNING("[FramePostProcessor] Render method was got empty \"onRenderFrameCallback\" argument");
			return RC_INVALID_ARGS;
		}

		const bool bindDepthBuffer = E_FRAME_RENDER_PARAMS_FLAGS::BIND_DEPTH_BUFFER == (flags & E_FRAME_RENDER_PARAMS_FLAGS::BIND_DEPTH_BUFFER);
		const bool clearRT = E_FRAME_RENDER_PARAMS_FLAGS::CLEAR_RENDER_TARGET == (flags & E_FRAME_RENDER_PARAMS_FLAGS::CLEAR_RENDER_TARGET);
		const bool isMainTarget = E_FRAME_RENDER_PARAMS_FLAGS::RENDER_MAIN == (flags & E_FRAME_RENDER_PARAMS_FLAGS::RENDER_MAIN);


		TPtr<IRenderTarget> pCurrRenderTarget = mpResourceManager->GetResource<IRenderTarget>(isMainTarget ? mRenderTargetHandle : mUITargetHandle);
		TPtr<IDepthBufferTarget> pMainDepthBuffer = mpResourceManager->GetResource<IDepthBufferTarget>(mMainDepthBufferHandle);

		{
			mpGraphicsContext->BindRenderTarget(0, pCurrRenderTarget.Get());
			if (bindDepthBuffer)
			{
				mpGraphicsContext->BindDepthBufferTarget(pMainDepthBuffer.Get());
			}

			if (clearRT)
			{
				mpGraphicsContext->ClearRenderTarget(pCurrRenderTarget.Get(), TColor32F{});
			}

			onRenderFrameCallback();

			if (bindDepthBuffer)
			{
				mpGraphicsContext->BindDepthBufferTarget(nullptr);
			}

			mpGraphicsContext->BindRenderTarget(0, nullptr);
		}

		return RC_OK;
	}

	E_RESULT_CODE CFramePostProcessor::PreRender()
	{
		const U32 width = mpWindowSystem->GetWidth();
		const U32 height = mpWindowSystem->GetHeight();

		const auto& toneMappingParameters = mpCurrPostProcessingProfile->GetToneMappingParameters();
		const auto& colorGradingParameters = mpCurrPostProcessingProfile->GetColorGradingParameters();

		const bool isHDREnabled = toneMappingParameters.mIsEnabled;

		if (auto pToneMappingMaterial = mpResourceManager->GetResource<IMaterial>(mToneMappingPassMaterialHandle))
		{
			pToneMappingMaterial->SetVariableForInstance<TVector4>(DefaultMaterialInstanceId, "toneMappingParams", TVector4(isHDREnabled ? 1.0f : 0.0f, toneMappingParameters.mExposure, toneMappingParameters.mKeyValue, 0.0f));
			pToneMappingMaterial->SetVariableForInstance<TVector4>(DefaultMaterialInstanceId, "colorGradingParams", TVector4(colorGradingParameters.mIsEnabled ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f));
		}

		_prepareRenderTargetsChain(width, height, isHDREnabled);

		return RC_OK;
	}

	E_RESULT_CODE CFramePostProcessor::RunPostProcess()
	{
		TPtr<IRenderTarget> pCurrRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mRenderTargetHandle);
		TPtr<IRenderTarget> pBloomRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mBloomRenderTargetHandle);
		TPtr<IRenderTarget> pTempRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mTemporaryRenderTargetHandle);

		{ // Luminance calculations
#if TDE2_EDITORS_ENABLED
			auto pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(mpGraphicsContext);
			TDE_RENDER_SECTION(pGraphicsContext, "EyeAdaptation");
#endif

			TPtr<IRenderTarget> pLuminanceTarget = mpResourceManager->GetResource<IRenderTarget>(mLuminanceTargetHandle);

			_renderTargetToTarget(pCurrRenderTarget, nullptr, pLuminanceTarget, mGenerateLuminanceMaterialHandle); // Extract luminance
			pLuminanceTarget->GenerateMipMaps();

			// Eye adaptation 
			TPtr<IRenderTarget> pLuminanceAdaptPrevTarget = mpResourceManager->GetResource<IRenderTarget>(mFramesLuminanceHistoryTargets[(mCurrActiveLuminanceFrameTargetIndex + 1) & 0x1]);
			TPtr<IRenderTarget> pLuminanceAdaptCurrTarget = mpResourceManager->GetResource<IRenderTarget>(mFramesLuminanceHistoryTargets[mCurrActiveLuminanceFrameTargetIndex]);

			if (auto pMaterial = mpResourceManager->GetResource<IMaterial>(mLuminanceAdaptationMaterialHandle))
			{
				const auto& toneMappingParameters = mpCurrPostProcessingProfile->GetToneMappingParameters();

				pMaterial->SetVariableForInstance<F32>(DefaultMaterialInstanceId, "mAdaptationRate", toneMappingParameters.mEyeAdaptionCoeff);
			}

			_renderTargetToTarget(pLuminanceTarget, pLuminanceAdaptPrevTarget.Get(), pLuminanceAdaptCurrTarget, mLuminanceAdaptationMaterialHandle);
		}

		_processBloomPass(pCurrRenderTarget, pTempRenderTarget, pBloomRenderTarget);

		mpPreUIRenderQueue->Clear(); // commands above are executed immediately, so we don't need to store them anymore

		return RC_OK;
	}

	E_RESULT_CODE CFramePostProcessor::PostRender()
	{
		_submitFullScreenTriangle(mpOverlayRenderQueue, mToneMappingPassMaterialHandle);

		mCurrActiveLuminanceFrameTargetIndex = (mCurrActiveLuminanceFrameTargetIndex + 1) & 0x1;

		return RC_OK;
	}


	E_RESULT_CODE CFramePostProcessor::RunVolumetricCloudsPass()
	{
		TDE2_PROFILER_SCOPE("CFramePostProcessor::RunVolumetricCloudsPass");

		if (!CGameUserSettings::Get()->mpIsVolumetricCloudsEnabledCVar->Get())
		{
			return RC_OK;
		}

		auto pVolumetricCloudsScreenBufferTexture = mpResourceManager->GetResource<ITexture2D>(mVolumetricCloudsScreenBufferHandle);
		if (!pVolumetricCloudsScreenBufferTexture)
		{
			return RC_FAIL;
		}

		auto pDepthBufferResource = mpResourceManager->GetResource<IDepthBufferTarget>(mMainDepthBufferHandle);
		auto pMainRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mRenderTargetHandle);

		// Main pass
		{
			struct
			{
				TVector2 mInvTextureSizes;
				I32      mStepsCount;
			} uniformsData;

			uniformsData.mInvTextureSizes = TVector2
			{
				1 / static_cast<F32>(pVolumetricCloudsScreenBufferTexture->GetWidth()), 1 / static_cast<F32>(pVolumetricCloudsScreenBufferTexture->GetHeight())
			};
			uniformsData.mStepsCount = 64;

			auto pVolumetricCloudsRenderPassShader = mpResourceManager->GetResource<IShader>(mVolumetricCloudsComputeShaderHandle);
			pVolumetricCloudsRenderPassShader->SetTextureResource("OutputTexture", pVolumetricCloudsScreenBufferTexture.Get());
			pVolumetricCloudsRenderPassShader->SetTextureResource("DepthTexture", pDepthBufferResource.Get());
			pVolumetricCloudsRenderPassShader->SetTextureResource("MainTexture", pMainRenderTarget.Get());

			pVolumetricCloudsRenderPassShader->SetUserUniformsBuffer(0, reinterpret_cast<const U8*>(&uniformsData), sizeof(uniformsData));
			pVolumetricCloudsRenderPassShader->Bind();

			mpGraphicsContext->DispatchCompute(pVolumetricCloudsScreenBufferTexture->GetWidth() / 16, pVolumetricCloudsScreenBufferTexture->GetHeight() / 16, 1);

			pVolumetricCloudsRenderPassShader->Unbind();
		}

		auto pVolumetricCloudsFullSizeBufferTexture = mpResourceManager->GetResource<ITexture2D>(mVolumetricCloudsFullResScreenBufferHandle);

		// Blur + Upsample
		{
			auto pVolumetricCloudsUpsampleBlurPassShader = mpResourceManager->GetResource<IShader>(mVolumetricCloudsUpsampleBlurShaderHandle);
			pVolumetricCloudsUpsampleBlurPassShader->SetTextureResource("OutputTexture", pVolumetricCloudsFullSizeBufferTexture.Get());
			pVolumetricCloudsUpsampleBlurPassShader->SetTextureResource("DepthTexture", pDepthBufferResource.Get());
			pVolumetricCloudsUpsampleBlurPassShader->SetTextureResource("MainTexture", pVolumetricCloudsScreenBufferTexture.Get());

			pVolumetricCloudsUpsampleBlurPassShader->Bind();

			mpGraphicsContext->DispatchCompute(pVolumetricCloudsFullSizeBufferTexture->GetWidth() / 16, pVolumetricCloudsFullSizeBufferTexture->GetHeight() / 16, 1);

			pVolumetricCloudsUpsampleBlurPassShader->Unbind();
		}

		// Compose pass
		_renderTargetToTarget(
			DynamicPtrCast<IRenderTarget>(pDepthBufferResource),
			pVolumetricCloudsFullSizeBufferTexture.Get(),
			pMainRenderTarget,
			mVolumetricCloudsComposeMaterialHandle); // Compose

		return RC_OK;
	}


	static TResult<TResourceId> GetOrCreateVolumetricCloudsBuffer(TPtr<IResourceManager> pResourceManager, U32 width, U32 height, const std::string& suffix = "Main")
	{
		TTexture2DParameters textureParams
		{
			width,
			height,
			FT_FLOAT4, 1, 1, 0
		};
		textureParams.mIsWriteable = true;

		const TResourceId textureHandle = pResourceManager->Create<ITexture2D>("VolumetricCloudsScreenBuffer_" + suffix, textureParams);
		if (auto pScreenBufferTexture = pResourceManager->GetResource<ITexture2D>(textureHandle))
		{
			pScreenBufferTexture->SetFilterType(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);

			if (pScreenBufferTexture->GetWidth() != width || pScreenBufferTexture->GetHeight() != height)
			{
				LOG_MESSAGE(Wrench::StringUtils::Format("[CFramePostProcessor] The sizes has of volumetric clouds buffer been changed", width, height));
				pScreenBufferTexture->Resize(width, height);
			}
		}

		if (TResourceId::Invalid == textureHandle)
		{
			TDE2_ASSERT(false);
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<TResourceId>(textureHandle);
	}


	static TResult<TResourceId> GetOrCreateDepthBuffer(TPtr<IResourceManager> pResourceManager, U32 width, U32 height)
	{
		TRenderTargetParameters depthBufferParams;
		depthBufferParams.mWidth = width;
		depthBufferParams.mHeight = height;
		depthBufferParams.mFormat = FT_D32;
		depthBufferParams.mNumOfMipLevels = 1;
		depthBufferParams.mNumOfSamples = 1;
		depthBufferParams.mSamplingQuality = 0;
		depthBufferParams.mType = TRenderTargetParameters::E_TARGET_TYPE::TEXTURE2D;

		const TResourceId depthBufferHandle = pResourceManager->Create<IDepthBufferTarget>("MainDepthBuffer", depthBufferParams);
		if (TResourceId::Invalid == depthBufferHandle)
		{
			TDE2_ASSERT(false);
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		if (auto pDepthBufferTexture = pResourceManager->GetResource<IDepthBufferTarget>(depthBufferHandle))
		{
			pDepthBufferTexture->SetUWrapMode(E_ADDRESS_MODE_TYPE::AMT_CLAMP);
			pDepthBufferTexture->SetVWrapMode(E_ADDRESS_MODE_TYPE::AMT_CLAMP);
			pDepthBufferTexture->SetFilterType(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);

			if (pDepthBufferTexture->GetWidth() != width || pDepthBufferTexture->GetHeight() != height)
			{
				LOG_MESSAGE("[CFramePostProcessor] The sizes has been changed");
				pDepthBufferTexture->Resize(width, height);
			}
		}

		return Wrench::TOkValue<TResourceId>(depthBufferHandle);
	}


	void CFramePostProcessor::_prepareRenderTargetsChain(U32 width, U32 height, bool isHDRSupport)
	{
		auto createVolumetricCloudsBufferResult = GetOrCreateVolumetricCloudsBuffer(mpResourceManager, width / 4, height / 4);
		if (createVolumetricCloudsBufferResult.IsOk())
		{
			mVolumetricCloudsScreenBufferHandle = createVolumetricCloudsBufferResult.Get();
		}

		auto createVolumetricCloudsFullResBufferResult = GetOrCreateVolumetricCloudsBuffer(mpResourceManager, width, height, "FullRes");
		if (createVolumetricCloudsFullResBufferResult.IsOk())
		{
			mVolumetricCloudsFullResScreenBufferHandle = createVolumetricCloudsFullResBufferResult.Get();
		}

		if (TResourceId::Invalid != mRenderTargetHandle)
		{
			if (auto pCurrRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mRenderTargetHandle))
			{
				if (pCurrRenderTarget && (pCurrRenderTarget->GetWidth() != width || pCurrRenderTarget->GetHeight() != height))
				{
					_resizeRenderTargetsChain(width, height);
				}
			}

			return;
		}

		mRenderTargetHandle = GetRenderTarget(mpResourceManager, width, height, isHDRSupport, E_FRAME_RENDER_PARAMS_FLAGS::RENDER_MAIN);
		mTemporaryRenderTargetHandle = GetRenderTarget(mpResourceManager, width, height, isHDRSupport, E_FRAME_RENDER_PARAMS_FLAGS::NONE);
		mUITargetHandle = GetRenderTarget(mpResourceManager, width, height, false, E_FRAME_RENDER_PARAMS_FLAGS::RENDER_UI);

		auto mainDepthBufferRetrieveResult = GetOrCreateDepthBuffer(mpResourceManager, width, height);
		if (mainDepthBufferRetrieveResult.IsOk())
		{
			mMainDepthBufferHandle = mainDepthBufferRetrieveResult.Get();
		}

		auto pCurrRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mRenderTargetHandle);
		auto pUIRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mUITargetHandle);
		auto pTempRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mTemporaryRenderTargetHandle);
		auto pLuminanceTarget = mpResourceManager->GetResource<IRenderTarget>(mFramesLuminanceHistoryTargets[mCurrActiveLuminanceFrameTargetIndex]);

		pCurrRenderTarget->SetFilterType(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);
		pTempRenderTarget->SetFilterType(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);

		const bool isPostProcessingEnabled = mpCurrPostProcessingProfile->IsPostProcessingEnabled();

		if (auto pToneMappingMaterial = mpResourceManager->GetResource<IMaterial>(mToneMappingPassMaterialHandle))
		{
			pToneMappingMaterial->SetTextureResource(FrontFrameTextureUniformId, pCurrRenderTarget.Get());

			/// \note If the color grading's pass was enabled assign its look up texture (assume that it has linear sampling)
			{
				const auto& colorGradingParameters = mpCurrPostProcessingProfile->GetColorGradingParameters();

				if (isPostProcessingEnabled/* && colorGradingParameters.mIsEnabled*/)
				{
					if (auto pColorLUT = mpResourceManager->GetResource<ITexture>(mpResourceManager->Load<ITexture2D>(colorGradingParameters.mLookUpTextureId)))
					{
						pToneMappingMaterial->SetTextureResource("ColorGradingLUT", pColorLUT.Get());
					}
				}

				pToneMappingMaterial->SetTextureResource("LuminanceBuffer", pLuminanceTarget.Get());
				pToneMappingMaterial->SetTextureResource("UIBuffer", pUIRenderTarget.Get());
			}
		}

		if (isPostProcessingEnabled && mpCurrPostProcessingProfile->GetBloomParameters().mIsEnabled)
		{
			const U16 downsampleCoeff = 1 << (TPostProcessingProfileParameters::TBloomParameters::mMaxQuality - mpCurrPostProcessingProfile->GetBloomParameters().mQuality + 1);

			mBloomRenderTargetHandle = GetRenderTarget(mpResourceManager, width / downsampleCoeff, height / downsampleCoeff, isHDRSupport, E_FRAME_RENDER_PARAMS_FLAGS::NONE);

			if (auto pBloomRenderTarget = mpResourceManager->GetResource<IRenderTarget>(mBloomRenderTargetHandle))
			{
				pBloomRenderTarget->SetFilterType(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);
			}
		}
	}

	void CFramePostProcessor::_resizeRenderTargetsChain(U32 width, U32 height)
	{
		const std::array<TResourceId, 4> renderTargetHandles
		{
			mRenderTargetHandle,
			mUITargetHandle,
			mBloomRenderTargetHandle,
			mTemporaryRenderTargetHandle
		};

		for (const TResourceId& currTargetHandle : renderTargetHandles)
		{
			if (auto pCurrRenderTarget = mpResourceManager->GetResource<IRenderTarget>(currTargetHandle))
			{
				pCurrRenderTarget->Resize(width, height);
			}
		}
	}

	void CFramePostProcessor::_processBloomPass(TPtr<IRenderTarget> pFrontTarget, TPtr<IRenderTarget> pBackTarget, TPtr<IRenderTarget> pBloomTarget)
	{
		const auto& bloomParameters = mpCurrPostProcessingProfile->GetBloomParameters();
		const auto& toneMappingParameters = mpCurrPostProcessingProfile->GetToneMappingParameters();

		if (!mpCurrPostProcessingProfile->IsPostProcessingEnabled() || !bloomParameters.mIsEnabled)
		{
			return;
		}

#if TDE2_EDITORS_ENABLED
		auto pGraphicsContext = MakeScopedFromRawPtr<IGraphicsContext>(mpGraphicsContext);
		TDE_RENDER_SECTION(pGraphicsContext, "Bloom");
#endif

		if (auto pBloomMaterial = mpResourceManager->GetResource<IMaterial>(mBloomFilterMaterialHandle))
		{
			pBloomMaterial->SetVariableForInstance<F32>(DefaultMaterialInstanceId, "threshold", bloomParameters.mThreshold);
			pBloomMaterial->SetVariableForInstance<F32>(DefaultMaterialInstanceId, "keyValue", toneMappingParameters.mKeyValue);
		}

		const TVector4 blurParams{ bloomParameters.mSmoothness, 0.0, 1.0f / static_cast<F32>(pFrontTarget->GetWidth()), 1.0f / static_cast<F32>(pFrontTarget->GetHeight()) };
		const TVector4 vertBlurRotation{ 0.0f, CMathConstants::Pi * 0.5f, 0.0f, 0.0f };

		if (auto pBlurMaterial = mpResourceManager->GetResource<IMaterial>(mGaussianBlurMaterialHandle))
		{
			pBlurMaterial->SetVariableForInstance<TVector4>(DefaultMaterialInstanceId, "blurParams", blurParams);
			pBlurMaterial->SetVariableForInstance<U32>(DefaultMaterialInstanceId, "samplesCount", bloomParameters.mSamplesCount);
		}

		auto pLuminanceTarget = mpResourceManager->GetResource<IRenderTarget>(mFramesLuminanceHistoryTargets[mCurrActiveLuminanceFrameTargetIndex]);

		_renderTargetToTarget(pFrontTarget, pLuminanceTarget.Get(), pBloomTarget, mBloomFilterMaterialHandle); // Bloom pass
		_renderTargetToTarget(pBloomTarget, nullptr, pBackTarget, mGaussianBlurMaterialHandle); // Horizontal Blur pass

		if (auto pBlurMaterial = mpResourceManager->GetResource<IMaterial>(mGaussianBlurMaterialHandle))
		{
			pBlurMaterial->SetVariableForInstance<TVector4>(DefaultMaterialInstanceId, "blurParams", blurParams + vertBlurRotation);
		}

		_renderTargetToTarget(pBackTarget, nullptr, pBloomTarget, mGaussianBlurMaterialHandle); // Vertical Blur pass
		_renderTargetToTarget(pFrontTarget, pBloomTarget.Get(), pBackTarget, mBloomFinalPassMaterialHandle); // Compose
		_renderTargetToTarget(pBackTarget, nullptr, pFrontTarget, mDefaultScreenSpaceMaterialHandle); // Blit Temp -> Main render target
	}

	void CFramePostProcessor::_submitFullScreenTriangle(CRenderQueue* pRenderQueue, TResourceId materialHandle, bool drawImmediately)
	{
		// \note The fullscreen triangle is drawn without vertex buffer, all the geometry is generated inside the vertex shader
		TDrawCommandPtr pDrawCommand = pRenderQueue->SubmitDrawCommand<TDrawCommand>(static_cast<U32>(E_GEOMETRY_SUBGROUP_TAGS::IMAGE_EFFECTS));
		pDrawCommand->mNumOfVertices = 3;
		pDrawCommand->mPrimitiveType = E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST;
		pDrawCommand->mMaterialHandle = materialHandle;
		pDrawCommand->mVertexBufferHandle = mFullScreenTriangleVertexBufferHandle;
		pDrawCommand->mpVertexDeclaration = mpVertexFormatDeclaration;

		if (drawImmediately)
		{
			pDrawCommand->Submit(mpGraphicsContext, mpResourceManager.Get(), mpGlobalShaderProperties);
		}
	}

	void CFramePostProcessor::_renderTargetToTarget(TPtr<IRenderTarget> pSource, ITexture* pExtraSource, TPtr<IRenderTarget> pDest, TResourceId materialHandle)
	{
		mpGraphicsContext->SetDepthBufferEnabled(false);
		mpGraphicsContext->BindRenderTarget(0, pDest.Get());
		mpGraphicsContext->SetViewport(0.0f, 0.0f, static_cast<F32>(pDest->GetWidth()), static_cast<F32>(pDest->GetHeight()), 0.0f, 1.0f);

		if (auto pMaterial = mpResourceManager->GetResource<IMaterial>(materialHandle))
		{
			pMaterial->SetTextureResource(FrontFrameTextureUniformId, pSource.Get());

			if (pExtraSource)
			{
				pMaterial->SetTextureResource(BackFrameTextureUniformId, pExtraSource);
			}
		}

		_submitFullScreenTriangle(mpPreUIRenderQueue, materialHandle, true);

		mpGraphicsContext->SetViewport(0.0f, 0.0f, static_cast<F32>(mpWindowSystem->GetWidth()), static_cast<F32>(mpWindowSystem->GetHeight()), 0.0f, 1.0f);
		mpGraphicsContext->SetDepthBufferEnabled(true);
		mpGraphicsContext->BindRenderTarget(0, nullptr);
	}


	static IFramePostProcessor* CreateFramePostProcessor(const TFramePostProcessorParameters& desc, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IFramePostProcessor, CFramePostProcessor, result, desc);
	}


	/*!
		\brief CForwardRenderer's definition
	*/

	CForwardRenderer::CForwardRenderer():
		CBaseObject(), mpMainCamera(nullptr), mpResourceManager(nullptr), mpGlobalShaderProperties(nullptr), mpFramePostProcessor(nullptr)
	{
	}


	static TResult<TResourceId> GetOrCreateDirectionalShadowMap(TPtr<IResourceManager> pResourceManager)
	{
		const U32 shadowMapSizes = static_cast<U32>(CGameUserSettings::Get()->mpShadowMapSizesCVar->Get());
		TDE2_ASSERT(shadowMapSizes > 0 && shadowMapSizes < 65536);

		TRenderTargetParameters shadowMapParams;
		shadowMapParams.mWidth = shadowMapSizes;
		shadowMapParams.mHeight = shadowMapSizes;
		shadowMapParams.mFormat = FT_D32;
		shadowMapParams.mNumOfMipLevels = 1;
		shadowMapParams.mNumOfSamples = 1;
		shadowMapParams.mSamplingQuality = 0;
		shadowMapParams.mType = TRenderTargetParameters::E_TARGET_TYPE::TEXTURE2D_ARRAY;
		shadowMapParams.mArraySize = CGameUserSettings::Get()->mpShadowCascadesCountCVar->Get();

		const TResourceId shadowMapHandle = pResourceManager->Create<IDepthBufferTarget>("ShadowMap", shadowMapParams);
		if (shadowMapHandle == TResourceId::Invalid)
		{
			TDE2_ASSERT(false);
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		if (auto pShadowMapBuffer = pResourceManager->GetResource<IDepthBufferTarget>(shadowMapHandle))
		{
			pShadowMapBuffer->SetUWrapMode(E_ADDRESS_MODE_TYPE::AMT_CLAMP);
			pShadowMapBuffer->SetVWrapMode(E_ADDRESS_MODE_TYPE::AMT_CLAMP);
			pShadowMapBuffer->SetFilterType(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);

			if (pShadowMapBuffer->GetWidth() != shadowMapSizes)
			{
				LOG_MESSAGE(Wrench::StringUtils::Format("[CForwardRenderer] The shadow map sizes has been changed, from {0}x{0} to {1}x{1}", pShadowMapBuffer->GetWidth(), shadowMapSizes));
				pShadowMapBuffer->Resize(shadowMapSizes, shadowMapSizes);
			}
		}

		return Wrench::TOkValue<TResourceId>(shadowMapHandle);
	}


	static TResult<TResourceId> GetOrCreatePointShadowMap(TPtr<IResourceManager> pResourceManager, USIZE pointLightIndex)
	{
		const U32 shadowMapSizes = static_cast<U32>(CGameUserSettings::Get()->mpShadowMapSizesCVar->Get());
		TDE2_ASSERT(shadowMapSizes > 0 && shadowMapSizes < 65536);

		TRenderTargetParameters shadowMapParams;
		shadowMapParams.mWidth = shadowMapSizes;
		shadowMapParams.mHeight = shadowMapSizes;
		shadowMapParams.mFormat = FT_D32;
		shadowMapParams.mNumOfMipLevels = 1;
		shadowMapParams.mNumOfSamples = 1;
		shadowMapParams.mSamplingQuality = 0;
		shadowMapParams.mType = TRenderTargetParameters::E_TARGET_TYPE::CUBEMAP;

		const TResourceId shadowMapHandle = pResourceManager->Create<IDepthBufferTarget>("PointShadowMap" + std::to_string(pointLightIndex), shadowMapParams);
		if (shadowMapHandle == TResourceId::Invalid)
		{
			TDE2_ASSERT(false);
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		if (auto pShadowMapBuffer = pResourceManager->GetResource<IDepthBufferTarget>(shadowMapHandle))
		{
			pShadowMapBuffer->SetUWrapMode(E_ADDRESS_MODE_TYPE::AMT_CLAMP);
			pShadowMapBuffer->SetVWrapMode(E_ADDRESS_MODE_TYPE::AMT_CLAMP);
			pShadowMapBuffer->SetFilterType(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);

			if (pShadowMapBuffer->GetWidth() != shadowMapSizes)
			{
				LOG_MESSAGE(Wrench::StringUtils::Format("[CForwardRenderer] The shadow map sizes has been changed, from {0}x{0} to {1}x{1}", pShadowMapBuffer->GetWidth(), shadowMapSizes));
				pShadowMapBuffer->Resize(shadowMapSizes, shadowMapSizes);
			}
		}

		return Wrench::TOkValue<TResourceId>(shadowMapHandle);
	}


	E_RESULT_CODE CForwardRenderer::Init(const TRendererInitParams& params)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!params.mAllocatorFactoryFunctor || !params.mpGraphicsContext || !params.mpResourceManager || !params.mpWindowSystem)
		{
			return RC_INVALID_ARGS;
		}

		mpGraphicsContext    = params.mpGraphicsContext;
		mpResourceManager    = params.mpResourceManager;
		mpWindowSystem       = params.mpWindowSystem;
		
		E_RESULT_CODE result = RC_OK;

		mpFrameGraph = CreateFrameGraph(result);

		auto allocatorFactory = params.mAllocatorFactoryFunctor;
				
		IAllocator* pCurrAllocator = nullptr;

		void* pCurrMemoryBlock = nullptr;

		for (U8 i = 0; i < NumOfRenderQueuesGroup; ++i)
		{
			pCurrAllocator = allocatorFactory(PerRenderQueueMemoryBlockSize, result);

			if (result != RC_OK)
			{
				return result;
			}

			/// \note this CRenderQueue's instance now owns this allocator
			mpRenderQueues[i] = TPtr<CRenderQueue>(CreateRenderQueue(pCurrAllocator, result));

			if (result != RC_OK)
			{
				return result;
			}
			
			LOG_MESSAGE(std::string("[Forward Renderer] A new render queue buffer was created ( mem-size : ").append(std::to_string(PerRenderQueueMemoryBlockSize / 1024)).
																											  append(" KiB; group-type : ").
																											  append(std::to_string(i)).
																											  append(")"));
		}

		auto pGraphicsObjectManager = mpGraphicsContext->GetGraphicsObjectManager();

		mpGlobalShaderProperties = TPtr<IGlobalShaderProperties>(CreateGlobalShaderProperties(pGraphicsObjectManager, result));

		/// \todo fill in data into TConstantsShaderData buffer
		mpGlobalShaderProperties->SetInternalUniformsBuffer(IUBR_CONSTANTS, nullptr, 0);

		if (result != RC_OK)
		{
			return result;
		}

		auto debugUtilityResult = pGraphicsObjectManager->CreateDebugUtility(mpResourceManager.Get(), this);
		if (debugUtilityResult.HasError())
		{
			return debugUtilityResult.GetError();
		}

		mpDebugUtility = debugUtilityResult.Get();

		if (CGameUserSettings::Get()->mpIsShadowMappingEnabledCVar->Get())
		{
			GetOrCreateDirectionalShadowMap(mpResourceManager).Get(); /// \note Create a shadow map's texture before any Update will be executed
			GetOrCreatePointShadowMap(mpResourceManager, 0).Get();
		}

		mpFramePostProcessor = TPtr<IFramePostProcessor>(CreateFramePostProcessor({ this, mpGraphicsContext->GetGraphicsObjectManager(), mpWindowSystem.Get() }, result));
		if (result != RC_OK)
		{
			return result;
		}

		mIsInitialized = true;

		return RC_OK;
	}


	static inline void SubmitCommandsToDraw(TPtr<IGraphicsContext> pGraphicsContext, TPtr<IResourceManager> pResourceManager, TPtr<IGlobalShaderProperties> pGlobalShaderProperties,
									TPtr<CRenderQueue> pRenderQueue, U32 upperRenderIndexLimit)
	{
		CRenderQueue::CRenderQueueIterator iter = pRenderQueue->GetIterator();

		TRenderCommand* pCurrDrawCommand = nullptr;

		while (iter.HasNext())
		{
			pCurrDrawCommand = *(iter++);

			if (!pCurrDrawCommand)
			{
				continue;
			}

			if (iter.GetIndex() >= upperRenderIndexLimit)
			{
				break;
			}

			pCurrDrawCommand->Submit(pGraphicsContext.Get(), pResourceManager.Get(), pGlobalShaderProperties.Get());
		}
	}


	static inline void ExecuteDrawCommands(TPtr<IGraphicsContext> pGraphicsContext, TPtr<IResourceManager> pResourceManager, TPtr<IGlobalShaderProperties> pGlobalShaderProperties,
									TPtr<CRenderQueue> pCommandsBuffer, bool shouldClearBuffers, U32 upperRenderIndexLimit = (std::numeric_limits<U32>::max)())
	{
		pCommandsBuffer->Sort();
		SubmitCommandsToDraw(pGraphicsContext, pResourceManager, pGlobalShaderProperties, pCommandsBuffer, upperRenderIndexLimit);

		if (shouldClearBuffers)
		{
			pCommandsBuffer->Clear();
		}
	};


	static void RenderShadowCasters(TPtr<IGraphicsContext> pGraphicsContext, TPtr<IResourceManager> pResourceManager, TResourceId shadowMapHandle, const std::function<void()> action)
	{
		const F32 shadowMapSizes = static_cast<F32>(CGameUserSettings::Get()->mpShadowMapSizesCVar->Get());

		pGraphicsContext->SetViewport(0.0f, 0.0f, shadowMapSizes, shadowMapSizes, 0.0f, 1.0f);
		{
			pGraphicsContext->BindDepthBufferTarget(pResourceManager->GetResource<IDepthBufferTarget>(shadowMapHandle).Get(), true);

			pGraphicsContext->ClearDepthBuffer(1.0f);

			if (action)
			{
				action();
			}

			pGraphicsContext->BindDepthBufferTarget(nullptr);
		}

		if (auto pWindowSystem = pGraphicsContext->GetWindowSystem())
		{
			pGraphicsContext->SetViewport(0.0f, 0.0f, static_cast<F32>(pWindowSystem->GetWidth()), static_cast<F32>(pWindowSystem->GetHeight()), 0.0f, 1.0f);
		}
	}


	static E_RESULT_CODE ProcessShadowPass(TPtr<IGraphicsContext> pGraphicsContext, TPtr<IResourceManager> pResourceManager, TPtr<IGlobalShaderProperties> pGlobalShaderProperties,
										TPtr<CRenderQueue> pShadowCastersRenderGroup)
	{
		if (!pShadowCastersRenderGroup)
		{
			LOG_ERROR("[ForwardRenderer] Invalid \"Shadow Pass\" commands buffer was found");
			return RC_FAIL;
		}

		TDE2_PROFILER_SCOPE("Renderer::RenderShadows");
		TDE_RENDER_SECTION(pGraphicsContext, "RenderShadows");

		static std::vector<TResourceId> shadowMapHandles;
		shadowMapHandles.clear();

		shadowMapHandles.push_back(GetOrCreateDirectionalShadowMap(pResourceManager).Get());
		shadowMapHandles.push_back(GetOrCreatePointShadowMap(pResourceManager, 0).Get());

		/// \todo Replace hardcoded identifiers
		const TResourceId shadowPassMaterialHandle = pResourceManager->Load<IMaterial>("ShadowPassMaterial.material");

		for (const TResourceId currShadowMapHandle : shadowMapHandles)
		{
			/// \todo Assign "ShadowPass" material's variables that defines which type of a light casts shadows
			if (auto pMaterial = pResourceManager->GetResource<IMaterial>(shadowPassMaterialHandle))
			{
				pMaterial->SetVariableForInstance(DefaultMaterialInstanceId, "mIsSunLight", shadowMapHandles.front() == currShadowMapHandle);
				pMaterial->SetVariableForInstance(DefaultMaterialInstanceId, "mPointLightIndex", 0);
			}

			if (!pShadowCastersRenderGroup->IsEmpty())
			{
				RenderShadowCasters(pGraphicsContext, pResourceManager, currShadowMapHandle, [&]
				{
					ExecuteDrawCommands(pGraphicsContext, pResourceManager, pGlobalShaderProperties, pShadowCastersRenderGroup, shadowMapHandles.back() == currShadowMapHandle);
				});
			}
			else
			{
				RenderShadowCasters(pGraphicsContext, pResourceManager, currShadowMapHandle, nullptr);
			}
		}

		return RC_OK;
	}


#if TDE2_EDITORS_ENABLED

	static E_RESULT_CODE ProcessEditorSelectionBuffer(TPtr<IGraphicsContext> pGraphicsContext, TPtr<IResourceManager> pResourceManager, TPtr<IGlobalShaderProperties> pGlobalShaderProperties,
													ISelectionManager* pSelectionManager, TPtr<CRenderQueue> pRenderGroup)
	{
		if (!pRenderGroup)
		{
			LOG_ERROR("[ForwardRenderer] Invalid \"Editor Only\" commands buffer was found");
			return RC_FAIL;
		}

		TDE2_PROFILER_SCOPE("Renderer::RenderSelectionBuffer");
		TDE_RENDER_SECTION(pGraphicsContext, "RenderSelectionBuffer");

		if (!pRenderGroup->IsEmpty() && pSelectionManager)
		{
			if (pSelectionManager->BuildSelectionMap([&, pRenderGroup]
			{
				ExecuteDrawCommands(pGraphicsContext, pResourceManager, pGlobalShaderProperties, pRenderGroup, true);
				return RC_OK;
			}) != RC_OK)
			{
				TDE2_ASSERT(false);
			}
		}

		return RC_OK;
	}

#endif


	static inline E_RESULT_CODE RenderMainPasses(TPtr<IGraphicsContext> pGraphicsContext, TPtr<IResourceManager> pResourceManager, TPtr<IGlobalShaderProperties> pGlobalShaderProperties,
										TPtr<IFramePostProcessor> pFramePostProcessor, TPtr<CRenderQueue> pRenderQueues[])
	{
		TDE2_PROFILER_SCOPE("Renderer::RenderAll");
		TDE_RENDER_SECTION(pGraphicsContext, "RenderMainPass");

		pFramePostProcessor->PreRender();

		pFramePostProcessor->Render([&]
		{
			pGraphicsContext->ClearDepthBuffer(1.0f);

			const U8 firstGroupId = static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_FIRST_GROUP);
			const U8 lastGroupId = static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_LAST_GROUP);

			TPtr<CRenderQueue> pCurrCommandBuffer;

			for (U8 currGroup = firstGroupId; currGroup <= lastGroupId; ++currGroup)
			{
				pCurrCommandBuffer = pRenderQueues[currGroup];

				if (!pCurrCommandBuffer || pCurrCommandBuffer->IsEmpty() || static_cast<E_RENDER_QUEUE_GROUP>(currGroup) == E_RENDER_QUEUE_GROUP::RQG_OVERLAY)
				{
					continue;
				}

				ExecuteDrawCommands(pGraphicsContext, pResourceManager, pGlobalShaderProperties, pCurrCommandBuffer, true, (std::numeric_limits<U32>::max)());
			}
		}, E_FRAME_RENDER_PARAMS_FLAGS::BIND_DEPTH_BUFFER |E_FRAME_RENDER_PARAMS_FLAGS::CLEAR_RENDER_TARGET | E_FRAME_RENDER_PARAMS_FLAGS::RENDER_MAIN);

		return RC_OK;
	}


	static E_RESULT_CODE RenderOverlayAndPostEffects(TPtr<IGraphicsContext> pGraphicsContext, TPtr<IResourceManager> pResourceManager, TPtr<IGlobalShaderProperties> pGlobalShaderProperties,
		TPtr<IFramePostProcessor> pFramePostProcessor, TPtr<CRenderQueue> pUIRenderGroup, TPtr<CRenderQueue> pDebugUIRenderGroup)
	{
		if (!pUIRenderGroup)
		{
			LOG_ERROR("[ForwardRenderer] Invalid \"Overlays\" commands buffer was found");
			return RC_INVALID_ARGS;
		}

		{
			TDE2_PROFILER_SCOPE("CFramePostProcessor::RunPostProcess");
			TDE_RENDER_SECTION(pGraphicsContext, "PostProcessing");

			pFramePostProcessor->RunPostProcess();
		}

		{
			TDE2_PROFILER_SCOPE("Renderer::UI");
			TDE_RENDER_SECTION(pGraphicsContext, "RenderUI");

			pFramePostProcessor->Render([&] /// Render UI elements
			{
				ExecuteDrawCommands(pGraphicsContext, pResourceManager, pGlobalShaderProperties, pUIRenderGroup, true);
			}, E_FRAME_RENDER_PARAMS_FLAGS::CLEAR_RENDER_TARGET | E_FRAME_RENDER_PARAMS_FLAGS::RENDER_UI);
		}

		{
			TDE_RENDER_SECTION(pGraphicsContext, "FinalOutput");

			pFramePostProcessor->PostRender();

			if (pUIRenderGroup->IsEmpty())
			{
				return RC_FAIL;
			}

			pGraphicsContext->ClearDepthBuffer(1.0f);
			ExecuteDrawCommands(pGraphicsContext, pResourceManager, pGlobalShaderProperties, pUIRenderGroup, true);
		}

		{
			TDE_RENDER_SECTION(pGraphicsContext, "DebugUI");
			ExecuteDrawCommands(pGraphicsContext, pResourceManager, pGlobalShaderProperties, pDebugUIRenderGroup, true);
		}

		return RC_OK;
	}


	E_RESULT_CODE CForwardRenderer::Draw(F32 currTime, F32 deltaTime)
	{
		TDE2_PROFILER_SCOPE("Renderer::Draw"); 

		if (!mIsInitialized)
		{
			return RC_FAIL;
		}
		
		{
			TDE2_BUILTIN_SPEC_PROFILER_EVENT(E_SPECIAL_PROFILE_EVENT::RENDER);
			TDE2_STATS_COUNTER_SET(mDrawCallsCount, 0);

			mpFrameGraph->Reset();

			// \todo shadow pass
			// \todo directional shadow pass
			// \todo omni shadow pass
			// 
			// \todo main pass
			// 
			// \todo volumetric clouds main pass
			// \todo volumetric clouds blur pass
			// \todo volumetric clouds compose pass
			// 
			// \todo eye-adaptation pass
			// 
			// \todo bloom threshold
			// \todo bloom horizontal pass
			// \todo bloom vertical pass
			// \todo bloom compose
			// 
			// \todo tone-mapping pass
			// \todo ui pass
			// \todo compose pass
			// \todo imgui pass

			mpFrameGraph->Compile();
			mpFrameGraph->Execute();

			_prepareFrame(currTime, deltaTime);

#if TDE2_EDITORS_ENABLED
			ProcessEditorSelectionBuffer(mpGraphicsContext, mpResourceManager, mpGlobalShaderProperties, mpSelectionManager, mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_EDITOR_ONLY)]);
#endif

			if (CGameUserSettings::Get()->mpIsShadowMappingEnabledCVar->Get())
			{
				ProcessShadowPass(mpGraphicsContext, mpResourceManager, mpGlobalShaderProperties, mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_SHADOW_PASS)]);
			}

			RenderMainPasses(mpGraphicsContext, mpResourceManager, mpGlobalShaderProperties, mpFramePostProcessor, mpRenderQueues);

			{
				TDE_RENDER_SECTION(mpGraphicsContext, "VolumetricCloudsPass");
				mpFramePostProcessor->RunVolumetricCloudsPass();
			}

			RenderOverlayAndPostEffects(mpGraphicsContext, mpResourceManager, mpGlobalShaderProperties, mpFramePostProcessor, 
				mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_OVERLAY)],
				mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_DEBUG_UI)]);
		}

		{
			TDE2_BUILTIN_SPEC_PROFILER_EVENT(E_SPECIAL_PROFILE_EVENT::PRESENT);
			mpGraphicsContext->Present();
		}

		mpDebugUtility->PostRender();

		return RC_OK;
	}

	void CForwardRenderer::SetCamera(const ICamera* pCamera)
	{
		mpMainCamera = pCamera;
	}

	E_RESULT_CODE CForwardRenderer::SetPostProcessProfile(const IPostProcessingProfile* pProfileResource)
	{
		if (!pProfileResource)
		{
			return RC_INVALID_ARGS;
		}

		if (!mpFramePostProcessor)
		{
			return RC_FAIL;
		}

		return mpFramePostProcessor->SetProcessingProfile(pProfileResource);
	}

	E_RESULT_CODE CForwardRenderer::SetSelectionManager(ISelectionManager* pSelectionManager)
	{
		if (!pSelectionManager)
		{
			return RC_INVALID_ARGS;
		}

		mpSelectionManager = pSelectionManager;

		return RC_OK;
	}

	E_RESULT_CODE CForwardRenderer::SetLightingData(const TLightingShaderData& lightingData)
	{
		if (Length(lightingData.mSunLightDirection) < 1e-3f)
		{
			LOG_ERROR("[ForwardRenderer] Sun light's direction could not be a zero vector");
			TDE2_ASSERT(false);

			return RC_INVALID_ARGS;
		}

		mLightingData = lightingData;
		return RC_OK;
	}
	
	E_ENGINE_SUBSYSTEM_TYPE CForwardRenderer::GetType() const
	{
		return EST_RENDERER;
	}

	CRenderQueue* CForwardRenderer::GetRenderQueue(E_RENDER_QUEUE_GROUP queueType)
	{
		return mpRenderQueues[static_cast<U8>(queueType)].Get();
	}

	TPtr<IResourceManager> CForwardRenderer::GetResourceManager() const
	{
		return mpResourceManager;
	}

	TPtr<IGlobalShaderProperties> CForwardRenderer::GetGlobalShaderProperties() const
	{
		return mpGlobalShaderProperties;
	}

	void CForwardRenderer::_prepareFrame(F32 currTime, F32 deltaTime)
	{
		TDE2_PROFILER_SCOPE("Renderer::PreRender");
		TDE_RENDER_SECTION(mpGraphicsContext, "PreRender");

		///set up global shader properties for TPerFrameShaderData buffer
		TPerFrameShaderData perFrameShaderData;

		perFrameShaderData.mLightingData = mLightingData;
		
		if (mpMainCamera)
		{
			perFrameShaderData.mProjMatrix        = Transpose(mpMainCamera->GetProjMatrix());
			perFrameShaderData.mViewMatrix        = Transpose(mpMainCamera->GetViewMatrix());
			perFrameShaderData.mInvProjMatrix     = Transpose(Inverse(mpMainCamera->GetProjMatrix()));
			perFrameShaderData.mInvViewMatrix     = Transpose(Inverse(mpMainCamera->GetViewMatrix()));
			perFrameShaderData.mInvViewProjMatrix = Transpose(mpMainCamera->GetInverseViewProjMatrix());
			perFrameShaderData.mCameraPosition    = TVector4(mpMainCamera->GetPosition(), 1.0f);
		}

		perFrameShaderData.mTime = TVector4(currTime, deltaTime, 0.0f, 0.0f);

		mpGlobalShaderProperties->SetInternalUniformsBuffer(IUBR_PER_FRAME, reinterpret_cast<const U8*>(&perFrameShaderData), sizeof(perFrameShaderData));

		mpGraphicsContext->ClearBackBuffer(TColor32F(0.0f, 0.0f, 0.5f, 1.0f));
		mpGraphicsContext->ClearDepthBuffer(1.0f);
		mpGraphicsContext->ClearStencilBuffer(0x0);

		mpDebugUtility->PreRender();
	}


	TDE2_API IRenderer* CreateForwardRenderer(const TRendererInitParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IRenderer, CForwardRenderer, result, params);
	}
}