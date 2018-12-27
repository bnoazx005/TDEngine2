#include "./../include/CCustomEngineListener.h"
#include <TDEngine2.h>
#include <iostream>


TDEngine2::E_RESULT_CODE CCustomEngineListener::OnStart()
{
	TDEngine2::E_RESULT_CODE result = TDEngine2::RC_OK;

	mpShader = mpResourceManager->Load<TDEngine2::CBaseShader>("testDXShader.shader");

	TVertex vertices[] = 
	{
		{TDEngine2::TVector4(-0.5f, -0.5f, 1.0f, 1.0f), TDEngine2::TVector4(1.0f, 0.0f, 0.0f, 1.0f) },
		{TDEngine2::TVector4(0.0f, 0.5f, 1.0f, 1.0f), TDEngine2::TVector4(0.0f, 1.0f, 0.0f, 1.0f) },
		{TDEngine2::TVector4(0.5f, -0.5f, 1.0f, 1.0f), TDEngine2::TVector4(0.0f, 0.0f, 1.0f, 1.0f) }
	};

	mpVertexBuffer = mpGraphicsObjectManager->CreateVertexBuffer(TDEngine2::BUT_DEFAULT, 100, vertices).Get();

	mpIndexBuffer = mpGraphicsObjectManager->CreateIndexBuffer(TDEngine2::BUT_DEFAULT, TDEngine2::IFT_INDEX16, 100, nullptr).Get();
	
	TDEngine2::IVertexDeclaration* pVertexDeclaration = mpGraphicsObjectManager->CreateVertexDeclaration().Get();
	
	pVertexDeclaration->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_POSITION });
	pVertexDeclaration->AddElement({ TDEngine2::FT_FLOAT4, 0, TDEngine2::VEST_COLOR });

	pVertexDeclaration->Bind(mpGraphicsContext, mpVertexBuffer, dynamic_cast<TDEngine2::IShader*>(mpShader->Get(TDEngine2::RAT_BLOCKING)));

	mpWorld = mpEngineCoreInstance->GetWorldInstance();
	
	auto pMaterial = mpResourceManager->Create<TDEngine2::CBaseMaterial>("NewMaterial.material", TDEngine2::TMaterialParameters{ "testDXShader.shader" });

	for (TDEngine2::I32 i = 0; i < 10; ++i)
	{
		auto pEntity = mpWorld->CreateEntity();

		auto pTransform = pEntity->GetComponent<TDEngine2::CTransform>();

		pTransform->SetPosition(pTransform->GetPosition() + TDEngine2::TVector3(0.0f, 0.0f, rand() % 10));

		auto pSprite = pEntity->AddComponent<TDEngine2::CQuadSprite>();

		pSprite->SetMaterialName("NewMaterial.material");
	}

	mpGlobalShaderProperties = TDEngine2::CreateGlobalShaderProperties(mpGraphicsObjectManager, result);

	if (result != TDEngine2::RC_OK)
	{
		return result;
	}

	return TDEngine2::RC_OK;
}

TDEngine2::E_RESULT_CODE CCustomEngineListener::OnUpdate(const float& dt)
{
	mpWindowSystem->SetTitle(std::to_string(dt));

	if (dynamic_cast<TDEngine2::IDesktopInputContext*>(mpEngineCoreInstance->GetSubsystem(TDEngine2::EST_INPUT_CONTEXT))->IsMouseButtonPressed(0))
	{
		std::cout << "pressed\n";
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
