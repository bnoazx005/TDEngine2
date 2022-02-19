#include "../../include/graphics/CForwardRenderer.h"
#include "../../include/graphics/ICamera.h"
#include "../../include/graphics/CRenderQueue.h"
#include "../../include/core/memory/IAllocator.h"
#include "../../include/core/memory/CLinearAllocator.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IWindowSystem.h"
#include "../../include/core/CProjectSettings.h"
#include "../../include/graphics/CGlobalShaderProperties.h"
#include "../../include/graphics/InternalShaderData.h"
#include "../../include/graphics/CDebugUtility.h"
#include "../../include/graphics/IGraphicsObjectManager.h"
#include "../../include/editor/CPerfProfiler.h"
#include "../../include/graphics/IFramePostProcessor.h"
#include "../../include/graphics/CBaseRenderTarget.h"
#if TDE2_EDITORS_ENABLED
	#include "../../include/editor/ISelectionManager.h"
#endif


namespace TDEngine2
{
	CForwardRenderer::CForwardRenderer():
		CBaseObject(), mpMainCamera(nullptr), mpResourceManager(nullptr), mpGlobalShaderProperties(nullptr), mpFramePostProcessor(nullptr)
	{
	}


	static TResult<TResourceId> GetOrCreateShadowMap(TPtr<IResourceManager> pResourceManager)
	{
		const U32 shadowMapSizes = CProjectSettings::Get()->mGraphicsSettings.mRendererSettings.mShadowMapSizes;
		TDE2_ASSERT(shadowMapSizes > 0 && shadowMapSizes < 65536);

		const TTexture2DParameters shadowMapParams{ shadowMapSizes, shadowMapSizes, FT_D32, 1, 1, 0 };

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

		if (!params.mAllocatorFactoryFunctor || !params.mpGraphicsContext || !params.mpResourceManager)
		{
			return RC_INVALID_ARGS;
		}

		mpGraphicsContext    = params.mpGraphicsContext;
		mpResourceManager    = params.mpResourceManager;
		mpFramePostProcessor = params.mpFramePostProcessor;

		auto allocatorFactory = params.mAllocatorFactoryFunctor;

		E_RESULT_CODE result = RC_OK;
		
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

		if (CProjectSettings::Get()->mGraphicsSettings.mRendererSettings.mIsShadowMappingEnabled)
		{
			GetOrCreateShadowMap(mpResourceManager).Get(); /// \note Create a shadow map's texture before any Update will be executed
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


	static E_RESULT_CODE ProcessShadowPass(TPtr<IGraphicsContext> pGraphicsContext, TPtr<IResourceManager> pResourceManager, TPtr<IGlobalShaderProperties> pGlobalShaderProperties,
										TPtr<CRenderQueue> pShadowCastersRenderGroup)
	{
		if (!pShadowCastersRenderGroup)
		{
			LOG_ERROR("[ForwardRenderer] Invalid \"Shadow Pass\" commands buffer was found");
			return RC_FAIL;
		}

		TDE2_PROFILER_SCOPE("Renderer::RenderShadows");

		const TResourceId shadowMapHandle = GetOrCreateShadowMap(pResourceManager).Get();

		if (!pShadowCastersRenderGroup->IsEmpty())
		{
			const F32 shadowMapSizes = static_cast<F32>(CProjectSettings::Get()->mGraphicsSettings.mRendererSettings.mShadowMapSizes);

			pGraphicsContext->SetViewport(0.0f, 0.0f, shadowMapSizes, shadowMapSizes, 0.0f, 1.0f);
			{
				pGraphicsContext->BindDepthBufferTarget(pResourceManager->GetResource<IDepthBufferTarget>(shadowMapHandle).Get(), true);

				pGraphicsContext->ClearDepthBuffer(1.0f);

				ExecuteDrawCommands(pGraphicsContext, pResourceManager, pGlobalShaderProperties, pShadowCastersRenderGroup, true);

				pGraphicsContext->BindDepthBufferTarget(nullptr);
			}

			if (auto pWindowSystem = pGraphicsContext->GetWindowSystem())
			{
				pGraphicsContext->SetViewport(0.0f, 0.0f, static_cast<F32>(pWindowSystem->GetWidth()), static_cast<F32>(pWindowSystem->GetHeight()), 0.0f, 1.0f);
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
		pGraphicsContext->ClearDepthBuffer(1.0f);

		pFramePostProcessor->Render([&]
		{
			TDE2_PROFILER_SCOPE("Renderer::RenderAll");

			const U8 firstGroupId = static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_FIRST_GROUP);
			const U8 lastGroupId = static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_LAST_GROUP);

			TPtr<CRenderQueue> pCurrCommandBuffer;

			for (U8 currGroup = firstGroupId; currGroup <= lastGroupId; ++currGroup)
			{
				pCurrCommandBuffer = pRenderQueues[currGroup];

				if (!pCurrCommandBuffer || pCurrCommandBuffer->IsEmpty())
				{
					continue;
				}

				const bool isOverlayCommandBuffer = (static_cast<E_RENDER_QUEUE_GROUP>(currGroup) == E_RENDER_QUEUE_GROUP::RQG_OVERLAY);

				ExecuteDrawCommands(pGraphicsContext, pResourceManager, pGlobalShaderProperties, pCurrCommandBuffer, true,
					isOverlayCommandBuffer ? static_cast<U32>(E_GEOMETRY_SUBGROUP_TAGS::IMAGE_EFFECTS) : (std::numeric_limits<U32>::max)());
			}
		});

		return RC_OK;
	}


	static E_RESULT_CODE RenderOverlayAndPostEffects(TPtr<IGraphicsContext> pGraphicsContext, TPtr<IResourceManager> pResourceManager, TPtr<IGlobalShaderProperties> pGlobalShaderProperties,
													TPtr<CRenderQueue> pRenderGroup)
	{
		if (!pRenderGroup)
		{
			LOG_ERROR("[ForwardRenderer] Invalid \"Overlays\" commands buffer was found");
			return RC_INVALID_ARGS;
		}

		TDE2_PROFILER_SCOPE("Renderer::UI");

		if (pRenderGroup->IsEmpty())
		{
			return RC_FAIL;
		}

		pGraphicsContext->ClearDepthBuffer(1.0f);

		ExecuteDrawCommands(pGraphicsContext, pResourceManager, pGlobalShaderProperties, pRenderGroup, true);

		return RC_OK;
	}


	E_RESULT_CODE CForwardRenderer::Draw(F32 currTime, F32 deltaTime)
	{
		TDE2_PROFILER_SCOPE("Renderer::Draw");

		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		_prepareFrame(currTime, deltaTime);

#if TDE2_EDITORS_ENABLED
		ProcessEditorSelectionBuffer(mpGraphicsContext, mpResourceManager, mpGlobalShaderProperties, mpSelectionManager, mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_EDITOR_ONLY)]);
#endif

		if (CProjectSettings::Get()->mGraphicsSettings.mRendererSettings.mIsShadowMappingEnabled)
		{
			ProcessShadowPass(mpGraphicsContext, mpResourceManager, mpGlobalShaderProperties, mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_SHADOW_PASS)]);
		}
		
		RenderMainPasses(mpGraphicsContext, mpResourceManager, mpGlobalShaderProperties, mpFramePostProcessor, mpRenderQueues);
		RenderOverlayAndPostEffects(mpGraphicsContext, mpResourceManager, mpGlobalShaderProperties, mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_OVERLAY)]);

		mpGraphicsContext->Present();

		mpDebugUtility->PostRender();

		return RC_OK;
	}

	void CForwardRenderer::SetCamera(const ICamera* pCamera)
	{
		mpMainCamera = pCamera;
	}

	E_RESULT_CODE CForwardRenderer::SetFramePostProcessor(TPtr<IFramePostProcessor> pFramePostProcessor)
	{
		if (!pFramePostProcessor)
		{
			return RC_INVALID_ARGS;
		}

		mpFramePostProcessor = pFramePostProcessor;

		return RC_OK;
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

		///set up global shader properties for TPerFrameShaderData buffer
		TPerFrameShaderData perFrameShaderData;

		perFrameShaderData.mLightingData = mLightingData;
		
		if (mpMainCamera)
		{
			perFrameShaderData.mProjMatrix     = Transpose(mpMainCamera->GetProjMatrix());
			perFrameShaderData.mViewMatrix     = Transpose(mpMainCamera->GetViewMatrix());
			perFrameShaderData.mCameraPosition = TVector4(mpMainCamera->GetPosition(), 1.0f);
		}

		perFrameShaderData.mTime = TVector4(currTime, deltaTime, 0.0f, 0.0f);

		mpGlobalShaderProperties->SetInternalUniformsBuffer(IUBR_PER_FRAME, reinterpret_cast<const U8*>(&perFrameShaderData), sizeof(perFrameShaderData));

		mpGraphicsContext->ClearBackBuffer(TColor32F(0.0f, 0.0f, 0.5f, 1.0f));
		mpGraphicsContext->ClearDepthBuffer(1.0f);

		mpDebugUtility->PreRender();
	}


	TDE2_API IRenderer* CreateForwardRenderer(const TRendererInitParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IRenderer, CForwardRenderer, result, params);
	}
}