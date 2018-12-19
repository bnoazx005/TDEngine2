#include <catch2/catch.hpp>
#include <TDEngine2.h>


using namespace TDEngine2;


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
}