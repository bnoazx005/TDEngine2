#include <catch2/catch_test_macros.hpp>
#include <TDEngine2.h>
#include <chrono>
#include <thread>


using namespace TDEngine2;


constexpr U32 SamplesCount = 1000;
const U32 NumOfWorkerThreads = 1;// std::thread::hardware_concurrency();


TEST_CASE("CBaseJobManager Tests")
{
	E_RESULT_CODE result = RC_OK;
	
	TPtr<IJobManager> pJobManager = TPtr<IJobManager>(CreateBaseJobManager({ NumOfWorkerThreads, CreateLinearAllocator}, result));
	REQUIRE((pJobManager && RC_OK == result));
#if 0
	SECTION("TestSubmitJob_PassEmptyJob_ReturnsError")
	{
		REQUIRE(RC_INVALID_ARGS == pJobManager->SubmitJob(nullptr, nullptr));
	}

	SECTION("TestSubmitJob_PassSimpleJob_ExecutesThatAndArgumentIndexEqualsToZero")
	{
		for (U32 i = 0; i < SamplesCount; i++)
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

		for (I32 i = 0; i < SamplesCount; ++i)
		{
			std::string actualString;

			TJobCounter counter;

			E_RESULT_CODE result = pJobManager->SubmitJob(&counter, [&actualString, pJobManager](const TJobArgs& jobArgs)
			{
				auto pJob = jobArgs.mpCurrJob;

				LOG_MESSAGE(">>>>");

				pJobManager->SubmitJob(pJob->mpCounter, [&actualString](auto)
				{
					actualString += "AB";
					LOG_MESSAGE("Second Job Finished");
				}, E_JOB_PRIORITY_TYPE::NORMAL, "TDE2SecondJob");

				pJobManager->SubmitJob(pJob->mpCounter, [&actualString](auto)
				{
					actualString += "CC";
					LOG_MESSAGE(" Third Job Finished");
				}, E_JOB_PRIORITY_TYPE::NORMAL, "TDE2ThirdJob");

				LOG_MESSAGE(" WaitForCounter1");
				pJobManager->WaitForJobCounter(*pJob->mpCounter, 1, jobArgs.mpCurrJob);

				actualString += "BA";
				LOG_MESSAGE(" First Job Finished");

			}, E_JOB_PRIORITY_TYPE::NORMAL, "TDE2FirstJob");

			LOG_MESSAGE(" WaitForCounter");
			pJobManager->WaitForJobCounter(counter);

			REQUIRE(actualString == expectedResult);

			LOG_MESSAGE("<<<<\n");
		}
	}
#endif

	SECTION("TestWaitJobCounter_JobEmitsSubTask_JobManagerCorrectlyProcessThemBoth")
	{
		const int expectedCounterValue = 2;
		std::atomic_int actualCounterValue = 0;

		TJobCounter counter;

		E_RESULT_CODE result = pJobManager->SubmitJob(&counter, [&actualCounterValue, pJobManager](const TJobArgs& jobArgs)
		{
			auto pJob = jobArgs.mpCurrJob;
			pJobManager->SubmitJob(pJob->mpCounter, [&actualCounterValue](auto)
			{
				LOG_MESSAGE("Second Job Finished");
			}, E_JOB_PRIORITY_TYPE::NORMAL, "TDE2SecondJob");

			pJobManager->WaitForJobCounter(*jobArgs.mpCurrJob->mpCounter, 1, jobArgs.mpCurrJob);
			actualCounterValue++;
		}, E_JOB_PRIORITY_TYPE::NORMAL, "TDE2FirstJob");

		LOG_MESSAGE(" WaitForCounter");
		pJobManager->WaitForJobCounter(counter);

		REQUIRE(actualCounterValue == expectedCounterValue);

		LOG_MESSAGE("<<<<\n");
	}
}