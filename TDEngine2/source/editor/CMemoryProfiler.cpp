#include "../../include/editor/CMemoryProfiler.h"
#include "../../include/utils/CFileLogger.h"
#include "stringUtils.hpp"
#include "backward.hpp"
#include <sstream>
#include "tracy/Tracy.hpp"



#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	TDE2_STATIC_CONSTEXPR U32 MAX_CALLSTACK_DEPTH = 10;


	CMemoryProfiler::CMemoryProfiler() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CMemoryProfiler::BeginFrame()
	{
		return RC_OK;
	}

	E_RESULT_CODE CMemoryProfiler::EndFrame()
	{
		return RC_OK;
	}

	E_RESULT_CODE CMemoryProfiler::SetTotalMemoryAvailable(USIZE size)
	{
		mTotalMemorySize = size;
		return RC_OK;
	}

	E_RESULT_CODE CMemoryProfiler::RegisterGlobalMemoryBlock(const std::string& name, USIZE offset, USIZE size)
	{
		auto it = mBlocksInfoRegistry.find(name);
		if (it != mBlocksInfoRegistry.cend())
		{
			return RC_FAIL;
		}

		mBlocksInfoRegistry.insert({ name, { offset, 0, size } });

		return RC_OK;
	}

	E_RESULT_CODE CMemoryProfiler::UpdateMemoryBlockInfo(const std::string& name, USIZE usedSize)
	{
		auto it = mBlocksInfoRegistry.find(name);
		if ((it == mBlocksInfoRegistry.cend()) || (it->second.mTotalSize < usedSize))
		{
			return RC_FAIL;
		}

		(it->second).mCurrOccupiedSize = usedSize;

		return RC_OK;
	}


	static std::string GetStackTrace() {
		std::ostringstream ss;

		backward::StackTrace stackTrace;
		backward::TraceResolver resolver;
		stackTrace.load_here();
		resolver.load_stacktrace(stackTrace);

		for (std::size_t i = 0; i < stackTrace.size(); ++i) {
			const backward::ResolvedTrace trace = resolver.resolve(stackTrace[i]);

			ss << "#" << i << " at " << trace.object_function << "\n";
		}

		return ss.str();
	}


	E_RESULT_CODE CMemoryProfiler::RegisterBaseObject(const std::string& typeId, U32Ptr address, USIZE size)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		TracyAllocS(reinterpret_cast<const void*>(address), size, MAX_CALLSTACK_DEPTH);

		auto& entity = mLivingBaseObjectsTable[address];

		entity.mTypeIdStr = typeId;
		entity.mAddress = address;

#if TDE2_MEM_PROFILER_BASE_OBJECT_SAVE_STACKTRACE
		entity.mAllocationStacktrace = std::move(GetStackTrace());
#endif

		return RC_OK;
	}
		
	E_RESULT_CODE CMemoryProfiler::UnregisterBaseObject(U32Ptr address)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!address)
		{
			return RC_OK;
		}

		TracyFreeS(reinterpret_cast<const void*>(address), MAX_CALLSTACK_DEPTH);

		auto it = mLivingBaseObjectsTable.find(address);

		if (it == mLivingBaseObjectsTable.cend())
		{
			return RC_FAIL;
		}

		mLivingBaseObjectsTable.erase(it);

		return RC_OK;
	}

	E_RESULT_CODE CMemoryProfiler::RegisterCustomAllocation(const void* pPtr, USIZE size, const C8* name)
	{
		if (!pPtr || !size)
		{
			return RC_INVALID_ARGS;
		}

		std::lock_guard<std::mutex> lock(mMutex);

		if (mAllocationsStats.find(reinterpret_cast<uintptr_t>(pPtr)) != mAllocationsStats.cend())
		{
			return RC_FAIL;
		}

		if (name)
		{
			TracyAllocNS(pPtr, size, MAX_CALLSTACK_DEPTH, name);
		}
		else
		{
			TracyAllocS(pPtr, size, MAX_CALLSTACK_DEPTH);
		}

		mAllocationsStats[reinterpret_cast<U32Ptr>(pPtr)] = size;
		mAllocationsGroupStats[name] += size;

		return RC_OK;
	}
		
	E_RESULT_CODE CMemoryProfiler::UnregisterCustomAllocation(const void* pPtr, const C8* name)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!pPtr)
		{
			return RC_OK;
		}

		auto it = mAllocationsStats.find(reinterpret_cast<uintptr_t>(pPtr));
		if (it == mAllocationsStats.cend())
		{
			return RC_FAIL;
		}

		if (name)
		{
			TracyFreeNS(pPtr, MAX_CALLSTACK_DEPTH, name);
		}
		else
		{
			TracyFreeS(pPtr, MAX_CALLSTACK_DEPTH);
		}

		TDE2_ASSERT(mAllocationsStats.find(reinterpret_cast<U32Ptr>(pPtr)) != mAllocationsStats.cend());

		mAllocationsGroupStats[name] -= mAllocationsStats[reinterpret_cast<U32Ptr>(pPtr)];
		mAllocationsStats.erase(reinterpret_cast<U32Ptr>(pPtr));

		return RC_OK;
	}

	void CMemoryProfiler::DumpInfo()
	{
		LOG_MESSAGE(Wrench::StringUtils::Format("\n[Memory Profiler] Amount of leaked objects: {0}", mLivingBaseObjectsTable.size()));

		for (auto&& entity : mLivingBaseObjectsTable)
		{
#if TDE2_MEM_PROFILER_BASE_OBJECT_SAVE_STACKTRACE
			LOG_MESSAGE(Wrench::StringUtils::Format("[Memory Profiler] Object <{0}> at {1},\nstacktrace:{2}\n", entity.second.mTypeIdStr, entity.first, entity.second.mAllocationStacktrace));
#else
			LOG_MESSAGE(Wrench::StringUtils::Format("[Memory Profiler] Object <{0}> at {1}\n", entity.second.mTypeIdStr, entity.first));
#endif
		}
	}

	const CMemoryProfiler::TProfilerStatisticsData& CMemoryProfiler::GetStatistics() const
	{
		return mBlocksInfoRegistry;
	}

	USIZE CMemoryProfiler::GetTotalMemoryAvailable() const
	{
		return mTotalMemorySize;
	}

	U32 CMemoryProfiler::GetLiveObjectsCount() const
	{
		return static_cast<U32>(mLivingBaseObjectsTable.size());
	}

#if TDE2_EDITORS_ENABLED
	void CMemoryProfiler::OnBeforeMemoryRelease()
	{
	}
#endif

	TDE2_API TPtr<CMemoryProfiler> CMemoryProfiler::Get()
	{
		static TPtr<CMemoryProfiler> pInstance = TPtr<CMemoryProfiler>(new (std::nothrow) CMemoryProfiler());
		return pInstance;
	}
}

#endif