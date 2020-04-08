#include "./../../include/editor/CLevelEditorWindow.h"
#include "./../../include/editor/IEditorsManager.h"
#include "./../../include/editor/ISelectionManager.h"
#include "./../../include/core/IImGUIContext.h"
#include "./../../include/core/IInputContext.h"
#include "./../../include/utils/CFileLogger.h"
#include "./../../include/graphics/IDebugUtility.h"
#include "./../../include/ecs/IWorld.h"
#include "./../../include/ecs/CEntity.h"
#include "./../../include/ecs/CTransform.h"
#include "./../../include/graphics/CPerspectiveCamera.h"
#include "./../../include/graphics/COrthoCamera.h"
#include "./../../include/ecs/CCameraSystem.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CLevelEditorWindow::CLevelEditorWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CLevelEditorWindow::Init(IEditorsManager* pEditorsManager, IInputContext* pInputContext, IDebugUtility* pDebugUtility)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pEditorsManager || !pInputContext)
		{
			return RC_INVALID_ARGS;
		}

		mpEditorsManager   = pEditorsManager;
		mpInputContext     = dynamic_cast<IDesktopInputContext*>(pInputContext);
		mpSelectionManager = nullptr;
		mpDebugUtility     = pDebugUtility;
		
		if (!mpInputContext)
		{
			return RC_INVALID_ARGS;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CLevelEditorWindow::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;
		delete this;

		return RC_OK;
	}

	void CLevelEditorWindow::_onDraw()
	{
		bool isEnabled = mIsVisible;

		static const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			TVector2(150.0f, 400.0f),
			TVector2(150.0f, 400.0f),
		};

		if (mpImGUIContext->BeginWindow("Level Editor", isEnabled, params))
		{
		}

		mpImGUIContext->EndWindow();

		mIsVisible = isEnabled;

		if (_onDrawGizmos())
		{
			return; // \note If some of gizmo's axes are selected then skip rest propagation of the input
		}

		_onHandleInput();
	}

	void CLevelEditorWindow::_onHandleInput()
	{
		ISelectionManager* pSelectionManager = _getSelectionManager();

		if (mpInputContext->IsMouseButtonPressed(0))
		{
			TVector3 mousePosition = mpInputContext->GetMousePosition();

			mSelectedEntityId = pSelectionManager->PickObject({ mousePosition.x, mousePosition.y });

			LOG_MESSAGE(CStringUtils::Format("[Level Editor] Picked object id : {0}", mSelectedEntityId));
		}

	}

	bool CLevelEditorWindow::_onDrawGizmos()
	{
		if (mSelectedEntityId == InvalidEntityId)
		{
			return false;
		}

		IWorld* pWorld = mpEditorsManager->GetWorldInstance();
		if (!pWorld)
		{
			return false;
		}

		if (mCameraEntityId == InvalidEntityId)
		{
			mCameraEntityId = pWorld->FindEntitiesWithAny<CPerspectiveCamera, COrthoCamera>().front();
		}

		CEntity* pCameraEntity = pWorld->FindEntity(mCameraEntityId);

		if (auto pSelectedEntity = pWorld->FindEntity(mSelectedEntityId))
		{
			TMatrix4 matrix = pSelectedEntity->GetComponent<CTransform>()->GetTransform();
			
			auto&& ray = NormalizedScreenPointToWorldRay(*GetValidPtrOrDefault<CBaseCamera*>(pCameraEntity->GetComponent<CPerspectiveCamera>(), pCameraEntity->GetComponent<COrthoCamera>()),
														 mpInputContext->GetNormalizedMousePosition());

			TVector3 origin { matrix.m[0][3], matrix.m[1][3], matrix.m[2][3] };

			const TRay3D axes[3]
			{
				{ origin, RightVector3 },
				{ origin, UpVector3 },
				{ origin, ForwardVector3 }
			};

			U8 selectedAxisIndex = -1;

			for (U8 i = 0; i < 3; ++i)
			{
				if (CalcShortestDistanceBetweenLines(axes[i], ray) < 0.1f)
				{
					selectedAxisIndex = i;
				}
			}

			E_GIZMO_TYPE type = selectedAxisIndex >= 0 ? static_cast<E_GIZMO_TYPE>(static_cast<U8>(E_GIZMO_TYPE::TRANSLATION_X) + selectedAxisIndex) : E_GIZMO_TYPE::TRANSLATION;

			// \todo Implement all types of gizmos here
			mpDebugUtility->DrawTransformGizmo(type, matrix);

			return selectedAxisIndex >= 0 && selectedAxisIndex < (std::numeric_limits<U8>::max)();
		}

		return false;
	}

	ISelectionManager* CLevelEditorWindow::_getSelectionManager()
	{
		if (!mpSelectionManager)
		{
			mpSelectionManager = mpEditorsManager->GetSelectionManager();
		}

		return mpSelectionManager;
	}


	TDE2_API IEditorWindow* CreateLevelEditorWindow(IEditorsManager* pEditorsManager, IInputContext* pInputContext, IDebugUtility* pDebugUtility, E_RESULT_CODE& result)
	{
		CLevelEditorWindow* pEditorInstance = new (std::nothrow) CLevelEditorWindow();

		if (!pEditorInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pEditorInstance->Init(pEditorsManager, pInputContext, pDebugUtility);

		if (result != RC_OK)
		{
			delete pEditorInstance;

			pEditorInstance = nullptr;
		}

		return dynamic_cast<IEditorWindow*>(pEditorInstance);
	}
}

#endif