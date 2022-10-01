#include "../include/CCustomEngineListener.h"
#include <TDEngine2.h>
#include <iostream>


using namespace TDEngine2;


static const std::string DefaultSpriteId = "ProjectResources/Textures/DefaultSprite.tga";


E_RESULT_CODE CCustomEngineListener::OnStart()
{
	E_RESULT_CODE result = RC_OK;

	mpWorld = mpEngineCoreInstance->GetWorldInstance();

	auto pBufferPtr = mpGraphicsObjectManager->CreateStructuredBuffer({ mpGraphicsContext, E_STRUCTURED_BUFFER_TYPE::DEFAULT, E_BUFFER_USAGE_TYPE::BUT_DEFAULT, 30, sizeof(TVector2), false }).Get();

	auto pComputeShader = mpResourceManager->GetResource<IShader>(mpResourceManager->Load<IShader>("ProjectShaders/TestCompute.cshader"));
	//pComputeShader->SetTextureResource()

	mpResourceManager->Load<IPrefabsManifest>("ProjectResources/PrefabsCollection.manifest");

	auto pFontMaterial = mpResourceManager->GetResource<IMaterial>(mpResourceManager->Load<IMaterial>("DefaultResources/Materials/DebugTextMaterial.material"));
	if (pFontMaterial)
	{
		if (auto pFontAtlas = mpResourceManager->GetResource<ITextureAtlas>(mpResourceManager->Load<ITextureAtlas>("atlas.info")))
		{
			pFontMaterial->SetTextureResource("FontTextureAtlas", pFontAtlas->GetTexture());
		}
	}

	auto pCloneMaterial = pFontMaterial->Clone();

	if (auto pMaterial = mpResourceManager->GetResource<IMaterial>(mpResourceManager->Load<IMaterial>("ProjectResources/Materials/DefaultMaterialWithParallax.material")))
	{
		pMaterial->SetVariableForInstance(DefaultMaterialInstanceId, "parallaxMappingEnabled", 1.0f);
	}

	// \note create a trigger
	auto pTriggerEntity = mpWorld->CreateEntity();
	auto pTransform = pTriggerEntity->GetComponent<CTransform>();
	pTransform->SetPosition({ 0.0f, -1.0f, 1.0f });
	pTransform->SetScale({ 10.0f, 1.0f, 1.0f });
	auto collider = pTriggerEntity->AddComponent<CBoxCollisionObject2D>();
	collider->SetCollisionType(E_COLLISION_OBJECT_TYPE::COT_STATIC);
	pTriggerEntity->AddComponent<CTrigger2D>();

	//mpResourceManager->Load<IStaticMesh>("hq.mesh");
	mpWorld->CreateLocalComponentsSlice<CTransform, CLabel>();

#if 1 /// Create a static mesh
	auto pMeshEntity = mpWorld->CreateEntity();
	auto shadowCaster = pMeshEntity->AddComponent<CShadowCasterComponent>();
	auto sr = pMeshEntity->AddComponent<CShadowReceiverComponent>();
	auto bounds = pMeshEntity->AddComponent<CBoundsComponent>();
	auto pMeshTransform = pMeshEntity->GetComponent<CTransform>();
	pMeshTransform->SetPosition({ 0.0f, -0.2f, 2.0f });
	auto pMeshContainer = pMeshEntity->AddComponent<CStaticMeshContainer>();
	pMeshContainer->SetMaterialName("ProjectResources/Materials/DefaultMaterial.material");
	pMeshContainer->SetMeshName("ProjectResources/Models/scene.mesh");
	
#if 0 // LODs test
	pMeshContainer->SetMeshName("ProjectResources/Models/LOD_test.mesh");
	if (auto lodStrategy = pMeshEntity->AddComponent<CLODStrategyComponent>())
	{
		lodStrategy->AddLODInstance({ 5.0f, "ProjectResources/Models/LOD_test.mesh", "Cube", "ProjectResources/Materials/DefaultMaterial.material", E_LOD_INSTANCE_ACTIVE_PARAMS::SUBMESH_ID });
		lodStrategy->AddLODInstance({ 10.0f, "ProjectResources/Models/LOD_test.mesh", "Cube_LOD", "ProjectResources/Materials/DefaultMaterial.material", E_LOD_INSTANCE_ACTIVE_PARAMS::SUBMESH_ID });
	}
#endif
#if 0
	auto collision = pMeshEntity->AddComponent<CBoxCollisionObject3D>();
	collision->SetCollisionType(E_COLLISION_OBJECT_TYPE::COT_DYNAMIC);
#endif
#endif

	// plane
#if 1
	auto pPlaneEntity = mpWorld->CreateEntity();
	auto sr2 = pPlaneEntity->AddComponent<CShadowReceiverComponent>();
	auto pPlaneMeshContainer = pPlaneEntity->AddComponent<CStaticMeshContainer>();
	pPlaneMeshContainer->SetMaterialName("ProjectResources/Materials/DefaultMaterial.material");
	pPlaneMeshContainer->SetMeshName("Plane");
	if (auto pTransform = pPlaneEntity->GetComponent<CTransform>())
	{
		pTransform->SetPosition(TVector3(0.0f, 1.5f, 0.0f));
	}
#endif

#if 0
	// Create a 3D trigger
	if (auto p3DTrigger = mpWorld->CreateEntity())
	{
		if (auto pTransform = p3DTrigger->GetComponent<CTransform>())
		{
			pTransform->SetPosition({ 0.0f, 0.0f, 1.0f });
			pTransform->SetScale({ 25.0f, 0.45f, 25.0f });
		}

		auto p3DTriggerCollider = p3DTrigger->AddComponent<CBoxCollisionObject3D>();
		p3DTriggerCollider->SetCollisionType(E_COLLISION_OBJECT_TYPE::COT_KINEMATIC);
		p3DTriggerCollider->SetSizes(TVector3(25.0f, 1.0f, 25.0f));
		//p3DTrigger->AddComponent<CTrigger3D>();
	}
#endif

	// TEMP CODE
	{
		ISceneManager* pSceneManager = mpEngineCoreInstance->GetSubsystem<ISceneManager>().Get();

		if (auto pMainScene = pSceneManager->GetScene(MainScene).Get())
		{
			TPerspectiveCameraParameters cameraParams;
			cameraParams.mAspect = static_cast<F32>(mpWindowSystem->GetWidth() / mpWindowSystem->GetHeight());
			cameraParams.mFOV    = 0.5f * CMathConstants::Pi;
			cameraParams.mZNear  = 0.1f;
			cameraParams.mZFar   = 1000.0f;

			if (auto pCameraEntity = pMainScene->CreateCamera("MainCamera", E_CAMERA_PROJECTION_TYPE::PERSPECTIVE, cameraParams))
			{
				pCameraEntity->AddComponent<CAudioListenerComponent>();

				if (auto pCamerasContextEntity = mpWorld->FindEntity(mpWorld->FindEntityWithUniqueComponent<CCamerasContextComponent>()))
				{
					if (auto pCamerasContext = pCamerasContextEntity->GetComponent<CCamerasContextComponent>())
					{
						pCamerasContext->SetActiveCameraEntity(pCameraEntity->GetId());
					}
				}
			}

			pMainScene->Spawn("TestPrefab");
		}

#if 0
		if (auto result = mpFileSystem->Open<IYAMLFileWriter>("ProjectResources/Scenes/TestScene.scene", true))
		{
			if (auto pFileWriter = mpFileSystem->Get<IYAMLFileWriter>(result.Get()))
			{
				if (auto pScene = pSceneManager->GetScene(pSceneManager->CreateScene("Test").Get()).Get())
				{
					if (auto pLight = pScene->CreateDirectionalLight(TColor32F(1.0f, 1.0f, 1.0f, 1.0f), 1.5f, TVector3(1.0f, 1.0f, 0.0f)))
					{
						pLight->GetComponent<CTransform>()->SetPosition(SunLightPos);
					}

					for (I32 i = 0; i < 10; ++i)
					{
						auto pEnt = pScene->CreateEntity(Wrench::StringUtils::Format("Entity{0}", i));

						if (auto pTransform = pEnt->GetComponent<CTransform>())
						{
							pTransform->SetPosition(pTransform->GetPosition() + RandVector3({ -2.0f, 0.0f, 0.0f }, { 5.0f, 5.0f, 10.0f }));
						}

						if (auto pSprite = pEnt->AddComponent<CQuadSprite>())
						{
							pSprite->SetMaterialName("NewMaterial.material");
							pSprite->SetColor(colors[rand() % 7]);
						}

						//auto pBoxCollision = pEntity->AddComponent<CBoxCollisionObject2D>();
					}

					pScene->Save(pFileWriter);
					pFileWriter->Close();
				}
			}
		}
#endif

#if 0 // TEST_CODE for binary archive writer
		if (auto result = mpFileSystem->Open<IBinaryArchiveWriter>("ProjectResources/Misc/TestArchive.bin", true))
		{
			if (IBinaryArchiveWriter* pArchiveWriter = mpFileSystem->Get<IBinaryArchiveWriter>(result.Get()))
			{
				pArchiveWriter->BeginGroup("Test");
				pArchiveWriter->SetString("Key1", "This is a string");
				pArchiveWriter->SetInt32("int", 32);

				pArchiveWriter->BeginGroup("Array");
				{
					for (int i = 0; i < 3; ++i)
					{
						pArchiveWriter->BeginGroup("");
						pArchiveWriter->SetInt32("value", i);
						pArchiveWriter->EndGroup();
					}
				}
				pArchiveWriter->EndGroup();

				pArchiveWriter->EndGroup();

				pArchiveWriter->Close();
			}
		}
#endif

#if 0 // TEST_CODE for binary archive reader
		if (auto result = mpFileSystem->Open<IBinaryArchiveReader>("ProjectResources/Misc/TestArchive.bin", true))
		{
			if (IBinaryArchiveReader* pArchiveReader = mpFileSystem->Get<IBinaryArchiveReader>(result.Get()))
			{
				pArchiveReader->BeginGroup("Test");
				auto str = pArchiveReader->GetString("Key1");
				auto iValue = pArchiveReader->GetInt32("int");

				pArchiveReader->BeginGroup("Array");
				{
					while (pArchiveReader->HasNextItem())
					{
						pArchiveReader->BeginGroup("");
						auto k = pArchiveReader->GetInt32("value");
						pArchiveReader->EndGroup();
					}
				}
				pArchiveReader->EndGroup();

				auto i2 = pArchiveReader->GetInt32("int");

				pArchiveReader->EndGroup();

				pArchiveReader->Close();
			}
		}
#endif

#if 1
#if 1 // note Test hierarchy of entities
		pSceneManager->LoadSceneAsync("ProjectResources/Scenes/TestScene.scene", [pSceneManager, this](auto)
		{
			int x = 0;

			auto pScene = pSceneManager->GetScene(pSceneManager->GetSceneId("Test")).Get();
			
			if (auto pTestObject = pScene->CreateEntity("TestObject"))
			{
				if (auto pMeshContainer = pTestObject->AddComponent<CStaticMeshContainer>())
				{
					pMeshContainer->SetMaterialName("ProjectResources/Materials/DefaultMaterial.material");
					pMeshContainer->SetMeshName("Cube");
				}
			}
			
			auto pSpriteGroup = pScene->CreateEntity("Parent");

			auto entities = mpWorld->FindEntitiesWithComponents<CQuadSprite>();

			for (auto id : entities)
			{
				GroupEntities(mpWorld.Get(), pSpriteGroup->GetId(), id);
			}

			auto c1 = pScene->CreateEntity("Child1")->GetId();
			auto c2 = pScene->CreateEntity("Child2")->GetId();

			GroupEntities(mpWorld.Get(), c1, c2);
			GroupEntities(mpWorld.Get(), pSpriteGroup->GetId(), c1);

			if (auto pAnimationContainer = pSpriteGroup->AddComponent<CAnimationContainerComponent>())
			{
				pAnimationContainer->SetAnimationClipId("ProjectResources/Animations/Animation2.animation");
			}

			pScene->CreateSkybox(mpResourceManager, "DefaultResources/Textures/DefaultSkybox");
			pScene->CreatePointLight(TColorUtils::mWhite, 1.0f, 10.0f);

#if 0 /// Test UI layout
			if (auto pCanvasEntity = pScene->CreateEntity("Canvas"))
			{
				if (auto pCanvas = pCanvasEntity->AddComponent<CCanvas>())
				{
					pCanvas->SetWidth(1024);
					pCanvas->SetHeight(768);
				}

				if (auto pParentEntity = pScene->CreateEntity("RootUI"))
				{
					if (auto pLayoutElement = pParentEntity->AddComponent<CLayoutElement>())
					{
						pLayoutElement->SetMinAnchor(TVector2(0.0f));
						pLayoutElement->SetMaxAnchor(TVector2(0.5f, 1.0f));
						pLayoutElement->SetMinOffset(ZeroVector2);
						pLayoutElement->SetMaxOffset(ZeroVector2);
						pLayoutElement->SetPivot(TVector2(0.5f));
					}

					if (auto pGridGroup = pParentEntity->AddComponent<CGridGroupLayout>())
					{
						pGridGroup->SetCellSize(TVector2(250.0f));
						pGridGroup->SetSpaceBetweenElements(TVector2(10.0f));
					}

#if 0
					if (auto pEntity = pScene->CreateEntity("Text0"))
					{
						if (auto pLayoutElement = pEntity->AddComponent<CLayoutElement>())
						{
							pLayoutElement->SetMinAnchor(TVector2(0.2f));
							pLayoutElement->SetMaxAnchor(TVector2(0.8f));
							pLayoutElement->SetPivot(TVector2(0.f));
						}

						if (auto pLabel = pEntity->AddComponent<CLabel>())
						{
							pLabel->SetAlignType(E_FONT_ALIGN_POLICY::RIGHT_BOTTOM);
							pLabel->SetFontId("OpenSans.font");
						}

						GroupEntities(mpWorld.Get(), pParentEntity->GetId(), pEntity->GetId());
					}
#endif

					if (auto pEntity = pScene->CreateEntity("UIElement"))
					{
						if (auto pLayoutElement = pEntity->AddComponent<CLayoutElement>())
						{
							pLayoutElement->SetMinAnchor(TVector2(0.2f));
							pLayoutElement->SetMaxAnchor(TVector2(0.8f));
							pLayoutElement->SetPivot(TVector2(0.f));

							//pLayoutElement->SetMaxOffset(TVector2(150.0f));
						}

						if (auto pImage = pEntity->AddComponent<C9SliceImage>())
						{
							pImage->SetLeftXSlicer(0.25f);
							pImage->SetRightXSlicer(0.75f);
							pImage->SetBottomYSlicer(0.25f);
							pImage->SetTopYSlicer(0.75f);

							pImage->SetImageId(DefaultSpriteId);
						}

						pEntity->AddComponent<CInputReceiver>();

						GroupEntities(mpWorld.Get(), pParentEntity->GetId(), pEntity->GetId());
					}

					if (auto pEntity = pScene->CreateEntity("UIElement2"))
					{
						if (auto pLayoutElement = pEntity->AddComponent<CLayoutElement>())
						{
							pLayoutElement->SetMinAnchor(TVector2(0.2f));
							pLayoutElement->SetMaxAnchor(TVector2(0.8f));
							pLayoutElement->SetPivot(TVector2(0.f));

							//pLayoutElement->SetMaxOffset(TVector2(150.0f));
						}

						if (auto pImage = pEntity->AddComponent<C9SliceImage>())
						{
							pImage->SetLeftXSlicer(0.25f);
							pImage->SetRightXSlicer(0.75f);
							pImage->SetBottomYSlicer(0.25f);
							pImage->SetTopYSlicer(0.75f);

							pImage->SetImageId(DefaultSpriteId);
						}

						pEntity->AddComponent<CInputReceiver>();

						GroupEntities(mpWorld.Get(), pParentEntity->GetId(), pEntity->GetId());
					}

					if (auto pEntity = pScene->CreateEntity("UIElement3"))
					{
						if (auto pLayoutElement = pEntity->AddComponent<CLayoutElement>())
						{
							pLayoutElement->SetMinAnchor(TVector2(0.2f));
							pLayoutElement->SetMaxAnchor(TVector2(0.8f));
							pLayoutElement->SetPivot(TVector2(0.f));

							//pLayoutElement->SetMaxOffset(TVector2(150.0f));
						}

						if (auto pImage = pEntity->AddComponent<C9SliceImage>())
						{
							pImage->SetLeftXSlicer(0.25f);
							pImage->SetRightXSlicer(0.75f);
							pImage->SetBottomYSlicer(0.25f);
							pImage->SetTopYSlicer(0.75f);

							pImage->SetImageId(DefaultSpriteId);
						}

						pEntity->AddComponent<CInputReceiver>();

						GroupEntities(mpWorld.Get(), pParentEntity->GetId(), pEntity->GetId());
					}

					if (auto pEntity = pScene->CreateEntity("UIElement4"))
					{
						if (auto pLayoutElement = pEntity->AddComponent<CLayoutElement>())
						{
							pLayoutElement->SetMinAnchor(TVector2(0.2f));
							pLayoutElement->SetMaxAnchor(TVector2(0.8f));
							pLayoutElement->SetPivot(TVector2(0.f));

							//pLayoutElement->SetMaxOffset(TVector2(150.0f));
						}

						if (auto pImage = pEntity->AddComponent<C9SliceImage>())
						{
							pImage->SetLeftXSlicer(0.25f);
							pImage->SetRightXSlicer(0.75f);
							pImage->SetBottomYSlicer(0.25f);
							pImage->SetTopYSlicer(0.75f);

							pImage->SetImageId(DefaultSpriteId);
						}

						pEntity->AddComponent<CInputReceiver>();

						GroupEntities(mpWorld.Get(), pParentEntity->GetId(), pEntity->GetId());
					}

#if 0
					if (auto pEntity = pScene->CreateEntity("UIElement2"))
					{
						if (auto pLayoutElement = pEntity->AddComponent<CLayoutElement>())
						{
							pLayoutElement->SetMinAnchor(TVector2(0.2f));
							pLayoutElement->SetMaxAnchor(TVector2(0.8f));
							pLayoutElement->SetPivot(TVector2(0.f));

							//pLayoutElement->SetMaxOffset(TVector2(150.0f));
						}

						if (auto pImage = pEntity->AddComponent<CImage>())
						{
							pImage->SetImageId(ButtonTextureId);
						}

						pEntity->AddComponent<CInputReceiver>();

						GroupEntities(mpWorld.Get(), pParentEntity->GetId(), pEntity->GetId());
					}

					if (auto pEntity = pScene->CreateEntity("Text"))
					{
						if (auto pLayoutElement = pEntity->AddComponent<CLayoutElement>())
						{
							pLayoutElement->SetMinAnchor(TVector2(0.2f));
							pLayoutElement->SetMaxAnchor(TVector2(0.8f));
							pLayoutElement->SetPivot(TVector2(0.f));
						}

						if (auto pLabel = pEntity->AddComponent<CLabel>())
						{
							pLabel->SetAlignType(E_FONT_ALIGN_POLICY::CENTER);
							pLabel->SetOverflowPolicyType(E_TEXT_OVERFLOW_POLICY::BREAK_ALL);
							pLabel->SetFontId("OpenSans.font");
							pLabel->SetText("TTTTTTTTTTTTTTTTTTTTTTTT");
						}

						GroupEntities(mpWorld.Get(), pParentEntity->GetId(), pEntity->GetId());
					}
#endif

					GroupEntities(mpWorld.Get(), pCanvasEntity->GetId(), pParentEntity->GetId());
				}				
			}
#endif

#if 0
			/// \note animated mesh entity
			if (auto pAnimatedMeshEntity = pScene->CreateEntity("AnimatedMesh"))
			{
				pAnimatedMeshEntity->AddComponent<CShadowCasterComponent>();
				pAnimatedMeshEntity->AddComponent<CShadowReceiverComponent>();
				pAnimatedMeshEntity->AddComponent<CBoundsComponent>();
				pAnimatedMeshEntity->AddComponent<CMeshAnimatorComponent>();

				if (auto pTransform = pAnimatedMeshEntity->GetComponent<CTransform>())
				{
					pTransform->SetPosition({ 0.0f, 0.0f, 2.0f });
				}

				if (auto pMeshContainer = pAnimatedMeshEntity->AddComponent<CSkinnedMeshContainer>())
				{
					const std::string meshName = "ProjectResources/Models/TestAnim2";

					pMeshContainer->SetMaterialName("ProjectResources/Materials/DefaultSkinningMaterial.material");
					pMeshContainer->SetMeshName(meshName + ".mesh");
					pMeshContainer->SetSkeletonName(meshName + ".skeleton");
					pMeshContainer->SetShowDebugSkeleton(true);

					if (auto pAnimationPlayer = pAnimatedMeshEntity->AddComponent<CAnimationContainerComponent>())
					{
						pAnimationPlayer->SetAnimationClipId(meshName + "_Idle.animation");
						pAnimationPlayer->Play();
					}
				}
			}
#endif

			/*if (auto result = mpFileSystem->Open<IYAMLFileWriter>("TestScene2.scene", true))
			{
				if (auto pFileWriter = mpFileSystem->Get<IYAMLFileWriter>(result.Get()))
				{
					if (auto pScene = pSceneManager->GetScene(pSceneManager->GetSceneId("Test")).Get())
					{
						pScene->Save(pFileWriter);
						pFileWriter->Close();
					}
				}
			}*/
		});
#else
		pSceneManager->LoadSceneAsync("ProjectResources/Scenes/TestScene2.scene", nullptr);
#endif
#endif

		pSceneManager->LoadSceneAsync("ProjectResources/Scenes/TestPlayground.scene", nullptr);
	}

	mpResourceManager->Load<CBaseTexture2D, TResourceProviderInfo<CBaseTexture2D, CBaseTexture2D>>("test");
	
	auto s = mpFileSystem->GetUserDirectory();

	if (auto pPackage = mpFileSystem->Get<IPackageFileReader>(mpFileSystem->Open<IPackageFileReader>("ProjectResources/Misc/NewArchive.pak").Get()))
	{
		auto data = pPackage->ReadFileBytes("test.txt");
	}

	return RC_OK;
}

