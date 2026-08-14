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

		U64 frameIndex = 0;

		for (auto& currFramePacket : mFramePackets)
		{
			for (U8 i = 0; i < NumOfRenderQueuesGroup; ++i)
			{
				IAllocator* pRenderGroupAllocator = allocatorFactoryFunctor(PerRenderQueueMemoryBlockSize, result);
				if (result != RC_OK)
				{
					return result;
				}

				/// \note this CRenderQueue's instance now owns this allocators
				currFramePacket.mpRenderQueues[i] = TPtr<CRenderQueue>(CreateRenderQueue(pRenderGroupAllocator, result));
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

	TFramePacket& CFramePacketsStorage::AcquireGameLogicFramePacket()
	{
		const U64 nextIndex = static_cast<U64>((mCurrGameLogicFrameIndex.load() + 1) & (MAX_FRAME_PACKETS_COUNT - 1));

		while (true)
		{
			E_PACKET_STATE expectedState = E_PACKET_STATE::EMPTY;

			if (mFramePacketsState[nextIndex].compare_exchange_weak(expectedState, E_PACKET_STATE::WRITING))
			{
				break;
			}

			std::this_thread::yield();
		}

		mFramePacketsState[nextIndex].store(E_PACKET_STATE::EMPTY);
		++mCurrGameLogicFrameIndex;

		return mFramePackets[nextIndex];
	}

	E_RESULT_CODE CFramePacketsStorage::SubmitGameLogicFramePacket()
	{
		const U64 currentIndex = static_cast<U64>((mCurrGameLogicFrameIndex.load()) & (MAX_FRAME_PACKETS_COUNT - 1));
		mFramePacketsState[currentIndex].store(E_PACKET_STATE::READY);

		mSignal.notify_one();

		return RC_OK;
	}

	TFramePacket& CFramePacketsStorage::AcquireRenderLogicFramePacket()
	{
		const U64 nextIndex = static_cast<U64>((mCurrRenderFrameIndex.load() + 1) & (MAX_FRAME_PACKETS_COUNT - 1));

		std::unique_lock<std::mutex> lock(mMutex);
		mSignal.wait(lock, [this, nextIndex] { return mFramePacketsState[nextIndex].load() == E_PACKET_STATE::READY; });

		++mCurrRenderFrameIndex;

		return mFramePackets[nextIndex];
	}

	E_RESULT_CODE CFramePacketsStorage::SubmitRenderLogicFramePacket()
	{
		const U64 currentIndex = static_cast<U64>((mCurrRenderFrameIndex.load()) & (MAX_FRAME_PACKETS_COUNT - 1));
		mFramePacketsState[currentIndex].store(E_PACKET_STATE::EMPTY);

		return RC_OK;
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

	U64 CFramePacketsStorage::GetGameLogicFrameIndex() const
	{
		return mCurrGameLogicFrameIndex;
	}

	U64 CFramePacketsStorage::GetRenderFrameIndex() const
	{
		return mCurrRenderFrameIndex;
	}


	TDE2_API CFramePacketsStorage* CreateFramePacketsStorage(TAllocatorFactoryFunctor allocatorFactoryFunctor, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CFramePacketsStorage, CFramePacketsStorage, result, allocatorFactoryFunctor);
	}
}