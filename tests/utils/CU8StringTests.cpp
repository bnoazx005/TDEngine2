#include <catch2/catch_test_macros.hpp>
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
		
		static constexpr USIZE expectedCodePointsCount = 3;

		REQUIRE(CU8String::Length(str.begin(), str.end()) == expectedCodePointsCount);

		const TUtf8CodePoint expectedCodePoints[3]
		{
			static_cast<TUtf8CodePoint>('8'),
			static_cast<TUtf8CodePoint>('!'),
			CU8String::StringToUTF8CodePoint(u8"\u2660"),
		};

		USIZE actualCodePointsCount = 0;

		TUtf8CodePoint currCP;

		std::string::iterator it = str.begin();

		while (CU8String::MoveNext(it, str.end(), currCP))
		{
			REQUIRE(currCP == expectedCodePoints[actualCodePointsCount++]);
		}

		REQUIRE(actualCodePointsCount == expectedCodePointsCount);
	}

	SECTION("TestStringToUTF8CodePoint_ConvertUtf8StringToCodePointTypeAndBack_InitialStringAndConvertedShouldBeSame")
	{
		std::vector<std::string> testCases
		{
			u8"\u2660",
			" ",
			u8"😀"
		};

		for (auto&& currString : testCases)
		{
			auto expectedResult = currString;

			auto codePoint = CU8String::StringToUTF8CodePoint(std::move(currString));
			auto actualResult = CU8String::UTF8CodePointToString(codePoint);

			REQUIRE(actualResult == expectedResult);
		}
	}

	SECTION("TestEraseAt_PassStringWithSomeUtf8codePoint_ReturnsNewStringWithRemovedCodePoint")
	{
		std::vector<std::tuple<std::string, USIZE, std::string>> testCases
		{
			{ "", 0, ""},
			{ u8"\u2660", 0, u8"" },
			{ u8"Hello!", 2, u8"Helo!" },
			{ u8"Hello😀!", 5, u8"Hello!" },
			{ u8"Hello!😀", 6, u8"Hello!" },
			{ u8"😀Hello!", 0, u8"Hello!" },
		};

		std::string str, expectedResult;
		USIZE codePointIndex;

		for (auto&& currTest : testCases)
		{
			std::tie(str, codePointIndex, expectedResult) = currTest;

			REQUIRE(CU8String::EraseAt(str, codePointIndex) == expectedResult);
		}
	}

	SECTION("TestInsertAt_AddSomeCodePointToCurrentString_ReturnsNewStringWithNewCodePointThere")
	{
		std::vector<std::tuple<std::string, TUtf8CodePoint, USIZE, std::string>> testCases
		{
			{ "", CU8String::StringToUTF8CodePoint(u8"😀"), 0, u8"😀" },
			{ "", CU8String::StringToUTF8CodePoint(u8"😀"), 4, "" }, /// If position goes out of range nothing happens and original string is returned
			{ u8"Hello!", CU8String::StringToUTF8CodePoint(u8"😀"), 2, u8"He😀llo!" },
		};

		std::string str, expectedResult;
		TUtf8CodePoint codePoint;
		USIZE codePointIndex;

		for (auto&& currTest : testCases)
		{
			std::tie(str, codePoint, codePointIndex, expectedResult) = currTest;

			auto&& actualResult = CU8String::InsertAt(str, codePointIndex, codePoint);
			REQUIRE(actualResult == expectedResult);
		}
	}
}