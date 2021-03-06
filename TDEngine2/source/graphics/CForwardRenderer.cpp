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


namespace TDEngine2
{
	CForwardRenderer::CForwardRenderer():
		mIsInitialized(false), mpMainCamera(nullptr), mpResourceManager(nullptr), mpGlobalShaderProperties(nullptr)
	{
	}

	E_RESULT_CODE CForwardRenderer::Init(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, IAllocator* pTempAllocator)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pTempAllocator || !pGraphicsContext || !pResourceManager)
		{
			return RC_INVALID_ARGS;
		}

		mpGraphicsContext = pGraphicsContext;

		mpResourceManager = pResourceManager;

		mpTempAllocator = pTempAllocator;

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
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		_prepareFrame();

		CRenderQueue* pCurrCommandBuffer = nullptr;

		const U8 firstGroupId = static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_FIRST_GROUP);
		const U8 lastGroupId  = static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_LAST_GROUP);

		for (U8 currGroup = firstGroupId; currGroup <= lastGroupId; ++currGroup)
		{
			pCurrCommandBuffer = mpRenderQueues[currGroup];

			if (!pCurrCommandBuffer || pCurrCommandBuffer->IsEmpty())
			{
				continue;
			}

			pCurrCommandBuffer->Sort();

			_submitToDraw(pCurrCommandBuffer);

			pCurrCommandBuffer->Clear();
		}

		mpGraphicsContext->Present();

		mpDebugUtility->PostRender();

		return RC_OK;
	}

	void CForwardRenderer::SetCamera(const ICamera* pCamera)
	{
		mpMainCamera = pCamera;
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


	TDE2_API IRenderer* CreateForwardRenderer(IGraphicsContext* pGraphicsContext, IResourceManager* pResourceManager, IAllocator* pTempAllocator, E_RESULT_CODE& result)
	{
		CForwardRenderer* pRenderer = new (std::nothrow) CForwardRenderer();

		if (!pRenderer)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pRenderer->Init(pGraphicsContext, pResourceManager, pTempAllocator);

		if (result != RC_OK)
		{
			delete pRenderer;

			pRenderer = nullptr;
		}

		return pRenderer;
	}
}