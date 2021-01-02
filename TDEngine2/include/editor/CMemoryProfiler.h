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
				U32 mOffset; // < An offset within a global chunk of memory
				U32 mCurrOccupiedSize = 0;
				U32 mTotalSize;
			} TGlobalMemoryBlockInfo, *TMemoryBlockInfoPtr;

		public:
			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

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

			/*!
				\brief The method creates an information block which is related with some memory block

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE RegisterGlobalMemoryBlock(const std::string& name, U32 offset, U32 size) override;

			/*!
				\brief The function is replacement of factory method for instances of this type.
				The only instance will be created per program's lifetime. To destroy it call Free
				as for any other type within the engine

				\return A pointer to an instance of IProfiler type
			*/

			TDE2_API static IMemoryProfiler* Get();
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CMemoryProfiler);
		private:
			std::unordered_map<std::string, TGlobalMemoryBlockInfo> mBlocksInfoRegistry;
	};


#define TDE2_REGISTER_MEMORY_BLOCK_PROFILE(Name, Offset, Size) CMemoryProfiler::Get()->RegisterGlobalMemoryBlock(Name, Offset, Size)
}

#endif

#define TDE2_DECLARE_MEMORY_BLOCK_PROFILE(Name, Offset, Size) 