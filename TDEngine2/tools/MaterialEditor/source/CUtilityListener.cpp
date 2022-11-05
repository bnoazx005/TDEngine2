#include "../include/CUtilityListener.h"
#include "../include/CMaterialEditorWindow.h"
#include <memory>
#include <vector>
#include <string>
#include <tuple>


#if TDE2_EDITORS_ENABLED

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
	mCurrEditableMaterialId = TResourceId::Invalid;

	mpMaterialEditor->SetMaterialResourceHandle(mCurrEditableMaterialId);

	ISceneManager* pSceneManager = mpEngineCoreInstance->GetSubsystem<ISceneManager>().Get();

	if (auto getSceneResult = pSceneManager->GetScene(MainScene))
	{
		auto pScene = getSceneResult.Get();

		pScene->CreateSkybox(mpResourceManager.Get(), "DefaultResources/Textures/DefaultSkybox");
		
		if (auto pLightEntity = pScene->CreateDirectionalLight(TColorUtils::mWhite, 1.0f, TVector3(1.0f, 1.0f, 0.0f)))
		{
			auto pTransform = pLightEntity->GetComponent<CTransform>();
			pTransform->SetPosition(TVector3(0.0f, 10.0f, 0.0f));
		}

		if (auto pModelEntity = pScene->CreateEntity(EditableEntityId))
		{
			if (CTransform* pTransform = pModelEntity->GetComponent<CTransform>())
			{
				pTransform->SetPosition(ForwardVector3 * 2.0f);
			}

			mEditableEntity = pModelEntity->GetId();
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


static void TryLoadModel(TPtr<IWindowSystem> pWindowSystem, TPtr<IWorld> pWorld, TEntityId currModelEntity, TPtr<IResource> pMaterial, bool loadSkinnedModel = false)
{
	auto openFileResult = pWindowSystem->ShowOpenFileDialog({ { "Meshes", "*.mesh" } });
	if (openFileResult.HasError())
	{
		return;
	}

	CEntity* pEntity = pWorld->FindEntity(currModelEntity);
	if (!pEntity)
	{
		return;
	}

	E_RESULT_CODE result = pEntity->RemoveComponents();
	TDE2_ASSERT(RC_OK == result && pEntity->HasComponent<CTransform>());

	pEntity->AddComponent<CShadowReceiverComponent>();
	pEntity->AddComponent<CShadowCasterComponent>();

	if (loadSkinnedModel)
	{
		auto pMeshContainer = pEntity->AddComponent<CSkinnedMeshContainer>();

		std::string pathToMesh = openFileResult.Get();

		pMeshContainer->SetMeshName(pathToMesh);
		pMeshContainer->SetMaterialName(pMaterial ? pMaterial->GetName() : Wrench::StringUtils::GetEmptyStr());

		if (Wrench::StringUtils::EndsWith(pathToMesh, ".mesh"))
		{
			pathToMesh = Wrench::StringUtils::ReplaceAll(pathToMesh, ".mesh", ".skeleton"); // \todo Replace with constants
		}
		else
		{
			pathToMesh.append(".skeleton");
		}

		pMeshContainer->SetSkeletonName(pathToMesh);

		pEntity->AddComponent<CMeshAnimatorComponent>();
		pEntity->AddComponent<CAnimationContainerComponent>();

		return;
	}

	auto pMeshContainer = pEntity->AddComponent<CStaticMeshContainer>();
	pMeshContainer->SetMeshName(openFileResult.Get());
	pMeshContainer->SetMaterialName(pMaterial ? pMaterial->GetName() : Wrench::StringUtils::GetEmptyStr());
}


void CUtilityListener::_drawMainMenu()
{
	auto pImGUIContext = mpEngineCoreInstance->GetSubsystem<IImGUIContext>();
	auto pFileSystem = mpEngineCoreInstance->GetSubsystem<IFileSystem>().Get();
	
	pImGUIContext->DisplayMainMenu([this, pFileSystem](IImGUIContext& imguiContext)
	{
		imguiContext.MenuGroup("File", [this, pFileSystem](IImGUIContext& imguiContext)
		{
			imguiContext.MenuItem("New Material", "CTRL+N", [this]
			{
				mLastSavedPath = Wrench::StringUtils::GetEmptyStr();

				mCurrEditableMaterialId = mpResourceManager->Create<IMaterial>("unnamed.material", TMaterialParameters{ "Default" });
				mpMaterialEditor->SetMaterialResourceHandle(mCurrEditableMaterialId);
			});

			imguiContext.MenuItem("Open Material", "CTRL+O", [this, pFileSystem]
			{
				if (auto openFileResult = OpenFromFile(mpWindowSystem.Get(), pFileSystem, mpResourceManager.Get()))
				{
					mCurrEditableMaterialId = openFileResult.Get();
					mpMaterialEditor->SetMaterialResourceHandle(mCurrEditableMaterialId);

					/// \todo Assign material to current active model
				}
			});

			imguiContext.MenuItem("Save Material", "CTRL+S", [this, pFileSystem]
			{
				SaveToFile(pFileSystem, mpResourceManager.Get(), mCurrEditableMaterialId, mLastSavedPath);
			});

			imguiContext.MenuItem("Save Material As...", "SHIFT+CTRL+S", [this, pFileSystem]
			{
				if (auto saveFileDialogResult = mpWindowSystem->ShowSaveFileDialog(FileExtensionsFilter))
				{
					mLastSavedPath = saveFileDialogResult.Get();
					SaveToFile(pFileSystem, mpResourceManager.Get(), mCurrEditableMaterialId, mLastSavedPath);
				}
			});

			imguiContext.MenuItem("Quit", "Ctrl+Q", [this] { mpEngineCoreInstance->Quit(); });
		});

		imguiContext.MenuGroup("View", [this](IImGUIContext& imguiContext)
		{
			imguiContext.MenuGroup("Load model for tests", [this](IImGUIContext& imgui)
			{
				imgui.MenuItem("Load Static Model", Wrench::StringUtils::GetEmptyStr(), [this]
				{
					TryLoadModel(mpWindowSystem, 
						mpEngineCoreInstance->GetSubsystem<ISceneManager>()->GetWorld(), 
						mEditableEntity, 
						mpResourceManager->GetResource(mCurrEditableMaterialId));
				});

				imgui.MenuItem("Load Skinned Model", Wrench::StringUtils::GetEmptyStr(), [this]
				{
					TryLoadModel(mpWindowSystem, 
						mpEngineCoreInstance->GetSubsystem<ISceneManager>()->GetWorld(), 
						mEditableEntity, 
						mpResourceManager->GetResource(mCurrEditableMaterialId),
						true);
				});
			});
		});
	});
}

#endif