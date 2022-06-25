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

		mIsInitialized = true;
		mIsVisible = true;

		return RC_OK;
	}

	void CEditorWindow::SetResourcesManifest(TPtr<CResourcesBuildManifest> pResourcesManifest)
	{
		mpResourcesManifest = pResourcesManifest;
	}


	static void DrawResourcesBrowserPanel(TPtr<CResourcesBuildManifest>& pResourcesManifest, IWindowSystem* pWindowSystem, IImGUIContext* pImGUIContext, bool isEnabled)
	{
		const IImGUIContext::TWindowParams params
		{
			ZeroVector2,
			TVector2(150.0f, 400.0f),
			TVector2(pWindowSystem->GetWidth() * 0.5f, static_cast<F32>(pWindowSystem->GetHeight())),
		};

		const std::string& baseResourcesPath = pResourcesManifest ? pResourcesManifest->GetBaseResourcesPath() : Wrench::StringUtils::GetEmptyStr();

		if (pImGUIContext->BeginWindow("Resources Hierarchy", isEnabled, params))
		{
			if (baseResourcesPath.empty())
			{
				pImGUIContext->EndWindow();
				return;
			}

			std::function<void(const fs::path&)> displayPathElement = [&displayPathElement, &baseResourcesPath, pImGUIContext, &pResourcesManifest](const fs::path& currPath)
			{
				std::string currItemId = currPath.filename().string();

				if (fs::is_directory(currPath))
				{
					if (std::get<0>(pImGUIContext->BeginTreeNode(currItemId)))
					{
						for (auto& currDirectory : fs::directory_iterator(currPath))
						{
							displayPathElement(currDirectory);
						}

						pImGUIContext->EndTreeNode();
					}

					return;
				}

				pImGUIContext->SelectableItem(currItemId);
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

		DrawResourcesBrowserPanel(mpResourcesManifest, mpWindowSystem, mpImGUIContext, mIsVisible);

		mIsVisible = isEnabled;
	}


	TDE2_API IEditorWindow* CreateEditorWindow(IResourceManager* pResourceManager, IInputContext* pInputContext, IWindowSystem* pWindowSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IEditorWindow, CEditorWindow, result, pResourceManager, pInputContext, pWindowSystem);
	}
}