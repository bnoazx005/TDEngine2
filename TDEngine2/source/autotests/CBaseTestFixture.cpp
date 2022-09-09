#include "../../include/autotests/CBaseTestFixture.h"
#include "../../include/autotests/ITestCase.h"
#include "../../include/autotests/CTestContext.h"
#include "stringUtils.hpp"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CAssertException::CAssertException(const std::string& message, const std::string& filename, U32 line):
		std::exception(message.c_str()), mMessage(message), mFilename(filename), mLine(line)
	{
	}


	/*!
		CBaseTestFixture's definition
	*/

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

	void CBaseTestFixture::SetOnSetUpAction(const TActionCallback& action)
	{
		mSetUpCallback = action;
	}

	void CBaseTestFixture::SetOnTearDownAction(const TActionCallback& action)
	{
		mTearDownCallback = action;
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

		try
		{
			if (mSetUpCallback && !pCurrTestCase->IsStarted())
			{
				mSetUpCallback();
			}

			if (!pCurrTestCase->IsFinished())
			{
				pCurrTestCase->Update(dt);
				return;
			}

			if (mTearDownCallback)
			{
				mTearDownCallback();
			}
		}
		catch (const CAssertException& e)
		{
			if (mTearDownCallback)
			{
				mTearDownCallback();
			}

			mTestCases.erase(mTestCases.cbegin());

			CTestContext::Get()->AddTestResult(mName, currTestCaseName, { e.mMessage, e.mFilename, e.mLine, false });

			return;
		}

		CTestContext::Get()->AddTestResult(mName, currTestCaseName, { Wrench::StringUtils::GetEmptyStr(), Wrench::StringUtils::GetEmptyStr(), 0, true });
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

#endif