#include "./../include/CCustomEngineListener.h"
#include <TDEngine2.h>
#include <iostream>


using namespace TDEngine2;


E_RESULT_CODE CCustomEngineListener::OnStart()
{
	E_RESULT_CODE result = RC_OK;

	mpWorld = mpEngineCoreInstance->GetWorldInstance();
	
	IMaterial* pMaterial = mpResourceManager->Create<CBaseMaterial>("NewMaterial.material", TMaterialParameters{ "testGLShader.shader", true })->Get<IMaterial>(RAT_BLOCKING);
	pMaterial->SetTextureResource("TextureAtlas", mpResourceManager->Load<CBaseTexture2D>("Tim.tga")->Get<ITexture2D>(RAT_BLOCKING));
	pMaterial->SetTextureResource("SkyboxTexture", mpResourceManager->Load<CBaseCubemapTexture>("DefaultSkybox")->Get<ICubemapTexture>(RAT_BLOCKING));

	mpResourceManager->Create<CBaseMaterial>("DebugMaterial.material", TMaterialParameters{ "DebugGLShader.shader" });
	
	IMaterial* pFontMaterial = mpResourceManager->Create<CBaseMaterial>("DebugTextMaterial.material", 
																		TMaterialParameters{ "DebugTextGLShader.shader", true })->Get<IMaterial>(RAT_BLOCKING);

	auto pFontAtlas = mpResourceManager->Load<CTextureAtlas>("atlas")->Get<ITextureAtlas>(RAT_BLOCKING);
	pFontMaterial->SetTextureResource("FontTextureAtlas", pFontAtlas->GetTexture());

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

	return RC_OK;
}

E_RESULT_CODE CCustomEngineListener::OnUpdate(const float& dt)
{	
	if (mpInputContext->IsMouseButtonPressed(0))
	{
		std::cout << "pressed\n";
	}

	CTransform* pCameraTransform = mpCameraEntity->GetComponent<CTransform>();
	
	if (mpInputContext->IsKeyPressed(E_KEYCODES::KC_ESCAPE))
	{
		return mpEngineCoreInstance->Quit();
	}

	if (mpInputContext->IsKey(E_KEYCODES::KC_W))
	{
		pCameraTransform->SetPosition(pCameraTransform->GetPosition() + dt * 5.0f * UpVector3);
	}

	if (mpInputContext->IsKey(E_KEYCODES::KC_S))
	{
		pCameraTransform->SetPosition(pCameraTransform->GetPosition() - dt * 5.0f * UpVector3);
	}

	if (mpInputContext->IsKey(E_KEYCODES::KC_A))
	{
		pCameraTransform->SetPosition(pCameraTransform->GetPosition() - dt * 5.0f * RightVector3);
	}

	if (mpInputContext->IsKey(E_KEYCODES::KC_D))
	{
		pCameraTransform->SetPosition(pCameraTransform->GetPosition() + dt * 5.0f * RightVector3);
	}

	if (mpInputContext->IsKey(E_KEYCODES::KC_Q))
	{
		pCameraTransform->SetPosition(pCameraTransform->GetPosition() - dt * 5.0f * ForwardVector3);
	}

	if (mpInputContext->IsKey(E_KEYCODES::KC_E))
	{
		pCameraTransform->SetPosition(pCameraTransform->GetPosition() + dt * 5.0f * ForwardVector3);
	}
	
	auto pDebugUtility = mpGraphicsObjectManager->CreateDebugUtility(mpResourceManager, mpEngineCoreInstance->GetSubsystem<IRenderer>()).Get();
	pDebugUtility->DrawLine(ZeroVector3, { 10.0f, 10.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f });
	pDebugUtility->DrawLine(ZeroVector3, { -10.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f });
	pDebugUtility->DrawLine(ZeroVector3, { -10.0f, 10.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f });
	pDebugUtility->DrawLine(ZeroVector3, { 10.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f });
	pDebugUtility->DrawText({ 0, 0 }, 0.008f, std::to_string(dt), { 1.0f, 1.0f, 1.0f, 1.0f });
	pDebugUtility->DrawCross(ZeroVector3, 1.0f, { 1.0f, 0.0f, 0.0f, 1.0f });
	pDebugUtility->DrawRect({ 0.0f, 0.0f, 2.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f });

	// rotate the cube
	auto pEntity = mpWorld->FindEntity(mpWorld->FindEntitiesWithComponents<CStaticMeshContainer>()[0]);
	auto pTransform = pEntity->GetComponent<CTransform>();
	static F32 time = 0.0f;
	time += 5.0f * dt;
	pTransform->SetRotation(TDEngine2::TQuaternion(ForwardVector3, time));

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
