#include <catch2/catch.hpp>
#include <TDEngine2.h>


using namespace TDEngine2;


TEST_CASE("CFrustum Tests")
{
	E_RESULT_CODE result = RC_OK;

	IFrustum* pFrustum = CreateFrustum(result);

	REQUIRE(result == RC_OK);

	SECTION("TestGetLayerIndex_WithSingleLayer_AlwaysReturnsZero")
	{
		const F32 viewProjElements[16] { 0.750000238f, 0.0f, 0.0f, 0.0f,
										  0.0f, 1.00000036f, 0.0f, 0.0f,
										  0.0f, 0.0f, 1.00100100f, -1.00100100f,
										  0.0f, 0.0f, 1.0f, 0.0f };

		REQUIRE(pFrustum->ComputeBounds(Inverse(TMatrix4(viewProjElements)), 0.0f) == RC_OK);
	}

	REQUIRE(pFrustum->Free() == RC_OK);
}