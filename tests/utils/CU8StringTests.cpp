#include <catch2/catch.hpp>
#include <TDEngine2.h>
#include <string>


using namespace TDEngine2;


TEST_CASE("CU8String Tests")
{
	SECTION("TestConstructor_PassCorrectData_CreatesInstances")
	{
		REQUIRE(CU8String().Length() == 0);
		REQUIRE(CU8String(u8"\u2660").Length() == 1);
		REQUIRE(CU8String(std::string(u8"\u2660")).Length() == 1);
		REQUIRE(CU8String(std::string(u8"\u2660+")).Length() == 2);
		REQUIRE(CU8String(CU8String(u8"\u2660")).Length() == 1);
		REQUIRE(CU8String(u8"Hello!").Length() == 6);
	}

	SECTION("TestAt_IterateOverString_ReturnCorrectCodePoints")
	{
		CU8String str(u8"8!\u2660");
		
		REQUIRE(str.Length() == 3);

		U8C ch;

		for (U32 i = 0; i < str.Length() + 1; ++i)
		{
			ch = str.At(i);
		}
	}

	SECTION("TestAt_TryToGetChatAtIncorrectIndex_ReturnZero")
	{
		CU8String str(u8"\u2660");

		REQUIRE(str.Length() == 1);
		REQUIRE(str.At(0) == StringToUTF8Char(u8"\u2660"));
	}
}