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

	if (auto pEntity = pWorld->CreateEntity())
	{
		if (auto pTransform = pEntity->GetComponent<CTransform>())
		{
			auto props = pTransform->GetAllProperties();

			auto pos = pTransform->GetProperty("position");
			pos->Set(ForwardVector3);
		}
	}

#if 1
	TAnimationClipParameters clip;
	clip.mDuration = 2.5f;

	if (IAnimationClip* pClip = mpResourceManager->GetResource<IAnimationClip>(mpResourceManager->Create<CAnimationClip>("Animation2", clip)))
	{
		if (auto pTrack = pClip->GetTrack<IAnimationTrack>(pClip->CreateTrack<CVector3AnimationTrack>("testTrack")))
		{
			pTrack->CreateKey(0.0f);
			pTrack->CreateKey(1.0f);
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
