#include "../../include/editor/IEditorWindow.h"
#include "../../include/core/IImGUIContext.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CBaseEditorWindow::CBaseEditorWindow() :
		CBaseObject(), mIsVisible(false)
	{
	}

	void CBaseEditorWindow::Draw(IImGUIContext* pImGUIContext, F32 dt)
	{
		if (!mIsVisible || !pImGUIContext)
		{
			return;
		}

		mpImGUIContext = pImGUIContext;

		_onUpdate(dt);
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
	}
}

#endif