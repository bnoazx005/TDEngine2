#include "./../../include/graphics/CForwardRenderer.h"
#include "./../../include/graphics/ICamera.h"
#include "./../../include/graphics/CRenderQueue.h"
#include "./../../include/core/memory/IAllocator.h"
#include "./../../include/core/memory/CLinearAllocator.h"
#include "./../../include/utils/CFileLogger.h"
#include "./../../include/core/IGraphicsContext.h"
#include "./../../include/core/IResourceManager.h"
#include "./../../include/graphics/CGlobalShaderProperties.h"
#include "./../../include/graphics/InternalShaderData.h"
#include "./../../include/graphics/CDebugUtility.h"
#include "./../../include/graphics/IGraphicsObjectManager.h"
#include "./../../include/editor/CPerfProfiler.h"
#include "./../../include/graphics/IFramePostProcessor.h"
#if TDE2_EDITORS_ENABLED
	#include "./../../include/editor/ISelectionManager.h"
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

		/*while (true)
		{
			auto queue = mpRenderQueues[(U32)E_RENDER_QUEUE_GROUP::RQG_SPRITES];

			for (int i = 0; i < 1000; ++i)
			{
				auto c = queue->SubmitDrawCommand<TDrawIndexedInstancedCommand>(i);
			}

			queue->Sort();
			queue->Clear();
		}*/

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CForwardRenderer::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

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

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CForwardRenderer::Draw()
	{
		TDE2_PROFILER_SCOPE("Renderer::Draw");

		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		_prepareFrame();

		auto executeCommands = [this](CRenderQueue* pCommandsBuffer, bool shouldClearBuffers)
		{
			pCommandsBuffer->Sort();
			_submitToDraw(pCommandsBuffer);

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

				if (!pCurrCommandBuffer || pCurrCommandBuffer->IsEmpty() || (static_cast<E_RENDER_QUEUE_GROUP>(currGroup) == E_RENDER_QUEUE_GROUP::RQG_OVERLAY))
				{
					continue;
				}

				executeCommands(pCurrCommandBuffer, shouldClearBuffers);
			}
		};

#if TDE2_EDITORS_ENABLED
		if (mpSelectionManager)
		{
			if (mpSelectionManager->BuildSelectionMap([this, &executeCommands] 
			{ 
				CRenderQueue* pCurrCommandBuffer = mpRenderQueues[static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_EDITOR_ONLY)];
				if (!pCurrCommandBuffer)
				{
					LOG_ERROR("[ForwardRenderer] Invalid \"Editor Only\" commands buffer was found");
					return RC_FAIL;
				}

				mpGraphicsContext->ClearDepthBuffer(1.0f);

				executeCommands(pCurrCommandBuffer, true);
			}) != RC_OK)
			{
				TDE2_ASSERT(false);
			}
		}
#endif

		mpGraphicsContext->ClearDepthBuffer(1.0f);

		mpFramePostProcessor->Render([&renderAllGroups] { renderAllGroups(true); });

		// \note draw UI meshes and screen-space effects
		{
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

	void CForwardRenderer::_submitToDraw(CRenderQueue* pRenderQueue)
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

			pCurrDrawCommand->Submit(mpGraphicsContext, mpResourceManager, mpGlobalShaderProperties);
		}
	}

	void CForwardRenderer::_prepareFrame()
	{
		///set up global shader properties for TPerFrameShaderData buffer
		TPerFrameShaderData perFrameShaderData;
		
		if (mpMainCamera)
		{
			perFrameShaderData.mProjMatrix = Transpose(mpMainCamera->GetProjMatrix());
			perFrameShaderData.mViewMatrix = Transpose(mpMainCamera->GetViewMatrix());
		}

		mpGlobalShaderProperties->SetInternalUniformsBuffer(IUBR_PER_FRAME, reinterpret_cast<const U8*>(&perFrameShaderData), sizeof(perFrameShaderData));

		mpGraphicsContext->ClearBackBuffer(TColor32F(0.0f, 0.0f, 0.5f, 1.0f));
		mpGraphicsContext->ClearDepthBuffer(1.0f);

		mpDebugUtility->PreRender();
	}


	TDE2_API IRenderer* CreateForwardRenderer(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, IAllocator* pTempAllocator, 
											  IFramePostProcessor* pFramePostProcessor, E_RESULT_CODE& result)
	{
		CForwardRenderer* pRenderer = new (std::nothrow) CForwardRenderer();

		if (!pRenderer)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pRenderer->Init(pGraphicsContext, pResourceManager, pTempAllocator, pFramePostProcessor);

		if (result != RC_OK)
		{
			delete pRenderer;

			pRenderer = nullptr;
		}

		return pRenderer;
	}
}