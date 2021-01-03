#include "../../include/editor/CMemoryProfiler.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CMemoryProfiler::CMemoryProfiler() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CMemoryProfiler::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		--mRefCounter;

		if (!mRefCounter)
		{
			delete this;
			mIsInitialized = false;
		}

		return RC_OK;
	}

	E_RESULT_CODE CMemoryProfiler::BeginFrame()
	{
		return RC_OK;
	}

	E_RESULT_CODE CMemoryProfiler::EndFrame()
	{
		return RC_OK;
	}

	E_RESULT_CODE CMemoryProfiler::SetTotalMemoryAvailable(U32 size)
	{
		mTotalMemorySize = size;
		return RC_OK;
	}

	E_RESULT_CODE CMemoryProfiler::RegisterGlobalMemoryBlock(const std::string& name, U32 offset, U32 size)
	{
		auto it = mBlocksInfoRegistry.find(name);
		if (it != mBlocksInfoRegistry.cend())
		{
			return RC_FAIL;
		}

		mBlocksInfoRegistry.insert({ name, { offset, 0, size } });

		return RC_OK;
	}

	E_RESULT_CODE CMemoryProfiler::UpdateMemoryBlockInfo(const std::string& name, U32 usedSize)
	{
		auto it = mBlocksInfoRegistry.find(name);
		if ((it == mBlocksInfoRegistry.cend()) || (it->second.mTotalSize < usedSize))
		{
			return RC_FAIL;
		}

		(it->second).mCurrOccupiedSize = usedSize;

		return RC_OK;
	}

	const CMemoryProfiler::TProfilerStatisticsData& CMemoryProfiler::GetStatistics() const
	{
		return mBlocksInfoRegistry;
	}

	U32 CMemoryProfiler::GetTotalMemoryAvailable() const
	{
		return mTotalMemorySize;
	}


	TDE2_API CMemoryProfiler* CMemoryProfiler::Get()
	{
		static CMemoryProfiler* pInstance = new (std::nothrow) CMemoryProfiler();
		return pInstance;
	}
}

#endif