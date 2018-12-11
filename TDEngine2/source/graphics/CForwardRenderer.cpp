#include "./../../include/graphics/CForwardRenderer.h"
#include "./../../include/graphics/ICamera.h"
#include "./../../include/graphics/CRenderQueue.h"
#include "./../../include/core/memory/IAllocator.h"
#include "./../../include/core/memory/CLinearAllocator.h"
#include "./../../include/utils/CFileLogger.h"


namespace TDEngine2
{
	CForwardRenderer::CForwardRenderer():
		mIsInitialized(false), mpMainCamera(nullptr)
	{
	}

	E_RESULT_CODE CForwardRenderer::Init(IAllocator* pTempAllocator)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pTempAllocator)
		{
			return RC_INVALID_ARGS;
		}

		mpTempAllocator = pTempAllocator;

		E_RESULT_CODE result = RC_OK;

		U8 numOfQueues = static_cast<U8>(E_RENDER_QUEUE_GROUP::RQG_LAST_GROUP);

		IAllocator* pCurrAllocator = nullptr;

		void* pCurrMemoryBlock = nullptr;

		std::string debugMessage;
		
		for (U8 i = 0; i < numOfQueues; ++i)
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

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CForwardRenderer::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}
	
	void CForwardRenderer::SubmitToRender(U8 group, const IRenderable* pRenderable)
	{

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


	TDE2_API IRenderer* CreateForwardRenderer(IAllocator* pTempAllocator, E_RESULT_CODE& result)
	{
		CForwardRenderer* pRenderer = new (std::nothrow) CForwardRenderer();

		if (!pRenderer)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pRenderer->Init(pTempAllocator);

		if (result != RC_OK)
		{
			delete pRenderer;

			pRenderer = nullptr;
		}

		return pRenderer;
	}
}