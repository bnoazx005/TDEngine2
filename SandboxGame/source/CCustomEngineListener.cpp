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

	pMaterial->SetTextureResource("TextureAtlas", dynamic_cast<TDEngine2::ITexture2D*>(mpResourceManager->Load<TDEngine2::CBaseTexture2D>("Tim.png")->Get(TDEngine2::RAT_BLOCKING)));

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
	}

	mpCameraEntity = mpWorld->CreateEntity("Camera");

	mpCameraEntity->AddComponent<TDEngine2::CPerspectiveCamera>();
	
	mpGlobalShaderProperties = TDEngine2::CreateGlobalShaderProperties(mpGraphicsObjectManager, result);

	mpInputContext = dynamic_cast<TDEngine2::IDesktopInputContext*>(mpEngineCoreInstance->GetSubsystem(TDEngine2::EST_INPUT_CONTEXT));

	if (result != TDEngine2::RC_OK)
	{
		return result;
	}

	TDEngine2::TTextureSamplerDesc textureSamplerDesc;

	textureSamplerDesc.mUAddressMode = TDEngine2::E_ADDRESS_MODE_TYPE::AMT_WRAP;
	textureSamplerDesc.mVAddressMode = TDEngine2::E_ADDRESS_MODE_TYPE::AMT_WRAP;
	textureSamplerDesc.mWAddressMode = TDEngine2::E_ADDRESS_MODE_TYPE::AMT_WRAP;

	mTextureSampler = mpGraphicsObjectManager->CreateTextureSampler(textureSamplerDesc).Get();
	
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

	TDEngine2::TVector2 lThumbShift = mpInputContext->GetGamepad(0)->GetLThumbShiftVec();

	std::cout << lThumbShift.x << " " << lThumbShift.y << std::endl;

	auto ms = mpInputContext->GetMousePosition();

	std::cout << "dx: " << ms.x << "; dy: " << ms.y << "\n";
	
	pCameraTransform->SetPosition(pCameraTransform->GetPosition() + dt * 5.0f * TDEngine2::TVector3(lThumbShift.x, lThumbShift.y, 0.0f));

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
