#include <TDEngine2.h>


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
}

#endif