#include <catch2/catch.hpp>
#include <TDEngine2.h>


using namespace TDEngine2;


enum class E_TEST_FLAGS_ENUM: U32
{
	TFE_FIRST  = 0x1,
	TFE_SECOND = 0x2,
	TFE_THIRD  = 0x4,
	TFE_FOURTH = 0x8,
	TFE_FIFTH  = 0x10,
	TFE_SIXTH  = 0x20,
	TFE_ALL    = TFE_FIRST | TFE_SECOND | TFE_THIRD | TFE_FOURTH | TFE_FIFTH | TFE_SIXTH,
	TFE_EMPTY  = 0x0,
};

TDE2_DECLARE_BITMASK_OPERATORS(E_TEST_FLAGS_ENUM)


TEST_CASE("Utils Tests")
{
	SECTION("TestSwap2Bytes_PassValue_ReturnsValueWithSwappedOrderOfBytes")
	{
		U16 testValue     = 0x1234;
		U16 expectedValue = 0x3412;

		REQUIRE(Swap2Bytes(testValue) == expectedValue);
	}

	SECTION("TestSwap4Bytes_PassValue_ReturnsValueWithSwappedOrderOfBytes")
	{
		U32 testValue     = 0x12FFFF34;
		U32 expectedValue = 0x34FFFF12;

		REQUIRE(Swap4Bytes(testValue) == expectedValue);
	}

	SECTION("TestSwap8Bytes_PassValue_ReturnsValueWithSwappedOrderOfBytes")
	{
		U64 testValue     = 0x1234567887654321;
		U64 expectedValue = 0x2143658778563412;
		
		REQUIRE(Swap8Bytes(testValue) == expectedValue);
	}

	SECTION("TestScopedEnumDisjunctionOperator_PassCorrectValue_ReturnsTFE_ALL")
	{
		E_TEST_FLAGS_ENUM expectedResult = E_TEST_FLAGS_ENUM::TFE_ALL;

		E_TEST_FLAGS_ENUM currValue = E_TEST_FLAGS_ENUM::TFE_EMPTY;

		for (E_TEST_FLAGS_ENUM i = E_TEST_FLAGS_ENUM::TFE_FIRST; i <= E_TEST_FLAGS_ENUM::TFE_SIXTH; i = static_cast<E_TEST_FLAGS_ENUM>(static_cast<U32>(i) << 1))
		{
			currValue = currValue | i;
		}

		REQUIRE(currValue == expectedResult);
	}

	SECTION("TestScopedEnumConjunctionOperator_PassCorrectValue_ForAllValuesEqualsToTFE_EMPTY")
	{
		E_TEST_FLAGS_ENUM expectedResult = E_TEST_FLAGS_ENUM::TFE_EMPTY;

		/// iterate over all pairs of values the result all of them should equals to expectedResult
		for (E_TEST_FLAGS_ENUM left = E_TEST_FLAGS_ENUM::TFE_FIRST; left <= E_TEST_FLAGS_ENUM::TFE_SIXTH; left = static_cast<E_TEST_FLAGS_ENUM>(static_cast<U32>(left) << 1))
		{
			for (E_TEST_FLAGS_ENUM right = static_cast<E_TEST_FLAGS_ENUM>(static_cast<U32>(left) << 1); right <= E_TEST_FLAGS_ENUM::TFE_SIXTH; right = static_cast<E_TEST_FLAGS_ENUM>(static_cast<U32>(right) << 1))
			{
				REQUIRE((left & right) == expectedResult);
			}
		}
	}

	//SECTION("")
}


class CTestCounter : public CBaseObject
{
public:
	CTestCounter(bool& destroyed):
		CBaseObject()
	{
		mDestroyed = &destroyed;
	}

	~CTestCounter() { *mDestroyed = true; }
private:
	bool* mDestroyed = nullptr;
};


TEST_CASE("CScopedPtr Tests")
{
	SECTION("TestConstructor_CreateTempObjectAndDestroyIt_FlagShouldBeTrue")
	{
		bool hasBeenDestroyed = false;

		{
			CScopedPtr<CTestCounter> pPtr{ new CTestCounter(hasBeenDestroyed) };
			REQUIRE(pPtr->GetRefCount() == 1);
		}

		REQUIRE(hasBeenDestroyed);
	}

	SECTION("TestCopyConstructor_CreateTwoTempObjects_BothShouldBeDestoryed")
	{
		bool hasBeenDestroyed = false;

		{
			CScopedPtr<CTestCounter> pPtr{ new CTestCounter(hasBeenDestroyed) };

			{
				CScopedPtr<CTestCounter> pPtrCopy = pPtr;
				REQUIRE(pPtrCopy->GetRefCount() == 2);
			}

			REQUIRE(pPtr->GetRefCount() == 1);
			REQUIRE(!hasBeenDestroyed);
		}

		REQUIRE(hasBeenDestroyed);
	}

	SECTION("TestMoveConstructor_CreateTwoTempObjects_BothShouldBeDestoryed")
	{
		bool hasBeenDestroyed = false;

		{
			CScopedPtr<CTestCounter> pPtr{ new CTestCounter(hasBeenDestroyed) };

			{
				CScopedPtr<CTestCounter> pPtrCopy = std::move(pPtr);
				REQUIRE((!pPtr && (pPtrCopy->GetRefCount() == 1)));
			}

			REQUIRE(hasBeenDestroyed);
		}

		REQUIRE(hasBeenDestroyed);
	}

	SECTION("TestNestedPointersCase_CreateThreeObjects_AllShouldBeDestroyed")
	{
		bool hasBeenDestroyed = false;

		{
			CScopedPtr<CTestCounter> pPtr{ new CTestCounter(hasBeenDestroyed) };

			{
				CScopedPtr<CTestCounter> pPtrCopy = pPtr; // copy
				REQUIRE((pPtr && (pPtrCopy->GetRefCount() == 2)));

				{
					CScopedPtr<CTestCounter> pThirdPtr = std::move(pPtrCopy); // move
					REQUIRE(((pPtr && pPtr->GetRefCount() == 2) && !pPtrCopy && (pThirdPtr->GetRefCount() == 2)));
				}
			}

			REQUIRE(!hasBeenDestroyed);
		}

		REQUIRE(hasBeenDestroyed);
	}

	SECTION("TestDynamicCast_TryToConvertOnePointerToPtrOfBaseClass_CorrectlyCastsPointer")
	{
		bool hasBeenDestroyed = false;
		
		{
			CScopedPtr<CTestCounter> pPtr{ new CTestCounter(hasBeenDestroyed) };

			CScopedPtr<CBaseObject> pBasePtr = DynamicPtrCast<CBaseObject, CTestCounter>(pPtr);

			REQUIRE((pBasePtr && pPtr));
		}

		REQUIRE(hasBeenDestroyed);
	}
}