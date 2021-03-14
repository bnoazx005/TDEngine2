#include <catch2/catch.hpp>
#include <TDEngine2.h>


using namespace TDEngine2;


TEST_CASE("CAnimationCurve Tests")
{
	E_RESULT_CODE result = RC_OK;

	const TRectF32 curveBounds { 0.0f, 0.0f, 1.0f, 1.0f };

	CAnimationCurve* pCurve = CreateAnimationCurve(curveBounds, result);

	REQUIRE(result == RC_OK);

	SECTION("TestGetBounds_ReturnsInitialBounds")
	{
		REQUIRE(curveBounds == pCurve->GetBounds());
	}

	SECTION("TestSample_PassAnyTimeOnEmptyCurve_ReturnsZero")
	{
		REQUIRE(CMathUtils::IsLessOrEqual(pCurve->Sample(0.5f), 0.0f));
	}

	SECTION("TestAddPoint_PassUnorderedPoints_CurveStoresPointsInOrderBasedOnTimeValue")
	{
		REQUIRE(RC_OK == pCurve->AddPoint({ 0.1f, 0.1f }));
		REQUIRE(RC_OK == pCurve->AddPoint({ 0.0f, -1.0f }));
		REQUIRE(RC_OK == pCurve->AddPoint({ 1.0f, 1.0f }));
		REQUIRE(RC_OK == pCurve->AddPoint({ 0.5f, 2.0f }));

		for (auto it = pCurve->begin(); it != pCurve->end() - 1; ++it)
		{
			if (it->mTime > (it + 1)->mTime)
			{
				REQUIRE(false);
			}
		}
	}

	SECTION("TestRemovePoint_TryToRemovePointOnEmptyCurve_Returns_RC_INVALID_ARGS")
	{
		REQUIRE(RC_INVALID_ARGS == pCurve->RemovePoint(0));
	}

	SECTION("TestRemovePoint_RemovePoints_ReturnsOkIfPointExists")
	{
		REQUIRE(RC_OK == pCurve->AddPoint({ 0.1f, 0.1f }));
		REQUIRE(RC_OK == pCurve->AddPoint({ 0.0f, -1.0f }));
		REQUIRE(RC_OK == pCurve->AddPoint({ 1.0f, 1.0f }));

		REQUIRE((RC_OK == pCurve->RemovePoint(0) && RC_OK == pCurve->RemovePoint(1)));
	}

	REQUIRE(pCurve->Free() == RC_OK);
}