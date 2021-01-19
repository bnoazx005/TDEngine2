#include "./../include/CUtilityListener.h"
#include "../include/CAnimationEditorWindow.h"
#include <memory>


using namespace TDEngine2;


TDEngine2::E_RESULT_CODE CUtilityListener::OnStart()
{
	TDEngine2::E_RESULT_CODE result = RC_OK;

	IWorld* pWorld = mpEngineCoreInstance->GetWorldInstance();

	// \note Create an editor's camera

	if (CEntity* pCameraEntity = pWorld->CreateEntity("Camera"))
	{
		if (auto pCamera = pCameraEntity->AddComponent<CPerspectiveCamera>())
		{
			pCamera->SetAspect(mpWindowSystem->GetWidth() / static_cast<F32>(mpWindowSystem->GetHeight()));
			pCamera->SetFOV(0.5f * CMathConstants::Pi);
		}
	}

	mpAnimationEditor = TDEngine2::CreateAnimationEditorWindow(result);

	return RC_OK;
}

TDEngine2::E_RESULT_CODE CUtilityListener::OnUpdate(const float& dt)
{
	mpAnimationEditor->Draw(mpEngineCoreInstance->GetSubsystem<IImGUIContext>(), dt);

	return RC_OK;
}

TDEngine2::E_RESULT_CODE CUtilityListener::OnFree()
{
	SafeFree(mpAnimationEditor);

	return RC_OK;
}

void CUtilityListener::SetEngineInstance(TDEngine2::IEngineCore* pEngineCore)
{
	if (!pEngineCore)
	{
		return;
	}

	mpEngineCoreInstance = pEngineCore;

	mpGraphicsContext = mpEngineCoreInstance->GetSubsystem<TDEngine2::IGraphicsContext>();

	mpWindowSystem = mpEngineCoreInstance->GetSubsystem<TDEngine2::IWindowSystem>();

	mpResourceManager = mpEngineCoreInstance->GetSubsystem<TDEngine2::IResourceManager>();
}
