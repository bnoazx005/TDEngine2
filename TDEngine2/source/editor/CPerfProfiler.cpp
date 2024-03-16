#include "../../include/editor/CPerfProfiler.h"
#include "../../include/platform/win32/CWin32Timer.h"
#include "../../include/platform/unix/CUnixTimer.h"
#include "../../include/math/MathUtils.h"
#include "../../include/editor/CEditorSettings.h"
#include <memory>
#include <algorithm>

namespace TDEngine2
{
	ITimeProfiler::TSampleRecord::TSampleRecord(F32 startTime, F32 duration, USIZE threadID, const std::string& name):
		mStartTime(startTime), mDuration(duration), mThreadID(threadID), mName(name)
	{
	}


	const U16 CPerfProfiler::mLogBuffer = 256;


	CPerfProfiler::CPerfProfiler() :
		CBaseObject(), mIsRecording(false), mCurrFrameIndex(0), mWorstTimeFrameIndex(0)
	{
		E_RESULT_CODE result = RC_OK;

#if defined(TDE2_USE_WINPLATFORM)
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

	E_RESULT_CODE CPerfProfiler::_onFreeInternal()
	{
		return mpPerformanceTimer->Free();
	}

	E_RESULT_CODE CPerfProfiler::BeginFrame()
	{
		if (mIsRecording)
		{
			return RC_FAIL;
		}

		mIsRecording = true;
		mFramesTimesStatistics[mCurrFrameIndex] = mpPerformanceTimer->GetCurrTime();

		for (auto& currSample : mFramesStatistics[mCurrFrameIndex])
		{
			currSample.second.clear();
		}

		++mCurrFrameCounter;

		if (mCurrFrameCounter >= mMeasurementsFramesCount)
		{
			const F32 invFramesCount = 1.0f / static_cast<F32>(mCurrFrameCounter);

			for (USIZE i = 0; i < mAverageSpecialEventsMeasurements.size(); i++)
			{
				mAverageSpecialEventsMeasurements[i] = invFramesCount * mSpecialEventsAccumulators[i];
				mSpecialEventsAccumulators[i] = 0.0f;
			}

			mCurrFrameCounter = 0;
		}

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

	void CPerfProfiler::WriteSample(const std::string& name, F32 startTime, F32 duration, USIZE threadID)
	{
		TDE2_ASSERT(mFramesStatistics.size() > mCurrFrameIndex);

		auto&& currSamplesLog = mFramesStatistics[mCurrFrameIndex][threadID];
		auto&& insertIter = std::lower_bound(currSamplesLog.begin(), currSamplesLog.end(), 0.0f, [startTime, duration](const TSampleRecord& sample, auto&&)
		{
			return startTime > sample.mStartTime && duration < sample.mDuration;
		});

		currSamplesLog.insert(insertIter, { startTime - mFramesTimesStatistics[mCurrFrameIndex], duration, threadID, name });
	}

	void CPerfProfiler::WriteSpecialSample(E_SPECIAL_PROFILE_EVENT eventName, F32 duration)
	{
		mSpecialEventsAccumulators[static_cast<U32>(eventName)] += duration;
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

	const CPerfProfiler::TSamplesTable& CPerfProfiler::GetSamplesLogByFrameIndex(U32 frameIndex) const
	{
		TDE2_ASSERT(frameIndex < mFramesStatistics.size());
		return mFramesStatistics[frameIndex];
	}

	F32 CPerfProfiler::GetAverageTimeByEventName(E_SPECIAL_PROFILE_EVENT eventName) const
	{
		return mAverageSpecialEventsMeasurements[static_cast<U32>(eventName)];
	}


	TDE2_API TPtr<ITimeProfiler> CPerfProfiler::Get()
	{
		static TPtr<ITimeProfiler> pInstance = TPtr<ITimeProfiler>(new (std::nothrow) CPerfProfiler());
		return pInstance;
	}


	bool IsProfileStartupEnabled()
	{
#if TDE2_EDITORS_ENABLED
		return CEditorSettings::Get()->mProfilerSettings.mIsCollectStartupProfileDataEnabled;
#else
		return false;
#endif
	}
}