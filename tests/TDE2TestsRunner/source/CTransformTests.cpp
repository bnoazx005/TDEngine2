#include <TDEngine2.h>


using namespace TDEngine2;


TDE2_TEST_FIXTURE("CTransform Tests")
{
	TDE2_TEST_CASE("TestGroupEntities_PassTwoEntitiesGroupThem_AllTransformsShouldBeInCorrectState")
	{
		static CTransform* pParentTransform = nullptr;
		static CTransform* pChildTransform = nullptr;

		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pMainScene = pSceneManager->GetScene(MainScene).Get();
			TDE2_TEST_IS_TRUE(pMainScene);

			CEntity* pParentEntity = pMainScene->CreateEntity("Parent");
			CEntity* pChildEntity = pMainScene->CreateEntity("Child");

			TDE2_TEST_IS_TRUE(pParentEntity && pChildEntity);

			pParentTransform = pParentEntity->GetComponent<CTransform>();
			pChildTransform = pChildEntity->GetComponent<CTransform>();

			TDE2_TEST_IS_TRUE(pParentTransform && pChildTransform);

			pParentTransform->SetScale(TVector3(0.5f));

			TDE2_TEST_IS_TRUE(RC_OK == GroupEntities(pWorld.Get(), pParentEntity->GetId(), pChildEntity->GetId()));
		});

		/// Wait for a few mseconds to allow CTransformSystem compute new values
		pTestCase->Wait(0.05f);

		pTestCase->ExecuteAction([&]
		{
			TDE2_TEST_IS_TRUE(pParentTransform && pChildTransform);

			TDE2_TEST_IS_TRUE(pParentTransform->GetScale() == TVector3(0.5f));
			TDE2_TEST_IS_TRUE(pChildTransform->GetScale() == TVector3(2.0f));
		});
	}
}