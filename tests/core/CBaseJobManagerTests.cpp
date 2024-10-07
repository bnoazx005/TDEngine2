#include <catch2/catch_test_macros.hpp>
#include <TDEngine2.h>
#include <chrono>
#include <thread>


using namespace TDEngine2;


constexpr U32 SamplesCount = 1000;
const U32 NumOfWorkerThreads = std::thread::hardware_concurrency();


TEST_CASE("CBaseJobManager Tests")
{
	E_RESULT_CODE result = RC_OK;
	
	TPtr<IJobManager> pJobManager = TPtr<IJobManager>(CreateBaseJobManager({ NumOfWorkerThreads }, result));
	REQUIRE((pJobManager && RC_OK == result));

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

			auto&& id = Wrench::StringUtils::Format("Job{0}", i);

			E_RESULT_CODE result = pJobManager->SubmitJob(&counter, [&isExecuted](auto)
			{
				isExecuted = true;
			}, { E_JOB_PRIORITY_TYPE::NORMAL, false, id.c_str() });

			pJobManager->WaitForJobCounter(counter);

			REQUIRE((RC_OK == result && isExecuted));
		}
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
				TJobCounter nestedCounter;

				pJobManager->SubmitJob(&nestedCounter, [&actualString, pJobManager](auto)
				{
					actualString += "AB";

					TJobCounter nestedCounter1;

					pJobManager->SubmitJob(&nestedCounter1, [&actualString](auto)
					{
						actualString += "CC";
					}, { E_JOB_PRIORITY_TYPE::NORMAL, false, "TDE2ThirdJob" });

					pJobManager->WaitForJobCounter(nestedCounter1);
				}, { E_JOB_PRIORITY_TYPE::NORMAL, false, "TDE2SecondJob" });

				pJobManager->WaitForJobCounter(nestedCounter);

				actualString += "BA";

			}, { E_JOB_PRIORITY_TYPE::NORMAL, false, "TDE2FirstJob" });

			pJobManager->WaitForJobCounter(counter);

			REQUIRE(actualString == expectedResult);
		}
	}

	SECTION("TestWaitJobCounter_JobEmitsSubTaskAndChangeGlobalVariable_JobManagerCorrectlyProcessThemBoth")
	{
		const int expectedCounterValue = 1;
		static std::atomic_int actualCounterValue{ 0 };

		for (U32 i = 0; i < SamplesCount; i++)
		{
			actualCounterValue = 0;
			TJobCounter counter;

			E_RESULT_CODE result = pJobManager->SubmitJob(&counter, [=](const TJobArgs& jobArgs)
			{
				TJobCounter nestedCounter;

				pJobManager->SubmitJob(&nestedCounter, [](auto)
				{
				}, { E_JOB_PRIORITY_TYPE::NORMAL, false, "TDE2SecondJob" });

				pJobManager->WaitForJobCounter(nestedCounter);
				actualCounterValue++;
			}, { E_JOB_PRIORITY_TYPE::NORMAL, false, "TDE2FirstJob" });

			pJobManager->WaitForJobCounter(counter);
			REQUIRE(actualCounterValue == expectedCounterValue);
		}
	}

	SECTION("TestSubmitJob_SumbitFewTasksUnderSameCounter_WaitForCounterBlocksExecutionUntilAllTasksCompleted")
	{
		for (U32 i = 0; i < SamplesCount; i++)
		{
			std::atomic_bool isJobAExecuted { false };
			std::atomic_bool isJobBExecuted { false };
			std::atomic_bool isJobCExecuted { false };

			TJobCounter counter;
			REQUIRE(counter.mValue == TJobCounterId::Invalid);

			pJobManager->SubmitJob(&counter, [&isJobAExecuted](auto) { isJobAExecuted = true; });

			REQUIRE(counter.mValue != TJobCounterId::Invalid);
			TJobCounterId prevCounterValue = counter.mValue;

			pJobManager->SubmitJob(&counter, [&isJobBExecuted](auto) { isJobBExecuted = true; });

			REQUIRE(counter.mValue == prevCounterValue);

			pJobManager->SubmitJob(&counter, [&isJobCExecuted](auto) { isJobCExecuted = true; });

			REQUIRE(counter.mValue == prevCounterValue);

			pJobManager->WaitForJobCounter(counter);

			REQUIRE((isJobAExecuted && isJobBExecuted && isJobCExecuted));
		}
	}
}