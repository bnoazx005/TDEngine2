#include "../../include/editor/CProfilerEditorWindow.h"
#include "../../include/core/IImGUIContext.h"
#include "../../include/math/MathUtils.h"
#include "stringUtils.hpp"
#include <stack>
#include <tuple>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CProfilerEditorWindow::CProfilerEditorWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CProfilerEditorWindow::Init(ITimeProfiler* pProfiler)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pProfiler)
		{
			return RC_INVALID_ARGS;
		}

		mpProfiler = pProfiler;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CProfilerEditorWindow::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;
		delete this;

		return RC_OK;
	}

	void CProfilerEditorWindow::SetMainThreadID(U32 mainThreadID)
	{
		mMainThreadID = mainThreadID;
	}

	void CProfilerEditorWindow::_onDraw()
	{
		bool isEnabled = mIsVisible;

		static const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			TVector2(600.0f, 100.0f),
			TVector2(800.0f, 400.0f),
		};

		if (mpImGUIContext->BeginWindow("Profiler", isEnabled, params))
		{
			auto framesStats = mpProfiler->GetFramesTimes();
			
			// \note convert to ms
			for (auto& currFrame : framesStats) 
			{
				currFrame *= 1000.0f;
			}

			U16 maxFrameTimeIndex = mpProfiler->GetWorstFrameIndexByTime();

			mpImGUIContext->Histogram("", framesStats, 0.0f, 16.0f, { mpImGUIContext->GetWindowWidth() - 15.0f, 64.0f }, "Frame Times (ms)");

			if (mpImGUIContext->BeginChildWindow("Frame Tracing Results", TVector2(mpImGUIContext->GetWindowWidth() - 15.0f, 250.0f)))
			{
				auto&& samplesPerThreads = mpProfiler->GetSamplesLogByFrameIndex(0);

				std::stack<std::tuple<F32, F32>> samplesStack;

				for (const auto& threadSamplesEntry : samplesPerThreads)
				{
					auto cursorPos = mpImGUIContext->GetCursorScreenPos();

					_drawRectWithText(*mpImGUIContext, Wrench::StringUtils::Format((mMainThreadID == threadSamplesEntry.first) ? "Main Thread" : "Thread: {0}", threadSamplesEntry.first), 
									  TRectF32{ 0.0f, 0.0f, mpImGUIContext->GetWindowWidth(), mIntervalRectHeight } + cursorPos, 
									  { 0.44f, 0.5f, 0.56f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });

					auto&& samples = threadSamplesEntry.second;
					decltype(threadSamplesEntry.second) samplesQueue { samples.begin(),  samples.end() };

					while (!samplesQueue.empty())
					{
						ITimeProfiler::TSampleRecord currSample = samplesQueue.front();
						samplesQueue.erase(samplesQueue.cbegin());

						_drawIntervalsTree(*mpImGUIContext,  cursorPos, currSample, samplesQueue, (mpImGUIContext->GetWindowWidth() - 15.0f) / framesStats[0], 1);
					}
				}
			}
			mpImGUIContext->EndChildWindow();
		}

		mpImGUIContext->EndWindow();

		mIsVisible = isEnabled;
	}

	TRectF32 CProfilerEditorWindow::_drawRectWithText(IImGUIContext& imguiContext, const std::string& text, const TRectF32& rect, const TColor32F& rectColor,
												  const TColor32F& textColor)
	{
		TVector2 labelSizes = imguiContext.GetTextSizes(text);
		TVector2 labelPos { rect.x + 0.5f * (rect.width - labelSizes.x), rect.y + 0.5f * (rect.height - labelSizes.y) };

		imguiContext.DrawRect(rect, rectColor);

		if (labelSizes.x < rect.width)
		{
			imguiContext.Label(text, labelPos, textColor);
		}

		return rect;
	}

	void CProfilerEditorWindow::_drawIntervalsTree(IImGUIContext& imguiContext, const TVector2& initPosition, const ITimeProfiler::TSampleRecord& currSample,
												   ITimeProfiler::TSamplesArray& samples, F32 pixelsPerMillisecond, I16 currTrackId)
	{
		const F32 scale = 1000.0f * pixelsPerMillisecond;

		TVector2 pos = initPosition + TVector2(scale * currSample.mStartTime, currTrackId * (mIntervalRectHeight + mSpacingSizes.y));

		_drawRectWithText(imguiContext, currSample.mName, TRectF32{ 0.0f, 0.0f, currSample.mDuration * scale, mIntervalRectHeight } + pos,
						  { 1.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });

		auto&& iter = samples.begin();

		while (!samples.empty() && (iter != samples.cend()))
		{
			ITimeProfiler::TSampleRecord sample = *iter;

			if (!CMathUtils::IsInInclusiveRange(1000.0f * currSample.mStartTime, (currSample.mStartTime + currSample.mDuration) * 1000.0f, sample.mStartTime * 1000.0f))
			{
				++iter;
				continue;
			}
			
			iter = samples.erase(iter);

			_drawIntervalsTree(imguiContext, initPosition, sample, samples, pixelsPerMillisecond, currTrackId + 1);
		}
	}


	TDE2_API IEditorWindow* CreateProfilerEditorWindow(ITimeProfiler* pProfiler, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CProfilerEditorWindow, result, pProfiler);
	}
}

#endif