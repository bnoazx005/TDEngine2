#include "../../include/editor/CProfilerEditorWindow.h"
#include "../../include/editor/CMemoryProfiler.h"
#include "../../include/core/IImGUIContext.h"
#include "../../include/math/MathUtils.h"
#include <stringUtils.hpp>
#include <stack>
#include <tuple>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	static TRectF32 DrawRectWithText(IImGUIContext& imguiContext, const std::string& text, const TRectF32& rect, const TColor32F& rectColor, const TColor32F& textColor)
	{
		TVector2 labelSizes = imguiContext.GetTextSizes(text);
		TVector2 labelPos{ rect.x + 0.5f * (rect.width - labelSizes.x), rect.y + 0.5f * (rect.height - labelSizes.y) };

		imguiContext.DrawRect(rect, rectColor);

		if (labelSizes.x < rect.width)
		{
			imguiContext.Label(text, labelPos, textColor);
		}

		return rect;
	}



	CTimeProfilerEditorWindow::CTimeProfilerEditorWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CTimeProfilerEditorWindow::Init(TPtr<ITimeProfiler> pProfiler)
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

	void CTimeProfilerEditorWindow::SetMainThreadID(USIZE mainThreadID)
	{
		mMainThreadID = mainThreadID;
	}


	static bool DrawIntervalsTree(IImGUIContext& imguiContext, const TVector2& initPosition, const ITimeProfiler::TSampleRecord& currSample, 
								ITimeProfiler::TSamplesArray& samples, F32 pixelsPerMillisecond, I16 currTrackId, F32 verticalSpacingSize, F32 intervalHeight)
	{
		const F32 scale = 1000.0f * pixelsPerMillisecond;

		TVector2 pos = initPosition + TVector2(scale * currSample.mStartTime, currTrackId * (intervalHeight + verticalSpacingSize));
		const TRectF32 sampleRect = TRectF32{ 0.0f, 0.0f, currSample.mDuration * scale, intervalHeight } +pos;

		DrawRectWithText(imguiContext, currSample.mName, sampleRect, { 1.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });

		{
			const TVector2 prevPosition = imguiContext.GetCursorScreenPos();
			imguiContext.SetCursorScreenPos(pos);

			if (sampleRect.width > 0.0f && sampleRect.height > 0.0f)
			{
				imguiContext.Button(Wrench::StringUtils::Format("##{0}", currSample.mName), sampleRect.GetSizes(), nullptr, true);
				imguiContext.Tooltip(Wrench::StringUtils::Format("{0} : {1} ms", currSample.mName, currSample.mDuration * 1000.0f));
			}

			imguiContext.SetCursorScreenPos(prevPosition);
		}

		auto iter = samples.begin();

		while (!samples.empty() && (iter != samples.end()))
		{
			ITimeProfiler::TSampleRecord sample = *iter;

			if (!CMathUtils::IsInInclusiveRange(1000.0f * currSample.mStartTime, (currSample.mStartTime + currSample.mDuration) * 1000.0f, sample.mStartTime * 1000.0f))
			{
				++iter;
				continue;
			}

			iter = samples.erase(iter);

			if (DrawIntervalsTree(imguiContext, initPosition, sample, samples, pixelsPerMillisecond, currTrackId + 1, verticalSpacingSize, intervalHeight))
			{
				return true;
			}
		}

		return false;
	}


	void CTimeProfilerEditorWindow::_onDraw()
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

					DrawRectWithText(*mpImGUIContext, Wrench::StringUtils::Format((mMainThreadID == threadSamplesEntry.first) ? "Main Thread" : "Thread: {0}", threadSamplesEntry.first), 
									  TRectF32{ 0.0f, 0.0f, mpImGUIContext->GetWindowWidth(), mIntervalRectHeight } + cursorPos, 
									  { 0.44f, 0.5f, 0.56f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });

					auto&& samples = threadSamplesEntry.second;
					decltype(threadSamplesEntry.second) samplesQueue { samples.begin(),  samples.end() };

					while (!samplesQueue.empty())
					{
						ITimeProfiler::TSampleRecord currSample = samplesQueue.front();
						samplesQueue.erase(samplesQueue.cbegin());

						DrawIntervalsTree(*mpImGUIContext,  cursorPos, currSample, samplesQueue, (mpImGUIContext->GetWindowWidth() - 15.0f) / framesStats[0], 1, mSpacingSizes.y, mIntervalRectHeight);
					}
				}
			}
			mpImGUIContext->EndChildWindow();
		}

		mpImGUIContext->EndWindow();

		mIsVisible = isEnabled;
	}


	TDE2_API IEditorWindow* CreateTimeProfilerEditorWindow(TPtr<ITimeProfiler> pProfiler, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CTimeProfilerEditorWindow, result, pProfiler);
	}



	/*!
		\brief CMemoryProfilerEditorWindow's definiton
	*/

	CMemoryProfilerEditorWindow::CMemoryProfilerEditorWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CMemoryProfilerEditorWindow::Init(TPtr<IMemoryProfiler> pProfiler)
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

		for (U16 i = 0; i < 25; ++i)
		{
			mPalette.emplace_back(RandColor32F());
		}

		mIsInitialized = true;

		return RC_OK;
	}

	void CMemoryProfilerEditorWindow::_onDraw()
	{
		bool isEnabled = mIsVisible;

		static const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			TVector2(600.0f, 100.0f),
			TVector2(800.0f, 400.0f),
		};

		if (mpImGUIContext->BeginWindow("Memory Profiler", isEnabled, params))
		{
			mpImGUIContext->Label("Memory Distribution (preallocated memory block):");

			if (mpImGUIContext->BeginChildWindow("Memory Distribution Results", TVector2(mpImGUIContext->GetWindowWidth() - 15.0f, 150.0f)))
			{
				const TColor32F textColor { 1.0f };

				const F32 windowWidth = mpImGUIContext->GetWindowWidth();
				const U32 totalMemory = static_cast<U32>(CMemoryProfiler::Get()->GetTotalMemoryAvailable());
				const F32 scale = windowWidth / static_cast<F32>(totalMemory > 0 ? totalMemory : 1);

				auto cursorPos = mpImGUIContext->GetCursorScreenPos();
				
				DrawRectWithText(*mpImGUIContext, Wrench::StringUtils::Format("Free space: {0} MiB", totalMemory / (1 << 20)),
								 TRectF32{ 0.0f, 0.0f, windowWidth, 15.0f } + cursorPos, 
								 TColor32F(0.5f), textColor);
				
				auto&& memoryStats = CMemoryProfiler::Get()->GetStatistics();

				auto colorIter = mPalette.cbegin();

				for (auto&& currMemoryBlock : memoryStats)
				{
					const F32 offset = currMemoryBlock.second.mOffset * scale;
					const F32 currSize = currMemoryBlock.second.mCurrOccupiedSize * scale;
					const F32 totalSize = currMemoryBlock.second.mTotalSize * scale;

					const std::string infoText = Wrench::StringUtils::Format("{0} / {1} MiB", currMemoryBlock.second.mCurrOccupiedSize / (1 << 20), currMemoryBlock.second.mTotalSize / (1 << 20));

					DrawRectWithText(*mpImGUIContext, currMemoryBlock.first, TRectF32{ offset, 0.0f, totalSize, 15.0f } + cursorPos, *colorIter++, textColor); // total block's size
					DrawRectWithText(*mpImGUIContext, infoText, TRectF32{ offset, 0.0f, currSize, 15.0f } + cursorPos, *colorIter++, textColor); // occupied memory
				}

				mpImGUIContext->EndChildWindow();
			}

			mpImGUIContext->EndWindow();
		}

		mIsVisible = isEnabled;
	}


	TDE2_API IEditorWindow* CreateMemoryProfilerEditorWindow(TPtr<IMemoryProfiler> pProfiler, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CMemoryProfilerEditorWindow, result, pProfiler);
	}
}

#endif