#include "../../include/autotests/CBaseTestCase.h"


namespace TDEngine2
{
	CBaseTestCase::CBaseTestCase() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CBaseTestCase::Init()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	void CBaseTestCase::ExecuteAction(const std::function<void()>& action)
	{

	}

	void CBaseTestCase::Wait(F32 delay)
	{

	}


	TDE2_API ITestCase* CreateBaseTestCase(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ITestCase, CBaseTestCase, result);
	}
}