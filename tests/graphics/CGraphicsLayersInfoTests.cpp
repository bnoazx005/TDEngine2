#include <catch2/catch.hpp>
#include <TDEngine2.h>
#include <climits>
#include <tuple>


using namespace TDEngine2;


TEST_CASE("CGraphicsLayersInfo Tests")
{
	E_RESULT_CODE result = RC_OK;

	IGraphicsLayersInfo* pGraphicsLayersInfo = CreateGraphicsLayersInfo(result);

	REQUIRE(result == RC_OK);

	SECTION("TestGetLayerIndex_WithSingleLayer_AlwaysReturnsZero")
	{
		F32 testSamples[] { (std::numeric_limits<F32>::min)(), 0.0f, 1000.0f ,(std::numeric_limits<F32>::max)() };

		for (F32 currTestSample : testSamples)
		{
			REQUIRE(pGraphicsLayersInfo->GetLayerIndex(currTestSample) == 0);
		}
	}

	SECTION("TestGetLayerIndex_AddLayersAndRetrieveTheirIndices_ReturnsCorrectIndices")
	{
		std::vector<std::pair<F32, U32>> testSet 
		{
			{ -(std::numeric_limits<F32>::infinity)(), 0 },
			{ 1.0f, 1 },
			{ 10.0f, 1 },
			{ 64.0f, 2 },
			{ 256.0f, 3 },
			{ 1024.0f, 4 },
		};

		F32 graphicsLayersBounds[] { 0.0f, 10.0f, 100.0f, 500.0f };

		for (F32 currLayerRightBound : graphicsLayersBounds)
		{
			REQUIRE(pGraphicsLayersInfo->AddLayer(currLayerRightBound) == RC_OK);
		}

		for (auto iter = testSet.cbegin(); iter != testSet.cend(); ++iter)
		{
			REQUIRE(pGraphicsLayersInfo->GetLayerIndex((*iter).first) == (*iter).second);
		}
	}

	SECTION("TestAddLayer_IncorrectDefinitionsOrder_AddLayerReturnsError")
	{
		std::vector<std::pair<F32, E_RESULT_CODE>> testSet
		{
			{ 0.0f, RC_OK },
			{ 10.0f, RC_OK },
			{ 5.0f, RC_INVALID_ARGS },
			{ 15.0f, RC_OK },
		};

		for (auto iter = testSet.cbegin(); iter != testSet.cend(); ++iter)
		{
			REQUIRE(pGraphicsLayersInfo->AddLayer((*iter).first) == (*iter).second);
		}
	}

	REQUIRE(pGraphicsLayersInfo->Free() == RC_OK);
}