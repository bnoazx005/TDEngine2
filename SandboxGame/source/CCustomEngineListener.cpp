#include "./../include/CCustomEngineListener.h"
#include <TDEngine2.h>
#include <iostream>


using namespace TDEngine2;


E_RESULT_CODE CCustomEngineListener::OnStart()
{
	E_RESULT_CODE result = RC_OK;

	mpWorld = mpEngineCoreInstance->GetWorldInstance();
	
	IMaterial* pMaterial = dynamic_cast<IMaterial*>(
										mpResourceManager->Create<CBaseMaterial>("NewMaterial.material", 
																							TMaterialParameters{ "testGLShader.shader" })->Get(RAT_BLOCKING));

	mpResourceManager->Create<CBaseMaterial>("DebugMaterial.material", TMaterialParameters{ "DebugGLShader.shader" });
	
	IMaterial* pFontMaterial = dynamic_cast<IMaterial*>(
												mpResourceManager->Create<CBaseMaterial>("DebugTextMaterial.material", 
																							TMaterialParameters{ "DebugTextGLShader.shader" })->Get(RAT_BLOCKING));

	pMaterial->SetTextureResource("TextureAtlas", dynamic_cast<ITexture2D*>(mpResourceManager->Load<CBaseTexture2D>("Tim.tga")->Get(RAT_BLOCKING)));

	auto pFontAtlas = dynamic_cast<ITextureAtlas*>(mpResourceManager->Load<CTextureAtlas>("atlas")->Get(RAT_BLOCKING));

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

		pSprite->SetColor(colors[rand() % 7]);

		pSprite->SetMaterialName("NewMaterial.material");

		//auto pBoxCollision = pEntity->AddComponent<CBoxCollisionObject2D>();
	}

	auto pMeshEntity = mpWorld->CreateEntity();
	
	mpCameraEntity = mpWorld->CreateEntity("Camera");

	mpCameraEntity->AddComponent<COrthoCamera>();
	
	mpInputContext = mpEngineCoreInstance->GetSubsystem<IDesktopInputContext>();

	if (result != RC_OK)
	{
		return result;
	}

	TTextureSamplerDesc textureSamplerDesc;
/*
	textureSamplerDesc.mUAddressMode = E_ADDRESS_MODE_TYPE::AMT_WRAP;
	textureSamplerDesc.mVAddressMode = E_ADDRESS_MODE_TYPE::AMT_WRAP;
	textureSamplerDesc.mWAddressMode = E_ADDRESS_MODE_TYPE::AMT_WRAP;*/

	textureSamplerDesc.mUAddressMode = E_ADDRESS_MODE_TYPE::AMT_CLAMP;
	textureSamplerDesc.mVAddressMode = E_ADDRESS_MODE_TYPE::AMT_CLAMP;
	textureSamplerDesc.mWAddressMode = E_ADDRESS_MODE_TYPE::AMT_CLAMP;

	textureSamplerDesc.mFilterFlags = (U32)E_FILTER_TYPE::FT_BILINEAR << 16 | 
									  (U32)E_FILTER_TYPE::FT_BILINEAR << 8  | 
									  (U32)E_FILTER_TYPE::FT_BILINEAR;

	mTextureSampler = mpGraphicsObjectManager->CreateTextureSampler(textureSamplerDesc).Get();
	
	auto jobManager = mpEngineCoreInstance->GetSubsystem<IJobManager>();

	auto fileSystem = mpEngineCoreInstance->GetSubsystem<IFileSystem>();

	pMaterial->SetTextureResource("SkyboxTexture", dynamic_cast<ICubemapTexture*>(mpResourceManager->Load<CBaseCubemapTexture>("DefaultSkybox")->Get(RAT_BLOCKING)));

	auto pRT = mpResourceManager->Create<CBaseRenderTarget>("default-rt", TTexture2DParameters { mpWindowSystem->GetWidth(), mpWindowSystem->GetHeight(), FT_NORM_UBYTE4, 1, 1, 0 });
	//mpGraphicsContext->BindRenderTarget(dynamic_cast<IRenderTarget*>(pRT->Get(RAT_BLOCKING)));
	return RC_OK;
}

E_RESULT_CODE CCustomEngineListener::OnUpdate(const float& dt)
{
	mpWindowSystem->SetTitle(std::to_string(dt));

	mpGraphicsContext->BindTextureSampler(0, mTextureSampler);
	
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
	
	auto pDebugUtility = mpGraphicsObjectManager->CreateDebugUtility(mpResourceManager, mpEngineCoreInstance->GetSubsystem<IRenderer>()).Get();
	pDebugUtility->DrawLine(ZeroVector3, { 10.0f, 10.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f });
	pDebugUtility->DrawLine(ZeroVector3, { -10.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f });
	pDebugUtility->DrawLine(ZeroVector3, { -10.0f, 10.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f });
	pDebugUtility->DrawLine(ZeroVector3, { 10.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f });
	pDebugUtility->DrawText({ 0, 0 }, 0.008f, "Test sample", { 1.0f, 1.0f, 1.0f, 1.0f });
	pDebugUtility->DrawCross(ZeroVector3, 1.0f, { 1.0f, 0.0f, 0.0f, 1.0f });

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

	mpGraphicsObjectManager = mpGraphicsContext->GetGraphicsObjectManager();

}
