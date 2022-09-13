#include "../../include/autotests/CTestContext.h"
#include "../../include/autotests/CBaseTestFixture.h"
#include "../../include/autotests/ITestResultsReporter.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	TDE2_DEFINE_SCOPED_PTR(ITestResultsReporter)


	CTestContext::CTestContext():
		CBaseObject()
	{
	}

	E_RESULT_CODE CTestContext::Init(const TTestContextConfig& config)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!config.mpEngineCore)
		{
			return RC_INVALID_ARGS;
		}

		mpEngineCore = config.mpEngineCore;
		
		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CTestContext::AddTestFixture(TPtr<ITestFixture> pFixture)
	{
		if (!pFixture)
		{
			return RC_INVALID_ARGS;
		}

		mTestFixtures.push_back(pFixture);

		return RC_OK;
	}

	void CTestContext::AddTestResult(const std::string& testFixtureName, const std::string& testCaseName, const TTestResultEntity& result)
	{
		++mTotalTestsCount;

		mTestResults[testFixtureName].insert({ testCaseName, result });

		if (result.mHasPassed)
		{
			++mPassedTestsCount;
		}
		else
		{
			++mFailedTestsCount;
		}
	}

	E_RESULT_CODE CTestContext::WriteTestResults(TPtr<ITestResultsReporter> pReporter)
	{
		if (!pReporter)
		{
			return RC_INVALID_ARGS;
		}

		for (auto&& currTestFixtureEntry : mTestResults)
		{
			pReporter->EnterTestFixtureSection(currTestFixtureEntry.first);

			for (auto&& currTestResult : currTestFixtureEntry.second)
			{
				pReporter->WriteTestResult(currTestResult.first, currTestResult.second);
			}

			pReporter->ExitTestFixtureSection();
		}
		
		return RC_OK;
	}

	E_RESULT_CODE CTestContext::Assert(const std::string& message, bool actual, bool expected, const std::string& filename, U32 line)
	{
		if (actual != expected)
		{
			throw CAssertException(message, filename, line);
		}

		return RC_OK;
	}

	E_RESULT_CODE CTestContext::RunAllTests()
	{
		if (mIsRunning)
		{
			return RC_FAIL;
		}

		mIsRunning = true;

		mTotalTestsCount = 0;
		mPassedTestsCount = 0;
		mFailedTestsCount = 0;

		return RC_OK;
	}

	void CTestContext::Update(F32 dt)
	{
		if (!mIsRunning)
		{
			return;
		}

		TPtr<ITestFixture> pCurrFixture = mTestFixtures.front();

		if (!pCurrFixture->IsFinished())
		{
			pCurrFixture->Update(dt);
			return;
		}

		mTestFixtures.erase(mTestFixtures.cbegin());

		mIsRunning = !mTestFixtures.empty();
	}

	bool CTestContext::IsFinished() const
	{
		return !mIsRunning && mTestFixtures.empty();
	}

	TPtr<CTestContext> CTestContext::Get()
	{
		static TPtr<CTestContext> pInstance = TPtr<CTestContext>(new CTestContext);
		return pInstance;
	}

	IEngineCore* CTestContext::GetEngineCore() const
	{
		return mpEngineCore;
	}
}

#endif