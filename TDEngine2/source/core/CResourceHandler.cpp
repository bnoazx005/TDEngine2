#include "./../../include/core/CResourceHandler.h"
#include "./../../include/core/IResourceManager.h"
#include "./../../include/core/IResource.h"


namespace TDEngine2
{
	CResourceHandler::CResourceHandler() :
		CBaseObject(), mpResourceManager(nullptr), mResourceId(InvalidResourceId)
	{
	}

	E_RESULT_CODE CResourceHandler::Init(IResourceManager* pResourceManager, TResourceId id)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pResourceManager || (id == InvalidResourceId))
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;

		mResourceId = id;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CResourceHandler::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		delete this;

		mIsInitialized = false;

		return RC_OK;
	}

	IResource* CResourceHandler::Get(E_RESOURCE_ACCESS_TYPE type)
	{
		IResource* pResource = mpResourceManager->GetResourceByHandler(this);

		if (type == RAT_STREAMING)
		{
			return pResource;
		}

		while (pResource->GetState() != RST_LOADED)
		{
		}

		return pResource;
	}

	void CResourceHandler::OnLoaded(const std::function<void(IResource*)>& onLoadedCallback)
	{
		/// \todo implement the method
	}


	TDE2_API IResourceHandler* CreateResourceHandler(IResourceManager* pResourceManager, TResourceId id, E_RESULT_CODE& result)
	{
		CResourceHandler* pResourceHandlerInstance = new (std::nothrow) CResourceHandler();

		if (!pResourceHandlerInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pResourceHandlerInstance->Init(pResourceManager, id);

		if (result != RC_OK)
		{
			delete pResourceHandlerInstance;

			pResourceHandlerInstance = nullptr;
		}

		return dynamic_cast<IResourceHandler*>(pResourceHandlerInstance);
	}
}