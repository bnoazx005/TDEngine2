#include "../../include/autotests/CBaseTestFixture.h"
#include "../../include/autotests/ITestCase.h"


namespace TDEngine2
{
	CBaseTestFixture::CBaseTestFixture() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CBaseTestFixture::Init(const std::string& name)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (name.empty())
		{
			return RC_INVALID_ARGS;
		}

		mName = name;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CBaseTestFixture::AddTestCase(const std::string& name, TPtr<ITestCase> pTestCase)
	{
		if (name.empty() || !pTestCase)
		{
			return RC_INVALID_ARGS;
		}

		mTestCases.emplace_back(name, pTestCase);

		return RC_OK;
	}

	void CBaseTestFixture::Update(F32 dt)
	{
		if (mTestCases.empty())
		{
			return;
		}

		TPtr<ITestCase> pCurrTestCase = nullptr;
		std::string currTestCaseName;

		std::tie(currTestCaseName, pCurrTestCase) = mTestCases.front();

		if (!pCurrTestCase->IsFinished())
		{
			pCurrTestCase->Update(dt);
			return;
		}

		mTestCases.erase(mTestCases.cbegin());
	}

	bool CBaseTestFixture::IsFinished() const
	{
		return mTestCases.empty();
	}


	TDE2_API ITestFixture* CreateBaseTestFixture(const std::string& name, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ITestFixture, CBaseTestFixture, result, name);
	}
}