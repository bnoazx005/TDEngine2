#include <catch2/catch_test_macros.hpp>
#include <TDEngine2.h>
#include <vector>
#include <tuple>


using namespace TDEngine2;


TEST_CASE("TRay Tests")
{
	SECTION("TestCalcShortestDistanceBetweenLines_PassParallelLines_ReturnsCorrectDistanceBetweenThem")
	{
		std::tuple<TRay3D, TRay3D, F32> testCases[]
		{
			{ { ZeroVector3, UpVector3 }, { { 2.0, 0.0f, 0.0f}, UpVector3 }, 2.0f },
			{ { ZeroVector3, RightVector3 }, { { 0.0, 2.0f, 0.0f}, RightVector3 }, 2.0f },
			{ { ZeroVector3, ForwardVector3 }, { { 0.0, 2.0f, 0.0f}, ForwardVector3 }, 2.0f },
		};

		for (auto&& currTestCase : testCases)
		{
			REQUIRE(std::abs(std::get<0>(CalcShortestDistanceBetweenLines(std::get<0>(currTestCase), std::get<1>(currTestCase))) - std::get<F32>(currTestCase)) < 1e-2f);
		}
	}

	SECTION("TestCalcShortestDistanceBetweenLines_PassOrthogonalRays_ReturnsDistanceBetweenThem")
	{
		std::tuple<TRay3D, TRay3D, F32> testCases[]
		{
			{ { ZeroVector3, ForwardVector3 }, { { 2.0, 0.0f, 0.0f}, UpVector3 }, 2.0f },
			{ { ZeroVector3, RightVector3 }, { { 0.0, 0.0f, -5.0f}, UpVector3 }, 5.0f },
		};

		for (auto&& currTestCase : testCases)
		{
			REQUIRE(std::abs(std::get<0>(CalcShortestDistanceBetweenLines(std::get<0>(currTestCase), std::get<1>(currTestCase))) - std::get<F32>(currTestCase)) < 1e-2f);
		}
	}

	SECTION("TestCalcShortestDistanceBetweenLines_PassCommonCaseRays_ReturnsDistanceBetweenThem")
	{
		std::tuple<TRay3D, TRay3D, F32> testCases[]
		{
			{ { { -1.0f, 2.0f, 0.0f }, { 2.0f, 3.0f, 1.0f } }, { { 3.0, -4.0f, 1.0f}, { 1.0f, 2.0f, 1.0f } }, 6.35085f },
		};

		for (auto&& currTestCase : testCases)
		{
			REQUIRE(std::abs(std::get<0>(CalcShortestDistanceBetweenLines(std::get<0>(currTestCase), std::get<1>(currTestCase))) - std::get<F32>(currTestCase)) < 1e-2f);
		}
	}

	SECTION("TestCalcDistanceBetweenRayAndPoint_PassCommonCase_ReturnsDistanceBetweenRayAndPoint")
	{
		std::tuple<TRay3D, TVector3, F32> testCases[]
		{
			{ { ZeroVector3, RightVector3 },  { 0.0f, 4.0f, 0.0f }, 4.0f },
			{ { ZeroVector3, RightVector3 },  { 0.0f, 4.0f, 0.0f }, 4.0f },
			{ { ZeroVector3, RightVector3 },  { 0.0f, 0.0f, 4.0f }, 4.0f },
		};

		for (auto&& currTestCase : testCases)
		{
			REQUIRE(std::abs(CalcDistanceBetweenRayAndPoint(std::get<TRay3D>(currTestCase), std::get<TVector3>(currTestCase)) - std::get<F32>(currTestCase)) < 1e-2f);
		}
	}

	SECTION("TestCalcDistanceBetweenRayAndPoint_PassPointThatLiesOnLine_ReturnsZero")
	{
		std::tuple<TRay3D, TVector3, F32> testCases[]
		{
			{ { ZeroVector3, RightVector3 },  { 4.0f, 0.0f, 0.0f }, 0.0f },
		};

		for (auto&& currTestCase : testCases)
		{
			REQUIRE(std::abs(CalcDistanceBetweenRayAndPoint(std::get<TRay3D>(currTestCase), std::get<TVector3>(currTestCase)) - std::get<F32>(currTestCase)) < 1e-2f);
		}
	}

	SECTION("TestSampleOperator_PassParameters_ReturnsValidPoints")
	{
		std::tuple<TRay3D, F32, TVector3> testCases[]
		{
			{ { ZeroVector3, RightVector3 }, 4.0f, { 4.0f, 0.0f, 0.0f } },
			{ { ZeroVector3, 4.0f * RightVector3 }, 4.0f, { 4.0f, 0.0f, 0.0f } },
		};

		for (auto&& currTestCase : testCases)
		{
			REQUIRE(std::get<TRay3D>(currTestCase)(std::get<1>(currTestCase)) == std::get<TVector3>(currTestCase));
		}
	}

	SECTION("TestCheckRayPlaneIntersection_PassParallelRayForPlane_ReturnsFalseIfRayDoesntLieOnPlane")
	{
		std::tuple<TRay3D, TPlane<F32>, bool> testCases[]
		{
			{ { ZeroVector3, RightVector3 }, { UpVector3, 0.0f }, true },
			{ { UpVector3, RightVector3 }, { UpVector3, 0.0f }, false },
		};

		for (auto&& currTestCase : testCases)
		{
			REQUIRE(std::get<bool>(CheckRayPlaneIntersection(std::get<TRay3D>(currTestCase), std::get<TPlane<F32>>(currTestCase))) == std::get<bool>(currTestCase));
		}
	}
}