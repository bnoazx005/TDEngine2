#include "../include/CEditorWindow.h"
#include <tuple>
#include <vector>
#include <string>

#if _HAS_CXX17
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif


namespace TDEngine2
{
	CEditorWindow::CEditorWindow() :
		CBaseEditorWindow()
	{
	}

	E_RESULT_CODE CEditorWindow::Init(IResourceManager* pResourceManager, IInputContext* pInputContext, IWindowSystem* pWindowSystem)
	{
		if (mIsInitialized)
		{
			return RC_OK;
		}

		if (!pResourceManager || !pWindowSystem || !pInputContext)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;
		mpInputContext = dynamic_cast<IDesktopInputContext*>(pInputContext);
		mpWindowSystem = pWindowSystem;

		mIconsTextureHandle = pResourceManager->Load<ITexture2D>("Resources/Editor/icons.png");

		mIsInitialized = true;
		mIsVisible = true;

		return RC_OK;
	}

	void CEditorWindow::SetResourcesManifest(TPtr<CResourcesBuildManifest> pResourcesManifest)
	{
		mpResourcesManifest = pResourcesManifest;
	}


	enum class E_PAYLOAD_TYPE : U8
	{
		UNKNOWN_RESOURCE,
		REGISTERED_RESOURCE,
		DIRECTORY
	};


	static const std::string DRAG_DROP_ITEM_PATH = "drag_drop_resource_path";
	static const std::string DRAG_DROP_ITEM_TYPE = "drag_drop_payload_type";


	struct TDrawResourcesBrowserParams
	{
		TPtr<CResourcesBuildManifest> mpResourcesManifest;
		IWindowSystem*                mpWindowSystem;
		IImGUIContext*                mpImGUIContext;
		bool                          mIsEnabled;
		TResourceId                   mIconsTextureAtlasHandle;
	};


	static const std::unordered_map<E_PAYLOAD_TYPE, TRectF32> ResourceTypeIcons
	{
		{ E_PAYLOAD_TYPE::DIRECTORY, TRectF32(0.66f, 0.0f, 1.0f, 1.0f) },
		{ E_PAYLOAD_TYPE::REGISTERED_RESOURCE, TRectF32(0.33f, 0.0f, 0.66f, 1.0f) },
		{ E_PAYLOAD_TYPE::UNKNOWN_RESOURCE, TRectF32(0.0f, 0.0f, 0.33f, 1.0f) },
	};

	static constexpr F32 IconsSizes = 15.0f;


	static void DrawResourceContextMenu(const std::string& id, IImGUIContext* pImGUIContext, TPtr<CResourcesBuildManifest> pResourcesManifest, TResourceBuildInfo* pResourceInfo = nullptr)
	{
		pImGUIContext->DisplayContextMenu(Wrench::StringUtils::Format("##{0}", id), [&id, pResourceInfo, &pResourcesManifest](IImGUIContext& imgui)
		{
			if (pResourceInfo)
			{
				imgui.MenuItem("Unregister Resource", Wrench::StringUtils::GetEmptyStr(), [&imgui, &pResourcesManifest, &id]()
				{
					if (auto pResourceInfo = pResourcesManifest->FindResourceBuildInfo(id))
					{
						E_RESULT_CODE result = pResourcesManifest->RemoveResourceBuildInfo(pResourceInfo->mRelativePathToResource);
						TDE2_ASSERT(RC_OK == result);
					}
				});
			}
			else
			{
				imgui.MenuItem("Register Resource", Wrench::StringUtils::GetEmptyStr(), [&id, &pResourcesManifest]
				{
					auto pResourceInfo = CreateResourceBuildInfoForFilePath(id);
					if (!pResourceInfo)
					{
						return;
					}

					pResourceInfo->mRelativePathToResource = Wrench::StringUtils::ReplaceAll(id, pResourcesManifest->GetBaseResourcesPath(), ".");

					E_RESULT_CODE result = pResourcesManifest->AddResourceBuildInfo(std::move(pResourceInfo));
					TDE2_ASSERT(RC_OK == result);
				});
			}

			imgui.MenuItem("Delete", Wrench::StringUtils::GetEmptyStr(), [&id]
			{
				/// TODO
			});

			imgui.MenuItem("Rename", Wrench::StringUtils::GetEmptyStr(), [&id]
			{
				/// TODO
			});
		});
	}


	static void DrawDirectoryContextMenu(const std::string& id, IImGUIContext* pImGUIContext, TPtr<CResourcesBuildManifest> pResourcesManifest, TResourceBuildInfo* pResourceInfo = nullptr)
	{

	}


