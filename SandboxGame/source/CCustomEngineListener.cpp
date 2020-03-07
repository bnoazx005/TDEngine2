#include "./../include/CCustomEngineListener.h"
#include <TDEngine2.h>
#include <iostream>


using namespace TDEngine2;


#define DX_GAPI 0

#if DX_GAPI
	const std::string ShaderType = "HLSL";
	const std::string GAPIType   = "DX";
#else
	const std::string ShaderType = "GLSL";
	const std::string GAPIType = "GL";
#endif


const std::string TestShaderName      = CStringUtils::Format("test{0}Shader.shader", GAPIType);
const std::string DebugShaderName     = CStringUtils::Format("Debug{0}Shader.shader", GAPIType);
const std::string DebugTextShaderName = CStringUtils::Format("DebugText{0}Shader.shader", GAPIType);


E_RESULT_CODE CCustomEngineListener::OnStart()
{
	E_RESULT_CODE result = RC_OK;

	mpWorld = mpEngineCoreInstance->GetWorldInstance();
	
	IMaterial* pMaterial = mpResourceManager->Create<CBaseMaterial>("NewMaterial.material", TMaterialParameters{ TestShaderName, true })->Get<IMaterial>(RAT_BLOCKING);
	pMaterial->SetTextureResource("TextureAtlas", mpResourceManager->Load<CBaseTexture2D>("Tim.tga")->Get<ITexture2D>(RAT_BLOCKING));
	pMaterial->SetTextureResource("SkyboxTexture", mpResourceManager->Load<CBaseCubemapTexture>("DefaultSkybox")->Get<ICubemapTexture>(RAT_BLOCKING));

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

		auto pTransform = pEntity->GetComponent<CTransform>();

		pTransform->SetPosition(pTransform->GetPosition() + TVector3(rand() % 5 - 2.0f, rand() % 5, rand() % 10));

		auto pSprite = pEntity->AddComponent<CQuadSprite>();

		pSprite->SetMaterialName("NewMaterial.material");
		pSprite->SetColor(colors[rand() % 7]);

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

	mpResourceManager->Load<CStaticMesh>("Test.mesh");

	auto pMeshEntity = mpWorld->CreateEntity();
	auto pMeshTransform = pMeshEntity->GetComponent<CTransform>();
	pMeshTransform->SetPosition({ 0.0f, 0.0f, 2.0f });
	auto pMeshContainer = pMeshEntity->AddComponent<CStaticMeshContainer>();
	pMeshContainer->SetMaterialName("DebugMaterial.material");
	pMeshContainer->SetMeshName("Cube");
	auto collision = pMeshEntity->AddComponent<CBoxCollisionObject3D>();
	collision->SetCollisionType(E_COLLISION_OBJECT_TYPE::COT_KINEMATIC);

	{
		TMaterialParameters skyboxMatParams 
		{ 
			mpFileSystem->ResolveVirtualPath(CStringUtils::Format("vfs://Shaders/Default/DefaultSkyboxShader_{0}.shader", ShaderType), false), true,
			{ true, true, E_COMPARISON_FUNC::LESS_EQUAL}, 
			{ E_CULL_MODE::NONE, false, false, 0.0f, 1.0f, false }
		};

		IMaterial* pMaterial = mpResourceManager->Create<CBaseMaterial>("DefaultSkybox.material", skyboxMatParams)->Get<IMaterial>(RAT_BLOCKING);
		pMaterial->SetTextureResource("SkyboxTexture", mpResourceManager->Load<CBaseCubemapTexture>("DefaultSkybox")->Get<ICubemapTexture>(RAT_BLOCKING));
		pMaterial->SetGeometrySubGroupTag(E_GEOMETRY_SUBGROUP_TAGS::SKYBOX);
		
		auto pSkyboxEntity = mpWorld->CreateEntity("Skybox");
		auto pSkyboxContainer = pSkyboxEntity->AddComponent<CStaticMeshContainer>();
		pSkyboxContainer->SetMeshName("Cube");
		pSkyboxContainer->SetMaterialName("DefaultSkybox.material");
	}

	return RC_OK;
}

E_RESULT_CODE CCustomEngineListener::OnUpdate(const float& dt)
{
	TDE2_PROFILER_SCOPE("OnUpdate");

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

		}
		imgui->EndWindow();
	}	

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
	auto pEntity = mpWorld->FindEntity(mpWorld->FindEntitiesWithComponents<CStaticMeshContainer>()[0]);
	auto pTransform = pEntity->GetComponent<CTransform>();
	static F32 time = 0.0f;
	time += 5.0f * dt;

	pTransform->SetScale(TVector3(0.5f + 0.5f * fabs(sinf(0.5f * time))));
	pTransform->SetRotation(TVector3(0.0f, time, 0.0f));
	auto&& m = pTransform->GetTransform();

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
