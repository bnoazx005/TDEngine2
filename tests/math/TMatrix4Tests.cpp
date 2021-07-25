#include <catch2/catch.hpp>
#include <TDEngine2.h>


using namespace TDEngine2;


TEST_CASE("TMatrix4 Tests")
{
	SECTION("TestEqualityOperator_PassSameMatrices_ReturnsTrue")
	{
		/// first is an expected, second is actual result
		std::tuple<TMatrix4, TMatrix4> testCases[]
		{
			{ IdentityMatrix4, IdentityMatrix4 },
			{ ZeroMatrix4, ZeroMatrix4 },
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
		std::tuple<TMatrix4, TMatrix4> testCases[]
		{
			{ IdentityMatrix4, ZeroMatrix4 },
			{ TMatrix4(TVector4(0.96f, 1.0f, -5.0f, 1.0f)), IdentityMatrix4 },
		};

		for (auto&& currCase : testCases)
		{
			REQUIRE(std::get<0>(currCase) != std::get<1>(currCase));
			REQUIRE(!(std::get<0>(currCase) == std::get<1>(currCase)));
		}
	}

	SECTION("TestInverse_PassInvertibleMatrix_ReturnsCorrectNonZeroMatrix")
	{
		/// first is an expected, second is actual result
		std::tuple<TMatrix4, TMatrix4> testCases[]
		{
			{ IdentityMatrix4, IdentityMatrix4 },
			{ TMatrix4(TVector4(1.0f, 2.0f, 3.0f, 4.0f)), TMatrix4(TVector4(1.0f, 1.0f / 2.0f, 1.0f / 3.0f, 1.0f / 4.0f)) },
			{ TranslationMatrix(TVector3(1.0f, 2.0f, 3.0f)), TranslationMatrix(TVector3(-1.0f, -2.0f, -3.0f)) },
			{ 
				TMatrix4(-2.703023f, 1.233540f, -47.567623f, -5.3724935f,
						 47.243594f, -5.618866f, -2.830377f, -28.876468f,
						 5.682177f, 47.312709f, 0.903921f, 14.569180f,
						 0.0f, 0.0f, 0.0f, 1.0f),
				TMatrix4(-0.00119f, 0.020798f, 0.002501f, 0.557742f,
						 0.000543f, -0.002474f, 0.020828f, -0.37197f, 
						-0.020941f, -0.001246f, 0.000398f, -0.154284f, 
						0.0f, 0.0f, 0.0f, 1.0f) 
			},
		};

		for (auto&& currCase : testCases)
		{
			const F32 det = Det(std::get<1>(currCase));
			REQUIRE(CMathUtils::Abs(det) > 0.0f);

			const TMatrix4 invertedMat = Inverse(std::get<1>(currCase));
			REQUIRE(std::get<0>(currCase) == invertedMat);
		}
	}

	SECTION("TestRotationMatrix_PassCorrectQuaternion_ReturnsCorrectRotationMatrix")
	{
		/// first is an expected, second is actual result
		std::tuple<TMatrix4, TQuaternion> testCases[]
		{
			{ IdentityMatrix4, UnitQuaternion },
			{ 
				TMatrix4(1.0f, 0.0f, 0.0f, 0.0f,
						 0.0f, 0.0f, -1.0f, 0.0f, 
						 0.0f, 1.0f, 0.0f, 0.0f, 
						 0.0f, 0.0f, 0.0f, 1.0f), 
				TQuaternion(1.0f, 0.0f, 0.0f, 1.0f) 
			},
			{ 
				TMatrix4(0.0f, 0.0f, 1.0f, 0.0f,
						 0.0f, 1.0f, 0.0f, 0.0f, 
						 -1.0f, 0.0f, 0.0f, 0.0f, 
						 0.0f, 0.0f, 0.0f, 1.0f), 
				TQuaternion(0.0f, 1.0f, 0.0f, 1.0f) 
			},
			{ 
				TMatrix4(0.0f, -1.0f, 0.0f, 0.0f,
						 1.0f, 0.0f, 0.0f, 0.0f,
						 0.0f, 0.0f, 1.0f, 0.0f, 
						 0.0f, 0.0f, 0.0f, 1.0f),
				TQuaternion(0.0f, 0.0f, 1.0f, 1.0f) 
			},
			{ 
				TMatrix4(0.333f, -0.667f, 0.667f, 0.0f,
						 0.667f, -0.333f, -0.667f, 0.0f, 
						 0.667f, 0.667f, 0.333f, 0.0f, 
						 0.0f, 0.0f, 0.0f, 1.0f), 
				TQuaternion(1.0f, 0.0f, 1.0f, 1.0f)
			},
			{ 
				TMatrix4(0.0f, 0.0f, 1.0f, 0.0f,
						 1.0f, 0.0f, 0.0f, 0.0f,
						 0.0f, 1.0f, 0.0f, 0.0f, 
						 0.0f, 0.0f, 0.0f, 1.0f), 
				TQuaternion(1.0f, 1.0f, 1.0f, 1.0f) 
			},
			{ 
				TMatrix4(-0.333f, -0.667f, 0.667f, 0.0f, 
						 0.667f, 0.333f, 0.667f, 0.0f, 
						 -0.667f, 0.667f, 0.333f, 0.0f,
						 0.0f, 0.0f, 0.0f, 1.0f), 
				TQuaternion(0.0f, 1.0f, 1.0f, 1.0f)
			},
		};

		for (auto&& currCase : testCases)
		{
			REQUIRE(std::get<TMatrix4>(currCase) == RotationMatrix(std::get<TQuaternion>(currCase)));
		}
	}
}