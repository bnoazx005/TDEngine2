#include "./../include/CCustomEngineListener.h"
#include <TDEngine2.h>
#include <iostream>

TDEngine2::E_RESULT_CODE CCustomEngineListener::OnStart()
{
	TDEngine2::E_RESULT_CODE result = TDEngine2::RC_OK;

	mpWorld = mpEngineCoreInstance->GetWorldInstance();
	
	TDEngine2::IMaterial* pMaterial = dynamic_cast<TDEngine2::IMaterial*>(
										mpResourceManager->Create<TDEngine2::CBaseMaterial>("NewMaterial.material", 
																							TDEngine2::TMaterialParameters{ "testGLShader.shader" })->Get(TDEngine2::RAT_BLOCKING));

	mpResourceManager->Create<TDEngine2::CBaseMaterial>("DebugMaterial.material", TDEngine2::TMaterialParameters{ "DebugGLShader.shader" });
	
	TDEngine2::IMaterial* pFontMaterial = dynamic_cast<TDEngine2::IMaterial*>(
												mpResourceManager->Create<TDEngine2::CBaseMaterial>("DebugTextMaterial.material", 
																							TDEngine2::TMaterialParameters{ "DebugTextGLShader.shader" })->Get(TDEngine2::RAT_BLOCKING));

	pMaterial->SetTextureResource("TextureAtlas", dynamic_cast<TDEngine2::ITexture2D*>(mpResourceManager->Load<TDEngine2::CBaseTexture2D>("Tim.tga")->Get(TDEngine2::RAT_BLOCKING)));

	auto pFontAtlas = dynamic_cast<TDEngine2::ITextureAtlas*>(mpResourceManager->Load<TDEngine2::CTextureAtlas>("atlas")->Get(TDEngine2::RAT_BLOCKING));

	pFontMaterial->SetTextureResource("FontTextureAtlas", pFontAtlas->GetTexture());

	const TDEngine2::TColor32F colors[] =
	{
		{ 1.0f, 0.0f, 0.0f, 1.0f },
		{ 0.0f, 1.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 1.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 0.0f, 1.0f, 1.0f, 1.0f },
		{ 1.0f, 0.0f, 1.0f, 1.0f },
		{ 1.0f, 1.0f, 0.0f, 1.0f },
	};

	for (TDEngine2::I32 i = 0; i < 10; ++i)
	{
		auto pEntity = mpWorld->CreateEntity();

		auto pTransform = pEntity->GetComponent<TDEngine2::CTransform>();

		pTransform->SetPosition(pTransform->GetPosition() + TDEngine2::TVector3(rand() % 5 - 2.0f, rand() % 5, rand() % 10));

		auto pSprite = pEntity->AddComponent<TDEngine2::CQuadSprite>();

		pSprite->SetColor(colors[rand() % 7]);

		pSprite->SetMaterialName("NewMaterial.material");

		//auto pBoxCollision = pEntity->AddComponent<TDEngine2::CBoxCollisionObject2D>();
	}
	
	mpCameraEntity = mpWorld->CreateEntity("Camera");

	mpCameraEntity->AddComponent<TDEngine2::COrthoCamera>();
	
	mpInputContext = dynamic_cast<TDEngine2::IDesktopInputContext*>(mpEngineCoreInstance->GetSubsystem(TDEngine2::EST_INPUT_CONTEXT));

	if (result != TDEngine2::RC_OK)
	{
		return result;
	}

	TDEngine2::TTextureSamplerDesc textureSamplerDesc;
/*
	textureSamplerDesc.mUAddressMode = TDEngine2::E_ADDRESS_MODE_TYPE::AMT_WRAP;
	textureSamplerDesc.mVAddressMode = TDEngine2::E_ADDRESS_MODE_TYPE::AMT_WRAP;
	textureSamplerDesc.mWAddressMode = TDEngine2::E_ADDRESS_MODE_TYPE::AMT_WRAP;*/

	textureSamplerDesc.mUAddressMode = TDEngine2::E_ADDRESS_MODE_TYPE::AMT_CLAMP;
	textureSamplerDesc.mVAddressMode = TDEngine2::E_ADDRESS_MODE_TYPE::AMT_CLAMP;
	textureSamplerDesc.mWAddressMode = TDEngine2::E_ADDRESS_MODE_TYPE::AMT_CLAMP;

	mTextureSampler = mpGraphicsObjectManager->CreateTextureSampler(textureSamplerDesc).Get();
	
	auto jobManager = dynamic_cast<TDEngine2::IJobManager*>(mpEngineCoreInstance->GetSubsystem(TDEngine2::EST_JOB_MANAGER));

	auto fileSystem = dynamic_cast<TDEngine2::IFileSystem*>(mpEngineCoreInstance->GetSubsystem(TDEngine2::EST_FILE_SYSTEM));

	pMaterial->SetTextureResource("SkyboxTexture", dynamic_cast<TDEngine2::ICubemapTexture*>(mpResourceManager->Load<TDEngine2::CBaseCubemapTexture>("DefaultSkybox")->Get(TDEngine2::RAT_BLOCKING)));

	return TDEngine2::RC_OK;
}

