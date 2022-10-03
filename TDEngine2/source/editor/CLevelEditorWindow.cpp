#include "../../include/editor/CLevelEditorWindow.h"
#include "../../include/editor/IEditorsManager.h"
#include "../../include/editor/ISelectionManager.h"
#include "../../include/editor/CEditorActionsManager.h"
#include "../../include/editor/CSceneHierarchyWindow.h"
#include "../../include/editor/Inspectors.h"
#include "../../include/core/IImGUIContext.h"
#include "../../include/core/IInputContext.h"
#include "../../include/core/IWindowSystem.h"
#include "../../include/utils/CFileLogger.h"
#include "../../include/graphics/IDebugUtility.h"
#include "../../include/ecs/CWorld.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/graphics/CPerspectiveCamera.h"
#include "../../include/graphics/COrthoCamera.h"
#include "../../include/ecs/CCameraSystem.h"
#include "../../include/scene/IScene.h"
#include "../../include/scene/ISceneManager.h"


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CLevelEditorWindow::CLevelEditorWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CLevelEditorWindow::Init(const TLevelEditorWindowDesc& desc)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!desc.mpEditorsManager || !desc.mpInputContext || !desc.mpDebugUtility)
		{
			return RC_INVALID_ARGS;
		}

		mpEditorsManager   = desc.mpEditorsManager;
		mpInputContext     = dynamic_cast<IDesktopInputContext*>(desc.mpInputContext);
		mpSelectionManager = nullptr;
		mpDebugUtility     = desc.mpDebugUtility;
		mpWindowSystem     = desc.mpWindowSystem;
		mpSceneManager     = desc.mpSceneManager;

		mCurrManipulatorType = E_GIZMO_TYPE::TRANSLATION;

		E_RESULT_CODE result = RC_OK;

		if (!(mpActionsHistory = CreateEditorActionsManager(result)) || result != RC_OK)
		{
			return result;
		}

		if (!(mpHierarchyWidget = CreateSceneHierarchyEditorWindow(
			{
				desc.mpSceneManager, 
				desc.mpWindowSystem, 
				_getSelectionManager(),
				mpInputContext,
				desc.mpFileSystem
			}, result)) || result != RC_OK)
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

	E_RESULT_CODE CLevelEditorWindow::_onFreeInternal()
	{
		return mpActionsHistory->Free();
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
			const TVector2 buttonSizes(mpImGUIContext->GetWindowWidth() - 15.0f, 25.0f);

			mpImGUIContext->Button("Show Hierarchy", buttonSizes, [this]
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

		if (mpInputContext->IsMouseButtonUnpressed(0))
		{
			mShouldRecordHistory = true;
		}

		E_RESULT_CODE result = RC_OK;

		// \note process shortcuts
		if (mpInputContext->IsKey(E_KEYCODES::KC_LCONTROL))
		{
			if (mpInputContext->IsKeyPressed(E_KEYCODES::KC_Z)) // \note Ctrl+Z
			{
				mpActionsHistory->ExecuteUndo();
				mpActionsHistory->Dump();
			}

			if (mpInputContext->IsKeyPressed(E_KEYCODES::KC_Y)) // \note Ctrl+Y
			{
				mpActionsHistory->ExecuteRedo();
				mpActionsHistory->Dump();
			}
		}	

		// \note Process changes of current manipulator's type
		if (mpInputContext->IsKey(E_KEYCODES::KC_LALT))
		{
			if (mpInputContext->IsKeyPressed(E_KEYCODES::KC_Q))
			{
				mCurrManipulatorType = E_GIZMO_TYPE::TRANSLATION;
			}
			if (mpInputContext->IsKeyPressed(E_KEYCODES::KC_W))
			{
				mCurrManipulatorType = E_GIZMO_TYPE::ROTATION;
			}
			if (mpInputContext->IsKeyPressed(E_KEYCODES::KC_E))
			{
				mCurrManipulatorType = E_GIZMO_TYPE::SCALING;
			}
		}		
	}


	static TVector3 GetAveragePoint(TPtr<ISelectionManager> pSelectionManager, TPtr<IWorld> pWorld)
	{
		TVector3 result = ZeroVector3;

		auto&& selectedEntities = pSelectionManager->GetSelectedEntities();

		for (auto&& selectedEntityId : selectedEntities)
		{
			auto pSelectedEntity = pWorld->FindEntity(selectedEntityId);
			if (!pSelectedEntity)
			{
				continue;
			}

			auto pTransform = pSelectedEntity->GetComponent<CTransform>();
			result = result + pTransform->GetPosition();
		}

		return result * static_cast<F32>(1.0f / static_cast<F32>(selectedEntities.size()));
	}


	static void DrawGizmoForSingleSelectedEntity(E_GIZMO_TYPE manipulatorType, TPtr<ISelectionManager> pSelectionManager, ISceneManager* pSceneManager, 
												 IEditorActionsHistory* pActionsHistory, IImGUIContext* pImGUIContext, bool& shouldRecordHistory)
	{
		auto pWorld = pSceneManager->GetWorld();

		const TEntityId selectedEntity = pSelectionManager->GetSelectedEntityId();

		if (auto pSelectedEntity = pWorld->FindEntity(selectedEntity))
		{
			auto pSelectedTransform = pSelectedEntity->GetComponent<CTransform>();

			TMatrix4 matrix = Transpose(pSelectedTransform->GetLocalToWorldTransform());

			auto&& pCamera = GetCurrentActiveCamera(pWorld.Get());

			pImGUIContext->DrawGizmo(manipulatorType, Transpose(pCamera->GetViewMatrix()), Transpose(pCamera->GetProjMatrix()), matrix,
				[=, &shouldRecordHistory, pWorldPtr = pWorld.Get()](const TVector3& pos, const TQuaternion& rot, const TVector3& scale)
			{
				TVector3 localPos    = pos;
				TVector3 localScale  = scale;
				TQuaternion localRot = Normalize(rot);

				if (CEntity* pParentEntity = pWorldPtr->FindEntity(pSelectedTransform->GetParent()))
				{
					CTransform* pParentTransform = pParentEntity->GetComponent<CTransform>();

					const TMatrix4& world2Local = pParentTransform->GetWorldToLocalTransform();

					localPos = (E_GIZMO_TYPE::TRANSLATION == manipulatorType) ? world2Local * localPos : pSelectedTransform->GetPosition();
					localRot = (E_GIZMO_TYPE::ROTATION == manipulatorType) ? localRot : pSelectedTransform->GetRotation();
					localScale = (E_GIZMO_TYPE::SCALING == manipulatorType) ? world2Local * TVector4(localScale, 0.0f) : pSelectedTransform->GetScale();
				}

				if (shouldRecordHistory)
				{
					E_RESULT_CODE result = RC_OK;

					if (auto pAction = CreateTransformObjectAction(pWorldPtr, selectedEntity, { localPos, localRot, localScale }, result))
					{
						PANIC_ON_FAILURE(pActionsHistory->PushAndExecuteAction(pAction));
						pActionsHistory->Dump();
					}

					shouldRecordHistory = false;
				}

				if (auto pTransform = pSelectedEntity->GetComponent<CTransform>())
				{
					switch (manipulatorType)
					{
						case E_GIZMO_TYPE::TRANSLATION:
							pTransform->SetPosition(localPos);
							break;
						case E_GIZMO_TYPE::ROTATION:
							pTransform->SetRotation(localRot);
							break;
						case E_GIZMO_TYPE::SCALING:
							pTransform->SetScale(localScale);
							break;
					}
				}
			});
		}
	}


	static void DrawGizmoForMultiSelectedEntities(E_GIZMO_TYPE manipulatorType, TPtr<ISelectionManager> pSelectionManager, ISceneManager* pSceneManager,
												  IEditorActionsHistory* pActionsHistory, IImGUIContext* pImGUIContext, TEntityId& tempGroupEntityId, 
												  bool& shouldRecordHistory)
	{
		auto pWorld = pSceneManager->GetWorld();

		if (TEntityId::Invalid == tempGroupEntityId)
		{
			if (auto pMultiselectionGroupEntity = pWorld->CreateEntity("TEMP_MULTISELECT_GROUP"))
			{
				tempGroupEntityId = pMultiselectionGroupEntity->GetId();
			}
		}

		auto pGroupEntity = pWorld->FindEntity(tempGroupEntityId);
		auto pGroupTransform = pGroupEntity->GetComponent<CTransform>();

		pGroupTransform->SetPosition(GetAveragePoint(pSelectionManager, pWorld));

		TMatrix4 matrix = Transpose(pGroupTransform->GetLocalToWorldTransform());

		auto&& pCamera = GetCurrentActiveCamera(pWorld.Get());

		pImGUIContext->DrawGizmo(manipulatorType, Transpose(pCamera->GetViewMatrix()), Transpose(pCamera->GetProjMatrix()), matrix,
			[=, &shouldRecordHistory, pWorldPtr = pWorld.Get()](const TVector3& pos, const TQuaternion& rot, const TVector3& scale)
		{
			if (shouldRecordHistory)
			{
				E_RESULT_CODE result = RC_OK;

				if (auto pAction = CreateTransformObjectAction(pWorldPtr, tempGroupEntityId, { pos, rot, scale }, result))
				{
					PANIC_ON_FAILURE(pActionsHistory->PushAndExecuteAction(pAction));
					pActionsHistory->Dump();
				}

				shouldRecordHistory = false;
			}

			TVector3 deltaShift;
			TQuaternion deltaRotation;

			if (pGroupTransform)
			{
				switch (manipulatorType)
				{
					case E_GIZMO_TYPE::TRANSLATION:
						deltaShift = pos - pGroupTransform->GetPosition();
						pGroupTransform->SetPosition(pos);
						break;
					case E_GIZMO_TYPE::ROTATION:
						deltaRotation = rot - pGroupTransform->GetRotation();
						pGroupTransform->SetRotation(rot);
						break;
					case E_GIZMO_TYPE::SCALING:
						deltaShift = scale - pGroupTransform->GetScale();
						pGroupTransform->SetScale(scale);
						break;
				}
			}

			/// \note Apply delta to all selected entities
			for (const TEntityId currSelectedId : pSelectionManager->GetSelectedEntities())
			{
				auto pSelectedEntity = pWorld->FindEntity(currSelectedId);
				if (!pSelectedEntity)
				{
					continue;
				}

				if (auto pSelectedEntityTransform = pSelectedEntity->GetComponent<CTransform>())
				{
					switch (manipulatorType)
					{
						case E_GIZMO_TYPE::TRANSLATION:
							pSelectedEntityTransform->SetPosition(deltaShift + pSelectedEntityTransform->GetPosition());
							break;
						case E_GIZMO_TYPE::ROTATION:
							pSelectedEntityTransform->SetRotation(deltaRotation + pSelectedEntityTransform->GetRotation());
							break;
						case E_GIZMO_TYPE::SCALING:
							pSelectedEntityTransform->SetScale(deltaShift + pSelectedEntityTransform->GetScale());
							break;
					}
				}
			}
		});
	}


	bool CLevelEditorWindow::_onDrawGizmos()
	{
		if (mSelectedEntityId == TEntityId::Invalid)
		{
			return false;
		}

		auto&& selectedEntities = mpSelectionManager->GetSelectedEntities();

		if (selectedEntities.size() == 1)
		{
			DrawGizmoForSingleSelectedEntity(mCurrManipulatorType, mpSelectionManager, mpSceneManager, mpActionsHistory, mpImGUIContext, mShouldRecordHistory);
			return false;
		}

		DrawGizmoForMultiSelectedEntities(mCurrManipulatorType, mpSelectionManager, mpSceneManager, mpActionsHistory, mpImGUIContext, mTemporaryGroupEntityId, mShouldRecordHistory);		

		return false;
	}

	ISelectionManager* CLevelEditorWindow::_getSelectionManager()
	{
		if (!mpSelectionManager)
		{
			mpSelectionManager = mpEditorsManager->GetSelectionManager();
		}

		return mpSelectionManager.Get();
	}

	void CLevelEditorWindow::_onDrawInspector()
	{
		static bool isEnabled = true;

		static const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			TVector2(290.0f, 400.0f),
			TVector2(290.0f, 400.0f),
		};

		if (mpImGUIContext->BeginWindow("Object Inspector", isEnabled, params))
		{
			if (!_onDrawSceneInspector())
			{
				_onDrawObjectInspector();
			}
		}

		mpImGUIContext->EndWindow();
	}


	static void DisplayAddComponentBar(IImGUIContext* pImGUIContext, IWorld& world, CEntity* pEntity)
	{
		static const std::string addComponentMenu = "AddComponentContextMenu";

		if (pImGUIContext->Button("Add Component", TVector2(pImGUIContext->GetWindowWidth() * 0.95f, 25.0f)))
		{
			pImGUIContext->ShowModalWindow(addComponentMenu);
		}

		pImGUIContext->DisplayContextMenu(addComponentMenu, [pEntity, &world](IImGUIContext& imguiContext)
		{
			for (auto&& currComponentEntity : world.GetRegisteredComponentsIdentifiers())
			{
				imguiContext.MenuItem(currComponentEntity.mName, Wrench::StringUtils::GetEmptyStr(), [pEntity, componentType = currComponentEntity.mTypeId]
					{
						auto pComponent = pEntity->AddComponent(componentType);
						TDE2_ASSERT(pComponent);
					});
			}
		});
	}


	bool CLevelEditorWindow::_onDrawObjectInspector()
	{
		if (mpSelectionManager->GetSelectedEntities().size() > 1)
		{
			mpImGUIContext->Label("Multi selection mode doesn't support components editing");
			return false;
		}

		CEntity* pSelectedEntity = mpEditorsManager->GetWorldInstance()->FindEntity(mSelectedEntityId);
		if (!pSelectedEntity)
		{
			return false;
		}

		mpImGUIContext->BeginHorizontal();
		{
			std::string entityName = pSelectedEntity->GetName();

			mpImGUIContext->TextField("##entityId", entityName, [pSelectedEntity](auto&& value){ pSelectedEntity->SetName(value); });
			mpImGUIContext->Label(ToString<TEntityId>(mSelectedEntityId));
		}
		mpImGUIContext->EndHorizontal();

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

			(iter->second)({ *mpImGUIContext, *pCurrComponent, *mpActionsHistory, *mpEditorsManager->GetWorldInstance().Get(), mSelectedEntityId });
		}

		DisplayAddComponentBar(mpImGUIContext, *mpEditorsManager->GetWorldInstance().Get(), pSelectedEntity);

		return true;
	}

	bool CLevelEditorWindow::_onDrawSceneInspector()
	{
		IScene* pSelectedScene = dynamic_cast<CSceneHierarchyEditorWindow*>(mpHierarchyWidget)->GetSelectedSceneInfo();
		if (!pSelectedScene)
		{
			return false;
		}

		mpImGUIContext->Label(pSelectedScene->GetName());

		return true;
	}


	TDE2_API IEditorWindow* CreateLevelEditorWindow(const TLevelEditorWindowDesc& desc, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CLevelEditorWindow, result, desc);
	}
}

#endif