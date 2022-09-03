#include "../../include/autotests/CTestContext.h"


namespace TDEngine2
{
	CTestContext::CTestContext():
		CBaseObject()
	{
	}


	TPtr<CTestContext> CTestContext::Get()
	{
		static TPtr<CTestContext> pInstance = TPtr<CTestContext>(new CTestContext);
		return pInstance;
	}
}