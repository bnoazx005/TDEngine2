#include <TDEngine2.h>


using namespace TDEngine2;


#if TDE2_EDITORS_ENABLED

TDE2_TEST_FIXTURE("Prefabs Changes Tests")
{
	TDE2_TEST_CASE("TestApplyChanges_LoadPrefabWithNestedOnesThatContainsChangesList_AllChangesShouldBeAppliedCorrectly")
	{
		static CEntity* pPrefabEntity = nullptr;

		/// \note Load a prefab with nested prefabs with changes list
		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pMainScene = pSceneManager->GetScene(MainScene).Get();
			TDE2_TEST_IS_TRUE(pMainScene);

			pPrefabEntity = pMainScene->Spawn("TestNestedPrefabChanges");
		});

		pTestCase->WaitForNextFrame();
		
		pTestCase->ExecuteAction([&]
		{
			TDE2_TEST_IS_TRUE(pPrefabEntity);

			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();
			
			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pRootTransform = pPrefabEntity->GetComponent<CTransform>();
			auto&& childrenEntities = pRootTransform->GetChildren();

			TDE2_TEST_IS_TRUE(childrenEntities.size() == 3);

			std::vector<CLabel*> pLabels;
			std::transform(childrenEntities.begin(), childrenEntities.end(), std::back_inserter(pLabels), [pWorld](TEntityId id)
			{
				CEntity* pEntity = pWorld->FindEntity(id);
				TDE2_TEST_IS_TRUE(pEntity);

				return pEntity->GetComponent<CLabel>();
			});

			TDE2_TEST_IS_TRUE(pLabels[0]->GetText() == "Test2");
			TDE2_TEST_IS_TRUE(pLabels[2]->GetText() == "Test1");
		});
	}
}

#endif