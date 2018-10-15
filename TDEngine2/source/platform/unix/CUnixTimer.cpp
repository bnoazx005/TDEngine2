#include "./../../../include/platform/unix/CUnixTimer.h"
#include <memory>
#include <cstring>


#if defined (TDE2_USE_UNIXPLATFORM)

namespace TDEngine2
{
	CUnixTimer::CUnixTimer() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CUnixTimer::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}
		
		mDeltaTime = 0.0f;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CUnixTimer::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}


	void CUnixTimer::Start()
	{
		gettimeofday(&mStartTime, NULL);
	}

	void CUnixTimer::Stop()
	{
		gettimeofday(&mStopTime, NULL);
	}

	void CUnixTimer::Tick()
	{
		memcpy(&mPrevTime, &mCurrTime, sizeof(mCurrTime));

		gettimeofday(&mCurrTime, NULL);

		mDeltaTime = (mCurrTime.tv_sec - mPrevTime.tv_sec) * 1000.0f + (mCurrTime.tv_usec - mPrevTime.tv_usec) / 1000.0f;
	}

	F32 CUnixTimer::GetDeltaTime() const
	{
		return mDeltaTime;
	}


	ITimer* CreateUnixTimer(E_RESULT_CODE& result)
	{
		CUnixTimer* pTimer = new (std::nothrow) CUnixTimer();

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