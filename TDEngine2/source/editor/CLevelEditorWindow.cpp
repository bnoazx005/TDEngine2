#include "../../include/editor/CLevelEditorWindow.h"
#include "../../include/editor/IEditorsManager.h"
#include "../../include/editor/ISelectionManager.h"
#include "../../include/editor/CEditorActionsManager.h"
#include "../../include/core/IImGUIContext.h"
#include "../../include/core/IInputContext.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/graphics/IDebugUtility.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/graphics/CPerspectiveCamera.h"
#include "../../include/graphics/COrthoCamera.h"
#include "../../include/ecs/CCameraSystem.h"


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

		if (!pEditorsManager || !pInputContext || !mpInputContext)
		{
			return RC_INVALID_ARGS;
		}

		mpEditorsManager   = pEditorsManager;
		mpInputContext     = dynamic_cast<IDesktopInputContext*>(pInputContext);
		mpSelectionManager = nullptr;
		mpDebugUtility     = pDebugUtility;

		E_RESULT_CODE result = RC_OK;

		if (!(mpActionsHistory = CreateEditorActionsManager(result)) || result != RC_OK)
		{
			return result;
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

		E_RESULT_CODE result = mpActionsHistory->Free();

		mIsInitialized = false;
		delete this;

		return RC_OK;
	}

	E_RESULT_CODE CLevelEditorWindow::ExecuteUndoAction()
	{
		if (!mpActionsHistory)
		{
			return RC_FAIL;
		}

		if (auto actionResult = mpActionsHistory->PopAction())
		{
			return actionResult.Get()->Restore();
		}

		return RC_OK;
	}

	E_RESULT_CODE CLevelEditorWindow::ExecuteRedoAction()
	{
		if (!mpActionsHistory)
		{
			return RC_FAIL;
		}

		if (auto actionResult = mpActionsHistory->PopAction())
		{
			return actionResult.Get()->Execute();
		}

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

		_onDrawInspector();

		if (_onDrawGizmos([this](TEntityId entityId, E_GIZMO_TYPE type, const TVector3& delta)
			{
				if (IWorld* pWorld = mpEditorsManager->GetWorldInstance())
				{
					if (CEntity* pEntity = pWorld->FindEntity(entityId))
					{
						auto pTransform = pEntity->GetComponent<CTransform>();
						
						switch (type)
						{
							case E_GIZMO_TYPE::TRANSLATION_X:
								pTransform->SetPosition(pTransform->GetPosition() + delta.x * pTransform->GetRightVector());
								break;
							case E_GIZMO_TYPE::TRANSLATION_Y:
								pTransform->SetPosition(pTransform->GetPosition() + delta.y * pTransform->GetUpVector());
								break;
							case E_GIZMO_TYPE::TRANSLATION_Z:
								pTransform->SetPosition(pTransform->GetPosition() + delta.z * pTransform->GetForwardVector());
								break;
						}
					}
				}
			}))
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

			LOG_MESSAGE(Wrench::StringUtils::Format("[Level Editor] Picked object id : {0}", ToString<TEntityId>(mSelectedEntityId)));
		}

		E_RESULT_CODE result = RC_OK;

		// \note process shortcuts
		if (mpInputContext->IsKey(E_KEYCODES::KC_LCONTROL))
		{
			if (mpInputContext->IsKeyPressed(E_KEYCODES::KC_Z)) // \note Ctrl+Z
			{
				if (auto pAction = CreateUndoAction(this, result))
				{
					pAction->Execute();
					PANIC_ON_FAILURE(mpActionsHistory->PushAction(pAction));
					mpActionsHistory->Dump();
				}
			}

			if (mpInputContext->IsKeyPressed(E_KEYCODES::KC_Y)) // \note Ctrl+Y
			{
				if (auto pAction = CreateRedoAction(this, result))
				{
					pAction->Execute();
					PANIC_ON_FAILURE(mpActionsHistory->PushAction(pAction));
					mpActionsHistory->Dump();
				}
			}
		}	
	}

	bool CLevelEditorWindow::_onDrawGizmos(const TGizmoManipulatorCallback& onGizmoManipulatorCallback)
	{
		if (mSelectedEntityId == TEntityId::Invalid)
		{
			return false;
		}

		IWorld* pWorld = mpEditorsManager->GetWorldInstance();
		if (!pWorld)
		{
			return false;
		}

		if (mCameraEntityId == TEntityId::Invalid)
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

			F32 currDistance = 0.0f, t1 = 0.0f, t2 = 0.0f;

			if (!mIsGizmoBeingDragged) // \note change selection only if a user doesn't drag gizmo
			{
				for (U8 i = 0; i < 3; ++i)
				{
					std::tie(currDistance, t1, t2) = CalcShortestDistanceBetweenLines(axes[i], ray);
					if (currDistance < 0.1f)
					{
						mCurrSelectedGizmoAxis = i;
					}
				}

				mFirstPosition = axes[mCurrSelectedGizmoAxis](t1);
			}
			else
			{
				std::tie(currDistance, t1, t2) = CalcShortestDistanceBetweenLines(axes[mCurrSelectedGizmoAxis], ray);
			}

			mpDebugUtility->DrawCross(mFirstPosition, 1.5f, { 0.0f, 0.0f, 1.0f, 1.0f });
			mpDebugUtility->DrawCross(mLastPosition, 1.5f, { 0.0f, 0.0f, 1.0f, 1.0f });

			E_GIZMO_TYPE type = (mCurrSelectedGizmoAxis >= 0 && mIsGizmoBeingDragged) ? (E_GIZMO_TYPE::TRANSLATION_X + mCurrSelectedGizmoAxis) : E_GIZMO_TYPE::TRANSLATION;

			// \todo Implement all types of gizmos here
			//mpDebugUtility->DrawTransformGizmo(type, matrix);
			mpDebugUtility->DrawTransformGizmo(E_GIZMO_TYPE::ROTATION, matrix);

			bool prevState = mIsGizmoBeingDragged;

			if (mCurrSelectedGizmoAxis >= 0 && mCurrSelectedGizmoAxis < (std::numeric_limits<U8>::max)())
			{
				if (onGizmoManipulatorCallback && (mIsGizmoBeingDragged = mpInputContext->IsMouseButton(0)))
				{
					mLastPosition = axes[mCurrSelectedGizmoAxis](t1);
					LOG_MESSAGE(mFirstPosition.ToString());
					if (prevState)
					{
						onGizmoManipulatorCallback(mSelectedEntityId, type, mLastPosition - mFirstPosition);
					}

					mFirstPosition = mLastPosition;
				}
				else if (mpInputContext->IsMouseButtonUnpressed(0))
				{
					mIsGizmoBeingDragged = false;
					mCurrSelectedGizmoAxis = -1;
				}

				return true;
			}
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

	void CLevelEditorWindow::_onDrawInspector()
	{
		CEntity* pSelectedEntity = mpEditorsManager->GetWorldInstance()->FindEntity(mSelectedEntityId);
		if (!pSelectedEntity)
		{
			return;
		}

		static bool isEnabled = true;

		static const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			TVector2(250.0f, 400.0f),
			TVector2(250.0f, 400.0f),
		};

		if (mpImGUIContext->BeginWindow("Object Inspector", isEnabled, params))
		{
			mpImGUIContext->Label(ToString<TEntityId>(mSelectedEntityId));

			/*!
				\todo 
				pseudocode for drawing object's properties:

				foreach (component: entity->GetAllComponents()) {
					propertyDrawer = editor->GetPropertyDrawerByID(component->GetID());
					propertyDrawer->Draw(component);
				}
			*/
		}

		mpImGUIContext->EndWindow();
	}


	TDE2_API IEditorWindow* CreateLevelEditorWindow(IEditorsManager* pEditorsManager, IInputContext* pInputContext, IDebugUtility* pDebugUtility, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CLevelEditorWindow, result, pEditorsManager, pInputContext, pDebugUtility);
	}
}

#endif