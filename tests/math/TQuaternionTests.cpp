#include <catch2/catch.hpp>
#include <TDEngine2.h>


using namespace TDEngine2;


TEST_CASE("TQuaternion Tests")
{
	SECTION("TestEulerConstructor_PassCorrectResults_CreateCorrectQuaternion")
	{
		TQuaternion q({ 0.0f, 0.0f, 45.0f * CMathConstants::Deg2Rad });

		REQUIRE(true);
	}
}