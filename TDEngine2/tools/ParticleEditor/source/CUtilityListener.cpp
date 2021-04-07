#include "../include/CUtilityListener.h"
#include "../include/CParticleEditorWindow.h"
#include <memory>
#include <vector>
#include <string>
#include <tuple>


using namespace TDEngine2;


const std::vector<std::tuple<std::string, std::string>> FileExtensionsFilter
{
	{ "Particle Effects", "*.particles" }
};


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

	mpParticleEditor = dynamic_cast<CParticleEditorWindow*>(TDEngine2::CreateParticleEditorWindow(mpResourceManager, mpEngineCoreInstance->GetSubsystem<IInputContext>(), result));

	mCurrEditableEffectId = TResourceId::Invalid;
	mLastSavedPath = Wrench::StringUtils::GetEmptyStr();

	mCurrEditableEffectId = mpResourceManager->Load<CParticleEffect>("testParticles.particles");
	TDE2_ASSERT(TResourceId::Invalid != mCurrEditableEffectId);

	mpParticleEditor->SetParticleEffectResourceHandle(mCurrEditableEffectId);

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

			mpEditableEffectEntity = pParticlesEntity;
		}
	}

	return RC_OK;
}

TDEngine2::E_RESULT_CODE CUtilityListener::OnUpdate(const float& dt)
{
	mpParticleEditor->Draw(mpEngineCoreInstance->GetSubsystem<IImGUIContext>(), dt);

	_drawMainMenu();
	_drawTimeControlBar();

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


static TResult<TResourceId> OpenFromFile(IWindowSystem* pWindowSystem, IFileSystem* pFileSystem, IResourceManager* pResourceManager)
{
	if (!pWindowSystem || !pFileSystem || !pResourceManager)
	{
		return Wrench::TErrValue<E_RESULT_CODE>(RC_INVALID_ARGS);
	}

	auto openFileResult = pWindowSystem->ShowOpenFileDialog(FileExtensionsFilter);
	if (openFileResult.HasError())
	{
		return Wrench::TErrValue<E_RESULT_CODE>(openFileResult.GetError());
	}

	return Wrench::TOkValue<TResourceId>(pResourceManager->Load<CParticleEffect>(openFileResult.Get()));
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
				// \todo Add reset of the app's state here
			});

			imguiContext.MenuItem("Open Effect", "CTRL+O", [this, pFileSystem]
			{
				if (auto openFileResult = OpenFromFile(mpWindowSystem, pFileSystem, mpResourceManager))
				{
					mCurrEditableEffectId = openFileResult.Get();

					if (auto pParticles = mpEditableEffectEntity->GetComponent<CParticleEmitter>())
					{
						if (auto pParticleEffect = mpResourceManager->GetResource<IResource>(mCurrEditableEffectId))
						{
							pParticles->SetParticleEffect(pParticleEffect->GetName());
						}

						mpParticleEditor->SetParticleEffectResourceHandle(mCurrEditableEffectId);
					}
				}
			});

			imguiContext.MenuItem("Save Effect", "CTRL+S", [this, pFileSystem]
			{
				SaveToFile(pFileSystem, mpResourceManager, mCurrEditableEffectId, mLastSavedPath);
			});

			imguiContext.MenuItem("Save Effect As...", "SHIFT+CTRL+S", [this, pFileSystem]
			{
				if (auto saveFileDialogResult = mpWindowSystem->ShowSaveFileDialog(FileExtensionsFilter))
				{
					mLastSavedPath = saveFileDialogResult.Get();
					SaveToFile(pFileSystem, mpResourceManager, mCurrEditableEffectId, mLastSavedPath);
				}
			});

			imguiContext.MenuItem("Quit", "Ctrl+Q", [this] { mpEngineCoreInstance->Quit(); });
		});
	});
}

void CUtilityListener::_drawTimeControlBar()
{
	auto pImGUIContext = mpEngineCoreInstance->GetSubsystem<IImGUIContext>();
	auto pWorld = mpEngineCoreInstance->GetSubsystem<ISceneManager>()->GetWorld();

	static const IImGUIContext::TWindowParams params
	{
		ZeroVector2,
		TVector2(350.0f, 60.0f),
		TVector2(350.0f, 60.0f),
	};

	if (mShouldPauseOnNextFrame)
	{
		pWorld->SetTimeScaleFactor(0.0f);
		mShouldPauseOnNextFrame = false;
	}

	if (pImGUIContext->BeginWindow("Time Control", mIsTimeControlBarWindowOpened, params))
	{
		pImGUIContext->BeginHorizontal();

		pImGUIContext->Button(pWorld->GetTimeScaleFactor() > 0.0f ? "Pause" : "Play", TVector2(80.0f, 20.0f), [pWorld]
		{
			pWorld->SetTimeScaleFactor(1.0f - pWorld->GetTimeScaleFactor());
		});

		pImGUIContext->Button("Step", TVector2(80.0f, 20.0f), [pWorld, this]
		{
			pWorld->SetTimeScaleFactor(1.0f);
			mShouldPauseOnNextFrame = true;
		});

		pImGUIContext->EndHorizontal();

		pImGUIContext->EndWindow();
	}
}