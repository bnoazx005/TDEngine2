#include "../../include/editor/IEditorWindow.h"
#include "../../include/core/IImGUIContext.h"
#include "../../include/editor/CPerfProfiler.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CBaseEditorWindow::CBaseEditorWindow() :
		CBaseObject(), mIsVisible(false)
	{
	}

	void CBaseEditorWindow::Draw(IImGUIContext* pImGUIContext, F32 dt)
	{
		TDE2_PROFILER_SCOPE("CBaseEditorWindow::Draw");
		_onUpdate(dt);

		if (!mIsVisible || !pImGUIContext)
		{
			return;
		}

		mpImGUIContext = pImGUIContext;

		_onDraw();
	}

	void CBaseEditorWindow::SetVisible(bool isVisible)
	{
		mIsVisible = isVisible;
	}

	const std::string& CBaseEditorWindow::GetName() const
	{
		return mName;
	}

	bool CBaseEditorWindow::IsVisible() const
	{
		return mIsVisible;
	}

	void CBaseEditorWindow::_onUpdate(F32 dt)
	{
		TDE2_PROFILER_SCOPE("CBaseEditorWindow::_onUpdate");

		mDeltaTimeAccumulator += dt;
		++mFramesCounter;

		if (mFramesCounter >= mAvgDeltaTimeFramesCount)
		{
			mCurrDeltaTime = mDeltaTimeAccumulator * (1.0f / static_cast<F32>(mFramesCounter));
			
			mDeltaTimeAccumulator = 0.0f;
			mFramesCounter = 0;
		}
	}
}

#endif