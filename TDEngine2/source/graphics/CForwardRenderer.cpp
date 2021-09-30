#include "../../include/graphics/CForwardRenderer.h"
#include "../../include/graphics/ICamera.h"
#include "../../include/graphics/CRenderQueue.h"
#include "../../include/core/memory/IAllocator.h"
#include "../../include/core/memory/CLinearAllocator.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/IWindowSystem.h"
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
		mIsInitialized(false), mpMainCamera(nullptr), mpResourceManager(nullptr), mpGlobalShaderProperties(nullptr), mpFramePostProcessor(nullptr)
	{
	}

	E_RESULT_CODE CForwardRenderer::Init(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, IAllocator* pTempAllocator,
										 IFramePostProcessor* pFramePostProcessor)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pTempAllocator || !pGraphicsContext || !pResourceManager)
		{
			return RC_INVALID_ARGS;
		}

		mpGraphicsContext    = pGraphicsContext;
		mpResourceManager    = pResourceManager;
		mpTempAllocator      = pTempAllocator;
		mpFramePostProcessor = pFramePostProcessor;

		E_RESULT_CODE result = RC_OK;
		
		IAllocator* pCurrAllocator = nullptr;

		void* pCurrMemoryBlock = nullptr;

		std::string debugMessage;
		
		for (U8 i = 0; i < NumOfRenderQueuesGroup; ++i)
		{
			pCurrMemoryBlock = pTempAllocator->Allocate(PerRenderQueueMemoryBlockSize, __alignof(U8));

			pCurrAllocator = CreateLinearAllocator(PerRenderQueueMemoryBlockSize, static_cast<U8*>(pCurrMemoryBlock), result);

			if (result != RC_OK)
			{
				return result;
			}

			mpRenderQueues[i] = CreateRenderQueue(pCurrAllocator, result);

			if (result != RC_OK)
			{
				return result;
			}
			
			LOG_MESSAGE(std::string("[Forward Renderer] A new render queue buffer was created ( mem-size : ").append(std::to_string(PerRenderQueueMemoryBlockSize / 1024)).
																											  append(" KiB; group-type : ").
																											  append(std::to_string(i)).
																											  append(")"));
		}

		auto pGraphicsObjectManager = pGraphicsContext->GetGraphicsObjectManager();

		mpGlobalShaderProperties = CreateGlobalShaderProperties(pGraphicsObjectManager, result);

		/// \todo fill in data into TConstantsShaderData buffer
		mpGlobalShaderProperties->SetInternalUniformsBuffer(IUBR_CONSTANTS, nullptr, 0);

		if (result != RC_OK)
		{
			return result;
		}

		auto debugUtilityResult = pGraphicsObjectManager->CreateDebugUtility(pResourceManager, this);

		if (debugUtilityResult.HasError())
		{
			return debugUtilityResult.GetError();
		}

		mpDebugUtility = debugUtilityResult.Get();

		// \todo Implement shadow mapping configuration in better manner
		mShadowMapWidth  = 1024;
		mShadowMapHeight = 1024;

		TTexture2DParameters shadowMapParams{ mShadowMapWidth, mShadowMapHeight, FT_D32, 1, 1, 0 };

		mShadowMapHandle = mpResourceManager->Create<IDepthBufferTarget>("ShadowMap", shadowMapParams);
		if (mShadowMapHandle == TResourceId::Invalid)
		{
			TDE2_ASSERT(false);
			return RC_FAIL;
		}

		if (auto pShadowMapBuffer = mpResourceManager->GetResource<IDepthBufferTarget>(mShadowMapHandle))
		{
			pShadowMapBuffer->SetUWrapMode(E_ADDRESS_MODE_TYPE::AMT_CLAMP);
			pShadowMapBuffer->SetVWrapMode(E_ADDRESS_MODE_TYPE::AMT_CLAMP);
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CForwardRenderer::_onFreeInternal()
	{
		/*!
			\bug The piece of code below throws an exception because of corrupted memory.
			
			\note I guess the reason of this is the renderer should be freed before any other subsystem.
			In other words all buffers are released by IGraphicsObjectManager automatically, so
			we don't really need to call Free method by ourselves

			\todo Think we should introduce a priority of deleting for IEngineSubsystem implementations
		*/

		/*if ((result = mpGlobalShaderProperties->Free()) != RC_OK)
		{
			return result;
		}*/

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

		auto executeCommands = [this](CRenderQueue* pCommandsBuffer, bool shouldClearBuffers, U32 upperRenderIndexLimit = (std::numeric_limits<U32>::max)())
		{
			pCommandsBuffer->Sort();
			_submitToDraw(pCommandsBuffer, upperRenderIndexLimit);

			if (shouldClearBuffers)
			{
				pCommandsBuffer->Clear();
			}
		};

		auto renderAllGroups = [this, &executeCommands](bool shouldClearBuffers = true)
		{
			const U8 firstGroupId = static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_FIRST_GROUP);
			const U8 lastGroupId = static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_LAST_GROUP);

			CRenderQueue* pCurrCommandBuffer = nullptr;

			for (U8 currGroup = firstGroupId; currGroup <= lastGroupId; ++currGroup)
			{
				pCurrCommandBuffer = mpRenderQueues[currGroup];

				if (!pCurrCommandBuffer || pCurrCommandBuffer->IsEmpty())
				{
					continue;
				}

				const bool isOverlayCommandBuffer = (static_cast<E_RENDER_QUEUE_GROUP>(currGroup) == E_RENDER_QUEUE_GROUP::RQG_OVERLAY);

				executeCommands(pCurrCommandBuffer, shouldClearBuffers, isOverlayCommandBuffer ? static_cast<U32>(E_GEOMETRY_SUBGROUP_TAGS::IMAGE_EFFECTS) : (std::numeric_limits<U32>::max)());
			}
		};

