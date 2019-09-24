#include <catch2/catch.hpp>
#include <TDEngine2.h>
#include <vector>

using namespace TDEngine2;


TEST_CASE("CDeferOperation Tests")
{
	SECTION("TestDeferExecution_PassFunction_ExecuteFunctionWhenGoesOutOfScope")
	{
		bool isEvaluated = false;

		{
			CDeferOperation defer{ [&isEvaluated]() { isEvaluated = true; } };
		}

		REQUIRE(isEvaluated);
	}
}