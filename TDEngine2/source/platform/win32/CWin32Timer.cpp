#include "./../../../include/platform/win32/CWin32Timer.h"
#include <memory>


#if defined (TDE2_USE_WIN32PLATFORM)

namespace TDEngine2
{
	CWin32Timer::CWin32Timer():
		CBaseObject()
	{
	}

	E_RESULT_CODE CWin32Timer::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		LARGE_INTEGER frequency { 0 };

		if (!QueryPerformanceFrequency(&frequency))
		{
			return RC_FAIL;
		}

		mFrequency = 1.0f / frequency.QuadPart;

		mDeltaTime = 0.0f;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CWin32Timer::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}


	void CWin32Timer::Start()
	{
		QueryPerformanceCounter(&mStartTime);

		mPrevTime = mStartTime;
		mCurrTime = mPrevTime;
		mStopTime = mStartTime;
	}

	void CWin32Timer::Stop()
	{
		QueryPerformanceCounter(&mStopTime);
	}

	void CWin32Timer::Tick()
	{
		memcpy(&mPrevTime, &mCurrTime, sizeof(LARGE_INTEGER));

		QueryPerformanceCounter(&mCurrTime);

		mDeltaTime = (mCurrTime.QuadPart - mPrevTime.QuadPart) * mFrequency;
		
		mDeltaTime = mDeltaTime > 1.0f ? DefaultFrameRate : mDeltaTime; /// delta's top bound is used to prevent too large values
	}
	
	F32 CWin32Timer::GetDeltaTime() const
	{
		return mDeltaTime;
	}

	F32 CWin32Timer::GetCurrTime() const
	{
		return (mCurrTime.QuadPart - mStartTime.QuadPart) * mFrequency;
	}


	ITimer* CreateWin32Timer(E_RESULT_CODE& result)
	{
		CWin32Timer* pTimer = new (std::nothrow) CWin32Timer();

		if (!pTimer)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pTimer->Init();

		if (result != RC_OK)
		{
			delete pTimer;

			pTimer = nullptr;
		}

		return dynamic_cast<ITimer*>(pTimer);
	}
}

#endif