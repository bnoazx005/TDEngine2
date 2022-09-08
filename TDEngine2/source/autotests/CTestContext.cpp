#include "../../include/autotests/CTestContext.h"
#include "../../include/autotests/ITestFixture.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
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

		if (!config.mpFileSystem)
		{
			return RC_INVALID_ARGS;
		}

		mpFileSystem = config.mpFileSystem;

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

	void CTestContext::AddTestResult(const std::string& testFixtureName, const std::string& testCaseName, bool hasPassed)
	{
		++mTotalTestsCount;

		mTestResults[testFixtureName][testCaseName] = hasPassed;

		if (hasPassed)
		{
			++mPassedTestsCount;
		}
		else
		{
			++mFailedTestsCount;
		}
	}

	E_RESULT_CODE CTestContext::Assert(bool actual, bool expected)
	{
		if (actual != expected)
		{
			throw std::exception("[CTestContext] Assertion failed");
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


	TPtr<CTestContext> CTestContext::Get()
	{
		static TPtr<CTestContext> pInstance = TPtr<CTestContext>(new CTestContext);
		return pInstance;
	}
}

#endif