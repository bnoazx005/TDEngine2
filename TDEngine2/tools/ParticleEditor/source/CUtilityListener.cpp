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

	ISceneManager* pSceneManager = mpEngineCoreInstance->GetSubsystem<ISceneManager>();

	if (auto getSceneResult = pSceneManager->GetScene(MainScene))
	{
		if (auto pParticlesEntity = getSceneResult.Get()->CreateEntity(EditableEntityId))
		{
			if (CTransform* pTransform = pParticlesEntity->GetComponent<CTransform>())
			{
				pTransform->SetPosition(ForwardVector3 * 2.0f);
			}

			if (auto pParticles = pParticlesEntity->AddComponent<CParticleEmitter>())
			{
				pParticles->SetParticleEffect("testParticles.particles");
			}
		}
	}

	return RC_OK;
}

TDEngine2::E_RESULT_CODE CUtilityListener::OnUpdate(const float& dt)
{
	mpParticleEditor->Draw(mpEngineCoreInstance->GetSubsystem<IImGUIContext>(), dt);

	_drawMainMenu();

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

void CUtilityListener::_drawMainMenu()
{
	auto pImGUIContext = mpEngineCoreInstance->GetSubsystem<IImGUIContext>();

	pImGUIContext->DisplayMainMenu([this](IImGUIContext& imguiContext)
	{
		imguiContext.MenuGroup("File", [this](IImGUIContext& imguiContext)
		{
			imguiContext.MenuItem("New Effect", "CTRL+N", []
			{
			});

			imguiContext.MenuItem("Open Effect", "CTRL+O", []
			{
			});

			imguiContext.MenuItem("Save Effect", "CTRL+S", []
			{

			});

			imguiContext.MenuItem("Save Effect As...", "SHIFT+CTRL+S", []
			{

			});

			imguiContext.MenuItem("Quit", "Ctrl+Q", [this] { mpEngineCoreInstance->Quit(); });
		});
	});
}