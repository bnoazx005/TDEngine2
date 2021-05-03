#include "../include/CCustomEngineListener.h"
#include <TDEngine2.h>
#include <iostream>


using namespace TDEngine2;


E_RESULT_CODE CCustomEngineListener::OnStart()
{
	E_RESULT_CODE result = RC_OK;

	if (RC_OK != (result = _mountResourcesDirectories()))
	{
		return result;
	}

	mpWorld = mpEngineCoreInstance->GetWorldInstance();

	mpResourceManager->Load<IMaterial>("ProjectMaterials/NewMaterial.material");
	mpResourceManager->Load<IMaterial>("DefaultMaterials/DebugMaterial.material");
	mpResourceManager->Load<IMaterial>("ProjectMaterials/DefaultMaterial.material");

	if (IMaterial* pFontMaterial = mpResourceManager->GetResource<IMaterial>(mpResourceManager->Load<IMaterial>("DefaultMaterials/DebugTextMaterial.material")))
	{
		if (auto pFontAtlas = mpResourceManager->GetResource<ITextureAtlas>(mpResourceManager->Load<ITextureAtlas>("atlas.info")))
		{
			pFontMaterial->SetTextureResource("FontTextureAtlas", pFontAtlas->GetTexture());
		}
	}

	for (I32 i = 0; i < 0; ++i)
	{
		auto pEntity = mpWorld->CreateEntity();

		if (auto pTransform = pEntity->GetComponent<CTransform>())
		{
			pTransform->SetPosition(pTransform->GetPosition() + RandVector3({ -2.0f, 0.0f, 0.0f }, { 5.0f, 5.0f, 10.0f }));
		}

		if (auto pSprite = pEntity->AddComponent<CQuadSprite>())
		{
			pSprite->SetMaterialName("ProjectMaterials/NewMaterial.material");
			pSprite->SetColor(RandColor());
		}

		//auto pBoxCollision = pEntity->AddComponent<CBoxCollisionObject2D>();
	}

	// \note create a trigger
	auto pTriggerEntity = mpWorld->CreateEntity();
	auto pTransform = pTriggerEntity->GetComponent<CTransform>();
	pTransform->SetPosition({ 0.0f, -1.0f, 1.0f });
	pTransform->SetScale({ 10.0f, 1.0f, 1.0f });
	auto collider = pTriggerEntity->AddComponent<CBoxCollisionObject2D>();
	collider->SetCollisionType(E_COLLISION_OBJECT_TYPE::COT_STATIC);
	pTriggerEntity->AddComponent<CTrigger2D>();
		
	mpCameraEntity = mpWorld->CreateEntity("Camera");
	auto pCamera = mpCameraEntity->AddComponent<CPerspectiveCamera>();	
	pCamera->SetAspect(mpWindowSystem->GetWidth() / mpWindowSystem->GetHeight());
	pCamera->SetFOV(0.5f * CMathConstants::Pi);

	mpCameraEntity->AddComponent<CAudioListenerComponent>();

	mpResourceManager->Load<IStaticMesh>("scene.mesh");

	auto pMeshEntity = mpWorld->CreateEntity();
	auto shadowCaster = pMeshEntity->AddComponent<CShadowCasterComponent>();
	auto sr = pMeshEntity->AddComponent<CShadowReceiverComponent>();
	auto bounds = pMeshEntity->AddComponent<CBoundsComponent>();
	auto pMeshTransform = pMeshEntity->GetComponent<CTransform>();
	pMeshTransform->SetPosition({ 0.0f, 0.0f, 2.0f });
	auto pMeshContainer = pMeshEntity->AddComponent<CStaticMeshContainer>();
	pMeshContainer->SetMaterialName("ProjectMaterials/DefaultMaterial.material");
	pMeshContainer->SetMeshName("scene.mesh");
	auto collision = pMeshEntity->AddComponent<CBoxCollisionObject3D>();
	collision->SetCollisionType(E_COLLISION_OBJECT_TYPE::COT_DYNAMIC);

	// plane
	auto pPlaneEntity = mpWorld->CreateEntity();
	auto sr2 = pPlaneEntity->AddComponent<CShadowReceiverComponent>();
	auto pPlaneMeshContainer = pPlaneEntity->AddComponent<CStaticMeshContainer>();
	pPlaneMeshContainer->SetMaterialName("ProjectMaterials/DefaultMaterial.material");
	pPlaneMeshContainer->SetMeshName("Plane");

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

	if (auto pPackageWriter = mpFileSystem->Get<IPackageFileWriter>(mpFileSystem->Open<IPackageFileWriter>("TestPackage.pak", true).Get()))
	{
		//pPackageWriter->WriteFile<Type>("path/within/package/where/to/mount", pFile)
		pPackageWriter->WriteFile<IYAMLFileReader>("test/", *mpFileSystem->Get<IYAMLFileReader>(mpFileSystem->Open<IYAMLFileReader>("Animation.animation", false).Get()));

		pPackageWriter->Close();
	}

	// TEMP CODE
	{
		ISceneManager* pSceneManager = mpEngineCoreInstance->GetSubsystem<ISceneManager>();

#if 0
		if (auto result = mpFileSystem->Open<IYAMLFileWriter>("TestScene.scene", true))
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
		if (auto result = mpFileSystem->Open<IBinaryArchiveWriter>("TestArchive.bin", true))
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
		if (auto result = mpFileSystem->Open<IBinaryArchiveReader>("TestArchive.bin", true))
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

#if 1 // note Test hierarchy of entities
		pSceneManager->LoadSceneAsync("TestScene.scene", [pSceneManager, this](auto)
		{
			int x = 0;

			auto pScene = pSceneManager->GetScene(pSceneManager->GetSceneId("Test")).Get();
			auto pSpriteGroup = pScene->CreateEntity("Parent");

			auto entities = mpWorld->FindEntitiesWithComponents<CQuadSprite>();

			for (auto id : entities)
			{
				GroupEntities(mpWorld, pSpriteGroup->GetId(), id);
			}

			auto c1 = mpWorld->CreateEntity("Child1")->GetId();
			auto c2 = mpWorld->CreateEntity("Child2")->GetId();

			GroupEntities(mpWorld, c1, c2);
			GroupEntities(mpWorld, pSpriteGroup->GetId(), c1);

			if (auto pAnimationContainer = pSpriteGroup->AddComponent<CAnimationContainerComponent>())
			{
				pAnimationContainer->SetAnimationClipId("Animation2.animation");
			}

			pScene->CreateSkybox(mpResourceManager, "Resources/Textures/DefaultSkybox");
			pScene->CreatePointLight(TColorUtils::mWhite, 1.0f, 10.0f);

#if 1 /// Test UI layout
			if (auto pCanvasEntity = pScene->CreateEntity("Canvas"))
			{
				if (auto pCanvas = pCanvasEntity->AddComponent<CCanvas>())
				{
					pCanvas->SetWidth(1024);
					pCanvas->SetHeight(768);
				}

				if (auto pEntity = pScene->CreateEntity("UIElement"))
				{
					if (auto pLayoutElement = pEntity->AddComponent<CLayoutElement>())
					{
						pLayoutElement->SetMinAnchor(TVector2(0.5f));
						pLayoutElement->SetMaxAnchor(TVector2(0.5f));
						pLayoutElement->SetPivot(TVector2(0.5f));

						pLayoutElement->SetMaxOffset(TVector2(50.0f));
					}

					GroupEntities(mpWorld, pCanvasEntity->GetId(), pEntity->GetId());
				}
			}
#endif
		});

#endif

	}

	mpResourceManager->Load<IAudioSource>("test.mp3");

	mpResourceManager->Load<CBaseTexture2D, TResourceProviderInfo<CBaseTexture2D, CBaseTexture2D>>("test");

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
			std::string str = "Hello!";
			imgui->TextField("tetet", str);

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

	CTransform* pCameraTransform = mpCameraEntity->GetComponent<CTransform>();
	
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
	
	//if (mpInputContext->IsMouseButtonPressed(0))
	//{
		auto&& mousePosition = mpInputContext->GetMousePosition();

		auto&& result = mpWorld->GetRaycastContext()->Raycast2DClosest(ZeroVector3, ForwardVector3, 1000.0f);
	//}
	
	auto pDebugUtility = mpGraphicsObjectManager->CreateDebugUtility(mpResourceManager, mpEngineCoreInstance->GetSubsystem<IRenderer>()).Get();
	pDebugUtility->DrawLine(ZeroVector3, { 10.0f, 10.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f });
	pDebugUtility->DrawLine(ZeroVector3, { -10.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f });
	pDebugUtility->DrawLine(ZeroVector3, { -10.0f, 10.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f });
	pDebugUtility->DrawLine(ZeroVector3, { 10.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f });
	pDebugUtility->DrawText({ 0, 0 }, 0.008f, std::to_string(dt), { 1.0f, 1.0f, 1.0f, 1.0f });
	pDebugUtility->DrawCross(ZeroVector3, 1.0f, { 1.0f, 0.0f, 0.0f, 1.0f });
	pDebugUtility->DrawRect({ 0.0f, 0.0f, 2.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f });
	pDebugUtility->DrawAABB({ ZeroVector3, { 10.0f, 10.0f, 10.0f } }, { 0.0f, 1.0f, 0.0f, 1.0f });
	pDebugUtility->DrawCircle(ZeroVector3, Normalize(TVector3(1, 1, 0)), 5.0f, { 0.0f, 1.0f, 0.0f, 1.0f }, 16);
	pDebugUtility->DrawSphere(ZeroVector3, 10.0f, { 0.0f, 1.0f, 0.0f, 1.0f }, 3);

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

	mpGraphicsContext = mpEngineCoreInstance->GetSubsystem<IGraphicsContext>();
	mpWindowSystem    = mpEngineCoreInstance->GetSubsystem<IWindowSystem>();
	mpResourceManager = mpEngineCoreInstance->GetSubsystem<IResourceManager>();
	mpInputContext    = mpEngineCoreInstance->GetSubsystem<IDesktopInputContext>();
	mpFileSystem      = mpEngineCoreInstance->GetSubsystem<IFileSystem>();

	mpGraphicsObjectManager = mpGraphicsContext->GetGraphicsObjectManager();
}

E_RESULT_CODE CCustomEngineListener::_mountResourcesDirectories()
{
	E_RESULT_CODE result = RC_OK;

	const std::string shaderLangSubdirectory = dynamic_cast<COGLGraphicsContext*>(mpGraphicsContext) ? "GL/" : "DX/";

	result = result | mpFileSystem->MountPhysicalPath("../../SandboxGame/Resources/Shaders/" + shaderLangSubdirectory, "ProjectShaders/");
	result = result | mpFileSystem->MountPhysicalPath("../../SandboxGame/Resources/Materials/", "ProjectMaterials/");
	result = result | mpFileSystem->MountPhysicalPath("../../SandboxGame/Resources/Textures/", "ProjectTextures/");

	return result;
}