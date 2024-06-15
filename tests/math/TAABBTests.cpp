#include <catch2/catch_test_macros.hpp>
#include <TDEngine2.h>


using namespace TDEngine2;


TEST_CASE("TAABB Tests")
{
	SECTION("TestContainsSphere_PassSphereThatOverlapsWithAABB_ReturnsTrue")
	{
		REQUIRE(ContainsSphere(TAABB(ZeroVector3, 1.0f, 1.0f, 1.0f), ZeroVector3, 1.0f));
		REQUIRE(ContainsSphere(TAABB(ZeroVector3, 1.0f, 1.0f, 1.0f), TVector3(10.0f, 0.0f, 0.0f), 15.0f));
	}

	SECTION("TestContainsSphere_PassSphereThatLiesOutsideOfAABB_ReturnsTrue")
	{
		REQUIRE(ContainsSphere(TAABB(ZeroVector3, 1.0f, 1.0f, 1.0f), TVector3(10.0f, 0.0f, 0.0f), 5.0f));
		REQUIRE(ContainsSphere(TAABB(TVector3(10.0f, 0.0f, 10.0f), 1.0f, 1.0f, 1.0f), ZeroVector3, 5.0f));
	}
}