#include "../include/CUtilityListener.h"


using namespace TDEngine2;


E_RESULT_CODE CUtilityListener::OnStart()
{
	E_RESULT_CODE result = RC_OK;

	mpEditorWindow = dynamic_cast<CEditorWindow*>(TDEngine2::CreateEditorWindow(mpResourceManager, mpEngineCoreInstance->GetSubsystem<IInputContext>(), result));

	return RC_OK;
}

E_RESULT_CODE CUtilityListener::OnUpdate(const float& dt)
{
	mpEditorWindow->Draw(mpEngineCoreInstance->GetSubsystem<IImGUIContext>(), dt);

	return RC_OK;
}

E_RESULT_CODE CUtilityListener::OnFree()
{
	return RC_OK;
}

void CUtilityListener::SetEngineInstance(TDEngine2::IEngineCore* pEngineCore)
{
	if (!pEngineCore)
	{
		return;
	}

	mpEngineCoreInstance = pEngineCore;

	mpGraphicsContext = mpEngineCoreInstance->GetSubsystem<IGraphicsContext>();

	mpWindowSystem = mpEngineCoreInstance->GetSubsystem<IWindowSystem>();

	mpResourceManager = mpEngineCoreInstance->GetSubsystem<IResourceManager>();
}
