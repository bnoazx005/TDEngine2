#include <catch2/catch_test_macros.hpp>
#include <TDEngine2.h>


using namespace TDEngine2;


TEST_CASE("TPlane Tests")
{
	SECTION("TestCalcDistanceFromPlaneToPoint_PassPointsThatDontLieOnPlane_ReturnsDistanceToThem")
	{
		const TPlane<F32> xzPlane(UpVector3, 0.0f);

		const std::vector<std::pair<TVector3, F32>> testCases
		{
			{ UpVector3, 1.0f },
			{ TVector3(-25.0f, 42.0f, 0.0f), 42.0f},
			{ TVector3(5.0f), 5.0f },
			{ TVector3(-5.0f), -5.0f },
			{ -UpVector3, -1.0f },
		};

		for (auto&& currTestCase : testCases)
		{
			const F32 actualDistance = CalcDistanceFromPlaneToPoint(xzPlane, std::get<TVector3>(currTestCase));
			const F32 expectedDistance = std::get<F32>(currTestCase);

			REQUIRE((CMathUtils::Abs(actualDistance - expectedDistance) < 1e-3f));
		}
	}

	SECTION("TestCalcDistanceFromPlaneToPoint_TestDisplacedFromOriginPlane_ReturnsCorrectDistancesToPoints")
	{
		const F32 offsetFromOrigin = 10.0f;
		const TPlane<F32> xzPlane(UpVector3, offsetFromOrigin);

		const std::vector<std::pair<TVector3, F32>> testCases
		{
			{ UpVector3, 1.0f },
			{ TVector3(-25.0f, 42.0f, 0.0f), 42.0f },
			{ TVector3(5.0f), 5.0f },
			{ TVector3(-5.0f), -5.0f },
			{ -UpVector3, -1.0f },
		};

		for (auto&& currTestCase : testCases)
		{
			const F32 actualDistance = offsetFromOrigin + CalcDistanceFromPlaneToPoint(xzPlane, std::get<TVector3>(currTestCase));
			const F32 expectedDistance = std::get<F32>(currTestCase);

			REQUIRE((CMathUtils::Abs(actualDistance - expectedDistance) < 1e-3f));
		}
	}
}