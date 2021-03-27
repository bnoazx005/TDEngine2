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

	SECTION("TestGetBounds_BoundsAreUpdatedAfterAddPointOrRemovePointCalls_ReturnsCorrectBounds")
	{
		REQUIRE(RC_OK == pCurve->AddPoint({ 0.0f, 1.0f }));
		REQUIRE(RC_OK == pCurve->AddPoint({ 2.0f, 5.0f }));
		REQUIRE(RC_OK == pCurve->AddPoint({ 5.0f, 5.0f }));

		auto bounds = pCurve->GetBounds();
		REQUIRE((CMathUtils::Abs(bounds.x) < 1e-3f && CMathUtils::Abs(bounds.width - 5.0f) < 1e-3f));

		pCurve->RemovePoint(2); // Remove last point, now width should equal to 2.0

		bounds = pCurve->GetBounds();
		REQUIRE((CMathUtils::Abs(bounds.x) < 1e-3f && CMathUtils::Abs(bounds.width - 2.0f) < 1e-3f));
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

	SECTION("TestSample_TryGetIntermediateValueOfConstantCurve_ReturnsCorrectValue")
	{
		REQUIRE(RC_OK == pCurve->AddPoint({ 0.0f, 1.0f, -RightVector2 + UpVector2, RightVector2 + UpVector2 }));
		REQUIRE(RC_OK == pCurve->AddPoint({ 2.0f, 1.0f, -RightVector2 + UpVector2, RightVector2 + UpVector2 }));

		for (F32 t = 0.0f; t < 2.0f; t += 0.1f)
		{
			REQUIRE(CMathUtils::Abs(1.0f - pCurve->Sample(t)) < 1e-3f);
		}
	}

	SECTION("TestSample_PassValueOutOfBoundaries_ReturnsValuesOfClosestBorders")
	{
		REQUIRE(RC_OK == pCurve->AddPoint({ 0.0f, 1.0f, -RightVector2, RightVector2 }));
		REQUIRE(RC_OK == pCurve->AddPoint({ 2.0f, 5.0f, -RightVector2, RightVector2 }));

		REQUIRE(CMathUtils::Abs(1.0f - pCurve->Sample(-1.0f)) < 1e-3f);
		REQUIRE(CMathUtils::Abs(5.0f - pCurve->Sample(2.5f)) < 1e-3f);
	}

	SECTION("TestSample_CheckDifferenceWithCubicFunc_ReturnsMinimalError")
	{
		REQUIRE(RC_OK == pCurve->AddPoint({ 0.0f, 0.0f, -RightVector2, 0.2f* RightVector2 }));
		REQUIRE(RC_OK == pCurve->AddPoint({ 1.0f, 1.0f, -0.2f*UpVector2, RightVector2 }));

		for (F32 x = 0.0f; x < 1.0f; x += 0.01f)
		{
			REQUIRE(CMathUtils::Abs(pCurve->Sample(x) - x * x * x) < 1e-1f);
		}
	}

	REQUIRE(pCurve->Free() == RC_OK);
}