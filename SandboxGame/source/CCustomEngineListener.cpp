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


TVector3 SunLightPos{ 5.0f, 0.0f, 0.0f };


E_RESULT_CODE CCustomEngineListener::OnStart()
{
	E_RESULT_CODE result = RC_OK;

	mpWorld = mpEngineCoreInstance->GetWorldInstance();

	IMaterial* pMaterial = mpResourceManager->Create<CBaseMaterial>("NewMaterial.material", TMaterialParameters{ TestShaderName, true })->Get<IMaterial>(RAT_BLOCKING);
	pMaterial->SetTextureResource("TextureAtlas", mpResourceManager->Load<CBaseTexture2D>("Tim.tga")->Get<ITexture2D>(RAT_BLOCKING));
	pMaterial->SetTextureResource("SkyboxTexture", mpResourceManager->Load<CBaseCubemapTexture>("DefaultSkybox")->Get<ICubemapTexture>(RAT_BLOCKING));

#if 0
	if (auto result = mpFileSystem->Open<IYAMLFileWriter>("DefaultMaterial.material", true))
	{
		if (auto materialFileWriter = mpFileSystem->Get<IYAMLFileWriter>(result.Get()))
		{
			pMaterial->Save(materialFileWriter);
			materialFileWriter->Close();
		}
	}
#endif

	//IMaterial* pDefaultMaterial = mpResourceManager->Load<CBaseMaterial>("DefaultMaterial.material")->Get<IMaterial>(RAT_BLOCKING);

	mpResourceManager->Create<CBaseMaterial>("DebugMaterial.material", TMaterialParameters{ DebugShaderName, false, {}, { E_CULL_MODE::NONE } });
	
	IMaterial* pFontMaterial = mpResourceManager->Create<CBaseMaterial>("DebugTextMaterial.material", 
																		TMaterialParameters{ DebugTextShaderName, true })->Get<IMaterial>(RAT_BLOCKING);

	auto pFontAtlas = mpResourceManager->Load<CTextureAtlas>("atlas")->Get<ITextureAtlas>(RAT_BLOCKING);
	pFontMaterial->SetTextureResource("FontTextureAtlas", pFontAtlas->GetTexture());

	TDE2_ASSERT(mpResourceManager->Load<CBasePostProcessingProfile>("default-profile")->IsValid());

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
	pMeshTransform->SetPosition({ 0.0f, 0.0f, 2.0f });
	auto pMeshContainer = pMeshEntity->AddComponent<CStaticMeshContainer>();
	pMeshContainer->SetMaterialName("DebugMaterial.material");
	pMeshContainer->SetMeshName("Cube");
	auto collision = pMeshEntity->AddComponent<CBoxCollisionObject3D>();
	collision->SetCollisionType(E_COLLISION_OBJECT_TYPE::COT_KINEMATIC);

	// plane
	auto pPlaneEntity = mpWorld->CreateEntity();
	auto sr2 = pPlaneEntity->AddComponent<CShadowReceiverComponent>();
	auto pPlaneMeshContainer = pPlaneEntity->AddComponent<CStaticMeshContainer>();
	pPlaneMeshContainer->SetMaterialName("DebugMaterial.material");
	pPlaneMeshContainer->SetMeshName("Plane");

	{
		TMaterialParameters skyboxMatParams 
		{ 
			mpFileSystem->ResolveVirtualPath(Wrench::StringUtils::Format("Shaders/Default/DefaultSkyboxShader_{0}.shader", ShaderType), false), true,
			{ true, true, E_COMPARISON_FUNC::LESS_EQUAL}, 
			{ E_CULL_MODE::NONE, false, false, 0.0f, 1.0f, true, false }
		};

		IMaterial* pMaterial = mpResourceManager->Create<CBaseMaterial>("DefaultSkybox.material", skyboxMatParams)->Get<IMaterial>(RAT_BLOCKING);
		pMaterial->SetTextureResource("SkyboxTexture", mpResourceManager->Load<CBaseCubemapTexture>("DefaultSkybox")->Get<ICubemapTexture>(RAT_BLOCKING));
		pMaterial->SetGeometrySubGroupTag(E_GEOMETRY_SUBGROUP_TAGS::SKYBOX);
		
		auto pSkyboxEntity = mpWorld->CreateEntity("Skybox");
		auto pSkyboxContainer = pSkyboxEntity->AddComponent<CStaticMeshContainer>();
		pSkyboxContainer->SetMeshName("Cube");
		pSkyboxContainer->SetMaterialName("DefaultSkybox.material");
	}

	IGeometryBuilder* pGeomBuilder = CreateGeometryBuilder(result);
	{
		pGeomBuilder->CreateCylinderGeometry(ZeroVector3, UpVector3, 1.0f, 2.0f, 3);
		pGeomBuilder->CreatePlaneGeometry(ZeroVector3, UpVector3, 10.0f, 10.0f, 3);
	}
	pGeomBuilder->Free();
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

	if (auto pLightEntity = mpWorld->CreateEntity())
	{
		if (auto pSunLight = pLightEntity->AddComponent<CDirectionalLight>())
		{
			pSunLight->SetIntensity(1.5f);
			//pSunLight->SetDirection(-TVector3(1.0f, 1.0f, 0.0f));
		}

		if (auto pTransform = pLightEntity->GetComponent<CTransform>())
		{
			pTransform->SetPosition(SunLightPos);
			pTransform->SetRotation(TVector3(45.0f, 45.0f, 0.0f));
		}
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
				if (auto pScene = pSceneManager->GetScene(pSceneManager->CreateScene("main").Get()).Get())
				{
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

			static int id = 0;

			id = imgui->Popup("TestPopup", id, { "First", "Second", "Third" });

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
