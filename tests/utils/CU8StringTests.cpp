#include <catch2/catch.hpp>
#include <TDEngine2.h>
#include <string>


using namespace TDEngine2;


TEST_CASE("CU8String Tests")
{
	SECTION("TestLength_PassCorrectData_ReturnsTheirLengthsInCodePoints")
	{
		std::vector<std::tuple<std::string, USIZE>> testCases 
		{
			{ "", 0 },
			{ u8"\u2660", 1 },
			{ u8"\u2660", 1 },
			{ u8"\u2660+", 2 },
			{ u8"\u2660", 1 },
			{ u8"Hello!", 6 },
			{ u8"Hello😀!", 7 },
		};

		std::string str;
		USIZE expectedLength;

		for (auto&& currTest : testCases)
		{
			std::tie(str, expectedLength) = currTest;

			REQUIRE(CU8String::Length(str.begin(), str.end()) == expectedLength);
		}
	}

	SECTION("TestAt_IterateOverString_ReturnCorrectCodePoints")
	{
		std::string str(u8"8!\u2660");
		
		REQUIRE(CU8String::Length(str.begin(), str.end()) == 3);

		const TUtf8CodePoint expectedCodePoints[3]
		{
			static_cast<TUtf8CodePoint>('8'),
			static_cast<TUtf8CodePoint>('!'),
			static_cast<TUtf8CodePoint>('\u2660'),
		};

		USIZE actualCodePointsCount = 0;

		TUtf8CodePoint currCP;

		std::string::iterator it = str.begin();

		while (CU8String::MoveNext(it, str.end(), currCP))
		{
			REQUIRE(currCP == expectedCodePoints[static_cast<U32>(std::distance(str.begin(), it) - 1)]);
		}
	}

}