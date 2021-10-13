#include "../include/CUtilityListener.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#pragma warning(push)
#pragma warning(disable:4996)
#include "./../deps/stb/stb_image_write.h"
#pragma warning(pop)


using namespace TDEngine2;


TDEngine2::E_RESULT_CODE CUtilityListener::OnStart()
{
	TDEngine2::E_RESULT_CODE result = RC_OK;

	mpPreviewEditorWindow = dynamic_cast<CFontPreviewWindow*>(CreateFontPreviewWindow(mpResourceManager.Get(), mpInputContext.Get(), mpWindowSystem.Get(), result));
	mpConfigEditorWindow = CreateConfigWindow({ mpResourceManager.Get(), mpInputContext.Get(), mpWindowSystem.Get(), mpFileSystem.Get() }, result);

	/// \note For this tool this entity isn't used but create it to suppress assertions
	if (auto pWorld = mpEngineCoreInstance->GetWorldInstance())
	{
		if (CEntity* pCameraEntity = pWorld->CreateEntity("Camera"))
		{
			pCameraEntity->AddComponent<TDEngine2::COrthoCamera>();
		}
	}

	if (result != RC_OK)
	{
		return result;
	}

	return RC_OK;
}

TDEngine2::E_RESULT_CODE CUtilityListener::OnUpdate(const float& dt)
{
	mpPreviewEditorWindow->Draw(mpEngineCoreInstance->GetSubsystem<IImGUIContext>().Get(), dt);
	mpConfigEditorWindow->Draw(mpEngineCoreInstance->GetSubsystem<IImGUIContext>().Get(), dt);

	/// \todo Replace with event driven approach
	if (CConfigWindow* pConfigWindow = dynamic_cast<CConfigWindow*>(mpConfigEditorWindow.Get()))
	{
		mpPreviewEditorWindow->SetTextureAtlasResourceHandle(pConfigWindow->GetFontAtlasHandle());
	}

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

	mpGraphicsContext = mpEngineCoreInstance->GetSubsystem<IGraphicsContext>();
	mpWindowSystem    = mpEngineCoreInstance->GetSubsystem<IWindowSystem>();
	mpResourceManager = mpEngineCoreInstance->GetSubsystem<IResourceManager>();
	mpFileSystem      = mpEngineCoreInstance->GetSubsystem<IFileSystem>();
	mpInputContext    = mpEngineCoreInstance->GetSubsystem<IInputContext>();
}
