#include <catch2/catch.hpp>
#include <TDEngine2.h>
#include <vector>


using namespace TDEngine2;


TEST_CASE("CResult<T, E> Tests")
{
	SECTION("TestConstructorOnPODType_PassCorrectInput_CreatesCorrectResultObject")
	{
		I32 expectedValue = rand();

		TResult<I32> intResult = TOkValue<I32>(expectedValue);

		REQUIRE(intResult.IsOk());
		REQUIRE(intResult.Get() == expectedValue);
	}

	SECTION("TestConstructorOnPointerType_PassCorrectInput_CreatesCorrectResultObject")
	{
		I32 expectedValue = rand();

		I32* pExpectedPtr = &expectedValue;

		TResult<I32*> intResult = TOkValue<I32*>(pExpectedPtr);

		REQUIRE(intResult.IsOk());
		REQUIRE(intResult.Get() == pExpectedPtr);
		REQUIRE(*intResult.Get() == expectedValue);
	}

	SECTION("TestConstructorOnSTLVector_PassCorrectInput_CreatesCorrectResultObject")
	{
		std::vector<I32> expectedResult;

		for (I32 i = 0; i < rand() % 10; ++i)
		{
			expectedResult.push_back(rand());
		}

		TResult<std::vector<I32>> vectorResult = TOkValue<std::vector<I32>>(expectedResult);

		REQUIRE(vectorResult.IsOk());
		REQUIRE(std::equal(expectedResult.cbegin(), expectedResult.cend(), vectorResult.Get().begin()));
	}

	SECTION("TestConstructorOnErrorValue_PassCorrectInput_CreateCorrectResultObjectContainingErrorCode")
	{
		E_RESULT_CODE expectedResult = RC_UNKNOWN;

		TResult<int> errorResult = TErrorValue<E_RESULT_CODE>(expectedResult);

		REQUIRE(errorResult.HasError());
		REQUIRE(errorResult.GetError() == expectedResult);
	}

	SECTION("TestConstructor_BothActualResultTypeAndErrorTypeAreSame_CreateCorrectResultObject")
	{
		E_RESULT_CODE expectedResult = RC_UNKNOWN;

		TResult<E_RESULT_CODE> errorResult = TOkValue<E_RESULT_CODE>(expectedResult);

		REQUIRE(errorResult.IsOk());
		REQUIRE(errorResult.Get() == expectedResult);
		
		errorResult = TErrorValue<E_RESULT_CODE>(expectedResult);

		REQUIRE(errorResult.HasError());
		REQUIRE(errorResult.GetError() == expectedResult);
	}

	SECTION("TestBoolConversionOperator_PassTOkValueObjectIntoIfStatement_TOkValueIsConvertedIntoTrue")
	{
		TResult<I32> result = TOkValue<I32>(42);
		REQUIRE(result);

		if (auto result = TResult<I32>(TOkValue<I32>(42))) { REQUIRE(true); } else { REQUIRE(false); }
	}

	SECTION("TestBoolConversionOperator_PassTErrValueObjectIntoIfStatement_TErrValueIsConvertedIntoFalse")
	{
		TResult<I32> result = TErrorValue<E_RESULT_CODE>(RC_FAIL);
		REQUIRE(!result);

		if (auto result = TResult<I32>(TErrorValue<E_RESULT_CODE>(RC_FAIL))) { REQUIRE(false); } else { REQUIRE(true); }
	}

	SECTION("TestCopyConstruction_PassReferenceToOtherResult_CreatesNewCopyBasedOnGivenObject")
	{
		TResult<std::vector<I32>> result = TOkValue<std::vector<I32>>({ 0, 1, 2, 3 });
		auto copyResult = result;

		;
	}
}