#include <TDEngine2.h>
#include <array>


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
		static CEntity* pCanvasEntity = nullptr;
		static CEntity* pImageEntity = nullptr;

		/// \note Load test canvas and create an image
		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pMainScene = pSceneManager->GetScene(MainScene).Get();
			TDE2_TEST_IS_TRUE(pMainScene);

			pCanvasEntity = pMainScene->Spawn("TestCanvas");
			TDE2_TEST_IS_TRUE(pCanvasEntity);

			pImageEntity = pMainScene->CreateEntity("Image");
			pImageEntity->AddComponent<CImage>();
			pImageLayoutElement = pImageEntity->AddComponent<CLayoutElement>();

			GroupEntities(pWorld.Get(), pCanvasEntity->GetId(), pImageEntity->GetId());
		});

		/// \todo Replace particular positions with access through identifiers of buttons
		pTestCase->WaitForNextFrame();
		pTestCase->AddPressKey(E_KEYCODES::KC_TILDE); /// Open dev menu

		pTestCase->SetCursorAtUIElement("Development Menu/Show Level Editor");
		pTestCase->AddPressMouseButton(0); /// Open the level editor

		pTestCase->SetCursorAtUIElement("Level Editor/Show Hierarchy");
		pTestCase->AddPressMouseButton(0); /// Open the hierarchy window

		pTestCase->WaitForNextFrame();
		pTestCase->SetCursorAtUIElement([&]
		{
			return Wrench::StringUtils::Format("Scene Hierarchy/{0}##{1}", pCanvasEntity->GetName(), static_cast<U32>(pCanvasEntity->GetId()));
		});

		pTestCase->AddPressMouseButton(0); /// Select the canvas entity

		pTestCase->WaitForNextFrame();
		pTestCase->SetCursorAtUIElement([&]
		{
			return Wrench::StringUtils::Format("Scene Hierarchy/{0}##{1}/{2}##{3}",
				pCanvasEntity->GetName(), static_cast<U32>(pCanvasEntity->GetId()),
				pImageEntity->GetName(), static_cast<U32>(pImageEntity->GetId()));
		});

		pTestCase->AddPressMouseButton(0); /// Select the Image entity
		
		constexpr F32 yOffset = -20;
		
		/// tuple's args: particlular preset pos, min anchor, max anchor, min offset, max offset
		const std::vector<std::tuple<std::string, TVector2, TVector2, TVector2, TVector2>> layoutPresetsTests
		{
			{ "Top Left", TVector2(0.0f, 1.0f), TVector2(0.0f, 1.0f), TVector2(0.0f, -100.0f), TVector2(100.0f) },
			{ "Top Right", TVector2(1.0f), TVector2(1.0f), TVector2(-100.0f), TVector2(100.0f) },
			{ "Bottom Left", ZeroVector2, ZeroVector2, ZeroVector2, TVector2(100.0f) },
			{ "Bottom Right", TVector2(1.0f, 0.0f), TVector2(1.0f, 0.0f), TVector2(-100.0f, 0.0f), TVector2(100.0f) },

			{ "Center Left", TVector2(0.0f, 0.5f), TVector2(0.0f, 0.5f), ZeroVector2, TVector2(100.0f) },
			{ "Center Top", TVector2(0.5f, 1.0f), TVector2(0.5f, 1.0f), TVector2(-50.0f, -100.0f), TVector2(100.0f) },
			{ "Center Right", TVector2(1.0f, 0.5f), TVector2(1.0f, 0.5f), TVector2(-100.0f, -50.0f), TVector2(100.0f) },
			{ "Center Bottom", TVector2(0.5f, 0.0f), TVector2(0.5f, 0.0f), TVector2(-50.0f, 0.0f), TVector2(100.0f) },
			
			{ "Center", TVector2(0.5f), TVector2(0.5f), TVector2(-50.0f, -50.0f), TVector2(100.0f) },

			{ "Left Wide", ZeroVector2, TVector2(0.0f, 1.0f), ZeroVector2, TVector2(100.0f, 0.0f) },
			{ "VCenter Wide", TVector2(0.5f, 0.0f), TVector2(0.5f, 1.0f), TVector2(-50.0f, 0.0f), TVector2(100.0f, 0.0f) },
			{ "HCenter Wide", TVector2(0.0f, 0.5f), TVector2(1.0f, 0.5f), TVector2(0.0f, -50.0f), TVector2(0.0f, 100.0f)},
			{ "Right Wide", TVector2(1.0f, 0.0f), TVector2(1.0f, 1.0f), TVector2(-100.0f, 0.0f), TVector2(100.0f, 0.0f) },

			{ "Top Wide", TVector2(0.0f, 1.0f), TVector2(1.0f, 1.0f), TVector2(0.0f, -100.0f), TVector2(0.0f, 100.0f)},
			{ "Bottom Wide", ZeroVector2, TVector2(1.0f, 0.0f), ZeroVector2, TVector2(0.0f, 100.0f) },
			{ "Full Rect", ZeroVector2, TVector2(1.0f), ZeroVector2, ZeroVector2 },
		};

		std::string currPresetName;
		TVector2 minAnchor, maxAnchor, minOffset, maxOffset;

		for (const auto& currTestCase : layoutPresetsTests)
		{
			std::tie(currPresetName, minAnchor, maxAnchor, minOffset, maxOffset) = currTestCase;

			pTestCase->WaitForNextFrame();
			pTestCase->SetCursorAtUIElement("Object Inspector/##LayoutPresetButton");
			pTestCase->AddPressMouseButton(0); /// Click over layout preset
			
			pTestCase->SetCursorAtUIElement(Wrench::StringUtils::Format("##Popup_9e0ca471/{0}##Button", currPresetName));
			pTestCase->AddPressMouseButton(0); /// Change the layout preset
			
			//pTestCase->TakeScreenshot();

			pTestCase->ExecuteAction([=]
			{
				TDE2_TEST_IS_TRUE(pImageLayoutElement->GetMinAnchor() == minAnchor);
				TDE2_TEST_IS_TRUE(pImageLayoutElement->GetMaxAnchor() == maxAnchor);

				TDE2_TEST_IS_TRUE(pImageLayoutElement->GetMinOffset() == minOffset);
				TDE2_TEST_IS_TRUE(pImageLayoutElement->GetMaxOffset() == maxOffset);
			});
		}

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

		pTestCase->AddPressKey(E_KEYCODES::KC_TILDE); /// Open dev menu
	}

	TDE2_TEST_CASE("TestGridLayoutGroup_AddGridGroupLayoutOntoRootOfEntities_TheComponentShouldHaveCorrectDefaultState")
	{
		static CEntity* pCanvasEntity = nullptr;
		static CEntity* pGridRootEntity = nullptr;

		auto prepareGridElement = [](CEntity* pEntity)
		{
			auto pLayout = pEntity->AddComponent<CLayoutElement>();
		};

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

			pGridRootEntity = pMainScene->CreateEntity("GridRoot");
			
			auto pGridRootElement = pGridRootEntity->AddComponent<CLayoutElement>();
			
			CEntity* pGridElement01Entity = pMainScene->CreateEntity("GridElement01");
			prepareGridElement(pGridElement01Entity);

			CEntity* pGridElement02Entity = pMainScene->CreateEntity("GridElement02");
			prepareGridElement(pGridElement02Entity);

			/*!
				\brief The hierarchy looks like the following

				Canvas
					GridRoot
						GridElement01
						GridElement02
			*/

			TDE2_TEST_IS_TRUE(RC_OK == GroupEntities(pWorld.Get(), pGridRootEntity->GetId(), pGridElement01Entity->GetId()));
			TDE2_TEST_IS_TRUE(RC_OK == GroupEntities(pWorld.Get(), pGridRootEntity->GetId(), pGridElement02Entity->GetId()));
			TDE2_TEST_IS_TRUE(RC_OK == GroupEntities(pWorld.Get(), pCanvasEntity->GetId(), pGridRootEntity->GetId()));
		});

		pTestCase->WaitForNextFrame();

		pTestCase->ExecuteAction([&]
		{
			/// Add CGridLayoutGroup component onto the root
			auto pGridGroupLayout = pGridRootEntity->AddComponent<CGridGroupLayout>();
			TDE2_TEST_IS_TRUE(pGridGroupLayout);
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

	TDE2_TEST_CASE("TestGridLayoutGroup_AddGridGroupLayoutThenCreateEmptyChildEntity_NoAssertsAndExceptionsHappen")
	{
		static CEntity* pCanvasEntity = nullptr;
		static CEntity* pGridRootEntity = nullptr;

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

			pGridRootEntity = pMainScene->CreateEntity("GridRoot");
			
			pGridRootEntity->AddComponent<CLayoutElement>();
			pGridRootEntity->AddComponent<CGridGroupLayout>();
			
			TDE2_TEST_IS_TRUE(RC_OK == GroupEntities(pWorld.Get(), pCanvasEntity->GetId(), pGridRootEntity->GetId()));

			/// \note Create an empty child
			auto pEntity = pMainScene->CreateEntity("EmptyChild");
			TDE2_TEST_IS_TRUE(RC_OK == GroupEntities(pWorld.Get(), pGridRootEntity->GetId(), pEntity->GetId()));
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
	}

	TDE2_TEST_CASE("TestInputEvents_SpawnCanvasWithSingleButton_ButtonShouldReactOnlyIfPressExecutedOverIt")
	{
		static CEntity* pCanvasEntity = nullptr;
		static CEntity* pButtonEntity = nullptr;

		/// \note Create a button
		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pMainScene = pSceneManager->GetScene(MainScene).Get();
			TDE2_TEST_IS_TRUE(pMainScene);

			auto&& canvasEntityResult = CSceneHierarchyUtils::CreateCanvasUIElement(pWorld, pMainScene, TEntityId::Invalid, [](auto) {});
			TDE2_TEST_IS_TRUE(canvasEntityResult.IsOk());

			pCanvasEntity = pWorld->FindEntity(canvasEntityResult.Get());

			auto&& buttonEntityResult = CSceneHierarchyUtils::CreateButtonUIElement(pWorld, pMainScene, canvasEntityResult.Get(), [](auto) {});
			TDE2_TEST_IS_TRUE(buttonEntityResult.IsOk());

			pButtonEntity = pWorld->FindEntity(buttonEntityResult.Get());
		});

		pTestCase->WaitForNextFrame();

		pTestCase->SetCursorPosition(TVector3(50.0f, 15.0f, 0.0f)); /// click over the button
		pTestCase->AddPressMouseButton(0);

		pTestCase->ExecuteAction([&]
		{
			if (auto pInputReceiver = pButtonEntity->GetComponent<CInputReceiver>())
			{
				TDE2_TEST_IS_TRUE(pInputReceiver->mIsHovered);
				TDE2_TEST_IS_TRUE(!pInputReceiver->mCurrState);
				TDE2_TEST_IS_TRUE(pInputReceiver->mPrevState);
			}
		});

		pTestCase->WaitForNextFrame();

		pTestCase->SetCursorPosition(TVector3(550.0f, 155.0f, 0.0f)); /// click somewhere over screen except the button
		pTestCase->AddPressMouseButton(0);

		pTestCase->ExecuteAction([&]
		{
			if (auto pInputReceiver = pButtonEntity->GetComponent<CInputReceiver>())
			{
				TDE2_TEST_IS_TRUE(!pInputReceiver->mIsHovered);
				TDE2_TEST_IS_TRUE(!pInputReceiver->mCurrState);
				TDE2_TEST_IS_TRUE(!pInputReceiver->mPrevState);
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

		pTestCase->WaitForNextFrame();
	}

	TDE2_TEST_CASE("TestUiMask_AddTwoImagesOneOfThemIsUiMask_ChildImageIsDrawnPartiallyBasedOnMask")
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

			// create a canvas
			auto&& canvasEntityResult = CSceneHierarchyUtils::CreateCanvasUIElement(pWorld, pMainScene, TEntityId::Invalid, [](auto) {});
			TDE2_TEST_IS_TRUE(canvasEntityResult.IsOk());

			pCanvasEntity = pWorld->FindEntity(canvasEntityResult.Get());

			// create a mask
			auto&& maskEntityResult = CSceneHierarchyUtils::CreateImageUIElement(pWorld, pMainScene, canvasEntityResult.Get(), [](auto) {});
			TDE2_TEST_IS_TRUE(maskEntityResult.IsOk());

			if (auto pMaskEntity = pWorld->FindEntity(maskEntityResult.Get()))
			{
				pMaskEntity->AddComponent<CUIMaskComponent>();
			}

			// create child image
			auto&& contentImageEntityResult = CSceneHierarchyUtils::CreateImageUIElement(pWorld, pMainScene, maskEntityResult.Get(), [](auto) {});
			TDE2_TEST_IS_TRUE(contentImageEntityResult.IsOk());

			if (auto pImageEntity = pWorld->FindEntity(contentImageEntityResult.Get()))
			{
				if (auto pLayoutElement = pImageEntity->GetComponent<CLayoutElement>())
				{
					TVector2 offset = pLayoutElement->GetMaxOffset();
					pLayoutElement->SetMinOffset(TVector2(offset.x * 0.5f, -offset.y * 0.5f));
				}
			}
		});

		pTestCase->Wait(1.0f);

		pTestCase->ExecuteAction([&]
		{
			TDE2_TEST_IS_TRUE(PackColor32F(CTestContext::Get()->GetFrameBufferPixel(35, 35)) == 0x000000ff);
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

	TDE2_TEST_CASE("TestUiMask_AddButtonThatPartiallyMasked_InputShouldBeReceivedOnlyOnVisiblePartOfButton")
	{
		static CEntity* pCanvasEntity = nullptr;
		static CInputReceiver* pInputReceiver = nullptr;

		/// \note Create an entity
		pTestCase->ExecuteAction([&]
		{
			IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

			auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
			auto pWorld = pSceneManager->GetWorld();

			auto pMainScene = pSceneManager->GetScene(MainScene).Get();
			TDE2_TEST_IS_TRUE(pMainScene);

			// create a canvas
			auto&& canvasEntityResult = CSceneHierarchyUtils::CreateCanvasUIElement(pWorld, pMainScene, TEntityId::Invalid, [](auto) {});
			TDE2_TEST_IS_TRUE(canvasEntityResult.IsOk());

			pCanvasEntity = pWorld->FindEntity(canvasEntityResult.Get());

			// create a mask
			auto&& maskEntityResult = CSceneHierarchyUtils::CreateImageUIElement(pWorld, pMainScene, canvasEntityResult.Get(), [](auto) {});
			TDE2_TEST_IS_TRUE(maskEntityResult.IsOk());

			if (auto pMaskEntity = pWorld->FindEntity(maskEntityResult.Get()))
			{
				pMaskEntity->AddComponent<CUIMaskComponent>();
			}

			// create a button 
			auto&& contentButtonEntityResult = CSceneHierarchyUtils::CreateImageUIElement(pWorld, pMainScene, maskEntityResult.Get(), [](auto) {});
			TDE2_TEST_IS_TRUE(contentButtonEntityResult.IsOk());

			if (auto pButtonEntity = pWorld->FindEntity(contentButtonEntityResult.Get()))
			{
				if (auto pLayoutElement = pButtonEntity->GetComponent<CLayoutElement>())
				{
					TVector2 offset = pLayoutElement->GetMaxOffset();
					pLayoutElement->SetMinOffset(TVector2(offset.x * 0.5f, -offset.y * 0.5f));
				}

				pInputReceiver = pButtonEntity->AddComponent<CInputReceiver>();
			}
		});

		pTestCase->Wait(1.0f);

		// Try to click over visible part
		pTestCase->SetCursorPosition(TVector3(80.0f, 35.0f, 0.0f));
		pTestCase->AddPressMouseButton(0);

		pTestCase->ExecuteAction([&]
		{
			TDE2_TEST_IS_TRUE(pInputReceiver);
			TDE2_TEST_IS_TRUE(pInputReceiver->mIsHovered && (pInputReceiver->mCurrState || pInputReceiver->mPrevState));
		});

		pTestCase->WaitForNextFrame();

		// Try to click over hidden part
		pTestCase->SetCursorPosition(TVector3(120.0f, 35.0f, 0.0f));
		pTestCase->AddPressMouseButton(0);

		pTestCase->ExecuteAction([&]
		{
			TDE2_TEST_IS_TRUE(pInputReceiver);
			TDE2_TEST_IS_TRUE(!pInputReceiver->mIsHovered && !pInputReceiver->mCurrState && !pInputReceiver->mPrevState);
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
	}

	//TDE2_TEST_CASE("TestGridGroupLayout_Spawn4PrefabsWithGridGroup_ChildrenShouldBeCorrectlyPositionedInAnyGroupPosition")
	//{
	//	static CEntity* pCanvasEntity = nullptr;

	//	/// \note Spawn prefabs
	//	pTestCase->ExecuteAction([&]
	//	{
	//		IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

	//		auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
	//		auto pWorld = pSceneManager->GetWorld();

	//		auto pMainScene = pSceneManager->GetScene(MainScene).Get();
	//		TDE2_TEST_IS_TRUE(pMainScene);

	//		// create a canvas
	//		auto&& canvasEntityResult = CSceneHierarchyUtils::CreateCanvasUIElement(pWorld, pMainScene, TEntityId::Invalid, [](auto) {});
	//		TDE2_TEST_IS_TRUE(canvasEntityResult.IsOk());

	//		pCanvasEntity = pWorld->FindEntity(canvasEntityResult.Get());

	//		const std::array<std::tuple<TVector2, TVector2, TVector2, TVector2>, 4> settings
	//		{
	//			std::make_tuple(ZeroVector2, TVector2(260.0f, 260.0f), ZeroVector2, ZeroVector2),
	//			std::make_tuple(TVector2(-260.0f, 0.0f), TVector2(250.0f, 260.0f), TVector2(1.0f, 0.0f), ZeroVector2),
	//			std::make_tuple(TVector2(0.0f, -260.0f), TVector2(260.0f, 0.0f), TVector2(0.0f, 1.0f), ZeroVector2),
	//			std::make_tuple(TVector2(-260.0f, 0.0f), TVector2(0.0f, -260.0f), TVector2(1.0f), TVector2(1.0f)),
	//		};

	//		TVector2 minOffset, maxOffset, minAnchor, maxAnchor;

	//		for (USIZE i = 0; i < settings.size(); i++)
	//		{
	//			std::tie(minOffset, maxOffset, minAnchor, maxAnchor) = settings[i];

	//			CEntity* pGridGroupRootEntity = pMainScene->Spawn("TestSimpleGridGroup");

	//			if (auto pLayoutElement = pGridGroupRootEntity->GetComponent<CLayoutElement>())
	//			{
	//				pLayoutElement->SetMinOffset(minOffset);
	//				pLayoutElement->SetMaxOffset(maxOffset);
	//				pLayoutElement->SetMinAnchor(minAnchor);
	//				pLayoutElement->SetMaxAnchor(maxAnchor);
	//			}

	//			GroupEntities(pWorld.Get(), pCanvasEntity->GetId(), pGridGroupRootEntity->GetId());
	//		}
	//	});

	//	pTestCase->WaitForNextFrame();
	//	pTestCase->TakeScreenshot();

	//	/// \note Destroy the canvas entity
	//	pTestCase->ExecuteAction([&]
	//	{
	//		IEngineCore* pEngineCore = CTestContext::Get()->GetEngineCore();

	//		auto pSceneManager = pEngineCore->GetSubsystem<ISceneManager>();
	//		auto pWorld = pSceneManager->GetWorld();

	//		auto pMainScene = pSceneManager->GetScene(MainScene).Get();
	//		TDE2_TEST_IS_TRUE(pMainScene);

	//		TDE2_TEST_IS_TRUE(RC_OK == pMainScene->RemoveEntity(pCanvasEntity->GetId()));
	//	});
	//}
}

#endif