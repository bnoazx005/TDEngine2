#include "./../include/CCustomEngineListener.h"


TDEngine2::E_RESULT_CODE CCustomEngineListener::OnStart()
{
	return TDEngine2::RC_OK;
}

TDEngine2::E_RESULT_CODE CCustomEngineListener::OnUpdate(const float& dt)
{
	mpGraphicsContext->ClearBackBuffer(TDEngine2::TColor32F(0.0, 0.0, 0.5f, 1.0));
	mpGraphicsContext->Present();

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
}
