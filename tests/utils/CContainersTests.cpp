#include <catch2/catch_test_macros.hpp>
#include <TDEngine2.h>

using namespace TDEngine2;


TEST_CASE("CFixedVector<T, USIZE> Tests")
{
	constexpr USIZE EXPECTED_VECTOR_MAX_SIZE = 16;

	class CNonPODType
	{
		public:
			CNonPODType() = default;
			CNonPODType(const std::string& name): mName(name) {}

			bool operator== (const CNonPODType& other) const { return mName == other.mName; }

			const std::string& GetName() const { return mName; }
		private:
			std::string mName{};
	};

	SECTION("TestConstructor_PassCount_InitializesFixedVectorWithCountElements")
	{
		const USIZE expectedCount = 5;
		CFixedVector<CNonPODType, EXPECTED_VECTOR_MAX_SIZE> vec{ expectedCount };
		REQUIRE(vec.size() == expectedCount);
	}

	SECTION("TestConstructor_PassCountAndValue_InitializesFixedVectorWithCountElementsWithSpecifiedValue")
	{
		const CNonPODType expectedValue{ "Test" };
		const USIZE expectedCount = 5;

		CFixedVector<CNonPODType, EXPECTED_VECTOR_MAX_SIZE> vec{ expectedCount, expectedValue };

		REQUIRE(vec.size() == expectedCount);
		REQUIRE(std::all_of(vec.cbegin(), vec.cend(), [&expectedValue](const CNonPODType& actualValue) { return actualValue == expectedValue; }));
	}

	SECTION("TestConstructor_PassInitializerList_ElementsAreCorrectlyInitialized")
	{
		const USIZE expectedCount = 3;

		CFixedVector<CNonPODType, EXPECTED_VECTOR_MAX_SIZE> vec
		{ 
			{
				{ "First" }, { "Second" }, { "Third" }
			}
		};

		REQUIRE(vec.size() == expectedCount);
		REQUIRE((vec[0].GetName() == "First" && vec[1].GetName() == "Second" && vec[2].GetName() == "Third"));
	}

	SECTION("TestConstructor_PassCopyInstance_ElementsAreCorrectlyInitialized")
	{
		const USIZE expectedCount = 3;

		CFixedVector<CNonPODType, EXPECTED_VECTOR_MAX_SIZE> vec0
		{
			{
				{ "First" }, { "Second" }, { "Third" }
			}
		};

		CFixedVector<CNonPODType, EXPECTED_VECTOR_MAX_SIZE> vecCopy{ vec0 };

		REQUIRE(vecCopy.size() == expectedCount);
		REQUIRE((vecCopy[0].GetName() == "First" && vecCopy[1].GetName() == "Second" && vecCopy[2].GetName() == "Third"));
	}

	SECTION("TestAssignmentOperator_AssignNonEmptyVectorIntoEmptyOne_AllElementsCorrectlyCopied")
	{
		const USIZE expectedCount = 3;

		CFixedVector<CNonPODType, EXPECTED_VECTOR_MAX_SIZE> vec0
		{
			{
				{ "First" }, { "Second" }, { "Third" }
			}
		};

		CFixedVector<CNonPODType, EXPECTED_VECTOR_MAX_SIZE> vecCopy{};
		vecCopy = vec0;

		REQUIRE(vecCopy.size() == expectedCount);
		REQUIRE((vecCopy[0].GetName() == "First" && vecCopy[1].GetName() == "Second" && vecCopy[2].GetName() == "Third"));
	}

	SECTION("TestConstructor_MoveInstance_ElementsAreCorrectlyInitialized")
	{
		const USIZE expectedCount = 3;

		CFixedVector<CNonPODType, EXPECTED_VECTOR_MAX_SIZE> vec0
		{
			{
				{ "First" }, { "Second" }, { "Third" }
			}
		};

		CFixedVector<CNonPODType, EXPECTED_VECTOR_MAX_SIZE> vecCopy{ std::move(vec0) };

		REQUIRE(vecCopy.size() == expectedCount);
		REQUIRE((vecCopy[0].GetName() == "First" && vecCopy[1].GetName() == "Second" && vecCopy[2].GetName() == "Third"));
	}

	SECTION("TestMovementOperator_MoveNonEmptyVectorIntoEmptyOne_AllElementsCorrectlyMoved")
	{
		const USIZE expectedCount = 3;

		CFixedVector<CNonPODType, EXPECTED_VECTOR_MAX_SIZE> vec0
		{
			{
				{ "First" }, { "Second" }, { "Third" }
			}
		};

		CFixedVector<CNonPODType, EXPECTED_VECTOR_MAX_SIZE> vecCopy{};
		vecCopy = std::move(vec0);

		REQUIRE(vecCopy.size() == expectedCount);
		REQUIRE((vecCopy[0].GetName() == "First" && vecCopy[1].GetName() == "Second" && vecCopy[2].GetName() == "Third"));
	}

	SECTION("TestAt_TryAccessToValidIndex_ReturnsValue")
	{
		const I32 expectedValue = 4;
		CFixedVector<I32, EXPECTED_VECTOR_MAX_SIZE> vec{ 0, 2, expectedValue, 5 };
		REQUIRE(vec.at(2) == expectedValue);
	}

	SECTION("TestAt_TryAccessToValidIndexInConstVector_ReturnsValue")
	{
		const I32 expectedValue = 4;
		const CFixedVector<I32, EXPECTED_VECTOR_MAX_SIZE> vec{ 0, 2, expectedValue, 5 };
		REQUIRE(vec.at(2) == expectedValue);
	}

	SECTION("TestOperatorAt_TryAccessToValidIndex_ReturnsValue")
	{
		const I32 expectedValue = 4;
		CFixedVector<I32, EXPECTED_VECTOR_MAX_SIZE> vec{ 0, 2, expectedValue, 5 };
		REQUIRE(vec[2] == expectedValue);
	}

	SECTION("TestOperatorAt_TryAccessToValidIndexInConstVector_ReturnsValue")
	{
		const I32 expectedValue = 4;
		const CFixedVector<I32, EXPECTED_VECTOR_MAX_SIZE> vec{ 0, 2, expectedValue, 5 };
		REQUIRE(vec[2] == expectedValue);
	}

	SECTION("TestFront_TryGetFirstElement_ReturnsFirstElementsValue")
	{
		const I32 expectedValue = 4;
		CFixedVector<I32, EXPECTED_VECTOR_MAX_SIZE> vec{ expectedValue, 2, 3 };
		REQUIRE(vec.front() == expectedValue);
	}

	SECTION("TestFront_TryGetFirstElementInConstVector_ReturnsFirstElementsValue")
	{
		const I32 expectedValue = 4;
		const CFixedVector<I32, EXPECTED_VECTOR_MAX_SIZE> vec{ expectedValue, 2, 3 };
		REQUIRE(vec.front() == expectedValue);
	}

	SECTION("TestBack_TryGetLastElement_ReturnsLastElementsValue")
	{
		const I32 expectedValue = 4;
		CFixedVector<I32, EXPECTED_VECTOR_MAX_SIZE> vec{ 2, 3, expectedValue };
		REQUIRE(vec.back() == expectedValue);
	}

	SECTION("TestBack_TryGetLastElementInConstVector_ReturnsLastElementsValue")
	{
		const I32 expectedValue = 4;
		const CFixedVector<I32, EXPECTED_VECTOR_MAX_SIZE> vec{ 2, 3, expectedValue };
		REQUIRE(vec.back() == expectedValue);
	}

	SECTION("TestEmpty_ExecuteForEmptyVector_ReturnsTrue")
	{
		CFixedVector<I32, EXPECTED_VECTOR_MAX_SIZE> vec{};
		REQUIRE(vec.empty());
	}

	SECTION("TestEmpty_ExecuteForNonEmptyVector_ReturnsFalse")
	{
		CFixedVector<I32, EXPECTED_VECTOR_MAX_SIZE> vec{ 1, 2, 3 };
		REQUIRE(!vec.empty());
	}

	SECTION("TestSize_PushNewElements_SizeIsIncrementedEachPush")
	{
		CFixedVector<I32, EXPECTED_VECTOR_MAX_SIZE> vec{ };
		REQUIRE(vec.size() == 0);

		for (I32 i = 0; i < 10; ++i)
		{
			vec.push_back(i);
			REQUIRE(vec.size() == i + 1);
		}
	}

	SECTION("TestSize_InsertNewElements_SizeIsIncrementedEachInsert")
	{
		CFixedVector<I32, EXPECTED_VECTOR_MAX_SIZE> vec{ };
		REQUIRE(vec.size() == 0);

		for (I32 i = 0; i < 10; ++i)
		{
			vec.insert(vec.cbegin(), i);
			REQUIRE(vec.size() == i + 1);
		}
	}

	SECTION("TestSize_EmplaceNewElements_SizeIsIncrementedEachInsert")
	{
		CFixedVector<CNonPODType, EXPECTED_VECTOR_MAX_SIZE> vec{ };
		REQUIRE(vec.size() == 0);

		for (I32 i = 0; i < 10; ++i)
		{
			vec.emplace(vec.cbegin(), std::to_string(i));
			REQUIRE(vec.size() == i + 1);
		}
	}

	SECTION("TestSize_EraseElements_SizeIsDecrementedEachErasure")
	{
		USIZE expectedCount = 10;
		CFixedVector<CNonPODType, EXPECTED_VECTOR_MAX_SIZE> vec{ expectedCount };

		REQUIRE(vec.size() == expectedCount);

		while (!vec.empty())
		{
			vec.erase(vec.cbegin());
			REQUIRE(vec.size() == --expectedCount);
		}
	}
}