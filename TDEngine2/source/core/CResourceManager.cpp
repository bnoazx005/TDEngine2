#include "./../../include/core/CResourceManager.h"
#include "./../../include/core/IResourceLoader.h"


namespace TDEngine2
{
	CResourceManager::CResourceManager():
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CResourceManager::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		/// \todo implement Init of CResourceManager

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CResourceManager::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		delete this;

		mIsInitialized = false;

		return RC_FAIL;
	}

	TRegisterLoaderResult CResourceManager::RegisterLoader(const IResourceLoader* pResourceLoader)
	{
		if (!mIsInitialized || !pResourceLoader)
		{
			return TRegisterLoaderResult::mInvalidValue;
		}

		U32 resourceTypeId = pResourceLoader->GetResourceTypeId(); // an id of a resource's type, which is processed with this loader

		TResourceLoaderId existingDuplicateId = mResourceLoadersMap[resourceTypeId];

		// if the duplicate exists, just return it
		if (existingDuplicateId != InvalidResourceLoaderId)
		{
			return { RC_OK, existingDuplicateId };
		}

		// there is no empty slots in the array, so allocate a new one
		if (mFreeLoadersEntriesRegistry.empty())
		{
			existingDuplicateId = mRegistredResourceLoaders.size() + 1;

			mRegistredResourceLoaders.push_back(pResourceLoader);

			return { RC_OK, existingDuplicateId };
		}

		// just reuse existing nullptr slot
		existingDuplicateId = mFreeLoadersEntriesRegistry.front() + 1;

		mFreeLoadersEntriesRegistry.pop_front();

		mRegistredResourceLoaders[existingDuplicateId - 1] = pResourceLoader;

		return { RC_OK, existingDuplicateId };
	}
	
	E_RESULT_CODE CResourceManager::UnregisterLoader(const TResourceLoaderId& resourceLoaderId)
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		if (resourceLoaderId == InvalidResourceLoaderId || resourceLoaderId > mRegistredResourceLoaders.size())
		{
			return RC_INVALID_ARGS;
		}
		
		const IResourceLoader* pResourceLoader = mRegistredResourceLoaders[resourceLoaderId - 1];

		U32 resourceTypeId = pResourceLoader->GetResourceTypeId(); // an id of a resource's type, which is processed with this loader

		mRegistredResourceLoaders[resourceLoaderId - 1] = nullptr;

		mResourceLoadersMap.erase(resourceTypeId);

		return RC_OK;
	}

	E_ENGINE_SUBSYSTEM_TYPE CResourceManager::GetType() const
	{
		return EST_RESOURCE_MANAGER;
	}

	IResource* CResourceManager::GetResourceByHandler(const IResourceHandler* pResourceHandler) const
	{
		/// \todo implement the method
		return nullptr;
	}

	IResourceHandler* CResourceManager::_loadResource(U32 resourceTypeId, const std::string& name)
	{
		/// \todo implement the method
		return nullptr;
	}


	IResourceManager* CreateResourceManager(E_RESULT_CODE& result)
	{
		CResourceManager* pResourceManagerInstance = new (std::nothrow) CResourceManager();

		if (!pResourceManagerInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pResourceManagerInstance->Init();

		if (result != RC_OK)
		{
			delete pResourceManagerInstance;

			pResourceManagerInstance = nullptr;
		}

		return dynamic_cast<IResourceManager*>(pResourceManagerInstance);
	}
}