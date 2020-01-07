#include "./../../include/editor/CPerfProfiler.h"
#include "./../../include/platform/win32/CWin32Timer.h"
#include "./../../include/platform/unix/CUnixTimer.h"
#include <memory>


namespace TDEngine2
{
	CPerfProfiler::CPerfProfiler() :
		CBaseObject()
	{
		E_RESULT_CODE result = RC_OK;

#if defined(TDE2_USE_WIN32PLATFORM)
		mpPerformanceTimer = CreateWin32Timer(result);
#elif defined(TDE2_USE_UNIXPLATFORM)
		mpPerformanceTimer = CreateUnixTimer(result);
#else
	#error "Undefined platform's been found. Abort"
#endif

		TDE2_ASSERT(mpPerformanceTimer);
		mpPerformanceTimer->Start();
	}

	E_RESULT_CODE CPerfProfiler::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = mpPerformanceTimer->Free();

		delete this;

		mIsInitialized = false;

		return result;
	}

	E_RESULT_CODE CPerfProfiler::PushScope(const std::string& scopeName)
	{
		mScopesContext.push(scopeName);
		return RC_OK;
	}

	E_RESULT_CODE CPerfProfiler::PopScope()
	{
		if (mScopesContext.empty())
		{
			return RC_FAIL;
		}

		mScopesContext.pop();

		return RC_OK;
	}

	void CPerfProfiler::WriteSample(F32 time, U32 threadID)
	{
		mSamplesTable[GetCurrParentScopeName()] = { time, threadID };
	}

	const std::string& CPerfProfiler::GetCurrParentScopeName() const
	{
		return mScopesContext.empty() ? CStringUtils::mEmptyStr : mScopesContext.top();
	}

	ITimer* CPerfProfiler::GetTimer() const
	{
		return mpPerformanceTimer;
	}


	TDE2_API IProfiler* CPerfProfiler::Get()
	{
		static IProfiler* pInstance = new (std::nothrow) CPerfProfiler();
		return pInstance;
	}
}