E_RESULT_CODE CCustomEngineListener::OnUpdate(const float& dt)
{
	TDE2_PROFILER_SCOPE("OnUpdate");

#if 1
	/// \note ImGUI tests
	{
		TDE2_PROFILER_SCOPE("Test");

		auto imgui = mpEngineCoreInstance->GetSubsystem<IImGUIContext>();

		static bool isOpened = true;
		if (imgui->BeginWindow("Hello, World!", isOpened, {}))
		{
			imgui->BeginHorizontal();
			imgui->Label("TEST");
			imgui->Label("Test2");
			imgui->EndHorizontal();

			static int i = 0;
			imgui->IntSlider("integer", i, 0, 20);
			TVector3 value;
			imgui->Vector3Field("vector", value);

			if (imgui->Button("Click", { 50.0f, 15.0f }))
			{
			}
			imgui->IntField("wewewe", i);
			imgui->Tooltip("Test");

			std::string str = "Hello!";
			imgui->Label("FPS: " + std::to_string(1.0f / mpWindowSystem->GetTimer()->GetDeltaTime()));

			imgui->DisplayMainMenu([](IImGUIContext& ui)
			{
				ui.MenuGroup("Menu", [](IImGUIContext& ui2)
				{
					ui2.MenuItem("Item1", "");
					ui2.MenuItem("Item2", "");
					ui2.MenuItem("Item3", "");
				});
			});

			imgui->DisplayContextMenu("m1", [](IImGUIContext& ui)
			{
				ui.MenuItem("terer", "r");
			});

			static int id = 0;

			id = imgui->Popup("TestPopup", id, { "First", "Second", "Third" });
			imgui->Image(mpResourceManager->Load<ITexture2D>("@"), TVector2(128.0f, 128.0f));

			static int selection = 1 << 2;

			bool isOpened = false;
			bool isSelected = false;

			if (std::get<0>(std::tie(isOpened, isSelected) = imgui->BeginTreeNode("Test1", selection & (1 << 0))))
			{
				if (isSelected) selection = (1 << 0);

				if (std::get<0>(std::tie(isOpened, isSelected) = imgui->BeginTreeNode("Node", selection & (1 << 1))))
				{
					if (isSelected) selection = (1 << 1);

					imgui->EndTreeNode();
				}

				if (std::get<0>(std::tie(isOpened, isSelected) = imgui->BeginTreeNode("Node2", selection & (1 << 2))))
				{
					if (isSelected) selection = (1 << 2);

					imgui->EndTreeNode();
				}

				if (imgui->SelectableItem("Node3", selection & (1 << 3)))
				{
					selection = (1 << 3);
				}

				imgui->EndTreeNode();
			}

			/*if (auto pEntity = mpWorld->FindEntity(TEntityId(24)))
			{
				auto pLayout = pEntity->GetComponent<CLayoutElement>();
				auto pos = pEntity->GetComponent<CTransform>()->GetPosition();

				imgui->DrawRect({ {pos.x, pos.y}, -pLayout->GetWorldRect().GetSizes() }, TColorUtils::mYellow);
			}*/

		}
		imgui->EndWindow();
	}	
#endif

	if (mpInputContext->IsKeyPressed(E_KEYCODES::KC_ESCAPE))
	{
		return mpEngineCoreInstance->Quit();
	}

	if (mpInputContext->IsKeyPressed(E_KEYCODES::KC_SPACE))
	{
		auto pSoundEntity = mpEngineCoreInstance->GetSubsystem<ISceneManager>()->GetScene(TSceneId(1)).Get()->CreateEntity("Sound");
		
		if (auto pAudioSource = pSoundEntity->AddComponent<CAudioSourceComponent>())
		{
			pAudioSource->SetAudioClipId("test.mp3");
		}
	}

	CTestContext::Get()->Update(dt);
	
	//if (mpInputContext->IsMouseButtonPressed(0))
	//{
		auto&& mousePosition = mpInputContext->GetMousePosition();

		auto&& result = mpWorld->GetRaycastContext()->Raycast2DClosest(ZeroVector3, ForwardVector3, 1000.0f);
	//}
	
	auto pDebugUtility = mpGraphicsObjectManager->CreateDebugUtility(mpResourceManager, mpEngineCoreInstance->GetSubsystem<IRenderer>().Get()).Get();
	
	pDebugUtility->DrawSphere(TVector3(10.0f, 0.0f, 0.0f), 5.0f, TColorUtils::mGreen);
	pDebugUtility->DrawSphere(TVector3(-10.0f, 10.0f, 0.0f), 5.0f, TColorUtils::mMagenta);

	// rotate the cube
#if 0
	auto pEntity = mpWorld->FindEntity(mpWorld->FindEntitiesWithComponents<CStaticMeshContainer>()[0]);
	auto pTransform = pEntity->GetComponent<CTransform>();
	static F32 time = 0.0f;
	time += 5.0f * dt;

	pTransform->SetScale(TVector3(0.5f + 0.5f * fabs(sinf(0.5f * time))));
	pTransform->SetRotation(TVector3(0.0f, time, 0.0f));
	auto&& m = pTransform->GetTransform();
#endif

	return RC_OK;
}

E_RESULT_CODE CCustomEngineListener::OnFree()
{
	return RC_OK;
}

void CCustomEngineListener::SetEngineInstance(IEngineCore* pEngineCore)
{
	if (!pEngineCore)
	{
		return;
	}

	mpEngineCoreInstance = pEngineCore;

	mpGraphicsContext = mpEngineCoreInstance->GetSubsystem<IGraphicsContext>().Get();
	mpWindowSystem    = mpEngineCoreInstance->GetSubsystem<IWindowSystem>().Get();
	mpResourceManager = mpEngineCoreInstance->GetSubsystem<IResourceManager>().Get();
	mpInputContext    = mpEngineCoreInstance->GetSubsystem<IDesktopInputContext>().Get();
	mpFileSystem      = mpEngineCoreInstance->GetSubsystem<IFileSystem>().Get();

	mpGraphicsObjectManager = mpGraphicsContext->GetGraphicsObjectManager();
}
