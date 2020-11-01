#include "./../../include/core/CResourceHandler.h"
#include "./../../include/core/IResourceManager.h"
#include "./../../include/core/IResource.h"


namespace TDEngine2
{
	CResourceHandler::CResourceHandler() :
		CBaseObject(), mpResourceManager(nullptr), mResourceId(TResourceId::Invalid)
	{
	}

	E_RESULT_CODE CResourceHandler::Init(IResourceManager* pResourceManager, TResourceId id)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pResourceManager)
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
		{
			std::lock_guard<std::mutex> lock(mMutex);

			if (!mIsInitialized)
			{
				return RC_FAIL;
			}

			mIsInitialized = false;
		}

		delete this;

		return RC_OK;
	}
	
	E_RESULT_CODE CResourceHandler::SetResourceId(TResourceId id)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		mResourceId = id;

		return RC_OK;
	}

	IResource* CResourceHandler::_getInternal(E_RESOURCE_ACCESS_TYPE type)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		IResource* pResource = mpResourceManager->GetResourceByHandler(this);

		if (type == RAT_STREAMING || pResource->GetState() == RST_PENDING)
		{
			return pResource;
		}

		while (pResource->GetState() != RST_LOADED)
		{
		}

		return pResource;
	}

	TResourceId CResourceHandler::GetResourceId() const
	{
		return mResourceId;
	}

	void CResourceHandler::OnLoaded(const std::function<void(IResource*)>& onLoadedCallback)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		/// \todo implement the method
	}

	bool CResourceHandler::IsValid() const
	{
		std::lock_guard<std::mutex> lock(mMutex);

		return mResourceId != TResourceId::Invalid;
	}

	TDE2_API IResourceHandler* CreateResourceHandler(IResourceManager* pResourceManager, TResourceId id, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceHandler, CResourceHandler, result, pResourceManager, id);
	}
}