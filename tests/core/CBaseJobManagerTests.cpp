#include <catch2/catch_test_macros.hpp>
#include <TDEngine2.h>
#include <chrono>
#include <thread>


using namespace TDEngine2;


TEST_CASE("CBaseJobManager Tests")
{
	E_RESULT_CODE result = RC_OK;
	
	TPtr<IJobManager> pJobManager = TPtr<IJobManager>(CreateBaseJobManager({ std::thread::hardware_concurrency(), CreateLinearAllocator}, result));
	REQUIRE((pJobManager && RC_OK == result));

	SECTION("TestSubmitJob_PassEmptyJob_ReturnsError")
	{
		REQUIRE(RC_INVALID_ARGS == pJobManager->SubmitJob(nullptr, nullptr));
	}

	SECTION("TestSubmitJob_PassSimpleJob_ExecutesThatAndArgumentIndexEqualsToZero")
	{
		std::atomic_bool isExecuted{ false };

		TJobCounter counter;

		E_RESULT_CODE result = pJobManager->SubmitJob(&counter, [&isExecuted](auto)
		{
			isExecuted = true;
		});

		pJobManager->WaitForJobCounter(counter);

		REQUIRE((RC_OK == result && isExecuted));
	}

	SECTION("TestWaitForJobCounter_PassTwoJobs_FinalPartExecutedOnlyAfterDependencies")
	{
		using namespace std::chrono_literals;

		const std::chrono::duration<F32, std::milli> expectedDuration = 2000ms;

		TJobCounter counter;

		auto start = std::chrono::high_resolution_clock::now();

		pJobManager->SubmitJob(&counter, [expectedDuration](auto&&)
		{
			std::this_thread::sleep_for(expectedDuration);
			LOG_MESSAGE("First Test Job");
		});
		pJobManager->SubmitJob(&counter, [](auto&&)
		{
			LOG_MESSAGE("Second Test Job");
		});

		pJobManager->WaitForJobCounter(counter);

		std::chrono::duration<F32, std::milli> duration = std::chrono::high_resolution_clock::now() - start;
		REQUIRE(duration.count() > expectedDuration.count() - 0.1f);

		LOG_MESSAGE("Final Job");
	}

	SECTION("TestSubmitMultipleJobs_PassArrayAndMakeParallelFor_CorreclyProcess")
	{
		TJobCounter counter;

		std::vector<U32> testVector(static_cast<USIZE>(rand() % 42 + 1));
		const U32 groupSize = static_cast<U32>(rand() % (testVector.size() / 2) + 1);

		pJobManager->SubmitMultipleJobs(&counter, static_cast<U32>(testVector.size()), groupSize, [&testVector](const TJobArgs& args)
		{
			testVector[args.mJobIndex] = args.mJobIndex;
		});

		pJobManager->WaitForJobCounter(counter);

		for (USIZE i = 0; i < testVector.size(); i++)
		{
			REQUIRE(testVector[i] == i);
		}
	}

	SECTION("TestWaitJobCounter_JobEmitsSubTask_JobManagerCorrectlyProcessThemBoth")
	{
		const std::string expectedResult = "ABCCBA";

		//for (I32 i = 0; i < 10; ++i)
		{
			std::string actualString;

			TJobCounter counter;

			E_RESULT_CODE result = pJobManager->SubmitJob(&counter, [&actualString, pJobManager](const TJobArgs& jobArgs)
			{
				auto pJob = jobArgs.mpCurrJob;

				pJobManager->SubmitJob(pJob->mpCounter, [&actualString](auto)
				{
					actualString += "AB";
				}, "TDE2SecondJob");

				pJobManager->SubmitJob(pJob->mpCounter, [&actualString](auto)
				{
					actualString += "CC";
				}, "TDE2ThirdJob");

				pJobManager->WaitForJobCounter(*pJob->mpCounter, 1, pJob);

				actualString += "BA";
			}, "TDE2FirstJob");

			pJobManager->WaitForJobCounter(counter);

			REQUIRE(actualString == expectedResult);
		}
	}
}