#include <TDEngine2.h>
#include "deferOperation.hpp"


using namespace TDEngine2;


#if TDE2_EDITORS_ENABLED

TDE2_TEST_FIXTURE("CameraFrustumTests")
{
	TDE2_TEST_CASE("TestCheckBounds_CheckSkyboxVisibility_SkyboxShouldBeVisibleAtAnyCameraTransformState")
	{
		static CEntity* pSkyboxEntity = nullptr;
		static CEntity* pCameraEntity = nullptr;

		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pMainScene = pSceneManager->GetScene(MainScene).Get();
			TDE2_TEST_IS_TRUE(pMainScene);

			pSkyboxEntity = pMainScene->CreateSkybox(pEngineCore->GetSubsystem<IResourceManager>().Get(), "DefaultResources/Textures/DefaultSkybox");

			if (CEntity* pCamerasContextEntity = pWorld->FindEntity(pWorld->FindEntityWithUniqueComponent<CCamerasContextComponent>()))
			{
				if (auto pCamerasContext = pCamerasContextEntity->GetComponent<CCamerasContextComponent>())
				{
					pCameraEntity = pWorld->FindEntity(pCamerasContext->GetActiveCameraEntityId());
				}
			}

			TDE2_TEST_IS_TRUE(pCameraEntity);
		});

		pTestCase->Wait(5.0f);

		std::vector<std::tuple<TVector3, TQuaternion>> cameraOrientations
		{
			{ ZeroVector3, UnitQuaternion },
			{ TVector3(0.0f, 10000.0f, 0.0f), TQuaternion(TVector3(CMathConstants::Pi * 0.5f, 0.0f, 0.0f)) },
			{ TVector3(0.0f, 10000.0f, 100000.0f), UnitQuaternion },
		};

		for (auto&& currTestValues : cameraOrientations)
		{
			pTestCase->ExecuteAction([currTestValues]
				{
					IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

					auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
					auto pWorld = pSceneManager->GetWorld();

					CTransform* pCameraTransform = pCameraEntity->GetComponent<CTransform>();

					pCameraTransform->SetPosition(std::get<TVector3>(currTestValues));
					pCameraTransform->SetRotation(std::get<TQuaternion>(currTestValues));
				});

			pTestCase->WaitForNextFrame();

			// \note Check whether the skybox is visible or not
			pTestCase->ExecuteAction([currTestValues]
				{
					IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

					auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
					auto pWorld = pSceneManager->GetWorld();

					TDE2_TEST_IS_TRUE(PackColor32F(CTestContext::Get()->GetFrameBufferPixel(35, 35)) != 0x000000ff);
				});

		}

		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();
			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();

			auto pMainScene = pSceneManager->GetScene(MainScene).Get();
			TDE2_TEST_IS_TRUE(pMainScene);

			TDE2_TEST_IS_TRUE(RC_OK == pMainScene->RemoveEntity(pSkyboxEntity->GetId()));
		});
	}
}

#endif