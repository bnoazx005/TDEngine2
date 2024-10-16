#include <catch2/catch.hpp>
#include <TDEngine2.h>


using namespace TDEngine2;


static F32 EaseInOutCubic(F32 x) { return x < 0.5f ? 4.0f * x * x * x : 1.0f - std::pow(-2.0f * x + 2.0f, 3.0) * 0.5f; }
static F32 EaseOutExpo(F32 x) { return CMathUtils::Abs(x - 1.0f) < 1e-3f ? 1.0f : 1.0f - std::pow(2.0, -10.0f * x); }


TEST_CASE("CAnimationCurve Tests")
{
	E_RESULT_CODE result = RC_OK;

	const TRectF32 curveBounds { 0.0f, 0.0f, 1.0f, 1.0f };

	CAnimationCurve* pCurve = CreateAnimationCurve(curveBounds, result);

	REQUIRE(result == RC_OK);

	REQUIRE((pCurve->GetPoint(0) && pCurve->GetPoint(1))); /// \note Two default points (0.0, 0.0) and (1.0, 1.0) already defined

	SECTION("TestGetBounds_ReturnsInitialBounds")
	{
		REQUIRE(curveBounds == pCurve->GetBounds());
	}

	SECTION("TestGetBounds_BoundsAreUpdatedAfterAddPointOrRemovePointCalls_ReturnsCorrectBounds")
	{
		REQUIRE(RC_OK == pCurve->AddPoint({ 2.0f, 5.0f }));
		REQUIRE(RC_OK == pCurve->AddPoint({ 5.0f, 5.0f }));

		auto bounds = pCurve->GetBounds();
		REQUIRE((CMathUtils::Abs(bounds.x) < 1e-3f && CMathUtils::Abs(bounds.width - 5.0f) < 1e-3f));

		pCurve->RemovePoint(pCurve->GetPointsCount() - 1); // Remove last point, now width should equal to 2.0

		bounds = pCurve->GetBounds();
		REQUIRE((CMathUtils::Abs(bounds.x) < 1e-3f && CMathUtils::Abs(bounds.width - 2.0f) < 1e-3f));
	}

	SECTION("TestSample_PassAnyTimeOnEmptyCurve_Returns0.5")
	{
		REQUIRE(CMathUtils::IsLessOrEqual(pCurve->Sample(0.5f), 0.5f));
	}

	SECTION("TestAddPoint_PassUnorderedPoints_CurveStoresPointsInOrderBasedOnTimeValue")
	{
		REQUIRE(RC_OK == pCurve->AddPoint({ 0.1f, 0.1f }));
		REQUIRE(RC_OK == pCurve->ReplacePoint({ 0.0f, -1.0f }));
		REQUIRE(RC_OK == pCurve->ReplacePoint({ 1.0f, 1.0f }));
		REQUIRE(RC_OK == pCurve->AddPoint({ 0.5f, 2.0f }));

		for (auto it = pCurve->begin(); it != pCurve->end() - 1; ++it)
		{
			if (it->mTime > (it + 1)->mTime)
			{
				REQUIRE(false);
			}
		}
	}

	SECTION("TestRemovePoint_TryToRemovePointOnEmptyCurve_Returns_RC_OK")
	{
		/// \note Because of a new created pCurve  already has two default points
		REQUIRE(RC_OK == pCurve->RemovePoint(0));
	}

	SECTION("TestRemovePoint_RemovePoints_ReturnsOkIfPointExists")
	{
		REQUIRE(RC_OK == pCurve->AddPoint({ 0.1f, 0.1f }));
		REQUIRE(RC_OK == pCurve->ReplacePoint({ 0.0f, -1.0f }));
		REQUIRE(RC_OK == pCurve->ReplacePoint({ 1.0f, 1.0f }));

		REQUIRE((RC_OK == pCurve->RemovePoint(0) && RC_OK == pCurve->RemovePoint(1)));
	}

	SECTION("TestSample_PassValueOutOfBoundaries_ReturnsValuesOfClosestBorders")
	{
		REQUIRE(RC_OK == pCurve->ReplacePoint({ 0.0f, 1.0f, -RightVector2, RightVector2 }));
		REQUIRE(RC_OK == pCurve->AddPoint({ 2.0f, 5.0f, -RightVector2, RightVector2 }));

		REQUIRE(CMathUtils::Abs(1.0f - pCurve->Sample(-1.0f)) < 1e-3f);
		REQUIRE(CMathUtils::Abs(5.0f - pCurve->Sample(2.5f)) < 1e-3f);
	}

	SECTION("TestSample_CheckDifferenceWithCubicFunc_ReturnsMinimalError")
	{
		REQUIRE(RC_OK == pCurve->ReplacePoint({ 0.0f, 0.0f, -RightVector2, RightVector2 }));
		REQUIRE(RC_OK == pCurve->ReplacePoint({ 1.0f, 1.0f, -UpVector2, RightVector2 }));

		for (F32 x = 0.0f; x < 1.0f; x += 0.01f)
		{
			REQUIRE(CMathUtils::Abs(pCurve->Sample(x) - x * x * x) < 11e-1f);
		}
	}

	SECTION("TestSample_CheckDifferenceWithLinearFunc_ReturnsMinimalError")
	{
		REQUIRE(RC_OK == pCurve->ReplacePoint({ 0.0f, 0.0f, ZeroVector2, ZeroVector2 }));
		REQUIRE(RC_OK == pCurve->ReplacePoint({ 1.0f, 1.0f, ZeroVector2, ZeroVector2 }));


		REQUIRE(CMathUtils::Abs(pCurve->Sample(0.0f)) < 1e-1f);
		REQUIRE(CMathUtils::Abs(pCurve->Sample(1.0f) - 1.0f) < 1e-1f);
		REQUIRE(CMathUtils::Abs(pCurve->Sample(0.75f) - 0.75f) < 1e-1f);
		REQUIRE(CMathUtils::Abs(pCurve->Sample(0.25f) - 0.25f) < 1e-1f);
	}

	SECTION("TestSample_CheckDifferenceWithLinearFunc_ReturnsMinimalError")
	{
		REQUIRE(RC_OK == pCurve->ReplacePoint({ 0.0f, 0.0f, -RightVector2, ZeroVector2 }));
		REQUIRE(RC_OK == pCurve->ReplacePoint({ 1.0f, 1.0f, ZeroVector2, RightVector2 }));

		for (F32 x = 0.0f; x < 1.0f; x += 0.01f)
		{
			REQUIRE(CMathUtils::Abs(pCurve->Sample(x) - x) < 11e-1f);
		}
	}

	SECTION("TestSample_CheckDifferenceWithEaseInOutCubic_ReturnsMinimalError")
	{
		REQUIRE(RC_OK == pCurve->ReplacePoint({ 0.0f, 0.0f, -RightVector2, RightVector2 }));
		REQUIRE(RC_OK == pCurve->ReplacePoint({ 1.0f, 1.0f, ZeroVector2, ZeroVector2 }));

		for (F32 x = 0.0f; x < 1.0f; x += 0.01f)
		{
			REQUIRE(CMathUtils::Abs(pCurve->Sample(x) - EaseInOutCubic(x)) < 12e-1f);
		}
	}

	SECTION("TestSample_CheckDifferenceWithEaseOutExpo_ReturnsMinimalError")
	{
		REQUIRE(RC_OK == pCurve->ReplacePoint({ 0.0f, 0.0f, ZeroVector2, UpVector2 }));
		REQUIRE(RC_OK == pCurve->ReplacePoint({ 1.0f, 1.0f, UpVector2, ZeroVector2 }));

		for (F32 x = 0.0f; x < 1.0f; x += 0.01f)
		{
			REQUIRE(CMathUtils::Abs(pCurve->Sample(x) - EaseOutExpo(x)) < 11e-1f);
		}
	}

	REQUIRE(pCurve->Free() == RC_OK);
}