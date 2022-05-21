#include "../../include/editor/CSceneHierarchyWindow.h"
#include "../../include/editor/ecs/EditorComponents.h"
#include "../../include/core/IImGUIContext.h"
#include "../../include/core/IWindowSystem.h"
#include "../../include/scene/ISceneManager.h"
#include "../../include/scene/IScene.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/IWorld.h"
#include "../../include/ecs/CTransform.h"
#include "../../include/editor/CSelectionManager.h"
#include "../../include/editor/ecs/EditorComponents.h"
#include <stack>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	CSceneHierarchyEditorWindow::CSceneHierarchyEditorWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CSceneHierarchyEditorWindow::Init(ISceneManager* pSceneManager, IWindowSystem* pWindowSystem, ISelectionManager* pSelectionManager)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pSceneManager || !pWindowSystem || !pSelectionManager)
		{
			return RC_INVALID_ARGS;
		}

		mpSceneManager = pSceneManager;
		mpWindowSystem = pWindowSystem;
		mpSelectionManager = pSelectionManager;
		mpSelectedScene = nullptr;

		mIsInitialized = true;

		return RC_OK;
	}


	static const std::string EntityContextMenuId = "EntityOperations";


	static void DrawEntityContextMenu(IImGUIContext* pImGUIContext, ISelectionManager* pSelectionManager, TPtr<IWorld> pWorld)
	{
		pImGUIContext->DisplayContextMenu(EntityContextMenuId, [pSelectionManager, pWorld](IImGUIContext& imguiContext)
		{
			imguiContext.MenuItem("Delete", "Del", [pSelectionManager, pWorld]
			{
				std::stack<TEntityId> entitiesToDestroy;
				entitiesToDestroy.push(pSelectionManager->GetSelectedEntityId());

				while (!entitiesToDestroy.empty())
				{
					const TEntityId id = entitiesToDestroy.top();
					entitiesToDestroy.pop();

					if (CEntity* pEntity = pWorld->FindEntity(id))
					{
						if (CTransform* pTransform = pEntity->GetComponent<CTransform>())
						{
							for (const TEntityId currChildId : pTransform->GetChildren())
							{
								entitiesToDestroy.push(currChildId);
							}
						}

						E_RESULT_CODE result = pWorld->Destroy(pEntity);
						TDE2_ASSERT(RC_OK == result);
					}
				}				
			});
		});
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

		std::string sceneName;

		bool isInvalidState = false;

		if (mpImGUIContext->BeginWindow("Scene Hierarchy", isEnabled, params))
		{
			mpImGUIContext->Button("Load Level Chunk", { mpImGUIContext->GetWindowWidth() - 15.0f, 25.0f }, std::bind(&CSceneHierarchyEditorWindow::_executeLoadLevelChunkOperation, this));

			for (IScene* pCurrScene : mpSceneManager->GetLoadedScenes())
			{
				sceneName = pCurrScene->GetName();

				if (mpImGUIContext->CollapsingHeader(sceneName, true, (mpSelectedScene == pCurrScene), [&isInvalidState, pCurrScene, sceneName, this] { mpSelectedScene = pCurrScene; }))
				{
					// \note Display context menu of the scene's header					
					mpImGUIContext->DisplayContextMenu(Wrench::StringUtils::Format("{0}##Context_Menu", sceneName), [&isInvalidState, this, sceneName, pCurrScene](IImGUIContext& imguiContext)
					{
						if (pCurrScene->IsMainScene()) // \note The main scene cannot be deleted by a user
						{
							return;
						}

						imguiContext.MenuItem("Unload Scene Chunk", Wrench::StringUtils::GetEmptyStr(), [&isInvalidState, this, sceneName]
						{
							_unloadSceneOperation(sceneName);
							isInvalidState = true;
						});
					});

					if (isInvalidState) // If the scene was deleted we can ignore the rest of the work 'til next frame
					{
						break;
					}

					auto pWorld = mpSceneManager->GetWorld();

					TEntityId currParentId = TEntityId::Invalid;

					std::function<void(CEntity*)> drawEntityHierarchy = [this, &drawEntityHierarchy, pWorld, &currParentId](CEntity* pEntity)
					{
						CTransform* pTransform = pEntity->GetComponent<CTransform>();

						const TEntityId parentEntityId = pTransform->GetParent();
						if (currParentId != parentEntityId)
						{
							return;
						}

						const bool isLeafEntity = pTransform->GetChildren().empty();

						const std::string fieldStr = Wrench::StringUtils::Format("{0}##{1}", pEntity->GetName(), static_cast<U32>(pEntity->GetId()));

						if (isLeafEntity)
						{
							if (mpImGUIContext->SelectableItem(fieldStr))
							{
								mpSelectionManager->SetSelectedEntity(pEntity->GetId());
								mpSelectedScene = nullptr;

								mpImGUIContext->RegisterDragAndDropSource([this, id = static_cast<I32>(pEntity->GetId())]
								{
										mpImGUIContext->SetDragAndDropData("test", id);
								});

								mpImGUIContext->RegisterDragAndDropTarget([this]
								{
									auto t = mpImGUIContext->GetDragAndDropData<I32>("test");

								});
							}

							if (mpSelectionManager->GetSelectedEntityId() == pEntity->GetId())
							{
								DrawEntityContextMenu(mpImGUIContext, mpSelectionManager, pWorld);
							}

							return;
						}

						bool isOpened = false;
						bool isSelected = false;

						std::tie(isOpened, isSelected) = mpImGUIContext->BeginTreeNode(fieldStr);

						if (isOpened)
						{
							const TEntityId prevParentId = currParentId;
							currParentId = pEntity->GetId();

							for (auto currChild : pTransform->GetChildren())
							{
								drawEntityHierarchy(pWorld->FindEntity(currChild));
							}

							currParentId = prevParentId;

							mpImGUIContext->EndTreeNode();
						}

						if (isSelected)
						{
							mpSelectionManager->SetSelectedEntity(pEntity->GetId());
							mpSelectedScene = nullptr;
						}

						if (mpSelectionManager->GetSelectedEntityId() == pEntity->GetId())
						{
							DrawEntityContextMenu(mpImGUIContext, mpSelectionManager, pWorld);

							if (mpImGUIContext->IsMouseClicked(1))
							{
								mpImGUIContext->ShowModalWindow(EntityContextMenuId);
							}
						}
					};

					pCurrScene->ForEachEntity(drawEntityHierarchy);
				}
			}

			mpImGUIContext->EndWindow();
		}

		mIsVisible = isEnabled;
	}

	IScene* CSceneHierarchyEditorWindow::GetSelectedSceneInfo() const
	{
		return mpSelectedScene;
	}

	void CSceneHierarchyEditorWindow::_executeLoadLevelChunkOperation()
	{
		auto openFileResult = mpWindowSystem->ShowOpenFileDialog({ { "Scene Files", "*.scene" } });
		if (openFileResult.IsOk())
		{
			const std::string& sceneFilepath = openFileResult.Get();

			mpSceneManager->LoadSceneAsync(sceneFilepath, [](auto) {});
		}
	}

	void CSceneHierarchyEditorWindow::_unloadSceneOperation(const std::string& sceneId)
	{
		if (!mpSceneManager)
		{
			return;
		}

		E_RESULT_CODE result = mpSceneManager->UnloadScene(mpSceneManager->GetSceneId(sceneId));
		TDE2_ASSERT(result == RC_OK);
	}


	TDE2_API IEditorWindow* CreateSceneHierarchyEditorWindow(ISceneManager* pSceneManager, IWindowSystem* pWindowSystem, ISelectionManager* pSelectionManager, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CSceneHierarchyEditorWindow, result, pSceneManager, pWindowSystem, pSelectionManager);
	}
}

#endif