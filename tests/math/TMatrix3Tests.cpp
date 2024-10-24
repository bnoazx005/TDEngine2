#include <catch2/catch.hpp>
#include <TDEngine2.h>


using namespace TDEngine2;


TEST_CASE("TMatrix3 Tests")
{
	SECTION("TestEqualityOperator_PassSameMatrices_ReturnsTrue")
	{
		/// first is an expected, second is actual result
		std::tuple<TMatrix3, TMatrix3> testCases[]
		{
			{ IdentityMatrix3, IdentityMatrix3 },
			{ ZeroMatrix3, ZeroMatrix3 },
		};

		for (auto&& currCase : testCases)
		{
			REQUIRE(std::get<0>(currCase) == std::get<1>(currCase));
			REQUIRE(!(std::get<0>(currCase) != std::get<1>(currCase)));
		}
	}

	SECTION("TestEqualityOperator_PassDifferentMatrices_ReturnsFalse")
	{
		/// first is an expected, second is actual result
		std::tuple<TMatrix3, TMatrix3> testCases[]
		{
			{ IdentityMatrix3, ZeroMatrix3 },
			{ TMatrix3(TVector4(0.96f, 1.0f, -5.0f, 1.0f)), IdentityMatrix3 },
		};

		for (auto&& currCase : testCases)
		{
			REQUIRE(std::get<0>(currCase) != std::get<1>(currCase));
			REQUIRE(!(std::get<0>(currCase) == std::get<1>(currCase)));
		}
	}

	SECTION("TestDet_PassMatrix_ReturnsCorrectResult")
	{
		std::tuple<F32, TMatrix3> testCases[]
		{
			{ 0.0f, ZeroMatrix3 },
			{ 1.0f, IdentityMatrix3 },
			{ (-0.00119f * (-0.002474f * 0.000398f - 0.020828f * -0.001246f) - 0.020798f * (0.000543f * 0.000398f - 0.020828f * -0.020941f) + 0.002501f * (0.000543f * -0.001246f - -0.002474f * -0.020941f)),
			TMatrix3(-0.00119f, 0.020798f, 0.002501f,
					 0.000543f, -0.002474f, 0.020828f,
					 -0.020941f, -0.001246f, 0.000398f) }
		};

		for (auto&& currCase : testCases)
		{
			REQUIRE(CMathUtils::Abs(std::get<0>(currCase) - Det(std::get<1>(currCase))) < 1e-3f);
		}
	}

	SECTION("TestInverse_PassInvertibleMatrix_ReturnsCorrectNonZeroMatrix")
	{
		/// first is an expected, second is actual result
		std::tuple<TMatrix3, TMatrix3> testCases[]
		{
			{ IdentityMatrix3, IdentityMatrix3 },
			{ TMatrix3(TVector3(1.0f, 2.0f, 3.0f)), TMatrix3(TVector3(1.0f, 1.0f / 2.0f, 1.0f / 3.0f)) },
			{ 
				TMatrix3(-2.703023f, 1.233540f, -47.567623f,
						 47.243594f, -5.618866f, -2.830377f,
						 5.682177f, 47.312709f, 0.903921f),
				TMatrix3(-0.00119f, 0.020798f, 0.002501f, 
						 0.000543f, -0.002474f, 0.020828f, 
						-0.020941f, -0.001246f, 0.000398f) 
			},
		};

		for (auto&& currCase : testCases)
		{
			REQUIRE(CMathUtils::Abs(Det(std::get<1>(currCase))) > 0.0f);

			const TMatrix3 invertedMat = Inverse(std::get<1>(currCase));
			REQUIRE(std::get<0>(currCase) == invertedMat);
		}
	}
}