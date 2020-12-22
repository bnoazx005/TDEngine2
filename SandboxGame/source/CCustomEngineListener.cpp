#include "./../include/CCustomEngineListener.h"
#include <TDEngine2.h>
#include <iostream>


using namespace TDEngine2;

#define DX_GAPI 1

#if DX_GAPI
const std::string ShaderType = "HLSL";
const std::string GAPIType = "DX";
#else
const std::string ShaderType = "GLSL";
const std::string GAPIType = "GL";
#endif


const std::string TestShaderName = Wrench::StringUtils::Format("test{0}Shader.shader", GAPIType);
const std::string DebugShaderName = Wrench::StringUtils::Format("Debug{0}Shader.shader", GAPIType);
const std::string DebugTextShaderName = Wrench::StringUtils::Format("DebugText{0}Shader.shader", GAPIType);


TVector3 SunLightPos{ 5.0f, 10.0f, 0.0f };


E_RESULT_CODE CCustomEngineListener::OnStart()
{
	E_RESULT_CODE result = RC_OK;

	mpWorld = mpEngineCoreInstance->GetWorldInstance();

	mpResourceManager->Load<CBaseMaterial>("NewMaterial.material");
	mpResourceManager->Load<CBaseMaterial>("DebugMaterial.material");

	IMaterial* pFontMaterial = mpResourceManager->GetResource<IMaterial>(mpResourceManager->Create<CBaseMaterial>("DebugTextMaterial.material", TMaterialParameters{ DebugTextShaderName, true }));

	auto pFontAtlas = mpResourceManager->GetResource<ITextureAtlas>(mpResourceManager->Load<CTextureAtlas>("atlas"));
	pFontMaterial->SetTextureResource("FontTextureAtlas", pFontAtlas->GetTexture());

#if 0
	if (auto result = mpFileSystem->Open<IYAMLFileWriter>("DebugTextMaterial.material", true))
	{
		if (auto materialFileWriter = mpFileSystem->Get<IYAMLFileWriter>(result.Get()))
		{
			pFontMaterial->Save(materialFileWriter);
			materialFileWriter->Close();
		}
	}
#endif

	TDE2_ASSERT(TResourceId::Invalid != mpResourceManager->Load<CBasePostProcessingProfile>("default-profile"));

	const TColor32F colors[] =
	{
		{ 1.0f, 0.0f, 0.0f, 1.0f },
		{ 0.0f, 1.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 1.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 0.0f, 1.0f, 1.0f, 1.0f },
		{ 1.0f, 0.0f, 1.0f, 1.0f },
		{ 1.0f, 1.0f, 0.0f, 1.0f },
	};

	for (I32 i = 0; i < 10; ++i)
	{
		auto pEntity = mpWorld->CreateEntity();

		if (auto pTransform = pEntity->GetComponent<CTransform>())
		{
			pTransform->SetPosition(pTransform->GetPosition() + RandVector3({ -2.0f, 0.0f, 0.0f }, { 5.0f, 5.0f, 10.0f }));
		}

		if (auto pSprite = pEntity->AddComponent<CQuadSprite>())
		{
			pSprite->SetMaterialName("NewMaterial.material");
			pSprite->SetColor(colors[rand() % 7]);
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

	auto pRT = mpResourceManager->Create<CBaseRenderTarget>("default-rt", TTexture2DParameters { mpWindowSystem->GetWidth(), mpWindowSystem->GetHeight(), FT_NORM_UBYTE4, 1, 1, 0 });
	//mpGraphicsContext->BindRenderTarget(dynamic_cast<IRenderTarget*>(pRT->Get(RAT_BLOCKING)));

	auto pCubeMesh = CStaticMesh::CreateCube(mpResourceManager);
	auto pPlaneMesh = CStaticMesh::CreatePlane(mpResourceManager);

	mpResourceManager->Load<CStaticMesh>("Test.mesh");

	auto pMeshEntity = mpWorld->CreateEntity();
	auto shadowCaster = pMeshEntity->AddComponent<CShadowCasterComponent>();
	auto sr = pMeshEntity->AddComponent<CShadowReceiverComponent>();
	auto bounds = pMeshEntity->AddComponent<CBoundsComponent>();
	auto pMeshTransform = pMeshEntity->GetComponent<CTransform>();
	pMeshTransform->SetPosition({ 0.0f, 1.0f, 2.0f });
	auto pMeshContainer = pMeshEntity->AddComponent<CStaticMeshContainer>();
	pMeshContainer->SetMaterialName("DebugMaterial.material");
	pMeshContainer->SetMeshName("Cube");
	auto collision = pMeshEntity->AddComponent<CBoxCollisionObject3D>();
	collision->SetCollisionType(E_COLLISION_OBJECT_TYPE::COT_DYNAMIC);

	// plane
	auto pPlaneEntity = mpWorld->CreateEntity();
	auto sr2 = pPlaneEntity->AddComponent<CShadowReceiverComponent>();
	auto pPlaneMeshContainer = pPlaneEntity->AddComponent<CStaticMeshContainer>();
	pPlaneMeshContainer->SetMaterialName("DebugMaterial.material");
	pPlaneMeshContainer->SetMeshName("Plane");

	// Create a 3D trigger
	if (auto p3DTrigger = mpWorld->CreateEntity())
	{
		if (auto pTransform = p3DTrigger->GetComponent<CTransform>())
		{
			pTransform->SetPosition({ 0.0f, -1.0f, 1.0f });
			pTransform->SetScale({ 25.0f, 1.0f, 25.0f });
		}

		auto p3DTriggerCollider = p3DTrigger->AddComponent<CBoxCollisionObject3D>();
		p3DTriggerCollider->SetCollisionType(E_COLLISION_OBJECT_TYPE::COT_KINEMATIC);
		p3DTriggerCollider->SetSizes(TVector3(25.0f, 1.0f, 25.0f));
		p3DTrigger->AddComponent<CTrigger3D>();
	}

	{
		TMaterialParameters skyboxMatParams 
		{ 
			mpFileSystem->ResolveVirtualPath(Wrench::StringUtils::Format("Shaders/Default/DefaultSkyboxShader_{0}.shader", ShaderType), false), true,
			{ true, true, E_COMPARISON_FUNC::LESS_EQUAL}, 
			{ E_CULL_MODE::NONE, false, false, 0.0f, 1.0f, true, false }
		};

		IMaterial* pMaterial = mpResourceManager->GetResource<IMaterial>(mpResourceManager->Create<CBaseMaterial>("DefaultSkybox.material", skyboxMatParams));
		pMaterial->SetTextureResource("SkyboxTexture", mpResourceManager->GetResource<ICubemapTexture>(mpResourceManager->Load<CBaseCubemapTexture>("DefaultSkybox")));
		pMaterial->SetGeometrySubGroupTag(E_GEOMETRY_SUBGROUP_TAGS::SKYBOX);
		
		auto pSkyboxEntity = mpWorld->CreateEntity("Skybox");
		auto pSkyboxContainer = pSkyboxEntity->AddComponent<CStaticMeshContainer>();
		pSkyboxContainer->SetMeshName("Cube");
		pSkyboxContainer->SetMaterialName("DefaultSkybox.material");
	}

	mpGraphicsContext->GetInfo();
	mpGraphicsContext->GetContextInfo();

#if 0
	TAnimationClipParameters clip;
	clip.mDuration  = 2.5f;

	if (IAnimationClip* pClip = mpResourceManager->Create<CAnimationClip>("Animation", clip)->Get<IAnimationClip>(RAT_BLOCKING))
	{
		if (auto result = mpFileSystem->Open<IYAMLFileWriter>("Animation.animation", true))
		{			
			if (auto animationFileWriter = mpFileSystem->Get<IYAMLFileWriter>(result.Get()))
			{
				pClip->Save(animationFileWriter);
				animationFileWriter->Close();
			}
		}
	}	
#endif

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

		pSceneManager->LoadSceneAsync("TestScene.scene", [](auto)
		{
			int x = 0;
		});
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
			imgui->Image(mpResourceManager->Load<CBaseTexture2D>("Tim.tga"), TVector2(128.0f, 128.0f));

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
		}
		imgui->EndWindow();
	}	
#endif

	CTransform* pCameraTransform = mpCameraEntity->GetComponent<CTransform>();
	
	if (mpInputContext->IsKeyPressed(E_KEYCODES::KC_ESCAPE))
	{
		return mpEngineCoreInstance->Quit();
	}
	
	//if (mpInputContext->IsMouseButtonPressed(0))
	//{
		auto&& mousePosition = mpInputContext->GetMousePosition();

		auto&& result = mpWorld->GetRaycastContext()->Raycast2DClosest(ZeroVector3, ForwardVector3, 1000.0f);
	//}
	
	if (mpInputContext->IsKey(E_KEYCODES::KC_LEFT)) SunLightPos = SunLightPos - dt * RightVector3;
	if (mpInputContext->IsKey(E_KEYCODES::KC_RIGHT)) SunLightPos = SunLightPos + dt * RightVector3;
	if (mpInputContext->IsKey(E_KEYCODES::KC_UP)) SunLightPos = SunLightPos + dt * UpVector3;
	if (mpInputContext->IsKey(E_KEYCODES::KC_DOWN)) SunLightPos = SunLightPos - dt * UpVector3;

	for (TEntityId entity : mpWorld->FindEntitiesWithComponents<CDirectionalLight>())
	{
		if (auto pTransform = mpWorld->FindEntity(entity)->GetComponent<CTransform>())
		{
			pTransform->SetPosition(SunLightPos);
		}
	}

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
