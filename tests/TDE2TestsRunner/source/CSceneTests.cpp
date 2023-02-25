#include <TDEngine2.h>
#define DEFER_IMPLEMENTATION
#include "deferOperation.hpp"


using namespace TDEngine2;


#if TDE2_EDITORS_ENABLED

TDE2_TEST_FIXTURE("EntitiesOperationsTests")
{
	TDE2_TEST_CASE("TestCopyPasteCase_TryToCopyAndPasteSingleEntityHierarchy_TheEntityShouldBeCopiedCorrectly")
	{
		static CEntity* pParentEntity = nullptr;

		/// \note Create an entity
		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pMainScene = pSceneManager->GetScene(MainScene).Get();
			TDE2_TEST_IS_TRUE(pMainScene);

			pParentEntity = pMainScene->CreateEntity("ParentEntity");
			TDE2_TEST_IS_TRUE(pParentEntity);

			CEntity* pChildEntity = pMainScene->CreateEntity("ChildEntity");
			TDE2_TEST_IS_TRUE(pChildEntity);

			TDE2_TEST_IS_TRUE(RC_OK == GroupEntities(pWorld.Get(), pParentEntity->GetId(), pChildEntity->GetId()));
		});

		pTestCase->WaitForNextFrame();
		
		/// \note Copy and paste that
		pTestCase->ExecuteAction([&]
		{
			TDE2_TEST_IS_TRUE(pParentEntity);

			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();
			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();

			TDE2_TEST_IS_TRUE(RC_OK == CEntitiesCommands::CopyEntitiesHierarchy(pSceneManager->GetPrefabsRegistry(), pSceneManager->GetWorld(), pParentEntity->GetId()));
			
			TDE2_TEST_IS_TRUE(CEntitiesCommands::PasteEntitiesHierarchy(
				pSceneManager->GetPrefabsRegistry(), 
				pSceneManager->GetWorld(), 
				pSceneManager->GetScene(MainScene).Get(),
				pParentEntity->GetId()).IsOk());
		});
	}

	TDE2_TEST_CASE("TestCopyPasteCase_CreateNestedHierarchyTryToCopyAndPasteIt_TheHierarchyCorrectlyCopied")
	{
		static CEntity* pParentEntity = nullptr;

		/// \note Create an entity
		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pMainScene = pSceneManager->GetScene(MainScene).Get();
			TDE2_TEST_IS_TRUE(pMainScene);

			pParentEntity = pMainScene->CreateEntity("ParentEntity");
			TDE2_TEST_IS_TRUE(pParentEntity);

			CEntity* pChildEntity1 = pMainScene->CreateEntity("ChildEntity1");
			TDE2_TEST_IS_TRUE(pChildEntity1);

			CEntity* pChildEntity2 = pMainScene->CreateEntity("ChildEntity2");
			TDE2_TEST_IS_TRUE(pChildEntity2);

			TDE2_TEST_IS_TRUE(RC_OK == GroupEntities(pWorld.Get(), pParentEntity->GetId(), pChildEntity1->GetId()));
			TDE2_TEST_IS_TRUE(RC_OK == GroupEntities(pWorld.Get(), pChildEntity1->GetId(), pChildEntity2->GetId()));
		});

		pTestCase->WaitForNextFrame();

		/// \note Copy and paste that
		pTestCase->ExecuteAction([&]
		{
			TDE2_TEST_IS_TRUE(pParentEntity);

			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();
			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();

			TDE2_TEST_IS_TRUE(RC_OK == CEntitiesCommands::CopyEntitiesHierarchy(pSceneManager->GetPrefabsRegistry(), pSceneManager->GetWorld(), pParentEntity->GetId()));

			TDE2_TEST_IS_TRUE(CEntitiesCommands::PasteEntitiesHierarchy(
				pSceneManager->GetPrefabsRegistry(),
				pSceneManager->GetWorld(),
				pSceneManager->GetScene(MainScene).Get(),
				pParentEntity->GetId()).IsOk());
		});
	}

	TDE2_TEST_CASE("TestCopyPasteCase_SpawnAPrefabAndTryToDuplicateThat_ThePrefabIsCorrectlyDuplicated")
	{
		static CEntity* pPrefabEntity = nullptr;

		/// \note Spawn a prefab
		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pMainScene = pSceneManager->GetScene(MainScene).Get();
			TDE2_TEST_IS_TRUE(pMainScene);

			pPrefabEntity = pMainScene->Spawn("TestPrefab");
		});

		pTestCase->WaitForNextFrame();

		/// \note Copy and paste that
		pTestCase->ExecuteAction([&]
		{
			TDE2_TEST_IS_TRUE(pPrefabEntity);

			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();
			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();

			TDE2_TEST_IS_TRUE(RC_OK == CEntitiesCommands::CopyEntitiesHierarchy(pSceneManager->GetPrefabsRegistry(), pSceneManager->GetWorld(), pPrefabEntity->GetId()));

			TDE2_TEST_IS_TRUE(CEntitiesCommands::PasteEntitiesHierarchy(
				pSceneManager->GetPrefabsRegistry(),
				pSceneManager->GetWorld(),
				pSceneManager->GetScene(MainScene).Get(),
				pPrefabEntity->GetId()).IsOk());
		});
	}

	TDE2_TEST_CASE("TestCopyPasteCase_TryToDuplicateHierarchyWithNestedPrefabs_DuplicationShouldBeSuccessful")
	{
		static TSceneId testSceneHandle = TSceneId::Invalid;

		/// \note Spawn a prefab
		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			pSceneManager->LoadSceneAsync("ProjectResources/Scenes/1.scene", [&](const TResult<TSceneId>& sceneIdResult)
			{
				testSceneHandle = sceneIdResult.Get();
			});
		});

		pTestCase->WaitForCondition([&] { return TSceneId::Invalid != testSceneHandle; });

		/// \note Copy and paste that
		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();
			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();

			auto pTestScene = pSceneManager->GetScene(testSceneHandle).Get();
			TDE2_TEST_IS_TRUE(pTestScene);

			auto sceneEntities = pTestScene->GetEntities();
			TDE2_TEST_IS_TRUE(!sceneEntities.empty());

			/// \note Copy the first entity because of it's a root
			TDE2_TEST_IS_TRUE(RC_OK == CEntitiesCommands::CopyEntitiesHierarchy(pSceneManager->GetPrefabsRegistry(), pSceneManager->GetWorld(), sceneEntities.front()));

			TDE2_TEST_IS_TRUE(CEntitiesCommands::PasteEntitiesHierarchy(
				pSceneManager->GetPrefabsRegistry(),
				pSceneManager->GetWorld(),
				pTestScene,
				TEntityId::Invalid).IsOk());

			TDE2_TEST_IS_TRUE(RC_OK == pSceneManager->UnloadScene(pSceneManager->GetSceneId(pTestScene->GetName())));
		});
	}

	TDE2_TEST_CASE("TestSaveEntitiesHierarchy_TryToSerializePrefabRoot_SavesAllEntitiesThatPartOfTheHierarchy")
	{
		static CEntity* pPrefabEntity = nullptr;

		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();
			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();

			auto pMainScene = pSceneManager->GetScene(MainScene).Get();
			TDE2_TEST_IS_TRUE(pMainScene);

			pPrefabEntity = pMainScene->Spawn("TestPrefab");
		});

		pTestCase->WaitForNextFrame();

		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();
			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();

			E_RESULT_CODE result = RC_OK;

			auto pMemoryMappedStream = TPtr<TDEngine2::IStream>(CreateMemoryIOStream(Wrench::StringUtils::GetEmptyStr(), {}, result));
			if (!pMemoryMappedStream)
			{
				TDE2_TEST_IS_TRUE(false);
				return result;
			}

			if (auto pPrefabsRegistry = pSceneManager->GetPrefabsRegistry())
			{
				IYAMLFileWriter* pFileWriter = dynamic_cast<IYAMLFileWriter*>(CreateYAMLFileWriter(nullptr, pMemoryMappedStream, result));
				if (!pFileWriter)
				{
					TDE2_TEST_IS_TRUE(false);
					return result;
				}

				result = result | pPrefabsRegistry->SavePrefabHierarchy(pFileWriter, pSceneManager->GetWorld().Get(), pPrefabEntity);
				result = result | pFileWriter->Close();

				TDE2_TEST_IS_TRUE(RC_OK == result);
			}

			/// \note Read back the structure of the archive
			IYAMLFileReader* pFileReader = dynamic_cast<IYAMLFileReader*>(CreateYAMLFileReader(nullptr, pMemoryMappedStream, result));
			if (!pFileReader)
			{
				TDE2_TEST_IS_TRUE(false);
				return result;
			}

			/*!
				\brief The structure of TestPrefab looks like the following
				- entities:
					- entity:
						- component:
					- entity:
						- component:
					- entity:
						- component:
			*/

			const std::string textData{ DynamicPtrCast<IInputStream>(pMemoryMappedStream)->ReadToEnd() };
			USIZE entitiesCount = 0;

			defer([=] { pFileReader->Close(); });

			pFileReader->BeginGroup("entities");
			{
				TEntityId currEntityId = TEntityId::Invalid;

				while (pFileReader->HasNextItem())
				{
					pFileReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());
					{
						pFileReader->BeginGroup("entity");
						
						currEntityId = static_cast<TEntityId>(pFileReader->GetUInt32("id", static_cast<U32>(TEntityId::Invalid)));
						if (TEntityId::Invalid != currEntityId)
						{
							entitiesCount++;
						}
						else
						{
							TDE2_TEST_IS_TRUE(false);
						}

						pFileReader->EndGroup();
					}
					pFileReader->EndGroup();
				}
			}
			pFileReader->EndGroup();

			TDE2_TEST_IS_TRUE(entitiesCount == 3);
		});
	}

	TDE2_TEST_CASE("TestSaveEntitiesHierarchy_CreateNewEntityLinkTestPrefabAndTryToSaveIt_CorrectlySavesNestedLink")
	{
		static CEntity* pHierarchyRootEntity = nullptr;

		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();
			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();

			auto pMainScene = pSceneManager->GetScene(MainScene).Get();
			TDE2_TEST_IS_TRUE(pMainScene);

			pHierarchyRootEntity = pMainScene->CreateEntity("HierarchyWithNestedPrefab");
			CEntity* pPrefab = pMainScene->Spawn("TestPrefab");

			TDE2_TEST_IS_TRUE(RC_OK == GroupEntities(pSceneManager->GetWorld().Get(), pHierarchyRootEntity->GetId(), pPrefab->GetId()));
		});

		pTestCase->WaitForNextFrame();

		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();
			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();

			E_RESULT_CODE result = RC_OK;

			auto pMemoryMappedStream = TPtr<TDEngine2::IStream>(CreateMemoryIOStream(Wrench::StringUtils::GetEmptyStr(), {}, result));
			if (!pMemoryMappedStream)
			{
				TDE2_TEST_IS_TRUE(false);
				return result;
			}

			if (auto pPrefabsRegistry = pSceneManager->GetPrefabsRegistry())
			{
				IYAMLFileWriter* pFileWriter = dynamic_cast<IYAMLFileWriter*>(CreateYAMLFileWriter(nullptr, pMemoryMappedStream, result));
				if (!pFileWriter)
				{
					TDE2_TEST_IS_TRUE(false);
					return result;
				}

				result = result | pPrefabsRegistry->SavePrefabHierarchy(pFileWriter, pSceneManager->GetWorld().Get(), pHierarchyRootEntity);
				result = result | pFileWriter->Close();

				TDE2_TEST_IS_TRUE(RC_OK == result);
			}

			/// \note Read back the structure of the archive
			IYAMLFileReader* pFileReader = dynamic_cast<IYAMLFileReader*>(CreateYAMLFileReader(nullptr, pMemoryMappedStream, result));
			if (!pFileReader)
			{
				TDE2_TEST_IS_TRUE(false);
				return result;
			}

			/*!
				\brief The structure of the hierarchy looks like the following
				- entities:
					- entity:
						- component:
					- link
						prefab_id: TestPrefab
			*/

			const std::string textData{ DynamicPtrCast<IInputStream>(pMemoryMappedStream)->ReadToEnd() };
			USIZE entitiesCount = 0;
			USIZE linksCount = 0;

			defer([=] { pFileReader->Close(); });

			pFileReader->BeginGroup("entities");
			{
				TEntityId currEntityId = TEntityId::Invalid;

				while (pFileReader->HasNextItem())
				{
					pFileReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());
					{
						pFileReader->BeginGroup("entity");

						currEntityId = static_cast<TEntityId>(pFileReader->GetUInt32("id", static_cast<U32>(TEntityId::Invalid)));
						if (TEntityId::Invalid != currEntityId)
						{
							entitiesCount++;
						}
						else
						{
							linksCount++;
						}

						pFileReader->EndGroup();
					}
					pFileReader->EndGroup();
				}
			}
			pFileReader->EndGroup();

			TDE2_TEST_IS_TRUE(entitiesCount == 1);
			TDE2_TEST_IS_TRUE(linksCount == 1);
		});
	}

	TDE2_TEST_CASE("TestSpawnPrefab_TryToSpawnPrefabWithNestedLinkage_CorrectlySpawnNestedPrefabWithTheGivenOne")
	{
		static CEntity* pPrefab = nullptr;

		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();
			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();

			auto pMainScene = pSceneManager->GetScene(MainScene).Get();
			TDE2_TEST_IS_TRUE(pMainScene);

			pPrefab = pMainScene->Spawn("TestNestedPrefab");
			TDE2_TEST_IS_TRUE(pPrefab);
		});

		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();
			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pMainScene = pSceneManager->GetScene(MainScene).Get();
			TDE2_TEST_IS_TRUE(pMainScene);

			/*!
				\note the structure of TestNestedPrefab

				Entity0
					Entity1
					Entity2
						Entity0 <---- Here the nested prefab starts
							Entity2
							Entity2
			*/

			std::array<std::string, 2> childrenIdentifiers
			{
				"Entity1", "Entity2"
			};

			if (auto pRootTransform = pPrefab->GetComponent<CTransform>())
			{
				auto& children = pRootTransform->GetChildren();
				TDE2_TEST_IS_TRUE(childrenIdentifiers.size() == children.size());

				CEntity* pNestedPrefabRoot = nullptr;

				for (USIZE i = 0; i < children.size(); i++)
				{
					CEntity* pEntity = pWorld->FindEntity(children[i]);
					TDE2_TEST_IS_TRUE(pEntity);

					TDE2_TEST_IS_TRUE(pEntity->GetName() == childrenIdentifiers[i]);

					if (i == children.size() - 1)
					{
						auto pTransform = pEntity->GetComponent<CTransform>();
						TDE2_TEST_IS_TRUE(pTransform->GetChildren().size() == 1);

						pNestedPrefabRoot = pWorld->FindEntity(pTransform->GetChildren().front());
					}
				}

				/// \note Nested prefabs checks
				if (auto pNestedRootTransform = pNestedPrefabRoot->GetComponent<CTransform>())
				{
					auto& children = pNestedRootTransform->GetChildren();
					TDE2_TEST_IS_TRUE(childrenIdentifiers.size() == children.size());

					for (USIZE i = 0; i < children.size(); i++)
					{
						CEntity* pEntity = pWorld->FindEntity(children[i]);
						TDE2_TEST_IS_TRUE(pEntity);

						TDE2_TEST_IS_TRUE(pEntity->GetName() == childrenIdentifiers[i]);
					}
				}
			}

			TDE2_TEST_IS_TRUE(RC_OK == pMainScene->RemoveEntity(pPrefab->GetId()));
		});
	}

	TDE2_TEST_CASE("TestPrefabsLinks_LoadSceneWithLinks_TheRootEntitiesOfPrefabsShouldHaveIdentifiersOfLinks")
	{
		static TSceneId testSceneHandle = TSceneId::Invalid;

		/// \note Spawn a prefab
		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			pSceneManager->LoadSceneAsync("ProjectResources/Scenes/TestEntityRefs.scene", [&](const TResult<TSceneId>& sceneIdResult)
			{
				testSceneHandle = sceneIdResult.Get();
			});
		});

		pTestCase->WaitForCondition([&] { return TSceneId::Invalid != testSceneHandle; });

		/// \note Copy and paste that
		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();
			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pTestScene = pSceneManager->GetScene(testSceneHandle).Get();
			TDE2_TEST_IS_TRUE(pTestScene);

			//TDE2_TEST_IS_TRUE(false);

			TDE2_TEST_IS_TRUE(RC_OK == pSceneManager->UnloadScene(pSceneManager->GetSceneId(pTestScene->GetName())));
		});
	}
}

#endif