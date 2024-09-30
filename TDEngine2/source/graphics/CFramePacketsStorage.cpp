#include "../../include/graphics/CFramePacketsStorage.h"
#include "../../include/graphics/CRenderQueue.h"
#include "../../include/core/memory/IAllocator.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/editor/CPerfProfiler.h"
#include "stringUtils.hpp"


namespace TDEngine2
{
	CFramePacketsStorage::CFramePacketsStorage() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CFramePacketsStorage::Init(TAllocatorFactoryFunctor allocatorFactoryFunctor)
	{
		TDE2_PROFILER_SCOPE("CFramePacketsStorage::Init");

		if (!allocatorFactoryFunctor)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = RC_OK;

		U32 frameIndex = 0;

		for (auto& currFramePacket : mFramePackets)
		{
			for (U8 i = 0; i < NumOfRenderQueuesGroup; ++i)
			{
				currFramePacket.mpRenderQueuesAllocators[i] = TPtr<IAllocator>(allocatorFactoryFunctor(PerRenderQueueMemoryBlockSize, result));
				if (result != RC_OK)
				{
					return result;
				}

				/// \note this CRenderQueue's instance now owns this allocator
				currFramePacket.mpRenderQueues[i] = TPtr<CRenderQueue>(CreateRenderQueue(currFramePacket.mpRenderQueuesAllocators[i].Get(), result));
				if (result != RC_OK)
				{
					return result;
				}

				LOG_MESSAGE(Wrench::StringUtils::Format("[Forward Renderer] A new render queue buffer was created (mem-size: {0} KiB; group-type: {1}; frame-index: {2}", PerRenderQueueMemoryBlockSize / 1024, static_cast<U16>(i), frameIndex));
			}

			++frameIndex;
		}

		mIsInitialized = true;

		return result;
	}

	void CFramePacketsStorage::IncrementGameLogicFrameCounter()
	{
		mCurrGameLogicFrameIndex = (mCurrGameLogicFrameIndex + 1) & (MAX_FRAME_PACKETS_COUNT - 1);
	}

	void CFramePacketsStorage::IncrementRenderFrameCounter()
	{
		mCurrRenderFrameIndex = (mCurrRenderFrameIndex + 1) & (MAX_FRAME_PACKETS_COUNT - 1);
	}

	TFramePacket& CFramePacketsStorage::GetCurrentFrameForGameLogic()
	{
		return mFramePackets[mCurrGameLogicFrameIndex];
	}

	TFramePacket& CFramePacketsStorage::GetCurrentFrameForRender()
	{
		return mFramePackets[mCurrRenderFrameIndex];
	}

	U32 CFramePacketsStorage::GetGameLogicFrameIndex() const
	{
		return mCurrGameLogicFrameIndex;
	}

	U32 CFramePacketsStorage::GetRenderFrameIndex() const
	{
		return mCurrRenderFrameIndex;
	}


	TDE2_API CFramePacketsStorage* CreateFramePacketsStorage(TAllocatorFactoryFunctor allocatorFactoryFunctor, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CFramePacketsStorage, CFramePacketsStorage, result, allocatorFactoryFunctor);
	}
}