#include <catch2/catch_test_macros.hpp>
#include <TDEngine2.h>
#include <string>


using namespace TDEngine2;


TEST_CASE("CStringUtils Tests")
{
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
}