/*!
	\file CMemoryProfiler.h
	\date 02.01.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "IProfiler.h"
#include "../core/CBaseObject.h"
#include <string>
#include <unordered_map>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{

	/*!
		class CMemoryProfiler

		\brief The interface describes functionality of built-in memory profiler system.
		Note that all profilers implementation should be singletons
	*/

	class CMemoryProfiler : public IMemoryProfiler, public CBaseObject
	{
		public:
			typedef struct TGlobalMemoryBlockInfo
			{
				USIZE mOffset; // < An offset within a global chunk of memory
				USIZE mCurrOccupiedSize = 0;
				USIZE mTotalSize;
			} TGlobalMemoryBlockInfo, *TMemoryBlockInfoPtr;

			typedef std::unordered_map<std::string, TGlobalMemoryBlockInfo> TProfilerStatisticsData;


			typedef struct TBaseObjectAllocInfo
			{
				std::string mTypeIdStr;
				std::string mAllocationStacktrace;
				U32Ptr mAddress;
			} TBaseObjectAllocInfo, *TBaseObjectAllocInfoPtr;

			typedef std::unordered_map<U32Ptr, TBaseObjectAllocInfo> TBaseObjectsRegistry;

		public:
			/*!
				\brief The method stars to record current frame's statistics. The method should be called only once per frame

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE BeginFrame() override;

			/*!
				\brief The method stops recording statistics. The method should be called only once per frame

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE EndFrame() override;

			TDE2_API E_RESULT_CODE SetTotalMemoryAvailable(USIZE size) override;

			/*!
				\brief The method creates an information block which is related with some memory block

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE RegisterGlobalMemoryBlock(const std::string& name, USIZE offset, USIZE size) override;

			/*!
				\brief The method updates information about memory block's occupation

				\param[in] name A name of the block
				\param[in] usedSize A value which tells how much the memory of the block is used

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE UpdateMemoryBlockInfo(const std::string& name, USIZE usedSize) override;

			TDE2_API E_RESULT_CODE RegisterBaseObject(const std::string& typeId, U32Ptr address) override;
			TDE2_API E_RESULT_CODE UnregisterBaseObject(U32Ptr address) override;

			TDE2_API void DumpInfo() override;

			/*!
				\brief The function is replacement of factory method for instances of this type.
				The only instance will be created per program's lifetime. 

				\return A pointer to an instance of IProfiler type
			*/

			TDE2_API static TPtr<CMemoryProfiler> Get();

			const TProfilerStatisticsData& GetStatistics() const;

			TDE2_API USIZE GetTotalMemoryAvailable() const override;

			TDE2_API U32 GetLiveObjectsCount() const override;

#if TDE2_EDITORS_ENABLED
			TDE2_API virtual void OnBeforeMemoryRelease();
#endif
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CMemoryProfiler);

		private:
			TProfilerStatisticsData mBlocksInfoRegistry;

			USIZE mTotalMemorySize;

			TBaseObjectsRegistry mLivingBaseObjectsTable;
	};


#define TDE2_REGISTER_MEMORY_BLOCK_PROFILE(Name, Offset, Size) CMemoryProfiler::Get()->RegisterGlobalMemoryBlock(Name, Offset, Size)
#define TDE2_UPDATE_MEMORY_BLOCK_INFO(Name, UsedSize) CMemoryProfiler::Get()->UpdateMemoryBlockInfo(Name, UsedSize)

}

#else

#define TDE2_DECLARE_MEMORY_BLOCK_PROFILE(Name, Offset, Size) 
#define TDE2_UPDATE_MEMORY_BLOCK_INFO(Name, UsedSize)

#endif
