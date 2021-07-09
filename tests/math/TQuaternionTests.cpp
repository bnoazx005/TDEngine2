#include <catch2/catch.hpp>
#include <TDEngine2.h>


using namespace TDEngine2;


TEST_CASE("TQuaternion Tests")
{
	SECTION("TestEulerConstructor_PassCorrectResults_CreateCorrectQuaternion")
	{
		constexpr F32 testAngle = 45.0f * CMathConstants::Deg2Rad;

		/// first is an expected, second is actual result
		std::tuple<TQuaternion, TQuaternion> testCases[] 
		{
			{ TQuaternion { 0.3826f, 0.0f, 0.0f, 0.9238f }, TQuaternion { TVector3 { testAngle, 0.0f, 0.0f } } },
			{ TQuaternion { 0.0f, 0.3826f, 0.0f, 0.9238f }, TQuaternion { TVector3 { 0.0f, testAngle, 0.0f } } },
			{ TQuaternion { 0.0f, 0.0f, 0.3826f, 0.9238f }, TQuaternion { TVector3 { 0.0f, 0.0f, testAngle } } },
		};

		for (auto&& currCase : testCases)
		{
			REQUIRE(std::get<0>(currCase) == std::get<1>(currCase));
		}
	}
}