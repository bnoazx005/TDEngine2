#include "../include/CUtilityListener.h"
#include "../include/CParticleEditorWindow.h"
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

	mpParticleEditor = TDEngine2::CreateParticleEditorWindow(result);

	auto handler = mpResourceManager->Load<CParticleEffect>("testParticles.particles");
	TDE2_ASSERT(TResourceId::Invalid != handler);

	return RC_OK;
}

TDEngine2::E_RESULT_CODE CUtilityListener::OnUpdate(const float& dt)
{
	mpParticleEditor->Draw(mpEngineCoreInstance->GetSubsystem<IImGUIContext>(), dt);

	E_RESULT_CODE result = RC_OK;
	static CAnimationCurveEditorWindow* pCurveEditor = dynamic_cast<CAnimationCurveEditorWindow*>(CreateAnimationCurveEditorWindow(result));
	static auto pCurve = CreateAnimationCurve({}, result);
	
	static bool init = false;
	if (!init)
	{
		pCurve->AddPoint({ 0.0f, 0.0f, -RightVector2 * 0.5f, RightVector2 * 0.5f });
		pCurve->AddPoint({ 0.5f, 0.5f, -RightVector2 * 0.3f, RightVector2 * 0.3f });
		pCurve->AddPoint({ 1.0f, 1.0f, -RightVector2 * 0.5f, RightVector2 * 0.5f });

		pCurveEditor->SetCurveForEditing(pCurve);

		init = true;
	}

	pCurveEditor->Draw(mpEngineCoreInstance->GetSubsystem<IImGUIContext>(), dt);

	return RC_OK;
}

TDEngine2::E_RESULT_CODE CUtilityListener::OnFree()
{
	SafeFree(mpParticleEditor);

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
