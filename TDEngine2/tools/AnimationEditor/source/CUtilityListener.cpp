#include "./../include/CUtilityListener.h"
#include "../include/CAnimationEditorWindow.h"
#include <memory>


using namespace TDEngine2;


TDEngine2::E_RESULT_CODE CUtilityListener::OnStart()
{
	TDEngine2::E_RESULT_CODE result = RC_OK;

	auto pSceneManager = mpEngineCoreInstance->GetSubsystem<ISceneManager>();

	mpAnimationEditor = dynamic_cast<CAnimationEditorWindow*>(
			TDEngine2::CreateAnimationEditorWindow(
				mpResourceManager, 
				mpEngineCoreInstance->GetWorldInstance(),
				DynamicPtrCast<IDesktopInputContext>(mpEngineCoreInstance->GetSubsystem<IInputContext>()), 
				result));

	mCurrEditableEffectId = mpResourceManager->Create<IAnimationClip>("unnamed.animation", TAnimationClipParameters {});
	mpAnimationEditor->SetAnimationResourceHandle(mCurrEditableEffectId);

#if 0
	TAnimationClipParameters clip;
	clip.mDuration = 2.5f;

	if (IAnimationClipClip* pClip = mpResourceManager->GetResource<IAnimationClipClip>(mpResourceManager->Create<IAnimationClipClip>("Animation2", clip)))
	{
		if (auto pTrack = pClip->GetTrack<IAnimationClipTrack>(pClip->CreateTrack<CVector3AnimationTrack>("testTrack")))
		{
			pTrack->SetPropertyBinding("transform.position");

			pTrack->CreateKey(0.0f);
			pTrack->CreateKey(1.0f);
			auto handle = pTrack->CreateKey(0.2f);
			pTrack->CreateKey(0.35f);
			pTrack->CreateKey(0.12f);
			pTrack->RemoveKey(handle);
		}

		auto pFileSystem = mpEngineCoreInstance->GetSubsystem<IFileSystem>();

		if (auto result = pFileSystem->Open<IYAMLFileWriter>("Animation2.animation", true))
		{
			if (auto animationFileWriter = pFileSystem->Get<IYAMLFileWriter>(result.Get()))
			{
				pClip->Save(animationFileWriter);
				animationFileWriter->Close();
			}
		}
	}
#endif

	if (auto pSceneManager = mpEngineCoreInstance->GetSubsystem<ISceneManager>())
	{
		if (auto sceneResult = pSceneManager->GetScene(pSceneManager->GetSceneId("MainScene.scene")))
		{
			if (auto pScene = sceneResult.Get())
			{
				pScene->CreateSkybox(mpResourceManager.Get(), "Resources/Textures/DefaultSkybox");

				if (CEntity* pEntity = pScene->CreateEntity("AnimableEntity"))
				{
					if (auto pAnimationComponent = pEntity->AddComponent<CAnimationContainerComponent>())
					{
						pAnimationComponent->SetAnimationClipId("unnamed.animation");
					}

					mpAnimationEditor->SetAnimatedEntityId(pEntity->GetId());
					mpEditableEntity = pEntity;
				}
			}
		}
		
	}

	return RC_OK;
}


TDEngine2::E_RESULT_CODE CUtilityListener::OnUpdate(const float& dt)
{
	mpAnimationEditor->Draw(mpEngineCoreInstance->GetSubsystem<IImGUIContext>().Get(), dt);

	DrawEditorGrid(mpGraphicsContext->GetGraphicsObjectManager()->CreateDebugUtility(mpResourceManager.Get(), mpEngineCoreInstance->GetSubsystem<IRenderer>().Get()).Get(), 10, 10);

	_drawMainMenu();

	return RC_OK;
}

TDEngine2::E_RESULT_CODE CUtilityListener::OnFree()
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

	mpGraphicsContext = mpEngineCoreInstance->GetSubsystem<TDEngine2::IGraphicsContext>();

	mpWindowSystem = mpEngineCoreInstance->GetSubsystem<TDEngine2::IWindowSystem>();

	mpResourceManager = mpEngineCoreInstance->GetSubsystem<TDEngine2::IResourceManager>();
}



const std::vector<std::tuple<std::string, std::string>> FileExtensionsFilter
{
	{ "Animations", "*.animation" }
};


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

	return Wrench::TOkValue<TResourceId>(pResourceManager->Load<IAnimationClip>(openFileResult.Get()));
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
			if (auto pAnimation = pResourceManager->GetResource<IAnimationClip>(resourceId))
			{
				if (RC_OK != (result = pAnimation->Save(pFileWriter)))
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
	auto pFileSystem = mpEngineCoreInstance->GetSubsystem<IFileSystem>().Get();

	pImGUIContext->DisplayMainMenu([this, pFileSystem](IImGUIContext& imguiContext)
	{
		imguiContext.MenuGroup("File", [this, pFileSystem](IImGUIContext& imguiContext)
		{
			imguiContext.MenuItem("New", "CTRL+N", [this]
			{
				mLastSavedPath = Wrench::StringUtils::GetEmptyStr();

				mCurrEditableEffectId = mpResourceManager->Create<IAnimationClip>("unnamed.animation", TAnimationClipParameters{});
				mpAnimationEditor->SetAnimationResourceHandle(mCurrEditableEffectId);
			});

			imguiContext.MenuItem("Open", "CTRL+O", [this, pFileSystem]
			{
				if (auto openFileResult = OpenFromFile(mpWindowSystem.Get(), pFileSystem, mpResourceManager.Get()))
				{
					mCurrEditableEffectId = openFileResult.Get();

					if (auto pAnimationContainer = mpEditableEntity->GetComponent<CAnimationContainerComponent>())
					{
						if (auto pAnimation = mpResourceManager->GetResource<IResource>(mCurrEditableEffectId))
						{
							pAnimationContainer->SetAnimationClipId(pAnimation->GetName());
						}

						mpAnimationEditor->SetAnimationResourceHandle(mCurrEditableEffectId);
					}
				}
			});

			imguiContext.MenuItem("Save", "CTRL+S", [this, pFileSystem]
			{
				SaveToFile(pFileSystem, mpResourceManager.Get(), mCurrEditableEffectId, mLastSavedPath);
			});

			imguiContext.MenuItem("Save As...", "SHIFT+CTRL+S", [this, pFileSystem]
			{
				if (auto saveFileDialogResult = mpWindowSystem->ShowSaveFileDialog(FileExtensionsFilter))
				{
					mLastSavedPath = saveFileDialogResult.Get();
					SaveToFile(pFileSystem, mpResourceManager.Get(), mCurrEditableEffectId, mLastSavedPath);
				}
			});

			imguiContext.MenuItem("Quit", "Ctrl+Q", [this] { mpEngineCoreInstance->Quit(); });
		});
	});
}