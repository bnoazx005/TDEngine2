#include "../include/CUtilityListener.h"
#include "../include/CMaterialEditorWindow.h"
#include <memory>
#include <vector>
#include <string>
#include <tuple>


using namespace TDEngine2;


const std::vector<std::tuple<std::string, std::string>> FileExtensionsFilter
{
	{ "Materials", "*.material" }
};


TDEngine2::E_RESULT_CODE CUtilityListener::OnStart()
{
	TDEngine2::E_RESULT_CODE result = RC_OK;

	auto pWorld = mpEngineCoreInstance->GetWorldInstance();

	// \note Create an editor's camera

	if (CEntity* pCameraEntity = pWorld->CreateEntity("Camera"))
	{
		if (auto pCamera = pCameraEntity->AddComponent<CPerspectiveCamera>())
		{
			pCamera->SetAspect(mpWindowSystem->GetWidth() / static_cast<F32>(mpWindowSystem->GetHeight()));
			pCamera->SetFOV(0.5f * CMathConstants::Pi);
		}
	}

	mpMaterialEditor = TPtr<CMaterialEditorWindow>(dynamic_cast<CMaterialEditorWindow*>(TDEngine2::CreateMaterialEditorWindow(mpResourceManager.Get(), mpEngineCoreInstance->GetSubsystem<IInputContext>().Get(), result)));

	mLastSavedPath = Wrench::StringUtils::GetEmptyStr();
	mCurrEditablMaterialId = TResourceId::Invalid;

	mpMaterialEditor->SetMaterialResourceHandle(mCurrEditablMaterialId);

	ISceneManager* pSceneManager = mpEngineCoreInstance->GetSubsystem<ISceneManager>().Get();

	if (auto getSceneResult = pSceneManager->GetScene(MainScene))
	{
		auto pScene = getSceneResult.Get();

		pScene->CreateSkybox(mpResourceManager.Get(), "Resources/Textures/DefaultSkybox");

		if (auto pMaterialsEntity = pScene->CreateEntity(EditableEntityId))
		{
			if (CTransform* pTransform = pMaterialsEntity->GetComponent<CTransform>())
			{
				pTransform->SetPosition(ForwardVector3 * 2.0f);
			}

#if 0
			if (auto pMaterials = pMaterialsEntity->AddComponent<CMaterialEmitter>())
			{
				pMaterials->SetMaterialEffect("testMaterials.Materials");
			}
#endif

			mpEditableEntity = pMaterialsEntity;
		}
	}

	return RC_OK;
}

TDEngine2::E_RESULT_CODE CUtilityListener::OnUpdate(const float& dt)
{
	mpMaterialEditor->Draw(mpEngineCoreInstance->GetSubsystem<IImGUIContext>().Get(), dt);

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

	return Wrench::TOkValue<TResourceId>(pResourceManager->Load<IMaterial>(openFileResult.Get()));
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
			if (auto pMaterialEffect = pResourceManager->GetResource<IMaterial>(resourceId))
			{
				if (RC_OK != (result = pMaterialEffect->Save(pFileWriter)))
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
			imguiContext.MenuItem("New Effect", "CTRL+N", [this]
			{
				mLastSavedPath = Wrench::StringUtils::GetEmptyStr();
				// \todo Add reset of the app's state here
			});

			imguiContext.MenuItem("Open Effect", "CTRL+O", [this, pFileSystem]
			{
				if (auto openFileResult = OpenFromFile(mpWindowSystem.Get(), pFileSystem, mpResourceManager.Get()))
				{
					mCurrEditablMaterialId = openFileResult.Get();

#if 0
					if (auto pMaterials = mpEditableEntity->GetComponent<CMa>())
					{
						if (auto pMaterialEffect = mpResourceManager->GetResource<IResource>(mCurrEditablMaterialId))
						{
							pMaterials->SetMaterialEffect(pMaterialEffect->GetName());
						}

						mpMaterialEditor->SetMaterialResourceHandle(mCurrEditablMaterialId);
					}
#endif
				}
			});

			imguiContext.MenuItem("Save Effect", "CTRL+S", [this, pFileSystem]
			{
				SaveToFile(pFileSystem, mpResourceManager.Get(), mCurrEditablMaterialId, mLastSavedPath);
			});

			imguiContext.MenuItem("Save Effect As...", "SHIFT+CTRL+S", [this, pFileSystem]
			{
				if (auto saveFileDialogResult = mpWindowSystem->ShowSaveFileDialog(FileExtensionsFilter))
				{
					mLastSavedPath = saveFileDialogResult.Get();
					SaveToFile(pFileSystem, mpResourceManager.Get(), mCurrEditablMaterialId, mLastSavedPath);
				}
			});

			imguiContext.MenuItem("Quit", "Ctrl+Q", [this] { mpEngineCoreInstance->Quit(); });
		});
	});
}
