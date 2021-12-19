#include <catch2/catch.hpp>
#include <TDEngine2.h>


using namespace TDEngine2;


TEST_CASE("CLinearAllocator Tests")
{
	E_RESULT_CODE result = RC_OK;

	const USIZE totalMemorySize = 1024;

	SECTION("TestInitialState_CreateNewInstance_UsedSizeEqualsToZero")
	{
		TPtr<IAllocator> pAllocator = TPtr<IAllocator>(CreateLinearAllocator(totalMemorySize, result));
		REQUIRE((RC_OK == result && pAllocator && !pAllocator->GetUsedMemorySize() && pAllocator->GetTotalMemorySize() == totalMemorySize));
	}

	SECTION("TestAllocate_MakeSingleBigAllocation_ReturnsNullptrAndAsserts")
	{
		TPtr<IAllocator> pAllocator = TPtr<IAllocator>(CreateLinearAllocator(totalMemorySize, result));
		REQUIRE((RC_OK == result && pAllocator && !pAllocator->GetUsedMemorySize() && pAllocator->GetTotalMemorySize() == totalMemorySize));

		REQUIRE(!pAllocator->Allocate(totalMemorySize * 2, __alignof(USIZE)));
	}

	SECTION("TestAllocate_AllocateSmallBlocksOfSameSize_UsedMemorySizeEqualsToBlockSizeTimesAllocationsCount")
	{
		const USIZE blockSize = 128;
		const U32 allocationsCount = 10;

		TPtr<IAllocator> pAllocator = TPtr<IAllocator>(CreateLinearAllocator(totalMemorySize, result));

		for (U32 i = 0; i < allocationsCount; ++i)
		{
			REQUIRE(pAllocator->Allocate(blockSize, 4));
		}

		REQUIRE(pAllocator->GetAllocationsCount() == allocationsCount);
		REQUIRE(pAllocator->GetUsedMemorySize() == blockSize * allocationsCount);
	}

	SECTION("TestClear_ClearAllAllocatedData_UniqueSizeShouldBeZeroTotalSizeSavesItsValue")
	{
		const USIZE blockSize = 128;
		const U32 allocationsCount = 10;

		TPtr<IAllocator> pAllocator = TPtr<IAllocator>(CreateLinearAllocator(totalMemorySize, result));
		REQUIRE((RC_OK == result && pAllocator && !pAllocator->GetUsedMemorySize() && pAllocator->GetTotalMemorySize() == totalMemorySize));

		for (U32 i = 0; i < allocationsCount; ++i)
		{
			REQUIRE(pAllocator->Allocate(blockSize, 4));
		}

		REQUIRE(RC_OK == pAllocator->Clear());
		REQUIRE((pAllocator->GetUsedMemorySize() == 0 && pAllocator->GetTotalMemorySize() == 2 * totalMemorySize));
	}
}


TEST_CASE("CStackAllocator's Test")
{
	E_RESULT_CODE result = RC_OK;

	const USIZE totalMemorySize = 1024;

	SECTION("TestInitialState_CreateNewInstance_UsedSizeEqualsToZero")
	{
		TPtr<IAllocator> pAllocator = TPtr<IAllocator>(CreateStackAllocator(totalMemorySize, result));
		REQUIRE((RC_OK == result && pAllocator && !pAllocator->GetUsedMemorySize() && pAllocator->GetTotalMemorySize() == totalMemorySize));
	}

	SECTION("TestAllocate_MakeSingleBigAllocation_ReturnsNullptrAndAsserts")
	{
		TPtr<IAllocator> pAllocator = TPtr<IAllocator>(CreateStackAllocator(totalMemorySize, result));
		REQUIRE(!pAllocator->Allocate(totalMemorySize * 2, __alignof(USIZE)));
	}

	SECTION("TestAllocate_AllocateSmallBlocksOfSameSize_UsedMemorySizeEqualsToBlockSizeTimesAllocationsCount")
	{
		const USIZE blockSize = 128;
		const U32 allocationsCount = 10;

		TPtr<IAllocator> pAllocator = TPtr<IAllocator>(CreateStackAllocator(totalMemorySize, result));

		for (U32 i = 0; i < allocationsCount; ++i)
		{
			REQUIRE(pAllocator->Allocate(blockSize, 4));
		}

		REQUIRE(pAllocator->GetAllocationsCount() == allocationsCount);
		REQUIRE(pAllocator->GetUsedMemorySize() >= blockSize * allocationsCount); // will be greater because of headers overhead
	}

	SECTION("TestClear_ClearAllAllocatedData_ClearCannotBeExecutedForStackAllocator")
	{
		TPtr<IAllocator> pAllocator = TPtr<IAllocator>(CreateStackAllocator(totalMemorySize, result));
		REQUIRE(RC_FAIL == pAllocator->Clear());
	}

	SECTION("TestDeallocate_DeallocateAllAllocatedBlocks_CorrectlyDoesIt")
	{
		const USIZE blockSize = 128;
		const U32 allocationsCount = 10;

		TPtr<IAllocator> pAllocator = TPtr<IAllocator>(CreateStackAllocator(totalMemorySize, result));

		std::array<void*, allocationsCount> allocations;

		for (U32 i = 0; i < allocationsCount; ++i)
		{
			REQUIRE((allocations[i] = pAllocator->Allocate(blockSize, 4)));
		}

		for (U32 i = 0; i < allocationsCount; ++i)
		{
			REQUIRE(RC_OK == pAllocator->Deallocate(allocations[i]));
		}

		REQUIRE(pAllocator->GetAllocationsCount() == 0);
		REQUIRE(pAllocator->GetUsedMemorySize() >= 0); 
	}
}


