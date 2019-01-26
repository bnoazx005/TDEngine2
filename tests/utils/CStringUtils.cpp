#include <catch2/catch.hpp>
#include <TDEngine2.h>
#include <string>


using namespace TDEngine2;


TEST_CASE("CStringUtils Tests")
{
	SECTION("RemoveSingleLineComments_PassEmptyStr_ReturnsEmptyStr")
	{
		REQUIRE(CStringUtils::RemoveSingleLineComments(CStringUtils::GetEmptyStr()) == CStringUtils::GetEmptyStr());
	}

	SECTION("RemoveSingleLineComments_PassEmptyAsCommentPrefix_ReturnsSourceStr")
	{
		std::string actualStr   = "Test string";
		std::string expectedStr = "Test string";

		REQUIRE(CStringUtils::RemoveSingleLineComments(actualStr, CStringUtils::GetEmptyStr()) == expectedStr);
	}

	SECTION("RemoveSingleLineComments_PassSourceWithComments_ReturnsSourceStrWithoutComments")
	{
		std::string actualStr = 
			"Test\n" \
			"//should be removed\n//second comment\n" \
			" string without comments";

		std::string expectedStr =
			"Test\n" \
			" string without comments";

		REQUIRE(CStringUtils::RemoveSingleLineComments(actualStr) == expectedStr);

		actualStr   = "Test\n//should be removed\n";
		expectedStr = "Test\n";

		REQUIRE(CStringUtils::RemoveSingleLineComments(actualStr) == expectedStr);
	}

	SECTION("RemoveMultiLineComments_PassSourceWithComments_ReturnsSourceStrWithoutComments")
	{
		std::string actualStr =
			"Test\n" \
			"/* mutli-line comment /* nested comment block */ end of a comment /* third block */ */" \
			"End of a test";

		std::string expectedStr =
			"Test\n" \
			"End of a test";
	
		REQUIRE(CStringUtils::RemoveMultiLineComments(actualStr) == expectedStr);
	}

	SECTION("RemoveMultiLineComments_PassSourceWithIncorrectComments_ReturnsSourceBeforeFirstComment")
	{
		std::string actualStr =
			"Test\n" \
			"/* mutli-line comment /* nested comment block" \
			"End of a test";

		std::string expectedStr =
			"Test\n";

		REQUIRE(CStringUtils::RemoveMultiLineComments(actualStr) == expectedStr);
	}

	SECTION("Split_PassStringWithDelimiters_ReturnsVectorOfStrings")
	{
		std::string str = "1 2 3 4 5";

		std::vector<std::string> expectedResult { "1", "2", "3", "4", "5" };

		std::vector<std::string> actualResult = CStringUtils::Split(str, " ");

		REQUIRE(expectedResult.size() == actualResult.size());

		for (U32 i = 0; i < expectedResult.size(); ++i)
		{
			REQUIRE(expectedResult[i] == actualResult[i]);
		}
	}

	SECTION("Split_PassEmptyString_ReturnsEmptyVector")
	{
		std::string str = CStringUtils::GetEmptyStr();
		
		std::vector<std::string> actualResult = CStringUtils::Split(str, " ");

		REQUIRE(0 == actualResult.size());
	}
}