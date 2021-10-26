#include "../../include/editor/CMemoryProfiler.h"
#include "../../include/utils/CFileLogger.h"
#include "stringUtils.hpp"
#include "backward.hpp"
#include <sstream>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
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


	E_RESULT_CODE CMemoryProfiler::RegisterBaseObject(const std::string& typeId, U32Ptr address)
	{
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
		auto it = mLivingBaseObjectsTable.find(address);

		if (it == mLivingBaseObjectsTable.cend())
		{
			return RC_FAIL;
		}

		mLivingBaseObjectsTable.erase(it);

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