TDEngine2::E_RESULT_CODE CCustomEngineListener::OnUpdate(const float& dt)
{
	mpWindowSystem->SetTitle(std::to_string(dt));

	mpGraphicsContext->BindTextureSampler(0, mTextureSampler);
	
	if (mpInputContext->IsMouseButtonPressed(0))
	{
		std::cout << "pressed\n";
	}

	TDEngine2::CTransform* pCameraTransform = mpCameraEntity->GetComponent<TDEngine2::CTransform>();
	
	if (mpInputContext->IsKeyPressed(TDEngine2::E_KEYCODES::KC_ESCAPE))
	{
		return mpEngineCoreInstance->Quit();
	}

	if (mpInputContext->IsKey(TDEngine2::E_KEYCODES::KC_W))
	{
		pCameraTransform->SetPosition(pCameraTransform->GetPosition() + dt * 5.0f * TDEngine2::UpVector3);
	}

	if (mpInputContext->IsKey(TDEngine2::E_KEYCODES::KC_S))
	{
		pCameraTransform->SetPosition(pCameraTransform->GetPosition() - dt * 5.0f * TDEngine2::UpVector3);
	}

	if (mpInputContext->IsKey(TDEngine2::E_KEYCODES::KC_A))
	{
		pCameraTransform->SetPosition(pCameraTransform->GetPosition() - dt * 5.0f * TDEngine2::RightVector3);
	}

	if (mpInputContext->IsKey(TDEngine2::E_KEYCODES::KC_D))
	{
		pCameraTransform->SetPosition(pCameraTransform->GetPosition() + dt * 5.0f * TDEngine2::RightVector3);
	}
	
	auto pDebugUtility = mpGraphicsObjectManager->CreateDebugUtility(mpResourceManager, dynamic_cast<TDEngine2::IRenderer*>(mpEngineCoreInstance->GetSubsystem(TDEngine2::EST_RENDERER))).Get();
	pDebugUtility->DrawLine(TDEngine2::ZeroVector3, { 10.0f, 10.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f });
	pDebugUtility->DrawLine(TDEngine2::ZeroVector3, { -10.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f });
	pDebugUtility->DrawLine(TDEngine2::ZeroVector3, { -10.0f, 10.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f });
	pDebugUtility->DrawLine(TDEngine2::ZeroVector3, { 10.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f });
	pDebugUtility->DrawText({ 0, 0 }, 0.01f, "Test sample", { 1.0f, 1.0f, 1.0f, 1.0f });
	pDebugUtility->DrawCross(TDEngine2::ZeroVector3, 1.0f, { 1.0f, 0.0f, 0.0f, 1.0f });

	return TDEngine2::RC_OK;
}

TDEngine2::E_RESULT_CODE CCustomEngineListener::OnFree()
{
	return TDEngine2::RC_OK;
}

void CCustomEngineListener::SetEngineInstance(TDEngine2::IEngineCore* pEngineCore)
{
	if (!pEngineCore)
	{
		return;
	}

	mpEngineCoreInstance = pEngineCore;

	mpGraphicsContext = dynamic_cast<TDEngine2::IGraphicsContext*>(mpEngineCoreInstance->GetSubsystem(TDEngine2::EST_GRAPHICS_CONTEXT));

	mpGraphicsObjectManager = mpGraphicsContext->GetGraphicsObjectManager();

	mpWindowSystem = dynamic_cast<TDEngine2::IWindowSystem*>(mpEngineCoreInstance->GetSubsystem(TDEngine2::EST_WINDOW));

	mpResourceManager = dynamic_cast<TDEngine2::IResourceManager*>(mpEngineCoreInstance->GetSubsystem(TDEngine2::EST_RESOURCE_MANAGER));
}