#if TDE2_EDITORS_ENABLED
		if (CRenderQueue* pCurrCommandBuffer = mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_EDITOR_ONLY)])
		{
			TDE2_PROFILER_SCOPE("Renderer::RenderSelectionBuffer");

			if (!pCurrCommandBuffer->IsEmpty() && mpSelectionManager)
			{
				if (mpSelectionManager->BuildSelectionMap([this, &executeCommands, pCurrCommandBuffer]
				{
					executeCommands(pCurrCommandBuffer, true);
					return RC_OK;
				}) != RC_OK)
				{
					TDE2_ASSERT(false);
				}
			}			
		}
		else
		{
			LOG_ERROR("[ForwardRenderer] Invalid \"Editor Only\" commands buffer was found");
			return RC_FAIL;
		}
#endif

		// \note Shadow pass
		if (CRenderQueue* pShadowRenderQueue = mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_SHADOW_PASS)])
		{
			TDE2_PROFILER_SCOPE("Renderer::RenderShadows");

			if (!pShadowRenderQueue->IsEmpty())
			{
				mpGraphicsContext->SetViewport(0.0f, 0.0f, static_cast<F32>(mShadowMapWidth), static_cast<F32>(mShadowMapHeight), 0.0f, 1.0f);
				{
					mpGraphicsContext->BindDepthBufferTarget(dynamic_cast<IDepthBufferTarget*>(mpResourceManager->GetResource(mShadowMapHandle)), true);

					mpGraphicsContext->ClearDepthBuffer(1.0f);

					executeCommands(pShadowRenderQueue, true);

					mpGraphicsContext->BindDepthBufferTarget(nullptr);
				}

				if (auto pWindowSystem = mpGraphicsContext->GetWindowSystem())
				{
					mpGraphicsContext->SetViewport(0.0f, 0.0f, static_cast<F32>(pWindowSystem->GetWidth()), static_cast<F32>(pWindowSystem->GetHeight()), 0.0f, 1.0f);
				}
			}			
		}
		else
		{
			LOG_ERROR("[ForwardRenderer] Invalid \"Shadow Pass\" commands buffer was found");
			return RC_FAIL;
		}

		mpGraphicsContext->ClearDepthBuffer(1.0f);

		mpFramePostProcessor->Render([&renderAllGroups] 
		{
			TDE2_PROFILER_SCOPE("Renderer::RenderAll");
			renderAllGroups(true); 
		});

		// \note draw UI meshes and screen-space effects
		{
			TDE2_PROFILER_SCOPE("Renderer::UI");

			CRenderQueue* pCurrCommandBuffer = mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_OVERLAY)];
			if (!pCurrCommandBuffer || pCurrCommandBuffer->IsEmpty())
			{
				LOG_ERROR("[ForwardRenderer] Invalid \"Overlays\" commands buffer was found");
				return RC_FAIL;
			}

			mpGraphicsContext->ClearDepthBuffer(1.0f);

			executeCommands(pCurrCommandBuffer, true);
		}

		mpGraphicsContext->Present();

		mpDebugUtility->PostRender();

		return RC_OK;
	}

	void CForwardRenderer::SetCamera(const ICamera* pCamera)
	{
		mpMainCamera = pCamera;
	}

	E_RESULT_CODE CForwardRenderer::SetFramePostProcessor(IFramePostProcessor* pFramePostProcessor)
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
			return RC_INVALID_ARGS;
		}

		mLightingData = lightingData;
		return RC_OK;
	}
	
	E_ENGINE_SUBSYSTEM_TYPE CForwardRenderer::GetType() const
	{
		return EST_RENDERER;
	}

	CRenderQueue* CForwardRenderer::GetRenderQueue(E_RENDER_QUEUE_GROUP queueType) const
	{
		return mpRenderQueues[static_cast<U8>(queueType)];
	}

	IResourceManager* CForwardRenderer::GetResourceManager() const
	{
		return mpResourceManager;
	}

	IGlobalShaderProperties* CForwardRenderer::GetGlobalShaderProperties() const
	{
		return mpGlobalShaderProperties;
	}

	void CForwardRenderer::_submitToDraw(CRenderQueue* pRenderQueue, U32 upperRenderIndexLimit)
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

			pCurrDrawCommand->Submit(mpGraphicsContext, mpResourceManager, mpGlobalShaderProperties);
		}
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


	TDE2_API IRenderer* CreateForwardRenderer(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, IAllocator* pTempAllocator, 
											  IFramePostProcessor* pFramePostProcessor, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IRenderer, CForwardRenderer, result, pGraphicsContext, pResourceManager, pTempAllocator, pFramePostProcessor);
	}
}