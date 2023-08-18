#include <catch2/catch_test_macros.hpp>
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
			{ { 10.0f, 10.0f }, true },
			{ { 0.0f, 0.0f }, false },
			{ { 100.0f, -100.0f }, false },
			{ { 10.0f, -105.0f }, false },
			{ { 99.5f, 99.5f }, true }
		};

		for (auto iter = testSamples.cbegin(); iter != testSamples.cend(); ++iter)
		{
			REQUIRE((*iter).second == ContainsPoint(rect, (*iter).first));
		}
	}

	SECTION("TestPointToNormalized_PassRectAndOutsidePoint_ReturnsClampedVector")
	{
		TRectF32 rect { 0.0f, 0.0f, 10.0f, 10.0f };

		TVector2 outsidePoint { 10.0f, 10.0f };

		REQUIRE(PointToNormalizedCoords(rect, outsidePoint) == TVector2(1.0f));
	}

	SECTION("TestPointToNormalized_PassRectAndInternalPoint_ReturnsNormalizedCoords")
	{
		TRectF32 rect { 0.0f, 0.0f, 10.0f, 10.0f };

		TVector2 internalPoint { 5.0f, 5.0f };

		REQUIRE(PointToNormalizedCoords(rect, internalPoint) == TVector2(0.5f, 0.5f));
	}

	SECTION("TestSplitRectWithLine_SplitVerticaly_ReturnsCorrectlySplittedRectangles")
	{
		TRectI32 sourceRect{ 0, 0, 4096, 4096 };

		TRectI32 expectedRects[] = 
		{
			{ 0, 0, 1024, 4096 },
			{ 1024, 0, 3072, 4096 }
		};

		TRectI32 firstRect, secondRect;

		std::tie(firstRect, secondRect) = SplitRectWithLine(sourceRect, TVector2(1024, 0), true);

		REQUIRE(firstRect == expectedRects[0]);
		REQUIRE(secondRect == expectedRects[1]);
	}

	SECTION("TestSplitRectWithLine_SplitHorizontally_ReturnsCorrectlySplittedRectangles")
	{
		TRectI32 sourceRect{ 0, 0, 4096, 4096 };

		TRectI32 expectedRects[] =
		{
			{ 0, 0, 4096, 1024 },
			{ 0, 1024, 4096, 3072 }
		};

		TRectI32 firstRect, secondRect;

		std::tie(firstRect, secondRect) = SplitRectWithLine(sourceRect, TVector2(0, 1024), false);

		REQUIRE(firstRect == expectedRects[0]);
		REQUIRE(secondRect == expectedRects[1]);
	}

	SECTION("TestIsOverlapped_PassCommonCasesOfIntersections_ReturnsTrue")
	{
		const std::vector<std::tuple<TRectF32, TRectF32>> testCases
		{
			{ TRectF32(0.0f, 0.0f, 100.0f, 100.0f), TRectF32(20.0f, 20.0f, 100.0f, 100.0f) },
			{ TRectF32(0.0f, 0.0f, 100.0f, 100.0f), TRectF32(-20.0f, 20.0f, 100.0f, 100.0f) },
			{ TRectF32(0.0f, 0.0f, 100.0f, 100.0f), TRectF32(-80.0f, -20.0f, 100.0f, 100.0f) },
			{ TRectF32(0.0f, 0.0f, 100.0f, 100.0f), TRectF32(20.0f, -20.0f, 100.0f, 100.0f) },

			// same as first four but rects are swapped
			{ TRectF32(20.0f, 20.0f, 100.0f, 100.0f), TRectF32(0.0f, 0.0f, 100.0f, 100.0f) },
			{ TRectF32(-20.0f, 20.0f, 100.0f, 100.0f), TRectF32(0.0f, 0.0f, 100.0f, 100.0f) },
			{ TRectF32(-80.0f, -20.0f, 100.0f, 100.0f), TRectF32(0.0f, 0.0f, 100.0f, 100.0f) },
			{ TRectF32(20.0f, -20.0f, 100.0f, 100.0f), TRectF32(0.0f, 0.0f, 100.0f, 100.0f) },

			// self-overlapping should be allowed too
			{ TRectF32(0.0f, 0.0f, 100.0f, 100.0f), TRectF32(0.0f, 0.0f, 100.0f, 100.0f) },
		};

		for (auto&& currTestCase : testCases)
		{
			REQUIRE(IsOverlapped(std::get<0>(currTestCase), std::get<1>(currTestCase)));
		}
	}

	SECTION("TestIsOverlapped_PassNonintersectingRectangles_ReturnsFalse")
	{
		const std::vector<std::tuple<TRectF32, TRectF32>> testCases
		{
			{ TRectF32(0.0f, 0.0f, 100.0f, 100.0f), TRectF32(120.0f, 120.0f, 100.0f, 100.0f) },
			{ TRectF32(0.0f, 0.0f, 100.0f, 100.0f), TRectF32(-101.0f, 0.0f, 100.0f, 100.0f) },
		};

		for (auto&& currTestCase : testCases)
		{
			REQUIRE(!IsOverlapped(std::get<0>(currTestCase), std::get<1>(currTestCase)));
		}
	}

	SECTION("TestIntersectRects_PassCommonCasesOfIntersections_ReturnsUnionOfTwoRects")
	{
		const std::vector<std::tuple<TRectF32, TRectF32, TRectF32>> testCases
		{
			{ TRectF32(0.0f, 0.0f, 100.0f, 100.0f), TRectF32(20.0f, 20.0f, 100.0f, 100.0f), TRectF32(20.0f, 20.0f, 80.0f, 80.0f) },
			{ TRectF32(0.0f, 0.0f, 100.0f, 100.0f), TRectF32(-20.0f, 20.0f, 100.0f, 100.0f), TRectF32(0.0f, 20.0f, 80.0f, 80.0f) },
			{ TRectF32(0.0f, 0.0f, 100.0f, 100.0f), TRectF32(-80.0f, -20.0f, 100.0f, 100.0f), TRectF32(0.0f, 0.0f, 20.0f, 80.0f) },
			{ TRectF32(0.0f, 0.0f, 100.0f, 100.0f), TRectF32(20.0f, -20.0f, 100.0f, 100.0f), TRectF32(20.0f, 0.0f, 80.0f, 80.0f) },

			// same as first four but rects positions are swapped
			{ TRectF32(20.0f, 20.0f, 100.0f, 100.0f), TRectF32(0.0f, 0.0f, 100.0f, 100.0f), TRectF32(20.0f, 20.0f, 80.0f, 80.0f) },
			{ TRectF32(-20.0f, 20.0f, 100.0f, 100.0f), TRectF32(0.0f, 0.0f, 100.0f, 100.0f), TRectF32(0.0f, 20.0f, 80.0f, 80.0f) },
			{ TRectF32(-80.0f, -20.0f, 100.0f, 100.0f), TRectF32(0.0f, 0.0f, 100.0f, 100.0f), TRectF32(0.0f, 0.0f, 20.0f, 80.0f) },
			{ TRectF32(20.0f, -20.0f, 100.0f, 100.0f), TRectF32(0.0f, 0.0f, 100.0f, 100.0f), TRectF32(20.0f, 0.0f, 80.0f, 80.0f) },
		};

		for (auto&& currTestCase : testCases)
		{
			REQUIRE(IntersectRects(std::get<0>(currTestCase), std::get<1>(currTestCase)) == std::get<2>(currTestCase));
		}
	}
}