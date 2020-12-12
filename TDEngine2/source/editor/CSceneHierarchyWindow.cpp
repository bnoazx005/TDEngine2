#include "../../include/editor/CSceneHierarchyWindow.h"
#include "../../include/core/IImGUIContext.h"
#include "../../include/scene/ISceneManager.h"
#include "../../include/scene/IScene.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/editor/CSelectionManager.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CSceneHierarchyEditorWindow::CSceneHierarchyEditorWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CSceneHierarchyEditorWindow::Init(ISceneManager* pSceneManager, ISelectionManager* pSelectionManager)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pSceneManager || !pSelectionManager)
		{
			return RC_INVALID_ARGS;
		}

		mpSceneManager = pSceneManager;
		mpSelectionManager = pSelectionManager;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CSceneHierarchyEditorWindow::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;
		delete this;

		return RC_OK;
	}

	void CSceneHierarchyEditorWindow::_onDraw()
	{
		bool isEnabled = mIsVisible;

		static const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			TVector2(200.0f, 300.0f),
			TVector2(500.0f, 500.0f),
		};

		if (mpImGUIContext->BeginWindow("Scene Hierarchy", isEnabled, params))
		{
			for (IScene* pCurrScene : mpSceneManager->GetLoadedScenes())
			{
				if (mpImGUIContext->CollapsingHeader(pCurrScene->GetName(), true))
				{
					pCurrScene->ForEachEntity([this](const CEntity* pEntity)
					{
						if (mpImGUIContext->SelectableItem(pEntity->GetName()))
						{
							mpSelectionManager->SetSelectedEntity(pEntity->GetId());
						}
					});
				}
			}
			
			mpImGUIContext->EndWindow();
		}

		mIsVisible = isEnabled;
	}


	TDE2_API IEditorWindow* CreateSceneHierarchyEditorWindow(ISceneManager* pSceneManager, ISelectionManager* pSelectionManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CSceneHierarchyEditorWindow, result, pSceneManager, pSelectionManager);
	}
}

#endif