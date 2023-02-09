#include <TDEngine2.h>


using namespace TDEngine2;


#if TDE2_EDITORS_ENABLED

TDE2_TEST_FIXTURE("UI Elements Tests")
{
	TDE2_TEST_CASE("TestCanvasSpawning_SpawnCanvasThenDestroyThatAndCreateAgain_NoErrorsHappenDuringTheScenario")
	{
		static CEntity* pCanvasEntity = nullptr;

		for (U8 i = 0; i < 2; i++)
		{
			/// \note Create an entity
			pTestCase->ExecuteAction([&]
			{
				IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

				auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
				auto pWorld = pSceneManager->GetWorld();

				auto pMainScene = pSceneManager->GetScene(MainScene).Get();
				TDE2_TEST_IS_TRUE(pMainScene);

				pCanvasEntity = pMainScene->Spawn("TestCanvas");
				TDE2_TEST_IS_TRUE(pCanvasEntity);
			});

			pTestCase->WaitForNextFrame();

			/// \note Destroy the canvas entity
			pTestCase->ExecuteAction([&]
			{
				IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

				auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
				auto pWorld = pSceneManager->GetWorld();

				auto pMainScene = pSceneManager->GetScene(MainScene).Get();
				TDE2_TEST_IS_TRUE(pMainScene);

				TDE2_TEST_IS_TRUE(RC_OK == pMainScene->RemoveEntity(pCanvasEntity->GetId()));
			});

			pTestCase->WaitForNextFrame();
		}

	}

	TDE2_TEST_CASE("TestFullScreenImageRect_SpawnCanvasWithImage_TheImageShouldFillTheScreen")
	{
		static CEntity* pCanvasEntity = nullptr;

		/// \note Create an entity
		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pMainScene = pSceneManager->GetScene(MainScene).Get();
			TDE2_TEST_IS_TRUE(pMainScene);

			pCanvasEntity = pMainScene->Spawn("TestCanvas_WithImage");
			TDE2_TEST_IS_TRUE(pCanvasEntity);
		});

		pTestCase->WaitForNextFrame();

		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pCanvas = pCanvasEntity->GetComponent<CCanvas>();
			
			for (const TEntityId& currEntityId : pWorld->FindEntitiesWithComponents<CImage>())
			{
				CEntity* pEntity = pWorld->FindEntity(currEntityId);
				auto pImageLayoutElement = pEntity->GetComponent<CLayoutElement>();
				
				const auto& worldRect = pImageLayoutElement->GetWorldRect();

				TDE2_TEST_IS_TRUE(CMathUtils::Abs(worldRect.width - static_cast<F32>(pCanvas->GetWidth())) < 1e-3f);
				TDE2_TEST_IS_TRUE(CMathUtils::Abs(worldRect.height - static_cast<F32>(pCanvas->GetHeight())) < 1e-3f);
			}
		});

		/// \note Destroy the canvas entity
		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pMainScene = pSceneManager->GetScene(MainScene).Get();
			TDE2_TEST_IS_TRUE(pMainScene);

			TDE2_TEST_IS_TRUE(RC_OK == pMainScene->RemoveEntity(pCanvasEntity->GetId()));
		});
	}
}

#endif