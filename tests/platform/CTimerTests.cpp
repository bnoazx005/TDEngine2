#include <catch2/catch.hpp>
#include <TDEngine2.h>
#include <chrono>
#include <thread>
#include <cmath>


using namespace TDEngine2;


inline void TestSleep(U32 milliseconds)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}


TEST_CASE("Timers Tests")
{
	E_RESULT_CODE result = RC_OK;

	ITimer* pTimer = nullptr;

#if defined (TDE2_USE_WIN32PLATFORM)
	pTimer = CreateWin32Timer(result);
#elif defined (TDE2_USE_UNIXPLATFORM)
	pTimer = CreateUnixTimer(result);
#else
#endif

	REQUIRE(pTimer);
	REQUIRE(result == RC_OK);

#if 0
	SECTION("TestGetCurrTime_CorrectInput_ReturnsElapsedTimeFromStart")
	{
		F32 expectedElapsedTime = 500.0f;

		pTimer->Start();

		TestSleep(expectedElapsedTime);

		pTimer->Tick();

		F32 actualElapsedTime = pTimer->GetCurrTime() * 1000.0f;

		REQUIRE(fabsf(actualElapsedTime - expectedElapsedTime) < 2.0f);
	}
#endif

	SECTION("TestGetDeltaTime_EmulateFreeze_ReturnsAcceptableDeltasValue")
	{
		F32 expectedDeltaTime = DefaultFrameRate;

		F32 freezeTime = 1000.0f;

		pTimer->Start();

		TestSleep(freezeTime);

		pTimer->Tick();

		F32 actualDeltaTime = pTimer->GetDeltaTime();

		REQUIRE(((actualDeltaTime < expectedDeltaTime) || (fabsf(actualDeltaTime - expectedDeltaTime) < FloatEpsilon)));
	}

	REQUIRE(pTimer->Free() == RC_OK);
}