TEST_CASE("CPoolAllocator Test")
{
	E_RESULT_CODE result = RC_OK;

	const USIZE totalMemorySize = 1024;

	struct TestObject
	{
		U32 mField[4];
	};

	SECTION("TestInitialState_CreateNewInstance_UsedSizeEqualsToZero")
	{
		TPtr<IAllocator> pAllocator = TPtr<IAllocator>(CreatePoolAllocator(sizeof(TestObject), alignof(TestObject), totalMemorySize, result));
		REQUIRE((RC_OK == result && pAllocator && !pAllocator->GetUsedMemorySize() && pAllocator->GetTotalMemorySize() == totalMemorySize));
	}

	SECTION("TestAllocate_MakeSingleBigAllocation_ReturnsNullptrAndAsserts")
	{
		TPtr<IAllocator> pAllocator = TPtr<IAllocator>(CreatePoolAllocator(sizeof(TestObject), alignof(TestObject), totalMemorySize, result));
		REQUIRE(!pAllocator->Allocate(totalMemorySize * 2, __alignof(USIZE)));
		REQUIRE(!pAllocator->Allocate(sizeof(TestObject) + 4, __alignof(USIZE)));
	}

	SECTION("TestAllocate_AllocateChunks_UsedMemorySizeEqualsToBlockSizeTimesAllocationsCount")
	{
		const U32 allocationsCount = 10;

		TPtr<IAllocator> pAllocator = TPtr<IAllocator>(CreatePoolAllocator(sizeof(TestObject), alignof(TestObject), totalMemorySize, result));

		for (U32 i = 0; i < allocationsCount; ++i)
		{
			REQUIRE(pAllocator->Allocate(sizeof(TestObject), alignof(TestObject)));
		}

		REQUIRE(pAllocator->GetAllocationsCount() == allocationsCount);
		REQUIRE(pAllocator->GetUsedMemorySize() == sizeof(TestObject) * allocationsCount); 
	}

	SECTION("TestClear_ClearAllAllocatedData_UniqueSizeShouldBeZeroTotalSizeSavesItsValue")
	{
		const USIZE blockSize = 128;
		const U32 allocationsCount = 10;

		TPtr<IAllocator> pAllocator = TPtr<IAllocator>(CreatePoolAllocator(sizeof(TestObject), alignof(TestObject), totalMemorySize, result));
		REQUIRE((RC_OK == result && pAllocator && !pAllocator->GetUsedMemorySize() && pAllocator->GetTotalMemorySize() == totalMemorySize));

		for (U32 i = 0; i < allocationsCount; ++i)
		{
			REQUIRE(pAllocator->Allocate(sizeof(TestObject), alignof(TestObject)));
		}

		REQUIRE(RC_OK == pAllocator->Clear());
		
		const USIZE expectedSize = (sizeof(TestObject) * allocationsCount / totalMemorySize + 1) * totalMemorySize;

		REQUIRE((pAllocator->GetUsedMemorySize() == 0 && pAllocator->GetTotalMemorySize() == expectedSize));
	}
}