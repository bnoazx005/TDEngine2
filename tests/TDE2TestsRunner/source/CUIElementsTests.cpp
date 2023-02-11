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

	TDE2_TEST_CASE("TestLayoutElementPresets_SwitchBetweenLayoutElementPresetsAndCheckUpValuesInInspector_PresetsValuesShouldBeRelevant")
	{
		static CLayoutElement* pImageLayoutElement = nullptr;

		/// \note Load test canvas and create an image
		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pMainScene = pSceneManager->GetScene(MainScene).Get();
			TDE2_TEST_IS_TRUE(pMainScene);

			auto pCanvasEntity = pMainScene->Spawn("TestCanvas");
			TDE2_TEST_IS_TRUE(pCanvasEntity);

			auto pImageEntity = pMainScene->CreateEntity("Image");
			pImageEntity->AddComponent<CImage>();
			pImageLayoutElement = pImageEntity->AddComponent<CLayoutElement>();

			GroupEntities(pWorld.Get(), pCanvasEntity->GetId(), pImageEntity->GetId());
		});

		/// \todo Replace particular positions with access through identifiers of buttons
		pTestCase->WaitForNextFrame();
		pTestCase->AddPressKey(E_KEYCODES::KC_TILDE); /// Open dev menu

		pTestCase->SetCursorPosition(TVector3(166.0f, 277.0f, 0.0f));
		pTestCase->AddPressMouseButton(0); /// Open the level editor

		pTestCase->SetCursorPosition(TVector3(116.0f, 488.0f, 0.0f));
		pTestCase->AddPressMouseButton(0); /// Open the hierarchy window

		pTestCase->SetCursorPosition(TVector3(860.0f, 304.0f, 0.0f));
		pTestCase->AddPressMouseButton(0); /// Select the canvas entity

		pTestCase->SetCursorPosition(TVector3(860.0f, 270.0f, 0.0f));
		pTestCase->AddPressMouseButton(0); /// Select the Image entity

		/// tuple's args: particlular preset pos, min anchor, max anchor, min offset, max offset
		const std::vector<std::tuple<F32, TVector2, TVector2, TVector2, TVector2>> layoutPresetsTests
		{
			{ 471.0f, TVector2(0.0f, 1.0f), TVector2(0.0f, 1.0f), TVector2(0.0f, -100.0f), TVector2(100.0f) },
			{ 453.0f, TVector2(1.0f), TVector2(1.0f), TVector2(-100.0f), TVector2(100.0f) },
			{ 437.0f, ZeroVector2, ZeroVector2, ZeroVector2, TVector2(100.0f) },
			{ 419.0f, TVector2(1.0f, 0.0f), TVector2(1.0f, 0.0f), TVector2(-100.0f, 0.0f), TVector2(100.0f) },

			{ 398.0f, TVector2(0.0f, 0.5f), TVector2(0.0f, 0.5f), ZeroVector2, TVector2(100.0f) },
			{ 385.0f, TVector2(0.5f, 1.0f), TVector2(0.5f, 1.0f), TVector2(-50.0f, -100.0f), TVector2(100.0f) },
			{ 365.0f, TVector2(1.0f, 0.5f), TVector2(1.0f, 0.5f), TVector2(-100.0f, -50.0f), TVector2(100.0f) },
			{ 345.0f, TVector2(0.5f, 0.0f), TVector2(0.5f, 0.0f), TVector2(-50.0f, 0.0f), TVector2(100.0f) },
			
			{ 325.0f, TVector2(0.5f), TVector2(0.5f), TVector2(-50.0f, -50.0f), TVector2(100.0f) },

			{ 315.0f, ZeroVector2, TVector2(0.0f, 1.0f), ZeroVector2, TVector2(100.0f, 0.0f) },
			{ 295.0f, TVector2(0.5f, 0.0f), TVector2(0.5f, 1.0f), TVector2(-50.0f, 0.0f), TVector2(100.0f, 0.0f) },
			{ 270.0f, TVector2(0.0f, 0.5f), TVector2(1.0f, 0.5f), TVector2(0.0f, -50.0f), TVector2(0.0f, 100.0f)},
			{ 255.0f, TVector2(1.0f, 0.0f), TVector2(1.0f, 1.0f), TVector2(-100.0f, 0.0f), TVector2(100.0f, 0.0f) },

			{ 238.0f, TVector2(0.0f, 1.0f), TVector2(1.0f, 1.0f), TVector2(0.0f, -100.0f), TVector2(0.0f, 100.0f)},
			{ 216.0f, ZeroVector2, TVector2(1.0f, 0.0f), ZeroVector2, TVector2(0.0f, 100.0f) },
			{ 201.0f, ZeroVector2, TVector2(1.0f), ZeroVector2, ZeroVector2 },
		};

		F32 layoutPresetYPos;
		TVector2 minAnchor, maxAnchor, minOffset, maxOffset;

		for (const auto& currTestCase : layoutPresetsTests)
		{
			std::tie(layoutPresetYPos, minAnchor, maxAnchor, minOffset, maxOffset) = currTestCase;

			pTestCase->SetCursorPosition(TVector3(725.0f, 488.0f, 0.0f));
			pTestCase->AddPressMouseButton(0); /// Click over layout preset

			pTestCase->SetCursorPosition(TVector3(744.0f, layoutPresetYPos, 0.0f));
			pTestCase->AddPressMouseButton(0); /// Change the layout preset

			pTestCase->ExecuteAction([=]
			{
				TDE2_TEST_IS_TRUE(pImageLayoutElement->GetMinAnchor() == minAnchor);
				TDE2_TEST_IS_TRUE(pImageLayoutElement->GetMaxAnchor() == maxAnchor);

				TDE2_TEST_IS_TRUE(pImageLayoutElement->GetMinOffset() == minOffset);
				TDE2_TEST_IS_TRUE(pImageLayoutElement->GetMaxOffset() == maxOffset);
			});
		}
	}
}

#endif