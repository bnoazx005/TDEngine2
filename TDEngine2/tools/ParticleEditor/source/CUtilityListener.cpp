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


	auto imgui = mpEngineCoreInstance->GetSubsystem<IImGUIContext>();

	static bool isOpened = true;
	if (imgui->BeginWindow("Hello, World!", isOpened, {}))
	{
		const F32 width = imgui->GetWindowWidth() - 20.0f;
		const F32 height = imgui->GetWindowHeight();

		static TVector2 p = ZeroVector2;

		auto t = p;
		imgui->Vector2Field("p", t, [&] { p = t; });

		imgui->DrawPlotGrid("Plot", { width, height - 100.0f }, [&](auto&& pos)
		{
			/*imgui->FloatSlider("px", p.x, -100.0f, 100.0f);
			imgui->FloatSlider("py", p.y, -100.0f, 100.0f);*/


			auto a = pos;
			auto b = pos + TVector2(width, height - 100.0f);


			imgui->DrawCubicBezier(a, a + p, b, b + TVector2(-200.0f, 0.0f), TColorUtils::mGreen);
		});

		imgui->EndWindow();
	}

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
