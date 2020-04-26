#include <catch2/catch.hpp>
#include <TDEngine2.h>


using namespace TDEngine2;


TEST_CASE("CFrustum Tests")
{
	E_RESULT_CODE result = RC_OK;

	IFrustum* pFrustum = CreateFrustum(result);

	REQUIRE(result == RC_OK);

	const F32 viewProjElements[16]{ 0.750000238f, 0.0f, 0.0f, 0.0f,
								    0.0f, 1.00000036f, 0.0f, 0.0f,
								    0.0f, 0.0f, 1.00100100f, -1.00100100f,
								    0.0f, 0.0f, 1.0f, 0.0f };

	REQUIRE(pFrustum->ComputeBounds(Inverse(TMatrix4(viewProjElements)), 0.0f) == RC_OK);

	SECTION("TestTestPoint_PassInternalPoints_ReturnsTrue")
	{
		auto points =
		{
			TVector3(0.5f, 0.5f, 2.0f),
			TVector3(1000.0f, 500.0f, 1000.0f),
		};

		for (auto&& currPoint : points)
		{
			REQUIRE(pFrustum->TestPoint(currPoint));
		}
	}

	SECTION("TestTestPoint_PassExpternalPoints_ReturnsFalse")
	{
		auto points =
		{
			ZeroVector3,
			TVector3(500.0f, 500.0f, 1001.0f)
		};

		for (auto&& currPoint : points)
		{
			REQUIRE(!pFrustum->TestPoint(currPoint));
		}
	}

	SECTION("TestTestSphere_PassSphereThatLiesInsideOfFrustum_ReturnsTrue")
	{
		std::tuple<TVector3, F32> testCases[]
		{
			{ ZeroVector3, 1.0f },
			{ { 500.0f, 500.0f, 500.0f }, 10.0f },
			{ { 500.0f, 500.0f, 500.0f }, 500.0f },
		};

		for (auto&& currSphere : testCases)
		{
			REQUIRE(pFrustum->TestSphere(std::get<TVector3>(currSphere), std::get<F32>(currSphere)));
		}
	}

	SECTION("TestTestSphere_PassSphereThatLiesOutsideOfFrustum_ReturnsFalse")
	{
		std::tuple<TVector3, F32> testCases[] 
		{
			{ ZeroVector3, 0.5f },
			{ ZeroVector3, 0.99f },
			{ { 1010.0f, 1010.0f, 0.0f }, 10.0f },
		};

		for (auto&& currSphere : testCases)
		{
			REQUIRE(!pFrustum->TestSphere(std::get<TVector3>(currSphere), std::get<F32>(currSphere)));
		}
	}

	REQUIRE(pFrustum->Free() == RC_OK);
}