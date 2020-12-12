#include "../../include/editor/CLevelEditorWindow.h"
#include "../../include/editor/IEditorsManager.h"
#include "../../include/editor/ISelectionManager.h"
#include "../../include/editor/CEditorActionsManager.h"
#include "../../include/editor/CSceneHierarchyWindow.h"
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

	E_RESULT_CODE CLevelEditorWindow::Init(IEditorsManager* pEditorsManager, IInputContext* pInputContext, IDebugUtility* pDebugUtility, ISceneManager* pSceneManager)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pEditorsManager || !pInputContext || !pDebugUtility)
		{
			return RC_INVALID_ARGS;
		}

		mpEditorsManager   = pEditorsManager;
		mpInputContext     = dynamic_cast<IDesktopInputContext*>(pInputContext);
		mpSelectionManager = nullptr;
		mpDebugUtility     = pDebugUtility;

		mCurrManipulatorType = E_GIZMO_TYPE::TRANSLATION;

		E_RESULT_CODE result = RC_OK;

		if (!(mpActionsHistory = CreateEditorActionsManager(result)) || result != RC_OK)
		{
			return result;
		}

		if (!(mpHierarchyWidget = CreateSceneHierarchyEditorWindow(pSceneManager, _getSelectionManager(), result)) || result != RC_OK)
		{
			return result;
		}

		if (RC_OK != (result != mpEditorsManager->RegisterEditor("Scene Hierarchy", mpHierarchyWidget, true)))
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

	E_RESULT_CODE CLevelEditorWindow::RegisterInspector(TypeId targetType, const TOnGuiCallback& onGuiFunctor)
	{
		if (!onGuiFunctor || (TypeId::Invalid == targetType))
		{
			return RC_INVALID_ARGS;
		}

		auto iter = mInspectorsDrawers.find(targetType);
		if (iter != mInspectorsDrawers.cend())
		{
			return RC_FAIL;
		}

		mInspectorsDrawers[targetType] = onGuiFunctor;

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
			mpImGUIContext->Button("Show Hierarchy", TVector2(mpImGUIContext->GetWindowWidth() - 15.0f, 25.0f), [this] 
			{
				mpHierarchyWidget->SetVisible(!mpHierarchyWidget->IsVisible());
			});
		}

		mpImGUIContext->EndWindow();

		mIsVisible = isEnabled;

		_onDrawInspector();
		if (_onDrawGizmos())
		{
			return; // \note If some of gizmo's axes are selected then skip rest propagation of the input
		}

		_onHandleInput();
	}

	void CLevelEditorWindow::_onHandleInput()
	{
		ISelectionManager* pSelectionManager = _getSelectionManager();

		mSelectedEntityId = pSelectionManager->GetSelectedEntityId();

		if (mpInputContext->IsMouseButtonPressed(0) && !mpImGUIContext->IsMouseOverUI())
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

		// \note Process changes of current manipulator's type
		if (mpInputContext->IsKey(E_KEYCODES::KC_LALT))
		{
			if (mpInputContext->IsKeyPressed(E_KEYCODES::KC_W))
			{
				mCurrManipulatorType = E_GIZMO_TYPE::TRANSLATION;
			}
			if (mpInputContext->IsKeyPressed(E_KEYCODES::KC_E))
			{
				mCurrManipulatorType = E_GIZMO_TYPE::ROTATION;
			}
			if (mpInputContext->IsKeyPressed(E_KEYCODES::KC_R))
			{
				mCurrManipulatorType = E_GIZMO_TYPE::SCALING;
			}
		}		
	}

	bool CLevelEditorWindow::_onDrawGizmos()
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

		if (auto pSelectedEntity = pWorld->FindEntity(mSelectedEntityId))
		{
			TMatrix4 matrix = Transpose(pSelectedEntity->GetComponent<CTransform>()->GetTransform());
			
			auto&& camera = _getCameraEntity();

			mpImGUIContext->DrawGizmo(mCurrManipulatorType, Transpose(camera.GetViewMatrix()), Transpose(camera.GetProjMatrix()), matrix, 
				[pSelectedEntity, this](const TVector3& pos, const TQuaternion& rot, const TVector3& scale)
			{
				if (auto pTransform = pSelectedEntity->GetComponent<CTransform>())
				{
					switch (mCurrManipulatorType)
					{
						case E_GIZMO_TYPE::TRANSLATION:
							pTransform->SetPosition(pos);
							break;
						case E_GIZMO_TYPE::ROTATION:
							pTransform->SetRotation(rot);
							break;
						case E_GIZMO_TYPE::SCALING:
							pTransform->SetScale(scale);
							break;
					}
				}
			});
			
#if 0
			auto&& ray = NormalizedScreenPointToWorldRay(_getCameraEntity(), mpInputContext->GetNormalizedMousePosition());

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
				mCurrSelectedGizmoAxis = -1;

				for (U8 i = 0; i < 3; ++i)
				{
					std::tie(currDistance, t1, t2) = CalcShortestDistanceBetweenLines(axes[i], ray);
					if (currDistance < 0.1f)
					{
						mCurrSelectedGizmoAxis = i;
					}
				}

				if (mCurrSelectedGizmoAxis >= 0)
				{
					mFirstPosition = axes[mCurrSelectedGizmoAxis](t1);
				}
			}
			else
			{
				std::tie(currDistance, t1, t2) = CalcShortestDistanceBetweenLines(axes[mCurrSelectedGizmoAxis], ray);
			}

			mpDebugUtility->DrawCross(mFirstPosition, 1.5f, { 1.0f, 0.0f, 1.0f, 1.0f });
			mpDebugUtility->DrawCross(mLastPosition, 1.5f, { 0.0f, 0.0f, 1.0f, 1.0f });

			E_GIZMO_TYPE type = (mCurrSelectedGizmoAxis >= 0 && mIsGizmoBeingDragged) ? (E_GIZMO_TYPE::TRANSLATION_X + mCurrSelectedGizmoAxis) : E_GIZMO_TYPE::TRANSLATION;

			// \todo Implement all types of gizmos here
			mpDebugUtility->DrawTransformGizmo(type, matrix);

			bool prevState = mIsGizmoBeingDragged;

			if (mCurrSelectedGizmoAxis >= 0 && mCurrSelectedGizmoAxis < (std::numeric_limits<U8>::max)())
			{
				if (onGizmoManipulatorCallback && (mIsGizmoBeingDragged = mpInputContext->IsMouseButton(0)))
				{
					mLastPosition = axes[mCurrSelectedGizmoAxis](t1);

					if (prevState)
					{
						onGizmoManipulatorCallback(mSelectedEntityId, E_GIZMO_EVENT_TYPE::DRAGGED, type, mLastPosition - mFirstPosition);
					}
					else
					{
						onGizmoManipulatorCallback(mSelectedEntityId, E_GIZMO_EVENT_TYPE::STARTED, type, ZeroVector3);
					}

					mFirstPosition = mLastPosition;
				}
				else if (mpInputContext->IsMouseButtonUnpressed(0))
				{
					onGizmoManipulatorCallback(mSelectedEntityId, E_GIZMO_EVENT_TYPE::FINISHED, type, ZeroVector3);

					mIsGizmoBeingDragged = false;
					mCurrSelectedGizmoAxis = -1;
				}

				return true;
			}
