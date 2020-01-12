#include "./../../include/editor/CPerfProfiler.h"
#include "./../../include/platform/win32/CWin32Timer.h"
#include "./../../include/platform/unix/CUnixTimer.h"
#include <memory>


namespace TDEngine2
{
	const U16 CPerfProfiler::mLogBuffer = 256;


	CPerfProfiler::CPerfProfiler() :
		CBaseObject(), mIsRecording(false), mCurrFrameIndex(0), mWorstTimeFrameIndex(0)
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

		mFramesTimesStatistics.resize(mLogBuffer);
		mFramesStatistics.resize(mLogBuffer);
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

	E_RESULT_CODE CPerfProfiler::BeginFrame()
	{
		if (mIsRecording)
		{
			return RC_FAIL;
		}

		mIsRecording = true;
		mFramesTimesStatistics[mCurrFrameIndex] = mpPerformanceTimer->GetCurrTime();

		return RC_OK;
	}

	E_RESULT_CODE CPerfProfiler::EndFrame()
	{
		if (!mIsRecording)
		{
			return RC_FAIL;
		}

		// \note save information about current frame's time
		mFramesTimesStatistics[mCurrFrameIndex] = (mpPerformanceTimer->GetCurrTime() - mFramesTimesStatistics[mCurrFrameIndex]);

		mWorstTimeFrameIndex = (mFramesTimesStatistics[mCurrFrameIndex] > mFramesTimesStatistics[mWorstTimeFrameIndex]) ? mCurrFrameIndex : mWorstTimeFrameIndex;

		mCurrFrameIndex = (mCurrFrameIndex + 1) % mLogBuffer;
		mIsRecording = false;

		return RC_OK;
	}

	void CPerfProfiler::WriteSample(F32 startTime, F32 duration, U32 threadID)
	{
		TDE2_ASSERT(mFramesStatistics.size() > mCurrFrameIndex);
		mFramesStatistics[mCurrFrameIndex][threadID] = { startTime, duration, threadID };
	}

	ITimer* CPerfProfiler::GetTimer() const
	{
		return mpPerformanceTimer;
	}

	const std::vector<F32>& CPerfProfiler::GetFramesTimes() const
	{
		return mFramesTimesStatistics;
	}

	U16 CPerfProfiler::GetWorstFrameIndexByTime() const
	{
		return mWorstTimeFrameIndex;
	}

	TDE2_API IProfiler* CPerfProfiler::Get()
	{
		static IProfiler* pInstance = new (std::nothrow) CPerfProfiler();
		return pInstance;
	}
}