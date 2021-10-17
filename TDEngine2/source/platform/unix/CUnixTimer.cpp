#include "../../../include/platform/unix/CUnixTimer.h"
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

		mDeltaTime = (mCurrTime.tv_sec - mPrevTime.tv_sec) + (mCurrTime.tv_usec - mPrevTime.tv_usec) / 1000000.0f;

		mDeltaTime = mDeltaTime > 1.0f ? DefaultFrameRate : mDeltaTime; /// delta's top bound is used to prevent too large values
	}

	F32 CUnixTimer::GetDeltaTime() const
	{
		return mDeltaTime;
	}

	F32 CUnixTimer::GetCurrTime() const
	{
		return (mCurrTime.tv_sec - mStartTime.tv_sec) + (mCurrTime.tv_usec - mStartTime.tv_usec) / 1000000.0f;
	}


	ITimer* CreateUnixTimer(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ITimer, CUnixTimer, result);
	}
}

#endif