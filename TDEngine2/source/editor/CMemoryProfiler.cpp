#include "../../include/editor/CMemoryProfiler.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CMemoryProfiler::CMemoryProfiler() :
		CBaseObject(), mLivingObjectsCount(0)
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

	E_RESULT_CODE CMemoryProfiler::RegisterBaseObject()
	{
		++mLivingObjectsCount;
		return RC_OK;
	}
		
	E_RESULT_CODE CMemoryProfiler::UnregisterBaseObject()
	{
		if (!mLivingObjectsCount)
		{
			return RC_FAIL;
		}

		--mLivingObjectsCount;
		return RC_OK;
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
		return mLivingObjectsCount;
	}

	E_RESULT_CODE CMemoryProfiler::_onFreeInternal()
	{
		return RC_OK;
	}


	TDE2_API TPtr<CMemoryProfiler> CMemoryProfiler::Get()
	{
		static TPtr<CMemoryProfiler> pInstance = TPtr<CMemoryProfiler>(new (std::nothrow) CMemoryProfiler());
		return pInstance;
	}
}

#endif