#include "../../include/autotests/CTestContext.h"


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

	E_RESULT_CODE CTestContext::Assert(bool actual, bool expected)
	{
		

		return RC_OK;
	}

	E_RESULT_CODE CTestContext::RunAllTests()
	{
		return RC_OK;
	}


	TPtr<CTestContext> CTestContext::Get()
	{
		static TPtr<CTestContext> pInstance = TPtr<CTestContext>(new CTestContext);
		return pInstance;
	}
}

#endif