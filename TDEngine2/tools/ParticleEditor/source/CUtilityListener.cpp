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
		const F32 width = imgui->GetWindowWidth() - 15.0f;
		const F32 height = imgui->GetWindowHeight();

		static TVector2 tangents[2]
		{
			TVector2(200.0f, 0.0f),
			TVector2(-200.0f, 0.0f)
		};

		TVector2* pT0 = &tangents[0];
		TVector2* pT1 = &tangents[1];

		imgui->DrawPlotGrid("Plot", { width, height - 100.0f, 5, 5, { 0.0f, 0.0f, 1.0f, 1.0f } }, [&](auto&& pos)
		{
			auto a = pos;
			auto b = pos + TVector2(width, height - 100.0f);

			imgui->DrawCubicBezier(a, a + tangents[0], b, b + tangents[1], TColorUtils::mGreen);

			imgui->DisplayIDGroup(0, [&, t = *pT0]
			{
				imgui->DrawLine(a, a + t, TColorUtils::mWhite);
				imgui->DrawCircle(a + t, 5.0f, true, TColorUtils::mBlue);

				auto pos = imgui->GetCursorScreenPos();

				imgui->SetCursorScreenPos(a + t);

				imgui->Button("", TVector2(30.0f, 30.0f), {}, true);

				if (imgui->IsItemActive() && imgui->IsMouseDragging(0))
				{
					auto&& mousePos = imgui->GetMousePosition();

					pT0->x = mousePos.x - a.x;
					pT0->y = mousePos.y - a.y;
				}

				imgui->SetCursorScreenPos(pos);
			});

			imgui->DisplayIDGroup(1, [&, t = tangents[1]]
			{
				imgui->DrawLine(b, b + t, TColorUtils::mWhite);
				imgui->DrawCircle(b + t, 5.0f, true, TColorUtils::mBlue);

				auto pos = imgui->GetCursorScreenPos();

				imgui->SetCursorScreenPos(b + t);

				imgui->Button("", TVector2(30.0f, 30.0f), {}, true);

				if (imgui->IsItemActive() && imgui->IsMouseDragging(0))
				{
					auto&& mousePos = imgui->GetMousePosition();

					pT1->x = mousePos.x - b.x;
					pT1->y = mousePos.y - b.y;
				}


				imgui->SetCursorScreenPos(pos);
			});
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
