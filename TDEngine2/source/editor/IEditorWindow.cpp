#include "./../../include/editor/IEditorWindow.h"
#include "./../../include/core/IImGUIContext.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CBaseEditorWindow::CBaseEditorWindow() :
		CBaseObject(), mIsVisible(false)
	{
	}

	void CBaseEditorWindow::Draw(IImGUIContext* pImGUIContext)
	{
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
}

#endif