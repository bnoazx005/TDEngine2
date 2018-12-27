#include "./../../include/core/CResourceManager.h"
#include "./../../include/core/IResourceLoader.h"
#include "./../../include/core/IJobManager.h"
#include "./../../include/core/IResourceFactory.h"
#include "./../../include/core/CResourceHandler.h"
#include "./../../include/core/IResource.h"
#include <memory>


namespace TDEngine2
{
	CResourceManager::CResourceManager():
		mIsInitialized(false)
	{
	}

	E_RESULT_CODE CResourceManager::Init(IJobManager* pJobManager)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pJobManager)
		{
			return RC_INVALID_ARGS;
		}

		mpJobManager = pJobManager;

		E_RESULT_CODE result = RC_OK;

		IResourceHandler* pInvalidResourceHandler = CreateResourceHandler(this, InvalidResourceId, result);

		if (result != RC_OK)
		{
			return result;
		}

		mResourceHandlersArray.push_back(pInvalidResourceHandler);

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CResourceManager::Free()
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

	TRegisterLoaderResult CResourceManager::RegisterLoader(const IResourceLoader* pResourceLoader)
	{
		std::lock_guard<std::mutex> lock(mMutex);

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

			mResourceLoadersMap[resourceTypeId] = existingDuplicateId;

			return { RC_OK, existingDuplicateId };
		}

		// just reuse existing nullptr slot
		existingDuplicateId = mFreeLoadersEntriesRegistry.front() + 1;

		mFreeLoadersEntriesRegistry.pop_front();

		mRegistredResourceLoaders[existingDuplicateId - 1] = pResourceLoader;

		mResourceLoadersMap[resourceTypeId] = existingDuplicateId;

		return { RC_OK, existingDuplicateId };
	}
	
	E_RESULT_CODE CResourceManager::UnregisterLoader(const TResourceLoaderId& resourceLoaderId)
	{
		std::lock_guard<std::mutex> lock(mMutex);

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

	TRegisterFactoryResult CResourceManager::RegisterFactory(const IResourceFactory* pResourceFactory)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		TRegisterFactoryResult result;

		if (!mIsInitialized || !pResourceFactory)
		{
			result.mResultCode = RC_INVALID_ARGS;

			return result;
		}

		result.mResultCode = RC_OK;

		U32 resourceTypeId = pResourceFactory->GetResourceTypeId(); // an id of a resource's type, which is processed with this loader

		TResourceFactoryId existingDuplicateId = mResourceFactoriesMap[resourceTypeId];

		// if the duplicate exists, just return it
		if (existingDuplicateId != InvalidResourceFactoryId)
		{
			result.mResourceFactoryId = existingDuplicateId;

			return result;
		}

		// there is no empty slots in the array, so allocate a new one
		if (mFreeFactoriesEntriesRegistry.empty())
		{
			existingDuplicateId = mRegistredResourceFactories.size() + 1;

			mRegistredResourceFactories.push_back(pResourceFactory);
			
			mResourceFactoriesMap[resourceTypeId] = existingDuplicateId;

			result.mResourceFactoryId = existingDuplicateId;

			return result;
		}

		// just reuse existing nullptr slot
		existingDuplicateId = mFreeFactoriesEntriesRegistry.front() + 1;

		mFreeFactoriesEntriesRegistry.pop_front();

		mRegistredResourceFactories[existingDuplicateId - 1] = pResourceFactory;

		mResourceFactoriesMap[resourceTypeId] = existingDuplicateId;

		result.mResourceFactoryId = existingDuplicateId;

		return result;
	}

	E_RESULT_CODE CResourceManager::UnregisterFactory(const TResourceFactoryId& resourceFactoryId)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		if (resourceFactoryId == InvalidResourceFactoryId || resourceFactoryId > mRegistredResourceFactories.size())
		{
			return RC_INVALID_ARGS;
		}

		const IResourceFactory* pResourceFactory = mRegistredResourceFactories[resourceFactoryId - 1];

		U32 resourceTypeId = pResourceFactory->GetResourceTypeId(); // an id of a resource's type, which is processed with this factory

		mRegistredResourceFactories[resourceFactoryId - 1] = nullptr;

		mResourceFactoriesMap.erase(resourceTypeId);

		return RC_OK;
	}

	E_ENGINE_SUBSYSTEM_TYPE CResourceManager::GetType() const
	{
		return EST_RESOURCE_MANAGER;
	}

	IResource* CResourceManager::GetResourceByHandler(const IResourceHandler* pResourceHandler) const
	{
		if (!pResourceHandler)
		{
			return nullptr;
		}

		std::lock_guard<std::mutex> lock(mMutex);

		TResourceId resourceId = pResourceHandler->GetResourceId();

		if (resourceId == InvalidResourceId ||
			resourceId >= mResources.size() + 1)
		{
			return nullptr;
		}

		return mResources[resourceId - 1];
	}

	TResourceId CResourceManager::GetResourceId(const std::string& name) const
	{
		if (name.empty())
		{
			return InvalidResourceId;
		}

		TResourcesMap::const_iterator resourceIter = mResourcesMap.find(name);

		if (resourceIter == mResourcesMap.cend())
		{
			return InvalidResourceId;
		}

		return (*resourceIter).second;
	}

	IResourceHandler* CResourceManager::_loadResource(U32 resourceTypeId, const std::string& name)
	{
		TResourceId resourceId = mResourcesMap[name];

		IResource* pResource = nullptr;

		IResourceHandler* pResourceHandler = nullptr;

		if (resourceId != InvalidResourceId) /// needed resource already exists
		{
			pResourceHandler = _createOrGetResourceHandler(resourceId);

			pResource = mResources[resourceId - 1];

			if (pResource->GetState() == RST_PENDING)
			{
				pResource->Load(); /// \todo move loading in the background thread
			}

			return pResourceHandler;
		}

		auto factoryIdIter = mResourceFactoriesMap.find(resourceTypeId);

		if (factoryIdIter == mResourceFactoriesMap.cend())
		{
			return mResourceHandlersArray[0]; /// return invalid handler
		}

		const IResourceFactory* pResourceFactory = mRegistredResourceFactories[(*factoryIdIter).second - 1];
			
		resourceId = _getFreeResourceId();

		mResourcesMap[name] = resourceId;
			
		pResource = pResourceFactory->CreateDefault(name, {});

		mResources[resourceId - 1] = pResource;

		pResourceHandler = _createOrGetResourceHandler(resourceId);

		pResource->Load(); /// \todo move loading in the background thread

		return pResourceHandler;
	}

	IResourceHandler* CResourceManager::_createResource(U32 resourceTypeId, const std::string& name, const TBaseResourceParameters& params)
	{
		TResourceId resourceId = GetResourceId(name);

		if (resourceId != InvalidResourceId)
		{
			return mResourceHandlersArray[resourceId];
		}

		auto factoryIdIter = mResourceFactoriesMap.find(resourceTypeId);

		if (factoryIdIter == mResourceFactoriesMap.cend())
		{
			return mResourceHandlersArray[0]; /// return invalid handler
		}

		const IResourceFactory* pResourceFactory = mRegistredResourceFactories[(*factoryIdIter).second - 1];
		
		IResource* pResource = nullptr;

		IResourceHandler* pResourceHandler = nullptr;

		resourceId = _getFreeResourceId();

		mResourcesMap[name] = resourceId;
		
		/// \todo move it to a background thread
		pResource = pResourceFactory->Create(name, params);

		mResources[resourceId - 1] = pResource;

		pResourceHandler = _createOrGetResourceHandler(resourceId);

		return pResourceHandler;
	}

	IResourceHandler* CResourceManager::_createOrGetResourceHandler(TResourceId resourceId)
	{
		U32 handlerHashValue = mResourceHandlersMap[resourceId];

		if (handlerHashValue) /// just return existing handler
		{
			return mResourceHandlersArray[handlerHashValue];
		}

		IResourceHandler* pNewHandlerInstance = nullptr;

		E_RESULT_CODE result = RC_OK;

		if (mFreeResourceHandlersRegistry.empty())
		{
			handlerHashValue = mResourceHandlersArray.size();

			pNewHandlerInstance = CreateResourceHandler(this, resourceId, result);

			if (result != RC_OK)
			{
				return mResourceHandlersArray[0]; /// invalid handler
			}

			mResourceHandlersArray.push_back(pNewHandlerInstance);

			return pNewHandlerInstance;
		}

		/// reuse existing handler just reset it
		handlerHashValue = mFreeResourceHandlersRegistry.front();

		mFreeResourceHandlersRegistry.pop_front();

		pNewHandlerInstance = mResourceHandlersArray[handlerHashValue];

		pNewHandlerInstance->SetResourceId(resourceId);

		return pNewHandlerInstance;
	}
	
	E_RESULT_CODE CResourceManager::_freeResourceHandler(TResourceId resourceId)
	{
		U32 handlerHashValue = mResourceHandlersMap[resourceId];

		if (!handlerHashValue) /// there is no handler associated with resourceId
		{
			return RC_FAIL;
		}

		IResourceHandler* pNewHandlerInstance = mResourceHandlersArray[handlerHashValue];

		pNewHandlerInstance->SetResourceId(InvalidEntityId);

		mFreeResourceHandlersRegistry.push_back(handlerHashValue);

		return RC_OK;
	}

	TResourceId CResourceManager::_getFreeResourceId()
	{
		TResourceId resourceId = InvalidResourceId;

		if (mFreeResourcesEntriesRegistry.empty())
		{
			resourceId = mResources.size() + 1;

			mResources.push_back(nullptr);

			return resourceId;
		}

		resourceId = mFreeResourcesEntriesRegistry.front();

		mFreeResourcesEntriesRegistry.pop_front();

		return resourceId;
	}

	const IResourceLoader* CResourceManager::_getResourceLoader(U32 resourceTypeId) const
	{
		auto resourceLoaderIdIter = mResourceLoadersMap.find(resourceTypeId);

		if (resourceLoaderIdIter == mResourceLoadersMap.cend())
		{
			return nullptr;
		}

		return mRegistredResourceLoaders[resourceLoaderIdIter->second - 1];
	}


	IResourceManager* CreateResourceManager(IJobManager* pJobManager, E_RESULT_CODE& result)
	{
		CResourceManager* pResourceManagerInstance = new (std::nothrow) CResourceManager();

		if (!pResourceManagerInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pResourceManagerInstance->Init(pJobManager);

		if (result != RC_OK)
		{
			delete pResourceManagerInstance;

			pResourceManagerInstance = nullptr;
		}

		return dynamic_cast<IResourceManager*>(pResourceManagerInstance);
	}
}