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

}

#endif