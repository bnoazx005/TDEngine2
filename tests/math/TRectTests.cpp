#include <catch2/catch.hpp>
#include <TDEngine2.h>
#include <vector>
#include <tuple>


using namespace TDEngine2;


TEST_CASE("TRect Tests")
{
	SECTION("TestContainsPoint_PassPoint_ReturnsCorrectResult")
	{
		TRectF32 rect { 0.0f, 0.0f, 100.0f, 100.0f }; /// a quad with sizes of 100

		std::vector<std::pair<TVector2, bool>> testSamples
		{
			{ { 10.0f, -10.0f }, true },
			{ { 0.0f, 0.0f }, false },
			{ { 100.0f, -100.0f }, false },
			{ { 10.0f, -105.0f }, false },
			{ { 99.5f, -99.5f }, true }
		};

		for (auto iter = testSamples.cbegin(); iter != testSamples.cend(); ++iter)
		{
			REQUIRE((*iter).second == ContainsPoint(rect, (*iter).first));
		}
	}

	SECTION("TestIsOverlapped_PassRects_ReturnsCorrectResult")
	{
		TRectF32 left { 0.0f, 0.0f, 100.0f, 100.0f };

		REQUIRE(IsOverlapped(left, { 50.0f, -40.0f, 100.0f, 100.0f }) == true);
		REQUIRE(IsOverlapped(left, { -10.0f, 10.0f, 100.0f, 100.0f }) == true);
		REQUIRE(IsOverlapped(left, { -10.0f, -10.0f, 100.0f, 100.0f }) == true);

		REQUIRE(IsOverlapped(left, { 200.0f, 10.0f, 100.0f, 100.0f }) == false);

		// self-overlapping should be possible
		REQUIRE(IsOverlapped(left, left) == true);
	}

	SECTION("TestPointToNormalized_PassRectAndOutsidePoint_ReturnsZeroVector2")
	{
		TRectF32 rect { 0.0f, 0.0f, 10.0f, 10.0f };

		TVector2 outsidePoint { 10.0f, 10.0f };

		REQUIRE(PointToNormalizedCoords(rect, outsidePoint) == ZeroVector2);
	}

	SECTION("TestPointToNormalized_PassRectAndInternalPoint_ReturnsNormalizedCoords")
	{
		TRectF32 rect { 0.0f, 0.0f, 10.0f, 10.0f };

		TVector2 internalPoint { 5.0f, -5.0f };

		REQUIRE(PointToNormalizedCoords(rect, internalPoint) == TVector2(0.5f, 0.5f));
	}
}