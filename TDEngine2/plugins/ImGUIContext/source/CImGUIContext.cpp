#include "./../include/CImGUIContext.h"
#include <core/IWindowSystem.h>
#include <core/IGraphicsContext.h>
#include <core/IInputContext.h>
#include <graphics/IGraphicsObjectManager.h>


namespace TDEngine2
{
	CImGUIContext::CImGUIContext():
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CImGUIContext::Init(IWindowSystem* pWindowSystem, IGraphicsObjectManager* pGraphicsObjectManager,
									  IInputContext* pInputContext)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pWindowSystem ||
			!pGraphicsObjectManager ||
			!pInputContext)
		{
			return RC_INVALID_ARGS;	
		}

		mpWindowSystem          = pWindowSystem;
		mpGraphicsContext       = pGraphicsObjectManager->GetGraphicsContext();
		mpGraphicsObjectManager = pGraphicsObjectManager;
		mpInputContext          = pInputContext;

		if (!mpGraphicsContext) // \note the really strange case, but if it's happened we should check for it
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CImGUIContext::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	E_ENGINE_SUBSYSTEM_TYPE CImGUIContext::GetType() const
	{
		return EST_IMGUI_CONTEXT;
	}


	TDE2_API IImGUIContext* CreateImGUIContext(IWindowSystem* pWindowSystem, IGraphicsObjectManager* pGraphicsObjectManager,
											   IInputContext* pInputContext, E_RESULT_CODE& result)
	{
		CImGUIContext* pImGUIContextInstance = new (std::nothrow) CImGUIContext();

		if (!pImGUIContextInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return pImGUIContextInstance;
		}

		result = pImGUIContextInstance->Init(pWindowSystem, pGraphicsObjectManager, pInputContext);

		if (result != RC_OK)
		{
			delete pImGUIContextInstance;

			pImGUIContextInstance = nullptr;
		}

		return dynamic_cast<IImGUIContext*>(pImGUIContextInstance);
	}
}