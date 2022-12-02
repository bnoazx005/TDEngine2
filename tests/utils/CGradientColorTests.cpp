#include <catch2/catch_test_macros.hpp>
#include <TDEngine2.h>
#include <vector>

using namespace TDEngine2;


TEST_CASE("CGradientColor Tests")
{
	E_RESULT_CODE result = RC_OK;
	CScopedPtr<CGradientColor> color{ CreateGradientColor(TColorUtils::mWhite, TColorUtils::mRed, result) };

	REQUIRE(RC_OK == result);

	SECTION("TestSample_PassMidPoint_ReturnsInterpolatedColor")
	{

	}
}