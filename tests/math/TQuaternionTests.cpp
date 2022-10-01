#include <catch2/catch.hpp>
#include <TDEngine2.h>


using namespace TDEngine2;


TEST_CASE("TQuaternion Tests")
{
	SECTION("TestEulerConstructor_PassCorrectResults_CreateCorrectQuaternion")
	{
		const F32 testAngle = 45.0f * CMathConstants::Deg2Rad;

		/// first is an expected, second is actual result
		std::tuple<TQuaternion, TQuaternion> testCases[] 
		{
			{ TQuaternion { 0.3826f, 0.0f, 0.0f, 0.9238f }, TQuaternion { TVector3 { testAngle, 0.0f, 0.0f } } },
			{ TQuaternion { 0.0f, 0.3826f, 0.0f, 0.9238f }, TQuaternion { TVector3 { 0.0f, testAngle, 0.0f } } },
			{ TQuaternion { 0.0f, 0.966f, 0.0f, 0.259f }, TQuaternion { TVector3 { 0.0f, 150.0f * CMathConstants::Deg2Rad, 0.0f } } },
			{ TQuaternion { 0.0f, 0.0f, 0.3826f, 0.9238f }, TQuaternion { TVector3 { 0.0f, 0.0f, testAngle } } },
			{ TQuaternion { 0.35355f, -0.14644f, 0.35355f, 0.85355f }, TQuaternion { TVector3 { testAngle, 0.0f, testAngle } } },
			{ TQuaternion { 0.14644f, 0.35355f, 0.35355f, 0.85355f }, TQuaternion { TVector3 { 0.0f, testAngle, testAngle } } },
			{ TQuaternion { 0.3536f, 0.3536f, 0.1464f, 0.8536f }, TQuaternion { TVector3 { testAngle, testAngle, 0.0f } } },
		};

		for (auto&& currCase : testCases)
		{
			REQUIRE(std::get<0>(currCase) == std::get<1>(currCase));
		}
	}

	SECTION("TestToEulerAngles_PassQuaternion_ReturnsCorrectAngles")
	{
		const F32 testAngle = 45.0f * CMathConstants::Deg2Rad;

		/// first is input, second is expected result
		std::tuple<TQuaternion, TVector3> testCases[]
		{
			{ TQuaternion { 0.3826f, 0.0f, 0.0f, 0.9238f }, TVector3 { testAngle, 0.0f, 0.0f } },
			{ TQuaternion { 0.0f, 0.3826f, 0.0f, 0.9238f }, TVector3 { 0.0f, testAngle, 0.0f } },
			{ TQuaternion { 0.0f, 0.0f, 0.3826f, 0.9238f }, TVector3 { 0.0f, 0.0f, testAngle } },
			{ TQuaternion { 0.35355f, -0.14644f, 0.35355f, 0.85355f }, TVector3 { testAngle, 0.0f, testAngle } },
			{ TQuaternion { 0.14644f, 0.35355f, 0.35355f, 0.85355f }, TVector3 { 0.0f, testAngle, testAngle } },
			{ TQuaternion { 0.3536f, 0.3536f, -0.1464f, 0.8536f }, TVector3 { testAngle, testAngle, 0.0f } },
		};

		for (auto&& currCase : testCases)
		{
			REQUIRE(std::get<TVector3>(currCase) == ToEulerAngles(std::get<TQuaternion>(currCase)));
		}
	}

	SECTION("TestSlerp_PassTwoSameQuaternions_ReturnsItAtWholeRange")
	{
		const TQuaternion q { 0.0f, 0.0f, 0.78f, 0.78f };

		for (F32 t = 0; t < 1.1f; t += 0.1f)
		{
			REQUIRE(q == Slerp(q, q, t));
		}
	}

	/*SECTION("TestQuaternion_RotateAroundYAxis_CorrectlyConvertibleBetweenQuaternionEulerAngles")
	{
		for (F32 angle = 0; angle < 360.0f; angle += 30.0f)
		{
			const TQuaternion rotation(TVector3(0.0f, angle, 0.0f) * CMathConstants::Deg2Rad);

			REQUIRE((Length(rotation) - 1.0f) < 1e-3f);
			REQUIRE(CMathUtils::Abs(Dot(TVector3(rotation.x, rotation.y, rotation.z), RightVector3)) < 1e-3f);

			REQUIRE(ToEulerAngles(rotation) == (TVector3(0.0f, angle, 0.0f) * CMathConstants::Deg2Rad));
		}
	}*/
}