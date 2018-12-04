#include "./../include/CCustomEngineListener.h"


TDEngine2::E_RESULT_CODE CCustomEngineListener::OnStart()
{
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

	TDEngine2::E_RESULT_CODE result = TDEngine2::RC_OK;

	mpWorld = TDEngine2::CreateWorld(result);

	for (TDEngine2::I32 i = 0; i < 50; ++i)
	{
		mpWorld->CreateEntity();
	}

	return result;
}

TDEngine2::E_RESULT_CODE CCustomEngineListener::OnUpdate(const float& dt)
{
	mpWorld->Update(dt);

	mpGraphicsContext->ClearBackBuffer(TDEngine2::TColor32F(0.0, 0.0, 0.5f, 1.0));
	mpGraphicsContext->ClearDepthBuffer(1.0f);

	dynamic_cast<TDEngine2::IShader*>(mpShader->Get(TDEngine2::RAT_BLOCKING))->Bind();

	TDEngine2::TPerFrameShaderData data;
	data.mProjMatrix = mpGraphicsContext->CalcPerspectiveMatrix(3.14 * 0.5f, mpWindowSystem->GetWidth() / (TDEngine2::F32)mpWindowSystem->GetHeight(), 1.0f, 1000.0f);
	data.mViewMatrix = TDEngine2::IdentityMatrix4;

	dynamic_cast<TDEngine2::IShader*>(mpShader->Get(TDEngine2::RAT_BLOCKING))->SetInternalUniformsBuffer(TDEngine2::IUBR_PER_FRAME, reinterpret_cast<const TDEngine2::U8*>(&data), sizeof(data));

	mpVertexBuffer->Bind(0, 0);

	mpGraphicsContext->Draw(TDEngine2::E_PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE_LIST, 0, 3);

	mpGraphicsContext->Present();

	mpWindowSystem->SetTitle(std::to_string(mpWindowSystem->GetTimer()->GetDeltaTime()));

	return TDEngine2::RC_OK;
}

TDEngine2::E_RESULT_CODE CCustomEngineListener::OnFree()
{
	TDEngine2::E_RESULT_CODE result = mpWorld->Free();

	if (result != TDEngine2::RC_OK)
	{
		return result;
	}

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