#endif
		}

		return false;
	}

	ICamera& CLevelEditorWindow::_getCameraEntity()
	{
		IWorld* pWorld = mpEditorsManager->GetWorldInstance();

		if (mCameraEntityId == TEntityId::Invalid)
		{
			mCameraEntityId = pWorld->FindEntitiesWithAny<CPerspectiveCamera, COrthoCamera>().front();
		}

		CEntity* pCameraEntity = pWorld->FindEntity(mCameraEntityId);

		return *GetValidPtrOrDefault<CBaseCamera*>(pCameraEntity->GetComponent<CPerspectiveCamera>(), pCameraEntity->GetComponent<COrthoCamera>());
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
			TVector2(290.0f, 400.0f),
			TVector2(290.0f, 400.0f),
		};

		if (mpImGUIContext->BeginWindow("Object Inspector", isEnabled, params))
		{
			mpImGUIContext->Label(ToString<TEntityId>(mSelectedEntityId));

			for (IComponent* pCurrComponent : pSelectedEntity->GetComponents())
			{
				if (!pCurrComponent)
				{
					continue;
				}

				auto iter = mInspectorsDrawers.find(pCurrComponent->GetComponentTypeId());
				if (iter == mInspectorsDrawers.cend())
				{
					LOG_WARNING(Wrench::StringUtils::Format("[Level Editor Window] There is no defined inspector's drawer for \"TypeId\": {0}", static_cast<U32>(pCurrComponent->GetComponentTypeId())));

					continue;
				}

				(iter->second)(*mpImGUIContext, *pCurrComponent);
			}
		}

		mpImGUIContext->EndWindow();
	}


	TDE2_API IEditorWindow* CreateLevelEditorWindow(IEditorsManager* pEditorsManager, IInputContext* pInputContext, IDebugUtility* pDebugUtility, ISceneManager* pSceneManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CLevelEditorWindow, result, pEditorsManager, pInputContext, pDebugUtility, pSceneManager);
	}
}

#endif