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

	mCurrEditableEffectId = TResourceId::Invalid;
	mLastSavedPath = Wrench::StringUtils::GetEmptyStr();

	mCurrEditableEffectId = mpResourceManager->Load<CParticleEffect>("testParticles.particles");
	TDE2_ASSERT(TResourceId::Invalid != mCurrEditableEffectId);

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


static E_RESULT_CODE SaveToFile(IFileSystem* pFileSystem, IResourceManager* pResourceManager, TResourceId resourceId, const std::string& destFilePath)
{
	if (destFilePath.empty() || (TResourceId::Invalid == resourceId) || !pFileSystem || !pResourceManager)
	{
		return RC_INVALID_ARGS;
	}

	E_RESULT_CODE result = RC_OK;

	if (auto openFileResult = pFileSystem->Open<IYAMLFileWriter>(destFilePath, true))
	{
		if (auto pFileWriter = pFileSystem->Get<IYAMLFileWriter>(openFileResult.Get()))
		{
			if (auto pParticleEffect = pResourceManager->GetResource<IParticleEffect>(resourceId))
			{
				if (RC_OK != (result = pParticleEffect->Save(pFileWriter)))
				{
					return result;
				}
			}

			if (RC_OK != (result = pFileWriter->Close()))
			{
				return result;
			}
		}
	}

	return RC_OK;
}


void CUtilityListener::_drawMainMenu()
{
	auto pImGUIContext = mpEngineCoreInstance->GetSubsystem<IImGUIContext>();
	auto pFileSystem = mpEngineCoreInstance->GetSubsystem<IFileSystem>();
	
	pImGUIContext->DisplayMainMenu([this, pFileSystem](IImGUIContext& imguiContext)
	{
		imguiContext.MenuGroup("File", [this, pFileSystem](IImGUIContext& imguiContext)
		{
			imguiContext.MenuItem("New Effect", "CTRL+N", [this]
			{
				mLastSavedPath = Wrench::StringUtils::GetEmptyStr();
			});

			imguiContext.MenuItem("Open Effect", "CTRL+O", []
			{
			});

			imguiContext.MenuItem("Save Effect", "CTRL+S", [this, pFileSystem]
			{
				SaveToFile(pFileSystem, mpResourceManager, mCurrEditableEffectId, mLastSavedPath);
			});

			imguiContext.MenuItem("Save Effect As...", "SHIFT+CTRL+S", [this, pFileSystem]
			{
				if (auto saveFileDialogResult = mpWindowSystem->ShowSaveFileDialog({}))
				{
					mLastSavedPath = saveFileDialogResult.Get();
					SaveToFile(pFileSystem, mpResourceManager, mCurrEditableEffectId, mLastSavedPath);
				}
			});

			imguiContext.MenuItem("Quit", "Ctrl+Q", [this] { mpEngineCoreInstance->Quit(); });
		});
	});
}