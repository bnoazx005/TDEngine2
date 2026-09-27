#include "../../include/graphics/CFramePacketsStorage.h"
#include "../../include/graphics/CRenderQueue.h"
#include "../../include/core/memory/IAllocator.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/editor/CPerfProfiler.h"
#include "stringUtils.hpp"


namespace TDEngine2
{
	void TFramePacket::ClearTransientData()
	{
		mMaterialProxies.clear();

		for (TPtr<CRenderQueue>& pQueue : mpRenderQueues)
		{
			if (!pQueue)
			{
				continue;
			}

			pQueue->Clear();
		}
	}

	TMaterialProxyId TFramePacket::GetOrCreateProxy(TMaterialRenderProxy&& proxy)
	{
		const USIZE index = mMaterialProxies.size();		
		mMaterialProxies.emplace_back(proxy);		
		return static_cast<TMaterialProxyId>(index);
	}

	const TMaterialRenderProxy* TFramePacket::GetProxy(TMaterialProxyId id) const
	{
		if (id == TMaterialProxyId::Invalid)
		{
			return nullptr;
		}

		TDE2_ASSERT(static_cast<USIZE>(id) < mMaterialProxies.size());
		if (static_cast<USIZE>(id) >= mMaterialProxies.size())
		{
			return nullptr;
		}

		return &mMaterialProxies[static_cast<USIZE>(id)];
	}


	/*!
		\brief CFramePacketsStorage's definition
	*/


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
		U64 slotIndex = (std::numeric_limits<U64>::max)();

		while (slotIndex == (std::numeric_limits<U64>::max)())
		{
			for (U64 i = 0; i < MAX_FRAME_PACKETS_COUNT; ++i)
			{
				E_PACKET_STATE expected = E_PACKET_STATE::EMPTY;
				if (mFramePacketsState[i].compare_exchange_strong(expected, E_PACKET_STATE::WRITING))
				{
					slotIndex = i;
					break;
				}
			}

			if (slotIndex == (std::numeric_limits<U64>::max)())
			{
				std::this_thread::sleep_for(std::chrono::microseconds(100));
			}
		}

		mCurrWritingIndex.store(slotIndex);

		TFramePacket& packet = mFramePackets[slotIndex];
		packet.ClearTransientData();

		return packet;
	}

	E_RESULT_CODE CFramePacketsStorage::SubmitGameLogicFramePacket()
	{
		const U64 slotIndex = mCurrWritingIndex.exchange((std::numeric_limits<U64>::max)());
		if (slotIndex == (std::numeric_limits<U64>::max)())
		{
			TDE2_ASSERT_MSG(false, "[CFramePacketsStorage] Submit called without prior Acquire");
			return RC_FAIL;
		}

		mFramePacketsState[slotIndex].store(E_PACKET_STATE::READY);

		const U64 prevLatestIndex = mLatestReadyPacketIndex.exchange(slotIndex);

		if (prevLatestIndex != (std::numeric_limits<U64>::max)() && prevLatestIndex != slotIndex)
		{
			E_PACKET_STATE expected = E_PACKET_STATE::READY;
			mFramePacketsState[prevLatestIndex].compare_exchange_strong(expected, E_PACKET_STATE::EMPTY);
		}

		mSignal.notify_one();

		return RC_OK;
	}

	TFramePacket* CFramePacketsStorage::AcquireRenderLogicFramePacket(std::chrono::milliseconds timeout)
	{
		std::unique_lock<std::mutex> lock(mMutex);

		mSignal.wait_for(lock, timeout, [this] 
			{
				return mLatestReadyPacketIndex.load(std::memory_order_acquire) != (std::numeric_limits<U64>::max)(); 
			});

		const U64 slotIndex = mLatestReadyPacketIndex.exchange((std::numeric_limits<U64>::max)(), std::memory_order_acq_rel);
		if (slotIndex == (std::numeric_limits<U64>::max)())
		{
			return nullptr;
		}

		mFramePacketsState[slotIndex].store(E_PACKET_STATE::READING);
		mCurrReadingIndex.store(slotIndex);

		return &mFramePackets[slotIndex];
	}

	E_RESULT_CODE CFramePacketsStorage::SubmitRenderLogicFramePacket()
	{
		const U64 slotIndex = mCurrReadingIndex.exchange((std::numeric_limits<U64>::max)());
		if (slotIndex == (std::numeric_limits<U64>::max)())
		{
			return RC_FAIL;
		}

		mFramePacketsState[slotIndex].store(E_PACKET_STATE::EMPTY);

		return RC_OK;
	}

	void CFramePacketsStorage::NotifyAll()
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mSignal.notify_all();
	}


	TDE2_API CFramePacketsStorage* CreateFramePacketsStorage(TAllocatorFactoryFunctor allocatorFactoryFunctor, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CFramePacketsStorage, CFramePacketsStorage, result, allocatorFactoryFunctor);
	}
}