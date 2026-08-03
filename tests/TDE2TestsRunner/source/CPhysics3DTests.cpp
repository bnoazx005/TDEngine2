#include <TDEngine2.h>


using namespace TDEngine2;


#if TDE2_EDITORS_ENABLED


TDE2_TEST_FIXTURE("Physics3D Tests")
{
	TDE2_TEST_CASE("TestPhysics3DSystem_CreateKinematicAndDynamicPhysicsBodies_FirstOneShouldStayAtSamePlaceAnotherOneGoDownDueToGravityForce")
	{
		static CEntity* pKinematicEntity = nullptr;
		static CEntity* pDynamicEntity = nullptr;

		static TVector3 kinematicOriginalPosition = ZeroVector3;
		static TVector3 dynamicOriginalPosition = ZeroVector3;

		/// \note Load a prefab with nested prefabs with changes list
		pTestCase->ExecuteAction([&]
			{
				IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

				auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
				auto pWorld = pSceneManager->GetWorld();

				auto pMainScene = pSceneManager->GetScene(MainScene).Get();
				TDE2_TEST_IS_TRUE(pMainScene);

				pKinematicEntity = pMainScene->CreateEntity("Kinematic");
				TDE2_TEST_IS_TRUE(pKinematicEntity);

				pKinematicEntity->AddComponent<CBoxCollisionObject3D>();

				IPhysicsBody3D* pKinematicObject = pKinematicEntity->AddComponent<IPhysicsBody3D>();
				TDE2_TEST_IS_TRUE(pKinematicObject);

				pKinematicObject->SetCollisionType(E_COLLISION_OBJECT_TYPE::COT_KINEMATIC);

				kinematicOriginalPosition = pKinematicEntity->GetComponent<CTransform>()->GetPosition();

				pDynamicEntity = pMainScene->CreateEntity("Dynamic");
				TDE2_TEST_IS_TRUE(pDynamicEntity);

				pDynamicEntity->AddComponent<CBoxCollisionObject3D>();

				IPhysicsBody3D* pDynamicObject = pDynamicEntity->AddComponent<IPhysicsBody3D>();
				TDE2_TEST_IS_TRUE(pDynamicObject);

				pDynamicObject->SetCollisionType(E_COLLISION_OBJECT_TYPE::COT_DYNAMIC);

				dynamicOriginalPosition = pKinematicEntity->GetComponent<CTransform>()->GetPosition();
			});

		pTestCase->Wait(5.0f);

		pTestCase->ExecuteAction([&]
			{
				IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

				auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
				auto pWorld = pSceneManager->GetWorld();

				TDE2_TEST_IS_TRUE(kinematicOriginalPosition == pKinematicEntity->GetComponent<CTransform>()->GetPosition());

				const TVector3& dynamicObjectCurrentPosition = pDynamicEntity->GetComponent<CTransform>()->GetPosition();
				TDE2_TEST_IS_TRUE(dynamicOriginalPosition != dynamicObjectCurrentPosition && dynamicObjectCurrentPosition.y < dynamicOriginalPosition.y);
			});

		pTestCase->ExecuteAction([&]
			{
				IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

				auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
				auto pWorld = pSceneManager->GetWorld();

				auto pMainScene = pSceneManager->GetScene(MainScene).Get();

				TDE2_TEST_IS_TRUE(RC_OK == pMainScene->RemoveEntity(pKinematicEntity->GetId()));
				TDE2_TEST_IS_TRUE(RC_OK == pMainScene->RemoveEntity(pDynamicEntity->GetId()));
			});
	}
}

#endif