	static void DrawResourcesBrowserPanel(const TDrawResourcesBrowserParams& panelParams)
	{
		auto pImGUIContext = panelParams.mpImGUIContext;
		auto pWindowSystem = panelParams.mpWindowSystem;
		auto&& pResourcesManifest = panelParams.mpResourcesManifest;

		const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			TVector2(150.0f, 400.0f),
			TVector2(pWindowSystem->GetWidth() * 0.5f, static_cast<F32>(pWindowSystem->GetHeight())),
		};

		const std::string& baseResourcesPath = pResourcesManifest ? pResourcesManifest->GetBaseResourcesPath() : Wrench::StringUtils::GetEmptyStr();

		bool isEnabled = panelParams.mIsEnabled;

		if (pImGUIContext->BeginWindow("Resources Hierarchy", isEnabled, params))
		{
			if (baseResourcesPath.empty())
			{
				pImGUIContext->EndWindow();
				return;
			}

			std::function<void(const fs::path&)> displayPathElement = [&displayPathElement, &baseResourcesPath, pImGUIContext, &pResourcesManifest, iconsHandle = panelParams.mIconsTextureAtlasHandle](const fs::path& currPath)
			{
				std::string currItemId = currPath.filename().string();

				if (fs::is_directory(currPath))
				{
					pImGUIContext->BeginHorizontal();
					pImGUIContext->Image(iconsHandle, TVector2(IconsSizes), ResourceTypeIcons.at(E_PAYLOAD_TYPE::DIRECTORY));
					const bool isDirectoryUnwrapped = std::get<0>(pImGUIContext->BeginTreeNode(currItemId));
					pImGUIContext->EndHorizontal();

					if (isDirectoryUnwrapped)
					{						
						for (auto& currDirectory : fs::directory_iterator(currPath))
						{
							displayPathElement(currDirectory);
						}

						pImGUIContext->EndTreeNode();
					}

					pImGUIContext->RegisterDragAndDropTarget([pImGUIContext]
					{
						const C8* pResourcePath = pImGUIContext->GetDragAndDropData<C8>(DRAG_DROP_ITEM_PATH);
						const E_PAYLOAD_TYPE* pPayloadType = pImGUIContext->GetDragAndDropData<E_PAYLOAD_TYPE>(DRAG_DROP_ITEM_TYPE);
						
						if (!pResourcePath || !pPayloadType)
						{
							return;
						}

						switch (*pPayloadType)
						{
							case E_PAYLOAD_TYPE::REGISTERED_RESOURCE:
							case E_PAYLOAD_TYPE::UNKNOWN_RESOURCE:
								break;
							case E_PAYLOAD_TYPE::DIRECTORY:
								break;
						}
					});

					return;
				}

				TResourceBuildInfo* pResourceInfo = pResourcesManifest->FindResourceBuildInfo(currPath.string());
				const E_PAYLOAD_TYPE resourceType = pResourceInfo ? E_PAYLOAD_TYPE::REGISTERED_RESOURCE : E_PAYLOAD_TYPE::UNKNOWN_RESOURCE;

				pImGUIContext->BeginHorizontal();
				pImGUIContext->Image(iconsHandle, TVector2(IconsSizes), ResourceTypeIcons.at(resourceType));
				pImGUIContext->SelectableItem(currItemId, E_PAYLOAD_TYPE::REGISTERED_RESOURCE == resourceType ? TColorUtils::mGreen : TColorUtils::mWhite);
				pImGUIContext->EndHorizontal();

				DrawResourceContextMenu(currPath.string(), pImGUIContext, pResourcesManifest, pResourceInfo);

				pImGUIContext->RegisterDragAndDropSource([pImGUIContext, currItemId, fullPathStr = currPath.string()]
				{
					pImGUIContext->SetDragAndDropData(DRAG_DROP_ITEM_PATH, fullPathStr);
					pImGUIContext->SetDragAndDropData(DRAG_DROP_ITEM_TYPE, E_PAYLOAD_TYPE::UNKNOWN_RESOURCE);

					pImGUIContext->Label(currItemId);
				});
			};

			for (auto& currDirectory : fs::directory_iterator(baseResourcesPath))
			{
				displayPathElement(currDirectory);				
			}
		}

		pImGUIContext->EndWindow();
	}


	void CEditorWindow::_onDraw()
	{
		bool isEnabled = mIsVisible;

		DrawResourcesBrowserPanel({ mpResourcesManifest, mpWindowSystem, mpImGUIContext, mIsVisible, mIconsTextureHandle });

		mIsVisible = isEnabled;
	}


	TDE2_API IEditorWindow* CreateEditorWindow(IResourceManager* pResourceManager, IInputContext* pInputContext, IWindowSystem* pWindowSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CEditorWindow, result, pResourceManager, pInputContext, pWindowSystem);
	}
}