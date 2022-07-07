#include "../include/CEditorWindow.h"
#include <tuple>
#include <vector>
#include <string>
#define DEFER_IMPLEMENTATION
#include "deferOperation.hpp"
#include "../include/metadata.h"

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

		mpSelectionManager = std::make_unique<CResourceInfoSelectionManager>();
		TDE2_ASSERT(mpSelectionManager);

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


	struct TDrawResourcesBrowserParams
	{
		TPtr<CResourcesBuildManifest>  mpResourcesManifest;
		IWindowSystem*                 mpWindowSystem;
		IImGUIContext*                 mpImGUIContext;
		CResourceInfoSelectionManager* mpSelectionManager;
		IDesktopInputContext*          mpInputContext;
		bool                           mIsEnabled;
		TResourceId                    mIconsTextureAtlasHandle;
	};


	static const std::unordered_map<E_PAYLOAD_TYPE, TRectF32> ResourceTypeIcons
	{
		{ E_PAYLOAD_TYPE::DIRECTORY, TRectF32(0.66f, 0.0f, 1.0f, 1.0f) },
		{ E_PAYLOAD_TYPE::REGISTERED_RESOURCE, TRectF32(0.33f, 0.0f, 0.66f, 1.0f) },
		{ E_PAYLOAD_TYPE::UNKNOWN_RESOURCE, TRectF32(0.0f, 0.0f, 0.33f, 1.0f) },
	};

	static constexpr F32 IconsSizes = 15.0f;


	static void UnregisterResources(TPtr<CResourcesBuildManifest> pResourcesManifest, CResourceInfoSelectionManager* pSelectionManager)
	{
		for (auto&& currSelectedItemPath : pSelectionManager->GetSelectedEntities())
		{
			if (fs::is_directory(currSelectedItemPath))
			{
				continue; /// \note Don't process directories 
			}

			if (auto pResourceInfo = pResourcesManifest->FindResourceBuildInfo(currSelectedItemPath))
			{
				E_RESULT_CODE result = pResourcesManifest->RemoveResourceBuildInfo(pResourceInfo->mRelativePathToResource);
				TDE2_ASSERT(RC_OK == result);
			}
		}
	}


	static void DrawItemContextMenu(const std::string& id, IImGUIContext* pImGUIContext, TPtr<CResourcesBuildManifest> pResourcesManifest, 
									CResourceInfoSelectionManager* pSelectionManager, TResourceBuildInfo* pResourceInfo = nullptr)
	{
		pImGUIContext->DisplayContextMenu(Wrench::StringUtils::Format("##{0}", id), [&id, pResourceInfo, &pResourcesManifest, pSelectionManager](IImGUIContext& imgui)
		{
			if (pResourceInfo)
			{
				imgui.MenuItem("Unregister Resource", Wrench::StringUtils::GetEmptyStr(), [&pResourcesManifest, pSelectionManager]()
				{
					UnregisterResources(pResourcesManifest, pSelectionManager);
				});
			}
			else if (!fs::is_directory(id))
			{
				imgui.MenuItem("Register Resource", Wrench::StringUtils::GetEmptyStr(), [&id, &pResourcesManifest, pSelectionManager]
				{
					for (auto&& currSelectedItemPath : pSelectionManager->GetSelectedEntities())
					{
						if (fs::is_directory(currSelectedItemPath))
						{
							continue; /// \note Don't process directories 
						}

						auto pResourceInfo = CreateResourceBuildInfoForFilePath(currSelectedItemPath);
						if (!pResourceInfo)
						{
							TDE2_ASSERT(false);
							continue;
						}

						pResourceInfo->mRelativePathToResource = Wrench::StringUtils::ReplaceAll(currSelectedItemPath, pResourcesManifest->GetBaseResourcesPath(), ".");

						E_RESULT_CODE result = pResourcesManifest->AddResourceBuildInfo(std::move(pResourceInfo));
						TDE2_ASSERT(RC_OK == result);
					}
				});
			}

			imgui.MenuItem("Delete", Wrench::StringUtils::GetEmptyStr(), [&id, &pResourcesManifest, pSelectionManager]
			{
				for (auto&& currSelectedItemPath : pSelectionManager->GetSelectedEntities())
				{
					if (fs::is_directory(currSelectedItemPath))
					{
						fs::remove_all(currSelectedItemPath);
						continue;
					}

					fs::remove(currSelectedItemPath);
				}

				UnregisterResources(pResourcesManifest, pSelectionManager);
				pSelectionManager->ResetSelection();
			});

			if (pSelectionManager->GetSelectedItemsCount() < 2)
			{
				imgui.MenuItem("Rename", Wrench::StringUtils::GetEmptyStr(), [&id]
				{
					/// TODO
				});
			}
		});
	}


	static void ProcessDragDropSource(const std::string& id, const std::string& fullPathStr, IImGUIContext* pImGUIContext, CResourceInfoSelectionManager* pSelectionManager)
	{
		pImGUIContext->RegisterDragAndDropSource([pImGUIContext, id, fullPathStr, pSelectionManager]
		{
			pImGUIContext->SetDragAndDropData(DRAG_DROP_ITEM_PATH, fullPathStr);

			if (pSelectionManager->GetSelectedItemsCount() < 1)
			{
				pImGUIContext->Label(id);
				return;
			}

			for (auto&& currSelectedItemPath : pSelectionManager->GetSelectedEntities())
			{
				pImGUIContext->Label(fs::path(currSelectedItemPath).filename().string());
			}
		});
	}


	static void ProcessDragDropTarget(const std::string& targetDirectoryPath, IImGUIContext* pImGUIContext, CResourceInfoSelectionManager* pSelectionManager)
	{
		pImGUIContext->RegisterDragAndDropTarget([=]
		{
			const C8* pResourcePath = pImGUIContext->GetDragAndDropData<C8>(DRAG_DROP_ITEM_PATH);

			if (!pResourcePath)
			{
				return;
			}

			if (pSelectionManager->GetSelectedItemsCount() < 1)
			{
				/// \note Copy the only item into the directory
				return;
			}

			/// \note Copy all selected items into the directory
			for (auto&& currSelectedItemPath : pSelectionManager->GetSelectedEntities())
			{
			}
		});
	}


	static void DrawResourcesBrowserPanel(const TDrawResourcesBrowserParams& panelParams)
	{
		auto pImGUIContext        = panelParams.mpImGUIContext;
		auto pWindowSystem        = panelParams.mpWindowSystem;
		auto&& pResourcesManifest = panelParams.mpResourcesManifest;
		auto pSelectionManager    = panelParams.mpSelectionManager;
		auto pInputContext        = panelParams.mpInputContext;

		const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			TVector2(150.0f, 400.0f),
			TVector2(pWindowSystem->GetWidth() * 0.5f, static_cast<F32>(pWindowSystem->GetHeight())),
		};

		const std::string& baseResourcesPath = pResourcesManifest ? pResourcesManifest->GetBaseResourcesPath() : Wrench::StringUtils::GetEmptyStr();

		bool isEnabled = panelParams.mIsEnabled;

		defer([pImGUIContext] { pImGUIContext->EndWindow(); });

		if (pImGUIContext->BeginWindow("Resources Hierarchy", isEnabled, params))
		{
			if (baseResourcesPath.empty())
			{
				return;
			}

			std::function<void(const fs::path&)> displayPathElement = [=, &displayPathElement, &baseResourcesPath, &pResourcesManifest, iconsHandle = panelParams.mIconsTextureAtlasHandle](const fs::path& currPath)
			{
				std::string currItemId = currPath.filename().string();

				if (fs::is_directory(currPath))
				{
					bool isDirectoryUnwrapped, isDirectorySelected;

					pImGUIContext->BeginHorizontal();
					pImGUIContext->Image(iconsHandle, TVector2(IconsSizes), ResourceTypeIcons.at(E_PAYLOAD_TYPE::DIRECTORY));
					std::tie(isDirectoryUnwrapped, isDirectorySelected) = pImGUIContext->BeginTreeNode(currItemId, pSelectionManager->IsSelected(currPath.string()));
					pImGUIContext->EndHorizontal();

					DrawItemContextMenu(currPath.string(), pImGUIContext, pResourcesManifest, pSelectionManager);
					ProcessDragDropSource(currItemId, currPath.string(), pImGUIContext, pSelectionManager);

					if (isDirectorySelected)
					{
						if (pInputContext->IsKey(E_KEYCODES::KC_LCONTROL))
						{
							pSelectionManager->AddSelection(currPath.string());
						}
						else
						{
							pSelectionManager->SetSelection(currPath.string());
						}
					}

					if (isDirectoryUnwrapped)
					{						
						for (auto& currDirectory : fs::directory_iterator(currPath))
						{
							displayPathElement(currDirectory);
						}

						pImGUIContext->EndTreeNode();
					}

					ProcessDragDropTarget(currPath.string(), pImGUIContext, pSelectionManager);

					return;
				}

				TResourceBuildInfo* pResourceInfo = pResourcesManifest->FindResourceBuildInfo(currPath.string());
				const E_PAYLOAD_TYPE resourceType = pResourceInfo ? E_PAYLOAD_TYPE::REGISTERED_RESOURCE : E_PAYLOAD_TYPE::UNKNOWN_RESOURCE;

				pImGUIContext->BeginHorizontal();
				pImGUIContext->Image(iconsHandle, TVector2(IconsSizes), ResourceTypeIcons.at(resourceType));
				
				if (pImGUIContext->SelectableItem(currItemId, 
												E_PAYLOAD_TYPE::REGISTERED_RESOURCE == resourceType ? TColorUtils::mGreen : TColorUtils::mWhite, 
												pSelectionManager->IsSelected(currPath.string())))
				{
					if (pInputContext->IsKey(E_KEYCODES::KC_LCONTROL))
					{
						pSelectionManager->AddSelection(currPath.string());
					}
					else
					{
						pSelectionManager->SetSelection(currPath.string());
					}
				}
				
				pImGUIContext->EndHorizontal();

				DrawItemContextMenu(currPath.string(), pImGUIContext, pResourcesManifest, pSelectionManager, pResourceInfo);
				ProcessDragDropSource(currItemId, currPath.string(), pImGUIContext, pSelectionManager);
			};

			for (auto& currDirectory : fs::directory_iterator(baseResourcesPath))
			{
				displayPathElement(currDirectory);				
			}
		}
	}


	static void DrawMeshResourceInfoInspector(IImGUIContext& imgui, TResourceBuildInfo* pResourceInfo)
	{
		TMeshResourceBuildInfo* pMeshInfo = dynamic_cast<TMeshResourceBuildInfo*>(pResourceInfo);
		if (!pMeshInfo)
		{
			return;
		}

		imgui.Label("Mesh Properties: ");

		/// \note Tangents's import information
		imgui.BeginHorizontal();
		imgui.Label("Import Tangents/Normals");
		imgui.Checkbox("##import_tangents", pMeshInfo->mImportTangents);
		imgui.EndHorizontal();

		imgui.BeginHorizontal();
		imgui.Label("Skinned Mesh");
		imgui.Checkbox("##skinned_mesh", pMeshInfo->mIsSkinned);
		imgui.EndHorizontal();

		if (pMeshInfo->mIsSkinned)
		{
			if (imgui.CollapsingHeader("Animation Clips", true))
			{
				if (imgui.Button("Add New Item", TVector2(imgui.GetWindowWidth() * 0.3f, 25.0f)))
				{
					pMeshInfo->mAnimations.push_back({});
				}

				bool shouldInterruptLoop = false;
				U32 index = 0;

				for (auto&& currAnimationClip : pMeshInfo->mAnimations)
				{
					imgui.DisplayIDGroup(index, [&imgui, &currAnimationClip, &shouldInterruptLoop, index, pMeshInfo]
					{
						if (std::get<0>(imgui.BeginTreeNode("Clip Info", false)))
						{
							imgui.BeginHorizontal();
							imgui.Label("Output clip path:");
							imgui.TextField("##outputPath", currAnimationClip.mOutputAnimationPath);
							imgui.EndHorizontal();

							I32 startRange = static_cast<I32>(currAnimationClip.mStartRange);

							imgui.BeginHorizontal();
							imgui.Label("Start Keyframe: ");
							imgui.IntField("##startRange", startRange, [&currAnimationClip, &startRange] { currAnimationClip.mStartRange = static_cast<U32>(startRange); });
							imgui.EndHorizontal();

							I32 endRange = static_cast<I32>(currAnimationClip.mEndRange);

							imgui.BeginHorizontal();
							imgui.Label("End Keyframe: ");
							imgui.IntField("##endRange", endRange, [&currAnimationClip, &endRange] { currAnimationClip.mEndRange = static_cast<U32>(endRange); });
							imgui.EndHorizontal();

							imgui.BeginHorizontal();
							imgui.Label("Looped: ");
							imgui.Checkbox("##looped", currAnimationClip.mIsLooped);
							imgui.EndHorizontal();

							if (imgui.Button("Remove Item", TVector2(imgui.GetWindowWidth() * 0.3f, 25.0f)))
							{
								pMeshInfo->mAnimations.erase(pMeshInfo->mAnimations.begin() + index);
								shouldInterruptLoop = true;
							}

							imgui.EndTreeNode();
						}
					});

					++index;

					if (shouldInterruptLoop)
					{
						break;
					}
				}
			}
		}
	}


	static void DrawTexture2DResourceInfoInspector(IImGUIContext& imgui, TResourceBuildInfo* pResourceInfo)
	{
		TTexture2DResourceBuildInfo* pTex2DInfo = dynamic_cast<TTexture2DResourceBuildInfo*>(pResourceInfo);
		if (!pTex2DInfo)
		{
			return;
		}

		imgui.Label("Texture Properties");

		static std::vector<std::string> filterTypes;
		
		if (filterTypes.empty())
		{
			for (auto&& enumFieldInfo : Meta::EnumTrait<E_TEXTURE_FILTER_TYPE>::GetFields())
			{
				filterTypes.push_back(enumFieldInfo.name);
			}
		}

		static std::vector<std::string> addressModes;
		
		if (addressModes.empty())
		{
			for (auto&& enumFieldInfo : Meta::EnumTrait<E_ADDRESS_MODE_TYPE>::GetFields())
			{
				addressModes.push_back(enumFieldInfo.name);
			}
		}

		/// \note Filter type
		{
			I32 currFilterType = static_cast<I32>(pTex2DInfo->mFilteringType);

			imgui.BeginHorizontal();
			imgui.Label("Filter Type: ");
			I32 newFilterType = imgui.Popup("##filterType", currFilterType, filterTypes);

			if (currFilterType != newFilterType)
			{
				pTex2DInfo->mFilteringType = static_cast<E_TEXTURE_FILTER_TYPE>(newFilterType);
			}

			imgui.EndHorizontal();
		}

		/// \note Address mode 
		{
			I32 currAddressModeType = static_cast<I32>(pTex2DInfo->mAddressMode);

			imgui.BeginHorizontal();
			imgui.Label("Address Mode: ");
			I32 newAddressModeType = imgui.Popup("##addressMode", currAddressModeType, addressModes);

			if (currAddressModeType != newAddressModeType)
			{
				pTex2DInfo->mAddressMode = static_cast<E_ADDRESS_MODE_TYPE>(newAddressModeType);
			}

			imgui.EndHorizontal();
		}

		imgui.BeginHorizontal();
		imgui.Label("Use Mip-Mapping");
		imgui.Checkbox("##mip-mapping", pTex2DInfo->mGenerateMipMaps);
		imgui.EndHorizontal();
	}


	struct TDrawResourceInspectorParams
	{
		TResourceBuildInfo* mpCurrEditingResource;
		IWindowSystem*      mpWindowSystem;
		IImGUIContext*      mpImGUIContext;
		bool                mIsEnabled;
	};


	static void DrawResourceInspectorPanel(const TDrawResourceInspectorParams& panelParams)
	{
		auto pImGUIContext = panelParams.mpImGUIContext;
		auto pWindowSystem = panelParams.mpWindowSystem;

		const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			TVector2(400.0f, 400.0f),
			TVector2(pWindowSystem->GetWidth() * 0.6f, static_cast<F32>(pWindowSystem->GetHeight())),
		};

		bool isEnabled = panelParams.mIsEnabled;

		static const std::unordered_map<TypeId, std::function<void(IImGUIContext&, TResourceBuildInfo*)>> inspectorDrawersTable
		{
			{ TDE2_TYPE_ID(TMeshResourceBuildInfo), DrawMeshResourceInfoInspector },
			{ TDE2_TYPE_ID(TTexture2DResourceBuildInfo), DrawTexture2DResourceInfoInspector },
		};

		defer([pImGUIContext] { pImGUIContext->EndWindow(); });

		if (pImGUIContext->BeginWindow("Resource Inspector", isEnabled, params))
		{
			auto pResourceInfo = panelParams.mpCurrEditingResource;
			if (!pResourceInfo)
			{
				return;
			}

			auto drawerIt = inspectorDrawersTable.find(pResourceInfo->GetResourceTypeId());
			if (drawerIt == inspectorDrawersTable.cend())
			{
				return;
			}

			(drawerIt->second)(*pImGUIContext, pResourceInfo);
		}
	};


	void CEditorWindow::_onDraw()
	{
		bool isEnabled = mIsVisible;

		DrawResourcesBrowserPanel({ mpResourcesManifest, mpWindowSystem, mpImGUIContext, mpSelectionManager.get(), mpInputContext, mIsVisible, mIconsTextureHandle });
		
		auto&& selectedElements = mpSelectionManager->GetSelectedEntities();

		DrawResourceInspectorPanel(
			{ 
				selectedElements.size() == 1 ? mpResourcesManifest->FindResourceBuildInfo(selectedElements.front()) : nullptr, 
				mpWindowSystem, 
				mpImGUIContext, 
				mIsVisible
			});

		mIsVisible = isEnabled;
	}


	TDE2_API IEditorWindow* CreateEditorWindow(IResourceManager* pResourceManager, IInputContext* pInputContext, IWindowSystem* pWindowSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CEditorWindow, result, pResourceManager, pInputContext, pWindowSystem);
	}
}