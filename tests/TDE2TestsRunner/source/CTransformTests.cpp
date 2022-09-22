#include <TDEngine2.h>


using namespace TDEngine2;


TDE2_TEST_FIXTURE("CTransform Tests")
{
	TDE2_TEST_CASE("TestInitialization_CreateNewTransform_InstanceShouldBeCorrectlyInitialized")
	{
		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pMainScene = pSceneManager->GetScene(MainScene).Get();
			TDE2_TEST_IS_TRUE(pMainScene);

			CEntity* pNewEntity = pMainScene->CreateEntity("Entity");
			TDE2_TEST_IS_TRUE(pNewEntity);

			CTransform* pTransform = pNewEntity->GetComponent<CTransform>();
			TDE2_TEST_IS_TRUE(pTransform);

			TDE2_TEST_IS_TRUE(pTransform->GetPosition() == ZeroVector3);
			TDE2_TEST_IS_TRUE(pTransform->GetRotation() == UnitQuaternion);
			TDE2_TEST_IS_TRUE(pTransform->GetScale() == TVector3(1.0f));

			TDE2_TEST_IS_TRUE(RC_OK == pMainScene->RemoveEntity(pNewEntity->GetId()));
		});
	}

	TDE2_TEST_CASE("TestCompoundTransformations_CreateNewTransformAndChangeItsRotationPosition_InstanceShouldBeCorrectlyPlacedAtScene")
	{
		static const TVector3 randPosition = RandVector3();
		static const TVector3 rotationAngles = CMathConstants::Deg2Rad * TVector3(45.0f, 0.0f, 0.0f);
		
		static CTransform* pTransform = nullptr;

		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pMainScene = pSceneManager->GetScene(MainScene).Get();
			TDE2_TEST_IS_TRUE(pMainScene);

			CEntity* pEntity = pMainScene->CreateEntity("Entity1");
			TDE2_TEST_IS_TRUE(pEntity);

			pTransform = pEntity->GetComponent<CTransform>();
			TDE2_TEST_IS_TRUE(pTransform);

			pTransform->SetPosition(randPosition);
			pTransform->SetRotation(TQuaternion(rotationAngles)); /// From Euler angles

			TDE2_TEST_IS_TRUE(pTransform->HasChanged());
		});

		/// Wait for a few mseconds to allow CTransformSystem compute new values
		pTestCase->WaitForNextFrame();

		pTestCase->ExecuteAction([&]
		{
			TDE2_TEST_IS_TRUE(pTransform->GetPosition() == randPosition);
			TDE2_TEST_IS_TRUE(pTransform->GetRotation() == TQuaternion(rotationAngles));
			TDE2_TEST_IS_TRUE(ToEulerAngles(pTransform->GetRotation()) == rotationAngles);
		});

		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pMainScene = pSceneManager->GetScene(MainScene).Get();
			TDE2_TEST_IS_TRUE(pMainScene);

			TDE2_TEST_IS_TRUE(RC_OK == pMainScene->RemoveEntity(pTransform->GetOwnerId()));
		});
	}

	TDE2_TEST_CASE("TestCompoundTransformations_ChangeRotationThenPosition_TransformShouldHaveCorrectMatrix")
	{
		static const TVector3 position = TVector3(50.0f, 0.0f, 0.0f);
		static const TVector3 rotation = CMathConstants::Deg2Rad * TVector3(45.0f, 0.0f, 0.0f);

		static CTransform* pTransform = nullptr;

		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pMainScene = pSceneManager->GetScene(MainScene).Get();
			TDE2_TEST_IS_TRUE(pMainScene);

			CEntity* pEntity = pMainScene->CreateEntity("Entity1");
			TDE2_TEST_IS_TRUE(pEntity);

			pTransform = pEntity->GetComponent<CTransform>();
			TDE2_TEST_IS_TRUE(pTransform);

			pTransform->SetRotation(TQuaternion(rotation)); /// From Euler angles
		});

		/// Wait for a few mseconds to allow CTransformSystem compute new values
		pTestCase->WaitForNextFrame();

		pTestCase->ExecuteAction([&]
		{
			TDE2_TEST_IS_TRUE(pTransform->GetRotation() == TQuaternion(rotation));
			TDE2_TEST_IS_TRUE(ToEulerAngles(pTransform->GetRotation()) == rotation);

			pTransform->SetPosition(position);
		});

		pTestCase->WaitForNextFrame();

		pTestCase->ExecuteAction([&]
		{
			TDE2_TEST_IS_TRUE(pTransform->GetPosition() == position);

			TDE2_TEST_IS_TRUE(pTransform->GetRotation() == TQuaternion(rotation));
			TDE2_TEST_IS_TRUE(ToEulerAngles(pTransform->GetRotation()) == rotation);
		});

		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pMainScene = pSceneManager->GetScene(MainScene).Get();
			TDE2_TEST_IS_TRUE(pMainScene);

			TDE2_TEST_IS_TRUE(RC_OK == pMainScene->RemoveEntity(pTransform->GetOwnerId()));
		});
	}

	TDE2_TEST_CASE("TestGroupEntities_PassTwoEntitiesGroupThem_AllTransformsShouldBeInCorrectState")
	{
		static CTransform* pParentTransform = nullptr;
		static CTransform* pChildTransform = nullptr;

		/// \note Create a child first specially to guarantee that CTransformSystem will have correct order of components
		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pMainScene = pSceneManager->GetScene(MainScene).Get();
			TDE2_TEST_IS_TRUE(pMainScene);

			CEntity* pChildEntity = pMainScene->CreateEntity("Child");

			TDE2_TEST_IS_TRUE(pChildEntity);

			pChildTransform = pChildEntity->GetComponent<CTransform>();

			TDE2_TEST_IS_TRUE(pChildTransform);
		});

		/// Wait for a few mseconds to allow CTransformSystem compute new values
		pTestCase->WaitForNextFrame();

		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pMainScene = pSceneManager->GetScene(MainScene).Get();
			TDE2_TEST_IS_TRUE(pMainScene);

			CEntity* pParentEntity = pMainScene->CreateEntity("Parent");

			TDE2_TEST_IS_TRUE(pParentEntity);

			pParentTransform = pParentEntity->GetComponent<CTransform>();

			TDE2_TEST_IS_TRUE(pParentTransform);

			pParentTransform->SetScale(TVector3(0.5f));
		});

		/// Wait for a few mseconds to allow CTransformSystem compute new values
		pTestCase->WaitForNextFrame();

		pTestCase->ExecuteAction([&] {
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			TDE2_TEST_IS_TRUE(RC_OK == GroupEntities(pWorld.Get(), pParentTransform->GetOwnerId(), pChildTransform->GetOwnerId()));
		});

		pTestCase->ExecuteAction([&]
		{
			TDE2_TEST_IS_TRUE(pParentTransform && pChildTransform);

			TDE2_TEST_IS_TRUE(pParentTransform->GetScale() == TVector3(0.5f));
			TDE2_TEST_IS_TRUE(pChildTransform->GetScale() == TVector3(2.0f));
		});

		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pMainScene = pSceneManager->GetScene(MainScene).Get();
			TDE2_TEST_IS_TRUE(pMainScene);

			TDE2_TEST_IS_TRUE(RC_OK == pMainScene->RemoveEntity(pParentTransform->GetOwnerId()));
			TDE2_TEST_IS_TRUE(RC_OK == pMainScene->RemoveEntity(pChildTransform->GetOwnerId()));
		});
	}

	TDE2_TEST_CASE("TestGroupEntities_GroupTwoEntitiesAndThenUngroupThem_AllTransformsShouldBeInStateThatWasInitialOne")
	{
		static CTransform* pParentTransform = nullptr;
		static CTransform* pChildTransform = nullptr;

		static const TVector3 parentPosition = TVector3(5.0f, 0.0f, 0.0f);
		static const TVector3 childPosition = TVector3(10.0f, -5.0f, 0.0f);

		/// \note Create a child first specially to guarantee that CTransformSystem will have correct order of components
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
			pParentTransform->SetPosition(parentPosition);
			pChildTransform->SetPosition(childPosition);
		});

		/// Wait for a few mseconds to allow CTransformSystem compute new values
		pTestCase->WaitForNextFrame();
		
		pTestCase->ExecuteAction([&] { /// Group two entities
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			TDE2_TEST_IS_TRUE(RC_OK == GroupEntities(pWorld.Get(), pParentTransform->GetOwnerId(), pChildTransform->GetOwnerId()));
		});

		/// Wait for a few mseconds to allow CTransformSystem compute new values
		pTestCase->WaitForNextFrame();

		pTestCase->ExecuteAction([&] { /// Ungroup these entities
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			TDE2_TEST_IS_TRUE(pParentTransform->GetScale() == TVector3(0.5f));
			TDE2_TEST_IS_TRUE(pChildTransform->GetScale() == TVector3(2.0f));

			TDE2_TEST_IS_TRUE(pParentTransform->GetPosition() == parentPosition);
			TDE2_TEST_IS_TRUE(pChildTransform->GetPosition() == 2.0f * (childPosition - parentPosition));

			TDE2_TEST_IS_TRUE(RC_OK == GroupEntities(pWorld.Get(), TEntityId::Invalid, pChildTransform->GetOwnerId()));
		});

		/// Wait for a few mseconds to allow CTransformSystem compute new values
		pTestCase->WaitForNextFrame();

		pTestCase->ExecuteAction([&]
		{
			TDE2_TEST_IS_TRUE(pParentTransform && pChildTransform);

			TDE2_TEST_IS_TRUE(pParentTransform->GetScale() == TVector3(0.5f));
			TDE2_TEST_IS_TRUE(pChildTransform->GetScale() == TVector3(1.0f));

			TDE2_TEST_IS_TRUE(pParentTransform->GetPosition() == parentPosition);
			TDE2_TEST_IS_TRUE(pChildTransform->GetPosition() == childPosition);
		});

		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pMainScene = pSceneManager->GetScene(MainScene).Get();
			TDE2_TEST_IS_TRUE(pMainScene);

			TDE2_TEST_IS_TRUE(RC_OK == pMainScene->RemoveEntity(pParentTransform->GetOwnerId()));
			TDE2_TEST_IS_TRUE(RC_OK == pMainScene->RemoveEntity(pChildTransform->GetOwnerId()));
		});
	}
}