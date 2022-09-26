#include "../../include/editor/CSceneHierarchyWindow.h"
#include "../../include/editor/ecs/EditorComponents.h"
#include "../../include/core/IImGUIContext.h"
#include "../../include/core/IWindowSystem.h"
#include "../../include/core/IFileSystem.h"
#include "../../include/core/IInputContext.h"
#include "../../include/scene/ISceneManager.h"
#include "../../include/scene/IScene.h"
#include "../../include/scene/IPrefabsRegistry.h"
#include "../../include/ecs/CEntity.h"
#include "../../include/ecs/CWorld.h"
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

	E_RESULT_CODE CSceneHierarchyEditorWindow::Init(const TSceneHierarchyInitParams& params)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!params.mpSceneManager || !params.mpWindowSystem || !params.mpSelectionManager || !params.mpInputContext || !params.mpFileSystem)
		{
			return RC_INVALID_ARGS;
		}

		mpSceneManager     = params.mpSceneManager;
		mpWindowSystem     = params.mpWindowSystem;
		mpSelectionManager = params.mpSelectionManager;
		mpInputContext     = params.mpInputContext;
		mpFileSystem       = params.mpFileSystem;
		mpSelectedScene    = nullptr;

		mIsInitialized = true;

		return RC_OK;
	}


	static const std::string EntityContextMenuId = "EntityOperations";
	static const std::string PrefabsSelectionMenuId = "Select a Prefab...##PrefabsSelectionWindow";


	static std::function<bool()> DrawPrefabsSelectionWindow = nullptr;


	struct TEntityOperationsDesc
	{
		IImGUIContext*     mpImGUIContext;
		ISceneManager*     mpSceneManager;
		IScene*            mpCurrScene;
		ISelectionManager* mpSelectionManager;
	};


	static void LoadPrefab(const TEntityOperationsDesc& desc)
	{
		auto pCurrScene = desc.mpCurrScene;
		if (!pCurrScene)
		{
			return;
		}

		auto pSceneManager = desc.mpSceneManager;
		auto pSelectionManager = desc.mpSelectionManager;
		
		auto pWorld = pSceneManager->GetWorld();
		if (!pWorld)
		{
			return;
		}

		CEntity* pParentEntity = pSelectionManager->GetSelectedEntities().empty() ? nullptr : pWorld->FindEntity(pSelectionManager->GetSelectedEntityId());

		static U32 SelectedPrefabIndex = 0;

		if (auto pPrefabsRegistry = pSceneManager->GetPrefabsRegistry())
		{
			auto&& prefabsIdentifiers = pPrefabsRegistry->GetKnownPrefabsIdentifiers();

			auto pImGUIContext = desc.mpImGUIContext;

			DrawPrefabsSelectionWindow = [prefabsIdentifiers, pCurrScene, pImGUIContext, pParentEntity]
			{
				bool shouldQuit = false;

				pImGUIContext->ShowModalWindow(PrefabsSelectionMenuId);

				if (pImGUIContext->BeginModalWindow(PrefabsSelectionMenuId))
				{
					const TVector2 buttonSizes(100.0f, 25.0f);

					pImGUIContext->BeginHorizontal();
					{
						if (pImGUIContext->BeginChildWindow("##PrefabsList", TVector2(400.0f, 250.0f)))
						{
							for (U32 i = 0; i < static_cast<U32>(prefabsIdentifiers.size()); ++i)
							{
								if (pImGUIContext->SelectableItem(prefabsIdentifiers[i], i == SelectedPrefabIndex))
								{
									SelectedPrefabIndex = i;
								}
							}

							pImGUIContext->EndChildWindow();
						}

						if (pImGUIContext->Button("Add", buttonSizes))
						{
							pCurrScene->Spawn(prefabsIdentifiers[SelectedPrefabIndex], pParentEntity);
							shouldQuit = true;
						}

						if (pImGUIContext->Button("Cancel", buttonSizes))
						{
							pImGUIContext->CloseCurrentModalWindow();
							shouldQuit = true;
						}
					}
					pImGUIContext->EndHorizontal();

					pImGUIContext->EndModalWindow();
				}

				return !shouldQuit;
			};
		}
	}


	struct TSaveHierarchyDesc
	{
		TEntityOperationsDesc mBase;

		IWindowSystem* mpWindowSystem;
		IFileSystem*   mpFileSystem;
	};


	static void SaveHierarchyToPrefab(const TSaveHierarchyDesc& params)
	{
		auto&& pWindowSystem = params.mpWindowSystem;
		auto&& pSceneManager = params.mBase.mpSceneManager;

		auto saveFileResult = pWindowSystem->ShowSaveFileDialog({ { "Prefabs", "*.prefab" } });
		if (saveFileResult.HasError())
		{
			return;
		}

		auto pWorld = pSceneManager->GetWorld();
		if (!pWorld)
		{
			return;
		}

		const std::string& prefabFilepath = saveFileResult.Get();
		TDE2_ASSERT(!prefabFilepath.empty());

		auto pPrefabsRegistry = pSceneManager->GetPrefabsRegistry();
		if (!pPrefabsRegistry)
		{
			return;
		}

		auto&& pFileSystem = params.mpFileSystem;
		auto&& pSelectionManager = params.mBase.mpSelectionManager;

		/// \note Extract prefab's name from the file path
		//std::string prefabId = desc.m


#if TDE2_EDITORS_ENABLED
		E_RESULT_CODE result = pPrefabsRegistry->SavePrefab("", prefabFilepath, pWorld->FindEntity(pSelectionManager->GetSelectedEntityId()));
		TDE2_ASSERT(RC_OK == result);
#endif
	}


	static void DrawEntityContextMenu(const TSaveHierarchyDesc& desc, TPtr<IWorld> pWorld)
	{
		auto&& pImGUIContext = desc.mBase.mpImGUIContext;
		auto&& pSelectionManager = desc.mBase.mpSelectionManager;

		pImGUIContext->DisplayContextMenu(EntityContextMenuId, [=](IImGUIContext& imguiContext)
		{
			imguiContext.MenuItem("Delete", "Del", [&desc, pWorld, pSelectionManager]
			{
				std::stack<TEntityId> entitiesToDestroy;

				for (TEntityId currEntity : pSelectionManager->GetSelectedEntities())
				{
					entitiesToDestroy.push(currEntity);
				}

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

			imguiContext.MenuItem("Link Prefab", Wrench::StringUtils::GetEmptyStr(), [&]
			{
				LoadPrefab(desc.mBase);
			});

			imguiContext.MenuItem("Save To Prefab", Wrench::StringUtils::GetEmptyStr(), [=]
			{
				SaveHierarchyToPrefab(desc);
			});
		});
	}


	static void ProcessDragAndDropLogic(IImGUIContext* pImGUIContext, TPtr<IWorld> pWorld, TEntityId entityId, bool initializeSource = true)
	{
		static const std::string DraggedEntityId = "DRAGGED_ENTITY_ID";

		if (initializeSource)
		{
			pImGUIContext->RegisterDragAndDropSource([pImGUIContext, pWorldPtr = pWorld.Get(), entityId]
				{
					pImGUIContext->SetDragAndDropData(DraggedEntityId, entityId);

					if (auto pEntity = pWorldPtr->FindEntity(entityId))
					{
						pImGUIContext->Label(pEntity->GetName());
					}
				});
		}

		pImGUIContext->RegisterDragAndDropTarget([pImGUIContext, pWorldPtr = pWorld.Get(), entityId]
		{
			if (auto pChildEntityId = pImGUIContext->GetDragAndDropData<TEntityId>(DraggedEntityId))
			{
				GroupEntities(pWorldPtr, entityId, *pChildEntityId);
			}
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

		auto pWorld = mpSceneManager->GetWorld();

		if (mpImGUIContext->BeginWindow("Scene Hierarchy", isEnabled, params))
		{
			/// \note Draw invisible button for drag & drop dettach functionality
			{
				auto currPosition = mpImGUIContext->GetCursorScreenPos();
				mpImGUIContext->Button("##NullParent", TVector2(mpImGUIContext->GetWindowWidth(), mpImGUIContext->GetWindowHeight()), nullptr, true, true);
				ProcessDragAndDropLogic(mpImGUIContext, pWorld, TEntityId::Invalid, false);
				mpImGUIContext->SetCursorScreenPos(currPosition);
			}

			mpImGUIContext->Button("Load Level Chunk", { mpImGUIContext->GetWindowWidth() - 15.0f, 25.0f }, std::bind(&CSceneHierarchyEditorWindow::_executeLoadLevelChunkOperation, this));

			for (IScene* pCurrScene : mpSceneManager->GetLoadedScenes())
			{
				sceneName = pCurrScene->GetName();

				if (mpImGUIContext->CollapsingHeader(sceneName, true, (mpSelectedScene == pCurrScene), [&isInvalidState, pCurrScene, sceneName, this] { mpSelectedScene = pCurrScene; }))
				{
					// \note Display context menu of the scene's header					
					mpImGUIContext->DisplayContextMenu(Wrench::StringUtils::Format("{0}##Context_Menu", sceneName), [&isInvalidState, this, sceneName, pCurrScene](IImGUIContext& imguiContext)
					{
						if (!pCurrScene->IsMainScene()) // \note The main scene cannot be deleted by a user
						{
							imguiContext.MenuItem("Unload Scene Chunk", Wrench::StringUtils::GetEmptyStr(), [&isInvalidState, this, sceneName]
							{
								_unloadSceneOperation(sceneName);
								isInvalidState = true;
							});
						}

						imguiContext.MenuItem("Save Scene Chunk", Wrench::StringUtils::GetEmptyStr(), [pCurrScene, this]
						{
#if TDE2_EDITORS_ENABLED
							if (auto saveFilepathResult = mpWindowSystem->ShowSaveFileDialog({ { "Scenes", "*.scene" } }))
							{
								mpSceneManager->SaveSceneChunk(saveFilepathResult.Get(), mpSceneManager->GetSceneId(pCurrScene->GetName()));
							}
#endif
						});

						imguiContext.MenuItem("Create New Entity", Wrench::StringUtils::GetEmptyStr(), [this, pCurrScene]
						{
							if (!pCurrScene) 
							{
								return;
							}

							/// \note Also a new created entity becomes selected at the same time
							if (CEntity* pEntity = pCurrScene->CreateEntity("NewEntity"))
							{
								mpSelectionManager->SetSelectedEntity(pEntity->GetId());
							}
						});

						imguiContext.MenuItem("Load Prefab", Wrench::StringUtils::GetEmptyStr(), [this, pCurrScene]
						{
							mpSelectionManager->ClearSelection(); /// \note Clear the selected entities list because the prefab will be instantiated into the root of the scene
							LoadPrefab({ mpImGUIContext, mpSceneManager, pCurrScene, mpSelectionManager });
						});
					});

					if (isInvalidState) // If the scene was deleted we can ignore the rest of the work 'til next frame
					{
						break;
					}

					TEntityId currParentId = TEntityId::Invalid;

					std::function<void(CEntity*)> drawEntityHierarchy = [this, &drawEntityHierarchy, pWorld, &currParentId, pCurrScene](CEntity* pEntity)
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
							if (mpImGUIContext->SelectableItem(fieldStr, mpSelectionManager->IsEntityBeingSelected(pEntity->GetId())))
							{
								if (mpInputContext->IsKey(E_KEYCODES::KC_LCONTROL))
								{
									mpSelectionManager->AddSelectedEntity(pEntity->GetId());
								}
								else
								{
									mpSelectionManager->SetSelectedEntity(pEntity->GetId());
								}

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

							ProcessDragAndDropLogic(mpImGUIContext, pWorld, pEntity->GetId());

							/// \note No matter of multiselection display context menu for last selected entity. It'll work for both cases 
							if (mpSelectionManager->GetSelectedEntityId() == pEntity->GetId())
							{
								DrawEntityContextMenu({ mpImGUIContext, mpSceneManager, pCurrScene, mpSelectionManager, mpWindowSystem, mpFileSystem }, pWorld);
							}

							return;
						}

						bool isOpened = false;
						bool isSelected = false;

						std::tie(isOpened, isSelected) = mpImGUIContext->BeginTreeNode(fieldStr, mpSelectionManager->IsEntityBeingSelected(pEntity->GetId()));
						ProcessDragAndDropLogic(mpImGUIContext, pWorld, pEntity->GetId());

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
							if (mpInputContext->IsKey(E_KEYCODES::KC_LCONTROL))
							{
								mpSelectionManager->AddSelectedEntity(pEntity->GetId());
							}
							else
							{
								mpSelectionManager->SetSelectedEntity(pEntity->GetId());
							}
							mpSelectedScene = nullptr;
						}

						if (mpSelectionManager->IsEntityBeingSelected(pEntity->GetId()))
						{
							DrawEntityContextMenu({ mpImGUIContext, mpSceneManager, pCurrScene, mpSelectionManager, mpWindowSystem, mpFileSystem }, pWorld);
						}
					};

					pCurrScene->ForEachEntity(drawEntityHierarchy);
				}
			}

			if (DrawPrefabsSelectionWindow)
			{
				const bool result = DrawPrefabsSelectionWindow();
				if (!result)
				{
					DrawPrefabsSelectionWindow = nullptr;
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


	TDE2_API IEditorWindow* CreateSceneHierarchyEditorWindow(const TSceneHierarchyInitParams& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CSceneHierarchyEditorWindow, result, params);
	}
}

#endif